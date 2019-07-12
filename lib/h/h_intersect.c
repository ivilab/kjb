
/* $Id: h_intersect.c 21545 2017-07-23 21:57:31Z kobus $ */

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
#include "h/h_intersect.h"

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
#define REALLY_TEST_INTERSECTION_ROUTINES
*/

#ifdef REALLY_TEST_INTERSECTION_ROUTINES
#    include "p/p_plot.h"
#    define TEST_INTERSECTION_ROUTINES
#endif

#define TEST_INTERSECTION_ROUTINES

#define NUM_POINTS_FOR_INTERIOR_POINT_AVERAGE 5
#define MIN_NUM_2D_HULL_POINTS                3
#define MIN_NUM_3D_HULL_POINTS                4

/* This one can be large--it is just used to increase performance. */
#define PLANE_CULL_EPSILON    (0.01)

#define HULL_POSITIVITY_ERROR_THRESHOLD   ((0.2))
#define HULL_POSITIVITY_WARNING_THRESHOLD ((1000000.0 * DBL_EPSILON))
#define HULL_POSITIVITY_COMMENT_THRESHOLD ((1000.0 * DBL_EPSILON))

/* -------------------------------------------------------------------------- */

static Method_option fs_hull_intersection_methods[ ] =
{
    { "original",      "o",  (int(*)())original_intersect_hulls },
    { "approximation", "a",  (int(*)())approximation_intersect_hulls},
    { "dual-space",    "d",  (int(*)())dual_space_intersect_hulls}
};

static const int fs_num_hull_intersection_methods =
                               sizeof(fs_hull_intersection_methods) /
                                        sizeof(fs_hull_intersection_methods[ 0 ]);
static int         fs_hull_intersection_method                  = 2;
static const char* fs_hull_intersection_method_option_short_str = "him";
static const char* fs_hull_intersection_method_option_long_str  =
                                                    "hull-intersection-method";

static int fs_hull_intersection_resolution = 100;

/* -------------------------------------------------------------------------- */

static int original_2D_intersect_hulls
(
    Queue_element* hull_list_head,
    int            resolution,
    unsigned long  options,
    Hull**         result_hp_ptr
);

static int original_3D_intersect_hulls
(
    Queue_element* hull_list_head,
    int            resolution,
    unsigned long  options,
    Hull**         result_hp_ptr
);

static int approximation_2D_intersect_hulls
(
    Queue_element* hull_list_head,
    int            resolution,
    unsigned long  options,
    Hull**         result_hp_ptr
);

static int approximation_3D_intersect_hulls
(
    Queue_element* hull_list_head,
    int            resolution,
    unsigned long  options,
    Hull**         result_hp_ptr
);

static int is_segment_in_2D_hull_intersection
(
    double  x,
    double  min_y,
    double  max_y,
    Matrix* normal_mp,
    Vector* constraint_vp,
    double* y_ptr,
    double* distance_ptr
);

static int is_segment_in_3D_hull_intersection
(
    double  x,
    double  y,
    double  min_z,
    double  max_z,
    Matrix* normal_mp,
    Vector* constraint_vp,
    double* z_ptr,
    double* distance_ptr
);

/* -------------------------------------------------------------------------- */

int set_hull_intersect_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_int_value;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hir")
          || match_pattern(lc_option, "hull-intersection-resolution")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Hull intersection resolution is %d.\n",
                    fs_hull_intersection_resolution));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hull-intersection-resolution = %d\n",
                    fs_hull_intersection_resolution));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_hull_intersection_resolution = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, fs_hull_intersection_method_option_short_str)
          || match_pattern(lc_option, fs_hull_intersection_method_option_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(fs_hull_intersection_methods,
                                fs_num_hull_intersection_methods,
                                fs_hull_intersection_method_option_long_str,
                                "hull intersection method", value,
                                &fs_hull_intersection_method));
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        intersect_positive_hulls
 *
 * Finds the intersection of positive convex hulls.
 *
 * This routine finds the intersection of a list of positive convex hulls and
 * returns the result as a convex hull.
 *
 * This routine is essentially a wrapper for intersect_hulls(). It checks that
 * the hulls to be intersected are all positive, uses intersect_hulls() to
 * intersect them, and then checks that the result is positive. If either an
 * input hull, or the output hull is almost positive (the magnitude of negative
 * values are less than a threshold), then this routine silently truncates the
 * data at zero. As the degree of deviation from positive increases, the routine
 * outputs messages of increasing verbose level. Finally, if the degree of
 * deviation from positive is too large, then the intersection fails and ERROR
 * is returned.
 *
 * Warning:
 *    If an input hull has a negative coordinate, then this hull will be
 *    destructively changed by this routine.
 *
 * Returns:
 *    NO_ERROR on success, with the address of the result in "result_hp_ptr",
 *    NO_SOLUTION if the intersection in empty, and ERROR if there are more
 *    serious problems.
 *
 * Related:
 *    intersect_hulls().
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/
int intersect_positive_hulls
(
    Queue_element* hull_list_head,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{
    Queue_element* cur_elem;
    double         min;
    int            result;


    if (hull_list_head == NULL)
    {
        set_bug("Null hull list passed to intersect_hulls.");
        return ERROR;
    }

    if (result_hp_ptr == NULL)
    {
        set_bug("Null result_hp_ptr passed to intersect_hulls.");
        return ERROR;
    }

    cur_elem = hull_list_head;

    while (cur_elem != NULL)
    {
        Matrix* cur_vertex_mp = ((Hull*)(cur_elem->contents))->vertex_mp;

        if (cur_vertex_mp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_VERTICES.");
            return ERROR;
        }

        min = min_matrix_element(cur_vertex_mp);

        if (min < -HULL_POSITIVITY_ERROR_THRESHOLD)
        {
            set_error("Hull to be intersected has large negative componant %e.",
                      min);
            return ERROR;
        }
        else if (min < -HULL_POSITIVITY_WARNING_THRESHOLD)
        {
            warn_pso("Hull to be intersected has small negative componant (%e)\n",
                     min);
            warn_pso(" Hull is truncated at zero.\n");
        }
        else if (min < -HULL_POSITIVITY_COMMENT_THRESHOLD)
        {
            verbose_pso(15,
                 "Hull to be intersected has small negative componant (%e).\n",
                 min);
            verbose_pso(15, "Hull is truncated at zero.\n");
        }

        if (min < 0.0)
        {
            ERE(ow_min_thresh_matrix(cur_vertex_mp, 0.0));
        }

        cur_elem = cur_elem->next;
    }

    if (fs_hull_intersection_method == NOT_SET)
    {
        set_error("No hull_intersection method is currently set.");
        return ERROR;
    }
    else if (    (fs_hull_intersection_method < 0)
              || (fs_hull_intersection_method >= fs_num_hull_intersection_methods)
            )
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }
    else
    {
        int (*fn) (Queue_element*, unsigned long, Hull**)
            = (int (*)(Queue_element*, unsigned long, Hull**)) fs_hull_intersection_methods[ fs_hull_intersection_method ].fn;

        result = fn(hull_list_head, options, result_hp_ptr);
    }

    if (result != NO_ERROR) return result;

    min = min_matrix_element((*result_hp_ptr)->vertex_mp);

    if (min < -HULL_POSITIVITY_ERROR_THRESHOLD)
    {
        set_error("Hull intersection result has large negative componant %e.",
                  min);
        free_hull(*result_hp_ptr);
        *result_hp_ptr = NULL;
        return ERROR;
    }
    else if (min < -HULL_POSITIVITY_WARNING_THRESHOLD)
    {
        pso("Warning: Hull intersection result has small negative componant (%e).\n",
            min);
        pso("         Hull is truncated at zero.\n");
    }
    else if (min < -HULL_POSITIVITY_COMMENT_THRESHOLD)
    {
        verbose_pso(1,
                "Hull intersection result has small negative componant (%e).\n",
                min);
        verbose_pso(1, "Hull is truncated at zero.\n");
    }

    if (min < 0.0)
    {
        Matrix* new_vertex_mp = NULL;

        if (copy_matrix(&new_vertex_mp, (*result_hp_ptr)->vertex_mp) == ERROR)
        {
            free_hull(*result_hp_ptr);
            *result_hp_ptr = NULL;
            return ERROR;
        }

        free_hull(*result_hp_ptr);

        if (ow_min_thresh_matrix(new_vertex_mp, 0.0) == ERROR)
        {
            free_matrix(new_vertex_mp);
            *result_hp_ptr = NULL;
            return ERROR;
        }

        *result_hp_ptr = find_convex_hull(new_vertex_mp, options);

        free_matrix(new_vertex_mp);

        if (*result_hp_ptr == NULL) return ERROR;
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           intersect_hulls
 *
 * Finds the intersection of convex hulls.
 *
 * This routine finds the intersection of a list of convex hulls and returns
 * the result as a convex hull.
 *
 * The first argument "hull_list_head" is the address of the queue containing
 * the list of convex hulls to process. Only lists of 2-D or 3-D hulls are
 * currently supported.
 *
 * The second argument "options" contains the flags controlling the data
 * returned in the output hull structure. For most circumstances, the value of
 * DEFAULT_HULL_OPTIONS will suffice. See the man page for "find_convex_hull"
 * for a description of the option flags.
 *
 * The method of hull intersection is under user control via the option
 * hull-intersection-method. One method "a" is a discrete approximationl. A
 * faster, more accurate method is the dual space method "d". In the case of the
 * discrete approximation, the resolution of the descrete space is user settable
 * with the "hir" option, provided that the KJB library options are made
 * available to the user (recommended!). The dual space method also makes use of
 * the resolution when finding an intial point inside the hull. The dual space
 * method normally returns an answer independent of the resolution, but the
 * resolution does set the smallest amount that hulls can intersect, and still
 * be recognized as intersecting (with all methods).
 *
 * Returns:
 *    NO_ERROR on success, with the address of the result in "result_hp_ptr",
 *    NO_SOLUTION if the intersection in empty, and ERROR if there are more
 *    serious problems.
 *
 * Related:
 *    create_hull, find_convex_hull, Hull, Queue_element
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/
int intersect_hulls
(
    Queue_element* hull_list_head,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{

    if (hull_list_head == NULL)
    {
        set_bug("Null hull list passed to intersect_hulls.");
        return ERROR;
    }

    if (result_hp_ptr == NULL)
    {
        set_bug("Null result_hp_ptr passed to intersect_hulls.");
        return ERROR;
    }

    if (fs_hull_intersection_method == NOT_SET)
    {
        set_error("No hull_intersection method is currently set.");
        return ERROR;
    }
    else if (    (fs_hull_intersection_method < 0)
              || (fs_hull_intersection_method >= fs_num_hull_intersection_methods)
            )
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }
    else
    {
        int (*fn)(Queue_element*, unsigned long, Hull**)
            /* Kobus: 05/01/22: Be very pedantic here, to keep C++ happy.  */
            = (int (*)(Queue_element*, unsigned long, Hull**))fs_hull_intersection_methods[ fs_hull_intersection_method ].fn;

        return fn(hull_list_head, options, result_hp_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int original_intersect_hulls
(
    Queue_element* hull_list_head,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{
    Queue_element* first_elem;
    int            dim;


    if (hull_list_head == NULL)
    {
        set_bug("Null hull list passed to original_intersect_hulls.");
        return ERROR;
    }

    if (result_hp_ptr == NULL)
    {
        set_bug("Null result_hp_ptr passed to original_intersect_hulls.");
        return ERROR;
    }

    first_elem = hull_list_head;
    dim = ((Hull *)(first_elem->contents))->dimension;

    if (dim == 2)
    {
        return original_2D_intersect_hulls(hull_list_head,
                                           fs_hull_intersection_resolution,
                                           options, result_hp_ptr);
    }
    else if (dim == 3)
    {
        return original_3D_intersect_hulls(hull_list_head,
                                           fs_hull_intersection_resolution,
                                           options, result_hp_ptr);
    }
    else
    {
        set_bug("Intersection of %d-D hulls not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 * STATIC                    original_2D_intersect_hulls
 * -----------------------------------------------------------------------------
*/
static int original_2D_intersect_hulls
(
    Queue_element* hull_list_head,
    int            resolution,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{
     Queue_element* cur_point_elem;
     Queue_element* min_point_elem;
     Queue_element* cur_normal_elem;
     Hull*          cur_hp;
     Matrix**       cur_facet_mp_list;
     Matrix*        cur_normal_mp;
     double           min_dim_product;
     double           x_range;
     double           y_range;
     double           dim_product;
     Matrix*        temp_normal_mp;
     Matrix*        normal_mp;
     Vector*        temp_constraint_vp;
     Vector*        constraint_vp;
     double**         temp_normal_row_ptr;
     double**         normal_row_ptr;
     double*          temp_normal_elem_ptr;
     double*          normal_elem_ptr;
     double*          center_elem_ptr;
     double*          temp_constraint_elem_ptr;
     double*          constraint_elem_ptr;
     int            max_num_normals;
     int            num_normals;
     int            num_result_points;
     int            i, j, k, num_rows;
     double           min_x;
     double           max_x;
     double           min_y;
     double           max_y;
     double           x, y;
     double           x_step;
     double           b;
     Matrix*        result_mp;
     double**         result_row_ptr;
     double           hull_min_y;
     double           hull_max_y;
     double           temp;
     int            result;
     double           new_min_x;
     double           new_max_x;
     double           new_min_y;
     double           new_max_y;


     cur_point_elem = hull_list_head;

     min_dim_product = DBL_MAX;

     /*
        Keep error checkers happy.
     */
     min_x = min_y = 0.0;
     max_x = max_y = 0.0;
     min_point_elem = NULL;


     while (cur_point_elem != NULL)
     {
         cur_hp = (Hull *)(cur_point_elem->contents);

         x_range = cur_hp->max_x - cur_hp->min_x;
         y_range = cur_hp->max_y - cur_hp->min_y;

         dim_product = x_range * y_range;

         if (dim_product < min_dim_product)
         {
             min_dim_product = dim_product;

             min_x = cur_hp->min_x;
             max_x = cur_hp->max_x;

             min_y = cur_hp->min_y;
             max_y = cur_hp->max_y;

             min_point_elem = cur_point_elem;
         }

         cur_point_elem = cur_point_elem->next;
     }

     max_num_normals = 0;
     cur_normal_elem = hull_list_head;

     while (cur_normal_elem != NULL)
     {
         cur_hp = (Hull *)(cur_normal_elem->contents);
         cur_normal_mp = cur_hp->normal_mp;

         max_num_normals += cur_normal_mp->num_rows;

         cur_normal_elem = cur_normal_elem->next;
     }

     NRE(temp_normal_mp = create_matrix(max_num_normals, 2));
     NRE(temp_constraint_vp = create_vector(max_num_normals));

     temp_normal_row_ptr = temp_normal_mp->elements;
     temp_constraint_elem_ptr = temp_constraint_vp->elements;

     /*
        Put smallest hull first.
     */
     cur_normal_elem = min_point_elem;

     cur_hp = (Hull *)(cur_normal_elem->contents);
     cur_normal_mp = cur_hp->normal_mp;
     cur_facet_mp_list = cur_hp->facets->elements;

     num_rows = cur_normal_mp->num_rows;

     normal_row_ptr = cur_normal_mp->elements;

     for (i=0; i<num_rows; i++)
     {
         temp_normal_elem_ptr = *temp_normal_row_ptr;
         normal_elem_ptr = *normal_row_ptr;
         center_elem_ptr = (*cur_facet_mp_list)->elements[0];
         b = 0.0;

         for (j=0; j<2; j++)
         {
             *temp_normal_elem_ptr = *normal_elem_ptr;
             b += (*normal_elem_ptr) * (*center_elem_ptr);

             temp_normal_elem_ptr++;
             normal_elem_ptr++;
             center_elem_ptr++;
         }

         *temp_constraint_elem_ptr = ADD_DBL_EPSILON(b);

         temp_constraint_elem_ptr++;
         temp_normal_row_ptr++;
         normal_row_ptr++;
         cur_facet_mp_list++;
     }

     cur_normal_elem = hull_list_head;

     while (cur_normal_elem != NULL)
     {
         if (cur_normal_elem != min_point_elem)
         {
             cur_hp = (Hull *)(cur_normal_elem->contents);
             cur_normal_mp = cur_hp->normal_mp;
             cur_facet_mp_list = cur_hp->facets->elements;

             num_rows = cur_normal_mp->num_rows;

             normal_row_ptr = cur_normal_mp->elements;

             for (i=0; i<num_rows; i++)
             {
                 temp_normal_elem_ptr = *temp_normal_row_ptr;
                 normal_elem_ptr = *normal_row_ptr;
                 center_elem_ptr = (*cur_facet_mp_list)->elements[0];
                 b = 0.0;

                 for (j=0; j<2; j++)
                 {
                     *temp_normal_elem_ptr = *normal_elem_ptr;

                     b += (*normal_elem_ptr) * (*center_elem_ptr);

                     temp_normal_elem_ptr++;
                     normal_elem_ptr++;
                     center_elem_ptr++;
                 }

                 *temp_constraint_elem_ptr = ADD_DBL_EPSILON(b);

                 temp_constraint_elem_ptr++;
                 temp_normal_row_ptr++;
                 normal_row_ptr++;
                 cur_facet_mp_list++;
             }
         }

         cur_normal_elem = cur_normal_elem->next;
     }

     /*
     //  Thin normals?. Not needed yet.
     */
     num_normals = max_num_normals;
     normal_mp = temp_normal_mp;
     constraint_vp = temp_constraint_vp;

     x_step = (max_x - min_x) / resolution;

     x = min_x;

     NRE(result_mp = create_matrix ( 2 * (resolution + 1), 2));
     result_row_ptr = result_mp->elements;

     num_result_points = 0;

     /*
     // Initialize min's to max's and vice versa.
     */
     new_min_x = max_x;
     new_min_y = max_y;
     new_max_x = min_x;
     new_max_y = min_y;

     for (i=0; i<resolution+1; i++)
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
                     ;   /* Do nothing: This normal is irrelavent */
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
             if (x > new_max_x)
             {
                  new_max_x = x;
              }
             if (x < new_min_x)
             {
                  new_min_x = x;
              }
             if (hull_max_y > new_max_y)
             {
                  new_max_y = hull_max_y;
              }
             if (hull_min_y < new_min_y)
             {
                  new_min_y = hull_min_y;
              }

             (*result_row_ptr)[0] = x;
             (*result_row_ptr)[1] = hull_min_y;
             result_row_ptr++;
             num_result_points++;

             (*result_row_ptr)[0] = x;
             (*result_row_ptr)[1] = hull_max_y;
             result_row_ptr++;
             num_result_points++;
         }

          x += x_step;
      }

     x_range = new_max_x - new_min_x;
     y_range = new_max_y - new_min_y;
     dim_product = x_range * y_range;

     if ((num_result_points > 0) &&
         (dim_product < (min_dim_product / 2.0)))
     {
         /*
         // Looks wrong to me. However, it seems that it just ends up
         // recomputing the same result instead of refining the answer, so I
         // will leave it as is to keep the original code original.
         */
         max_x = MAX_OF(max_x, (new_max_x + x_step));
         min_x = MIN_OF(min_x, (new_min_x - x_step));

#ifdef TEST_INTERSECTION_ROUTINES
         pso("Doing re-approximation in original.\n");
#endif

         x_step = (max_x - min_x) / resolution;

         result_row_ptr = result_mp->elements;

         num_result_points = 0;

         x = min_x;

         for (i=0; i<resolution+1; i++)
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
                         hull_min_y = DBL_MAX; /* Force no solution */
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
                 (*result_row_ptr)[0] = x;
                 (*result_row_ptr)[1] = hull_min_y;
                 result_row_ptr++;
                 num_result_points++;

                 (*result_row_ptr)[0] = x;
                 (*result_row_ptr)[1] = hull_max_y;
                 result_row_ptr++;
                 num_result_points++;
             }

              x += x_step;
          }
     }

     result_mp->num_rows = num_result_points;

     /*
     // This will break if resolution is too low (less than 7!). Also, the
     // "+= 4" should be "*= 5", but this is not too big of a deal, so we will
     // leave it to keep the "original" code, original.
     */
     if ((num_result_points < 4) && (num_result_points > 0))
     {
         double y_step, x_offset, y_offset;


         y_step = (max_y - min_y) / resolution;

         for (i=0; i<num_result_points; i++)
         {
             x = (result_mp->elements)[i][0];
             y = (result_mp->elements)[i][1];

             for (j=0; j<2; j++)
             {
                 for (k=0; k<2; k++)
                 {
                     if (IS_EVEN(j))
                     {
                         x_offset = x_step/2.0;
                     }
                     else
                     {
                         x_offset = -x_step/2.0;
                     }
                     if (IS_EVEN(k))
                     {
                         y_offset = y_step/2.0;
                     }
                     else
                     {
                         y_offset = -y_step/2.0;
                     }

                     (*result_row_ptr)[0] = x + x_offset;
                     (*result_row_ptr)[1] = y + y_offset;

                     result_row_ptr++;
                 }
             }
         }
         num_result_points += 4;
         result_mp->num_rows = num_result_points;
     }

     if (num_result_points == 0)
     {
         set_error("Hull intersection is empty.");
         free_hull(*result_hp_ptr);
         *result_hp_ptr = NULL;
         result = NO_SOLUTION;
     }
     else
     {
         free_hull(*result_hp_ptr);
         *result_hp_ptr = find_convex_hull(result_mp, options);

         if (*result_hp_ptr == NULL) result = ERROR;
         else                        result = NO_ERROR;
     }

    /* If we trim normals, then also free normal_mp, and constraint_vp */

    free_matrix(temp_normal_mp);
    free_vector(temp_constraint_vp);
    free_matrix(result_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                    original_3D_intersect_hulls
 * -----------------------------------------------------------------------------
*/
static int original_3D_intersect_hulls
(
    Queue_element* hull_list_head,
    int            resolution,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Queue_element* cur_point_elem;
    Queue_element* min_point_elem;
    Queue_element* cur_normal_elem;
    Hull*          cur_hp;
    Matrix**       cur_facet_mp_list;
    Matrix*        cur_normal_mp;
    double           min_dim_product;
    double           x_range;
    double           y_range;
    double           z_range;
    double           dim_product;
    Matrix*        temp_normal_mp;
    Matrix*        normal_mp;
    Vector*        temp_constraint_vp;
    Vector*        constraint_vp;
    double**         temp_normal_row_ptr;
    double**         normal_row_ptr;
    double*          temp_normal_elem_ptr;
    double*          normal_elem_ptr;
    double*          center_elem_ptr;
    double*          temp_constraint_elem_ptr;
    double*          constraint_elem_ptr;
    int            max_num_normals;
    int            num_normals;
    int            num_result_points;
    int     i,j,k,m, num_rows;
    double    min_x, max_x, min_y, max_y, min_z, max_z, x, y, z;
    double    x_step, y_step, b;
    Matrix* result_mp;
    double**  result_row_ptr;
    Vector* x_cache_vp;
    double*   x_cache_ptr;
    double    hull_min_z, hull_max_z, temp;
    double    new_min_x, new_max_x, new_min_y, new_max_y, new_min_z, new_max_z;
    int     result = NO_ERROR;


    cur_point_elem = hull_list_head;

    min_dim_product = DBL_MAX;

    /*
       Keep error checkers happy.
    */
    min_x = min_y = min_z = 0.0;
    max_x = max_y = max_z = 0.0;
    min_point_elem = NULL;

    while (cur_point_elem != NULL)
    {
        cur_hp = (Hull *)(cur_point_elem->contents);

        x_range = cur_hp->max_x - cur_hp->min_x;
        y_range = cur_hp->max_y - cur_hp->min_y;
        z_range = cur_hp->max_z - cur_hp->min_z;

        dim_product = x_range * y_range * z_range;

        if (dim_product < min_dim_product)
        {
            min_dim_product = dim_product;

            min_x = cur_hp->min_x;
            max_x = cur_hp->max_x;

            min_y = cur_hp->min_y;
            max_y = cur_hp->max_y;

            min_z = cur_hp->min_z;
            max_z = cur_hp->max_z;

            min_point_elem = cur_point_elem;
        }

        cur_point_elem = cur_point_elem->next;
    }

    max_num_normals = 0;
    cur_normal_elem = hull_list_head;

    while (cur_normal_elem != NULL)
    {
        cur_hp = (Hull *)(cur_normal_elem->contents);
        cur_normal_mp = cur_hp->normal_mp;

        max_num_normals += cur_normal_mp->num_rows;

        cur_normal_elem = cur_normal_elem->next;
    }

    NRE(temp_normal_mp = create_matrix(max_num_normals, 3));
    NRE(temp_constraint_vp = create_vector(max_num_normals));

    temp_normal_row_ptr = temp_normal_mp->elements;
    temp_constraint_elem_ptr = temp_constraint_vp->elements;

    /*
    // Put smallest hull first.
    */
    cur_normal_elem = min_point_elem;

    cur_hp = (Hull *)(cur_normal_elem->contents);
    cur_normal_mp = cur_hp->normal_mp;
    cur_facet_mp_list = cur_hp->facets->elements;

    num_rows = cur_normal_mp->num_rows;

    normal_row_ptr = cur_normal_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        temp_normal_elem_ptr = *temp_normal_row_ptr;
        normal_elem_ptr = *normal_row_ptr;
        center_elem_ptr = (*cur_facet_mp_list)->elements[0];
        b = 0.0;

        for (j=0; j<3; j++)
        {
            *temp_normal_elem_ptr = *normal_elem_ptr;
            b += (*normal_elem_ptr) * (*center_elem_ptr);

            temp_normal_elem_ptr++;
            normal_elem_ptr++;
            center_elem_ptr++;
        }

        *temp_constraint_elem_ptr = ADD_DBL_EPSILON(b);

        temp_constraint_elem_ptr++;
        temp_normal_row_ptr++;
        normal_row_ptr++;
        cur_facet_mp_list++;
    }

    cur_normal_elem = hull_list_head;

    while (cur_normal_elem != NULL)
    {
        if (cur_normal_elem != min_point_elem) {    /* Already done ! */
            cur_hp = (Hull *)(cur_normal_elem->contents);
            cur_normal_mp = cur_hp->normal_mp;
            cur_facet_mp_list = cur_hp->facets->elements;

            num_rows = cur_normal_mp->num_rows;

            normal_row_ptr = cur_normal_mp->elements;

            for (i=0; i<num_rows; i++)
            {
                temp_normal_elem_ptr = *temp_normal_row_ptr;
                normal_elem_ptr = *normal_row_ptr;
                center_elem_ptr = (*cur_facet_mp_list)->elements[0];
                b = 0.0;

                for (j=0; j<3; j++)
                {
                    *temp_normal_elem_ptr = *normal_elem_ptr;
                    b += (*normal_elem_ptr) * (*center_elem_ptr);

                    temp_normal_elem_ptr++;
                    normal_elem_ptr++;
                    center_elem_ptr++;
                }

                *temp_constraint_elem_ptr = ADD_DBL_EPSILON(b);

                temp_constraint_elem_ptr++;
                temp_normal_row_ptr++;
                normal_row_ptr++;
                cur_facet_mp_list++;
            }
        }

        cur_normal_elem = cur_normal_elem->next;
    }

    /*
    //  Thin normals?. Not needed yet.
    */
    num_normals = max_num_normals;
    normal_mp = temp_normal_mp;
    constraint_vp = temp_constraint_vp;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;

    x = min_x;

    NRE(x_cache_vp = create_vector(num_normals));
    NRE(result_mp = create_matrix( 2 * (resolution + 1)*(resolution + 1) , 3));
    result_row_ptr = result_mp->elements;

    num_result_points = 0;

    /*
    // Initialize min's to max's and vice versa.
    */
    new_min_x = max_x;
    new_min_y = max_y;
    new_min_z = max_z;
    new_max_x = min_x;
    new_max_y = min_y;
    new_max_z = min_z;

    for (i=0; i<resolution+1; i++)
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

        for (j=0; j<resolution+1; j++)
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

                    if (z < hull_max_z)
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

                    if (z > hull_min_z)
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

            if (IS_NOT_GREATER_DBL(hull_min_z, hull_max_z))
            {
                if (x > new_max_x)
                {
                    new_max_x = x;
                }
                if (x < new_min_x)
                {
                    new_min_x = x;
                }
                if (y > new_max_y)
                {
                    new_max_y = y;
                }
                if (y < new_min_y)
                {
                    new_min_y = y;
                }
                if (hull_max_z > new_max_z)
                {
                    new_max_z = hull_max_z;
                }
                if (hull_min_z < new_min_z)
                {
                    new_min_z = hull_min_z;
                }

                (*result_row_ptr)[0] = x;
                (*result_row_ptr)[1] = y;
                (*result_row_ptr)[2] = hull_min_z;
                result_row_ptr++;
                num_result_points++;

                (*result_row_ptr)[0] = x;
                (*result_row_ptr)[1] = y;
                (*result_row_ptr)[2] = hull_max_z;
                result_row_ptr++;
                num_result_points++;
            }

            y += y_step;
        }
        x += x_step;
    }

    if (result != ERROR)
    {
        x_range = new_max_x - new_min_x;
        y_range = new_max_y - new_min_y;
        z_range = new_max_z - new_min_z;
        dim_product = x_range * y_range * z_range;

        if (    (num_result_points > 0)
             && (dim_product < (min_dim_product / 3.0))
           )
        {
            /*
            // Looks wrong to me. However, it seems that it just ends up
            // recomputing the same result instead of refining the answer, so I
            // will leave it as is to keep the original code original.
            */
            max_x = MAX_OF(max_x, (new_max_x + x_step));
            min_x = MIN_OF(min_x, (new_min_x - x_step));
            max_y = MAX_OF(max_y, (new_max_y + y_step));
            min_y = MIN_OF(min_y, (new_min_y - y_step));

            x_step = (max_x - min_x) / resolution;
            y_step = (max_y - min_y) / resolution;

            result_row_ptr = result_mp->elements;

            num_result_points = 0;

            x = min_x;

            for (i=0; i<resolution+1; i++)
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

                for (j=0; j<resolution+1; j++)
                {
                    if (io_atn_flag)
                    {
                        set_error("Processing interrupted.");
                        halt_all_output = FALSE;
                        result = ERROR;
                        break;
                    }

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

                            if (z < hull_max_z)
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

                            if (z > hull_min_z)
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

                    if (IS_NOT_GREATER_DBL(hull_min_z, hull_max_z))
                    {
                        (*result_row_ptr)[0] = x;
                        (*result_row_ptr)[1] = y;
                        (*result_row_ptr)[2] = hull_min_z;
                        result_row_ptr++;
                        num_result_points++;

                        (*result_row_ptr)[0] = x;
                        (*result_row_ptr)[1] = y;
                        (*result_row_ptr)[2] = hull_max_z;
                        result_row_ptr++;
                        num_result_points++;
                    }

                    y += y_step;
                }

                if (result == ERROR) break;

                x += x_step;
            }
        }
    }

    if (result != ERROR)
    {
        result_mp->num_rows = num_result_points;

        /*
        // This will break if resolution is too low (less than 27!). Also, the
        // "+= 8" should be "*= 9", but this is not too big of a deal, so we
        // will leave it to keep the "original" code, original.
        */
        if ((num_result_points < 4) && (num_result_points > 0))
        {
            double z_step, x_offset, y_offset, z_offset;


            z_step = (max_z - min_z) / resolution;

            for (i=0; i<num_result_points; i++)
            {
                x = (result_mp->elements)[i][0];
                y = (result_mp->elements)[i][1];
                z = (result_mp->elements)[i][2];

                for (j=0; j<2; j++)
                {
                    for (k=0; k<2; k++)
                    {
                        for (m=0; m<2; m++)
                        {
                            if (IS_EVEN(j))
                            {
                                x_offset = x_step/2.0;
                            }
                            else
                            {
                                x_offset = -x_step/2.0;
                            }
                            if (IS_EVEN(k))
                            {
                                y_offset = y_step/2.0;
                            }
                            else
                            {
                                y_offset = -y_step/2.0;
                            }
                            if (IS_EVEN(m))
                            {
                                z_offset = z_step/2.0;
                            }
                            else
                            {
                                z_offset = -z_step/2.0;
                            }

                            (*result_row_ptr)[0] = x + x_offset;
                            (*result_row_ptr)[1] = y + y_offset;
                            (*result_row_ptr)[2] = z + z_offset;

                            result_row_ptr++;
                        }
                    }
                }
            }
            num_result_points += 8;
            result_mp->num_rows = num_result_points;
        }
    }

    if (result == ERROR)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (num_result_points == 0)
    {
        set_error("Hull intersection is empty.");
        free_hull(*result_hp_ptr);
        *result_hp_ptr = NULL;
        result = NO_SOLUTION;
    }
    else
    {
        free_hull(*result_hp_ptr);
        *result_hp_ptr = find_convex_hull(result_mp, options);

        if (*result_hp_ptr == NULL)
        {
            add_error("Intersection of 3D hulls failed.");
            result = ERROR;
        }
        else
        {
            result = NO_ERROR;
        }
    }

    free_vector(x_cache_vp);
    free_matrix(temp_normal_mp);
    free_vector(temp_constraint_vp);
    free_matrix(result_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int approximation_intersect_hulls
(
    Queue_element* hull_list_head,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{
    Queue_element* first_elem;
    int            dim;


    if (hull_list_head == NULL)
    {
        set_bug("Null hull list passed to approximation_intersect_hulls.");
        return ERROR;
    }

    if (result_hp_ptr == NULL)
    {
        set_bug("Null result_hp_ptr passed to approximation_intersect_hulls.");
        return ERROR;
    }

    first_elem = hull_list_head;
    dim = ((Hull *)(first_elem->contents))->dimension;

    if (dim == 2)
    {
        return approximation_2D_intersect_hulls(hull_list_head,
                                                fs_hull_intersection_resolution,
                                                options, result_hp_ptr);
    }
    else if (dim == 3)
    {
        return approximation_3D_intersect_hulls(hull_list_head,
                                                fs_hull_intersection_resolution,
                                                options, result_hp_ptr);
    }
    else
    {
        set_bug("Intersection of %d-D hulls not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                    approximation_2D_intersect_hulls
 * -----------------------------------------------------------------------------
*/
static int approximation_2D_intersect_hulls
(
    Queue_element* hull_list_head,
    int            resolution,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{
    Queue_element* cur_point_elem;
    Queue_element* cur_normal_elem;
    double           min_dim_product;
    double           x_range;
    double           y_range;
    double           dim_product;
    Matrix*        normal_mp;
    Vector*        constraint_vp;
    double**         normal_row_ptr;
    double*          normal_elem_ptr;
    double*          constraint_elem_ptr;
    int num_normals;
    int num_result_points;
    int i,j, k;
    double min_x, max_x, min_y, max_y, x, y, x_step;
    Matrix* result_mp;
    double**  result_row_ptr;
    double hull_min_y, hull_max_y, temp;
    int result;
    double new_min_x, new_max_x, new_min_y, new_max_y;


    if (resolution < 1)
    {
        set_error("Cannot find 2D hull intersection using approximation.");
        add_error("Resolution is %d. It must be at least 1.\n", resolution);
        return ERROR;
    }

    cur_point_elem = hull_list_head;

    min_x = min_y = -DBL_MAX;
    max_x = max_y = DBL_MAX;

    while (cur_point_elem != NULL)
    {
        Hull* cur_hp = (Hull*)(cur_point_elem->contents);

        /*
        // The inequalities may seem unatural, because we are doing a max of
        // mins, and a min of maxs.
        */
        if (cur_hp->min_x > min_x) min_x = cur_hp->min_x;
        if (cur_hp->min_y > min_y) min_y = cur_hp->min_y;

        if (cur_hp->max_x < max_x) max_x = cur_hp->max_x;
        if (cur_hp->max_y < max_y) max_y = cur_hp->max_y;

        cur_point_elem = cur_point_elem->next;
    }

    if ((min_x > max_x) || (min_y > max_y))
    {
        return NO_SOLUTION;
    }

    x_range = max_x - min_x;
    y_range = max_y - min_y;
    min_dim_product = x_range * y_range;

    num_normals = 0;
    cur_normal_elem = hull_list_head;

    while (cur_normal_elem != NULL)
    {
        Hull*   cur_hp        = (Hull *)(cur_normal_elem->contents);
        Matrix* cur_normal_mp = cur_hp->normal_mp;

        if (cur_normal_mp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_NORMALS.");
            return ERROR;
        }
        num_normals += cur_normal_mp->num_rows;

        cur_normal_elem = cur_normal_elem->next;
    }

    NRE(normal_mp = create_matrix(num_normals, 2));
    NRE(constraint_vp = create_vector(num_normals));

    normal_row_ptr = normal_mp->elements;
    constraint_elem_ptr = constraint_vp->elements;

    cur_normal_elem = hull_list_head;

    while (cur_normal_elem != NULL)
    {
        Hull*   cur_hp              = (Hull *)(cur_normal_elem->contents);
        Matrix* cur_normal_mp       = cur_hp->normal_mp;
        Vector* cur_b_vp            = cur_hp->b_value_vp;
        double*   cur_b_elem_ptr;
        double**  cur_normal_row_ptr;
        double*   cur_normal_elem_ptr;
        int     cur_num_rows;

        /* cur_normal_mp was not NULL when we counted normals. */
        ASSERT(cur_normal_mp != NULL);

        if (cur_b_vp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_B_VALUES.");
            return ERROR;
        }

        cur_b_elem_ptr     = cur_b_vp->elements;
        cur_num_rows = cur_normal_mp->num_rows;
        cur_normal_row_ptr = cur_normal_mp->elements;

        for (i=0; i<cur_num_rows; i++)
        {
            normal_elem_ptr = *normal_row_ptr;
            cur_normal_elem_ptr = *cur_normal_row_ptr;

            for (j=0; j<2; j++)
            {
                *normal_elem_ptr = *cur_normal_elem_ptr;

                normal_elem_ptr++;
                cur_normal_elem_ptr++;
            }

            *constraint_elem_ptr = *cur_b_elem_ptr;

            normal_row_ptr++;
            cur_normal_row_ptr++;

            constraint_elem_ptr++;
            cur_b_elem_ptr++;
        }

        cur_normal_elem = cur_normal_elem->next;
    }

    x_step = (max_x - min_x) / resolution;
    x = min_x;

    NRE(result_mp = create_matrix(MAX_OF(5 * (MIN_NUM_2D_HULL_POINTS-1),
                                         2 * (resolution + 1)),
                                  2));

    result_row_ptr = result_mp->elements;

    num_result_points = 0;

    /*
    // We re-estimate the intersection bounds, in order to later determine
    // whether refinement is in order.  Initialize min's to max's and vice
    // versa.
    */
    new_min_x = max_x;
    new_min_y = max_y;
    new_max_x = min_x;
    new_max_y = min_y;

    for (i=0; i<resolution+1; i++)
    {
        normal_row_ptr = normal_mp->elements;
        constraint_elem_ptr = constraint_vp->elements;
#ifdef TEST_INTERSECTION_ROUTINES
        hull_min_y = min_y - 1.0;
        hull_max_y = max_y + 1.0;
#else
        hull_min_y = min_y;
        hull_max_y = max_y;
#endif

        for (k=0; k<num_normals; k++)
        {
            normal_elem_ptr = *normal_row_ptr;
            temp = (*constraint_elem_ptr) - (*normal_elem_ptr)*x;
            normal_elem_ptr++;

            /*
            // Do expensive case first to avoid divide by zero later. This case
            // is it the place is essentially vertical.
            */
#ifdef HOW_IT_WAS
            if (IS_ZERO_DBL(*normal_elem_ptr))
#else
            if (    (*normal_elem_ptr > -HULL_EPSILON)
                 && (*normal_elem_ptr < HULL_EPSILON)
               )
#endif
            {
#ifdef HOW_IT_WAS
                if (temp < 0.0)
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
                    ;   /* Do nothing: This normal is irrelavent */
                }
            }
            else if (*normal_elem_ptr > 0.0)
            {
                y = temp / (*normal_elem_ptr);

                if (y < hull_max_y)
                {
                    hull_max_y = y;

                    if (hull_min_y > hull_max_y)
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

                    if (hull_min_y > hull_max_y)
                    {
                        break;  /* No solution */
                    }
                }
            }

            constraint_elem_ptr++;
            normal_row_ptr++;
        }

        if (hull_min_y <= hull_max_y)
        {
#ifdef TEST_INTERSECTION_ROUTINES
            if (    (hull_min_y < min_y - 0.5)
                 || (hull_max_y > max_y + 0.5))
            {
                pso("Min or max was not changed (approximation_2D_intersect_hulls)!\n");
                dbf(min_x);
                dbf(max_x);
                dbf(min_y);
                dbf(max_y);
                dbf(x);
                dbf(hull_min_y);
                dbf(hull_max_y);
            }
#endif
            if (x          > new_max_x) new_max_x = x;
            if (x          < new_min_x) new_min_x = x;
            if (hull_max_y > new_max_y) new_max_y = hull_max_y;
            if (hull_min_y < new_min_y) new_min_y = hull_min_y;

            (*result_row_ptr)[0] = x;
            (*result_row_ptr)[1] = hull_min_y;
            result_row_ptr++;
            num_result_points++;

            (*result_row_ptr)[0] = x;
            (*result_row_ptr)[1] = hull_max_y;
            result_row_ptr++;
            num_result_points++;
        }
         x += x_step;
   }

    x_range = new_max_x - new_min_x;
    y_range = new_max_y - new_min_y;
    dim_product = x_range * y_range;

    if ((num_result_points > 0) &&
        (dim_product < (min_dim_product / 4.0)))
    {
#ifdef TEST_INTERSECTION_ROUTINES
        ERE(pso("Refinement step.\n"));
#endif
        /*
        // Looks wrong to me. MIN_OF should be MAX_OF, and vice versa. However,
        // there seems to be no good reason to add in the step anyway.
        //
        // max_x = MAX_OF(max_x, (new_max_x + x_step));
        // min_x = MIN_OF(min_x, (new_min_x - x_step));
        */
        max_x = new_max_x;
        min_x = new_min_x;

        x_step = (max_x - min_x) / resolution;

        num_result_points = 0;
        result_row_ptr = result_mp->elements;

        x = min_x;

        for (i=0; i<resolution+1; i++)
        {
            normal_row_ptr = normal_mp->elements;
            constraint_elem_ptr = constraint_vp->elements;
#ifdef TEST_INTERSECTION_ROUTINES
            hull_min_y = min_y - 1.0;
            hull_max_y = max_y + 1.0;
#else
            hull_min_y = min_y;
            hull_max_y = max_y;
#endif

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
                    if (temp < 0.0)
#else
                    if (temp < HULL_EPSILON)
#endif
                    {
                        hull_min_y = DBL_MAX; /* Force no solution */
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
                    y = temp / (*normal_elem_ptr);

                    if (y < hull_max_y)
                    {
                        hull_max_y = y;

                        if (hull_min_y > hull_max_y)
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

                        if (hull_min_y > hull_max_y)
                        {
                            break;  /* No solution */
                        }
                    }
                }

                constraint_elem_ptr++;
                normal_row_ptr++;
            }

            if (hull_min_y <= hull_max_y)
            {
#ifdef TEST_INTERSECTION_ROUTINES
                if (    (hull_min_y < min_y - 0.5)
                     || (hull_max_y > max_y + 0.5)
                   )
                {
                    pso("Min or max was not changed (approximation_2D_intersect_hulls)!\n");
                    dbf(min_x);
                    dbf(max_x);
                    dbf(min_y);
                    dbf(max_y);
                    dbf(x);
                    dbf(hull_min_y);
                    dbf(hull_max_y);
                }
#endif

                (*result_row_ptr)[0] = x;
                (*result_row_ptr)[1] = hull_min_y;
                result_row_ptr++;
                num_result_points++;

                (*result_row_ptr)[0] = x;
                (*result_row_ptr)[1] = hull_max_y;
                result_row_ptr++;
                num_result_points++;
            }

             x += x_step;
         }
    }

    result_mp->num_rows = num_result_points;

    /*
    // If the number of points is too small, then finding the hull wont work,
    // so we expand it.
    */
    if (    (num_result_points < MIN_NUM_2D_HULL_POINTS)
         && (num_result_points > 0)
       )
    {
        double y_step, x_offset, y_offset;


        y_step = (max_y - min_y) / resolution;

        for (i=0; i<num_result_points; i++)
        {
            x = (result_mp->elements)[i][0];
            y = (result_mp->elements)[i][1];

            for (j=0; j<2; j++)
            {
                for (k=0; k<2; k++)
                {
                    if (IS_EVEN(j))
                    {
                        x_offset = x_step/2.0;
                    }
                    else
                    {
                        x_offset = -x_step/2.0;
                    }
                    if (IS_EVEN(k))
                    {
                        y_offset = y_step/2.0;
                    }
                    else
                    {
                        y_offset = -y_step/2.0;
                    }

                    (*result_row_ptr)[0] = x + x_offset;
                    (*result_row_ptr)[1] = y + y_offset;

                    result_row_ptr++;
                }
            }
        }
        /* Looks wrong to me.
        //
        // num_result_points += 4;
        */
        num_result_points *= 5;
        result_mp->num_rows = num_result_points;

#ifdef TEST_INTERSECTION_ROUTINES
        pso("At new expansion code!\n");
        EPE(fp_write_matrix_with_title(result_mp, stdout, "result_mp"));
#endif
    }

    if (num_result_points == 0)
    {
        set_error("Hull intersection is empty.");
        free_hull(*result_hp_ptr);
        *result_hp_ptr = NULL;
        result = NO_SOLUTION;
    }
    else
    {
        free_hull(*result_hp_ptr);
        *result_hp_ptr = find_convex_hull(result_mp, options);

        if (*result_hp_ptr == NULL) result = ERROR;
        else                        result = NO_ERROR;
    }

    free_matrix(normal_mp);
    free_vector(constraint_vp);
    free_matrix(result_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                    approximation_3D_intersect_hulls
 * -----------------------------------------------------------------------------
*/
static int approximation_3D_intersect_hulls
(
    Queue_element* hull_list_head,
    int            resolution,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Queue_element* cur_point_elem;
    Queue_element* cur_normal_elem;
    double           min_dim_product;
    double           x_range;
    double           y_range;
    double           z_range;
    double           dim_product;
    Matrix*        normal_mp;
    Vector*        constraint_vp;
    double**         normal_row_ptr;
    double*          normal_elem_ptr;
    double*          constraint_elem_ptr;
    int            num_normals;
    int            num_good_normals;
    int            num_result_points;
    int     i,j,k,m;
    double    min_x, max_x, min_y, max_y, min_z, max_z, x, y, z;
    double    x_step, y_step;
    Matrix* result_mp;
    double**  result_row_ptr;
    Vector* x_cache_vp;
    double*   x_cache_ptr;
    double    hull_min_z, hull_max_z, temp;
    double    new_min_x, new_max_x, new_min_y, new_max_y, new_min_z, new_max_z;
    int     result = NO_ERROR;


    cur_point_elem = hull_list_head;

    min_x = min_y = min_z = -DBL_MAX;
    max_x = max_y = max_z = DBL_MAX;

    while (cur_point_elem != NULL)
    {
        Hull* cur_hp = (Hull*)(cur_point_elem->contents);

        /*
        // The inequalities may seem unatural, because we are doing a max of
        // mins, and a min of maxs.
        */
        if (cur_hp->min_x > min_x) min_x = cur_hp->min_x;
        if (cur_hp->min_y > min_y) min_y = cur_hp->min_y;
        if (cur_hp->min_z > min_z) min_z = cur_hp->min_z;

        if (cur_hp->max_x < max_x) max_x = cur_hp->max_x;
        if (cur_hp->max_y < max_y) max_y = cur_hp->max_y;
        if (cur_hp->max_z < max_z) max_z = cur_hp->max_z;

        cur_point_elem = cur_point_elem->next;
    }

    if ((min_x > max_x) || (min_y > max_y) || (min_z > max_z))
    {
        return NO_SOLUTION;
    }

    x_range = max_x - min_x;
    y_range = max_y - min_y;
    z_range = max_z - min_z;
    min_dim_product = x_range * y_range * z_range;

    /*
    // Count normals.
    */
    num_normals = 0;
    cur_normal_elem = hull_list_head;

    while (cur_normal_elem != NULL)
    {
        Hull*   cur_hp        = (Hull *)(cur_normal_elem->contents);
        Matrix* cur_normal_mp = cur_hp->normal_mp;

        if (cur_normal_mp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_NORMALS.");
            return ERROR;
        }
        num_normals += cur_normal_mp->num_rows;
        cur_normal_elem = cur_normal_elem->next;
    }

    NRE(normal_mp = create_matrix(num_normals, 3));
    NRE(constraint_vp = create_vector(num_normals));

    normal_row_ptr = normal_mp->elements;
    constraint_elem_ptr = constraint_vp->elements;

    cur_normal_elem = hull_list_head;

    num_good_normals = 0;

    while (cur_normal_elem != NULL)
    {
        Hull*   cur_hp             = (Hull *)(cur_normal_elem->contents);
        Matrix* cur_normal_mp      = cur_hp->normal_mp;
        Vector* cur_b_vp           = cur_hp->b_value_vp;
        double*   cur_b_elem_ptr;
        double**  cur_normal_row_ptr;
        int     cur_num_rows;

        /* cur_normal_mp was not NULL when we counted normals. */
        ASSERT(cur_normal_mp != NULL);

        if (cur_b_vp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_B_VALUES.");
            return ERROR;
        }

        cur_b_elem_ptr     = cur_b_vp->elements;
        cur_num_rows       = cur_normal_mp->num_rows;
        cur_normal_row_ptr = cur_normal_mp->elements;

        for (i=0; i<cur_num_rows; i++)
        {
            double* cur_normal_elem_ptr = * cur_normal_row_ptr;
            double  min_x_n = min_x * cur_normal_elem_ptr[ 0 ];
            double  max_x_n = max_x * cur_normal_elem_ptr[ 0 ];
            double  min_y_n = min_y * cur_normal_elem_ptr[ 1 ];
            double  max_y_n = max_y * cur_normal_elem_ptr[ 1 ];
            double  min_z_n = min_z * cur_normal_elem_ptr[ 2 ];
            double  max_z_n = max_z * cur_normal_elem_ptr[ 2 ];
            double  b       = *cur_b_elem_ptr;
            double  c1      = min_x_n + min_y_n + max_z_n - b;
            double  c2      = max_x_n + min_y_n + max_z_n - b;
            double  c3      = min_x_n + max_y_n + max_z_n - b;
            double  c4      = max_x_n + max_y_n + max_z_n - b;
            double  c5      = min_x_n + min_y_n + min_z_n - b;
            double  c6      = max_x_n + min_y_n + min_z_n - b;
            double  c7      = min_x_n + max_y_n + min_z_n - b;
            double  c8      = max_x_n + max_y_n + min_z_n - b;

            /*
            // Exclude planes which miss the max extent cube.
            */
            if (    (c1 > -PLANE_CULL_EPSILON)
                 || (c2 > -PLANE_CULL_EPSILON)
                 || (c3 > -PLANE_CULL_EPSILON)
                 || (c4 > -PLANE_CULL_EPSILON)
                 || (c5 > -PLANE_CULL_EPSILON)
                 || (c6 > -PLANE_CULL_EPSILON)
                 || (c7 > -PLANE_CULL_EPSILON)
                 || (c8 > -PLANE_CULL_EPSILON)
               )
            {
                normal_elem_ptr = *normal_row_ptr;

                for (j=0; j<3; j++)
                {
                    *normal_elem_ptr = *cur_normal_elem_ptr;

                    normal_elem_ptr++;
                    cur_normal_elem_ptr++;
                }

                *constraint_elem_ptr = *cur_b_elem_ptr;

                normal_row_ptr++;
                constraint_elem_ptr++;

                num_good_normals++;
            }
            cur_normal_row_ptr++;
            cur_b_elem_ptr++;
        }

        cur_normal_elem = cur_normal_elem->next;
    }

#ifdef TEST_INTERSECTION_ROUTINES
    pso("%d/%d normals excluded.\n", num_normals - num_good_normals,
        num_normals);
#endif

    num_normals = num_good_normals;
    normal_mp->num_rows = num_good_normals;
    constraint_vp->length = num_good_normals;

    x_step = x_range / resolution;
    y_step = y_range / resolution;

    x = min_x;

    NRE(x_cache_vp = create_vector(num_normals));

    NRE(result_mp = create_matrix(MAX_OF(2 * (resolution + 1)*(resolution + 1),
                                         (MIN_NUM_3D_HULL_POINTS-1)*9),
                                  3));

    result_row_ptr = result_mp->elements;

    num_result_points = 0;

    /*
    // Initialize min's to max's and vice versa.
    */
    new_min_x = max_x;
    new_min_y = max_y;
    new_min_z = max_z;
    new_max_x = min_x;
    new_max_y = min_y;
    new_max_z = min_z;

    for (i=0; i<resolution+1; i++)
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

        for (j=0; j<resolution+1; j++)
        {
            normal_row_ptr = normal_mp->elements;
            constraint_elem_ptr = constraint_vp->elements;
            x_cache_ptr = x_cache_vp->elements;

#ifdef TEST_INTERSECTION_ROUTINES
            hull_min_z = min_z - 1.0;
            hull_max_z = max_z + 1.0;
#else
            hull_min_z = min_z;
            hull_max_z = max_z;
#endif

            for (k=0; k<num_normals; k++)
            {
                normal_elem_ptr = *normal_row_ptr;
                normal_elem_ptr++;
                temp = (*constraint_elem_ptr) - (*x_cache_ptr) -
                                                        (*normal_elem_ptr)*y;
                normal_elem_ptr++;

                if (IS_ZERO_DBL(*normal_elem_ptr))
                {
                    if (temp < 0.0)
                    {
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

                    if (z < hull_max_z)
                    {
                        hull_max_z = z;

                        if (hull_min_z > hull_max_z)
                        {
                            break;  /* No solution */
                        }
                    }
                }
                else
                {
                    z = temp / (*normal_elem_ptr);

                    if (z > hull_min_z)
                    {
                        hull_min_z = z;

                        if (hull_min_z > hull_max_z)
                        {
                            break;  /* No solution */
                        }
                    }
                }

                constraint_elem_ptr++;
                normal_row_ptr++;
                x_cache_ptr++;
            }

            if (hull_min_z <= hull_max_z)
            {
#ifdef TEST_INTERSECTION_ROUTINES
                if (    (hull_min_z < min_z - 0.5)
                     || (hull_max_z > max_z + 0.5)
                   )
                {
                    pso("Min or max was not changed (approximation_3D_intersect_hulls)!\n");
                }
#endif
                if (x > new_max_x)
                {
                    new_max_x = x;
                }
                if (x < new_min_x)
                {
                    new_min_x = x;
                }
                if (y > new_max_y)
                {
                    new_max_y = y;
                }
                if (y < new_min_y)
                {
                    new_min_y = y;
                }
                if (hull_max_z > new_max_z)
                {
                    new_max_z = hull_max_z;
                }
                if (hull_min_z < new_min_z)
                {
                    new_min_z = hull_min_z;
                }

                (*result_row_ptr)[0] = x;
                (*result_row_ptr)[1] = y;
                (*result_row_ptr)[2] = hull_min_z;
                result_row_ptr++;
                num_result_points++;

                (*result_row_ptr)[0] = x;
                (*result_row_ptr)[1] = y;
                (*result_row_ptr)[2] = hull_max_z;
                result_row_ptr++;
                num_result_points++;
            }

            y += y_step;
        }
        x += x_step;
    }

    if (result != ERROR)
    {
#ifdef TEST_INTERSECTION_ROUTINES
        pso("Doing refinement step.\n");
#endif
        x_range = new_max_x - new_min_x;
        y_range = new_max_y - new_min_y;
        z_range = new_max_z - new_min_z;
        dim_product = x_range * y_range * z_range;

        if (    (num_result_points > 0)
             && (dim_product < (min_dim_product / 8.0))
           )
        {
            /*
            // Looks wrong to me. MIN_OF should be MAX_OF, and vice versa.
            // However, there seems to be no good reason to add in the step
            // anyway.
            //
            max_x = MAX_OF(max_x, (new_max_x + x_step));
            min_x = MIN_OF(min_x, (new_min_x - x_step));
            max_y = MAX_OF(max_y, (new_max_y + y_step));
            min_y = MIN_OF(min_y, (new_min_y - y_step));
            */
            max_x = new_max_x;
            min_x = new_min_x;
            max_y = new_max_y;
            min_y = new_min_y;

            x_step = (max_x - min_x) / resolution;
            y_step = (max_y - min_y) / resolution;

            result_row_ptr = result_mp->elements;

            num_result_points = 0;

            x = min_x;

            for (i=0; i<resolution+1; i++)
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

                for (j=0; j<resolution+1; j++)
                {
                    if (io_atn_flag)
                    {
                        set_error("Processing interrupted.");
                        halt_all_output = FALSE;
                        result = ERROR;
                        break;
                    }

                    normal_row_ptr = normal_mp->elements;
                    constraint_elem_ptr = constraint_vp->elements;
                    x_cache_ptr = x_cache_vp->elements;

#ifdef TEST_INTERSECTION_ROUTINES
                    hull_min_z = min_z - 1.0;
                    hull_max_z = max_z + 1.0;
#else
                    hull_min_z = min_z;
                    hull_max_z = max_z;
#endif

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
                            if (temp < 0.0)
#else
                            if (temp < HULL_EPSILON)
#endif
                            {
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

                            if (z < hull_max_z)
                            {
                                hull_max_z = z;

                                if (hull_min_z > hull_max_z)
                                {
                                    break;  /* No solution */
                                }
                            }
                        }
                        else
                        {
                            z = temp / (*normal_elem_ptr);

                            if (z > hull_min_z)
                            {
                                hull_min_z = z;

                                if (hull_min_z > hull_max_z)
                                {
                                    break;  /* No solution */
                                }
                            }
                        }

                        constraint_elem_ptr++;
                        normal_row_ptr++;
                        x_cache_ptr++;
                    }

                    if (hull_min_z <= hull_max_z)
                    {
#ifdef TEST_INTERSECTION_ROUTINES
                        if (    (hull_min_z < min_z - 0.5)
                             || (hull_max_z > max_z + 0.5)
                           )
                        {
                            pso("Min or max was not changed (approximation_3D_intersect_hulls)!\n");
                        }
#endif
                        (*result_row_ptr)[0] = x;
                        (*result_row_ptr)[1] = y;
                        (*result_row_ptr)[2] = hull_min_z;
                        result_row_ptr++;
                        num_result_points++;

                        (*result_row_ptr)[0] = x;
                        (*result_row_ptr)[1] = y;
                        (*result_row_ptr)[2] = hull_max_z;
                        result_row_ptr++;
                        num_result_points++;
                    }

                    y += y_step;
                }

                if (result == ERROR) break;

                x += x_step;
            }
        }
    }

    if (result != ERROR)
    {
        result_mp->num_rows = num_result_points;

        if (    (num_result_points < MIN_NUM_3D_HULL_POINTS)
             && (num_result_points > 0)
           )
        {
            double z_step, x_offset, y_offset, z_offset;


            z_step = (max_z - min_z) / resolution;

            for (i=0; i<num_result_points; i++)
            {
                x = (result_mp->elements)[i][0];
                y = (result_mp->elements)[i][1];
                z = (result_mp->elements)[i][2];

                for (j=0; j<2; j++)
                {
                    for (k=0; k<2; k++)
                    {
                        for (m=0; m<2; m++)
                        {
                            if (IS_EVEN(j))
                            {
                                x_offset = x_step/2.0;
                            }
                            else
                            {
                                x_offset = -x_step/2.0;
                            }
                            if (IS_EVEN(k))
                            {
                                y_offset = y_step/2.0;
                            }
                            else
                            {
                                y_offset = -y_step/2.0;
                            }
                            if (IS_EVEN(m))
                            {
                                z_offset = z_step/2.0;
                            }
                            else
                            {
                                z_offset = -z_step/2.0;
                            }

                            (*result_row_ptr)[0] = x + x_offset;
                            (*result_row_ptr)[1] = y + y_offset;
                            (*result_row_ptr)[2] = z + z_offset;

                            result_row_ptr++;
                        }
                    }
                }
            }
            /*
            // Looks wrong to me.
            num_result_points += 8;
            */
            num_result_points *= 9;
            result_mp->num_rows = num_result_points;

#ifdef TEST_INTERSECTION_ROUTINES
            pso("At new expansion code!\n");
            EPE(fp_write_matrix_with_title(result_mp, stdout, "result_mp"));
#endif
        }
    }

    if (result == ERROR)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (num_result_points == 0)
    {
        set_error("Hull intersection is empty.");
        free_hull(*result_hp_ptr);
        *result_hp_ptr = NULL;
        result = NO_SOLUTION;
    }
    else
    {
        free_hull(*result_hp_ptr);
        *result_hp_ptr = find_convex_hull(result_mp, options);

        if (*result_hp_ptr == NULL)
        {
            add_error("Intersection of 3D hulls failed.");
            result = ERROR;
        }
        else
        {
            result = NO_ERROR;
        }
    }

    free_vector(x_cache_vp);
    free_matrix(normal_mp);
    free_vector(constraint_vp);
    free_matrix(result_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int dual_space_intersect_hulls
(
    Queue_element* hull_list_head,
    unsigned long  options,
    Hull**         result_hp_ptr
)
{
    Vector*    a_vp      = NULL;
    Matrix*    normal_mp = NULL;
    Vector*    b_vp      = NULL;
    int        point_in_hull_result;
    int        result;
    int        dim;


    if (((Hull*)(hull_list_head->contents))->vertex_mp == NULL)
    {
        set_bug("All hulls to be intersected must be created with option HULL_VERTICES.");
        return ERROR;
    }
    else
    {
        dim = ((Hull*)(hull_list_head->contents))->vertex_mp->num_cols;
    }

    if (dim == 2)
    {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
        IMPORT int     kjb_debug_level;
#endif

        point_in_hull_result =
            find_point_in_2D_hull_intersection(hull_list_head,
                                               fs_hull_intersection_resolution,
                                               &a_vp, &normal_mp, &b_vp);
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
        if (kjb_debug_level > 20)
        {
            int            count           = 1;
            int            plot_id;
            Queue_element* cur_elem;

            UNTESTED_CODE();

            EPE(plot_id = plot_open());

            if (plot_id > 0)
            {
                if (point_in_hull_result == NO_ERROR)
                {
                    EPE(plot_vector_point(plot_id, a_vp, "Interior point"));
                }

                cur_elem = hull_list_head;

                while (cur_elem != NULL)
                {
                    char title[ 100 ];

                    ERE(kjb_sprintf(title, sizeof(title), "Hull %d", count++));

                    EPE(plot_matrix_vector_cols(plot_id,
                                      ((Hull*)(cur_elem->contents))->facets,
                                      title));
                    cur_elem = cur_elem->next;
                }

                if (yes_no_query("Close plot"))
                {
                    plot_close(plot_id);
                }
            }
        }
#endif
    }
    else if (dim == 3)
    {
        point_in_hull_result =
            find_point_in_3D_hull_intersection(hull_list_head,
                                               fs_hull_intersection_resolution,
                                               &a_vp, &normal_mp, &b_vp);
    }
    else
    {
        set_error("Dual space hull intersection of %d-D hulls not supported.",
                  dim);
        return ERROR;
    }

    if (point_in_hull_result == ERROR)
    {
        result = ERROR;
    }
    else if (point_in_hull_result == TRUE)
    {
        result = NO_ERROR;
    }
    else
    {
        result = NO_SOLUTION;
    }

    if (result == NO_ERROR)
    {
#ifdef TEST_INTERSECTION_ROUTINES
        Queue_element* cur_elem = hull_list_head;
#endif

        Vector* new_b_vp                    = NULL;
        Vector* n_dot_a_vp                  = NULL;
        Matrix* dual_vertex_mp              = NULL;
        Matrix* hull_intersection_vertex_mp = NULL;
        Hull*   dual_hp                     = NULL;


#ifdef TEST_INTERSECTION_ROUTINES
        while (cur_elem != NULL)
        {
            if ( ! is_point_inside_hull((Hull*)(cur_elem->contents), a_vp))
            {
                set_error("Inside point for 3D hull intersection is not ");
                cat_error("inside all the hulls.");
                result = ERROR;
                break;
            }

            cur_elem = cur_elem->next;
        }
#endif

        if (result != ERROR)
        {
            result = multiply_matrix_and_vector(&n_dot_a_vp, normal_mp, a_vp);
        }

        if (result != ERROR)
        {
            result = subtract_vectors(&new_b_vp, b_vp, n_dot_a_vp);
        }

        if (result != ERROR)
        {
            result = divide_matrix_by_col_vector(&dual_vertex_mp, normal_mp,
                                                  new_b_vp);
        }

        if (result != ERROR)
        {
            dual_hp = find_convex_hull(dual_vertex_mp, DEFAULT_HULL_OPTIONS);

            if (dual_hp == NULL) result = ERROR;
        }

        if (result != ERROR)
        {
            result = divide_matrix_by_col_vector(&hull_intersection_vertex_mp,
                                                  dual_hp->normal_mp,
                                                  dual_hp->b_value_vp);
        }

        if (result != ERROR)
        {
            result = ow_add_row_vector_to_matrix(hull_intersection_vertex_mp,
                                                  a_vp);
        }

        if (result != ERROR)
        {
            Hull* hp = find_convex_hull(hull_intersection_vertex_mp, options);

            if (hp == NULL)
            {
                result = ERROR;
            }
            else
            {
                free_hull(*result_hp_ptr);
                *result_hp_ptr = hp;
            }
        }

        free_vector(new_b_vp);
        free_vector(n_dot_a_vp);
        free_hull(dual_hp);
        free_matrix(hull_intersection_vertex_mp);
        free_matrix(dual_vertex_mp);
    }

    free_vector(a_vp);
    free_vector(b_vp);
    free_matrix(normal_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                    find_point_in_2D_hull_intersection
 * -----------------------------------------------------------------------------
*/
int find_point_in_2D_hull_intersection
(
    Queue_element* hull_list_head,
    int            resolution,
    Vector**       result_ptr,
    Matrix**       normal_mpp,
    Vector**       constrant_vpp
)
{
    Queue_element* cur_point_elem;
    Queue_element* cur_normal_elem;
    Matrix*        normal_mp;
    Vector*        constraint_vp;
    double**         normal_row_ptr;
    double*          constraint_elem_ptr;
    int            num_normals;
    int            num_points          = 0;
    int            i, j;
    double           min_x;
    double           max_x;
    double           min_y;
    double           max_y;
    double           x, y;
    double           x_step;
    double           x_ave  = 0.0;
    double           y_ave  = 0.0;
#ifdef TEST_INTERSECTION_ROUTINES
    Vector*        test_constraint_vp = NULL;
    Vector*        diff_vp = NULL;
    Vector*        test_vp = NULL;
#endif
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
    int            found_by_exhaustive_search = FALSE;
#endif
    int            max_num_tries;
    int            try_count;
    double           distance;
    double         min_range;


    cur_point_elem = hull_list_head;

    min_x = min_y = -DBL_MAX;
    max_x = max_y =  DBL_MAX;

    while (cur_point_elem != NULL)
    {
        Hull* cur_hp = (Hull*)(cur_point_elem->contents);

        /*
        // The inequalities may seem unatural, because we are doing a max of
        // mins, and a min of maxs.
        */
        if (cur_hp->min_x > min_x) min_x = cur_hp->min_x;
        if (cur_hp->min_y > min_y) min_y = cur_hp->min_y;

        if (cur_hp->max_x < max_x) max_x = cur_hp->max_x;
        if (cur_hp->max_y < max_y) max_y = cur_hp->max_y;

        cur_point_elem = cur_point_elem->next;
    }

    if ((min_x >= max_x) || (min_y >= max_y))
    {
        return FALSE;
    }

    min_range = MIN_OF(max_x - min_x, max_y - min_y);

#ifdef TEST_INTERSECTION_ROUTINES
    ERE(get_target_vector(&test_vp, 2));
#endif

    /*
    // Count normals.
    */
    num_normals = 0;
    cur_normal_elem = hull_list_head;

    while (cur_normal_elem != NULL)
    {
        Hull*   cur_hp        = (Hull *)(cur_normal_elem->contents);
        Matrix* cur_normal_mp = cur_hp->normal_mp;

        if (cur_normal_mp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_NORMALS.");
            return ERROR;
        }
        num_normals += cur_normal_mp->num_rows;

        cur_normal_elem = cur_normal_elem->next;
    }

    ERE(get_target_matrix(normal_mpp, num_normals, 2));
    normal_mp = *normal_mpp;

    ERE(get_target_vector(constrant_vpp, num_normals));
    constraint_vp = *constrant_vpp;

    normal_row_ptr = normal_mp->elements;
    constraint_elem_ptr = constraint_vp->elements;

    cur_normal_elem = hull_list_head;

    while (cur_normal_elem != NULL)
    {
        Hull*   cur_hp              = (Hull *)(cur_normal_elem->contents);
        Matrix* cur_normal_mp       = cur_hp->normal_mp;
        Vector* cur_b_vp            = cur_hp->b_value_vp;
        double*   cur_b_elem_ptr;
        double**  cur_normal_row_ptr;
        double*   cur_normal_elem_ptr;
        double*   normal_elem_ptr;
        int     cur_num_rows;

        /* cur_normal_mp was not NULL when we counted normals. */
        ASSERT(cur_normal_mp != NULL);

        if (cur_b_vp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_B_VALUES.");
            return ERROR;
        }

        cur_b_elem_ptr     = cur_b_vp->elements;
        cur_num_rows = cur_normal_mp->num_rows;
        cur_normal_row_ptr = cur_normal_mp->elements;

        for (i=0; i<cur_num_rows; i++)
        {
            normal_elem_ptr = *normal_row_ptr;
            cur_normal_elem_ptr = *cur_normal_row_ptr;

            for (j=0; j<2; j++)
            {
                *normal_elem_ptr = *cur_normal_elem_ptr;

                normal_elem_ptr++;
                cur_normal_elem_ptr++;
            }

            *constraint_elem_ptr = *cur_b_elem_ptr;

            normal_row_ptr++;
            cur_normal_row_ptr++;

            constraint_elem_ptr++;
            cur_b_elem_ptr++;
        }

        cur_normal_elem = cur_normal_elem->next;
    }

    x_step = (max_x - min_x) / resolution;

    try_count = 0;

    max_num_tries = kjb_rint(2.0 * sqrt((double)resolution));

    while (    (try_count < max_num_tries)
            && (num_points < NUM_POINTS_FOR_INTERIOR_POINT_AVERAGE)
          )
    {
        try_count++;

        x = min_x + resolution * kjb_rand_2() * x_step;

        if (is_segment_in_2D_hull_intersection(x, min_y, max_y,
                                               normal_mp,
                                               constraint_vp,
                                               &y, &distance))
        {
#ifdef TEST_INTERSECTION_ROUTINES
            test_vp->elements[ 0 ] = x;
            test_vp->elements[ 1 ] = y;

            ERE(multiply_matrix_and_vector(&test_constraint_vp, normal_mp,
                                          test_vp));
            if (first_vector_is_less(constraint_vp, test_constraint_vp,
                                     0.0, 0.0))
            {
                ERE(subtract_vectors(&diff_vp, constraint_vp,
                                     test_constraint_vp));
                pso("Constraint failure by %e.\n",
                    min_vector_element(diff_vp));
            }
#endif
            x_ave += x;
            y_ave += y;

            num_points++;

            if (distance > min_range / 20.0)
            {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
                pso("Point %d (try_count %d) with good interior distance found.\n",
                    num_points, try_count);
#endif
                break;
            }
        }
    }

    if (    (try_count == max_num_tries)
         && (num_points < NUM_POINTS_FOR_INTERIOR_POINT_AVERAGE)
        )
    {
        /*
        // Note: We maybe keeping one or two found points in the average. This
        // should be no big deal, but if it is, then reset BOTH num_points, and
        // the three averages.
        */
        x = min_x;

        for (i=0; i<resolution+1; i++)
        {
            if (is_segment_in_2D_hull_intersection(x, min_y, max_y,
                                                   normal_mp,
                                                   constraint_vp,
                                                   &y, &distance))
            {
#ifdef TEST_INTERSECTION_ROUTINES
                test_vp->elements[ 0 ] = x;
                test_vp->elements[ 1 ] = y;

                ERE(multiply_matrix_and_vector(&test_constraint_vp,
                                               normal_mp, test_vp));
                if (first_vector_is_less(constraint_vp, test_constraint_vp,
                                         0.0, 0.0))
                {
                    ERE(subtract_vectors(&diff_vp, constraint_vp,
                                         test_constraint_vp));
                    pso("Constraint failure by %e.\n",
                        min_vector_element(diff_vp));
                }
#endif
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
                found_by_exhaustive_search = TRUE;
#endif
                x_ave += x;
                y_ave += y;

                num_points++;

                if (distance > min_range / 30.0)
                {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
                    pso("Point %d with good interior distance found.\n",
                        num_points);
#endif
                    break;
                }
            }
            x += x_step;
        }
    }
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
    else
    {
        pso("Tries: %d.\n", try_count);
        kjb_flush();
    }
#endif

#ifdef TEST_INTERSECTION_ROUTINES
    free_vector(test_vp);
    free_vector(test_constraint_vp);
    free_vector(diff_vp);
#endif

    if (num_points > 0)
    {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
        if (found_by_exhaustive_search)
        {
            pso("Exhaustive search was required to find intersection point.\n");
            kjb_flush();
        }
#endif
        ERE(get_target_vector(result_ptr, 2));

        (*result_ptr)->elements[ 0 ] = x_ave / num_points;
        (*result_ptr)->elements[ 1 ] = y_ave / num_points;

        return TRUE;
    }
    else
    {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
        pso("Exhaustive search indicates that there is no intersection.\n");
        kjb_flush();
#endif
        return FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                is_segment_in_2D_hull_intersection
 * -----------------------------------------------------------------------------
*/
static int is_segment_in_2D_hull_intersection
(
    double  x_in,
    double  min_y,
    double  max_y,
    Matrix* normal_mp,
    Vector* constraint_vp,
    double* y_ptr,
    double* distance_ptr
)
{
    double** normal_row_ptr;
    double*  normal_elem_ptr;
    double*  constraint_elem_ptr;
    int    num_normals;
    int    k;
    double   x                   = x_in;
    double   y;
    double   hull_min_y;
    double   hull_max_y;
    double   temp;


#ifdef TEST_INTERSECTION_ROUTINES
    hull_min_y = min_y - 1.0;
    hull_max_y = max_y + 1.0;
#else
    hull_min_y = min_y;
    hull_max_y = max_y;
#endif

    num_normals         = normal_mp->num_rows;
    normal_row_ptr      = normal_mp->elements;
    constraint_elem_ptr = constraint_vp->elements;

    for (k=0; k<num_normals; k++)
    {
        normal_elem_ptr = *normal_row_ptr;
        temp = *constraint_elem_ptr - (*normal_elem_ptr * x);
        normal_elem_ptr++; /* Second componant of normal. */

        /*
        // Expensive case is first, to avoid divide by zero later.
        */
#ifdef HOW_IT_WAS
        if (IS_ZERO_DBL(*normal_elem_ptr))
#else
        if (    (*normal_elem_ptr > -HULL_EPSILON)
             && (*normal_elem_ptr < HULL_EPSILON)
           )
#endif
        {
            /* Plane is vertical */

            /* REPLACED: if (temp < 0.0) */
            /* Want clean interior point. */
#ifdef HOW_IT_WAS
            if (temp < DBL_EPSILON) /* And we are on the wrong side. */
#else
            if (temp < HULL_EPSILON)
#endif
            {
                hull_min_y = DBL_MAX;  /* Force no solution */
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
            y = temp / (*normal_elem_ptr);

            if (y < hull_max_y)
            {
                hull_max_y = y;

                /* REPLACED: if (hull_min_y > hull_max_y) */
                /* Want clean interior point. */
                if (IS_NOT_LESSER_DBL(hull_min_y, hull_max_y))
                {
                    return FALSE;
                }
            }
        }
        else if (*normal_elem_ptr < 0.0)
        {
            y = temp / (*normal_elem_ptr);

            if (y > hull_min_y)
            {
                hull_min_y = y;

                /* REPLACED: if (hull_min_y > hull_max_y) */
                /* Want clean interior point. */
                if (IS_NOT_LESSER_DBL(hull_min_y, hull_max_y))
                {
                    return FALSE;
                }
            }
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        constraint_elem_ptr++;
        normal_row_ptr++;
    }

    /* REPLACED: if (hull_min_y < hull_max_y) */
    /* Want clean interior point. */
    if (IS_LESSER_DBL(hull_min_y, hull_max_y))
    {
        double d, min_distance;

#ifdef TEST_INTERSECTION_ROUTINES
        if ((hull_min_y < min_y - 0.5) || (hull_max_y > max_y + 0.5))
        {
            pso("Min or max was not changed (is_segment_in_2D_hull_intersection)!\n");
        }
#endif

        y = (hull_min_y + hull_max_y) / 2.0;

        normal_row_ptr = normal_mp->elements;
        normal_elem_ptr = *normal_row_ptr;
        constraint_elem_ptr = constraint_vp->elements;

        min_distance = *constraint_elem_ptr - x * normal_elem_ptr[ 0 ] -
                                                        y * normal_elem_ptr[ 1 ];
        normal_row_ptr++;
        constraint_elem_ptr++;

        for (k=1; k<num_normals; k++)
        {
            normal_elem_ptr = *normal_row_ptr;

            d = *constraint_elem_ptr - x * normal_elem_ptr[ 0 ] -
                                                        y * normal_elem_ptr[ 1 ];

            if (d <min_distance) min_distance = d;

            constraint_elem_ptr++;
            normal_row_ptr++;
        }

        *y_ptr = y;
        *distance_ptr = min_distance;

        ASSERT(min_distance >= 0.0);

        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                    find_point_in_3D_hull_intersection
 * -----------------------------------------------------------------------------
*/
int find_point_in_3D_hull_intersection
(
    Queue_element* hull_list_head,
    int            resolution,
    Vector**       result_ptr,
    Matrix**       normal_mpp,
    Vector**       constrant_vpp
)
{
    Queue_element* cur_point_elem;
    Queue_element* cur_normal_elem;
    Matrix*        normal_mp;
    Vector*        constraint_vp;
    double**         normal_row_ptr;
    double*          normal_elem_ptr;
    double*          constraint_elem_ptr;
    int            num_normals;
    int            num_points          = 0;
    int            i, j;
    double           min_x;
    double           max_x;
    double           min_y;
    double           max_y;
    double           min_z;
    double           max_z;
    double           min_range;
    double           x, y, z;
    double           x_step;
    double           y_step;
    double           x_ave     = 0.0;
    double           y_ave     = 0.0;
    double           z_ave     = 0.0;
#ifdef TEST_INTERSECTION_ROUTINES
    Vector*        test_constraint_vp = NULL;
    Vector*        diff_vp = NULL;
    Vector*        test_vp = NULL;
#endif
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
    int            save_num_normals;
    int            found_by_exhaustive_search = FALSE;
#endif
    int            try_count;
    double           distance;
    int            break_out_of_outer;
    int            max_num_tries;


    cur_point_elem = hull_list_head;

    min_x = min_y = min_z = -DBL_MAX;
    max_x = max_y = max_z = DBL_MAX;

    while (cur_point_elem != NULL)
    {
        Hull* cur_hp = (Hull*)(cur_point_elem->contents);

        /*
        // The inequalities may seem unatural, because we are doing a max of
        // mins, and a min of maxs.
        */
        if (cur_hp->min_x > min_x) min_x = cur_hp->min_x;
        if (cur_hp->min_y > min_y) min_y = cur_hp->min_y;
        if (cur_hp->min_z > min_z) min_z = cur_hp->min_z;

        if (cur_hp->max_x < max_x) max_x = cur_hp->max_x;
        if (cur_hp->max_y < max_y) max_y = cur_hp->max_y;
        if (cur_hp->max_z < max_z) max_z = cur_hp->max_z;

        cur_point_elem = cur_point_elem->next;
    }

    if ((min_x >= max_x) || (min_y >= max_y) || (min_z >= max_z))
    {
        return FALSE;
    }

    min_range = MIN_OF(max_x - min_x, MIN_OF(max_y - min_y, max_z - min_z));

#ifdef TEST_INTERSECTION_ROUTINES
    ERE(get_target_vector(&test_vp, 3));
#endif

    /*
    // Count normals.
    */
    num_normals = 0;
    cur_normal_elem = hull_list_head;

    while (cur_normal_elem != NULL)
    {
        Hull*   cur_hp        = (Hull *)(cur_normal_elem->contents);
        Matrix* cur_normal_mp = cur_hp->normal_mp;

        if (cur_normal_mp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_NORMALS.");
            return ERROR;
        }
        num_normals += cur_normal_mp->num_rows;
        cur_normal_elem = cur_normal_elem->next;
    }

#ifdef REALLY_TEST_INTERSECTION_ROUTINES
    save_num_normals = num_normals;
#endif

    ERE(get_target_matrix(normal_mpp, num_normals, 3));
    normal_mp = *normal_mpp;

    ERE(get_target_vector(constrant_vpp, num_normals));
    constraint_vp = *constrant_vpp;

    normal_row_ptr = normal_mp->elements;
    constraint_elem_ptr = constraint_vp->elements;

    cur_normal_elem = hull_list_head;

    num_normals = 0;

    while (cur_normal_elem != NULL)
    {
        Hull*   cur_hp             = (Hull *)(cur_normal_elem->contents);
        Matrix* cur_normal_mp      = cur_hp->normal_mp;
        Vector* cur_b_vp           = cur_hp->b_value_vp;
        double*   cur_b_elem_ptr;
        double**  cur_normal_row_ptr;
        int     cur_num_rows;

        /* cur_normal_mp was not NULL when we counted normals. */
        ASSERT(cur_normal_mp != NULL);

        if (cur_b_vp == NULL)
        {
            set_bug("All hulls to be intersected must be created with option HULL_B_VALUES.");
            return ERROR;
        }

        cur_b_elem_ptr     = cur_b_vp->elements;
        cur_num_rows       = cur_normal_mp->num_rows;
        cur_normal_row_ptr = cur_normal_mp->elements;

        for (i=0; i<cur_num_rows; i++)
        {
            double* cur_normal_elem_ptr =  *cur_normal_row_ptr;
            double min_x_n = min_x * cur_normal_elem_ptr[ 0 ];
            double max_x_n = max_x * cur_normal_elem_ptr[ 0 ];
            double min_y_n = min_y * cur_normal_elem_ptr[ 1 ];
            double max_y_n = max_y * cur_normal_elem_ptr[ 1 ];
            double min_z_n = min_z * cur_normal_elem_ptr[ 2 ];
            double max_z_n = max_z * cur_normal_elem_ptr[ 2 ];
            double b       = *cur_b_elem_ptr;
            double c1      = min_x_n + min_y_n + max_z_n - b;
            double c2      = max_x_n + min_y_n + max_z_n - b;
            double c3      = min_x_n + max_y_n + max_z_n - b;
            double c4      = max_x_n + max_y_n + max_z_n - b;
            double c5      = min_x_n + min_y_n + min_z_n - b;
            double c6      = max_x_n + min_y_n + min_z_n - b;
            double c7      = min_x_n + max_y_n + min_z_n - b;
            double c8      = max_x_n + max_y_n + min_z_n - b;


            /*
            // Exclude planes which miss the max extent cube.
            */
            if (    (c1 > -PLANE_CULL_EPSILON)
                 || (c2 > -PLANE_CULL_EPSILON)
                 || (c3 > -PLANE_CULL_EPSILON)
                 || (c4 > -PLANE_CULL_EPSILON)
                 || (c5 > -PLANE_CULL_EPSILON)
                 || (c6 > -PLANE_CULL_EPSILON)
                 || (c7 > -PLANE_CULL_EPSILON)
                 || (c8 > -PLANE_CULL_EPSILON)
               )
            {
                normal_elem_ptr = *normal_row_ptr;

                for (j=0; j<3; j++)
                {
                    *normal_elem_ptr = *cur_normal_elem_ptr;

                    normal_elem_ptr++;
                    cur_normal_elem_ptr++;
                }

                *constraint_elem_ptr = *cur_b_elem_ptr;

                normal_row_ptr++;
                constraint_elem_ptr++;

                num_normals++;
            }

            cur_normal_row_ptr++;
            cur_b_elem_ptr++;
        }

        cur_normal_elem = cur_normal_elem->next;
    }

#ifdef REALLY_TEST_INTERSECTION_ROUTINES
    pso("%d/%d normals excluded.\n", save_num_normals - num_normals,
        save_num_normals);
#endif

    normal_mp->num_rows = num_normals;
    constraint_vp->length = num_normals;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;

    max_num_tries = 5*resolution;
    try_count = 0;

    while (    (try_count < max_num_tries)
            && (num_points < NUM_POINTS_FOR_INTERIOR_POINT_AVERAGE)
          )
    {
        try_count++;

        x = min_x + resolution * kjb_rand_2() * x_step;
        y = min_y + resolution * kjb_rand_2() * y_step;

        if (is_segment_in_3D_hull_intersection(x, y, min_z, max_z,
                                               normal_mp,
                                               constraint_vp,
                                               &z, &distance))
        {
#ifdef TEST_INTERSECTION_ROUTINES
            test_vp->elements[ 0 ] = x;
            test_vp->elements[ 1 ] = y;
            test_vp->elements[ 2 ] = z;

            ERE(multiply_matrix_and_vector(&test_constraint_vp, normal_mp,
                                          test_vp));
            if (first_vector_is_less(constraint_vp, test_constraint_vp,
                                     0.0, 0.0))
            {
                ERE(subtract_vectors(&diff_vp, constraint_vp,
                                     test_constraint_vp));
                pso("Constraint failure by %e.\n",
                    min_vector_element(diff_vp));
            }
#endif
            x_ave += x;
            y_ave += y;
            z_ave += z;

            num_points++;

            if (distance > min_range / 10.0)
            {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
                pso("Point %d (try_count %d) with good interior distance found.\n",
                    num_points, try_count);
#endif
                break;
            }
        }

    }

    if (    (try_count == max_num_tries)
         && (num_points < NUM_POINTS_FOR_INTERIOR_POINT_AVERAGE)
        )
    {
        /*
        // Note: We maybe keeping one or two found points in the average. This
        // should be no big deal, but if it is, then reset BOTH num_points, and
        // the three averages.
        */
        x = min_x;

        break_out_of_outer = FALSE;

        for (i=0; i<resolution+1; i++)
        {
            y = min_y;

            for (j=0; j<resolution+1; j++)
            {
                if (is_segment_in_3D_hull_intersection(x, y, min_z, max_z,
                                                       normal_mp,
                                                       constraint_vp,
                                                       &z, &distance))
                {
#ifdef TEST_INTERSECTION_ROUTINES
                    test_vp->elements[ 0 ] = x;
                    test_vp->elements[ 1 ] = y;
                    test_vp->elements[ 2 ] = z;

                    ERE(multiply_matrix_and_vector(&test_constraint_vp,
                                                   normal_mp, test_vp));
                    if (first_vector_is_less(constraint_vp, test_constraint_vp,
                                             0.0, 0.0))
                    {
                        ERE(subtract_vectors(&diff_vp, constraint_vp,
                                             test_constraint_vp));
                        pso("Constraint failure by %e.\n",
                            min_vector_element(diff_vp));
                    }
#endif
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
                    found_by_exhaustive_search = TRUE;
#endif
                    x_ave += x;
                    y_ave += y;
                    z_ave += z;

                    num_points++;

                    if (distance > min_range / 30.0)
                    {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
                        pso("Point %d with good interior distance found.\n",
                            num_points);
#endif
                        break_out_of_outer = TRUE;
                        break;
                    }
                }
                y += y_step;
            }
            if (break_out_of_outer) break;

            x += x_step;
        }
    }
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
    else
    {
        pso("Tries: %d.\n", try_count);
        kjb_flush();
    }
#endif

#ifdef TEST_INTERSECTION_ROUTINES
    free_vector(test_vp);
    free_vector(test_constraint_vp);
    free_vector(diff_vp);
#endif

    if (num_points > 0)
    {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
        if (found_by_exhaustive_search)
        {
            pso("Exhaustive search was required to find intersection point.\n");
            kjb_flush();
        }
#endif
        ERE(get_target_vector(result_ptr, 3));

        (*result_ptr)->elements[ 0 ] = x_ave / num_points;
        (*result_ptr)->elements[ 1 ] = y_ave / num_points;
        (*result_ptr)->elements[ 2 ] = z_ave / num_points;

        return TRUE;
    }
    else
    {
#ifdef REALLY_TEST_INTERSECTION_ROUTINES
        pso("Exhaustive search indicates that there is no intersection.\n");
        kjb_flush();
#endif
        return FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                  is_segment_in_3D_hull_intersection
 * -----------------------------------------------------------------------------
*/
static int is_segment_in_3D_hull_intersection
(
    double  x_in,
    double  y,
    double  min_z,
    double  max_z,
    Matrix* normal_mp,
    Vector* constraint_vp,
    double* z_ptr,
    double* distance_ptr
)
{
    double** normal_row_ptr;
    double*  normal_elem_ptr;
    double*  constraint_elem_ptr;
    int    num_normals;
    int    k;
    double   x = x_in;
    double   z;
    double   hull_min_z;
    double   hull_max_z;
    double   temp;


#ifdef TEST_INTERSECTION_ROUTINES
    hull_min_z = min_z - 1.0;
    hull_max_z = max_z + 1.0;
#else
    hull_min_z = min_z;
    hull_max_z = max_z;
#endif

    num_normals = normal_mp->num_rows;
    normal_row_ptr = normal_mp->elements;
    constraint_elem_ptr = constraint_vp->elements;

    for (k=0; k<num_normals; k++)
    {
        normal_elem_ptr = *normal_row_ptr;
        temp = *constraint_elem_ptr - (*normal_elem_ptr * x);
        normal_elem_ptr++;
        temp -=  (*normal_elem_ptr)*y;

        normal_elem_ptr++; /* Third componant of normal. */

        /*
        // Expensive case is first, to avoid divide by zero later.
        */
#ifdef HOW_IT_WAS
        if (IS_ZERO_DBL(*normal_elem_ptr))
#else
        if (    (*normal_elem_ptr > -HULL_EPSILON)
             && (*normal_elem_ptr <  HULL_EPSILON)
           )
#endif
        {
            /* Plane is horizontal */

            /* REPLACED: if (temp < 0.0) */
            /* Want clean interior point */
#ifdef HOW_IT_WAS
            if (temp < DBL_EPSILON)
#else
            if (temp < HULL_EPSILON)
#endif
            {
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

            if (z < hull_max_z)
            {
                hull_max_z = z;

                /* REPLACED: if (hull_min_z > hull_max_z) */
                /* Want clean interior point */
                if (IS_NOT_LESSER_DBL(hull_min_z, hull_max_z))
                {
                    return FALSE;
                }
            }
        }
        else if (*normal_elem_ptr < 0.0)
        {
            z = temp / (*normal_elem_ptr);

            if (z > hull_min_z)
            {
                hull_min_z = z;

                /* REPLACED: if (hull_min_z > hull_max_z) */
                /* Want clean interior point */
                if (IS_NOT_LESSER_DBL(hull_min_z, hull_max_z))
                {
                    return FALSE;
                }
            }
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }


        constraint_elem_ptr++;
        normal_row_ptr++;
    }

    /* REPLACED: if (hull_min_z < hull_max_z) */
    /* Want clean interior point */
    if (IS_LESSER_DBL(hull_min_z, hull_max_z))
    {
        double d, min_distance;

#ifdef TEST_INTERSECTION_ROUTINES
        if ((hull_min_z < min_z - 0.5) || (hull_max_z > max_z + 0.5))
        {
            pso("Min or max was not changed (is_segment_in_3D_hull_intersection)!\n");
        }
#endif

        z = (hull_min_z + hull_max_z) / 2.0;

        normal_row_ptr = normal_mp->elements;
        normal_elem_ptr = *normal_row_ptr;
        constraint_elem_ptr = constraint_vp->elements;

        min_distance = *constraint_elem_ptr -
                          x * normal_elem_ptr[ 0 ] - y * normal_elem_ptr[ 1 ] -
                          z * normal_elem_ptr[ 2 ];
        normal_row_ptr++;
        constraint_elem_ptr++;

        for (k=1; k<num_normals; k++)
        {
            normal_elem_ptr = *normal_row_ptr;

            d = *constraint_elem_ptr - x * normal_elem_ptr[ 0 ] -
                       y * normal_elem_ptr[ 1 ] - z * normal_elem_ptr[ 2 ];

            if (d <min_distance) min_distance = d;

            constraint_elem_ptr++;
            normal_row_ptr++;
        }

        *distance_ptr = min_distance;

        ASSERT(min_distance >= 0.0);

        *z_ptr = z;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

