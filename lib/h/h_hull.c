
/* $Id: h_hull.c 21596 2017-07-30 23:33:36Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author). (Lindsay Martin
|  contributed to the documentation of this code).
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

#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define LOTS_OF_POINTS   (1000000)

#ifdef TEST
    #define CONSTRAINT_TOLERENCE  (0.001)
    #define RELATIVE_CONSTRAINT_TOLERENCE  (0.01)
    #define NORMAL_TOLERENCE      (0.001)
#endif

/* -------------------------------------------------------------------------- */

static int fs_hull_fill_resolution         = 200;
/*
// Orginally, we expanded hulls exactly, which is fast and logical, but leads to
// precision problems under the normal use. Thus I coded up expansion which is
// slower, and not exaclty what is wanted, but does not increase the number of
// hull facets. Because it is so slow, we leave the default as before. Note that
// some of the precision problems have since been mitigated by other means.
*/
static int     fs_recompute_hull_level = 0;
static jmp_buf fs_qhull_crash_env;
static Vector* fs_cached_n_dot_x_vp = NULL;

/* -------------------------------------------------------------------------- */

static Hull* find_convex_hull_2(const Matrix* input_point_mp, unsigned long options);

static Hull* compute_ordered_2D_hull(Hull* hp);

static Hull* find_convex_hull_3
(
    const Matrix* input_point_mp, /* Address of point matrix. */
    unsigned long         options         /* Returned data structure flags. */
);

static int fill_3D_hull(Hull* hp, int resolution);
static int fill_2D_hull(Hull* hp, int resolution);

static TRAP_FN_RETURN_TYPE qhull_crash_fn(TRAP_FN_ARGS);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

/* -------------------------------------------------------------------------- */

int set_hull_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_int_value;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);


    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hfr")
          || match_pattern(lc_option, "hull-fill-resolution")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_hull_fill_resolution > 0)
            {
                ERE(pso("hull-fill-resolution = %d\n",
                        fs_hull_fill_resolution));
            }
            else
            {
                ERE(pso("hull-fill-resolution = off\n"));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_hull_fill_resolution > 0)
            {
                ERE(pso("Hull fill resolution is %d.\n",
                        fs_hull_fill_resolution));
            }
            else
            {
                ERE(pso("Hull filling is not used.\n"));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_hull_fill_resolution = NOT_SET;
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_hull_fill_resolution = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "recompute-hull")
          || match_pattern(lc_option, "recompute-hulls")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Convex hulls are recomputed %d times using vertices as input.\n",
                    fs_recompute_hull_level));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("recompute-hulls = %d\n", fs_recompute_hull_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_recompute_hull_level = temp_int_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           create_hull
 *
 * Creates an empty convex hull structure.
 *
 * This routine dynamically allocates a convex hull structure. All
 * numeric members are initialized to NOT_SET. All pointer members
 * are set to NULL.
 *
 * Returns:
 *    Pointer to a Hull structure, or NULL if memory allocation fails.
 *
 * Note:
 *    This routine is of not much use outside the hull modulue, and should
 *    likely be static. If it is really needed outside the hull module, let the
 *    author know, otherwise it may become static!
 *
 * Related:
 *    Hull
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/

Hull* create_hull(void)
{
    Hull* hp;


    NRN(hp = TYPE_MALLOC(Hull));

    hp->dimension          = NOT_SET;
    hp->num_vertices       = NOT_SET;
    hp->num_facets         = NOT_SET;

    hp->vertex_mp          = NULL;
    hp->normal_mp          = NULL;
    hp->b_value_vp         = NULL;
    hp->facets             = NULL;
    hp->facet_angles_vp    = NULL;
    hp->geom_view_geometry = NULL;

    hp->min_x              = DBL_NOT_SET;
    hp->min_y              = DBL_NOT_SET;
    hp->min_z              = DBL_NOT_SET;
    hp->max_x              = DBL_NOT_SET;
    hp->max_y              = DBL_NOT_SET;
    hp->max_z              = DBL_NOT_SET;
    hp->filled_scale_x     = DBL_NOT_SET;
    hp->filled_scale_y     = DBL_NOT_SET;
    hp->filled_scale_z     = DBL_NOT_SET;
    hp->filled_2D_array    = NULL;
    hp->filled_3D_array    = NULL;

    hp->filled_resolution  = NOT_SET;

    return hp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           free_hull
 *
 * Deallocates a convex hull structure.
 *
 * This routine frees the memory allocated to all vector and matrix
 * data members of the convex hull structure.
 *
 * Related:
 *    Hull
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/
void free_hull(Hull* hp)
{


    if (hp == NULL) return;

    free_matrix(hp->vertex_mp);
    free_matrix(hp->normal_mp);
    free_vector(hp->b_value_vp);
    free_vector(hp->facet_angles_vp);
    free_matrix_vector(hp->facets);

    kjb_free(hp->geom_view_geometry);

    free_2D_byte_array(hp->filled_2D_array);
    free_3D_byte_array(hp->filled_3D_array);

    kjb_free(hp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           copy_hull
 *
 * Creates a copy of an existing convex hull.
 *
 * Copies the contents of the source convex hull to the target hull,
 * creating the new hull if required.
 *
 * The first argument is the address of the target hull. If the target hull
 * itself is NULL, then a hull is created. If the target hull already exists,
 * its contents are overwritten.
 *
 * Returns:
 *    NO_ERROR if the copy is successful, ERROR on failure. This routine can
 *    only fail if storage allocation fails.
 *
 * Related:
 *    Hull, create_hull
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/
int copy_hull(Hull** target_hp_ptr, const Hull* hp)
{
    Hull* target_hp;


    if (hp == NULL)
    {
        free_hull(*target_hp_ptr);
        *target_hp_ptr = NULL;
        return NO_ERROR;
    }

    if (*target_hp_ptr == NULL)
    {
        NRE(*target_hp_ptr = create_hull());
    }

    target_hp = *target_hp_ptr;

    target_hp->dimension    = hp->dimension;
    target_hp->num_vertices = hp->num_vertices;
    target_hp->num_facets   = hp->num_facets;

    ERE(copy_matrix(&(target_hp->vertex_mp), hp->vertex_mp));
    ERE(copy_matrix(&(target_hp->normal_mp), hp->normal_mp));
    ERE(copy_vector(&(target_hp->b_value_vp), hp->b_value_vp));
    ERE(copy_vector(&(target_hp->facet_angles_vp), hp->facet_angles_vp));

    ERE(copy_matrix_vector(&(target_hp->facets), hp->facets));

    if (hp->geom_view_geometry == NULL)
    {
        target_hp->geom_view_geometry = NULL;
    }
    else
    {
        UNTESTED_CODE();
        kjb_free(target_hp->geom_view_geometry);
        NRE(target_hp->geom_view_geometry=kjb_strdup(hp->geom_view_geometry));
    }

    if (hp->filled_2D_array != NULL)
    {
        int i, j;
        int resolution = hp->filled_resolution + 1;

        free_2D_byte_array(target_hp->filled_2D_array);
        target_hp->filled_2D_array = allocate_2D_byte_array(resolution,
                                                            resolution);
        for (i=0; i<resolution; i++)
        {
            for (j=0; j<resolution; j++)
            {
                (target_hp->filled_2D_array)[ i ][ j ] =
                                                (hp->filled_2D_array)[ i ][ j ];
            }
        }
    }

    if (hp->filled_3D_array != NULL)
    {
        int i, j, k;
        int resolution = hp->filled_resolution + 1;

        free_3D_byte_array(target_hp->filled_3D_array);
        target_hp->filled_3D_array = allocate_3D_byte_array(resolution,
                                                            resolution,
                                                            resolution);
        for (i=0; i<resolution; i++)
        {
            for (j=0; j<resolution; j++)
            {
                for (k=0; k<resolution; k++)
                {
                    (target_hp->filled_3D_array)[ i ][ j ][ k ] =
                                           (hp->filled_3D_array)[ i ][ j ][ k ];
                }
            }
        }
    }

    target_hp->filled_scale_x = hp->filled_scale_x;
    target_hp->filled_scale_y = hp->filled_scale_y;
    target_hp->filled_scale_z = hp->filled_scale_z;

    target_hp->min_x = hp->min_x;
    target_hp->min_y = hp->min_y;
    target_hp->min_z = hp->min_z;

    target_hp->max_x = hp->max_x;
    target_hp->max_y = hp->max_y;
    target_hp->max_z = hp->max_z;

    target_hp->filled_resolution = hp->filled_resolution;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_convex_hull
 *
 * Finds the convex hull.
 *
 * This routine is similar to find_convex_hull(), except that it implements the
 * KJB library semantics with respect to storage allocation.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Related:
 *    Hull
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/

int get_convex_hull(Hull** hp_ptr, const Matrix* mp, unsigned long options)
{

    free_hull(*hp_ptr);

    *hp_ptr = find_convex_hull(mp, options);

    if (*hp_ptr == NULL)
    {
        return ERROR;
    }
    else
    {
        return TRUE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           find_convex_hull
 *
 * Finds the convex hull.
 *
 * Finds the convex hull of a set of points according to the parameters in
 * the "options" argument. This routine is a wrapper to the "QHull" code
 * released by the Geomentry Centre at the University of Minneapolis.
 *
 * The first argument "point_mp" is the address of a matrix containing
 * the input point set.
 *
 * The second argument "options" indicates the options that may be set to
 * control the output of the routine. The following option flags may be
 * ANDed together to form the "options" argument:
 *
 * |    Option Flag                     Description
 * |
 * |    HULL_VERTICES                   Return hull verticies.
 * |    HULL_NORMALS                    Return facet normals in hull.
 * |    HULL_B_VALUES                   Return dot product of facet with vertex
 * |    HULL_FACETS                     Return matrix of facets in hull.
 * |    HULL_GEOM_VIEW_GEOMETRY         Generate geometry output file
 * |                                     for viewing computed hulls using
 * |                                     the external program "Geomview".
 *
 * The first four options may be specified using the default options
 * flag DEFAULT_HULL_OPTIONS.
 *
 * Returns:
 *    Address of the convex hull structure on success, NULL on failure.
 *
 * Related:
 *    Hull
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/

Hull* find_convex_hull(const Matrix* input_point_mp, unsigned long options)
{
    const int num_points = input_point_mp->num_rows;
    const int num_dim = input_point_mp->num_cols;
    int i, j;
    Hull*   hp;
    Matrix* temp_point_mp = NULL;
    Vector* temp_row_vp = NULL;


    if (num_points < LOTS_OF_POINTS)
    {
        hp = find_convex_hull_2(input_point_mp, options);
    }
    else
    {
        int step = 1 + num_points / LOTS_OF_POINTS;
        int temp_num_points = 1 + (num_points - 1) / step;
        int num_points_outside = NOT_SET;
        int is_inside;

        dbp("***************************************************************");
        dbp("***************************************************************");
        dbp("***************************************************************");
        dbp("***************************************************************");

        dbi(step);
        dbi(temp_num_points);

        ERN(get_target_matrix(&temp_point_mp, temp_num_points, num_dim));

        for (i = 0; i < temp_num_points; i++)
        {
            for (j = 0; j < num_dim; j++)
            {
                temp_point_mp->elements[ i ][ j ] = input_point_mp->elements[ i * step ][ j ];
            }
        }

        ASSERT(i == temp_point_mp->num_rows);

        if ((hp = find_convex_hull_2(temp_point_mp, DEFAULT_HULL_OPTIONS)) == NULL)
        {
            NOTE_ERROR();
            free_matrix(temp_point_mp);
            return NULL;
        }

        num_points_outside = hp->num_vertices;

        dbi(hp->num_vertices);

        for (i = 0; i < num_points; i++)
        {
            if (    (get_matrix_row(&temp_row_vp, input_point_mp, i) == ERROR)
                 || ((is_inside = is_point_inside_hull(hp, temp_row_vp)) == ERROR)
               )
            {
                NOTE_ERROR();
                free_hull(hp);
                free_matrix(temp_point_mp);
                free_vector(temp_row_vp);
                return NULL;
            }

            if ( ! is_inside) num_points_outside++;
        }

        if (    (get_target_matrix(&temp_point_mp, num_points_outside, num_dim) == ERROR)
             || (ow_copy_matrix(temp_point_mp, 0, 0, hp->vertex_mp) == ERROR)
           )
        {
            NOTE_ERROR();
            free_hull(hp);
            free_matrix(temp_point_mp);
            free_vector(temp_row_vp);
            return NULL;
        }

        dbi(num_points_outside);

        num_points_outside = hp->num_vertices;

        for (i = 0; i < num_points; i++)
        {
            if (    (get_matrix_row(&temp_row_vp, input_point_mp, i) == ERROR)
                 || ((is_inside = is_point_inside_hull(hp, temp_row_vp)) == ERROR)
               )
            {
                NOTE_ERROR();
                free_hull(hp);
                free_matrix(temp_point_mp);
                free_vector(temp_row_vp);
                return NULL;
            }

            if ( ! is_inside)
            {
                if (put_matrix_row(temp_point_mp, temp_row_vp, num_points_outside) == ERROR)
                {
                    NOTE_ERROR();
                    free_hull(hp);
                    free_matrix(temp_point_mp);
                    free_vector(temp_row_vp);
                    return NULL;
                }

                num_points_outside++;
            }
        }

        free_hull(hp);

        hp = find_convex_hull_2(temp_point_mp, options);

        dbp("***************************************************************");
        dbp("***************************************************************");
        dbp("***************************************************************");
        dbp("***************************************************************");

    }

#ifdef TEST
    /*
    // Code which verifies the convex hull. If the verification fails, then we
    // try to find a new hull using the vertices of the result hull as the new
    // input. We do this a few times. If we don't succeed, we proceed
    // optimistically with a less than perfect hull.
    */
    if (hp != NULL)
    {
        int try_count;
        int result;

        for (try_count = 0; try_count<3; try_count++)
        {
            result = NO_ERROR;

            if (    (options & HULL_VERTICES)
                 && (options & HULL_NORMALS)
                 && (options & HULL_B_VALUES)
                 && (options & HULL_FACETS)
               )
            {
                Matrix* facet_mp;
                Vector* normal_vp      = NULL;
                Vector* point_vp       = NULL;
                Vector* line_vp        = NULL;
                Vector* center_vp      = NULL;
                double  dot_product;
                double  b;
                double  constraint_err;

                for (i=0; i<hp->num_facets; i++)
                {
                    if (result == ERROR) break;

                    facet_mp = hp->facets->elements[ i ];

                    result = average_matrix_rows(&center_vp, facet_mp);

                    if (result != ERROR)
                    {
                        result = get_matrix_row(&normal_vp, hp->normal_mp, i);
                    }

                    if (result != ERROR)
                    {
                        result = ow_normalize_vector_2(normal_vp,
                                                     NORMALIZE_BY_MAGNITUDE,
                                                     (double*)NULL);
                    }

                    for (j=0; j<facet_mp->num_rows; j++)
                    {
                        if (result == ERROR) break;

                        result = get_matrix_row(&point_vp, facet_mp, j);

                        if (result != ERROR)
                        {
                            result = get_dot_product(point_vp, normal_vp, &dot_product);
                        }

                        if (result != ERROR)
                        {
                            b = hp->b_value_vp->elements[ i ];

                            constraint_err = MIN_OF(ABS_OF(dot_product - b),
                                                    ABS_OF((dot_product - b)/
                                                               MAX_OF(dot_product, b)));

                            if (constraint_err > CONSTRAINT_TOLERENCE)
                            {
                                set_error("|(x.n - b)/max(x.n,b)| = %e > %e.",
                                          constraint_err,
                                          CONSTRAINT_TOLERENCE);
                                result = ERROR;
                            }
                        }

                        if (result != ERROR)
                        {
                            result = subtract_vectors(&line_vp, point_vp, center_vp);
                        }

                        if (result != ERROR)
                        {
                            result = ow_normalize_vector_2(line_vp,
                                                         NORMALIZE_BY_MAGNITUDE,
                                                         (double*)NULL);
                        }

                        if (result != ERROR)
                        {
                            result = get_dot_product(line_vp, normal_vp,
                                                     &dot_product);
                        }

                        if (result != ERROR)
                        {
                            if (ABS_OF(dot_product) > NORMAL_TOLERENCE)
                            {
                                set_error("| n.(center-point)/(|n||center-point|) | = %e >  %e.",
                                          ABS_OF(dot_product),
                                          NORMAL_TOLERENCE);
                                result = ERROR;
                            }
                        }
                    }
                }

                for (i=0; i<input_point_mp->num_rows; i++)
                {
                    result = get_matrix_row(&point_vp, input_point_mp, i);

                    if (result != ERROR)
                    {
                        double error;

                        if (! is_point_inside_hull_2(hp, point_vp,
                                                     CONSTRAINT_TOLERENCE,
                                                     RELATIVE_CONSTRAINT_TOLERENCE,
                                                     &error))
                        {
                            dbe(error);
                            result = ERROR;
                        }
                    }
                }

                free_vector(normal_vp);
                free_vector(point_vp);
                free_vector(line_vp);
                free_vector(center_vp);

                if (result == ERROR)
                {
                    insert_error("Test of computed convex hull failed.");
                    add_error("Re-finding hull from vertex points.");
                    kjb_print_error();

                    if (copy_matrix(&temp_point_mp, hp->vertex_mp) == ERROR)
                    {
                        free_hull(hp);
                        hp = NULL;
                        break;
                    }

                    free_hull(hp);

                    hp = find_convex_hull_3(temp_point_mp, options);

                    if (hp == NULL) break;
                }
                else
                {
                    if (try_count > 0)
                    {
                        TEST_PSO(("Good hull found on try_count %d.\n", try_count + 1));
                    }

                    break;
                }
            }
        }

        if (result == ERROR)
        {
            set_error("No Good hull found even with %d tries.", try_count);
            add_error("Contuining processing optimistically.");
            kjb_print_error();
        }
    }

#endif

    free_matrix(temp_point_mp);
    free_vector(temp_row_vp);

    if (    (hp != NULL)
         && (hp->dimension == 2)
         && (hp->facets != NULL)
         && (    (options & ORDER_2D_HULL_FACETS)
              || (options & COMPUTE_2D_HULL_FACET_ANGLES)
            )
       )
    {
        Hull* ordered_hp = compute_ordered_2D_hull(hp);

        free_hull(hp);
        hp = ordered_hp;
    }

    return hp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Hull* find_convex_hull_2(const Matrix* input_point_mp, unsigned long options)
{
    int i;
    Hull*   hp;
    Matrix* temp_point_mp = NULL;


#ifdef TEST
    options |= ORDER_2D_HULL_FACETS;
#endif

    verify_matrix(input_point_mp, NULL);

    hp = find_convex_hull_3(input_point_mp, options);

    for (i=0; i<fs_recompute_hull_level; i++)
    {
        if (copy_matrix(&temp_point_mp, hp->vertex_mp) == ERROR)
        {
            free_hull(hp);
            hp = NULL;
        }
        else
        {
            free_hull(hp);
            hp = find_convex_hull_3(temp_point_mp, options);
        }
    }

#ifdef TEST
    /*
    // Code which verifies the convex hull. If the verification fails, then we
    // try to find a new hull using the vertices of the result hull as the new
    // input. We do this a few times. If we don't succeed, we proceed
    // optimistically with a less than perfect hull.
    */
    if (hp != NULL)
    {
        int try_count;
        int result;

        for (try_count = 0; try_count<3; try_count++)
        {
            result = NO_ERROR;

            if (    (options & HULL_VERTICES)
                 && (options & HULL_NORMALS)
                 && (options & HULL_B_VALUES)
                 && (options & HULL_FACETS)
               )
            {
                int     j;
                Matrix* facet_mp;
                Vector* normal_vp      = NULL;
                Vector* point_vp       = NULL;
                Vector* line_vp        = NULL;
                Vector* center_vp      = NULL;
                double  dot_product;
                double  b;
                double  constraint_err;

                for (i=0; i<hp->num_facets; i++)
                {
                    if (result == ERROR) break;

                    facet_mp = hp->facets->elements[ i ];

                    result = average_matrix_rows(&center_vp, facet_mp);

                    if (result != ERROR)
                    {
                        result = get_matrix_row(&normal_vp, hp->normal_mp, i);
                    }

                    if (result != ERROR)
                    {
                        result = ow_normalize_vector_2(normal_vp,
                                                     NORMALIZE_BY_MAGNITUDE,
                                                     (double*)NULL);
                    }

                    for (j=0; j<facet_mp->num_rows; j++)
                    {
                        if (result == ERROR) break;

                        result = get_matrix_row(&point_vp, facet_mp, j);

                        if (result != ERROR)
                        {
                            result = get_dot_product(point_vp, normal_vp, &dot_product);
                        }

                        if (result != ERROR)
                        {
                            b = hp->b_value_vp->elements[ i ];

                            constraint_err = MIN_OF(ABS_OF(dot_product - b),
                                                    ABS_OF((dot_product - b)/
                                                               MAX_OF(dot_product, b)));

                            if (constraint_err > CONSTRAINT_TOLERENCE)
                            {
                                set_error("|(x.n - b)/max(x.n,b)| = %e > %e.",
                                          constraint_err,
                                          CONSTRAINT_TOLERENCE);
                                result = ERROR;
                            }
                        }

                        if (result != ERROR)
                        {
                            result = subtract_vectors(&line_vp, point_vp, center_vp);
                        }

                        if (result != ERROR)
                        {
                            result = ow_normalize_vector_2(line_vp,
                                                         NORMALIZE_BY_MAGNITUDE,
                                                         (double*)NULL);
                        }

                        if (result != ERROR)
                        {
                            result = get_dot_product(line_vp, normal_vp,
                                                     &dot_product);
                        }

                        if (result != ERROR)
                        {
                            if (ABS_OF(dot_product) > NORMAL_TOLERENCE)
                            {
                                set_error("| n.(center-point)/(|n||center-point|) | = %e >  %e.",
                                          ABS_OF(dot_product),
                                          NORMAL_TOLERENCE);
                                result = ERROR;
                            }
                        }
                    }
                }

                for (i=0; i<input_point_mp->num_rows; i++)
                {
                    result = get_matrix_row(&point_vp, input_point_mp, i);

                    if (result != ERROR)
                    {
                        double error;

                        if (! is_point_inside_hull_2(hp, point_vp,
                                                     CONSTRAINT_TOLERENCE,
                                                     RELATIVE_CONSTRAINT_TOLERENCE,
                                                     &error))
                        {
                            dbe(error);
                            result = ERROR;
                        }
                    }
                }

                free_vector(normal_vp);
                free_vector(point_vp);
                free_vector(line_vp);
                free_vector(center_vp);

                if (result == ERROR)
                {
                    insert_error("Test of computed convex hull failed.");
                    add_error("Re-finding hull from vertex points.");
                    kjb_print_error();

                    if (copy_matrix(&temp_point_mp, hp->vertex_mp) == ERROR)
                    {
                        free_hull(hp);
                        hp = NULL;
                        break;
                    }

                    free_hull(hp);

                    hp = find_convex_hull_3(temp_point_mp, options);

                    if (hp == NULL) break;
                }
                else
                {
                    if (try_count > 0)
                    {
                        TEST_PSO(("Good hull found on try_count %d.\n", try_count + 1));
                    }

                    break;
                }
            }
        }

        if (result == ERROR)
        {
            set_error("No Good hull found even with %d tries.", try_count);
            add_error("Contuining processing optimistically.");
            kjb_print_error();
        }
    }
#endif

    free_matrix(temp_point_mp);

    if (    (hp != NULL)
         && (hp->dimension == 2)
         && (hp->facets != NULL)
         && (    (options & ORDER_2D_HULL_FACETS)
              || (options & COMPUTE_2D_HULL_FACET_ANGLES)
            )
       )
    {
        Hull* ordered_hp = compute_ordered_2D_hull(hp);

        /*
        for (i=0; i<hp->num_facets; i++)
        {
            db_mat(hp->facets->elements[ i ]);
        }
        db_mat(hp->vertex_mp);
        db_mat(hp->normal_mp);
        db_rv(hp->b_value_vp);
        db_rv(hp->facet_angles_vp);
        */

        /*
        for (i=0; i<ordered_hp->num_facets; i++)
        {
            db_mat(ordered_hp->facets->elements[ i ]);
        }
        db_mat(ordered_hp->vertex_mp);
        db_mat(ordered_hp->normal_mp);
        db_rv(ordered_hp->b_value_vp);
        db_rv(ordered_hp->facet_angles_vp);
        */

        free_hull(hp);
        hp = ordered_hp;
    }

    return hp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Hull* compute_ordered_2D_hull(Hull* hp)
{
    int            num_facets         = hp->num_facets;
    Matrix_vector* facets;
    Vector*        b_value_vp;
    Matrix*        normal_mp;
    Matrix_vector* ordered_facets;
    Matrix*        ordered_vertex_mp;
    Vector*        ordered_b_value_vp;
    Matrix*        ordered_normal_mp;
    int*           marked_facets      = INT_MALLOC(num_facets);
    Hull*          ordered_hp         = NULL;
    Vector*        facet_angles_vp    = create_vector(num_facets);
    Vector*        e1_vp              = NULL;
    Vector*        e2_vp              = NULL;
    Vector*        p1_vp              = NULL;
    Vector*        p2_vp              = NULL;
    int            i, j;
    double         dot_product;


    if (    (copy_hull(&ordered_hp, hp) == ERROR)
         || (marked_facets == NULL)
         || (facet_angles_vp == NULL)
       )
    {
        free_hull(ordered_hp);
        kjb_free(marked_facets);
        return NULL;
    }

    ordered_facets = ordered_hp->facets;
    ordered_vertex_mp = ordered_hp->vertex_mp;
    ordered_b_value_vp = ordered_hp->b_value_vp;
    ordered_normal_mp = ordered_hp->normal_mp;

    free_vector(ordered_hp->facet_angles_vp);
    ordered_hp->facet_angles_vp = facet_angles_vp;

    facets = hp->facets;
    b_value_vp = hp->b_value_vp;
    normal_mp = hp->normal_mp;

    /*
    // The first normal, b_value, and facet whill be correct due to the
    // above hull copy, but the vertex may not be correct. Make the vertex
    // the first point of the first facet.
    */
    ordered_vertex_mp->elements[ 0 ][ 0 ] =
        ordered_facets->elements[ 0 ]->elements[ 0 ][ 0 ];

    ordered_vertex_mp->elements[ 0 ][ 1 ] =
        ordered_facets->elements[ 0 ]->elements[ 0 ][ 1 ];

    marked_facets[ 0 ] = TRUE;

    for (i=1; i<num_facets; i++)
    {
        marked_facets[ i ] = FALSE;
    }

    /*
    // Loop bounds: We find the facet that connects to the current one, and
    // put that into the next position. Thus we do not want to do the last
    // one, and hence the fence at "num_facets-1".
    */
    for (i=0; i<num_facets-1; i++)
    {
        Matrix* mp = ordered_facets->elements[ i ];
        double x = mp->elements[ 1 ][ 0 ];
        double y = mp->elements[ 1 ][ 1 ];
        int  min_direction = NOT_SET;
        double min_distance_sqr = DBL_NOT_SET;
        int  min_j = NOT_SET;
        int  direction;

        for (j=1; j<num_facets; j++)
        {
            if ( ! marked_facets[ j ])
            {
                for (direction = 0; direction<2; direction++)
                {
                    Matrix* test_mp = facets->elements[ j ];
                    double dx = test_mp->elements[ direction ][ 0 ] - x;
                    double dy = test_mp->elements[ direction ][ 1 ] - y;
                    double distance_sqr = dx*dx + dy*dy;

                    if (    (min_distance_sqr < 0.0)
                         || (distance_sqr < min_distance_sqr)
                       )
                    {
                        min_distance_sqr = distance_sqr;
                        min_j = j;
                        min_direction = direction;
                    }
                }
            }
        }

        ASSERT(min_j != NOT_SET);
        ASSERT(min_direction != NOT_SET);

        marked_facets[ min_j ] = TRUE;

        /*
        // I am being a bit fancy here. The new found next point is being
        // copied into the facet array location that will be accessed next.
        */

        ordered_facets->elements[ i + 1 ]->elements[ 0 ][ 0 ] =
            facets->elements[ min_j ]->elements[ min_direction ][ 0 ];

        ordered_facets->elements[ i + 1 ]->elements[ 0 ][ 1 ] =
            facets->elements[ min_j ]->elements[ min_direction ][ 1 ];

        ordered_facets->elements[ i + 1 ]->elements[ 1 ][ 0 ] =
            facets->elements[ min_j ]->elements[ 1-min_direction ][ 0 ];

        ordered_facets->elements[ i + 1 ]->elements[ 1 ][ 1 ] =
            facets->elements[ min_j ]->elements[ 1-min_direction ][ 1 ];

        if (ordered_vertex_mp != NULL)
        {
            ordered_vertex_mp->elements[ i + 1 ][ 0 ] =
                ordered_facets->elements[ i + 1 ]->elements[ 0 ][ 0 ];
            ordered_vertex_mp->elements[ i + 1 ][ 1 ] =
                ordered_facets->elements[ i + 1 ]->elements[ 0 ][ 1 ];
        }

        if (normal_mp != NULL)
        {
            ordered_normal_mp->elements[ i + 1 ][ 0 ] =
                                normal_mp->elements[ min_j ][ 0 ];
            ordered_normal_mp->elements[ i + 1 ][ 1 ] =
                                normal_mp->elements[ min_j ][ 1 ];
        }

        if (b_value_vp != NULL)
        {
            ordered_b_value_vp->elements[ i + 1 ] =
                                b_value_vp->elements[ min_j ];
        }
    }

    kjb_free(marked_facets);

    for (i=0; i<num_facets; i++)
    {
        int i2 = (i == (num_facets-1) ? 0 : i + 1);
        Matrix* e1_mp = ordered_facets->elements[ i ];
        Matrix* e2_mp = ordered_facets->elements[ i2 ];

        if (    (get_matrix_row(&p2_vp, e1_mp, 0) == ERROR)
             || (get_matrix_row(&p1_vp, e1_mp, 1) == ERROR)
             || (subtract_vectors(&e1_vp, p2_vp, p1_vp) == ERROR)
             || (get_matrix_row(&p2_vp, e2_mp, 1) == ERROR)
             || (get_matrix_row(&p1_vp, e2_mp, 0) == ERROR)
             || (subtract_vectors(&e2_vp, p2_vp, p1_vp) == ERROR)
             || (get_dot_product(e1_vp, e2_vp, &dot_product) == ERROR)
           )
        {
            free_vector(e1_vp);
            free_vector(e2_vp);
            free_vector(p1_vp);
            free_vector(p2_vp);
            free_hull(ordered_hp);
            return NULL;
        }

        dot_product /= vector_magnitude(e1_vp);
        dot_product /= vector_magnitude(e2_vp);

        facet_angles_vp->elements[ i ] = acos(dot_product) * 180.0 / M_PI;
    }

    free_vector(e1_vp);
    free_vector(e2_vp);
    free_vector(p1_vp);
    free_vector(p2_vp);

    return ordered_hp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Hull* find_convex_hull_3
(
    const Matrix* input_point_mp,
    unsigned long         options
)
{
    IMPORT volatile Bool   io_atn_flag;
    IMPORT volatile Bool   halt_all_output;
    /* These are made static simply to be sure that there are no problems due to
    // the longjmp. Initialization is done as a separate step below. */
    static int             result;
    static FILE*           geom_view_fp;
    static Matrix**        vertex_mpp;
    static Matrix**        normal_mpp;
    static Vector**        b_value_vpp;
    static Matrix_vector** facets_ptr;
    static int             attempt;
    const int              max_num_attempts = 20;
    const double           fraction         = 0.002;
    static Hull*           hp;
    char                   user_id[ 200 ];
    char                   geom_view_file_name[ 200 ];
    char*                  geom_view_geometry;
    int                    read_res;
    off_t                  num_bytes;
    Matrix*                extended_point_mp = NULL;
    Vector*                min_vp   = NULL;
    Vector*                max_vp   = NULL;
    Matrix*                point_mp = NULL;


    result          = NOT_SET;
    geom_view_fp    = NULL;
    vertex_mpp   = NULL;
    normal_mpp   = NULL;
    b_value_vpp  = NULL;
    facets_ptr      = NULL;
    attempt         = 0;

    /*
    // Arguably, this does not belong here, but it is a convenient because
    // find_convex_hull() is used in slow loops.
    */
    if (io_atn_flag)
    {
        set_error("Processing interrupted.");
        halt_all_output = FALSE;
        return NULL;
    }

    if (input_point_mp->num_rows <= input_point_mp->num_cols)
    {
        int i, j;

        verbose_pso(7, "Not enough data (%d) for a %d-D hull. ",
                    input_point_mp->num_rows, input_point_mp->num_cols);
        verbose_pso(7, "Extending the data.\n");

        ERN(get_target_matrix(&extended_point_mp, input_point_mp->num_cols + 1,
                              input_point_mp->num_cols));

        i = 0;

        while (i < input_point_mp->num_rows)
        {
            for (j=0; j < input_point_mp->num_cols; j++)
            {
                extended_point_mp->elements[ i ][ j ] =
                                             input_point_mp->elements[ i ][ j ];
            }
            i++;
        }

        while (i < extended_point_mp->num_rows)
        {
            int ii = i % input_point_mp->num_rows;

            for (j=0; j<input_point_mp->num_cols; j++)
            {
                double factor = 1.0 + fraction * kjb_rand_2();

                extended_point_mp->elements[ i ][ j ] =
                                   factor * input_point_mp->elements[ ii ][ j ];
            }
            i++;
        }

        ERN(copy_matrix(&point_mp, extended_point_mp));
    }
    else
    {
        ERN(copy_matrix(&point_mp, input_point_mp));
    }

    NRN(hp = create_hull());
    hp->dimension = point_mp->num_cols;

    /*
    // We ignore the absence of HULL_VERTICES. The caller gets them whether
    // they think they need them or not (too dangerous otherwise).
    //
    // The code was ....
    //    if (options & HULL_VERTICES) vertex_mpp  = &(hp->vertex_mp);
    */
    vertex_mpp  = &(hp->vertex_mp);

    if (options & HULL_NORMALS)  normal_mpp  = &(hp->normal_mp);
    if (options & HULL_B_VALUES) b_value_vpp = &(hp->b_value_vp);
    if (options & HULL_FACETS)   facets_ptr  = &(hp->facets);

    if (options & HULL_GEOM_VIEW_GEOMETRY)
    {
        UNTESTED_CODE();

        if (get_user_id(user_id, sizeof(user_id)) == ERROR)
        {
            free_hull(hp);
            return NULL;
        }

        ERN(kjb_sprintf(geom_view_file_name, sizeof(geom_view_file_name),
                        "/tmp/%s-%d", user_id, (int)MY_PID));

        geom_view_fp = kjb_fopen(geom_view_file_name, "w+");

        if (geom_view_fp == NULL)
        {
            free_hull(hp);
            return NULL;
        }
    }

    set_abort_trap(qhull_crash_fn);

    while (result != NO_ERROR) 
    {
        if (setjmp(fs_qhull_crash_env))
        {
            FILE* fp;
            const char* file_name = "qhull_crash_data";

            unset_abort_trap();

            set_error("Qhull crashed!");
            add_error("Writing input to %s.", file_name);

            push_error_action(FORCE_ADD_ERROR_ON_ERROR); 

            fp = kjb_fopen(file_name, "w");

            if (fp != NULL)
            {
                fp_write_matrix_full_precision_with_title(point_mp, fp,
                                            "\n# Data which crashed qhull\n");
                kjb_fclose(fp);
            }

            pop_error_action(); 

            result = ERROR;

            if (attempt >= max_num_attempts)
            {
                break;
            }

            set_abort_trap(qhull_crash_fn);
        }

        if (result == ERROR) 
        {
            char error_buff[ 1000 ];

            kjb_get_error(error_buff, sizeof(error_buff)); 

            verbose_pso(2, "Attempt %d of finding the convext hull failed.\n",
                        attempt);
            verbose_pso(2, error_buff); 
            verbose_pso(2, "\n"); 
            verbose_pso(2, "Trying again to find the hull by perturbing data by %.2f%%.\n",
                     100.0 * fraction * attempt);
            ERN(ow_perturb_matrix(point_mp, fraction * attempt));
        }

        attempt++;

        result = qh_find_convex_hull(point_mp,
                                     &(hp->num_vertices),
                                     &(hp->num_facets),
                                     vertex_mpp,
                                     normal_mpp,
                                     b_value_vpp,
                                     facets_ptr,
                                     geom_view_fp);
    }

    unset_abort_trap();

    /*
     * End of setjmp() induced LOOP.
    */

    if (result == ERROR)
    {
        file_db_mat(point_mp);
        file_db_mat(input_point_mp);
    }

    free_matrix(extended_point_mp);
    free_matrix(point_mp);

    if (result == ERROR)
    {
        set_error("Unable to find convex hull even after %d attempts at perturbing the data.",
                  attempt);
        free_hull(hp);
        return NULL;
    }
    else if (attempt > 1)
    {
        verbose_pso(2, "Success on attempt %d (perturbation %.2f%%).\n\n",
                    attempt, 100.0 * fraction * attempt);
    }

    if (options & HULL_GEOM_VIEW_GEOMETRY)
    {
        if (kjb_fflush(geom_view_fp) == ERROR)
        {
            free_hull(hp);
            return NULL;
        }

        rewind(geom_view_fp);

        /* Untestested since get_file_size interface change. */
        UNTESTED_CODE();

        ERN(fp_get_byte_size(geom_view_fp, &num_bytes));

        geom_view_geometry = STR_MALLOC((size_t)num_bytes + ROOM_FOR_NULL );

        if (geom_view_geometry == NULL)
        {
            free_hull(hp);
            return NULL;
        }

        read_res = kjb_fread(geom_view_fp, geom_view_geometry,
                             (size_t)num_bytes);

        if ((read_res < 0) || ((off_t)read_res != num_bytes))
        {
            free_hull(hp);
            set_error("Problem reading geom view geometry from temp file.");
            return NULL;
        }

        geom_view_geometry[ num_bytes ] = '\0';
        hp->geom_view_geometry = geom_view_geometry;
        EPE(kjb_fclose(geom_view_fp));
        EPE(kjb_unlink(geom_view_file_name));
    }

    ERN(get_min_matrix_col_elements(&min_vp, hp->vertex_mp));
    ERN(get_max_matrix_col_elements(&max_vp, hp->vertex_mp));

    hp->min_x = (min_vp->elements)[ 0 ];
    hp->max_x = (max_vp->elements)[ 0 ];

    if (hp->dimension > 1)
    {
        hp->min_y = (min_vp->elements)[ 1 ];
        hp->max_y = (max_vp->elements)[ 1 ];
    }

    if (hp->dimension > 2)
    {
        hp->min_z = (min_vp->elements)[ 2 ];
        hp->max_z = (max_vp->elements)[ 2 ];
    }

    free_vector(min_vp);
    free_vector(max_vp);

    if (options & HULL_FILL)
    {
        UNTESTED_CODE();

        if (fill_hull(hp) == ERROR)
        {
            free_hull(hp);
            return NULL;
        }
    }

    return hp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fill_hull(Hull* hp)
{
    int dim;


    if (fs_hull_fill_resolution <= 0) return NO_ERROR;

    dim = hp->dimension;

    if (dim == 2)
    {
        return fill_2D_hull(hp, fs_hull_fill_resolution);
    }
    else if (dim == 3)
    {
        return fill_3D_hull(hp, fs_hull_fill_resolution);
    }
    else
    {
        set_bug("Filling of %d-D hulls is not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int fill_2D_hull(Hull* hp, int resolution)
{
    Matrix* normal_mp;
    Vector* constraint_vp;
    double**  normal_row_ptr;
    double*   normal_elem_ptr;
    double*   constraint_elem_ptr;
    int     i, j, k;
    int     num_normals;
    double    min_x;
    double    max_x;
    double    min_y;
    double    max_y;
    double    x, y;
    double    x_step;
    double    y_step;
    double    hull_min_y;
    double    hull_max_y;
    double    temp;
    int prev_beg_j, prev_end_j;
    int inside = FALSE;


#ifdef __GNUC__
    prev_beg_j = NOT_SET;
    prev_end_j = NOT_SET;
#endif

    if (hp->filled_resolution == resolution) return NO_ERROR;

    hp->filled_resolution = NOT_SET;

    free_2D_byte_array(hp->filled_2D_array);
    NRE(hp->filled_2D_array = allocate_2D_byte_array(resolution, resolution));

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            (hp->filled_2D_array)[ i ][ j ] = 0;
        }
    }

    normal_mp = hp->normal_mp;
    num_normals = normal_mp->num_rows;

    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    constraint_vp = hp->b_value_vp;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;

    x = min_x;

    for (i=0; i<resolution + 1; i++)
    {
        normal_row_ptr = normal_mp->elements;
        constraint_elem_ptr = constraint_vp->elements;
        hull_min_y = min_y;
        hull_max_y = max_y;

        for (k=0; k<num_normals; k++)
        {
            normal_elem_ptr = *normal_row_ptr;
            temp = (*constraint_elem_ptr) - (*normal_elem_ptr)*x;
            normal_elem_ptr++;

#ifdef HOW_IT_WAS
            if (IS_ZERO_DBL(*normal_elem_ptr))
#else
            if (    (*normal_elem_ptr > -HULL_EPSILON)
                 && (*normal_elem_ptr < HULL_EPSILON)
               )
#endif
            {
#ifdef HOW_IT_WAS
                if (IS_NEGATIVE_DBL(temp))
#else
                if (temp < HULL_EPSILON)
#endif
                {
                    hull_min_y = DBL_MAX;   /* Force no solution */
                    break;
                }
                else
                {
                    /*EMPTY*/
                    ;  /* Do nothing: This normal is irrelavent */
                }
            }
            else if (*normal_elem_ptr > 0.0)
            {
                y = temp / (*normal_elem_ptr);

                if (y < hull_max_y)
                {
                    hull_max_y = y;

                    if (IS_NOT_LESSER_DBL(hull_min_y, hull_max_y))
                    {
                        break;  /* No solution */
                    }
                }
            }
            else
            {
                y = temp / (*normal_elem_ptr);

                if (y > hull_min_y)
                {
                    hull_min_y = y;

                    if (IS_NOT_LESSER_DBL(hull_min_y, hull_max_y))
                    {
                        break;  /* No solution */
                    }
                }
            }

            constraint_elem_ptr++;
            normal_row_ptr++;
        }

        /*
        // if (hull_min_y < hull_max_y)
        //
        // Use macro to properly deal with imprecise math.
        */
        if (IS_NOT_GREATER_DBL(hull_min_y, hull_max_y))
        {
            int cur_beg_j = (int)((hull_min_y - min_y) / y_step);
            int cur_end_j = (int)((hull_max_y - min_y) / y_step);

            if (cur_end_j == resolution) cur_end_j--;
            if (cur_beg_j == resolution) cur_beg_j--;

            if (inside)
            {
                /*LINTED*/
                int beg_j = MIN_OF(prev_beg_j, cur_beg_j);
                /*LINTED*/
                int end_j = MAX_OF(prev_end_j, cur_end_j);

                for (j = beg_j; j<= end_j; j++)
                {
                    (hp->filled_2D_array)[ i-1 ][ j ] = 1;
                }
            }

            inside = TRUE;

            prev_beg_j = cur_beg_j;
            prev_end_j = cur_end_j;
        }
        else
        {
            /* We have now passed the end of the hull--no need to keep going.*/
            if (inside) break;
        }

        x += x_step;
    }

    hp->filled_scale_x = resolution / (max_x - min_x);
    hp->filled_scale_y = resolution / (max_y - min_y);

    hp->filled_resolution = resolution;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
#define SANITY_CHECK
*/

static int fill_3D_hull(Hull* hp, int resolution)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix* normal_mp;
    Vector* constraint_vp;
    double**  normal_row_ptr;
    double*   normal_elem_ptr;
    double*   constraint_elem_ptr;
    int     i, j, k, m;
    int     num_normals;
    double    min_x;
    double    max_x;
    double    min_y;
    double    max_y;
    double    min_z;
    double    max_z;
    double    x, y;
    double    x_step;
    double    y_step;
    double    z_step;
    double    z;
    double    hull_min_z;
    double    hull_max_z;
    double    temp;
    Vector* x_cache_vp;
    double*   x_cache_ptr;
    int     result     = NO_ERROR;
#ifdef SANITY_CHECK
    Vector* test_vp = NULL;
#endif 


    if (hp->filled_resolution == resolution) return NO_ERROR;

    hp->filled_resolution = NOT_SET;

    free_3D_byte_array(hp->filled_3D_array);
    NRE(hp->filled_3D_array = allocate_3D_byte_array(resolution,
                                                     resolution,
                                                     resolution));

    for (i=0; i<resolution; i++)
    {
        for (j=0; j<resolution; j++)
        {
            for (k = 0; k<resolution; k++)
            {
                (hp->filled_3D_array)[ i ][ j ][ k ] = 0;
            }
        }
    }

    normal_mp = hp->normal_mp;

    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    min_z = hp->min_z;
    max_z = hp->max_z;

    num_normals = normal_mp->num_rows;

    constraint_vp = hp->b_value_vp;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;
    z_step = (max_z - min_z) / resolution;

    x = min_x;

#ifdef SANITY_CHECK
    /*
     * More expensive code to do the same thing. 
    */
    verbose_pso(1, "Using SANITY_CHECK method for filling 3D hulls.\n");

    ERE(get_target_vector(&test_vp, 3)); 

    for (i=0; i<resolution; i++)
    {
        y = min_y;

        for (j=0; j<resolution; j++)
        {
            z = min_z;

            for (k = 0; k < resolution; k++)
            {
                test_vp->elements[ 0 ] = x;
                test_vp->elements[ 1 ] = y;
                test_vp->elements[ 2 ] = z;

                (hp->filled_3D_array)[ i ][ j ][ k ] = is_point_inside_hull(hp, test_vp);

                z += z_step;
            }

            y += y_step;
        }
        x += x_step;
    }
    free_vector(test_vp); 
#else 

    NRE(x_cache_vp = create_vector(num_normals));

    for (i=0; i<resolution; i++)
    {
        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            result = ERROR;
            break;
        }

        x_cache_ptr = x_cache_vp->elements;
        normal_row_ptr = normal_mp->elements;

        for (m=0; m<num_normals; m++)
        {
            normal_elem_ptr = *normal_row_ptr;
            *x_cache_ptr = (*normal_elem_ptr) * x;
            x_cache_ptr++;
            normal_row_ptr++;
        }

        y = min_y;

        for (j=0; j<resolution; j++)
        {
            normal_row_ptr = normal_mp->elements;
            constraint_elem_ptr = constraint_vp->elements;
            x_cache_ptr = x_cache_vp->elements;

            hull_min_z = min_z;
            hull_max_z = max_z;

            for (k=0; k<num_normals; k++)
            {
                normal_elem_ptr = *normal_row_ptr;
                normal_elem_ptr++;
                temp = (*constraint_elem_ptr) - (*x_cache_ptr) -
                                                       (*normal_elem_ptr)*y;
                normal_elem_ptr++;

#ifdef HOW_IT_WAS
                if (IS_ZERO_DBL(*normal_elem_ptr))
#else
                if (    (*normal_elem_ptr > -HULL_EPSILON)
                     && (*normal_elem_ptr < HULL_EPSILON)
                   )
#endif
                {
#ifdef HOW_IT_WAS
                    if (IS_NEGATIVE_DBL(temp))
#else
                    if (temp < HULL_EPSILON)
#endif
                    {
                        dbw(); 
                        hull_min_z = DBL_MAX;  /* Force no solution */
                        break;
                    }
                    else
                    {
                        /*EMPTY*/
                        ;   /* Do nothing: This normal is irrelavent */
                    }
                }
                else if (*normal_elem_ptr > 0.0)
                {
                    z = temp / (*normal_elem_ptr);

                    if (z <= hull_max_z)
                    {
                        hull_max_z = z;

                        if (IS_NOT_LESSER_DBL(hull_min_z, hull_max_z))
                        {
                            break;  /* No solution */
                        }
                    }
                }
                else
                {
                    z = temp / (*normal_elem_ptr);

                    if (z >= hull_min_z)
                    {
                        hull_min_z = z;

                        if (IS_NOT_LESSER_DBL(hull_min_z, hull_max_z))
                        {
                            break;  /* No solution */
                        }
                    }
                }

                constraint_elem_ptr++;
                normal_row_ptr++;
                x_cache_ptr++;
            }

            /*
            // if (hull_min_z < hull_max_z)
            //
            // Use macro to properly deal with imprecise math.
            */
            if (IS_NOT_GREATER_DBL(hull_min_z, hull_max_z))
            {
                int beg_k = kjb_rint((hull_min_z - min_z) / z_step);
                int end_k = kjb_rint((hull_max_z - min_z) / z_step);

                if (beg_k < 0) beg_k = 0;
                if (end_k < 0) end_k = 0;

                if (beg_k == resolution) beg_k--;
                if (end_k == resolution) end_k--;

                for (k = beg_k; k<=end_k; k++)
                {
                    (hp->filled_3D_array)[ i ][ j ][ k ] = 1;
                }
            }
            y += y_step;
         }
         x += x_step;
    }

    free_vector(x_cache_vp);

#endif 

    if (result != ERROR)
    {

        hp->min_x = min_x;
        hp->min_y = min_y;
        hp->min_z = min_z;

        hp->max_x = max_x;
        hp->max_y = max_y;
        hp->max_z = max_z;

        hp->filled_scale_x = resolution / (max_x - min_x);
        hp->filled_scale_y = resolution / (max_y - min_y);
        hp->filled_scale_z = resolution / (max_z - min_z);

        hp->filled_resolution = resolution;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             is_hull_inside_hull
 *
 * Determines if one hull is inside another
 *
 * This predicate function determines if the hull pointed to by the second
 * argument is inside the hull pointed to by the first argument.
 *
 * Returns:
 *    TRUE if  is in the hull, FALSE if it is not, and ERROR on failure.
 *
 * Related:
 *    Hull
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/
int is_hull_inside_hull(const Hull* outer_hp, const Hull* inner_hp)
{
    Vector* test_vp = NULL;
    int     i;
    int     result = TRUE;
    int     point_is_inside;

    for (i=0; i<inner_hp->num_vertices; i++)
    {
        if (get_matrix_row(&test_vp, inner_hp->vertex_mp, i) == ERROR)
        {
            result = ERROR;
            break;
        }

        point_is_inside = is_point_inside_hull(outer_hp, test_vp);

        if (point_is_inside == ERROR)
        {
            result = ERROR;
            break;
        }
        else if (point_is_inside == FALSE)
        {
            result = FALSE;
            break;
        }
    }

    free_vector(test_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             is_point_inside_hull
 *
 * Determines if the input point is inside the hull.
 *
 * This predicate function determines if the input point lies within the
 * input convex hull.
 *
 * The first argument "hp" is the address of the convex hull
 *
 * The second argument "test_vp" contains the address of a the query location.
 *
 * Returns:
 *    TRUE if test_vp is in the hull, FALSE if it is not, and ERROR on failure.
 *
 * Related:
 *    Hull
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/
int is_point_inside_hull(const Hull* hp, const Vector* test_vp)
{
    Vector* constraint_vp;
    int     result        = TRUE;
    int     i, j;
    double  x, y, z;


    if (hp->dimension != test_vp->length)
    {
        set_bug("Test point not the same dimension as hull in is_point_inside_hull.");
        return ERROR;
    }

    if (hp->dimension < 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    x = test_vp->elements[ 0 ];
    if ((x < hp->min_x) || (x > hp->max_x)) return FALSE;

    y = test_vp->elements[ 1 ];
    if ((y < hp->min_y) || (y > hp->max_y)) return FALSE;

    if (hp->dimension > 2)
    {
        z = test_vp->elements[ 2 ];
        if ((z < hp->min_z) || (z > hp->max_z)) return FALSE;
    }

    if (KJB_IS_SET(hp->filled_resolution))
    {
        i = (int)((x - hp->min_x) * hp->filled_scale_x);
        j = (int)((y - hp->min_y) * hp->filled_scale_y);

        if (i == hp->filled_resolution) i--;
        if (j == hp->filled_resolution) j--;

        /*
        // FIX This tests for a bug: Once we don't need the code to run in spite
        // of this happening, the code should be changed to reflect that fact
        // that getting here is in fact a bug.
        */
        if (    (i < 0) || (i >= hp->filled_resolution)
             || (j < 0) || (j >= hp->filled_resolution)
           )
        {
            set_bug("Invalid index in is_point_inside_hull (filled hull code)");
            return ERROR;
        }

        if (test_vp->length == 2)
        {
            return (hp->filled_2D_array)[ i ][ j ];
        }
        else if (test_vp->length == 3)
        {
            int k;

            z = test_vp->elements[ 2 ];

            if ((z < hp->min_z) || (z > hp->max_z))
            {
                return FALSE;
            }

            k = (int)((z - hp->min_z) * hp->filled_scale_z);

            return (hp->filled_3D_array)[ i ][ j ][ k ];
        }
    }

    if (hp->normal_mp == NULL)
    {
        set_bug("Hulls sent to is_point_inside_hull must have HULL_NORMALS.");
        return ERROR;
    }

    if (hp->b_value_vp == NULL)
    {
        set_bug("Hulls sent to is_point_inside_hull must have HULL_B_VALUES.");
        return ERROR;
    }

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (multiply_matrix_and_vector(&fs_cached_n_dot_x_vp, hp->normal_mp, test_vp)
        ==ERROR)
    {
        return ERROR;
    }

    constraint_vp = hp->b_value_vp;

    for (i=0; i<constraint_vp->length; i++)
    {
        if (IS_GREATER_DBL(fs_cached_n_dot_x_vp->elements[i], constraint_vp->elements[i]))
        {
            result = FALSE;
            break;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_point_inside_hull_2
(
    const Hull*   hp,
    const Vector* test_vp,
    double        rel_tol,
    double        abs_tol,
    double*       error_ptr
)
{
    Vector* constraint_vp;
    int     result        = TRUE;
    int     i;
    double  x, y, z;


    if (hp->dimension != test_vp->length)
    {
        set_bug("Test point not the same dimension as hull in is_point_inside_hull.");
        return ERROR;
    }

    if (hp->dimension < 2)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    *error_ptr = DBL_NOT_SET;

    x = test_vp->elements[ 0 ];
    if (    (ADD_RELATIVE_DBL(x + abs_tol, rel_tol) < SUB_RELATIVE_DBL(hp->min_x - abs_tol, rel_tol))
         || (SUB_RELATIVE_DBL(x - abs_tol, rel_tol) > ADD_RELATIVE_DBL(hp->max_x + abs_tol, rel_tol))
       )
    {
        *error_ptr = - MIN_OF(x - hp->min_x, hp->max_x - x);
        return FALSE;
    }

    y = test_vp->elements[ 1 ];
    if (    (ADD_RELATIVE_DBL(y + abs_tol, rel_tol) < SUB_RELATIVE_DBL(hp->min_y - abs_tol, rel_tol))
         || (SUB_RELATIVE_DBL(y - abs_tol, rel_tol) > ADD_RELATIVE_DBL(hp->max_y + abs_tol, rel_tol))
         || (y < hp->min_y - abs_tol)
         || (y > hp->max_y + abs_tol)
        )
    {
        *error_ptr = - MIN_OF(y - hp->min_y, hp->max_y - y);
        return FALSE;
    }

    if (hp->dimension > 2)
    {
        z = test_vp->elements[ 2 ];
        if (   (ADD_RELATIVE_DBL(z + abs_tol, rel_tol) < SUB_RELATIVE_DBL(hp->min_z - abs_tol, rel_tol))
            || (SUB_RELATIVE_DBL(z - abs_tol, rel_tol) > ADD_RELATIVE_DBL(hp->max_z + abs_tol, rel_tol))
            || (z < hp->min_z - abs_tol)
            || (z > hp->max_z + abs_tol)
           )
        {
            *error_ptr = - MIN_OF(z - hp->min_z, hp->max_z - z);
            return FALSE;
        }
    }

    if (hp->normal_mp == NULL)
    {
        set_bug("Hulls sent to is_point_inside_hull must have HULL_NORMALS.");
        return ERROR;
    }

    if (hp->b_value_vp == NULL)
    {
        set_bug("Hulls sent to is_point_inside_hull must have HULL_B_VALUES.");
        return ERROR;
    }

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (multiply_matrix_and_vector(&fs_cached_n_dot_x_vp, hp->normal_mp, test_vp)
        ==ERROR)
    {
        return ERROR;
    }

    constraint_vp = hp->b_value_vp;

    for (i=0; i<constraint_vp->length; i++)
    {
        if ((SUB_RELATIVE_DBL(fs_cached_n_dot_x_vp->elements[i] - abs_tol, rel_tol)) > ADD_RELATIVE_DBL(constraint_vp->elements[i] + abs_tol, rel_tol))
        {
            result = FALSE;
            *error_ptr = fs_cached_n_dot_x_vp->elements[i] - constraint_vp->elements[i];
            break;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           find_hull_bounds
 *
 * Finds the bounds in each coord of a hull.
 *
 * This routine finds the bounds in each coord of a hull. Thus if the hull has
 * 2-dimensions, this routine can be used to find (min_x, min_y) and (max_x,
 * max_y). The first argument is a pointer to the hull. The second argument is a
 * pointer to a pointer to a vector for the min coords, and the third argument
 * is a pointer to a pointer to a vector for the max coords. If these vector
 * pointers are null (e.g, *min_vpp is NULL), then the vector is created. If
 * they are the wrong size, then they are resized, and if they are the right
 * size, then the storage is recycled. If either of these arguments are NULL
 * (e.g. min_vpp is NULL), then that request is ignored. Thus this routine
 * can be used to find either min's or max's without dummy arguments.
 *
 * Note:
 *    Hull bounds for 2 and 3 dimensional hulls are already available in the
 *    hull structure, so this routine is of use only when it is most convenient
 *    if the results are put into vectors, and/or the hull dimension is greater
 *    than 3.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure.
 *
 * Related:
 *    create_hull, find_convex_hull, Hull, Queue_element
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/

int find_hull_bounds(const Hull* hp, Vector** min_vpp, Vector** max_vpp)
{

    if (min_vpp != NULL)
    {
        ERE(get_min_matrix_col_elements(min_vpp, hp->vertex_mp));
    }

    if (max_vpp != NULL)
    {
        ERE(get_max_matrix_col_elements(max_vpp, hp->vertex_mp));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/   /* Usually have "sig" as "int" as first arg (always on UNIX) */
static TRAP_FN_RETURN_TYPE qhull_crash_fn(TRAP_FN_DUMMY_ARGS)
{
    allow_abort_sig();
    longjmp(fs_qhull_crash_env, TRUE);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
static void prepare_memory_cleanup(void)
{
    static int first_time = TRUE;


    if (first_time)
    {
        add_cleanup_function(free_allocated_static_data);
        first_time = FALSE;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
static void free_allocated_static_data(void)
{
    free_vector(fs_cached_n_dot_x_vp);
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

