
/* $Id: h_qh.c 6669 2010-09-06 07:18:06Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#include "h/h_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "h/h_qh.h"

#include "qhull/qhull_a.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static FILE*  fs_qhull_error_fp         = NULL;
static char   fs_qhull_command[ 1000 ]  = "qhull -C-0 -Qc -Qm -Tv";
static double fs_qhull_facet_cosine_min = DBL_NOT_SET;

/* -------------------------------------------------------------------------- */

static void close_qhull_error_file(void);

/* -------------------------------------------------------------------------- */

int set_qhull_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "qhull-error-file"))
       )
    {
        FILE* temp_fp;

        if (fs_qhull_error_fp == NULL) fs_qhull_error_fp = stderr;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("qhull-error-file = %F\n", fs_qhull_error_fp));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("qhull errors are sent to %F\n", fs_qhull_error_fp));
        }
        else
        {
            NRE(temp_fp = kjb_fopen(value, "a"));

            if ((fs_qhull_error_fp != NULL) && (fileno(fs_qhull_error_fp) > 2))
            {
                kjb_fclose(fs_qhull_error_fp);
            }
            else
            {
                /* First time. */
                ERE(add_cleanup_function(close_qhull_error_file));
            }

            fs_qhull_error_fp = temp_fp;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "qhull-facet-cosine-min"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (IS_EQUAL_DBL(fs_qhull_facet_cosine_min, DBL_NOT_SET))
            {
                ERE(pso("qhull-facet-cosine-min = off\n"));
            }
            else
            {
                ERE(pso("qhull-facet-cosine-min = %e\n",
                        fs_qhull_facet_cosine_min));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (IS_EQUAL_DBL(fs_qhull_facet_cosine_min, DBL_NOT_SET))
            {
                ERE(pso("Near parallel adjacent hull facets are not merged.\n"));
            }
            else
            {
                ERE(pso("Adjacent hull facets with cosine less than %e are merged.\n",
                        fs_qhull_facet_cosine_min));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_qhull_facet_cosine_min = DBL_NOT_SET;
        }
        else
        {
            double temp_real;

            ERE(ss1d(value, &temp_real));

            fs_qhull_facet_cosine_min = temp_real;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "qhull-command"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("qhull-command = \"%s\"\n", fs_qhull_command));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Default qhull command is \"%s\".\n", fs_qhull_command));
        }
        else
        {
            BUFF_CPY(fs_qhull_command, value);
        }

        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* -------------------------------------------------------------------------- */

/*
// IMPORTANT : Treat this module as static to m_hull.c. It should only be called
//             by find_convex_hull(). The code is in a separte module in order
//             to control including and loading.
*/

#include "h/h_qh.h"

/*
// NOTE: The mess that follows is not entirely my fault!
//
// The was adapted from some of the qhull code provided by the geometry center.
// I have tried to clean it up a bit, for the purpose of error reporting;
// however, I recomend not dealing with this stuff if possible.
*/


/*
// This code is not dead!
*/
char qh_version[] = "qhull";


int qh_find_convex_hull
(
    Matrix*         point_mp,
    int*            num_vertices_ptr,
    int*            num_facets_ptr,
    Matrix**        vertex_mpp,
    Matrix**        normal_mpp,
    Vector**        b_value_vpp,
    Matrix_vector** facets_ptr,
    FILE*           geom_view_fp
)
{
    static int num_vertices;        /* Make static so it can survive longjmp. */
    static int result;              /* Make static so it can survive longjmp. */
    static int clean_up_sets_error; /* Make static so it can survive longjmp. */
    static int build_hull_error;    /* Make static so it can survive longjmp. */
    static int clean_up_memory_error;/*Make static so it can survive longjmp. */
    static int num_facets;           /*Make static so it can survive longjmp. */
    int        dim;
    int        num_points;
    int        tempsize      = qh_setsize((setT*)qhmem.tempstack);
    int        allpoints;
    int        curlong;
    int        totlong;
    int        exitcode;
    coordT*    in_points;
    coordT*    in_points_pos;
    setT*      vertices;
    vertexT*   vertex;
    vertexT**  vertexp;
    facetT*    facet;
    setT*      points        = NULL;
    pointT*    point;
    boolT      ismalloc;
    char       command[ 1000 ];
    int        id;
    Matrix**   facet_mp_list_pos;
    int        num_points_in_facet;
    int        i;
    int        j;
    int        facet_count;
    int        point_i;
    int        point_n;


#ifdef TEST_CRASH_HANDLING
    if (kjb_rand() < 0.2)
    {
        kill_myself(11);
    }
#endif 

    num_facets = 0;

    dim = point_mp->num_cols;
    num_points = point_mp->num_rows;

#ifdef TEST_QHULL_CRACH_CODE
    if (kjb_rand() > 0.8) TEST_PSO(("%d\n", *(int*)num_facets));
#endif

    /*
    //  Set the pointers to null, because if we fail, we try to free anything
    //  that is not null. BUT, due to the "fancy" qhull error handler, we may
    //  get to the deallocation point before we have allocated anything.   Thus
    //  we need to make sure that we can safely free, and to do this we set the
    //  pointers in question to NULL.   This step is not strictly necessary
    //  under the assumption that we are being called from find_convex_hull(),
    //  as that routine has already set the pointers as they are set below, but
    //  rather than assume this is always the case, we simply make sure.
    */
    if (vertex_mpp != NULL)
    {
        free_matrix(*vertex_mpp);
        *vertex_mpp = NULL;
    }
    if (normal_mpp != NULL)
    {
        free_matrix(*normal_mpp);
        *normal_mpp = NULL;
    }
    if (b_value_vpp != NULL)
    {
        free_vector(*b_value_vpp);
        *b_value_vpp = NULL;
    }
    if (facets_ptr != NULL)
    {
        free_matrix_vector(*facets_ptr);
        *facets_ptr = NULL;
    }

    NRE(in_points = N_TYPE_MALLOC(realT, dim*num_points));

    in_points_pos = in_points;

    for (i=0; i<num_points; i++)
    {
        for (j=0; j<dim; j++)
        {
            *in_points_pos = (realT) ((point_mp->elements)[i][j]);
            in_points_pos++;
        }
    }

    if (fs_qhull_error_fp == NULL) fs_qhull_error_fp = stderr;

    if (geom_view_fp != NULL)
    {
        qh_init_A(stdin, geom_view_fp, fs_qhull_error_fp, 0, NULL);
    }
    else
    {
        qh_init_A (stdin, stdout, fs_qhull_error_fp, 0, NULL);
    }

    result                = NO_ERROR;
    build_hull_error      = FALSE;
    clean_up_sets_error   = FALSE;
    clean_up_memory_error = FALSE;

    exitcode = setjmp (qh errexit);

    if (! exitcode)
    {
        /* command = "qhull -C-0 -Qc -Qm -Tv -Po -E1.0e-8"; */
        /* command = "qhull -C-0 -Qc -Qm -Tv -E1.0e-8 -A0.9"; */
        BUFF_CPY(command, fs_qhull_command);

        if (! IS_EQUAL_DBL(fs_qhull_facet_cosine_min, DBL_NOT_SET))
        {
            char qhull_facet_cosine_min_str[ 100 ];

            if (kjb_sprintf(qhull_facet_cosine_min_str,
                            sizeof(qhull_facet_cosine_min_str), " -A%e",
                            fs_qhull_facet_cosine_min)
                == ERROR)
            {
                SET_CANT_HAPPEN_BUG();
                return ERROR;
            }

            BUFF_CAT(command, qhull_facet_cosine_min_str);
        }

        qh_initflags(command);

        ismalloc = False;     /* True if qhull should 'free(points)' at end */
        qh_init_B(in_points, num_points, dim, ismalloc);

        qh_qhull();

        qh_check_output();

        allpoints = qh num_points + qh_setsize (qh other_points);

        points = qh_settemp (allpoints);

        qh_setzero (points, 0, allpoints);

        /*LINTED warning: constant operand to op: "!"*/
        vertices = qh_facetvertices (qh facet_list, NULL, !qh_ALL);

        FOREACHvertex_(vertices)
        {
           id = qh_pointid (vertex->point);

           if (id >= 0)
           {
               SETelem_(points, id)= vertex->point;
           }
        }

        qh_settempfree (&vertices);

        num_vertices = 0;

        FOREACHpoint_i_(points)
        {
           if (point)
           {
               num_vertices++;
           }
        }

        *num_vertices_ptr = num_vertices;

        if (vertex_mpp != NULL)
        {
            (*vertex_mpp) = create_matrix(num_vertices, dim);

            if ((*vertex_mpp) == NULL)
            {
                longjmp(qh errexit, qh_ERRmem);
            }

            i = 0;

            FOREACHpoint_i_(points)
           {
               if (point)
               {
                   for (j=0; j<dim; j++)
                   {
                       ((*vertex_mpp)->elements)[i][j] = point[j];
                   }
                   i++;
               }
           }
        }

        num_facets = 0;

        FORALLfacet_(qh facet_list)
        {
            num_facets++;
        }

        *num_facets_ptr = num_facets;

        if (facets_ptr != NULL)
        {
            *facets_ptr = create_matrix_vector(num_facets);

            if (*facets_ptr == NULL)
            {
                longjmp(qh errexit, qh_ERRmem);
            }

            facet_mp_list_pos = (*facets_ptr)->elements;

            FORALLfacet_(qh facet_list)
            {
                vertices = facet->vertices;
                qh_setzero (points, 0, allpoints);

                FOREACHvertex_(vertices)
                {
                    id = qh_pointid (vertex->point);

                    if (id >= 0)
                    {
                        SETelem_(points, id)= vertex->point;
                    }
                }

                num_points_in_facet = 0;

                FOREACHpoint_i_(points)
                {
                    if (point)
                    {
                        num_points_in_facet++;
                    }
                }

                *facet_mp_list_pos = create_matrix(num_points_in_facet, dim);

                if ((*facet_mp_list_pos) == NULL)
                {
                    longjmp(qh errexit, qh_ERRmem);
                }

                i = 0;

                FOREACHpoint_i_(points)
                {
                    if (point)
                    {
                        for (j=0; j<dim; j++)
                        {
                            ((*facet_mp_list_pos)->elements)[i][j] = point[j];
                        }

                        i++;
                    }
                }
               facet_mp_list_pos++;
           }
        }

        if (normal_mpp != NULL)
        {
            (*normal_mpp) = create_matrix(num_facets, dim);

            if ((*normal_mpp) == NULL)
            {
                longjmp(qh errexit, qh_ERRmem);
            }

            facet_count = 0;

            FORALLfacet_(qh facet_list)
            {
                for (j=0; j<dim; j++)
                {
                    ((*normal_mpp)->elements)[facet_count][j] =
                                                            (facet->normal)[j];
                }
                facet_count++;
            }
        }

        if (b_value_vpp != NULL)
        {
            double b_value, b_value_sum;
            int point_count;


            (*b_value_vpp) = create_vector(num_facets);

            if ((*b_value_vpp) == NULL)
            {
                longjmp(qh errexit, qh_ERRmem);
            }

            facet_count = 0;

            FORALLfacet_(qh facet_list)
            {
                vertices = facet->vertices;
                qh_setzero (points, 0, allpoints);

                FOREACHvertex_(vertices)
                {
                   id= qh_pointid (vertex->point);
                   if (id >= 0)
                   SETelem_(points, id)= vertex->point;
               }

                i = 0;
                b_value_sum = 0.0;
                point_count = 0;

                FOREACHpoint_i_(points)
                {
                   if (point)
                   {
                      /*
                       * The sum for each facet should be the same.
                       * We are summing over all facets to get a (hopefully)
                       * better answer by averaging. If need be, this code
                       * could be made to run faster by simply computing the
                       * result for one facet.
                       */
                       for (j=0; j<dim; j++)
                       {
                           b_value_sum += point[j] * (facet->normal)[j];
                       }

                       point_count++;
                       i++;
                   }
               }

               b_value = b_value_sum / point_count;
               ((*b_value_vpp)->elements)[facet_count] = b_value;
               facet_count++;
           }
        }

        if (geom_view_fp != NULL)
        {
            qh_appendprint (qh_PRINTgeom);
            qh_produce_output();
        }

        exitcode = qh_ERRnone;
    }

    if (points != NULL)
    {
        qh_settempfree (&points);
    }

    kjb_free(in_points);

    if (exitcode)
    {
        if (vertex_mpp != NULL)
        {
            free_matrix(*vertex_mpp);
            *vertex_mpp = NULL;
        }

        if (facets_ptr != NULL)
        {
            free_matrix_vector(*facets_ptr);
            *facets_ptr = NULL;
        }

        if (normal_mpp != NULL)
        {
            free_matrix(*normal_mpp);
            *normal_mpp = NULL;
        }

        if (b_value_vpp != NULL)
        {
            free_vector(*b_value_vpp);
            *b_value_vpp = NULL;
        }

        result = ERROR;
        build_hull_error = TRUE;
    }

    if (qh_setsize ((setT *)qhmem.tempstack) != tempsize)
    {
        clean_up_sets_error = TRUE;
        result = ERROR;
    }

    /*LINTED warning: constant operand to op: "!"*/
    qh_freeqhull(!qh_ALL);
    qh_memfreeshort (&curlong, &totlong);

    if (curlong || totlong)
    {
        clean_up_memory_error = TRUE;
        result = ERROR;
    }

    if (result == ERROR)
    {
        set_error("Convex hull not found due to problems in qhull.");

        if (clean_up_sets_error)
        {
            if (build_hull_error)
            {
                add_error("Subsequent qhull error: temporary sets not empty");
            }
            else
            {
                add_error("Qhull internal error: temporary sets not empty");
            }
        }

        if (clean_up_memory_error)
        {
            if (build_hull_error)
            {
                add_error("Subsequent qhull error: qh_memfreeshort failed.");
            }
            else
            {
                add_error("Qhull error: qh_memfreeshort failed.");
            }

            add_error("Can't happen if qh_NOmem is defined!");
        }

        if (build_hull_error)
        {
            add_error("Computation of convex hull failed");
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void close_qhull_error_file()
{

    if ((fs_qhull_error_fp != NULL) && (fileno(fs_qhull_error_fp) > 2))
    {
        kjb_fclose(fs_qhull_error_fp);
    }

    /* Guard against second close. Should not really happen. */
    fs_qhull_error_fp = NULL;
}


#ifdef __cplusplus
}
#endif

