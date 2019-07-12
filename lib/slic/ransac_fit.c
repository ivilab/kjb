/* $Id: ransac_fit.c 21596 2017-07-30 23:33:36Z kobus $
 */
#include "slic/ransac_fit.h"

/* Kobus: I hope these commented out values are correct, because before they
 * were declared in ransac_fit.h but without value. 
*/
static int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *) = fit_homography; 
static int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **) = get_homography_distance; 
 static int (*degen_func)(const Matrix *) = is_homography_degenerate;

static double fs_minimum_inliers_percentage = NOT_SET; 
static double fs_acceptable_inliers_percentage = NOT_SET; 
static int fs_max_random_trials = 100; 
static double fs_inlier_prob = 0.99; /* Desired probability of choosing at least one
                              sample free from outliers */ 
static double fs_duplicates_dist_thresh = 5.0; /* how many px away should a match be to be an inlier */
static double fs_duplicates_thresh_percentage = 0.50; /* percentage of inliers that is allowed to be duplicated */

int set_ransac_options(const char* option, const char* value)
{
    int result = NOT_FOUND;
    char lc_option[ 100 ];
    double temp_dbl_value;

    EXTENDED_LC_BUFF_CPY( lc_option, option );

    if ( match_pattern( lc_option, "minimum-inliers-percentage" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The minimum percentage of RANSAC inliers is '%f'\n", fs_minimum_inliers_percentage );
        }
        else if (value[0] == '?')
        {
            pso( "minimum-inliers-percentage = %f\n", fs_minimum_inliers_percentage );
        }
        else
        {
            ERE( ss1d(value, &temp_dbl_value) );
            fs_minimum_inliers_percentage = temp_dbl_value;
        }
        result = NO_ERROR;
    }
    else if ( match_pattern( lc_option, "acceptable-inliers-percentage" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The minimum percentage of RANSAC inliers is '%f'\n", fs_acceptable_inliers_percentage );
        }
        else if (value[0] == '?')
        {
            pso( "acceptable-inliers-percentage = %f\n", fs_acceptable_inliers_percentage );
        }
        else
        {
            ERE( ss1d(value, &temp_dbl_value) );
            fs_acceptable_inliers_percentage = temp_dbl_value;
        }
        result = NO_ERROR;
    }
    else if ( match_pattern( lc_option, "duplicates-distance-threshold" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The distance threshold for inliers to not be a duplicate is '%f'\n", fs_duplicates_dist_thresh );
        }
        else if (value[0] == '?')
        {
            pso( "duplicates-distance-threshold = %f\n", fs_duplicates_dist_thresh );
        }
        else
        {
            ERE( ss1d(value, &temp_dbl_value) );
            fs_duplicates_dist_thresh = temp_dbl_value;
        }
        result = NO_ERROR;
    }
    else if ( match_pattern( lc_option, "duplicates-threshold-percentage" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The distance threshold for inliers to not be a duplicate is '%f'\n", fs_duplicates_thresh_percentage );
        }
        else if (value[0] == '?')
        {
            pso( "duplicates-threshold-percentage = %f\n", fs_duplicates_thresh_percentage );
        }
        else
        {
            ERE( ss1d(value, &temp_dbl_value) );
            fs_duplicates_thresh_percentage = temp_dbl_value;
        }
        result = NO_ERROR;
    }

    /* --------------------------------------------------------------------------------------------- */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int calculate_max_num_iterations(int min_num_samples);
int calculate_max_num_iterations(int min_num_samples)
{
  /* probability that any selected data point is an inlier, w */
  /* double frac = ((double)fs_minimum_num_inliers_percentage) / num_points; /\* (# of inliers) / (total # of points) *\/ */
  int max_num_iterations;
  double frac = fs_minimum_inliers_percentage;
  /* probability of an outlier, epsilon = 1 - w */ 
  double pNoOutliers = 1 - pow(frac, min_num_samples);
  pNoOutliers = pNoOutliers > 0 ? pNoOutliers : 0.000000001;
  pNoOutliers = pNoOutliers < 1 ? pNoOutliers : 0.999999999;
  /* N = log(1 - p) / log(1 - (1 - epsilon)^s) */
  max_num_iterations = SAFE_LOG(1 - fs_inlier_prob) / SAFE_LOG(pNoOutliers);
  max_num_iterations     = (int)(max_num_iterations > INT_MAX ? INT_MAX : max_num_iterations);

  return max_num_iterations;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


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
// index_ivpp         ---- a vector of inlier indices
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
int ransac_fit
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    int          min_num_samples,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr,
    int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *),
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **),
    int (*degen_func)(const Matrix *)
)
{
    int num; /* = x_mp->num_rows; */
    /*int num = x_mp->num_rows;*/
    Matrix *A_mp = NULL;
    Matrix *B_mp = NULL;

    Matrix *a_mp = NULL;
    Int_vector *index_ivp = NULL;
    int try_num = 0; /* counter for trials */
    int count;
    int new_count;
    int max_inliers = 0;
    int result;
    int N = INT_MAX; /* dummy initialization for number of trials */
    double pNoOutliers; /* probability that at least one of the random samples has no outliers */
    double frac;
    double d_N;
    double min_fit_err = DBL_HALF_MOST_POSITIVE;
    double fit_err;

    if (x_mp == NULL)
    {
        EGC(ERROR);
    }
    num = x_mp->num_rows;

    if(num < min_num_samples)
    {
       add_error("Warning: minimum number of samples is set to %d instead of the required %d\n", num, min_num_samples);
       warn_pso("Warning: minimum number of samples is set to %d instead of the required %d\n", num, min_num_samples);
       /*return ERROR;*/
       return 0;
    }

    /*ERE(get_target_int_vector(&index_ivp, num));*/
    result = get_target_int_vector(&index_ivp, num);
    EGC(result);

    verbose_pso(7, " | Starting RANSAC with max # tries = %d, N = %d, good fit threshold = %f\n", max_num_tries, N, good_fit_threshold);

    while(N > try_num && try_num < max_num_tries)
    {
     /*   pso("%d %d\n", N, try_num);*/
        verbose_pso(8, " | Try #%d out of %d\n", try_num, N);

        /* randomly pick the mininum required number of points */
        result = get_random_samples(x_mp, y_mp, min_num_samples, &A_mp, &B_mp, degen_func);
        /*if(result <= 0) break;*/
     if (result == 0)
        {
            warn_pso("Failed to pick %d samples.\n", min_num_samples);
            max_inliers = NOT_FOUND;
            if(fit_err_ptr != NULL) *fit_err_ptr = DBL_MAX;
            break;
        }
        else
        {
            verbose_pso(9, " | Successfully chose %d random samples.\n", result);
        }

        /* we have checked the validity of the picked samples. we double check
          here in case we still encounter problem fitting the samples */
        if(fitting_func(A_mp, B_mp, &a_mp, NULL) == ERROR)
        {
            pso("Warning: cannot fit the data!\n");
            try_num++;
            continue;
        }

        /* Now we can see how many points agree with this */
        count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp, dist_func); 
        
        if(count >= min_num_samples && count >= max_inliers) /* keep better result */
        {
            verbose_pso(8, " | Refitting using all %d inliers (max_inliers = %d).\n", count, max_inliers);
            verbose_pso(8, " | Get fitting data.\n");
            ERE(get_fitting_data(x_mp, y_mp, index_ivp, &A_mp, &B_mp));
            verbose_pso(8, " | Fitting data.\n");
            ERE(fitting_func(A_mp, B_mp, &a_mp, &fit_err));
            new_count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold,
                                    index_ivp, dist_func);
            verbose_pso(8, " | New inliers count = %d; fit error = %e (min fit = %e)\n", new_count, fit_err, min_fit_err);
            /* Here we break ties by using the minimum fitting error*/
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
                if(new_count > max_inliers)
                {
                    /* probability that any selected data point is an inlier, w */
                    frac = ((double)count) / num; /* (# of inliers) / (total # of points) */
                    /* probability of an outlier, epsilon = 1 - w */ 
                    pNoOutliers = 1 - pow(frac, min_num_samples);
                    pNoOutliers = pNoOutliers > 0 ? pNoOutliers : 0.000000001;
                    pNoOutliers = pNoOutliers < 1 ? pNoOutliers : 0.999999999;
                    /* N = log(1 - p) / log(1 - (1 - epsilon)^s) */
                    d_N = SAFE_LOG(1-fs_inlier_prob) / SAFE_LOG(pNoOutliers);
                    N = (int)(d_N > INT_MAX ? INT_MAX:d_N);
                    verbose_pso(8, " | N (# tries) after re-calculation = %d \n", N);
                }
               /* pso("---%d %d----\n", count, N);*/
                verbose_pso(8, " | Updating max_inliers from %d to %d. Min fit error = %e.\n", max_inliers, new_count, min_fit_err);
                max_inliers = new_count;   
            }
        }

        try_num++;
    }
    if( try_num == max_num_tries)
    {
        verbose_pso(8, " | RANSAC reached the maximum number of %d trials!\n", max_num_tries);
    }
    verbose_pso(7, " | Finished RANSAC with %d tries (max tries = %d, N = %d)\n", try_num-1, max_num_tries, N);
    verbose_pso(7, " | --------------------------------------------------\n");
 
cleanup: 
    free_matrix(A_mp);
    free_matrix(B_mp);
    free_matrix(a_mp);
    free_int_vector(index_ivp);

    /*pso("--------------------\n");*/
    return max_inliers;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
int ransac_fit_basic
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const int          min_num_samples,
    const double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr,
    int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *),
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **),
    int (*degen_func)(const Matrix *)
)
{
    int num; 
    Matrix *A_mp = NULL;
    Matrix *B_mp = NULL;
    Matrix *a_mp = NULL;
    Int_vector *index_ivp = NULL;
    Int_vector *current_best_index_ivp = NULL;
    int try_num = 0; /* counter for trials */
    int count = NOT_SET;
    int max_inliers = 0;
    int result;
    double min_fit_err = DBL_HALF_MOST_POSITIVE;
    double fit_err;
    double max_inlier_percentage = 0.0;
    int max_num_tries;
    int acceptable_num_inliers = NOT_SET; 

    if (x_mp == NULL)
    {
        EGC(ERROR);
    }
    num = x_mp->num_rows;

    if(num < min_num_samples)
    {
       add_error("Warning: minimum number of samples required is %d\n", min_num_samples);
       return 0;
    }

    ERE(get_target_int_vector(&index_ivp, num));
    ERE(get_target_int_vector(&current_best_index_ivp, num));
    
    if (fs_minimum_inliers_percentage < DBL_EPSILON)
    {
      fs_minimum_inliers_percentage = DEFAULT_MINIMUM_PERCENTAGE_OF_INLIERS;
      verbose_pso(7, " | Minimum percentage of RANSAC inliers is not set. Setting default.\n");
    }
    if (fs_acceptable_inliers_percentage != NOT_SET)
    {
        acceptable_num_inliers = fs_acceptable_inliers_percentage * num;
    }
    else
    {
        acceptable_num_inliers = num;
    }
    max_num_tries = calculate_max_num_iterations(min_num_samples);
    verbose_pso(7, " | Minimum percentage of RANSAC inliers is set to %e\n", fs_minimum_inliers_percentage);
    verbose_pso(7, " | RANSAC will run at most %d iterations\n", max_num_tries);
    if (fs_acceptable_inliers_percentage != NOT_SET)
    {
        verbose_pso(7, " | or until it finds %d or more inliers\n", acceptable_num_inliers);
    }

    /* By default, the expected number of inliers is set to NOT_SET, which is negative */
    /*
    while (   (   (fs_acceptable_inliers_percentage == NOT_SET)
               && (max_inlier_percentage < fs_acceptable_inliers_percentage)
              )
           || (try_num < max_num_tries) 
          )
          */
    while ( (count == NOT_SET || count < acceptable_num_inliers) && (try_num < max_num_tries) )
    {
        verbose_pso(8, " | Try #%d\n", try_num );

        /* randomly pick the mininum required number of points */
        result = get_random_samples(x_mp, y_mp, min_num_samples, &A_mp, &B_mp, degen_func);
        /*if(result <= 0) break;*/
        if (result == 0)
        {
            warn_pso("Failed to pick %d samples.\n", min_num_samples);
            max_inliers = NOT_FOUND;
            if(fit_err_ptr != NULL) *fit_err_ptr = DBL_MAX;
            break;
        }
        else
        {
            verbose_pso(9, " | Successfully chose %d random samples.\n", result);
        }

        /* we have checked the validity of the picked samples. we double check
          here in case we still encounter problem fitting the samples */
        /*if(fitting_func(A_mp, B_mp, &a_mp, NULL) == ERROR)*/
        if(fitting_func(A_mp, B_mp, &a_mp, &fit_err) == ERROR)
        {
            pso("Warning: cannot fit the data!\n");
            try_num++;
            continue;
        }

        /* Now we can see how many points agree with this */
        /*count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp, dist_func); */
        count = get_unique_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp, dist_func); 
        
       /* pso("%d %d\n", try_num, count);*/
        if(count >= min_num_samples && count > max_inliers) /* keep better result */
        {
            min_fit_err = fit_err;

            if(index_ivpp != NULL)
            {
               ERE(copy_int_vector(index_ivpp, index_ivp));
            }
            ERE(copy_int_vector(&current_best_index_ivp, index_ivp));


            if(a_mpp != NULL)
            {
                ERE(copy_matrix(a_mpp, a_mp));
            }

            if(fit_err_ptr != NULL) *fit_err_ptr = min_fit_err;

            /* pso("---%d %d----\n", count, N);*/
            verbose_pso(8, " | Updating max_inliers from %d to %d. Min fit error = %e.\n", 
                        max_inliers, count, min_fit_err);
            max_inlier_percentage = ((double)count)/num;
            max_inliers = count;

            /* /\* Since we are not refitting until the end, there is no such thing as breaking ties with error *\/ */
            /* if (count > max_inliers || (count == max_inliers && fit_err < min_fit_err)) */
            /* { */
            /*     min_fit_err = fit_err; */

            /*     if(index_ivpp != NULL) */
            /*     { */
            /*         ERE(copy_int_vector(index_ivpp, index_ivp)); */
            /*     } */

            /*     if(a_mpp != NULL) */
            /*     { */
            /*         ERE(copy_matrix(a_mpp, a_mp)); */
            /*     } */

            /*     if(fit_err_ptr != NULL) *fit_err_ptr = min_fit_err; */

            /*    /\* pso("---%d %d----\n", count, N);*\/ */
            /*     verbose_pso(8, " | Updating max_inliers from %d to %d. Min fit error = %e.\n",  */
            /*                 max_inliers, count, min_fit_err); */
            /*     max_inlier_percentage = ((double)count)/num; */
            /*     max_inliers = count; */
            /* } */
        }

        try_num++;
    }
    /* Re-estimate best guess*/
    if (max_inliers > 0)
    {
        verbose_pso(8, " | Refitting using all %d inliers (max_inliers = %d).\n", count, max_inliers);
        verbose_pso(8, " | Get fitting data.\n");
        ERE(get_fitting_data(x_mp, y_mp, current_best_index_ivp, &A_mp, &B_mp));
        verbose_pso(8, " | Fitting data.\n");
        ERE(fitting_func(A_mp, B_mp, &a_mp, &fit_err));
        verbose_pso(8, " | Fitting with model with maximum inliers has error %e (min fit = %e)\n", 
                    fit_err, min_fit_err);
        if(fit_err_ptr != NULL) *fit_err_ptr = fit_err;
    }
    else
    {
        warn_pso("max_inliers = %d!!!\n", max_inliers);
    }

    if( try_num == max_num_tries)
    {
        verbose_pso(8, " | RANSAC reached the maximum number of %d trials!\n", max_num_tries);
    }
    if (count >= acceptable_num_inliers)
    {
        verbose_pso(8, " | The max number of inliers (%d) is >= the early-exit number of inliers (%d).\n", 
                    count, acceptable_num_inliers);
    }
    /*
    if (fs_minimum_inliers_percentage <= fs_acceptable_inliers_percentage)
    {
        verbose_pso(8, " | The max percentage of inliers (%e) is <= the early-exit percentage of inliers (%e).\n", 
                    max_inlier_percentage, fs_acceptable_inliers_percentage);
    }
    */

cleanup:
    free_matrix(A_mp);
    free_matrix(B_mp);
    free_matrix(a_mp);
    free_int_vector(index_ivp);
    free_int_vector(current_best_index_ivp);

    pso("--------------------\n");
    return max_inliers;
}


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

// Note the best model returned is the one with the lowest fitting error, instead of the one with
// the minimum number of inliers. However, ties are broken by using the number of inliers.

// Refereneces:
// Richard Hartley and Andrew Zisserman. "Multiple View Geometry in Computer
// Vision". pp 101-113. Cambridge University Press, 2001
*/
int ransac_fit_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          max_num_tries,
    int          min_num_samples,
    double       good_fit_threshold,
    Matrix       **a_mpp,
    Int_vector   **index_ivpp,
    double       *fit_err_ptr,
    int          *num_inliers_ptr,
    int (*fitting_func)(const Matrix *, const Matrix *, Matrix **, double *),
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **),
    int (*degen_func)(const Matrix *)
)
{
    int num = x_mp->num_rows;
    Matrix *A_mp = NULL;
    Matrix *B_mp = NULL;
    Matrix *a_mp = NULL;
    Int_vector *index_ivp = NULL;
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
       add_error("Warning: minimum number of samples required is %d\n", min_num_samples);
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
        if(fitting_func(A_mp, B_mp, &a_mp, NULL) == ERROR)
        {
            pso("Warning: cannot fit the data!\n");
            try_num++;
            continue;
        }

        /* Now we can see how many points agree with this */
        count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp, dist_func); 

       /* pso("%d %d\n", try_num, count);*/
        if(count >= min_num_samples) 
        {
            /* refit using all the inliers */
            ERE(get_fitting_data(x_mp, y_mp, index_ivp, &A_mp, &B_mp));
            ERE(fitting_func(A_mp, B_mp, &a_mp, &fit_err));
            new_count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold,
                                    index_ivp, dist_func);

            /* Here we break ties by using the minimum fitting error*/
            if(fit_err < min_fit_err || ((fabs(fit_err - min_fit_err) < DBL_EPSILON && new_count > max_inliers)))
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
                if(num_inliers_ptr != NULL) *num_inliers_ptr = new_count;

                /* Update estimate of N, the number of trials to ensure we pick,
                // with probability p, a data set with no outliers */
                if(new_count > max_inliers)
                {
                    frac = ((double)count) / num;
                    pNoOutliers = 1 - pow(frac, min_num_samples);
                    pNoOutliers = pNoOutliers > 0 ? pNoOutliers : 0.000000001;
                    pNoOutliers = pNoOutliers < 1 ? pNoOutliers : 0.999999999;
                    d_N = SAFE_LOG(1-fs_inlier_prob) / SAFE_LOG(pNoOutliers);
                    N = (int)(d_N > INT_MAX ? INT_MAX:d_N);
                }
               /* pso("---%d %d----\n", count, N);*/
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

    pso("--------------------\n");
    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Qiyam: transferring this method from wrap_sift/ransac_fit.c
 *        I do not think it has been tested very much, so keep that
 *        in mind when using the method.
 *
 * KATE'S TRY ON THIS ROUTINE - SLIGHTLY DIFFERENT THAN THE ABOVE
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

Kate: I want the model with the smallest fitting error, not with most inliers. Not sure:
// Note the best model returned is the one with the most inliers, instead of the one with
// the minimum fitting error. However, ties are broken by using the fitting error.


// Refereneces:
// Richard Hartlye and Andere Zisserman. "Multiple View Geometry in Computer
// Vision". pp 101-113. Cambridge University Press, 2001
*/

int ransac_fit_2
(
    const Matrix *x_mp,
    const Matrix *y_mp,
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
    int try_num = 0; /* counter for trials */
    int count;
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
       add_error("Warning: minimum number of samples required is %d\n", min_num_samples);
       return 0;
    }

    ERE(get_target_int_vector(&index_ivp, num));
    while(N > try_num && try_num < max_num_tries)
    {
        /* randomly pick the mininum required number of points */
      result = get_random_samples(x_mp, y_mp, min_num_samples, &A_mp, &B_mp, degen_func);
      if(result <= 0) 
      {
        break;
      }
        /* we have checked the validity of the picked samples. we double check
       here in case we still encounter problem fitting the samples */
        if(fitting_func(A_mp, B_mp, &a_mp, &fit_err) == ERROR)
        {
            pso("Warning: cannot fit the data!\n");
            try_num++;
            continue;
        }

    if(fit_err < min_fit_err)
      {
        /* Now we can see how many points agree with this */
        count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp, dist_func);

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
        d_N = SAFE_LOG(1-fs_inlier_prob) / SAFE_LOG(pNoOutliers);
        N = (int)(d_N > INT_MAX ? INT_MAX:d_N);
          }
    
        max_inliers = count;
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
} /*end Kate's try on ransac_fit_squared_disance_2*/

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   
// Robustly fits a constrained model to data with the RANSAC algorithm.
// See routine 'ransac_fit' for more details.
*/
/* int ransac_fit_constrained */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int (*validate_constraints)(const Matrix *), */
/*     int          max_num_tries, */
/*     int          min_num_samples, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ) */
/* { */
/*     int num = x_mp->num_rows; */
/*     Matrix *A_mp = NULL; */
/*     Matrix *B_mp = NULL; */
/*     Matrix *a_mp = NULL; */
/*     Int_vector *index_ivp = NULL; */
/*     int i; */
/*     int try_num = 0; /\* counter for trials *\/ */
/*     int count; */
/*     int new_count; */
/*     int max_inliers = 0; */
/*     int result; */
/*     int N = INT_MAX; /\* dummy initialization for number of trials *\/ */
/*     double pNoOutliers; */
/*     double frac; */
/*     double d_N; */
/*     double min_fit_err = DBL_HALF_MOST_POSITIVE; */
/*     double fit_err; */

/*     if(num < min_num_samples) */
/*     { */
/*        add_error("Warning: minumum number of samples required is %d\n", min_num_samples); */
/*        return 0; */
/*     } */

/*     ERE(get_target_int_vector(&index_ivp, num)); */
/*     while(N > try_num && try_num < max_num_tries) */
/*     { */
/*      /\*   pso("%d %d\n", N, try_num);*\/ */

/*         /\* randomly pick the mininum required number of points *\/ */
/*         result = get_random_samples(x_mp, y_mp, min_num_samples, &A_mp, &B_mp); */
/*         if(result <= 0) break; */
/*        /\* write_matrix(A_mp, 0); */
/*         write_matrix(B_mp, 0);*\/ */

/*         /\* we have checked the validity of the picked samples. we double check */
/*           here in case we still encounter problem fitting the samples *\/ */
/*         ERE(fitting_func(A_mp, B_mp, &a_mp, NULL)); */
/*     /\*       if(fitting_func(A_mp, B_mp, &a_mp, NULL) == ERROR) */
/*         { */
/*         pso("Warning: cannot fit the data!\n"); */
/*             try_num++; */
/*             continue; */
/*         } */
/*     *\/ */

/*         /\* check if the constraints meet *\/ */
/*         if(validate_constraints != NULL && !validate_constraints(a_mp)) */
/*         { */
/*           /\*  pso("Warning: constraints are not met!\n");*\/ */
/*             try_num++; */
/*             continue; */
/*         } */

/*         /\* Now we can see how many points agree with this *\/ */
/*         count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp); */

/*        /\* pso("%d %d\n", try_num, count);*\/ */
/*         if(count >= min_num_samples && count >= max_inliers) /\* keep better result *\/ */
/*         { */
/*             /\* Here we break ties by using the minimum fitting error so it's */
/*              * more expensive *\/ */
/*             /\* refit using all the inliers *\/ */
/*             ERE(get_fitting_data(x_mp, y_mp, index_ivp, &A_mp, &B_mp)); */
/*             ERE(fitting_func(A_mp, B_mp, &a_mp, &fit_err)); */
/*             new_count = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, */
/*                 index_ivp); */
            
/*             if(new_count > max_inliers || fit_err < min_fit_err) */
/*             { */
/*                 min_fit_err = fit_err; */

/*                 if(index_ivpp != NULL) */
/*                 { */
/*                     ERE(copy_int_vector(index_ivpp, index_ivp)); */
/*                 } */

/*                 if(a_mpp != NULL) */
/*                 { */
/*                     ERE(copy_matrix(a_mpp, a_mp)); */
/*                 } */

/*                 if(fit_err_ptr != NULL) *fit_err_ptr = min_fit_err; */

/*             /\* Update estimate of N, the number of trials to ensure we pick, */
/*             // with probability p, a data set with no outliers *\/ */
/*                 if(count > max_inliers) */
/*                 { */
/*                     frac = ((double)count) / num; */
/*                     pNoOutliers = 1 - pow(frac, min_num_samples); */
/*                     pNoOutliers = pNoOutliers > 0 ? pNoOutliers : 0.000000001; */
/*                     pNoOutliers = pNoOutliers < 1 ? pNoOutliers : 0.999999999; */
/*                     d_N = SAFE_LOG(1-fs_inlier_prob) / SAFE_LOG(pNoOutliers); */
/*                     N = (int)(d_N > INT_MAX ? INT_MAX:d_N); */
/*                    /\* pso("---%d %d----\n", count, N);*\/ */
/*                 } */
/*                 max_inliers = new_count;    */
/*             } */
/*         } */

/*         try_num++; */
/*         if( try_num == max_num_tries) */
/*         { */
/*             /\*pso("Warning: RANSAC reaches the maxinum number of %d trials!\n", */
/*                 max_num_tries);*\/ */
/*         } */
/*     } */
 
/*     free_matrix(A_mp); */
/*     free_matrix(B_mp); */
/*     free_matrix(a_mp); */
/*     free_int_vector(index_ivp); */

/*     return max_inliers; */
/* } */

/* int ransac_fit_affine */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int          max_num_tries, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ) */
/* { */
/*     int min_num_samples; */

/*     fitting_func = fit_affine; */
/*     dist_func = get_affine_distance; */
/*     degen_func = is_affine_degenerate; */

/*     if(x_mp->num_cols == 2) */
/*     { */
/*         min_num_samples = 3; */
/*     } */
/*     else */
/*     { */
/*         min_num_samples = 4; */
/*     } */

/*     return ransac_fit(x_mp, y_mp, max_num_tries, min_num_samples, good_fit_threshold,  */
/*         a_mpp, index_ivpp, fit_err_ptr); */
/* } */

/* int ransac_fit_constrained_affine */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int (*validate_constraints)(const Matrix *), */
/*     int          max_num_tries, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ) */
/* { */
/*     int min_num_samples; */

/*     fitting_func = fit_affine; */
/*     dist_func = get_affine_distance; */
/*     degen_func = is_affine_degenerate; */

/*     if(x_mp->num_cols == 2) */
/*     { */
/*         min_num_samples = 3; */
/*     } */
/*     else */
/*     { */
/*         min_num_samples = 4; */
/*     } */

/*     return ransac_fit_constrained(x_mp, y_mp, validate_constraints, max_num_tries, min_num_samples, good_fit_threshold,  */
/*         a_mpp, index_ivpp, fit_err_ptr); */
/* } */

/* int ransac_fit_similarity */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int          max_num_tries, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ) */
/* { */
/*     int min_num_samples = 2; */

/*     fitting_func = fit_similarity; */
/*     dist_func = get_similarity_distance; */
/*     degen_func = is_similarity_degenerate; */

/*     return ransac_fit(x_mp, y_mp, max_num_tries, min_num_samples, good_fit_threshold,  */
/*         a_mpp, index_ivpp, fit_err_ptr); */
/* } */


/* int ransac_fit_constrained_similarity */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int (*validate_constraints)(const Matrix *), */
/*     int          max_num_tries, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ) */
/* { */
/*     int min_num_samples = 2; */

/*     fitting_func = fit_similarity; */
/*     dist_func = get_similarity_distance; */
/*     degen_func = is_similarity_degenerate; */

/*     return ransac_fit_constrained(x_mp, y_mp, validate_constraints, max_num_tries, min_num_samples, good_fit_threshold,  */
/*         a_mpp, index_ivpp, fit_err_ptr); */
/* } */

/* int ransac_fit_homography */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int          max_num_tries, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ) */
/* { */
/*     fitting_func = fit_homography; */
/*     dist_func = get_homography_distance; */
/*     degen_func = is_homography_degenerate; */

/*     return ransac_fit(x_mp, y_mp, max_num_tries, 4, */
/*         good_fit_threshold, a_mpp, index_ivpp, fit_err_ptr); */
/* } */

/* int ransac_fit_constrained_homography */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int (*validate_constraints)(const Matrix *), */
/*     int          max_num_tries, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ) */
/* { */
/*     fitting_func = fit_homography; */
/*     /\*dist_func = get_homography_distance;*\/ */
/*     dist_func = get_dual_homography_distance; */
/*     degen_func = is_homography_degenerate; */

/*     return ransac_fit_constrained(x_mp, y_mp, validate_constraints, max_num_tries, 4, */
/*         good_fit_threshold, a_mpp, index_ivpp, fit_err_ptr); */
/* } */

/* int ransac_fit_fundamental_matrix */
/* ( */
/*     const Matrix *x_mp, */
/*     const Matrix *y_mp, */
/*     int          max_num_tries, */
/*     double       good_fit_threshold, */
/*     Matrix       **a_mpp, */
/*     Int_vector   **index_ivpp, */
/*     double       *fit_err_ptr */
/* ) */
/* { */
/*     int min_num_samples; */

/*     fitting_func = fit_fundamental_matrix; */
/*     dist_func = get_fundamental_matrix_distance; */
/*     degen_func = is_fundamental_matrix_degenerate; */

/*     min_num_samples = 8; */

/*     return ransac_fit(x_mp, y_mp, max_num_tries, min_num_samples, good_fit_threshold,  */
/*         a_mpp, index_ivpp, fit_err_ptr); */
/* } */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   
// Iteratively fits a model to data given an initial estimation
// (roughly correct).
*/
int iterative_fit
(
    const Matrix *x_mp,
    const Matrix *y_mp,
   /* int (*validate_constraints)(const Matrix *),*/
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
    int fit_num = INT_MAX;
    int old_fit_num = -1;
    int count;
    double fit_err;

    if(a_mpp == NULL || *a_mpp == NULL)
    {
        add_error("Initial estimation is required!\n");
        return ERROR;
    }

    ERE(get_target_int_vector(&index_ivp, num));
    ERE(copy_matrix(&a_mp, *a_mpp));

    old_fit_num = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp, dist_func);
    
    count = 0;
    while(fit_num > old_fit_num)
    {
        old_fit_num = fit_num;
        ERE(get_fitting_data(x_mp, y_mp, index_ivp, &A_mp, &B_mp));
        ERE(fitting_func(A_mp, B_mp, &a_mp, &fit_err));
        fit_num = get_inliers(x_mp, y_mp, a_mp, good_fit_threshold, index_ivp, dist_func);
        pso("Iteration ---- %d %d %d\n", count, old_fit_num, fit_num);
        count++;
    }

    ERE(copy_matrix(a_mpp, a_mp));
    if(index_ivpp != NULL)
    {
        ERE(copy_int_vector(index_ivpp, index_ivp));
    }

    if(fit_err_ptr != NULL) *fit_err_ptr = fit_err;

    free_matrix(A_mp);
    free_matrix(B_mp);
    free_matrix(a_mp);
    free_int_vector(index_ivp);

    return fit_num;
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
int get_inliers
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp,
    double       good_fit_threshold,
    Int_vector   *index_ivp,
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **)
)
{
    Vector *dist_vp = NULL;
    int count;
    int i;
    
    /* Now we can see how many points agree with this */
    ERE(dist_func(x_mp, y_mp, a_mp, &dist_vp));

    count = 0;
    for(i=0; i<dist_vp->length; i++)
    {
        if(dist_vp->elements[i] <= good_fit_threshold) /* good fit */
        {
            index_ivp->elements[count++] = i;
        }
    }
    index_ivp->length = count;

    free_vector(dist_vp);
    return count;
}

        
/* Copy the values/rows from x_mp and y_mp
 * using the indices provided in index_ivp */
int get_fitting_data
(
    const Matrix     *x_mp,
    const Matrix     *y_mp,
    const Int_vector *index_ivp,
    Matrix       **fit_x_mpp,
    Matrix       **fit_y_mpp
)
{
    int m = index_ivp->length;
    int dim = x_mp->num_cols;
    Matrix *fit_x_mp = NULL;
    Matrix *fit_y_mp = NULL;
    int id;
    int i;
    int k;

    if(fit_x_mpp == NULL || fit_y_mpp == NULL) return NO_ERROR;

    ERE(get_target_matrix(fit_x_mpp, m, dim));
    ERE(get_target_matrix(fit_y_mpp, m, dim));
    fit_x_mp = *fit_x_mpp;
    fit_y_mp = *fit_y_mpp;

    for(i=0; i<m; i++)
    {
        id = index_ivp->elements[i];
        for(k=0; k<dim; k++)
        {
            fit_x_mp->elements[i][k] = x_mp->elements[id][k];
            fit_y_mp->elements[i][k] = y_mp->elements[id][k];
        }
    }

    return NO_ERROR;
}

int get_random_samples
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    int          m,
    Matrix       **rand_x_mpp,
    Matrix       **rand_y_mpp,
    int (*degen_func)(const Matrix *)    
)
{
    Int_vector *index_ivp = NULL;
    int num = NOT_SET;
    int dim = NOT_SET;
    Matrix *rand_x_mp = NULL;
    Matrix *rand_y_mp = NULL;
    int id;
    int i;
    int k;
    int count = 0;

    if(rand_x_mpp == NULL || rand_y_mpp == NULL) return NO_ERROR;

    if( m <= 0 )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(x_mp->num_rows != y_mp->num_rows || 
       x_mp->num_cols != y_mp->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num = x_mp->num_rows;
    dim = x_mp->num_cols;

    ERE(get_target_matrix(rand_x_mpp, m, dim));
    ERE(get_target_matrix(rand_y_mpp, m, dim));
    rand_x_mp = *rand_x_mpp;
    rand_y_mp = *rand_y_mpp;

    while(count < fs_max_random_trials)
    {
        ERE(generate_random_index(m, num, &index_ivp));

        for(i=0; i<m; i++)
        {
            id = index_ivp->elements[i];
            for(k=0; k<dim; k++)
            {
                rand_x_mp->elements[i][k] = x_mp->elements[id][k];
                rand_y_mp->elements[i][k] = y_mp->elements[id][k];
            }
        }

        if(degen_func == NULL)
        {
            /*warn_pso("The function to check degeneracy is NULL!\n");*/
            break;
        }

        /* for affine and projective transformation, if 3 points of x or y are
         * colinear, the problem is degenerate. */
        if(!degen_func(rand_x_mp) && !degen_func(rand_y_mp)) break;
        count++;
    }

    free_int_vector(index_ivp);

    if(count == fs_max_random_trials)
    {
       /* pso("Warning: reached maximum trials and still failed to pick
        * non-degenerate set of samples!\n");*/
        free_matrix(*rand_x_mpp); 
        free_matrix(*rand_y_mpp);
        *rand_x_mpp = NULL;
        *rand_y_mpp = NULL;
        return 0;
    }
    
    return m;
}

/* Generate m non-duplicate numbers no larger than n */
static int generate_random_index
(
    int              m,
    int              n,
    Int_vector       **index_ivpp
)
{
    int current_picked;
    int current_left;
    int a_pos;
    double r;
    int tmp;
    int *id_ptr;
    int *a;
    int i;

    ASSERT(m > 0 && m <= n);


    if(index_ivpp == NULL) return NO_ERROR;

    ERE(get_target_int_vector(index_ivpp, m));
 
    NRE(a = N_TYPE_MALLOC(int, n));
    for(i=0; i<n; i++)
    {
        a[i] = i;
    }

    current_left = m;
    id_ptr = (*index_ivpp)->elements;
    a_pos = n;
    while(current_left > 0)
    {
        r = kjb_rand();
        current_picked = (int)( r * a_pos);
        if(current_picked == a_pos) current_picked--;

        *(id_ptr++) = a[current_picked];   
        current_left--;
        a_pos--;

        /* swap the current picked elements and the last one not picked yet*/
        tmp = a[current_picked];
        a[current_picked] = a[a_pos];
        a[a_pos] = tmp;
    }

    kjb_free(a);

    return NO_ERROR;
}


/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* Project points from x_mp using a_mp and compute 
 * the distance between the result and y_mp using
 * good_fit_threshold. Save the indices of the inliers
 * into index_ivp (whose size will be the same as the
 * number of inliers).
 * Finally, check that the selected inliers are not
 * degenerate by counting how many are too close to 
 * each other.
 * */
int get_unique_inliers
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp,
    double       good_fit_threshold,
    Int_vector   *index_ivp,
    int (*dist_func)(const Matrix *, const Matrix *, const Matrix *, Vector **)
)
{
    Vector *dist_vp = NULL;
    Int_vector *inlier_index_ivp = NULL;
    int count;
    int i;
    int j;

    int pt;
    int next;
    int num_duplicates = 0;
    int found_duplicate = FALSE;

    if (index_ivp == NULL)
    {
        ERE(ERROR);
    }

    if (x_mp == NULL || y_mp == NULL || a_mp == NULL)
    {
        return NO_ERROR;
    }

    /* Now we can see how many points agree with this */
    verbose_pso(9," | Checking how many points agree with the model...\n");
    verbose_pso(9," | x has %d points, y has %d points...\n", x_mp->num_rows, y_mp->num_rows);
    ERE(dist_func(x_mp, y_mp, a_mp, &dist_vp));

    count = 0;
    for(i = 0; i < dist_vp->length; i++)
    {
        if(dist_vp->elements[i] <= good_fit_threshold) /* good fit */
        {
            index_ivp->elements[count++] = i;
            verbose_pso(9," | Found a good fit: distance %e\n", dist_vp->elements[i]);
        }
    }
    index_ivp->length = count;

    verbose_pso(8, " | Checking the validity of %d inliers.\n", count);
    ERE( copy_int_vector( &inlier_index_ivp, index_ivp ) );
    /* Start with the first inlier and mark how many of the other inliers
       are too close to it - mark them, so that you don't use them again */
    for (i = 0; i < count - 1; i++) 
    {
        if (inlier_index_ivp->elements[i] != NOT_SET)
        {
            pt = inlier_index_ivp->elements[i] ;
            for (j = i+1; (j < count); j++)
            {
                if (inlier_index_ivp->elements[j] != NOT_SET)
                {
                    next = inlier_index_ivp->elements[j];
                    if ( ((fabs(x_mp->elements[pt][0] - x_mp->elements[next][0]) <= fs_duplicates_dist_thresh )
                          && (fabs(x_mp->elements[pt][1] - x_mp->elements[next][1]) <= fs_duplicates_dist_thresh))
                         || ((fabs(y_mp->elements[pt][0] - y_mp->elements[next][0]) <= fs_duplicates_dist_thresh)
                             && (fabs(y_mp->elements[pt][1] - y_mp->elements[next][1]) <= fs_duplicates_dist_thresh)) 
                       )
                    {
                        verbose_pso(9, " | identical inliers %d and %d\n", pt, next);
                        inlier_index_ivp->elements[j] = NOT_SET;
                        found_duplicate = TRUE;
                        num_duplicates++;
                    }
                }
            }
        }
        /* If the current kpt had at least one duplicate, count it in the total */
        if (found_duplicate == TRUE)
        {
            inlier_index_ivp->elements[i] = NOT_SET; /* not really needed, since we are not going back to i */
            found_duplicate = FALSE;
            num_duplicates++;
        }
    }

    /*if (num_duplicates >= (index_ivp->length/2.0))*/
    if (num_duplicates >= fs_duplicates_thresh_percentage * (index_ivp->length))
    {
        verbose_pso(9, " | Degenerate case: too many duplicate points (%d out of %d)!\n", num_duplicates, count);
        count = NOT_SET;
    }

    free_vector(dist_vp);
    free_int_vector(inlier_index_ivp);
    return count;
}
