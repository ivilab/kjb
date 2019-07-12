/* $Id: ransac_fit.c 15688 2013-10-14 08:46:32Z predoehl $
 */
#include "slic/ransac_fit.h"

static int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *) = fit_homography; 
static int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **) = get_homography_distance; 
static int (*degen_func)(const Matrix *) = is_homography_degenerate;

static int fs_max_random_trials = 100; 
static double fs_prob = 0.99; /* Desired probability of choosing at least one
                              sample free from outliers */ 


/*static int get_random_samples
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          m,
    Matrix       **rand_x_mpp,
    Matrix       **rand_y_mpp
);*/

static int generate_random_index
(
    int              m,
    int              n,
    Int_vector       **index_ivpp
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// Robustly fits a model to data with the RANSAC algorithm.
// x_mp, y_mp         ---- data to be fitted. y=f(x).
// max_num_tries      ---- the maximum number of trials before giving up;
// min_num_samples    ---- the minimum number required by fitting a model. For
//                         2D affine transformation, this is 3.
// good_fit_threshold ---- the distance threshold used to decide whether a point
//                         is an inlier;
// a_mpp              ---- a 3x2 affine matrix of the best model.
// fit_error_ptr      ---- the fitting error of the best model.
//
// Modified from Peter Kovesi' code which is available at
// http://www.csse.uwa.edu.au/~pk/Research/MatlabFns/index.html

// Note the best model returned is the one with the most inliers, instead of the one with
// the minimum fitting error. However, ties are broken by using the fitting error.

// Refereneces:
// Richard Hartlye and Andere Zisserman. "Multiple View Geometry in Computer
// Vision". pp 101-113. Cambridge University Press, 2001
*/


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   
// Robustly fits a constrained model to data with the RANSAC algorithm.
// See routine 'ransac_fit' for more details.
*/
int ransac_fit_constrained
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int (*validate_constraints)(const Matrix *),
    int          max_num_tries,
    int          min_num_samples,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
)
{
    int num = x_mp->num_rows;
    Matrix *A_mp = NULL;
    Matrix *B_mp = NULL;
    Matrix *a_mp = NULL;
    Int_vector *index_ivp = NULL;
    int i;
    int try_num = 0; /* counter for trials */
    int count;
    int new_count;
    int max_inliers = 0;
    int result;
    int N = INT_MAX; /* dummy initialization for number of trials */
    double pNoOutliers;
    double frac;
    double d_N;
    double min_fit_err = DBL_HALF_MOST_POSITIVE;
    double fit_err;

    if(num < min_num_samples)
    {
       add_error("Warning: minumum number of samples required is %d\n", min_num_samples);
       return 0;
    }

    ERE(get_target_int_vector(&index_ivp, num));
    while(N > try_num && try_num < max_num_tries)
    {
     /*   pso("%d %d\n", N, try_num);*/

        /* randomly pick the mininum required number of points */
      result = get_random_samples(x_mp, y_mp, min_num_samples, &A_mp, &B_mp, degen_func);
        if(result <= 0) break;
       /* write_matrix(A_mp, 0);
        write_matrix(B_mp, 0);*/

        /* we have checked the validity of the picked samples. we double check
          here in case we still encounter problem fitting the samples */
        ERE(fitting_func(A_mp, B_mp, &a_mp, NULL));
    /*       if(fitting_func(A_mp, B_mp, &a_mp, NULL) == ERROR)
        {
        pso("Warning: cannot fit the data!\n");
            try_num++;
            continue;
        }
    */

        /* check if the constraints meet */
        if(validate_constraints != NULL && !validate_constraints(a_mp))
        {
          /*  pso("Warning: constraints are not met!\n");*/
            try_num++;
            continue;
        }

        /* Now we can see how many points agree with this */
        count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp, dist_func);

       /* pso("%d %d\n", try_num, count);*/
        if(count >= min_num_samples && count >= max_inliers) /* keep better result */
        {
            /* Here we break ties by using the minimum fitting error so it's
             * more expensive */
            /* refit using all the inliers */
            ERE(get_fitting_data(x_mp, y_mp, index_ivp, &A_mp, &B_mp));
            ERE(fitting_func(A_mp, B_mp, &a_mp, &fit_err));
            new_count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold,
                                    index_ivp, dist_func);
            
            if(new_count > max_inliers || fit_err < min_fit_err)
            {
                min_fit_err = fit_err;

                if(index_ivpp != NULL)
                {
                    ERE(copy_int_vector(index_ivpp, index_ivp));
                }

                if(a_mpp != NULL)
                {
                    ERE(copy_matrix(a_mpp, a_mp));
                }

                if(fit_err_ptr != NULL) *fit_err_ptr = min_fit_err;

            /* Update estimate of N, the number of trials to ensure we pick,
            // with probability p, a data set with no outliers */
                if(count > max_inliers)
                {
                    frac = ((double)count) / num;
                    pNoOutliers = 1 - pow(frac, min_num_samples);
                    pNoOutliers = pNoOutliers > 0 ? pNoOutliers : 0.000000001;
                    pNoOutliers = pNoOutliers < 1 ? pNoOutliers : 0.999999999;
                    d_N = SAFE_LOG(1-fs_prob) / SAFE_LOG(pNoOutliers);
                    N = (int)(d_N > INT_MAX ? INT_MAX:d_N);
                   /* pso("---%d %d----\n", count, N);*/
                }
                max_inliers = new_count;   
            }
        }

        try_num++;
        if( try_num == max_num_tries)
        {
            /*pso("Warning: RANSAC reaches the maxinum number of %d trials!\n",
                max_num_tries);*/
        }
    }
 
    free_matrix(A_mp);
    free_matrix(B_mp);
    free_matrix(a_mp);
    free_int_vector(index_ivp);

    return max_inliers;
}

int ransac_fit_affine
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
)
{
    int min_num_samples;

    fitting_func = fit_affine;
    dist_func = get_affine_distance;
    degen_func = is_affine_degenerate;

    if(x_mp->num_cols == 2)
    {
        min_num_samples = 3;
    }
    else
    {
        min_num_samples = 4;
    }

    return ransac_fit(x_mp, y_mp, max_num_tries, min_num_samples, good_fit_threshold, 
                      a_mpp, index_ivpp, fit_err_ptr, fitting_func, dist_func, degen_func);
}

int ransac_fit_constrained_affine
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int (*validate_constraints)(const Matrix *),
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
)
{
    int min_num_samples;

    fitting_func = fit_affine;
    dist_func = get_affine_distance;
    degen_func = is_affine_degenerate;

    if(x_mp->num_cols == 2)
    {
        min_num_samples = 3;
    }
    else
    {
        min_num_samples = 4;
    }

    return ransac_fit_constrained(x_mp, y_mp, validate_constraints, max_num_tries, min_num_samples, good_fit_threshold, 
        a_mpp, index_ivpp, fit_err_ptr);
}

int ransac_fit_similarity
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
)
{
    int min_num_samples = 2;

    fitting_func = fit_similarity;
    dist_func = get_similarity_distance;
    degen_func = is_similarity_degenerate;

    return ransac_fit(x_mp, y_mp, max_num_tries, min_num_samples, good_fit_threshold, 
                      a_mpp, index_ivpp, fit_err_ptr, fitting_func, dist_func, degen_func);
}


int ransac_fit_constrained_similarity
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int (*validate_constraints)(const Matrix *),
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
)
{
    int min_num_samples = 2;

    fitting_func = fit_similarity;
    dist_func = get_similarity_distance;
    degen_func = is_similarity_degenerate;

    return ransac_fit_constrained(x_mp, y_mp, validate_constraints, max_num_tries, min_num_samples, good_fit_threshold, 
        a_mpp, index_ivpp, fit_err_ptr);
}

int ransac_fit_homography
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
)
{
    fitting_func = fit_homography;
    dist_func = get_homography_distance;
    degen_func = is_homography_degenerate;

    return ransac_fit(x_mp, y_mp, max_num_tries, 4,
                      good_fit_threshold, a_mpp, index_ivpp, fit_err_ptr, fitting_func, dist_func, degen_func);
}

int ransac_fit_constrained_homography
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int (*validate_constraints)(const Matrix *),
    int          max_num_tries,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr
)
{
    fitting_func = fit_homography;
    /*dist_func = get_homography_distance;*/
    dist_func = get_dual_homography_distance;
    degen_func = is_homography_degenerate;

    return ransac_fit_constrained(x_mp, y_mp, validate_constraints, max_num_tries, 4,
        good_fit_threshold, a_mpp, index_ivpp, fit_err_ptr);
}



int main21(int argc, char *argv[])
{
    Matrix *x_mp = NULL;
    Matrix *y_mp = NULL;
    Matrix *a_mp = NULL;
    Matrix *x_fit_mp = NULL;
    Matrix *y_fit_mp = NULL;
    Int_vector *index_ivp = NULL;
    int i;
    double tmp;
    int id;
    KJB_image *src_ip = NULL;
    KJB_image *dst_ip = NULL;
    int num_rows, num_cols;
    int warp_type;
    Vector *dist_vp = NULL;
    Matrix *inv_mp = NULL;

    read_matrix(&x_mp, argv[1]);
    read_matrix(&y_mp, argv[2]);
    ERE(fit_affine(x_mp, y_mp, &a_mp, NULL));
    write_matrix(a_mp, 0);
    ERE(get_homography_distance(x_mp, y_mp, a_mp, &dist_vp));
    write_col_vector(dist_vp,0);
    ERE(get_matrix_inverse(&inv_mp, a_mp));
    ERE(get_homography_distance(y_mp, x_mp, inv_mp, &dist_vp));
    pso("\n");
    write_col_vector(dist_vp,0);
    
    exit(0);

    for(i=0; i<x_mp->num_rows; i++)
    {
        tmp = x_mp->elements[i][0];
        x_mp->elements[i][0] = x_mp->elements[i][1];
        x_mp->elements[i][1] =tmp;
        tmp = y_mp->elements[i][0];
        y_mp->elements[i][0] = y_mp->elements[i][1];
        y_mp->elements[i][1] =tmp;
    }

    warp_type = atoi(argv[4]);
    if(warp_type == HOMOGRAPHY)
    {
        ERE(ransac_fit_homography(x_mp, y_mp, 6000,  3.0, &a_mp, &index_ivp,
            NULL));
    }
    else
    {
        ERE(ransac_fit_affine(x_mp, y_mp, 6000, 3.0, &a_mp, &index_ivp,
            NULL)); }
    /* fit_homography(x_mp, y_mp, &a_mp, NULL);*/
    write_matrix(a_mp, 0);
    pso("--%d--\n", index_ivp->length);

   /* ERE(get_target_matrix(&x_fit_mp, index_ivp->length, 2));    
    ERE(get_target_matrix(&y_fit_mp, index_ivp->length, 2));
    for(i=0; i<index_ivp->length; i++)
    {
        id = index_ivp->elements[i];
        x_fit_mp->elements[i][0] = x_mp->elements[id][0];
        x_fit_mp->elements[i][1] = x_mp->elements[id][1];
        y_fit_mp->elements[i][0] = y_mp->elements[id][0];
        y_fit_mp->elements[i][1] = y_mp->elements[id][1];
    }*/
    
    num_rows = 341;
    num_cols = 442;
    kjb_read_image(&src_ip, argv[3]);
    
    ERE(transform_image(src_ip, a_mp, num_cols, num_rows, NULL, warp_type, &dst_ip, NULL));
    kjb_display_image(dst_ip, NULL);

    /*ERE(transform_image(src_ip, x_fit_mp, y_fit_mp, a_mp, inverse_affine,
    num_rows, num_cols, &dst_ip));*/
    /*ERE(transform_image(src_ip, x_fit_mp, y_fit_mp, a_mp, inverse_homography,
    num_rows, num_cols, &dst_ip));*/
    getchar();

    exit(0);

    /*if(is_degenerate(x_mp))
    {
        pso("Coliner!\n");
    }
    else
    {
        pso("Ok!\n");
    }*/
    
    exit(0);

    ERE(fit_affine(x_mp, y_mp, &a_mp, NULL));

    ERE(affine_transform(a_mp, x_mp, &y_fit_mp));
    pso("-----X------\n");
    write_matrix(x_mp, 0);
    pso("-----Y------\n");
    write_matrix(y_mp, 0);
    pso("----Affine----\n");
    write_matrix(a_mp, 0);
    pso("----Fit----\n");
    write_matrix(y_fit_mp, 0);

    ERE(ransac_fit_affine(x_mp, y_mp, 10, 0.5, &a_mp, &index_ivp, NULL));
    pso("----Affine----\n");
    write_matrix(a_mp, 0);
    
    free_matrix(a_mp);
    free_matrix(x_mp);
    free_matrix(y_mp);
    free_matrix(y_fit_mp);

    return 0;
}

