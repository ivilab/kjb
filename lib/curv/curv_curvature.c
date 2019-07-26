
/* $Id: curv_curvature.c 21448 2017-06-28 22:00:33Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               |
|        Kobus Barnard                                                         |
|        Amy Platt                                                             |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#include "i/i_incl.h"
#include "i2/i2_incl.h"
#include "m/m_incl.h"
#include "n/n_invert.h"
#include "cgi_draw/cgi_draw.h"
#include "curv/curv_curvature.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define CURVE_IMAGE_RESOLUTION   300


#define LINELEN 16
#define MIN_BUFFER_LENGTH 8


#define SYMMETRIC_FIT
#define BALANCE_WEIGHT

/*
#define ITERATIVE_FIT
*/

#define ABS_CURVATURE   /* No future in anything else. */
/*
#define WEIGHT_OBS
*/


#define NEW_CURVATURE_FORMULA

/*
#define WEIGHT_BY_FIT
#define NEW_WEIGHT_BY_FIT
*/
/*
*/
/*
*/


/* -------------------------------------------------------------------------- */

int fs_search_directions[CURV_NUM_DIRECTIONS] = { 0, 1, 7, 2, 6, 3, 5, 4 };

/* for passing around the fs_points to solve the equation with */
ALP_Point fs_points[LINELEN*2 + 1];

static int  fs_line_len = LINELEN;

/* -------------------------------------------------------------------------- */

static int        curvature_at_point
(
    const Int_matrix* on_mp,
    int         i,
    int         j,
    double*     weight,
    double*     point_kappa_ptr,
    const KJB_image* in_ip,
    KJB_image** out_ipp,
    const char* file
);

static int curvature_coeffs_at_point
(
    const Int_matrix* on_mp,
    int               i,
    int               j,
    double*           weight,
    double*           point_kappa_ptr,
    Vector*           tangent_vector,
    const KJB_image*  in_ip,
    KJB_image**       out_ipp,
    const char*       file
);

static void       get_neighbors
(
    const Int_matrix* on_mp,
    int        i,
    int        j,
    int*       pos,
    int*       neg,
    double*    weight
);

static int        get_neighbors_in_dir
(
    const Int_matrix* on_mp,
    int        i,
    int        j,
    int        start_dir,
    int        depth
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double image_curvature
(
    const KJB_image*  in_ip,
    Matrix**    curvature_mpp,
    int         num_output_images,
    KJB_image** out_ip_list,
    const char* file
)
{
    int             i;
    int             j;
    int             num_rows            = in_ip->num_rows;
    int             num_cols            = in_ip->num_cols;
    KJB_image*      output              = NULL;
    double          weight              = DBL_NOT_SET;
    double          point_kappa         = DBL_NOT_SET;
    double          total_curvature     = 0.0;
    double          total_length        = 0.0;
    Indexed_vector* sorted_curvature_vp = NULL;
    Int_matrix*     coord_index_mp      = NULL;
    int             point_count         = 0;
    Int_matrix*     on_mp               = NULL;


    EPETE(get_zero_int_matrix(&on_mp, num_rows, num_cols));

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (IS_NEURON_PIXEL(in_ip, i, j ))
            {
                on_mp->elements[ i ][ j ] = TRUE;
            }
        }
    }

    EPETE(thin_pixels_not_needed_for_contiguity(on_mp));

    EPETE(get_target_indexed_vector(&sorted_curvature_vp, num_rows*num_cols));
    EPETE(get_target_int_matrix(&coord_index_mp, num_rows*num_cols, 2));

    total_curvature = 0.0;
    total_length = 0.0;

    EPETE(get_zero_image(&output, num_rows, num_cols));

    EPETE(get_initialized_matrix(curvature_mpp, num_rows, num_cols,
                                 DBL_NOT_SET));

    fs_line_len = LINELEN;

    for(i = 0; i < in_ip->num_rows; i++)
    {
        for(j = 0; j < in_ip->num_cols; j++)
        {
            if (PIXEL_IS_ON(on_mp, i, j))
            {
                int res = curvature_at_point(on_mp, i, j, &weight, &point_kappa,
                                             in_ip, NULL, file);

                if (res == NO_ERROR)
                {
                    (*curvature_mpp)->elements[i][j] = point_kappa;

                    /*
                    p_stderr("%03d  %03d  %e   %e   %e\n", i, j, point_kappa, weight, weight*point_kappa);
                     */

                    total_curvature += weight*point_kappa;
                    total_length += weight;

                    if (num_output_images > 0)
                    {
                        sorted_curvature_vp->elements[ point_count ].element = point_kappa;
                        coord_index_mp->elements[ point_count ][ 0 ] = i;
                        coord_index_mp->elements[ point_count ][ 1 ] = j;
                        point_count++;
                    }
                }
                else if (res == WRONG_NUMBER_OF_NEIGHBORS)
                {
                    (*curvature_mpp)->elements[i][j] = 2.0 * DBL_NOT_SET;
                }
                else if (res == NOT_LONG_ENOUGH)
                {
                    (*curvature_mpp)->elements[i][j] = 3.0 * DBL_NOT_SET;
                }
                else if (res == DEGENERATE_SYSTEM)
                {
                    (*curvature_mpp)->elements[i][j] = 4.0 * DBL_NOT_SET;
                }
            }
            else if (IS_NEURON_PIXEL(in_ip, i, j))
            {
                /* Thinned pixel. */
                (*curvature_mpp)->elements[i][j] = 5.0 * DBL_NOT_SET;
            }
        }
    }

    sorted_curvature_vp->length = point_count;
    coord_index_mp->num_rows = point_count;

    /*
     * Only true if the number of output images is greater than zero. 
    */
    if (point_count > 0)
    {
        int  count;
        char title[ 1000 ];
        int index;
        int point;
        int i, j;

        ERE(descend_sort_indexed_vector(sorted_curvature_vp));

        for (count = 0; count < num_output_images - 1; count++)
        {
            index = count * point_count / (num_output_images - 1);
            point = sorted_curvature_vp->elements[ index ].index;
            i = coord_index_mp->elements[ point ][ 0 ];
            j = coord_index_mp->elements[ point ][ 1 ];

            ERE(curvature_at_point(on_mp, i, j, &weight, &point_kappa,
                                   in_ip, &(out_ip_list[ count ]), file));
            ERE(kjb_sprintf(title, sizeof(title), "%d-%.3f", count,
                            (*curvature_mpp)->elements[ i ][ j ]));
        }

        point = sorted_curvature_vp->elements[ point_count - 1 ].index;
        i = coord_index_mp->elements[ point ][ 0 ];
        j = coord_index_mp->elements[ point ][ 1 ];

        ERE(curvature_at_point(on_mp, i, j, &weight, &point_kappa,
                               in_ip, &(out_ip_list[ num_output_images - 1 ]), file));

        ERE(kjb_sprintf(title, sizeof(title), "%d-%.3f", num_output_images,
                        (*curvature_mpp)->elements[ i ][ j ]));
    }

    kjb_free_image(output);

    free_indexed_vector(sorted_curvature_vp);
    free_int_matrix(coord_index_mp);

    free_int_matrix(on_mp);

    return  total_curvature/total_length;
}

double image_curvature_tangent_weight
(
    const KJB_image*  in_ip,
    Matrix**    curvature_mpp,
    Matrix**    curvature_tangent_weight_vector_mpp,
    int         num_output_images,
    KJB_image** out_ip_list,
    const char* file
)
{
    int             i;
    int             j;
    int             num_rows            = in_ip->num_rows;
    int             num_cols            = in_ip->num_cols;
    KJB_image*      output              = NULL;
    double          weight              = DBL_NOT_SET;
    double          point_kappa         = DBL_NOT_SET;
    double          total_curvature     = 0.0;
    double          total_length        = 0.0;
    Indexed_vector* sorted_curvature_vp = NULL;
    Int_matrix*     coord_index_mp      = NULL;
    int             point_count         = 0;
    Int_matrix*     on_mp               = NULL;
    Vector*         tangent_vector      = NULL;


    EPETE(get_zero_int_matrix(&on_mp, num_rows, num_cols));

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (IS_NEURON_PIXEL(in_ip, i, j ))
            {
                on_mp->elements[ i ][ j ] = TRUE;
            }
        }
    }

    EPETE(thin_pixels_not_needed_for_contiguity(on_mp));

    EPETE(get_target_indexed_vector(&sorted_curvature_vp, num_rows*num_cols));
    EPETE(get_target_int_matrix(&coord_index_mp, num_rows*num_cols, 2));

    total_curvature = 0.0;
    total_length = 0.0;

    EPETE(get_zero_image(&output, num_rows, num_cols));

    EPETE(get_initialized_matrix(curvature_mpp, num_rows, num_cols,
                                 DBL_NOT_SET));
    EPETE(get_initialized_matrix(curvature_tangent_weight_vector_mpp, 
                                 num_rows*num_cols, 
                                 3,
                                 0.0));
    EPETE(get_target_vector(&tangent_vector, 2));
    fs_line_len = LINELEN;

    for(i = 0; i < in_ip->num_rows; i++)
    {
        for(j = 0; j < in_ip->num_cols; j++)
        {
            if (PIXEL_IS_ON(on_mp, i, j))
            {
                
                /* int res = curvature_at_point(on_mp, i, j, &weight, &point_kappa, */
                /*                              in_ip, NULL, file); */
                int res = curvature_coeffs_at_point(on_mp, i, j, &weight, &point_kappa,
                                                    tangent_vector,
                                                    in_ip, NULL, file);


                if (res == NO_ERROR)
                {
                  double dx, dy;
                  dx = tangent_vector->elements[0];
                  dy = tangent_vector->elements[1];

                    (*curvature_mpp)->elements[i][j] = point_kappa;
                    (*curvature_tangent_weight_vector_mpp)->elements[i*num_cols + j][0] = dx;
                    (*curvature_tangent_weight_vector_mpp)->elements[i*num_cols + j][1] = dy;
                    (*curvature_tangent_weight_vector_mpp)->elements[i*num_cols + j][2] = weight;

                    /*
                    p_stderr("%03d  %03d  %e   %e   %e\n", i, j, point_kappa, weight, weight*point_kappa);
                     */

                    total_curvature += weight*point_kappa;
                    total_length += weight;
                    if (num_output_images > 0)
                    {
                        sorted_curvature_vp->elements[ point_count ].element = point_kappa;
                        coord_index_mp->elements[ point_count ][ 0 ] = i;
                        coord_index_mp->elements[ point_count ][ 1 ] = j;
                        point_count++;
                    }
                }
                else if (res == WRONG_NUMBER_OF_NEIGHBORS)
                {
                    (*curvature_mpp)->elements[i][j] = 2.0 * DBL_NOT_SET;
                }
                else if (res == NOT_LONG_ENOUGH)
                {
                    (*curvature_mpp)->elements[i][j] = 3.0 * DBL_NOT_SET;
                }
                else if (res == DEGENERATE_SYSTEM)
                {
                    (*curvature_mpp)->elements[i][j] = 4.0 * DBL_NOT_SET;
                }
            }
            else if (IS_NEURON_PIXEL(in_ip, i, j))
            {
                /* Thinned pixel. */
                (*curvature_mpp)->elements[i][j] = 5.0 * DBL_NOT_SET;
            }
        }
    }
    sorted_curvature_vp->length = point_count;
    coord_index_mp->num_rows = point_count;

    /*
     * Only true if the number of output images is greater than zero. 
    */
    if (point_count > 0)
    {
        int  count;
        char title[ 1000 ];
        int index;
        int point;
        int i, j;

        ERE(descend_sort_indexed_vector(sorted_curvature_vp));

        for (count = 0; count < num_output_images - 1; count++)
        {
            index = count * point_count / (num_output_images - 1);
            point = sorted_curvature_vp->elements[ index ].index;
            i = coord_index_mp->elements[ point ][ 0 ];
            j = coord_index_mp->elements[ point ][ 1 ];

            ERE(curvature_at_point(on_mp, i, j, &weight, &point_kappa,
                                   in_ip, &(out_ip_list[ count ]), file));
            ERE(kjb_sprintf(title, sizeof(title), "%d-%.3f", count,
                            (*curvature_mpp)->elements[ i ][ j ]));
        }

        point = sorted_curvature_vp->elements[ point_count - 1 ].index;
        i = coord_index_mp->elements[ point ][ 0 ];
        j = coord_index_mp->elements[ point ][ 1 ];

        ERE(curvature_at_point(on_mp, i, j, &weight, &point_kappa,
                               in_ip, &(out_ip_list[ num_output_images - 1 ]), file));

        ERE(kjb_sprintf(title, sizeof(title), "%d-%.3f", num_output_images,
                        (*curvature_mpp)->elements[ i ][ j ]));
    }

    kjb_free_image(output);

    free_indexed_vector(sorted_curvature_vp);
    free_int_matrix(coord_index_mp);

    free_int_matrix(on_mp);
    free_vector(tangent_vector);

    return  total_curvature/total_length;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int curvature_at_point
(
    const Int_matrix* on_mp,
    int               i,
    int               j,
    double*           weight,
    double*           point_kappa_ptr,
    const KJB_image*  in_ip,
    KJB_image**       out_ipp,
    const char*       file
)
{
    int     pos = 0;
    int     neg = 0;
    Matrix* c_mp  = NULL;
    int     k;
    double  error;
    Matrix* r_mp  = NULL;
    Vector* t_vp  = NULL;
    int len;

    *point_kappa_ptr = 0.0;


    if (count_neighbors(on_mp, i, j) != 2)
    {
        return WRONG_NUMBER_OF_NEIGHBORS;
    }

    get_neighbors(on_mp, i, j, &pos, &neg, weight);

#ifdef BALANCE_WEIGHT
    *weight = (fs_points[1+LINELEN].t - fs_points[LINELEN-1].t) / 2.0;
#endif
    if(pos-neg+1 < 4 || pos < MIN_BUFFER_LENGTH || neg > -MIN_BUFFER_LENGTH)
    {
        /*
         * under-constrained - can't really do anything about this, but
         * it shouldn't happen too often
        */
        return NOT_LONG_ENOUGH;
    }

#ifdef SYMMETRIC_FIT
    pos = MIN_OF(ABS_OF(pos), ABS_OF(neg));
    neg = -pos;
#endif

    len = pos - neg + 1;

    EPETE(get_target_vector(&t_vp, len));
    EPETE(get_target_matrix(&r_mp, len, 2));

    for(k = 0; k < len; k++)
    {
        t_vp->elements[ k ] = fs_points[k+LINELEN+neg].t;
        r_mp->elements[ k ][ 0 ] = fs_points[k+LINELEN+neg].j;
        r_mp->elements[ k ][ 1 ] = fs_points[k+LINELEN+neg].i;
    }

#ifdef ITERATIVE_FIT
    EPETE(fit_parametric_cubic(t_vp, r_mp, NULL, NULL, &t_vp, &c_mp, &error));
#else
    EPETE(fit_parametric_cubic(t_vp, r_mp, NULL, NULL, NULL, &c_mp, &error));
#endif

    {
        double t = t_vp->elements[ -neg ];
        double t2 = t*t;
        double xp =  3.0 * c_mp->elements[0][0] * t2 + 2.0 * c_mp->elements[1][0] * t + c_mp->elements[2][0];
        double xpp = 6.0 * c_mp->elements[0][0] * t  + 2.0 * c_mp->elements[1][0];
        double yp =  3.0 * c_mp->elements[0][1] * t2 + 2.0 * c_mp->elements[1][1] * t + c_mp->elements[2][1];
        double ypp = 6.0 * c_mp->elements[0][1] * t  + 2.0 * c_mp->elements[1][1];

        *point_kappa_ptr = 2.0 * (xp*ypp - yp*xpp) / pow((xp*xp + yp*yp), 1.5);
#ifdef ABS_CURVATURE
        *point_kappa_ptr = ABS_OF( *point_kappa_ptr );
#endif
    }

    ASSERT_IS_NUMBER_DBL(*point_kappa_ptr);

    if (out_ipp != NULL)
    {
        KJB_image* window_ip = NULL;
        KJB_image* mag_ip = NULL;
        int tt;
        int ii, jj;
        char buff[ 1000 ];

        EPETE(get_image_window(&window_ip, in_ip, i - 20, j - 20, 40, 40));

        for (ii = 0; ii < 40; ii++)
        {
            for (jj =0; jj < 40; jj++)
            {
                if (window_ip->pixels[ ii ][ jj ].r > 128.0)
                {
                    window_ip->pixels[ ii ][ jj ].r = 128.0;
                    window_ip->pixels[ ii ][ jj ].g = 128.0;
                    window_ip->pixels[ ii ][ jj ].b = 128.0;
                }
            }
        }

        for(k = neg; k <= pos; k++)
        {
            ii = fs_points[k+LINELEN].i;
            jj = fs_points[k+LINELEN].j;
            EPETE(image_draw_point(window_ip, 20 + (ii - i), 20 + (jj - j), 0, 0, 0, 0));
        }

        EPETE(magnify_image(&mag_ip, window_ip, 13, 13));

        for(k = neg; k <= pos; k++)
        {
            ii = fs_points[k+LINELEN].i;
            jj = fs_points[k+LINELEN].j;

            if (k == 0)
            {
                EPETE(image_draw_box(mag_ip, 13 * (ii - i) + 266, 13 * (jj - j) + 266, 6, 2, 255, 255, 255));
            }
            else
            {
                EPETE(image_draw_box(mag_ip, 13 * (ii - i) + 266, 13 * (jj - j) + 266, 6, 0, 255, 255, 255));
            }
        }

        for (tt = 0; tt < CURVE_IMAGE_RESOLUTION; tt++)
        {
            double ttt  = fs_points[neg+LINELEN].t + (tt * (fs_points[pos+LINELEN].t - fs_points[neg+LINELEN].t)) / (CURVE_IMAGE_RESOLUTION - 1.0);
            double d_j  = c_mp->elements[ 0  ][ 0 ]*ttt*ttt*ttt + c_mp->elements[ 1  ][ 0 ]*ttt*ttt + c_mp->elements[ 2  ][ 0 ]*ttt + c_mp->elements[ 3  ][ 0 ];
            double d_i  = c_mp->elements[ 0 ][ 1 ]*ttt*ttt*ttt + c_mp->elements[ 1 ][ 1 ]*ttt*ttt + c_mp->elements[ 2 ][ 1 ]*ttt + c_mp->elements[ 3 ][ 1 ];
            int    d_ii = 13.0 * (d_i - i) + 266.0;
            int    d_jj = 13.0 * (d_j - j) + 266.0;
            int    ii   = kjb_rint(d_ii);
            int    jj   = kjb_rint(d_jj);
            int    iii;
            int    jjj;
            double sum  = 0.0;

            for (iii = -1; iii <= 1; iii++)
            {
                for (jjj = -1; jjj <= 1; jjj++)
                {
                    double d_iii = ii + iii;
                    double d_jjj = jj + jjj;
                    double w = exp( - 0.5 * ((d_iii - d_ii)*(d_iii - d_ii) + (d_jjj - d_jj)*(d_jjj - d_jj) ) );

                    sum += w;
                }
            }

            for (iii = -1; iii <= 1; iii++)
            {
                for (jjj = -1; jjj <= 1; jjj++)
                {
                    double d_iii = ii + iii;
                    double d_jjj = jj + jjj;
                    double w = exp( - 0.5 * ((d_iii - d_ii)*(d_iii - d_ii) + (d_jjj - d_jj)*(d_jjj - d_jj) ) );

                    EPETE(image_draw_add_to_point(mag_ip, ii + iii, jj + jjj, 0,
                                                  (int)(500.0 * w / sum),
                                                  (int)(500.0 * w / sum),
                                                  (int)(500.0 * w / sum)));

                }
            }
        }

        for (ii = 0; ii < 40; ii++)
        {
            for (jj =0; jj < 40; jj++)
            {
                if (    (window_ip->pixels[ ii ][ jj ].r < 128.0)
                     && (window_ip->pixels[ ii ][ jj ].g > 128.0)
                   )
                {
                    EPETE(image_draw_rectangle(mag_ip, 13 * ii, 13 * jj, 13, 13, 0, 0, 0));

                    EPETE(image_draw_box(mag_ip, 6 + 13 * ii, 6 + 13 * jj, 6, 0, 255, 255, 255));

                    EPETE(image_draw_segment_2(mag_ip,
                                               13 * ii,
                                               13 * jj,
                                               13 * ii + 12,
                                               13 * jj + 12,
                                               0, 255, 255, 255));

                    EPETE(image_draw_segment_2(mag_ip,
                                               13 * ii,
                                               13 * jj + 12,
                                               13 * ii + 12,
                                               13 * jj,
                                               0, 255, 255, 255));
                }
            }
        }

        EPETE(ow_invert_image(mag_ip));

        EPETE(image_draw_text_top_left(mag_ip, file, 5, 5, "times18"));
        EPETE(kjb_sprintf(buff, sizeof(buff), "Curvature estimate: %.3f", *point_kappa_ptr));
        EPETE(image_draw_text_top_left(mag_ip, buff, 30, 5, "times18"));
        EPETE(kjb_copy_image(out_ipp, mag_ip));

        kjb_free_image(mag_ip);
        kjb_free_image(window_ip);

    }

    free_matrix(c_mp);
    free_vector(t_vp);
    free_matrix(r_mp);

    ASSERT_IS_NUMBER_DBL(*point_kappa_ptr);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int curvature_coeffs_at_point
(
    const Int_matrix* on_mp,
    int               i,
    int               j,
    double*           weight,
    double*           point_kappa_ptr,
    Vector*           tangent_vector,
    const KJB_image*  in_ip,
    KJB_image**       out_ipp,
    const char*       file
)
{
    int     pos = 0;
    int     neg = 0;
    Matrix* c_mp  = NULL;
    int     k;
    double  error;
    Matrix* r_mp  = NULL;
    Vector* t_vp  = NULL;
    int len;

    *point_kappa_ptr = 0.0;


    if (count_neighbors(on_mp, i, j) != 2)
    {
        return WRONG_NUMBER_OF_NEIGHBORS;
    }

    get_neighbors(on_mp, i, j, &pos, &neg, weight);

#ifdef BALANCE_WEIGHT
    *weight = (fs_points[1+LINELEN].t - fs_points[LINELEN-1].t) / 2.0;
#endif

    if(pos-neg+1 < 4 || pos < MIN_BUFFER_LENGTH || neg > -MIN_BUFFER_LENGTH)
    {
        /*
         * under-constrained - can't really do anything about this, but
         * it shouldn't happen too often
        */
        return NOT_LONG_ENOUGH;
    }

#ifdef SYMMETRIC_FIT
    pos = MIN_OF(ABS_OF(pos), ABS_OF(neg));
    neg = -pos;
#endif

    len = pos - neg + 1;

    EPETE(get_target_vector(&t_vp, len));
    EPETE(get_target_matrix(&r_mp, len, 2));

    for(k = 0; k < len; k++)
    {
        t_vp->elements[ k ] = fs_points[k+LINELEN+neg].t;
        r_mp->elements[ k ][ 0 ] = fs_points[k+LINELEN+neg].j;
        r_mp->elements[ k ][ 1 ] = fs_points[k+LINELEN+neg].i;
    }

#ifdef ITERATIVE_FIT
    EPETE(fit_parametric_cubic(t_vp, r_mp, NULL, NULL, &t_vp, &c_mp, &error));
#else
    EPETE(fit_parametric_cubic(t_vp, r_mp, NULL, NULL, NULL, &c_mp, &error));
#endif

    {
        double t = t_vp->elements[ -neg ];
        double t2 = t*t;
        double xp =  3.0 * c_mp->elements[0][0] * t2 + 2.0 * c_mp->elements[1][0] * t + c_mp->elements[2][0];
        double xpp = 6.0 * c_mp->elements[0][0] * t  + 2.0 * c_mp->elements[1][0];
        double yp =  3.0 * c_mp->elements[0][1] * t2 + 2.0 * c_mp->elements[1][1] * t + c_mp->elements[2][1];
        double ypp = 6.0 * c_mp->elements[0][1] * t  + 2.0 * c_mp->elements[1][1];

        tangent_vector->elements[0] = xp;
        tangent_vector->elements[1] = yp;


        *point_kappa_ptr = 2.0 * (xp*ypp - yp*xpp) / pow((xp*xp + yp*yp), 1.5);
#ifdef ABS_CURVATURE
        *point_kappa_ptr = ABS_OF( *point_kappa_ptr );
#endif
    }

    ASSERT_IS_NUMBER_DBL(*point_kappa_ptr);

    if (out_ipp != NULL)
    {
        KJB_image* window_ip = NULL;
        KJB_image* mag_ip = NULL;
        int tt;
        int ii, jj;
        char buff[ 1000 ];

        EPETE(get_image_window(&window_ip, in_ip, i - 20, j - 20, 40, 40));

        for (ii = 0; ii < 40; ii++)
        {
            for (jj =0; jj < 40; jj++)
            {
                if (window_ip->pixels[ ii ][ jj ].r > 128.0)
                {
                    window_ip->pixels[ ii ][ jj ].r = 128.0;
                    window_ip->pixels[ ii ][ jj ].g = 128.0;
                    window_ip->pixels[ ii ][ jj ].b = 128.0;
                }
            }
        }

        for(k = neg; k <= pos; k++)
        {
            ii = fs_points[k+LINELEN].i;
            jj = fs_points[k+LINELEN].j;
            EPETE(image_draw_point(window_ip, 20 + (ii - i), 20 + (jj - j), 0, 0, 0, 0));
        }

        EPETE(magnify_image(&mag_ip, window_ip, 13, 13));

        for(k = neg; k <= pos; k++)
        {
            ii = fs_points[k+LINELEN].i;
            jj = fs_points[k+LINELEN].j;

            if (k == 0)
            {
                EPETE(image_draw_box(mag_ip, 13 * (ii - i) + 266, 13 * (jj - j) + 266, 6, 2, 255, 255, 255));
            }
            else
            {
                EPETE(image_draw_box(mag_ip, 13 * (ii - i) + 266, 13 * (jj - j) + 266, 6, 0, 255, 255, 255));
            }
        }

        for (tt = 0; tt < CURVE_IMAGE_RESOLUTION; tt++)
        {
            double ttt  = fs_points[neg+LINELEN].t + (tt * (fs_points[pos+LINELEN].t - fs_points[neg+LINELEN].t)) / (CURVE_IMAGE_RESOLUTION - 1.0);
            double d_j  = c_mp->elements[ 0  ][ 0 ]*ttt*ttt*ttt + c_mp->elements[ 1  ][ 0 ]*ttt*ttt + c_mp->elements[ 2  ][ 0 ]*ttt + c_mp->elements[ 3  ][ 0 ];
            double d_i  = c_mp->elements[ 0 ][ 1 ]*ttt*ttt*ttt + c_mp->elements[ 1 ][ 1 ]*ttt*ttt + c_mp->elements[ 2 ][ 1 ]*ttt + c_mp->elements[ 3 ][ 1 ];
            int    d_ii = 13.0 * (d_i - i) + 266.0;
            int    d_jj = 13.0 * (d_j - j) + 266.0;
            int    ii   = kjb_rint(d_ii);
            int    jj   = kjb_rint(d_jj);
            int    iii;
            int    jjj;
            double sum  = 0.0;

            for (iii = -1; iii <= 1; iii++)
            {
                for (jjj = -1; jjj <= 1; jjj++)
                {
                    double d_iii = ii + iii;
                    double d_jjj = jj + jjj;
                    double w = exp( - 0.5 * ((d_iii - d_ii)*(d_iii - d_ii) + (d_jjj - d_jj)*(d_jjj - d_jj) ) );

                    sum += w;
                }
            }

            for (iii = -1; iii <= 1; iii++)
            {
                for (jjj = -1; jjj <= 1; jjj++)
                {
                    double d_iii = ii + iii;
                    double d_jjj = jj + jjj;
                    double w = exp( - 0.5 * ((d_iii - d_ii)*(d_iii - d_ii) + (d_jjj - d_jj)*(d_jjj - d_jj) ) );

                    EPETE(image_draw_add_to_point(mag_ip, ii + iii, jj + jjj, 0,
                                                  (int)(500.0 * w / sum),
                                                  (int)(500.0 * w / sum),
                                                  (int)(500.0 * w / sum)));

                }
            }
        }

        for (ii = 0; ii < 40; ii++)
        {
            for (jj =0; jj < 40; jj++)
            {
                if (    (window_ip->pixels[ ii ][ jj ].r < 128.0)
                     && (window_ip->pixels[ ii ][ jj ].g > 128.0)
                   )
                {
                    EPETE(image_draw_rectangle(mag_ip, 13 * ii, 13 * jj, 13, 13, 0, 0, 0));

                    EPETE(image_draw_box(mag_ip, 6 + 13 * ii, 6 + 13 * jj, 6, 0, 255, 255, 255));

                    EPETE(image_draw_segment_2(mag_ip,
                                               13 * ii,
                                               13 * jj,
                                               13 * ii + 12,
                                               13 * jj + 12,
                                               0, 255, 255, 255));

                    EPETE(image_draw_segment_2(mag_ip,
                                               13 * ii,
                                               13 * jj + 12,
                                               13 * ii + 12,
                                               13 * jj,
                                               0, 255, 255, 255));
                }
            }
        }

        EPETE(ow_invert_image(mag_ip));

        EPETE(image_draw_text_top_left(mag_ip, file, 5, 5, "times18"));
        EPETE(kjb_sprintf(buff, sizeof(buff), "Curvature estimate: %.3f", *point_kappa_ptr));
        EPETE(image_draw_text_top_left(mag_ip, buff, 30, 5, "times18"));
        EPETE(kjb_copy_image(out_ipp, mag_ip));

        kjb_free_image(mag_ip);
        kjb_free_image(window_ip);

    }

    free_matrix(c_mp);
    free_vector(t_vp);
    free_matrix(r_mp);

    ASSERT_IS_NUMBER_DBL(*point_kappa_ptr);

    return NO_ERROR;
}


/* TODO - pass back how many fs_points found in each direction
 * TODO - take care of total_length
 */
static void get_neighbors(const Int_matrix *on_mp, int i, int j, int *pos, int *neg, double *weight)
{
  int k;
  int new_i, new_j;
  int new_dir;
  int found = FALSE;


  if (count_neighbors(on_mp, i, j) > 2)
  {
     return;
  }
  /* Without any assumptions about previous searches, just start from the
    * top until a neighbor is found, and then search in that direction and
    * the opposite direction.
    */
  fs_points[fs_line_len].i = i;
  fs_points[fs_line_len].j = j;
  fs_points[fs_line_len].t = 0.0;

  for(k = 0; k < CURV_NUM_DIRECTIONS; k++)
  {
     new_dir = fs_search_directions[k];
     new_i = i + curv_neighbors[new_dir][0];
     new_j = j + curv_neighbors[new_dir][1];

     if (PIXEL_IS_ON(on_mp, new_i, new_j))
     {
        found = TRUE;
        break;
     }
  }

  if(!found)
  {
     *pos = 0;
     *neg = 0;
     return;
  }

  *weight = ((new_dir%2)==0)?1.0:M_SQRT2;
  fs_points[fs_line_len+1].i = new_i;
  fs_points[fs_line_len+1].j = new_j;
  fs_points[fs_line_len+1].t = *weight;
  /* Start at the new point with depth 1, since we already have a point
    */
  *pos = get_neighbors_in_dir(on_mp, new_i, new_j, new_dir, 2);

  /* since we don't know for sure we have a point in this direction,
    * start at the initial point with depth 0
    */
  *neg = get_neighbors_in_dir(on_mp, i, j, (new_dir+4)%8, -1);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* assumes this pixel is 'on' */
static int get_neighbors_in_dir(const Int_matrix *on_mp, int i, int j, int start_dir, int depth)
{
  /* find a neighbor in this direction
    * start with this exact dir, and fan outward, alternating
    * left and right; last thing checked will be opposite direction
    */
  int result = depth;
  int sign = (depth <= 0)?(-1):(1);

  int k;
  int new_i, new_j;
  int new_dir;
  int found = FALSE;


  if (count_neighbors(on_mp, i, j) > 2)
  {
     /* didn't fill in anything at this index; last valid index is one before*/
     return depth - sign;
  }

  for(k = 0; k < CURV_NUM_DIRECTIONS - 1; k++)
  {
     new_dir = (start_dir + fs_search_directions[k]) % CURV_NUM_DIRECTIONS;

     new_i = i + curv_neighbors[new_dir][0];
     new_j = j + curv_neighbors[new_dir][1];

     if (PIXEL_IS_ON(on_mp, new_i, new_j))
     {
         int already_got_one = FALSE;
         int kk;

         if (depth < 0)
         {
             for (kk = depth + 1; kk <= 0; kk++)
             {
                 if (    (fs_points[fs_line_len+kk].i == new_i)
                      && (fs_points[fs_line_len+kk].j == new_j)
                    )
                 {
                     already_got_one = TRUE;
                     break;
                 }
             }
         }
         else
         {
             for (kk = 0; kk < depth; kk++)
             {
                 if (    (fs_points[fs_line_len+kk].i == new_i)
                      && (fs_points[fs_line_len+kk].j == new_j)
                    )
                 {
                     already_got_one = TRUE;
                     break;
                 }
             }
         }

         if ( ! already_got_one)
         {
            found = TRUE;
            break;
         }
     }
  }


  if(!found)
  {
     /* didn't fill in anything at this index; last valid index is one before*/
     return depth - sign;
  }

  fs_points[fs_line_len+depth].i = new_i;
  fs_points[fs_line_len+depth].j = new_j;
  fs_points[fs_line_len+depth].t = fs_points[fs_line_len+depth-sign].t +
     sign*(((new_dir%2)==0)?(1.0):M_SQRT2);

  /* Call self again in same direction as neighbor found, starting from
    * new location
    */
  if (ABS_OF(depth) < fs_line_len)
  {
     int new_depth = depth+(sign);
     result = get_neighbors_in_dir(on_mp, new_i, new_j, new_dir, new_depth);
  }
  return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

