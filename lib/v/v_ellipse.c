
/* $Id: v_ellipse.c 4727 2009-11-16 20:53:54Z kobus $ */

#define USE_FANCY_ERROR

#define AS_FIND_ELLIPSES_WAS

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-2008 by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */

#include "v/v_gen.h"

#include "v/v_ellipse.h"
#include "t2/t2_segment.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static double fs_ellipse_min_axis_product = 100;
static double fs_ellipse_max_axis_product = 1000;
static double fs_ellipse_max_hull_fit_error = 0.1;
static double fs_ellipse_max_abs_radial_error = 5;
static double fs_ellipse_max_rel_radial_error = 0.2;

/* -------------------------------------------------------------------------- */

int set_image_ellipse_options(const char *option, const char *value)
{
    char   lc_option[ 100 ];
    double temp_double;
    int    result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "ellipse-min-axis-product"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("ellipse-min-axis-product = %.2f\n",
                    fs_ellipse_min_axis_product));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Min product of ellipses axis is %.2f.\n",
                    fs_ellipse_min_axis_product));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_ellipse_min_axis_product = temp_double;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "ellipse-max-axis-product"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("ellipse-max-axis-product = %.2f\n",
                    fs_ellipse_max_axis_product));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Min product of ellipses axis is %.2f.\n",
                    fs_ellipse_max_axis_product));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_ellipse_max_axis_product = temp_double;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "ellipse-max-hull-fit-error"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("ellipse-max-hull-fit-error = %.2f\n",
                    fs_ellipse_max_hull_fit_error));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Min error in fitting segment hull to ellipses is %.2f.\n",
                    fs_ellipse_max_hull_fit_error));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_ellipse_max_hull_fit_error = temp_double;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "ellipse-max-abs-radial-error"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("ellipse-max-abs-radial-error = %.2f\n",
                    fs_ellipse_max_abs_radial_error));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Min absolute radail error in ellipses is %.2f.\n",
                    fs_ellipse_max_abs_radial_error));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_ellipse_max_abs_radial_error = temp_double;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "ellipse-max-rel-radial-error"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("ellipse-max-rel-radial-error = %.2f\n",
                    fs_ellipse_max_rel_radial_error));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Min relolute radail error in ellipses is %.2f.\n",
                    fs_ellipse_max_rel_radial_error));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_ellipse_max_rel_radial_error = temp_double;
        }

        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Current ! */
#ifdef AS_FIND_ELLIPSES_WAS

int find_ellipses
(
    Segmentation_t3*  segmentation_ptr,
    Ellipse_list** ellipse_list_ptr_ptr
)
{
    int result   = NO_ERROR;
    int i, j, seg_index, count;
    Matrix*     fit_data_mp       = NULL;
    Vector*     x_vp          = NULL;
    Vector*     y_vp          = NULL;
    Vector*     xy_vp         = NULL;
    Vector*     x2_vp         = NULL;
    Vector*     y2_vp         = NULL;
    Vector*     result_vp     = NULL;
    double        error  = DBL_NOT_SET;
    Vector*     est_vp = NULL;
    double      theta;
    Vector*     ave_vp = NULL;
    Vector* one_vp = NULL;
    Vector* ellipse_vp = NULL;
    Matrix* cov_mp = NULL;
    Vector* D_vp = NULL;
    Matrix* E_mp = NULL;
    Matrix* E_trans_mp = NULL;
    Matrix* hull_mp = NULL;
    Matrix* shifted_hull_mp = NULL;
    Matrix* rotated_hull_mp = NULL;
    Matrix* shifted_boundary_mp = NULL;
    Matrix* rotated_boundary_mp = NULL;
    Hull*   outside_hp = NULL;
    Ellipse* ellipse_ptr;
    Queue_element* ellipse_queue_head = NULL;
    int num_ellipses = 0;



    /* Outside Boundaries */
    for (seg_index=0; seg_index<segmentation_ptr->num_segments; seg_index++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ seg_index ];

        if (result == ERROR) break;

        if ((seg_index + 1) % 10 == 0)
        {
            verbose_pso(10,"Processing segment %d\n", seg_index + 1);
        }

        result = get_convex_hull(&outside_hp, seg_ptr->outside_boundary_mp,
                                 DEFAULT_HULL_OPTIONS);
        if (result != ERROR)
        {
            count = 0;

            for (i = 0; i<outside_hp->facets->length; i++)
            {
                Matrix* facet_mp = outside_hp->facets->elements[ i ];
                double  x1       = facet_mp->elements[ 0 ][ 0 ];
                double  x2       = facet_mp->elements[ 1 ][ 0 ];
                double  y1       = facet_mp->elements[ 0 ][ 1 ];
                double  y2       = facet_mp->elements[ 1 ][ 1 ];
                double  x_diff   = x2 - x1;
                double  y_diff   = y2 - y1;
                double  dist     = sqrt(x_diff*x_diff + y_diff*y_diff);

                count += (int)(3*dist + 0.5);
            }
            result = get_target_matrix(&hull_mp, count, 2);
        }

        if (result != ERROR)
        {
            count = 0;

            for (i = 0; i<outside_hp->facets->length; i++)
            {
                Matrix* facet_mp   = outside_hp->facets->elements[ i ];
                double  x1         = facet_mp->elements[ 0 ][ 0 ];
                double  x2         = facet_mp->elements[ 1 ][ 0 ];
                double  y1         = facet_mp->elements[ 0 ][ 1 ];
                double  y2         = facet_mp->elements[ 1 ][ 1 ];
                double  x_diff     = x2 - x1;
                double  y_diff     = y2 - y1;
                double  dist       = sqrt(x_diff*x_diff + y_diff*y_diff);
                int     num_points = kjb_rint(3.0 * dist + 0.5);
                double  x_step     = x_diff / num_points;
                double  y_step     = y_diff / num_points;

                for (j = 0; j<num_points; j++)
                {
                    hull_mp->elements[ count ][ 0 ] = x1 + j * x_step;
                    hull_mp->elements[ count ][ 1 ] = y1 + j * y_step;
                    count++;
                }
            }

            ASSERT(count == hull_mp->num_rows);

            result = average_matrix_rows(&ave_vp, hull_mp);

            if (result != ERROR)
            {
                result = subtract_row_vector_from_matrix(&shifted_hull_mp,
                                                          hull_mp, ave_vp);
            }

            if (result != ERROR)
            {
                result = multiply_with_transpose(&cov_mp, shifted_hull_mp,
                                                 shifted_hull_mp);

                ASSERT(cov_mp->num_rows == 2);
                ASSERT(cov_mp->num_cols == 2);
            }

            if (result != ERROR)
            {
                result = diagonalize(cov_mp, &E_mp, &D_vp);
            }

            if (result != ERROR)
            {
                result = get_transpose(&E_trans_mp, E_mp);
            }

            if (result != ERROR)
            {
                result = multiply_matrices(&rotated_hull_mp,
                                           shifted_hull_mp, E_mp);
            }

            if (result != ERROR)
            {
                result = get_initialized_vector(&one_vp, count + 1, 1.0);
            }

            if (result != ERROR)
            {
                one_vp->elements[ count ] = 0;
                result = get_target_matrix(&fit_data_mp, count + 1, 2);
            }

            if (result != ERROR)
            {
                (fit_data_mp->num_rows)--;
                result = get_matrix_col(&x_vp, rotated_hull_mp, 0);
            }
            if (result != ERROR)
            {
                result = get_matrix_col(&y_vp, rotated_hull_mp, 1);
            }
            if (result != ERROR)
            {
                result = multiply_vectors(&x2_vp, x_vp, x_vp);
            }

            if (result != ERROR)
            {
                result = multiply_vectors(&y2_vp, y_vp, y_vp);
            }

            if (result != ERROR)
            {
                result = put_matrix_col(fit_data_mp, x2_vp, 0);
            }

            if (result != ERROR)
            {
                result = put_matrix_col(fit_data_mp, y2_vp, 1);
            }

            if (result != ERROR)
            {
                (fit_data_mp->num_rows)++;
                fit_data_mp->elements[ count ][ 0 ] = 1.0;
                fit_data_mp->elements[ count ][ 1 ] = -1.0;

                result = least_squares_solve(&result_vp, fit_data_mp, one_vp);
            }

            if (result != ERROR)
            {
                result = multiply_matrix_and_vector(&est_vp, fit_data_mp,
                                                    result_vp);
            }

            if (result != ERROR)
            {
                error = 0.0;

                for (i = 0; i<count; i++)
                {
                    double dist_sqr = ABS_OF(est_vp->elements[ i ]);
                    double dist = sqrt(dist_sqr);
                    double diff = ABS_OF(dist - 1.0);

                    error += diff;
                }

                error /= count;

                /* dbf(error);  */
            }

            if (    (result != ERROR)
                 && (ABS_OF(error) < fs_ellipse_max_hull_fit_error)
               )
            {
                double a = result_vp->elements[0];
                double b = result_vp->elements[1];
                double root_a = sqrt(a);
                double root_b = sqrt(b);
                double ratio = MAX_OF(root_a / root_b, root_b / root_a);
                double product = 1.0 / (root_a * root_b);
                double radial_error = 0.0;
                double rel_radial_error = 0.0;
#ifdef USE_FANCY_ERROR
                int ray_count = 0;

                error *= sqrt(ratio);

                if (    (result != ERROR)
                     && (product < fs_ellipse_max_axis_product)
                     && (product > fs_ellipse_min_axis_product)
                     && (ratio < 5.0)
                   )
                {
                    result = subtract_row_vector_from_matrix(
                                                          &shifted_boundary_mp,
                                                  seg_ptr->outside_boundary_mp,
                                                  ave_vp);

                    if (result != ERROR)
                    {
                        result = multiply_matrices(&rotated_boundary_mp,
                                                   shifted_boundary_mp, E_mp);
                    }

                    for (theta=0.0; theta < M_PI; theta += 0.3)
                    {
                        int b_len;
                        double min_1 = DBL_NOT_SET;
                        double min_2 = DBL_NOT_SET;
                        int  min_1_i, min_2_i;
                        double ct = cos(theta);
                        double st = sin(theta);
                        double ex1 = ct / root_a;
                        double ey1 = st / root_b;
                        double ex2 = -ex1;
                        double ey2 = -ey1;
                        double e_dist = sqrt(ex1*ex1 + ey1*ey1);
                        double d_temp;


                        if (result == ERROR) break;

                        b_len = rotated_boundary_mp->num_rows;

                        for (i=0; i<b_len; i++)
                        {
                            int next = (i == (b_len - 1)) ? 0 : i + 1;
                            double x1 = rotated_boundary_mp->elements[ i ][ 0 ];
                            double x2 = rotated_boundary_mp->elements[ next ][ 0 ];
                            double y1 = rotated_boundary_mp->elements[ i ][ 1 ];
                            double y2 = rotated_boundary_mp->elements[ next ][ 1 ];
                            double d1 = x1*st - y1*ct;
                            double d2 = x2*st - y2*ct;
                            double s = d1 * d2;

                            if (s < 0.0) /* Sign change */
                            {
                                double d_x1_ex1 = x1 - ex1;
                                double d_x1_ex2 = x1 - ex2;
                                double d_x2_ex1 = x2 - ex1;
                                double d_x2_ex2 = x2 - ex2;
                                double d_y1_ey1 = y1 - ey1;
                                double d_y1_ey2 = y1 - ey2;
                                double d_y2_ey1 = y2 - ey1;
                                double d_y2_ey2 = y2 - ey2;
                                double d_x1_ex1_sqr = d_x1_ex1 * d_x1_ex1;
                                double d_x1_ex2_sqr = d_x1_ex2 * d_x1_ex2;
                                double d_x2_ex1_sqr = d_x2_ex1 * d_x2_ex1;
                                double d_x2_ex2_sqr = d_x2_ex2 * d_x2_ex2;
                                double d_y1_ey1_sqr = d_y1_ey1 * d_y1_ey1;
                                double d_y1_ey2_sqr = d_y1_ey2 * d_y1_ey2;
                                double d_y2_ey1_sqr = d_y2_ey1 * d_y2_ey1;
                                double d_y2_ey2_sqr = d_y2_ey2 * d_y2_ey2;
                                double d_1_e1 = d_x1_ex1_sqr + d_y1_ey1_sqr;
                                double d_1_e2 = d_x1_ex2_sqr + d_y1_ey2_sqr;
                                double d_2_e1 = d_x2_ex1_sqr + d_y2_ey1_sqr;
                                double d_2_e2 = d_x2_ex2_sqr + d_y2_ey2_sqr;

                                if ((min_1 < 0.0) || (d_1_e1 < min_1))
                                {
                                    min_1 = d_1_e1;
                                    min_1_i = i;
                                }

                                if (d_2_e1 < min_1)
                                {
                                    min_1 = d_2_e1;
                                    min_1_i = i;
                                }

                                if ((min_2 < 0.0) || (d_1_e2 < min_2))
                                {
                                    min_2 = d_1_e2;
                                    min_2_i = i;
                                }

                                if (d_2_e2 < min_2)
                                {
                                    min_2 = d_2_e2;
                                    min_2_i = i;
                                }

                            }
                        }

                        d_temp = sqrt(min_1);
                        rel_radial_error += d_temp / e_dist;
                        radial_error += d_temp;

                        d_temp = sqrt(min_2);
                        rel_radial_error += d_temp / e_dist;
                        radial_error += d_temp;

                        ray_count += 2;

                        /*
                        image_draw_point(out_ip,
                          (int)(seg_ptr->outside_boundary_mp->elements[ min_1_i ][ 0 ]),
                          (int)(seg_ptr->outside_boundary_mp->elements[ min_1_i ][ 1 ]),
                          0, seg_index+1, (int)(200 * sqrt(min_1) / e_dist), 255);

                        image_draw_point(out_ip,
                          (int)(seg_ptr->outside_boundary_mp->elements[ min_2_i ][ 0 ]),
                          (int)(seg_ptr->outside_boundary_mp->elements[ min_2_i ][ 1 ]),
                          0, seg_index+1, (int)(200 * sqrt(min_2) / e_dist), 255);
                        */
                    }

                    radial_error /= ray_count;
                    radial_error *= sqrt(ratio);

                    rel_radial_error /= ray_count;
                    rel_radial_error *= sqrt(ratio);
                }
                else
                {
                    radial_error = DBL_MAX;
                    rel_radial_error = DBL_MAX;
                }
#endif

                if (    (result != ERROR)
                     && (rel_radial_error < fs_ellipse_max_rel_radial_error)
                     && (radial_error < fs_ellipse_max_abs_radial_error)
                   )
                {
                    if ((ellipse_ptr = TYPE_MALLOC(Ellipse)) == NULL)
                    {
                        result = ERROR;
                    }

                    if (result != ERROR)
                    {
                        ellipse_ptr->a = a;
                        ellipse_ptr->b = b;
                        ellipse_ptr->max_x = 1.0 / root_a;
                        ellipse_ptr->max_y = 1.0 / root_b;
                        ellipse_ptr->offset_vp = NULL;
                        ellipse_ptr->rotation_mp = NULL;
                        ellipse_ptr->error_1 = error;
                        ellipse_ptr->error_2 = rel_radial_error;
                        ellipse_ptr->error_3 = radial_error;
                        ellipse_ptr->r_chrom_ave = seg_ptr->r_chrom_ave;
                        ellipse_ptr->g_chrom_ave = seg_ptr->g_chrom_ave;
                        ellipse_ptr->R_ave = seg_ptr->R_ave;
                        ellipse_ptr->G_ave = seg_ptr->G_ave;
                        ellipse_ptr->B_ave = seg_ptr->B_ave;
                        ellipse_ptr->seg_index = seg_index;
                    }

                    if (result != ERROR)
                    {
                        result = copy_vector(&(ellipse_ptr->offset_vp),
                                             ave_vp);
                    }

                    if (result != ERROR)
                    {
                        result = copy_matrix(&(ellipse_ptr->rotation_mp),
                                             E_mp);
                    }

                    if (result != ERROR)
                    {
                        result = insert_into_queue(&ellipse_queue_head,
                                                   (Queue_element**)NULL,
                                                   (void*)ellipse_ptr);
                    }

                    num_ellipses++;
                }
            }
        }
    }

    if ((result != ERROR) && (num_ellipses > 0))
    {
        result = get_target_ellipse_list(ellipse_list_ptr_ptr,
                                         num_ellipses);
    }

    if ((result != ERROR) && (num_ellipses > 0))
    {
        Queue_element* curr_elem = ellipse_queue_head;
        Queue_element* elem_to_free;

        for (i=0; i<num_ellipses; i++)
        {
            free_ellipse((*ellipse_list_ptr_ptr)->ellipses[ i ]);

            (*ellipse_list_ptr_ptr)->ellipses[ i ] =
                                               (Ellipse*)(curr_elem->contents);
            elem_to_free = curr_elem;
            curr_elem = curr_elem->next;

            kjb_free(elem_to_free);
        }
    }

    free_hull(outside_hp);
    free_matrix(hull_mp);
    free_matrix(shifted_hull_mp);
    free_matrix(rotated_hull_mp);
    free_matrix(shifted_boundary_mp);
    free_matrix(rotated_boundary_mp);
    free_matrix(fit_data_mp);
    free_matrix(cov_mp);
    free_matrix(E_mp);
    free_matrix(E_trans_mp);
    free_vector(D_vp);
    free_vector(ellipse_vp);
    free_vector(x_vp);
    free_vector(y_vp);
    free_vector(xy_vp);
    free_vector(x2_vp);
    free_vector(y2_vp);
    free_vector(result_vp);
    free_vector(est_vp);
    free_vector(ave_vp);
    free_vector(one_vp);

    if (result == ERROR)
    {
        return result;
    }
    else
    {
        return num_ellipses;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#else

int find_ellipses
(
    Segmentation_t3*  segmentation_ptr,
    Ellipse_list** ellipse_list_ptr_ptr
)
{
    int result   = NO_ERROR;
    int i, seg_index;
    Ellipse* ellipse_ptr;
    Queue_element* ellipse_queue_head = NULL;
    int num_ellipses = 0;



    /* Outside Boundaries */
    for (seg_index=0; seg_index<segmentation_ptr->num_segments; seg_index++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ seg_index ];

        if (result == ERROR) break;

        if ((seg_index + 1) % 10 == 0)
        {
            verbose_pso(10,"Processing segment %d\n", seg_index + 1);
        }

        ellipse_ptr = fit_ellipse(seg_ptr->outside_boundary_mp,
                                  fs_ellipse_min_axis_product,
                                  fs_ellipse_max_axis_product,
                                  fs_ellipse_max_hull_fit_error,
                                  fs_ellipse_max_abs_radial_error,
                                  fs_ellipse_max_rel_radial_error);
        if (ellipse_ptr != NULL)
        {
            ellipse_ptr->id = seg_index + 1;

            result = insert_into_queue(&ellipse_queue_head,
                                       (Queue_element**)NULL,
                                       (void*)ellipse_ptr);
            num_ellipses++;
        }
    }

    if ((result != ERROR) && (num_ellipses > 0))
    {
        result = get_target_ellipse_list(ellipse_list_ptr_ptr,
                                         num_ellipses);
    }

    if ((result != ERROR) && (num_ellipses > 0))
    {
        Queue_element* curr_elem = ellipse_queue_head;
        Queue_element* elem_to_free;

        for (i=0; i<num_ellipses; i++)
        {
            free_ellipse((*ellipse_list_ptr_ptr)->ellipses[ i ]);

            (*ellipse_list_ptr_ptr)->ellipses[ i ] =
                                               (Ellipse*)(curr_elem->contents);
            elem_to_free = curr_elem;
            curr_elem = curr_elem->next;

            kjb_free(elem_to_free);
        }
    }

    if (result == ERROR)
    {
        return result;
    }
    else
    {
        return num_ellipses;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Ellipse* fit_ellipse
(
    Matrix* outside_boundary_mp,
    double  min_axis_product,
    double  max_axis_product,
    double  max_hull_fit_error,
    double  max_abs_radial_error,
    double  max_rel_radial_error
)
{
    int     result      = NO_ERROR;
    int     i, j;
    int     count;
    Matrix* fit_data_mp = NULL;
    Vector* x_vp        = NULL;
    Vector* y_vp        = NULL;
    Vector* xy_vp       = NULL;
    Vector* x2_vp       = NULL;
    Vector* y2_vp       = NULL;
    Vector* result_vp   = NULL;
    double  error       = DBL_NOT_SET;
    Vector* est_vp      = NULL;
    double  theta;
    Vector* ave_vp      = NULL;
    Vector* one_vp      = NULL;
    Vector* ellipse_vp  = NULL;
    Matrix* cov_mp      = NULL;
    Vector* D_vp        = NULL;
    Matrix* E_mp        = NULL;
    Matrix* E_trans_mp  = NULL;
    Matrix* hull_mp     = NULL;
    Matrix*  shifted_hull_mp     = NULL;
    Matrix*  rotated_hull_mp     = NULL;
    Matrix*  shifted_boundary_mp = NULL;
    Matrix*  rotated_boundary_mp = NULL;
    Hull*    outside_hp          = NULL;
    Ellipse* ellipse_ptr         = NULL;


    result = get_convex_hull(&outside_hp, outside_boundary_mp,
                             DEFAULT_HULL_OPTIONS);

    if (result != ERROR)
    {
        count = 0;

        for (i = 0; i<outside_hp->facets->length; i++)
        {
            Matrix* facet_mp = outside_hp->facets->elements[ i ];
            double  x1       = facet_mp->elements[ 0 ][ 0 ];
            double  x2       = facet_mp->elements[ 1 ][ 0 ];
            double  y1       = facet_mp->elements[ 0 ][ 1 ];
            double  y2       = facet_mp->elements[ 1 ][ 1 ];
            double  x_diff   = x2 - x1;
            double  y_diff   = y2 - y1;
            double  dist     = sqrt(x_diff*x_diff + y_diff*y_diff);

            count += (int)(3.0 * dist + 0.5);
        }
        result = get_target_matrix(&hull_mp, count, 2);
    }

    if (result != ERROR)
    {
        count = 0;

        for (i = 0; i<outside_hp->facets->length; i++)
        {
            Matrix* facet_mp   = outside_hp->facets->elements[ i ];
            double  x1         = facet_mp->elements[ 0 ][ 0 ];
            double  x2         = facet_mp->elements[ 1 ][ 0 ];
            double  y1         = facet_mp->elements[ 0 ][ 1 ];
            double  y2         = facet_mp->elements[ 1 ][ 1 ];
            double  x_diff     = x2 - x1;
            double  y_diff     = y2 - y1;
            double  dist       = sqrt(x_diff*x_diff + y_diff*y_diff);
            int     num_points = (3 * dist + 0.5);
            double  x_step     = x_diff / num_points;
            double  y_step     = y_diff / num_points;

            for (j = 0; j<num_points; j++)
            {
                hull_mp->elements[ count ][ 0 ] = x1 + j * x_step;
                hull_mp->elements[ count ][ 1 ] = y1 + j * y_step;
                count++;
            }
        }

        ASSERT(count == hull_mp->num_rows);

        result = average_matrix_rows(&ave_vp, hull_mp);

        if (result != ERROR)
        {
            result = subtract_row_vector_from_matrix(&shifted_hull_mp,
                                                      hull_mp, ave_vp);
        }

        if (result != ERROR)
        {
            result = multiply_with_transpose(&cov_mp, shifted_hull_mp,
                                             shifted_hull_mp);

            ASSERT(cov_mp->num_rows == 2);
            ASSERT(cov_mp->num_cols == 2);
        }

        if (result != ERROR)
        {
            result = diagonalize(cov_mp, &E_mp, &D_vp);
        }

        if (result != ERROR)
        {
            result = get_transpose(&E_trans_mp, E_mp);
        }

        if (result != ERROR)
        {
            result = multiply_matrices(&rotated_hull_mp,
                                       shifted_hull_mp, E_mp);
        }

        if (result != ERROR)
        {
            result = get_initialized_vector(&one_vp, count + 1, 1.0);
        }

        if (result != ERROR)
        {
            one_vp->elements[ count ] = 0;
            result = get_target_matrix(&fit_data_mp, count + 1, 2);
        }

        if (result != ERROR)
        {
            (fit_data_mp->num_rows)--;
            result = get_matrix_col(&x_vp, rotated_hull_mp, 0);
        }
        if (result != ERROR)
        {
            result = get_matrix_col(&y_vp, rotated_hull_mp, 1);
        }
        if (result != ERROR)
        {
            result = multiply_vectors(&x2_vp, x_vp, x_vp);
        }

        if (result != ERROR)
        {
            result = multiply_vectors(&y2_vp, y_vp, y_vp);
        }

        if (result != ERROR)
        {
            result = put_matrix_col(fit_data_mp, x2_vp, 0);
        }

        if (result != ERROR)
        {
            result = put_matrix_col(fit_data_mp, y2_vp, 1);
        }

        if (result != ERROR)
        {
            (fit_data_mp->num_rows)++;
            fit_data_mp->elements[ count ][ 0 ] = 1.0;
            fit_data_mp->elements[ count ][ 1 ] = -1.0;

            result = least_squares_solve(&result_vp, fit_data_mp, one_vp);
        }

        if (result != ERROR)
        {
            result = multiply_matrix_and_vector(&est_vp, fit_data_mp,
                                                result_vp);
        }

        if (result != ERROR)
        {
            error = 0.0;

            for (i = 0; i<count; i++)
            {
                double dist_sqr = ABS_OF(est_vp->elements[ i ]);
                double dist = sqrt(dist_sqr);
                double diff = ABS_OF(dist - 1.0);

                error += diff;
            }

            error /= count;

            /* dbf(error);  */
        }

        if (    (result != ERROR)
             && (ABS_OF(error) < max_hull_fit_error)
           )
        {
            double a = result_vp->elements[0];
            double b = result_vp->elements[1];
            double root_a = sqrt(a);
            double root_b = sqrt(b);
            double ratio = MAX_OF(root_a / root_b, root_b / root_a);
            double product = 1.0 / (root_a * root_b);
            double radial_error = 0.0;
            double rel_radial_error = 0.0;
#ifdef USE_FANCY_ERROR
            int ray_count = 0;

            error *= sqrt(ratio);

            if (    (result != ERROR)
                 && (product < max_axis_product)
                 && (product > min_axis_product)
                 && (ratio < 5.0)
               )
            {
                result = subtract_row_vector_from_matrix(&shifted_boundary_mp,
                                                          outside_boundary_mp,
                                                          ave_vp);

                if (result != ERROR)
                {
                    result = multiply_matrices(&rotated_boundary_mp,
                                               shifted_boundary_mp, E_mp);
                }

                for (theta=0.0; theta < M_PI; theta += 0.3)
                {
                    int b_len;
                    double min_1 = DBL_NOT_SET;
                    double min_2 = DBL_NOT_SET;
                    int  min_1_i, min_2_i;
                    double ct = cos(theta);
                    double st = sin(theta);
                    double ex1 = ct / root_a;
                    double ey1 = st / root_b;
                    double ex2 = -ex1;
                    double ey2 = -ey1;
                    double e_dist = sqrt(ex1*ex1 + ey1*ey1);
                    double d_temp;


                    if (result == ERROR) break;

                    b_len = rotated_boundary_mp->num_rows;

                    for (i=0; i<b_len; i++)
                    {
                        int next = (i == (b_len - 1)) ? 0 : i + 1;
                        double x1 = rotated_boundary_mp->elements[ i ][ 0 ];
                        double x2 = rotated_boundary_mp->elements[ next ][ 0 ];
                        double y1 = rotated_boundary_mp->elements[ i ][ 1 ];
                        double y2 = rotated_boundary_mp->elements[ next ][ 1 ];
                        double d1 = x1*st - y1*ct;
                        double d2 = x2*st - y2*ct;
                        double s = d1 * d2;

                        if (s < 0.0) /* Sign change */
                        {
                            double d_x1_ex1 = x1 - ex1;
                            double d_x1_ex2 = x1 - ex2;
                            double d_x2_ex1 = x2 - ex1;
                            double d_x2_ex2 = x2 - ex2;
                            double d_y1_ey1 = y1 - ey1;
                            double d_y1_ey2 = y1 - ey2;
                            double d_y2_ey1 = y2 - ey1;
                            double d_y2_ey2 = y2 - ey2;
                            double d_x1_ex1_sqr = d_x1_ex1 * d_x1_ex1;
                            double d_x1_ex2_sqr = d_x1_ex2 * d_x1_ex2;
                            double d_x2_ex1_sqr = d_x2_ex1 * d_x2_ex1;
                            double d_x2_ex2_sqr = d_x2_ex2 * d_x2_ex2;
                            double d_y1_ey1_sqr = d_y1_ey1 * d_y1_ey1;
                            double d_y1_ey2_sqr = d_y1_ey2 * d_y1_ey2;
                            double d_y2_ey1_sqr = d_y2_ey1 * d_y2_ey1;
                            double d_y2_ey2_sqr = d_y2_ey2 * d_y2_ey2;
                            double d_1_e1 = d_x1_ex1_sqr + d_y1_ey1_sqr;
                            double d_1_e2 = d_x1_ex2_sqr + d_y1_ey2_sqr;
                            double d_2_e1 = d_x2_ex1_sqr + d_y2_ey1_sqr;
                            double d_2_e2 = d_x2_ex2_sqr + d_y2_ey2_sqr;

                            if ((min_1 < 0.0) || (d_1_e1 < min_1))
                            {
                                min_1 = d_1_e1;
                                min_1_i = i;
                            }

                            if (d_2_e1 < min_1)
                            {
                                min_1 = d_2_e1;
                                min_1_i = i;
                            }

                            if ((min_2 < 0.0) || (d_1_e2 < min_2))
                            {
                                min_2 = d_1_e2;
                                min_2_i = i;
                            }

                            if (d_2_e2 < min_2)
                            {
                                min_2 = d_2_e2;
                                min_2_i = i;
                            }

                        }
                    }

                    d_temp = sqrt(min_1);
                    rel_radial_error += d_temp / e_dist;
                    radial_error += d_temp;

                    d_temp = sqrt(min_2);
                    rel_radial_error += d_temp / e_dist;
                    radial_error += d_temp;

                    ray_count += 2;
                }

                radial_error /= ray_count;
                radial_error *= sqrt(ratio);

                rel_radial_error /= ray_count;
                rel_radial_error *= sqrt(ratio);
            }
            else
            {
                radial_error = DBL_MAX;
                rel_radial_error = DBL_MAX;
            }
#endif

            if (    (result != ERROR)
                 && (rel_radial_error < max_rel_radial_error)
                 && (radial_error < max_abs_radial_error)
               )
            {
                if ((ellipse_ptr = TYPE_MALLOC(Ellipse)) == NULL)
                {
                    result = ERROR;
                }

                if (result != ERROR)
                {
                    ellipse_ptr->a = a;
                    ellipse_ptr->a = b;
                    ellipse_ptr->max_x = 1.0 / root_a;
                    ellipse_ptr->max_y = 1.0 / root_b;
                    ellipse_ptr->offset_vp = NULL;
                    ellipse_ptr->rotation_mp = NULL;
                    ellipse_ptr->error_1 = error;
                    ellipse_ptr->error_2 = rel_radial_error;
                    ellipse_ptr->error_3 = radial_error;
                }

                if (result != ERROR)
                {
                    result = copy_vector(&(ellipse_ptr->offset_vp),
                                         ave_vp);
                }

                if (result != ERROR)
                {
                    result = copy_matrix(&(ellipse_ptr->rotation_mp),
                                         E_mp);
                }
            }
        }
    }


    free_hull(outside_hp);
    free_matrix(hull_mp);
    free_matrix(shifted_hull_mp);
    free_matrix(rotated_hull_mp);
    free_matrix(shifted_boundary_mp);
    free_matrix(rotated_boundary_mp);
    free_matrix(fit_data_mp);
    free_matrix(cov_mp);
    free_matrix(E_mp);
    free_matrix(E_trans_mp);
    free_vector(D_vp);
    free_vector(ellipse_vp);
    free_vector(x_vp);
    free_vector(y_vp);
    free_vector(xy_vp);
    free_vector(x2_vp);
    free_vector(y2_vp);
    free_vector(result_vp);
    free_vector(est_vp);
    free_vector(ave_vp);
    free_vector(one_vp);

    if (result == ERROR)
    {
        return NULL;
    }
    else
    {
        return ellipse_ptr;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef AS_IT_WAS
int image_draw_ellipse(KJB_image* out_ip, Ellipse* ellipse_ptr)
{
    double theta;
    Vector* canon_ellipse_vp = NULL;
    Vector* ellipse_vp       = NULL;
    double  max_x           = ellipse_ptr->max_x;
    double  max_y           = ellipse_ptr->max_y;
    Matrix* rotation_mp     = ellipse_ptr->rotation_mp;
    Vector* offset_vp       = ellipse_ptr->offset_vp;
    int     result          = NO_ERROR;
    int     i, j;

    ERE(get_target_vector(&canon_ellipse_vp, 2));

    for (theta=0.0; theta <= 2.0 * M_PI; theta += 0.0002)
    {
        canon_ellipse_vp->elements[ 0 ] = cos(theta) * max_x;
        canon_ellipse_vp->elements[ 1 ] = sin(theta) * max_y;

        if (result == ERROR) break;

        result = multiply_matrix_and_vector(&ellipse_vp, rotation_mp,
                                            canon_ellipse_vp);

        if (result != ERROR)
        {
            result = ow_add_vectors(ellipse_vp, offset_vp);
        }

        if (result != ERROR)
        {
            i = (ellipse_vp->elements[ 0 ] + 0.5);
            j = (ellipse_vp->elements[ 1 ] + 0.5);

            if (    (i >= 0) && (i < out_ip->num_rows)
                 && (j >= 0) && (j < out_ip->num_cols)
               )
            {
                float r = MIN_OF(255, 5.0 / ellipse_ptr->error_1);
                float g = MIN_OF(255, 25.0  / ellipse_ptr->error_2);
                float b = MIN_OF(255, 250.0 / ellipse_ptr->error_3);

                out_ip->pixels[ i ][ j ].r = r;
                out_ip->pixels[ i ][ j ].g = g;
                out_ip->pixels[ i ][ j ].b = b;
            }
        }
    }

    free_vector(canon_ellipse_vp);
    free_vector(ellipse_vp);

    return NO_ERROR;
}
#else
int image_draw_ellipse(KJB_image *out_ip, Ellipse *ellipse_ptr)
{
    int r = MIN_OF(255, (int)(5.0 / ellipse_ptr->error_1));
    int g = MIN_OF(255, (int)(25.0  / ellipse_ptr->error_2));
    int b = MIN_OF(255, (int)(250.0 / ellipse_ptr->error_3));

    return image_draw_ellipse_2(out_ip, ellipse_ptr, r, g, b);
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int image_draw_ellipse_2
(
    KJB_image* out_ip,
    Ellipse*   ellipse_ptr,
    int        r,
    int        g,
    int        b
)
{
    double theta;
    Vector* canon_ellipse_vp = NULL;
    Vector* ellipse_vp       = NULL;
    double  max_x           = ellipse_ptr->max_x;
    double  max_y           = ellipse_ptr->max_y;
    Matrix* rotation_mp     = ellipse_ptr->rotation_mp;
    Vector* offset_vp       = ellipse_ptr->offset_vp;
    int     result          = NO_ERROR;
    int     i, j;

    ERE(get_target_vector(&canon_ellipse_vp, 2));

    for (theta=0.0; theta <= 2.0 * M_PI; theta += 0.0002)
    {
        canon_ellipse_vp->elements[ 0 ] = cos(theta) * max_x;
        canon_ellipse_vp->elements[ 1 ] = sin(theta) * max_y;

        if (result == ERROR) break;

        result = multiply_matrix_and_vector(&ellipse_vp, rotation_mp,
                                            canon_ellipse_vp);

        if (result != ERROR)
        {
            result = ow_add_vectors(ellipse_vp, offset_vp);
        }

        if (result != ERROR)
        {
            i = kjb_rint(ellipse_vp->elements[ 0 ] + 0.5);
            j = kjb_rint(ellipse_vp->elements[ 1 ] + 0.5);

            if (    (i >= 0) && (i < out_ip->num_rows)
                 && (j >= 0) && (j < out_ip->num_cols)
               )
            {
                out_ip->pixels[ i ][ j ].r = r;
                out_ip->pixels[ i ][ j ].g = g;
                out_ip->pixels[ i ][ j ].b = b;
            }
        }
    }

    free_vector(canon_ellipse_vp);
    free_vector(ellipse_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

