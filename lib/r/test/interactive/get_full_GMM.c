
/* $Id: get_full_GMM.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003, by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

/*
//  Most programs need at least the "m" library. 
*/

#include "m/m_incl.h" 
#include "r/r_incl.h" 
#include "sample/sample_gauss.h" 


/* -------------------------------------------------------------------------- */

#define NUM_CLUSTERS  5
#define NUM_POINTS    1000
#define NUM_FEATURES  8

/* -------------------------------------------------------------------------- */

static double fs_data_perturbation                   = DBL_NOT_SET;
static int    fs_num_tries_per_cluster_count         = 1;
static int    fs_max_num_iterations                  = 20;
static double fs_iteration_tolerance                 = 1.0e-6;
static int fs_normalize_data = FALSE;    /* Switched Dec 21, 2004. */
static double fs_var_offset               = 0.0001;

/* -------------------------------------------------------------------------- */

static int old_do_fixed_ind_con_em
(
    int           num_clusters,
    const Matrix* feature_mp,
    Matrix**      P_mpp,
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Vector**      a_vpp
);

static int old_do_fixed_ind_con_em_guts
(
    int           num_clusters,
    const Matrix* feature_arg_mp,
    Matrix**      P_mpp,
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Vector**      a_vpp, 
    double*       log_likelihood_ptr 
);

/* -------------------------------------------------------------------------- */

int main(int argc, char** argv)
{
    Matrix* P_mp = NULL; 
    Vector* P_vp = NULL; 
    Matrix* data_mp = NULL; 
    Matrix* mean_mp = NULL;
    Matrix* var_mp  = NULL; 
    Matrix* est_var_mp  = NULL; 
    Matrix* est_mean_mp = NULL;
    Vector* mean_vp = NULL;
    Matrix_vector* est_cov_mvp = NULL; 
    Vector* var_vp = NULL; 
    Vector* a_vp = NULL; 
    int cluster; 
    double p; 
    int i,j;
    IMPORT int kjb_debug_level;

    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    kjb_set_verbose_level(5);
    kjb_debug_level = 10;

    EPETE(get_target_matrix(&data_mp, NUM_POINTS, NUM_FEATURES));

    EPETE(get_target_matrix(&mean_mp, NUM_CLUSTERS, NUM_FEATURES));
    EPETE(get_target_matrix(&var_mp, NUM_CLUSTERS, NUM_FEATURES));

    kjb_seed_rand(0,0);
    kjb_seed_rand_2(0);

    for (cluster = 0; cluster < NUM_CLUSTERS; cluster++)
    {
        ERE(get_random_vector(&mean_vp, NUM_FEATURES));
        ERE(put_matrix_row(mean_mp, mean_vp, cluster)); 
        ERE(get_random_vector(&var_vp, NUM_FEATURES));
        ERE(put_matrix_row(var_mp, var_vp, cluster)); 
    }

    for (i = 0; i < NUM_POINTS; i++)
    {
        cluster = NUM_CLUSTERS * kjb_rand(); 

        for (j = 0; j < NUM_FEATURES; j++)
        {
            p = gauss_rand(); 
            p *= sqrt(var_mp->elements[ cluster ][ j ]);
            p += mean_mp->elements[ cluster ][ j ]; 

            data_mp->elements[ i ][ j ] = p; 
        }
    }

    EPETE(get_independent_GMM(NUM_CLUSTERS, data_mp, 
                              (Vector**)NULL,
                              &est_mean_mp,
                              &est_var_mp, &P_mp)); 

    dbp("\n---------------------------------------------\n");

    db_mat(mean_mp);

    dbp("\n---------------------------------------------\n");

    db_mat(est_mean_mp);

    dbp("\n---------------------------------------------\n");

    dbe(rms_matrix_difference(mean_mp, est_mean_mp)); 
    dbe(rms_matrix_difference(var_mp, est_var_mp)); 
    dbe(max_abs_matrix_difference(mean_mp, est_mean_mp)); 
    dbe(max_abs_matrix_difference(var_mp, est_var_mp)); 
    db_rv(a_vp); 

    EPETE(get_matrix_row(&P_vp, P_mp, 0));
    db_rv(P_vp);

    /*
    db_mat(mean_mp);
    db_mat(var_mp);
    db_rv(a_vp); 
    */

    kjb_seed_rand(0,0);
    kjb_seed_rand_2(0);

    EPETE(old_do_fixed_ind_con_em(NUM_CLUSTERS, data_mp, &P_mp , &est_mean_mp, 
                                  &est_var_mp, &a_vp)); 

    dbp("\n---------------------------------------------\n");

    db_mat(est_mean_mp);

    dbp("\n---------------------------------------------\n");

    dbe(rms_matrix_difference(mean_mp, est_mean_mp)); 
    dbe(rms_matrix_difference(var_mp, est_var_mp)); 
    dbe(max_abs_matrix_difference(mean_mp, est_mean_mp)); 
    dbe(max_abs_matrix_difference(var_mp, est_var_mp)); 
    db_rv(a_vp); 

    EPETE(get_matrix_row(&P_vp, P_mp, 0));
    db_rv(P_vp);


    /*
    db_mat(mean_mp);
    db_mat(var_mp);
    db_rv(a_vp); 
    */

    kjb_seed_rand(0,0);
    kjb_seed_rand_2(0);

    EPETB(get_full_GMM(NUM_CLUSTERS, 
                       data_mp, 
                       (const Matrix*)NULL, 
                       (Vector**)NULL,
                       &est_mean_mp,
                       &est_cov_mvp, 
                       &P_mp)); 

    dbp("\n---------------------------------------------\n");

    db_mat(mean_mp);

    dbp("\n---------------------------------------------\n");

    db_mat(est_mean_mp);

    dbp("\n---------------------------------------------\n");

    dbe(rms_matrix_difference(mean_mp, est_mean_mp)); 
    dbe(rms_matrix_difference(var_mp, est_var_mp)); 
    dbe(max_abs_matrix_difference(mean_mp, est_mean_mp)); 
    dbe(max_abs_matrix_difference(var_mp, est_var_mp)); 
    db_rv(a_vp); 

    EPETE(get_matrix_row(&P_vp, P_mp, 0));
    db_rv(P_vp);

    /*
    db_mat(mean_mp);
    db_mat(var_mp);
    db_rv(a_vp); 
    */

    free_matrix(data_mp);
    free_matrix(mean_mp);
    free_matrix(var_mp);
    free_vector(a_vp); 
    free_vector(mean_vp); 
    free_vector(var_vp); 
    free_matrix(est_mean_mp); 
    free_matrix(est_var_mp); 
    free_matrix_vector(est_cov_mvp); 
    free_matrix(P_mp); 
    free_vector(P_vp); 

    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int old_do_fixed_ind_con_em
(
    int           num_clusters,
    const Matrix* feature_mp,
    Matrix**      P_mpp,
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Vector**      a_vpp
)
{
    int        num_points   = feature_mp->num_rows;
    int        num_features = feature_mp->num_cols;
    int        j, k;
    double     max_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    Matrix*    P_mp     = NULL;
    Matrix*    means_mp = NULL;
    Matrix*    var_mp = NULL;
    Vector*    a_vp = NULL; 
    Matrix*    perturbed_feature_mp = NULL;
    double     encoding_cost;
    int        num_outliers;
    int        result        = NO_ERROR;
    Matrix*    norm_feature_mp = NULL; 
    Vector*    feature_mean_vp = NULL;
    Vector*    feature_var_vp = NULL;
    const double norm_stdev = 1.0; 


    if ((result != ERROR) && (fs_data_perturbation > 10.0 * DBL_EPSILON))
    {
        result = perturb_unary_cluster_data(&perturbed_feature_mp, 
                                            feature_mp, fs_data_perturbation,
                                            (Int_vector*)NULL); 
        feature_mp = perturbed_feature_mp; 
    }

    if (fs_normalize_data)
    {
        result = copy_matrix(&norm_feature_mp, feature_mp);

        if (result != ERROR)
        {
            result = ow_normalize_cluster_data(norm_feature_mp,
                                               (Matrix*)NULL,
                                               &feature_mean_vp, 
                                               &feature_var_vp,
                                               norm_stdev,
                                               (Int_vector*)NULL);
        }

        if (result != ERROR)
        {
            feature_mp = norm_feature_mp; 
        }
    }

    for (j = 0; j < fs_num_tries_per_cluster_count; j++)
    {
        double log_likelihood;

        if (result == ERROR) { NOTE_ERROR(); break; }

        num_clusters = old_do_fixed_ind_con_em_guts(num_clusters, 
                                               feature_mp, 
                                               &P_mp, 
                                               &means_mp, &var_mp, &a_vp, 
                                               &log_likelihood); 
        if (num_clusters == ERROR)
        {
            result = ERROR;
            break; 
        }

        num_outliers = 0;

        for (k = 0; k < num_points; k++)
        {
            if (P_mp->elements[ k ][ 0 ] < 0.0) num_outliers++;
        }

        verbose_pso(2,
                    "Raw log likelihood with %d clusters is %.5e,\n", 
                    num_clusters, log_likelihood);

        encoding_cost = 1.5 * num_clusters * num_features;
        encoding_cost += 0.5 * num_outliers * num_features;
        encoding_cost *= log((double)(num_points - num_outliers)); 

        log_likelihood -= encoding_cost; 

        verbose_pso(2,
                "Adjusted log likelihood with %d clusters is %.5e,\n",
                num_clusters, log_likelihood);

        if (log_likelihood > max_log_likelihood)
        {
            max_log_likelihood = log_likelihood; 

            if (fs_normalize_data)
            {
                result = ow_un_normalize_cluster_data(means_mp, var_mp, 
                                                      feature_mean_vp, 
                                                      feature_var_vp, 
                                                      norm_stdev); 
            }

            if ((result != ERROR) && (P_mpp != NULL))
            {
                result = copy_matrix(P_mpp, P_mp);
            }

            if ((result != ERROR) && (means_mpp != NULL))
            {
                result = copy_matrix(means_mpp, means_mp); 
            }

            if ((result != ERROR) && (var_mpp != NULL))
            {
                result = copy_matrix(var_mpp, var_mp); 
            }

            if ((result != ERROR) && (a_vpp != NULL))
            {
                result = copy_vector(a_vpp, a_vp); 
            }
        }

        free_vector(a_vp);
        a_vp = NULL;
        free_matrix(means_mp);
        means_mp = NULL;
        free_matrix(var_mp);
        var_mp = NULL;
        free_matrix(P_mp);
        P_mp = NULL; 
    }

    free_matrix(norm_feature_mp);
    free_vector(feature_var_vp); 
    free_vector(feature_mean_vp); 

    free_matrix(perturbed_feature_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef RESCALE
#define feature_arg_mp feature_mp
#endif 

static int old_do_fixed_ind_con_em_guts
(
    int           num_clusters,
    const Matrix* feature_mp,
    Matrix**      P_mpp,
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Vector**      a_vpp, 
    double*       log_likelihood_ptr 
)
{
    int           num_points          = feature_arg_mp->num_rows;
    int           num_features        = feature_arg_mp->num_cols;
    Matrix*       rescaled_feature_mp = NULL;
    int           cluster;
    int           it, i, j;
    Matrix*       P_mp        = NULL;
    Int_vector*   outlier_vp  = NULL;
    Vector*       a_vp        = NULL;
    Vector*       p_sum_vp    = NULL;
    Matrix*       S_mp        = NULL;
    Vector*       S_vp        = NULL;
    Vector*       x_vp        = NULL;
    Vector*       d_vp        = NULL;
    double        dot_product;
    double        log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        diff;
    Matrix*       u_mp    = NULL;
    Vector*       u_vp    = NULL;
    Vector*       temp_vp = NULL;
    Vector*       var_vp  = NULL;
    int           num_good_points;
    int           result       = NO_ERROR;
#ifdef USE_LOGS
    Vector*       const_vp     = NULL;
#else
    double        det;
    double        sqrt_det;
    Vector*       sqrt_det_vp  = NULL;
#endif
#ifdef RESCALE
    Vector*       ave_sigma_vp = NULL;
    Matrix*       feature_mp;
    int           rescaled_on_previous_iteration = FALSE;
#endif


    dbi(num_clusters); 

#ifdef RESCALE
    /*
    // In the rescale scenario, we want to change the feature matrix, even if we
    // did not perturb it. 
    */
    else 
    {
        result = copy_matrix(&rescaled_feature_mp, feature_arg_mp);
        feature_mp = rescaled_feature_mp; 
    }
#endif 

    num_good_points = num_points; 

    if (    (get_cluster_random_matrix(&P_mp, num_points, num_clusters) == ERROR) 
         || (get_target_vector(&a_vp, num_clusters)             == ERROR) 
#ifdef USE_LOGS
         || (get_target_vector(&const_vp, num_clusters)         == ERROR) 
#else 
         || (get_target_vector(&sqrt_det_vp, num_clusters)      == ERROR) 
#endif 
         || (get_target_vector(&p_sum_vp, num_clusters)         == ERROR) 
         || (get_initialized_int_vector(&outlier_vp, num_points, FALSE)==ERROR) 
       )
    {
        result = ERROR;
    }

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        int backed_off_during_this_iteration = FALSE; 

        if (result == ERROR) { NOTE_ERROR(); break; }
        
        /*CONSTCOND*/
        while (TRUE)    /* We try_count to back of each cluster. */
        {   
            int back_off = NOT_SET; 

            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef DEF_OUT_UNTIL_OPTION
            /*
            // Disable back off until we make it an option.
            */
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double meat = 0.0; 

                for (i = 0; i < num_points; i++)
                {
                    if ( ! outlier_vp->elements[ i ])
                    {
                        meat += P_mp->elements[ i ][ cluster ]; 
                    }
                }

                if (meat < 1.5)
                {
                    back_off = cluster; 
                    break; 
                }
            }
#endif 

            if (IS_SET(back_off))
            {
                int bad_cluster = back_off; 

                backed_off_during_this_iteration = TRUE; 

                verbose_pso(2, "Backing off cluster %d.\n", bad_cluster); 

                num_clusters--;
                
                if (num_clusters == 0)
                {
                    set_bug("All clusters have been removed."); 
                    result = ERROR; 
                    break; 
                }

                result = remove_matrix_col((Vector**)NULL, P_mp, bad_cluster);
                if (result == ERROR) { NOTE_ERROR(); break; } 

                result = remove_matrix_col((Vector**)NULL, P_mp, bad_cluster);
                if (result == ERROR) { NOTE_ERROR(); break; } 

                (a_vp->length)--;
                (p_sum_vp->length)--;
#ifdef USE_LOGS
                (const_vp->length)--;
#else 
                (sqrt_det_vp->length)--; 
#endif 

                for (i = 0; i<num_points; i++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    if ( ! outlier_vp->elements[ i ])
                    {
                        double p_sum = sum_matrix_row_elements(P_mp, i);

                        if (p_sum < 1e10 * DBL_MIN)  /* This is taken to be an outlier. */
                        {
                            verbose_pso(2, "Point %d is taken as an outlier.\n",
                                        i + 1);

                            outlier_vp->elements[ i ] = TRUE; 
                            num_good_points--; 
                    
                            if (num_good_points == 0)
                            {
                                set_bug("All points are outliers."); 
                                result = ERROR; 
                                break; 
                            }
                        }
                        else 
                        {
                            result = ow_divide_matrix_row_by_scalar(P_mp, p_sum,
                                                                    i);
                        }
                    }
                }
            }
            else 
            {
                break;  /* No more clusters to back off. */
            }
        }

        /* M-Step */

        if (result != ERROR)
        {
            result = get_zero_matrix(&u_mp, num_clusters, num_features);
        }

        /*
        // Calculate the means. 
        */
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            p_sum_vp->elements[ cluster ] = 0.0; 

            for (i = 0; i < num_points; i++)
            {
                if ( ! outlier_vp->elements[ i ])
                {
                    double  p = P_mp->elements[ i ][ cluster ];

                    p_sum_vp->elements[ cluster ] += p; 

                    result = get_matrix_row(&x_vp, feature_mp, i);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = ow_multiply_vector_by_scalar(x_vp, p); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = ow_add_vector_to_matrix_row(u_mp, x_vp, cluster); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 
                }
            }
        }

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = divide_vector_by_scalar(&a_vp, p_sum_vp, 
                                         (double)num_good_points);
        if (result == ERROR) { NOTE_ERROR(); break; }

        if (ABS_OF(sum_vector_elements(a_vp) - 1.0) > 0.00001) 
        {
            warn_pso("Sum of mixing weights is %f.\n",
                     sum_vector_elements(a_vp));
        }

        if (ow_divide_matrix_by_col_vector(u_mp, p_sum_vp) == ERROR)
        {
            db_rv(p_sum_vp); 
            result = ERROR;
            break; 
        }

        /*
        // Calculate the new variances/covariances. We can't merge it with the
        // above because it depends on the calculation of the means.
        */

        result = get_initialized_matrix(&S_mp, num_clusters, num_features,
                                        0.0);
        if (result == ERROR) { NOTE_ERROR(); break; }
                
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            result = get_matrix_row(&u_vp, u_mp, cluster); 
            if (result == ERROR) { NOTE_ERROR(); break; } 

            for (i = 0; i < num_points; i++)
            {
                if ( ! outlier_vp->elements[ i ])
                {
                    double  p = P_mp->elements[ i ][ cluster ];
                
                    result = get_matrix_row(&x_vp, feature_mp, i);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = subtract_vectors(&d_vp, x_vp, u_vp); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = multiply_vectors(&S_vp, d_vp, d_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = ow_multiply_vector_by_scalar(S_vp, p); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = ow_add_vector_to_matrix_row(S_mp, S_vp, cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; } 
                }
            }

            /*
            // Normalize the cluster variance sums by the total weight.
            */
            if (result != ERROR)
            {
                double  temp = p_sum_vp->elements[ cluster ];
                
                if (temp < 1e10 * DBL_MIN)
                {
                    dbe(temp); 
                }
                else
                {
                    result = ow_divide_matrix_row_by_scalar(S_mp, temp, cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; } 
                }
            }
        }

        if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef RESCALE
        result = average_matrix_rows(&ave_sigma_vp, S_mp);
        if (result == ERROR) { NOTE_ERROR(); break; }

        if (    (max_vector_element(ave_sigma_vp) > it * 10.0)
             || (min_vector_element(ave_sigma_vp) < 0.1 / it)
           )
        {
            verbose_pso(2, "Rescaling data.\n");

            result = ow_sqrt_vector(ave_sigma_vp);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = ow_divide_matrix_by_row_vector(feature_mp, ave_sigma_vp);
            if (result == ERROR) { NOTE_ERROR(); break; }

            rescaled_on_previous_iteration = TRUE; 

            continue; 
        }
#endif 

        if (fs_var_offset > 0.0)
        {
            if (ow_add_scalar_to_matrix(S_mp, fs_var_offset) == ERROR)
            {
                result = ERROR;
                break; 
            }
        }

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            result = get_matrix_row(&S_vp, S_mp, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef USE_LOGS
            {
                int ii;

                const_vp->elements[cluster] = log(a_vp->elements[cluster]);

                for (ii = 0; ii < S_mp->num_cols; ii++)
                {
                    const_vp->elements[cluster] -= 0.5 * log(S_mp->elements[ cluster ][ ii ]);
                }

                /*
                // If we wanted to put the constant in for some reason.
                //
                // const_vp->elements[cluster] -= 0.9189 * num_features; 
                */
            }
#else

            det = multiply_vector_elements(S_vp); 

            if (det < 1e-100) 
            {
                int ii;

                if (min_vector_element(S_vp) < 1e-20)
                {
                    dbe(min_vector_element(S_vp));
                }

                sqrt_det = 1.0; 

                for (ii = 0; ii < S_vp->length; ii++)
                {
                    sqrt_det *= sqrt(S_vp->elements[ ii ]); 
                }

                if (sqrt_det < 1e-150)
                {
                    dbe(sqrt_det);
                    sqrt_det = 1e-150; 
                }
            }
            else 
            {
                sqrt_det = sqrt(det);
            }

            sqrt_det_vp->elements[ cluster ]= sqrt_det;

            if (det < 1e-300) 
            {
                dbe(det);
                det = 1e-300; 
            }
#endif 
        }


        if (result == ERROR) { NOTE_ERROR(); break; }

        /* E Step */

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
#ifdef USE_LOGS
            double factor = const_vp->elements[ cluster ];
#else 
            double factor = a_vp->elements[ cluster ] / sqrt_det_vp->elements[ cluster ];
#endif 

            if (result == ERROR) { NOTE_ERROR(); break; } 

            result = get_matrix_row(&u_vp, u_mp, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_matrix_row(&var_vp, S_mp, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }

            for (i = 0; i < num_points; i++)
            {
                if ( ! outlier_vp->elements[ i ])
                {
                    result = get_matrix_row(&x_vp, feature_mp, i);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = subtract_vectors(&d_vp, x_vp, u_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = divide_vectors(&temp_vp, d_vp, var_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_dot_product(d_vp, temp_vp, &dot_product);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    if (dot_product < 0.0)
                    {
                        dbe(dot_product);
                        dot_product = 0.0; 
                    }

#ifdef USE_LOGS
                    P_mp->elements[ i ][ cluster ] = factor - 0.5 * dot_product;
#else 
                    P_mp->elements[ i ][ cluster ] =  
                                          factor * exp(-0.5 * dot_product);

                    /*
                    // If we wanted to put the constant in for some reason.
                    //
                    P_mp->elements[ cluster ][ feature_source ] /= 
                              pow(2.0 * M_PI, ((double)num_features) / 2.0);
                    */
#endif 
                }
            }

        }

        if (result == ERROR) { NOTE_ERROR(); break; } 

#ifdef USE_LOGS
        for (i = 0; i < num_points; i++)
        {
            /* For points not already outliers ... */
            if ( ! outlier_vp->elements[ i ])
            {
                for (j = 0; j< P_mp->num_cols; j++)
                {
                    P_mp->elements[ i ][ j ] = exp(P_mp->elements[ i ][ j ]);
                }
            }
        }
#endif  

        log_likelihood = 0.0; 

        if (result != ERROR)
        {
            for (i = 0; i < num_points; i++)
            {
                /* For points not already outliers ... */
                if ( ! outlier_vp->elements[ i ])
                {
                    double p_sum = sum_matrix_row_elements(P_mp, i); 

                    if (p_sum > 0.0)
                    {
                        log_likelihood += log(p_sum); 
                    }
                    else 
                    {
                        log_likelihood += LOG_ZERO; 
                    }
                }
            }
        }

        for (i = 0; i<num_points; i++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            if ( ! outlier_vp->elements[ i ])
            {
                double p_sum = sum_matrix_row_elements(P_mp, i);

                if (p_sum < 1e10 * DBL_MIN) /* This is taken to be an outlier. */
                {
                    verbose_pso(2, "Point %d is taken as an outlier.\n", i + 1);

                    outlier_vp->elements[ i ] = TRUE; 
                    num_good_points--; 

                    if (num_good_points == 0)
                    {
                        set_bug("All points are outliers."); 
                        result = ERROR; 
                        break; 
                    }
                }
                else 
                {
                    result = ow_divide_matrix_row_by_scalar(P_mp, p_sum, i);
                }
            }
        }

        if (     ( ! backed_off_during_this_iteration )
#ifdef RESCALE
              && ( ! rescaled_on_previous_iteration ) 
#endif 
              && (IS_LESSER_DBL(log_likelihood, prev_log_likelihood))
           )
        {
            warn_pso("Log likelihood %.8e is less than previous %.8e.\n",
                     log_likelihood, prev_log_likelihood);
        }

#ifdef RESCALE
        /* If we got here, we did not rescale. */ 
        rescaled_on_previous_iteration = FALSE; 
#endif 

        diff = ABS_OF(log_likelihood - prev_log_likelihood);

        diff *= 2.0;
        diff /= (ABS_OF(log_likelihood) +  ABS_OF(prev_log_likelihood));

        verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e\n", 
                    it + 1, log_likelihood, diff); 

        if (diff < fs_iteration_tolerance) break; 

        prev_log_likelihood = log_likelihood; 
    }


    if (result != ERROR)
    {
        verbose_pso(2, "%d points used as outliers.\n", 
                    num_points - num_good_points);

        if (log_likelihood_ptr != NULL)
        {
            *log_likelihood_ptr = log_likelihood;
        }

        if (a_vpp != NULL) 
        {
            result = copy_vector(a_vpp, a_vp);
        }
    }
    
    if ((result != ERROR) && (var_mpp != NULL))
    {
        result = copy_matrix(var_mpp, S_mp);
    }

    if ((result != ERROR) && (means_mpp != NULL))
    {
        result = copy_matrix(means_mpp, u_mp);
    }

    if ((result != ERROR) && (P_mpp != NULL))
    {
        free_matrix(*P_mpp); 
        *P_mpp = P_mp; 

        for (i = 0; i<num_points; i++)
        {
            if (outlier_vp->elements[ i ])
            {
                for (j = 0; j<num_clusters; j++)
                {
                    P_mp->elements[ i ][ j ] = DBL_NOT_SET; 
                }
            }
        }
        
    }
    else
    {
        free_matrix(P_mp); 
    }

    free_matrix(u_mp); 

#ifdef USE_LOGS
    free_vector(const_vp);
#else 
    free_vector(sqrt_det_vp);
#endif 

#ifdef RESCALE
    free_vector(ave_sigma_vp);
#endif 

    free_vector(p_sum_vp);
    free_vector(a_vp);

    free_matrix(S_mp); 
    free_vector(S_vp); 
    free_vector(x_vp); 
    free_vector(d_vp); 
    free_vector(temp_vp); 
    free_vector(var_vp); 
    free_vector(u_vp); 

    free_int_vector(outlier_vp); 
    free_matrix(rescaled_feature_mp);

    if (result == ERROR) return ERROR; else return num_clusters;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

