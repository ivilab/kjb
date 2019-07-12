
/* $Id: curv_lib.c 21545 2017-07-23 21:57:31Z kobus $ */

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


#include "m/m_gen.h"
#include "n/n_invert.h"
#include "nr/nr_roots.h"
#include "i/i_incl.h"
#include "curv/curv_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int thin_pixels_not_needed_for_contiguity_2
(
    Int_matrix* image_mp,
    int         num_neighbors
);

static int get_connectivity
(
    Int_matrix**      connectivity_mpp,
    const Int_matrix* image_mp,
    int               i,
    int               j
);

static int update_connectivity
(
    int*              connectivity_ptr,
    const Int_matrix* image_mp,
    int               i,
    int               j
);

static int refit_parametric_cubic_parameter
(
    Vector**      new_t_vpp,
    const Vector* t_vp,
    const Matrix* data_mp,
    const Matrix* c_mp
);

/* -------------------------------------------------------------------------- */

int thin_pixels_not_needed_for_contiguity(Int_matrix* image_mp)
{
    int num_neigbours;

    /* Thin outside in */
    for (num_neigbours = 2; num_neigbours <= 8; num_neigbours++)
    {
        while (thin_pixels_not_needed_for_contiguity_2(image_mp, num_neigbours) > 0)
        {
            /*EMPTY*/
            ; /* Do nothing. */
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int thin_pixels_not_needed_for_contiguity_2
(
    Int_matrix* image_mp,
    int         num_neighbors
)
{
    int num_removed = 0;
    int num_rows = image_mp->num_rows;
    int num_cols = image_mp->num_cols;
    int i,j, k;
    Int_matrix* connectivity_mp = NULL;
    Int_matrix* new_connectivity_mp = NULL;


    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if ( ! image_mp->elements[ i ][ j ]) continue;

            if (    ( ! image_mp->elements[ i ][ j ])
                 || (count_neighbors(image_mp, i, j) != num_neighbors)
               )
            {
                continue;
            }

            ERE(get_connectivity(&connectivity_mp, image_mp, i, j));

            for (k = 0; k < 9; k++)
            {
                connectivity_mp->elements[ 4 ][ k ] = 0;
                connectivity_mp->elements[ k ][ 4 ] = 0;
            }

            image_mp->elements[ i ][ j ] = FALSE;

            ERE(get_connectivity(&new_connectivity_mp, image_mp, i, j));

            if (max_abs_int_matrix_difference(connectivity_mp, new_connectivity_mp) > 0)
            {
                /* We broke it. Back off. */
                image_mp->elements[ i ][ j ] = TRUE;
            }
            else
            {
                num_removed++;
            }
        }
    }

    free_int_matrix(connectivity_mp);
    free_int_matrix(new_connectivity_mp);

    return num_removed;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_connectivity
(
    Int_matrix**      connectivity_mpp,
    const Int_matrix* image_mp,
    int               i,
    int               j
)
{
    int num_rows = image_mp->num_rows;
    int num_cols = image_mp->num_cols;
    int di, dj;
    int pixel_index;
    int* connectivity_ptr;
    int connectivity_count;
    int new_connectivity_count;

    ERE(get_zero_int_matrix(connectivity_mpp, 9, 9));

    for (di = -1; di <= 1; di++)
    {
        for (dj = -1; dj <= 1; dj++)
        {
            if (    (i + di < 0)
                 || (j + dj < 0)
                 || (i + di >= num_rows - 1)
                 || (j + dj >= num_cols - 1)
                 || ( ! image_mp->elements[ i + di ][ j + dj ])
              )
            {
                continue;
            }

            pixel_index = 3 * (di + 1) + dj + 1;

            connectivity_ptr = (*connectivity_mpp)->elements[ pixel_index ];
            connectivity_ptr[ pixel_index ] = 1;

            connectivity_count = 1;

            while (TRUE)
            {
                new_connectivity_count = update_connectivity(connectivity_ptr, image_mp, i, j);

                if (new_connectivity_count == connectivity_count)
                {
                    break;
                }

                connectivity_count = new_connectivity_count;
            }
        }
    }

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int update_connectivity
(
    int* connectivity_ptr,
    const Int_matrix* image_mp,
    int               i,
    int               j
)
{
    int num_rows = image_mp->num_rows;
    int num_cols = image_mp->num_cols;
    int di, dj, ddi, ddj, dddi, dddj;
    int connectivity_count = 0;
    int pixel_index;
    int count;
    int test_i, test_j;


    for (count = 0; count < 9; count++)
    {
        if ( ! connectivity_ptr[ count ]) continue;

        di = count / 3 - 1;
        dj = count % 3 - 1;

        for (ddi = -1; ddi <= 1; ddi++)
        {
            dddi = di + ddi;

            if ((dddi < -1) || (dddi > 1)) continue;

            for (ddj = -1; ddj <= 1; ddj++)
            {
                dddj = dj + ddj;

                if ((dddj < -1) || (dddj > 1)) continue;

                test_i = i + dddi;
                test_j = j + dddj;

                if (    (test_i < 0)
                     || (test_i >= num_rows)
                     || (test_j < 0)
                     || (test_j >= num_cols)
                     || (! image_mp->elements[ test_i ][ test_j ])
                   )
                {
                    continue;
                }

                pixel_index = 3 * (dddi + 1) + dddj + 1;
                connectivity_ptr[ pixel_index ] = TRUE;
            }
        }
    }

    for (count = 0; count < 9; count++)
    {
        if (connectivity_ptr[ count ])
        {
            connectivity_count++;
        }
    }

    return connectivity_count;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int count_neighbors(const Int_matrix* image_mp, int i, int j)
{
    int num_rows = image_mp->num_rows;
    int num_cols = image_mp->num_cols;
    int min_ii = MAX_OF(0, i - 1);
    int min_jj = MAX_OF(0, j - 1);
    int max_ii = MIN_OF(i + 1, num_rows - 1);
    int max_jj = MIN_OF(j + 1, num_cols - 1);
    int ii, jj;
    int count = 0;

    for (ii = min_ii; ii <= max_ii; ii++)
    {
        for (jj = min_jj; jj <= max_jj; jj++)
        {
            if ((ii != i) || (jj != j))
            {
                if (image_mp->elements[ ii ][ jj ])
                {
                    count++;
                }
            }
        }
    }

    return count;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ==========================================================================
 *                       fit_parametric_cubic_known_time
 *
 *  Returns a pair of parametric equations in t which most closely fit a set
 *  of specified points. The time values for the set of points must be known.
 *  For a set of points whose times are not known, see fit_parametric_cubic.
 *
 *  Parameters:
 *  t_vp: a pointer to a set of time values 
 *  corresponding to the input points in data_mp.
 *  data_mp: a pointer to a set of points.
 *  weight_vp: a pointer to a set of weight values for points in data_mp. 
 *  new_c_mpp: a pointer to a pointer to a set of parametric cubic coefficients.
 *  error_ptr: a pointer to a fitting error value.
 *
 *  Returns:
 *  An integer indicating whether the function executed successfully. 
 *  Returns ERROR if a problem was encountered.
 *
 *  -------------------------------------------------------------------------
*/

int fit_parametric_cubic_known_time
(
    const Vector* t_vp,
    const Matrix* data_mp,
    const Vector* weight_vp,
    Matrix** new_c_mpp,
    double* error_ptr
)
{
    return fit_parametric_cubic(t_vp, data_mp, weight_vp, NULL, NULL, new_c_mpp, error_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* ==========================================================================
 *                          fit_parametric_cubic
 *
 *  Returns a pair of parametric equations in t which most closely fit a set
 *  of specified points. The time values for the set of points is not known.
 *  For a set of points whose times are known, see
 *  fit_parametric_cubic_known_time.
 *
 *  Parameters:
 *  t_vp: a pointer to a set of time values 
 *  corresponding to the input points in data_mp.
 *  data_mp: a pointer to a set of points.
 *  weight_vp: a pointer to a set of weight values for points in data_mp. 
 *  c_mp: a pointer to the current estimate for the coeffcients of the cubics.
 *  new_t_vpp: a pointer to an updated estimate for the times at 
 *  which the points occurred.
 *  new_c_mpp: a pointer to a pointer to a set of parametric cubic coefficients.
 *  error_ptr: a pointer to a fitting error value.
 *
 *  Returns:
 *  An integer indicating whether the function executed successfully. 
 *  Returns ERROR if a problem was encountered.
 *
 *  -------------------------------------------------------------------------
*/
#define NUM_ITS 2

int fit_parametric_cubic
(
    const Vector* t_vp,
    const Matrix* data_mp,
    const Vector* weight_vp,
    const Matrix* c_mp,
    Vector**      new_t_vpp,
    Matrix**      new_c_mpp,
    double*       error_ptr
)
{
    int n = data_mp->num_rows;
    int result = NO_ERROR;
    int j;
    Matrix* A_mp = NULL;
    int it;
    Matrix *inv_A_mp = NULL;


    verify_matrix(data_mp, NULL);

    ERE(get_target_matrix(&A_mp, n, 4));

    for (it = 0; it < NUM_ITS; it++)
    {
        for (j = 0; j < n; j++)
        {
            double obs_weight = (weight_vp == NULL) ? 1.0 : weight_vp->elements[ j ];
            double t = t_vp->elements[ j ];

            ASSERT_IS_NUMBER_DBL(t);

            A_mp->elements[ j ][0] = obs_weight*t*t*t;
            A_mp->elements[ j ][1] = obs_weight*t*t;
            A_mp->elements[ j ][2] = obs_weight*t;
            A_mp->elements[ j ][3] = obs_weight;
        }

        verify_matrix(A_mp, NULL);

        if ((c_mp == NULL) && (new_c_mpp != NULL))
        {
            result = get_MP_inverse(&inv_A_mp,A_mp);

            if (result != ERROR)
            {
                result = multiply_matrices(new_c_mpp, inv_A_mp, data_mp);
            }
            else
            {
               NOTE_ERROR(); break;
            }

            c_mp = *new_c_mpp;
        }

        if ((new_t_vpp == NULL) || (it == (NUM_ITS - 1)))
        {
            break;
        }
        else
        {
            result = refit_parametric_cubic_parameter(new_t_vpp, t_vp, data_mp, c_mp);
            if (result == ERROR) {NOTE_ERROR(); break;}
            t_vp = (*new_t_vpp);
        }
    }

    verify_matrix(c_mp, NULL);

    if ((result != ERROR) && (c_mp != NULL) && (error_ptr != NULL))
    {
        Matrix *est_mp = NULL;

         result = multiply_matrices(&est_mp, A_mp, c_mp);

         verify_matrix(est_mp, NULL);

         if (result != ERROR)
         {
             *error_ptr = rms_matrix_difference(est_mp, data_mp);
             *error_ptr *= sqrt((double)data_mp->num_cols);
         }

         free_matrix(est_mp);
    }

    free_matrix(A_mp);
    free_matrix(inv_A_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int refit_parametric_cubic_parameter
(
    Vector**      new_t_vpp,
    const Vector* t_vp,
    const Matrix* data_mp,
    const Matrix* c_mp
)
{
    int n = data_mp->num_rows;
    int m = data_mp->num_cols;
    int result = NO_ERROR;
    int k;
    Vector* prev_t_vp = NULL;
    Vector* px_vp = NULL;
    Vector* py_vp = NULL;


    if (m != 2)
    {
        set_error("Iterative refitting only implemented for 1 or 2 variables.");
        return ERROR;
    }

    /*
     * Copy t_vp, in case it is the same as new_t_vpp.
    */

    if (    (copy_vector(&prev_t_vp, t_vp) == ERROR)
         || (get_target_vector(&px_vp, 4) == ERROR)
         || (get_target_vector(&py_vp, 4) == ERROR)
       )
    {
        NOTE_ERROR(); result = ERROR;
    }

    if (result != ERROR)
    {
        px_vp->elements[ 3 ] = c_mp->elements[ 0  ][ 0 ];
        px_vp->elements[ 2 ] = c_mp->elements[ 1  ][ 0 ];
        px_vp->elements[ 1 ] = c_mp->elements[ 2  ][ 0 ];

        py_vp->elements[ 3 ] = c_mp->elements[ 0  ][ 1 ];
        py_vp->elements[ 2 ] = c_mp->elements[ 1  ][ 1 ];
        py_vp->elements[ 1 ] = c_mp->elements[ 2  ][ 1 ];

        for(k = 0; k < n; k++)
        {
            double t = prev_t_vp->elements[ k ];
            double t_new = DBL_NOT_SET;
            double roots_1[ 3 ];
            double roots_2[ 3 ];
            int num_roots_1;
            int num_roots_2;
            int ii, jj;
            double min_new_fit_err = DBL_MOST_POSITIVE;
            double min_new_err = DBL_MOST_POSITIVE;
            double min_err = DBL_MOST_POSITIVE;
            double min_ex;
            double min_ey;
            double min_new_ex;
            double min_new_ey;

            px_vp->elements[ 0 ] = c_mp->elements[ 3  ][ 0 ] - data_mp->elements[ k ][ 0 ];
            py_vp->elements[ 0 ] = c_mp->elements[ 3  ][ 1 ] - data_mp->elements[ k ][ 1 ];

            num_roots_1 = nr_real_roots_of_real_polynomial(3, px_vp->elements, roots_1);

            if (num_roots_1 == ERROR)
            {
                if (ABS_OF(px_vp->elements[ 0 ]) > 1.0e5 * DBL_EPSILON)
                {
                    db_mat(c_mp);
                    db_rv(px_vp);
                    kjb_print_error();
                }
                continue;
            }

            num_roots_2 = nr_real_roots_of_real_polynomial(3, py_vp->elements, roots_2);

            if (num_roots_2 == ERROR)
            {
                if (ABS_OF(py_vp->elements[ 0 ]) > 1.0e5 * DBL_EPSILON)
                {
                    db_mat(c_mp);
                    db_rv(py_vp);
                    kjb_print_error();
                }
                continue;
            }

            for (ii = 0; ii < num_roots_1; ii++)
            {
                for (jj = 0; jj < num_roots_2; jj++)
                {
                    double r1 = roots_1[ ii ];
                    double r2 = roots_2[ jj ];
                    double ex = ABS_OF(px_vp->elements[ 3 ]*t*t*t + px_vp->elements[ 2 ]*t*t + px_vp->elements[ 1 ]*t + px_vp->elements[ 0 ]);
                    double ey = ABS_OF(py_vp->elements[ 3 ]*t*t*t + py_vp->elements[ 2 ]*t*t + py_vp->elements[ 1 ]*t + py_vp->elements[ 0 ]);
                    double p1x = px_vp->elements[ 3 ]*r1*r1*r1 + px_vp->elements[ 2 ]*r1*r1 + px_vp->elements[ 1 ]*r1 + c_mp->elements[ 3  ][ 0 ];
                    double p1y = py_vp->elements[ 3 ]*r1*r1*r1 + py_vp->elements[ 2 ]*r1*r1 + py_vp->elements[ 1 ]*r1 + c_mp->elements[ 3  ][ 1 ];
                    double p2x = px_vp->elements[ 3 ]*r2*r2*r2 + px_vp->elements[ 2 ]*r2*r2 + px_vp->elements[ 1 ]*r2 + c_mp->elements[ 3  ][ 0 ];
                    double p2y = py_vp->elements[ 3 ]*r2*r2*r2 + py_vp->elements[ 2 ]*r2*r2 + py_vp->elements[ 1 ]*r2 + c_mp->elements[ 3  ][ 1 ];
                    double dxp1p2 = p2x - p1x;
                    double dyp1p2 = p2y - p1y;
                    /*
                     * Don't need, should be zero.
                     double dxp1p0 = p0x - p1x;
                     */
                    /*
                     * Don't need, should be same as dyp1p2.
                     double dyp1p0 = p0y - p1y;
                     */
                    double dyp1p2_sqrd = dyp1p2*dyp1p2;
                    double dp1p2_sqrd = dyp1p2_sqrd + dxp1p2*dxp1p2;
                    double a = dyp1p2_sqrd / dp1p2_sqrd;
                    double r = a*r2 + (1.0-a)*r1;
                    double new_ex = ABS_OF(px_vp->elements[ 3 ]*r*r*r + px_vp->elements[ 2 ]*r*r + px_vp->elements[ 1 ]*r + px_vp->elements[ 0 ]);
                    double new_ey = ABS_OF(py_vp->elements[ 3 ]*r*r*r + py_vp->elements[ 2 ]*r*r + py_vp->elements[ 1 ]*r + py_vp->elements[ 0 ]);
                    double e3 = ABS_OF(r1 - r2);
                    double e4 = ABS_OF(r - t);

                    if (e3 * e4 * (new_ex*new_ex + new_ey*new_ey) < min_new_err)
                    {
                        min_new_err = e3* e4 * (new_ex*new_ex + new_ey*new_ey);
                        min_new_fit_err = (new_ex*new_ex + new_ey*new_ey);
                        t_new = r;
                        min_ex = ex;
                        min_ey = ey;
                        min_new_ex = new_ex;
                        min_new_ey = new_ey;
                        min_err = ex*ex + ey*ey;
                    }
                }
            }

            if (isnan(t_new))
            {
                dbe(t_new);
                kjb_exit(EXIT_FAILURE);
            }

            (*new_t_vpp)->elements[ k ] = t_new;
        }
    }

    free_vector(prev_t_vp);
    free_vector(px_vp);
    free_vector(py_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST_CIRCLE

static KJB_image *create_test_circle(int rows, int cols)
{
    KJB_image *im = kjb_create_image(rows, cols);

    double pi = 3.14159265358979;
    int i, j, k;
    double diam = MIN(rows,cols)/2 - 1.1;


    for(i = 0; i < rows; i++)
    {
        for(j = 0; j < cols; j++)
        {
            im->pixels[i][j].r = 0;
            im->pixels[i][j].g = 0;
            im->pixels[i][j].b = 0;
        }
    }

    for(k = 0; k < 360; k++)
    {
        i = rows/2 + diam*sin((double)k/2.0/pi);
        j = cols/2 + diam*cos((double)k/2.0/pi);
        im->pixels[i][j].r = 255.0;
        im->pixels[i][j].g = 255.0;
        im->pixels[i][j].b = 255.0;
    }

    return im;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

