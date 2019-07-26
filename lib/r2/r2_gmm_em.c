/* $Id: r_gmm_em.c 13018 2012-09-24 19:34:52Z jguan1 $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Authors: 
   |      Prasad Gabbur
   |      Kobus Barnard 
   |      Yekaterina Kharitonova
   |      Jinyan Guan  
 * =========================================================================== */
#include "m/m_incl.h"      /*  Only safe if first #include in a ".c" file  */

#include "n/n_invert.h"  
#include "n/n_svd.h"  

#include "r/r_cluster_lib.h"  
#include "r2/r2_gmm_em.h"

#include "l/l_sys_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Gauss routine slows things down a bit--used for debugging.
 * 
*/
#define DONT_USE_GAUSS_ROUTINE

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */
#define ENABLE_SHIFT_MODELS
#endif  /* End of code block that is removed for export. */


#define EM_MIN_CLUSTER_SIZE  (1e-10)
#define REQUIRED_SUBSET_FIT  (1e-20)

#define USE_LOGS
#define NUM_THREADS 4

static double fs_data_perturbation                           = DBL_NOT_SET;
static int    fs_plot_log_likelihood_vs_num_clusters         = FALSE;
static int    fs_num_tries_per_cluster_count                 = 1;
static int    fs_num_cluster_count_samples                   = 30;
static int    fs_max_num_iterations                          = 20;
static double fs_iteration_tolerance                         = 1.0e-6;
static double fs_dis_item_prob_threshold                     = DBL_NOT_SET;
static int    fs_normalize_data                              = FALSE;    /* Switched Dec 21, 2004. */
static double fs_var_offset                                  = 0.0001;
static int    fs_use_unbiased_var_estimate_in_M_step         = FALSE;
static double fs_held_out_data_fraction                      = 0.1;
static int    fs_tie_var                                     = FALSE;
static int    fs_tie_feature_var                             = FALSE;
static int    fs_tie_cluster_var                             = FALSE;
static int    fs_model_selection_training_MDL                = FALSE;
static int    fs_model_selection_held_out_LL                 = TRUE;
static int    fs_model_selection_held_out_MDL                = FALSE;
static int    fs_model_selection_held_out_corr_diff          = TRUE;
static int    fs_model_selection_held_out_max_membership     = TRUE;
static int    fs_EM_stop_criterion_training_LL               = TRUE;
static int    fs_EM_stop_criterion_held_out_LL               = FALSE;
static int    fs_use_initialized_cluster_means_variances_and_priors = FALSE;
static int    fs_max_num_CEM_iterations                      = 100;
static int    fs_write_CEM_intermediate_results              = TRUE;
static int    fs_force_equal_prob_for_CEM_split_and_merge    = TRUE;
static int    fs_crop_feature_dimensions                     = FALSE;
static int    fs_crop_num_feature_dimensions_left            = 0;
static int    fs_crop_num_feature_dimensions_right           = 0;


double old_bic(int num_clusters, int num_features, int num_observations, int independent, int tie_var, int tie_feature_var);
double bic(int num_clusters, int num_features, int num_observations, int independent, int tie_var, int tie_feature_var);


#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */
static int select_GMM_helper
(
    int             independent_flag,
    int             max_num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix**        var_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp 
);
#endif  /* End of code block that is removed for export. */

/**
 * TODO: Document
 */
int set_em_cluster_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result         = NOT_FOUND;
    int  temp_int_value;
    int  temp_boolean_value;
    double temp_double_value; 
    double   temp_real_value; 
    

    EXTENDED_LC_BUFF_CPY(lc_option, option); 

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-data-perturbation")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Cluster data perturbation is %.3e.\n", 
                    fs_data_perturbation)); 
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-data-perturbation = %.3e\n", 
                    fs_data_perturbation)); 
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_data_perturbation = temp_double_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-discrete-threshold")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Cluster threshold for excluding discrete probabilities %.3e.\n", 
                    fs_dis_item_prob_threshold)); 
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-discrete-threshold = %.3e\n", 
                    fs_dis_item_prob_threshold)); 
        }
        else
        {
            ERE(ss1snd(value, &temp_real_value));
            fs_dis_item_prob_threshold = temp_real_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-num-tries-per-cluster-count")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-num-tries-per-cluster-count = %d\n", 
                    fs_num_tries_per_cluster_count)); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Each number of clusters gets %d EM tries.\n", 
                    fs_num_tries_per_cluster_count)); 
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            if (temp_int_value == 0)
            {
                set_error("Number of tries per cluster count must be at least one."); 
                return ERROR;
            }
            fs_num_tries_per_cluster_count = temp_int_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-num-cluster-count-samples")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-num-cluster-count-samples = %d\n", 
                    fs_num_cluster_count_samples)); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d log spaced EM clusters counts are tried.\n", 
                    fs_num_cluster_count_samples)); 
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            if (temp_int_value == 0)
            {
                set_error("Number of EM cluster counts must be at least one."); 
                return ERROR;
            }
            fs_num_cluster_count_samples = temp_int_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-max-num-iterations")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-max-num-iterations = %d\n", 
                    fs_max_num_iterations)); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("A maximum of %d EM iterations are used.\n", 
                    fs_max_num_iterations)); 
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_max_num_iterations = temp_int_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-iteration-tolerance")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("EM iteration tolerance is %.3e.\n", 
                    fs_iteration_tolerance)); 
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-iteration-tolerance = %.3e\n", fs_iteration_tolerance)); 
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_iteration_tolerance = temp_double_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-plot-log-likelihood")
          || match_pattern(lc_option, "cluster-plot-log-likelihood-vs-num-clusters")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("EM log likelihood vs num clusters %s plotted.\n", 
                    fs_plot_log_likelihood_vs_num_clusters ? "is" : "is not")); 
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-plot-log-likelihood-vs-num-clusters = %s\n", 
                    fs_plot_log_likelihood_vs_num_clusters ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_plot_log_likelihood_vs_num_clusters = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-norm-data")
          || match_pattern(lc_option, "cluster-norm-data-flag")
          || match_pattern(lc_option, "cluster-normalize-data")
          || match_pattern(lc_option, "cluster-normalize-data-flag")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Cluster data %s normalized.\n", 
                    fs_normalize_data ? "is" : "is not")); 
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-normalize-data = %s\n", 
                    fs_normalize_data ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_normalize_data = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-var-offset")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Cluster variance offset is %.3e.\n", fs_var_offset)); 
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-var-offset = %.3e\n", fs_var_offset)); 
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_var_offset = temp_double_value; 
        }
        result = NO_ERROR; 
    }

   if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-use-unbiased-var-estimate-in-M-step")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Unbiased estimate for variance %s used in the M step.\n", 
                    fs_use_unbiased_var_estimate_in_M_step ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-use-unbiased-var-estimate-in-M-step = %s\n", 
                    fs_use_unbiased_var_estimate_in_M_step ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_use_unbiased_var_estimate_in_M_step = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-held-out-data-fraction")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Held out data fraction is %.3e.\n", fs_held_out_data_fraction)); 
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("Held out data fraction = %.3e\n", fs_held_out_data_fraction)); 
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_held_out_data_fraction = temp_double_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-tie-var")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Cluster tie variance %s used.\n", 
                    fs_tie_var ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-tie-var = %s\n", 
                    fs_tie_var ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_tie_var = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-tie-feature-var")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Cluster tie feature variance %s used.\n", 
                    fs_tie_feature_var ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-tie-feature-var = %s\n", 
                    fs_tie_feature_var ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_tie_feature_var = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-tie-cluster-var")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Cluster tie cluster variance %s used.\n", 
                    fs_tie_cluster_var ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-tie-cluster-var = %s\n", 
                    fs_tie_cluster_var ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_tie_cluster_var = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-model-selection-training-MDL")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Training data MDL criterion for model selection %s used.\n", 
                    fs_model_selection_training_MDL ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-model-selection-training-MDL = %s\n", 
                    fs_model_selection_training_MDL ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_model_selection_training_MDL = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }
    
    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-model-selection-held-out-LL")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Held out data log likelihood criterion for model selection %s used.\n", 
                    fs_model_selection_held_out_LL ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-model-selection-held-out-LL = %s\n", 
                    fs_model_selection_held_out_LL ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_model_selection_held_out_LL = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-model-selection-held-out-MDL")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Held out data MDL criterion for model selection %s used.\n", 
                    fs_model_selection_held_out_MDL ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-model-selection-held-out-MDL = %s\n", 
                    fs_model_selection_held_out_MDL ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_model_selection_held_out_MDL = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }
    
    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-model-selection-held-out-corr-diff")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Held out data correlation difference criterion for model selection %s used.\n", 
                    fs_model_selection_held_out_corr_diff ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-model-selection-held-out-corr-diff = %s\n", 
                    fs_model_selection_held_out_corr_diff ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_model_selection_held_out_corr_diff = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-model-selection-held-out-max-membership")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Held out data maximum membership criterion for model selection %s used.\n", 
                    fs_model_selection_held_out_max_membership ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-model-selection-held-out-max-membership = %s\n", 
                    fs_model_selection_held_out_max_membership ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_model_selection_held_out_max_membership = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-EM-stop-criterion-training-LL")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Training data log likelihood %s used as EM stopping criterion.\n", 
                    fs_EM_stop_criterion_training_LL ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-EM-stop-criterion-training-LL = %s\n", 
                    fs_EM_stop_criterion_training_LL ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_EM_stop_criterion_training_LL = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-EM-stop-criterion-held-out-LL")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Held out data log likelihood %s used as EM stopping criterion.\n", 
                    fs_EM_stop_criterion_held_out_LL ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-EM-stop-criterion-held-out-LL = %s\n", 
                    fs_EM_stop_criterion_held_out_LL ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_EM_stop_criterion_held_out_LL = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-use-initialized-cluster-means-variances-and-priors")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Initialized cluster means and variances %s used in the EM algorithm.\n", 
                    fs_use_initialized_cluster_means_variances_and_priors ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-use-initialized-cluster-means-variances-and-priors = %s\n", 
                    fs_use_initialized_cluster_means_variances_and_priors ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_use_initialized_cluster_means_variances_and_priors = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-max-num-CEM-iterations")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-max-num-CEM-iterations = %d\n", 
                    fs_max_num_CEM_iterations)); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("A maximum of %d CEM iterations are used.\n", 
                    fs_max_num_CEM_iterations)); 
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_max_num_CEM_iterations = temp_int_value; 
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-write-CEM-intermediate-results")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("CEM intermediate results %s written.\n", 
                    fs_write_CEM_intermediate_results ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-write-CEM-intermediate-results = %s\n", 
                    fs_write_CEM_intermediate_results ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_write_CEM_intermediate_results = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-force-equal-prob-for-CEM-split-and-merge")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The probabilities of choosing a CEM split or a merge operation %s forced to be equal.\n", 
                    fs_force_equal_prob_for_CEM_split_and_merge ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-force-equal-prob-for-CEM-split-and-merge = %s\n", 
                    fs_force_equal_prob_for_CEM_split_and_merge ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_force_equal_prob_for_CEM_split_and_merge = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-crop-feature-dimensions")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Some feature dimensions %s cropped before clustering.\n", 
                    fs_crop_feature_dimensions ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-crop-feature-dimensions = %s\n", 
                    fs_crop_feature_dimensions ? "t" : "f")); 
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_crop_feature_dimensions = temp_boolean_value; 
        }
        result = NO_ERROR; 
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-crop-num-feature-dimensions-left")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-crop-num-feature-dimensions-left = %d\n", 
                    fs_crop_num_feature_dimensions_left)); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d feature dimensions to the left are cropped.\n", 
                    fs_crop_num_feature_dimensions_left)); 
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_crop_num_feature_dimensions_left = temp_int_value; 
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0') 
          || match_pattern(lc_option, "cluster-crop-num-feature-dimensions-right")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("cluster-crop-num-feature-dimensions-right = %d\n", 
                    fs_crop_num_feature_dimensions_right)); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d feature dimensions to the right are cropped.\n", 
                    fs_crop_num_feature_dimensions_right)); 
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_crop_num_feature_dimensions_right = temp_int_value; 
        }
        result = NO_ERROR;
    }

    return result; 
}    

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */
int select_independent_GMM
(
    int           max_num_clusters,
    const Matrix* feature_mp,
    Vector**      a_vpp, 
    Matrix**      u_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp
)
{
    const int independent_flag = TRUE; 

    return select_GMM_helper(independent_flag, 
                             max_num_clusters, feature_mp,
                             (Matrix*)NULL, 
                             a_vpp, u_mpp, var_mpp, 
                             (Matrix_vector**)NULL,
                             P_mpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int select_full_GMM
(
    int             max_num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp 
)
{
    const int independent_flag = FALSE; 

    return select_GMM_helper(independent_flag,
                             max_num_clusters, feature_mp, 
                             covariance_mask_mp, 
                             a_vpp, u_mpp, (Matrix**)NULL, S_mvpp,
                             P_mpp);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int select_GMM_helper
(
    int             independent_flag,
    int             max_num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix**        var_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp 
)
{
    int        num_points   = feature_mp->num_rows;
    int        num_features = feature_mp->num_cols;
    int        i, j;
    int        num_clusters;
    double     d_num_clusters;
    double     d_min_num_clusters;
    double     d_max_num_clusters;
    Vector*    log_likelihood_sum_vp = NULL;
    Vector*    min_log_likelihood_vp = NULL;
    Vector*    max_log_likelihood_vp = NULL;
    double     f;
    double     max_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    Vector*    a_vp = NULL; 
    Matrix*    u_mp = NULL;
    Matrix*    var_mp = NULL;
    Matrix_vector* S_mvp = NULL; 
    Matrix*    local_P_mp = NULL; 
    Matrix**   local_P_mpp = (P_mpp == NULL) ? NULL : &local_P_mp; 
    Matrix*    perturbed_feature_mp = NULL;
    Vector*    counts_vp     = NULL;
    double     encoding_cost;
    int        result        = NO_ERROR;
    int        slightly_below_count = 0; 
    int        downward_trend = FALSE; 
    Matrix*    norm_feature_mp = NULL; 
    Vector*    feature_mean_vp = NULL;
    Vector*    feature_var_vp = NULL;
    const double norm_stdev = 1.0; 


    UNTESTED_CODE();  

    /*
     * Need to ensure that we can specify the number of clusters so that we just
     * select from initializations. Ideally, some sort of averaging would occur.
    */

    /*
     *        Controlled via user options:
     *            min_num_clusters
     *            max_num_clusters
     *
     *            Model selection based on held out percentage instead of adjusted
     *            log likelihood. In this case, the model is chosen based on the
     *            best result for held out data. 
     *
     *            A second option could cut the process when the held out log
     *            likelihood dropped. 
     * 
     *            A number of tries to average.
     *
     *            The number of iterations controlled by held out percentage
     *
    */

    d_min_num_clusters = 2.0;
    d_max_num_clusters = max_num_clusters; 

    f = exp(log(d_max_num_clusters / d_min_num_clusters) / 
                                      (double)(fs_num_cluster_count_samples - 1));

    d_num_clusters = d_min_num_clusters;

    /* Make room for bucket sort. */
    max_num_clusters += 1; 

    if ((result != ERROR) && (fs_data_perturbation > 10.0 * DBL_EPSILON))
    {
        result = perturb_unary_cluster_data(&perturbed_feature_mp, 
                                            feature_mp, fs_data_perturbation,
                                            (Int_vector*)NULL); 
        feature_mp = perturbed_feature_mp; 
    }

    /*
     * Normalization does not make immediate sense in the case of the full
     * model.
    */
    if ((independent_flag) && (fs_normalize_data))
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
            db_rv(feature_var_vp); 
        }

        if (result != ERROR)
        {
            feature_mp = norm_feature_mp; 
        }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&log_likelihood_sum_vp, max_num_clusters); 
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&counts_vp, max_num_clusters); 
    }
    if (result != ERROR)
    {
        result = get_initialized_vector(&min_log_likelihood_vp, 
                                        max_num_clusters, 
                                        DBL_HALF_MOST_POSITIVE); 
    }
    if (result != ERROR)
    {
        result = get_initialized_vector(&max_log_likelihood_vp,
                                        max_num_clusters, 
                                        DBL_HALF_MOST_NEGATIVE); 
    }

    for (i = 0; i < fs_num_cluster_count_samples; i++)
    {
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (j = 0; j < fs_num_tries_per_cluster_count; j++)
        {
            double log_likelihood;

            if (result == ERROR) { NOTE_ERROR(); break; }

            num_clusters = kjb_rint(d_num_clusters);

            if (num_clusters > max_num_clusters)
            {
                num_clusters = max_num_clusters;
            }

            verbose_pso(2, "Trying %d clusters for %d points.\n", num_clusters,
                        num_points); 

            if (independent_flag)
            {
                if( respect_missing_values() )
                {
                    num_clusters = get_independent_GMM_2_with_missing_data(num_clusters, 
                                                         feature_mp, 
                                                         &a_vp, &u_mp, &var_mp,  
                                                         local_P_mpp, 
                                                         &log_likelihood); 
                }
                else
                {
                    num_clusters = get_independent_GMM_2(num_clusters, 
                                                         feature_mp, 
                                                         &a_vp, &u_mp, &var_mp,  
                                                         local_P_mpp, 
                                                         &log_likelihood); 
                }
            }
            else
            {
                num_clusters = get_full_GMM_2(num_clusters,
                                              feature_mp, 
                                              covariance_mask_mp, 
                                              &a_vp, &u_mp, &S_mvp, 
                                              local_P_mpp, 
                                              &log_likelihood); 
            }

            if (num_clusters == ERROR)
            {
                result = ERROR;
                break; 
            }

            verbose_pso(2, "Raw log likelihood with %d clusters is %.5e.\n", 
                        num_clusters, log_likelihood);

            encoding_cost = bic(num_clusters, num_features, num_points, 
                                independent_flag, fs_tie_var, fs_tie_feature_var);

            log_likelihood -= encoding_cost; 

            verbose_pso(2, "Adjusted log likelihood with %d clusters is %.5e.\n",
                        num_clusters, log_likelihood);

            if (log_likelihood > max_log_likelihood)
            {
                max_log_likelihood = log_likelihood; 

                if (    (result != ERROR) 
                     && ((u_mpp != NULL) || (var_mpp != NULL))
                   )
                {
                    if ((result != ERROR) && (a_vpp != NULL))
                    {
                        result = copy_vector(a_vpp, a_vp); 
                    }

                    if ((result != ERROR) && (u_mpp != NULL))
                    {
                        result = copy_matrix(u_mpp, u_mp); 
                    }

                    if ((result != ERROR) && (var_mpp != NULL))
                    {
                        result = copy_matrix(var_mpp, var_mp); 
                    }

                    if ((result != ERROR) && (S_mvpp != NULL))
                    {
                        result = copy_matrix_vector(S_mvpp, S_mvp); 
                    }

                    if ((result != ERROR) && (P_mpp != NULL))
                    {
                        result = copy_matrix(P_mpp, local_P_mp); 
                    }

                }

                slightly_below_count = 0;
            }
            else if (   (    (log_likelihood >= 0.0)
                          && (log_likelihood < 0.95 * max_log_likelihood)
                        )
                      ||
                        (    (log_likelihood < 0.0)
                          && (0.95 * log_likelihood < max_log_likelihood)
                        )
                    )
            {
                slightly_below_count++;
            }

            if (    (slightly_below_count > fs_num_tries_per_cluster_count + 1)
                 && (    (    (log_likelihood >= 0.0)
                           && (log_likelihood < 0.80 * max_log_likelihood)
                         )
                      || 
                         (    (log_likelihood < 0.0)
                           && (0.80 * log_likelihood < max_log_likelihood) 
                         )
                     )
                 )
            {
                verbose_pso(2, "Downward trend spotted. ");
                verbose_pso(2, "Aborting search for number of clusters.\n"); 
                downward_trend = TRUE; 
                break; 
            }

            log_likelihood_sum_vp->elements[ num_clusters ] += log_likelihood;

            if (log_likelihood < min_log_likelihood_vp->elements[ num_clusters])
            {
                min_log_likelihood_vp->elements[ num_clusters] = log_likelihood;
            }

            if (log_likelihood > max_log_likelihood_vp->elements[ num_clusters])
            {
                max_log_likelihood_vp->elements[ num_clusters] = log_likelihood;
            }

            counts_vp->elements[ num_clusters ] += 1.0; 
        }

        if (downward_trend) break; 

        d_num_clusters *= f; 
    }

    if ((result != ERROR) && (fs_plot_log_likelihood_vs_num_clusters))
    {
        result = plot_log_likelihood(log_likelihood_sum_vp, counts_vp,
                                     min_log_likelihood_vp, 
                                     max_log_likelihood_vp,
                                     (const char*)NULL,
                                     (const char*)NULL); 
    }

    if ((result != ERROR) && (fs_normalize_data) && (independent_flag))
    {
        result = ow_un_normalize_cluster_data((u_mpp == NULL) ? NULL : *u_mpp, 
                                              (var_mpp == NULL) ? NULL : *var_mpp, 
                                              feature_mean_vp, 
                                              feature_var_vp, 
                                              norm_stdev); 
    }

    free_vector(a_vp);
    free_matrix(u_mp);
    free_matrix(var_mp);
    free_matrix_vector(S_mvp); 
    free_matrix(local_P_mp);

    free_matrix(norm_feature_mp);
    free_vector(feature_var_vp); 
    free_vector(feature_mean_vp); 
    free_vector(log_likelihood_sum_vp); 
    free_vector(min_log_likelihood_vp); 
    free_vector(max_log_likelihood_vp); 
    free_vector(counts_vp); 

    free_matrix(perturbed_feature_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


int select_GMM_helper_2
(
    int             independent_flag,
    int             max_num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix**        var_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp,
    const Vector*   initial_a_vp,
    const Matrix*   initial_u_mp,
    const Matrix*   initial_var_mp
)
{
   
    int        num_points   = feature_mp->num_rows;
    int        num_features = feature_mp->num_cols;
    int        i, j, k, l;
    int        num_clusters;
    double     d_num_clusters;
    double     d_min_num_clusters;
    double     d_max_num_clusters;
    double     log_likelihood;
    double     held_out_log_likelihood;
    Vector*    log_likelihood_sum_vp = NULL;
    Vector*    min_log_likelihood_vp = NULL;
    Vector*    max_log_likelihood_vp = NULL;
    double     f;
    double     max_log_likelihood          = DBL_HALF_MOST_NEGATIVE;
    double     max_held_out_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    Vector*    a_vp = NULL; 
    Matrix*    u_mp = NULL;
    Matrix*    var_mp = NULL;
    Matrix_vector* S_mvp = NULL; 
    Matrix*    local_P_mp = NULL; 
    Matrix**   local_P_mpp = (P_mpp == NULL) ? NULL : &local_P_mp; 
    Matrix*    perturbed_feature_mp = NULL;
    Vector*    counts_vp     = NULL;
    double     encoding_cost;
    int        result        = NO_ERROR;
    int        slightly_below_count = 0; 
    int        downward_trend = FALSE; 
    Matrix*    norm_feature_mp = NULL; 
    
    Vector*    feature_mean_vp = NULL;
    Vector*    feature_var_vp = NULL;
    const double norm_stdev = 1.0;
    
    Int_matrix*    held_out_indicator_mp = NULL;
    Int_vector*    held_out_indicator_vp = NULL;

    Int_vector* num_clusters_vp                = NULL;
    Matrix*     held_out_log_likelihood_mp     = NULL;
    Int_matrix* num_iterations_mp              = NULL; 
    Vector*     held_out_log_likelihood_sum_vp = NULL;
    
    int        num_iterations;
    int        max_num_held_out_points;

    Vector*    aux_a_vp   = NULL;
    Matrix*    aux_u_mp   = NULL;
    Matrix*    aux_var_mp = NULL;
    Matrix*    aux_P_mp   = NULL;

    Matrix*    held_out_corr_diff_mp     = NULL;
    Vector*    held_out_corr_diff_sum_vp = NULL;
    Vector*    held_out_P_vp             = NULL;
    Vector*    training_P_vp             = NULL;
    Vector*    held_out_aux_P_vp         = NULL;
    Vector*    training_aux_P_vp         = NULL;

    double     held_out_train_corr;
    double     aux_held_out_train_corr;

    Matrix*    held_out_max_membership_mp     = NULL;
    Vector*    held_out_max_membership_sum_vp = NULL;

    double     held_out_max_membership = 0.0;
    int num_held_out_points = 0;

    UNTESTED_CODE();  

    /*
     * Kobus: I changed some stuff quickly while Prasad was away. THe
     * UNTESTED_CODE() was already there, but any testing that went before needs
     * to be redone. 
     *
     * Main changes: 
     *     num_clusters_vp is now an Int_vector.
     *     declarations are all now at the begining of a block. 
     *
    */

    /*
     * Need to ensure that we can specify the number of clusters so that we just
     * select from initializations. Ideally, some sort of averaging would occur.
    */

    /*
     *        Controlled via user options:
     *            min_num_clusters
     *            max_num_clusters
     *
     *            Model selection based on held out percentage instead of adjusted
     *            log likelihood. In this case, the model is chosen based on the
     *            best result for held out data. 
     *
     *            A second option could cut the process when the held out log
     *            likelihood dropped. 
     * 
     *            A number of tries to average.
     *
     *            The number of iterations controlled by held out percentage
     *
    */

    d_min_num_clusters = 2.0;
    d_max_num_clusters = max_num_clusters; 

    f = exp(log(d_max_num_clusters / d_min_num_clusters) / 
                                      (double)(fs_num_cluster_count_samples - 1));

    d_num_clusters = d_min_num_clusters;

    /* Make room for bucket sort. */
    max_num_clusters += 1; 

    if ((result != ERROR) && (fs_data_perturbation > 10.0 * DBL_EPSILON))
    {
        result = perturb_unary_cluster_data(&perturbed_feature_mp, 
                                            feature_mp, fs_data_perturbation,
                                            (Int_vector*)NULL); 
        feature_mp = perturbed_feature_mp; 
    }

    /*
     * Normalization does not make immediate sense in the case of the full
     * model.
    */
    if ((independent_flag) && (fs_normalize_data))
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
            db_rv(feature_var_vp); 
        }

        if (result != ERROR)
        {
            feature_mp = norm_feature_mp; 
        }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&log_likelihood_sum_vp, max_num_clusters); 
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&counts_vp, max_num_clusters); 
    }
    if (result != ERROR)
    {
        result = get_initialized_vector(&min_log_likelihood_vp, 
                max_num_clusters, 
                DBL_HALF_MOST_POSITIVE); 
    }
    if (result != ERROR)
    {
        result = get_initialized_vector(&max_log_likelihood_vp,
                max_num_clusters, 
                DBL_HALF_MOST_NEGATIVE); 
    }
    if (result != ERROR)
    {
        result = get_zero_int_matrix(&held_out_indicator_mp, fs_num_tries_per_cluster_count, num_points);
    }
    if (result != ERROR)
    {
        result = get_zero_int_vector(&held_out_indicator_vp, num_points);
    }
    if (result != ERROR)
    {
        result = get_zero_int_vector(&num_clusters_vp, fs_num_cluster_count_samples);
    }
    if (result != ERROR)
    {
        result = get_target_matrix(&held_out_log_likelihood_mp, fs_num_cluster_count_samples, fs_num_tries_per_cluster_count);
    }
    if (result != ERROR)
    {
        result = get_target_int_matrix(&num_iterations_mp, fs_num_cluster_count_samples, fs_num_tries_per_cluster_count);
    }
    if (result != ERROR)
    {
        result = get_target_vector(&held_out_log_likelihood_sum_vp, fs_num_cluster_count_samples);
    }
    if (result != ERROR)
    {
        result = get_zero_matrix(&held_out_corr_diff_mp, fs_num_cluster_count_samples, fs_num_tries_per_cluster_count);
    }
    if (result != ERROR)
    {
        result = get_target_vector(&held_out_corr_diff_sum_vp, fs_num_cluster_count_samples);
    }
    if (result != ERROR)
    {
        result = get_zero_matrix(&held_out_max_membership_mp, fs_num_cluster_count_samples, fs_num_tries_per_cluster_count);
    }
    if (result != ERROR)
    {
        result = get_target_vector(&held_out_max_membership_sum_vp, fs_num_cluster_count_samples);
    }
    
    /* Sample a user specified fraction of the data randomly. Use
     * this data as held out data for choosing a model based on
     * maximizing the likelihood on this held out data. */

    max_num_held_out_points = (int) (fs_held_out_data_fraction * num_points);

    for (i = 0; i < fs_num_tries_per_cluster_count; i++)
    {
        if (result == ERROR) { NOTE_ERROR(); break; }

        num_held_out_points = 0;

        /* Randomly select which points will be used as held out data */
        while (1)
        {
            int random_index = ((int) (num_points * kjb_rand())) - 1;

            if (random_index < 0)
            {
                /* There is a possibility that this might happen */
                random_index = 0;
            }

            ASSERT(random_index < num_points);

            if (held_out_indicator_vp->elements[random_index] == 0)
            {
                held_out_indicator_vp->elements[random_index] = 1;
                num_held_out_points = num_held_out_points + 1;
            }

            if (num_held_out_points >= max_num_held_out_points)
            {
                break;
            }
        }

        result = put_int_matrix_row(held_out_indicator_mp, held_out_indicator_vp, i);

        if (result != ERROR)
        {
            result = get_zero_int_vector(&held_out_indicator_vp, num_points);
        }

        if (result == ERROR)
        {
            break;
        }
    }
    
    for (i = 0; i < fs_num_cluster_count_samples; i++)
    {
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (j = 0; j < fs_num_tries_per_cluster_count; j++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            num_clusters = kjb_rint(d_num_clusters);

            if (num_clusters > max_num_clusters - 1)
            {
                num_clusters--; 
            }

            verbose_pso(2, "Trying %d clusters for %d training points.\n", num_clusters,
                        (num_points - max_num_held_out_points)); 

            if (independent_flag)
            {
                result = get_int_matrix_row(&held_out_indicator_vp, held_out_indicator_mp, j);

                if (result == ERROR)
                {
                    break;
                }

                num_clusters = get_independent_GMM_3(num_clusters, 
                                                     feature_mp,
                                                     held_out_indicator_vp,
                                                     initial_a_vp, 
                                                     initial_u_mp, 
                                                     initial_var_mp,
                                                     &a_vp, &u_mp, &var_mp,  
                                                     local_P_mpp, 
                                                     &log_likelihood,
                                                     &held_out_log_likelihood,
                                                     &num_iterations);

                if (num_clusters != ERROR)
                {
                    if ( (fs_model_selection_held_out_LL == TRUE) || (fs_model_selection_held_out_MDL == TRUE) )
                    {
                        num_clusters_vp->elements[i] = num_clusters;
                        
                        if (fs_model_selection_held_out_MDL == TRUE)
                        {
                            /*
                             * I refactored this to call old_bic, which should give identical results.
                             * I'm not sure how this is MDL.  Maybe BIC was meant?
                             * ---Kyle, April 10, 2014
                             * */
                            encoding_cost = old_bic(num_clusters, num_features, max_num_held_out_points, 
                                                independent_flag, fs_tie_var, fs_tie_feature_var);

                            held_out_log_likelihood -= encoding_cost;
                        }

                        held_out_log_likelihood_mp->elements[i][j] = held_out_log_likelihood;
                        
                        num_iterations_mp->elements[i][j] = num_iterations;
                    }
                
                
                    if (fs_model_selection_held_out_corr_diff == TRUE)
                    {
                        num_clusters_vp->elements[i] = num_clusters;

                        num_iterations_mp->elements[i][j] = num_iterations;

                        /* Calling get_independent_GMM_3 instead of _2, to get the number of iterations,
                         * which is used to compute good_iterations to test on the held-out data */
                        num_clusters = get_independent_GMM_3(num_clusters, 
                                                             feature_mp,
                                                             (Int_vector*) NULL,
                                                             initial_a_vp, 
                                                             initial_u_mp, 
                                                             initial_var_mp,
                                                             &aux_a_vp, &aux_u_mp, &aux_var_mp,  
                                                             &aux_P_mp, 
                                                             &log_likelihood,
                                                             &held_out_log_likelihood,
                                                             &num_iterations);

                        if (num_clusters != ERROR)
                        {
                            for (k = 0; k < num_points; k++)
                            {
                                if (held_out_indicator_vp->elements[k] == 1)
                                {
                                    ERE(get_matrix_row(&held_out_P_vp, *local_P_mpp, k));
                                    ERE(get_matrix_row(&held_out_aux_P_vp, aux_P_mp, k));

                                    ERE(ow_normalize_vector(held_out_P_vp, NORMALIZE_BY_MAGNITUDE));
                                    ERE(ow_normalize_vector(held_out_aux_P_vp, NORMALIZE_BY_MAGNITUDE));

                                    for (l = 0; l < num_points; l++)
                                    {
                                        if (held_out_indicator_vp->elements[l] != 1)
                                        {
                                            ERE(get_matrix_row(&training_P_vp, *local_P_mpp, l));
                                            ERE(get_matrix_row(&training_aux_P_vp, aux_P_mp, l));
                                        }
                                        ERE(ow_normalize_vector(training_P_vp, NORMALIZE_BY_MAGNITUDE));
                                        ERE(ow_normalize_vector(training_aux_P_vp, NORMALIZE_BY_MAGNITUDE));

                                        ERE(get_dot_product(held_out_P_vp, training_P_vp, &held_out_train_corr));
                                        ERE(get_dot_product(held_out_aux_P_vp, training_aux_P_vp, &aux_held_out_train_corr));

                                        held_out_corr_diff_mp->elements[i][j] += fabs(held_out_train_corr - aux_held_out_train_corr);
                                    }
                                }
                            }
                        }
                    }

                    if (fs_model_selection_held_out_max_membership == TRUE)
                    {
                       for (k = 0; k < num_points; k++)
                       {
                           if (held_out_indicator_vp->elements[k] == 1)
                           {
                               held_out_max_membership = 0.0;

                               for (l = 0; l < num_clusters; l++)
                               {
                                   if ( (*local_P_mpp)->elements[k][l] > held_out_max_membership )
                                   {
                                       held_out_max_membership = (*local_P_mpp)->elements[k][l];
                                   }
                               }

                               held_out_max_membership_mp->elements[i][j] += held_out_max_membership;
                           }
                       }
                    }
                }
            }
            else /* if it is not independent */
            {
                num_clusters = get_full_GMM_2(num_clusters,
                                              feature_mp, 
                                              covariance_mask_mp, 
                                              &a_vp, &u_mp, &S_mvp, 
                                              local_P_mpp, 
                                              &log_likelihood); 
            }

            if (num_clusters == ERROR)
            {
                result = ERROR;
                break; 
            }

            verbose_pso(2, "Raw training log likelihood with %d clusters is %.5e.\n", 
                        num_clusters, log_likelihood);

            encoding_cost = bic(num_clusters, num_features, num_points-max_num_held_out_points, 
                                independent_flag, fs_tie_var, fs_tie_feature_var);

            log_likelihood -= encoding_cost; 

            /* A hack to deal with MDL based model selection in case of
             * full GMM. Remove this later by accomodating held out data 
             * likelihood based model selection in case of full covariance
             * matrix GMM. */
            if ( !independent_flag )
            {
                held_out_log_likelihood = log_likelihood;
            }
            
            verbose_pso(2, "Adjusted training log likelihood with %d clusters is %.5e.\n",
                        num_clusters, log_likelihood);

            if (fs_model_selection_held_out_LL == TRUE)
            {
                verbose_pso(2, "Held out log likelihood with %d clusters is %.5e.\n",
                            num_clusters, held_out_log_likelihood);
            }
            else if (fs_model_selection_held_out_MDL == TRUE)
            {
                verbose_pso(2, "Adjusted held out log likelihood with %d clusters is %.5e.\n",
                            num_clusters, held_out_log_likelihood);
            }
            
            if (fs_model_selection_training_MDL == TRUE)
            {
                if (log_likelihood > max_log_likelihood)
                {
                    max_log_likelihood = log_likelihood; 

                    if (    (result != ERROR) 
                            && ((u_mpp != NULL) || (var_mpp != NULL))
                       )
                    {
                        if ((result != ERROR) && (a_vpp != NULL))
                        {
                            result = copy_vector(a_vpp, a_vp); 
                        }

                        if ((result != ERROR) && (u_mpp != NULL))
                        {
                            result = copy_matrix(u_mpp, u_mp); 
                        }

                        if ((result != ERROR) && (var_mpp != NULL))
                        {
                            result = copy_matrix(var_mpp, var_mp); 
                        }

                        if ((result != ERROR) && (S_mvpp != NULL))
                        {
                            result = copy_matrix_vector(S_mvpp, S_mvp); 
                        }

                        if ((result != ERROR) && (P_mpp != NULL))
                        {
                            result = copy_matrix(P_mpp, local_P_mp); 
                        }

                    }

                    slightly_below_count = 0;
                }
                else if (   (    (log_likelihood >= 0.0)
                                 && (log_likelihood < 0.95 * max_log_likelihood)
                            )
                            ||
                            (    (log_likelihood < 0.0)
                                 && (0.95 * log_likelihood < max_log_likelihood)
                            )
                        )
                {
                    slightly_below_count++;
                }

                if (    (slightly_below_count > fs_num_tries_per_cluster_count + 1)
                     && (    (    (log_likelihood >= 0.0)
                                   && (log_likelihood < 0.80 * max_log_likelihood)
                             )
                          || 
                             (    (log_likelihood < 0.0)
                               && (0.80 * log_likelihood < max_log_likelihood) 
                             )
                        )
                   )
                {
                    verbose_pso(2, "Downward trend spotted. ");
                    verbose_pso(2, "Aborting search for number of clusters.\n"); 
                    downward_trend = TRUE; 
                    break; 
                }

                log_likelihood_sum_vp->elements[ num_clusters ] += log_likelihood;

                if (log_likelihood < min_log_likelihood_vp->elements[ num_clusters])
                {
                    min_log_likelihood_vp->elements[ num_clusters] = log_likelihood;
                }

                if (log_likelihood > max_log_likelihood_vp->elements[ num_clusters])
                {
                    max_log_likelihood_vp->elements[ num_clusters] = log_likelihood;
                }
            }
            
            counts_vp->elements[ num_clusters ] += 1.0; 
        }

        if (fs_model_selection_training_MDL == TRUE)
        {
            if (downward_trend) break; 
        }
        
        d_num_clusters *= f; 
    }
    
    /* Choose the number of clusters based on maximum held out (raw or adjusted) log likelihood
     * sum over all the different samples of held-out data. */    
    if (result != ERROR)
    {
        if ( (fs_model_selection_held_out_LL == TRUE) || (fs_model_selection_held_out_MDL == TRUE) )
        {
            result = sum_matrix_cols(&held_out_log_likelihood_sum_vp, held_out_log_likelihood_mp);
        }
    }

    if (result != ERROR)
    {
        if ( (fs_model_selection_held_out_LL == TRUE) || (fs_model_selection_held_out_MDL == TRUE) )
        {
            double max_held_out_log_likelihood_sum       = DBL_HALF_MOST_NEGATIVE;
            int    max_held_out_log_likelihood_sum_index = 0;
            int    good_num_clusters   = num_clusters_vp->elements[max_held_out_log_likelihood_sum_index];
            int    good_num_iterations;

            for (i = 0; i < fs_num_cluster_count_samples; i++)
            {
                pso("Held out log likelihood sum with %d clusters is %.5e.\n",
                    num_clusters_vp->elements[i], held_out_log_likelihood_sum_vp->elements[i]);

                if (held_out_log_likelihood_sum_vp->elements[i] > max_held_out_log_likelihood_sum)
                {
                    max_held_out_log_likelihood_sum       = held_out_log_likelihood_sum_vp->elements[i];
                    max_held_out_log_likelihood_sum_index = i;
                }
            }

            max_held_out_log_likelihood = DBL_HALF_MOST_NEGATIVE;

            for (j = 0; j < fs_num_tries_per_cluster_count; j++)
            {
                if (held_out_log_likelihood_mp->elements[max_held_out_log_likelihood_sum_index][j] > max_held_out_log_likelihood)
                {
                    max_held_out_log_likelihood = held_out_log_likelihood_mp->elements[max_held_out_log_likelihood_sum_index][j];
                    good_num_iterations = num_iterations_mp->elements[max_held_out_log_likelihood_sum_index][j];
                }
            }

            /* Retrain using the entire data set as training set, number of clusters
             * set to good_num_clusters and maximum number of iterations set to good_num_iterations. */
            fs_max_num_iterations = good_num_iterations;

            good_num_clusters = get_independent_GMM_3(good_num_clusters, 
                                                      feature_mp,
                                                      (Int_vector*) NULL,
                                                      initial_a_vp,
                                                      initial_u_mp,
                                                      initial_var_mp,
                                                      a_vpp, u_mpp, var_mpp,  
                                                      P_mpp, 
                                                      &log_likelihood,
                                                      &held_out_log_likelihood,
                                                      &good_num_iterations);

            result = good_num_clusters;
        }

        if (fs_model_selection_held_out_corr_diff == TRUE)
        {
            double min_held_out_corr_diff_sum       = DBL_HALF_MOST_POSITIVE;
            int    min_held_out_corr_diff_sum_index = 0;
            int    good_num_clusters      = num_clusters_vp->elements[min_held_out_corr_diff_sum_index];
            int    good_num_iterations;
            double min_held_out_corr_diff = DBL_HALF_MOST_POSITIVE;

            ERE(sum_matrix_cols(&held_out_corr_diff_sum_vp, held_out_corr_diff_mp));

            for (i = 0; i < fs_num_cluster_count_samples; i++)
            {
                pso("Held out data correlation difference sum with %d clusters is %f.\n",
                    num_clusters_vp->elements[i], held_out_corr_diff_sum_vp->elements[i]);

                if (held_out_corr_diff_sum_vp->elements[i] < min_held_out_corr_diff_sum)
                {
                    min_held_out_corr_diff_sum = held_out_corr_diff_sum_vp->elements[i];
                    min_held_out_corr_diff_sum_index = i;
                }
            }


            for (j = 0; j < fs_num_tries_per_cluster_count; j++)
            {
                if (held_out_corr_diff_mp->elements[min_held_out_corr_diff_sum_index][j] < min_held_out_corr_diff)
                {
                    min_held_out_corr_diff = held_out_corr_diff_mp->elements[min_held_out_corr_diff_sum_index][j];
                    good_num_iterations = num_iterations_mp->elements[min_held_out_corr_diff_sum_index][j];
                }
            }

            /* Retrain using the entire data set as training set, number of clusters
             * set to good_num_clusters and maximum number of iterations set to good_num_iterations. */
            fs_max_num_iterations = good_num_iterations;

            good_num_clusters = get_independent_GMM_3(good_num_clusters, 
                                                      feature_mp,
                                                      (Int_vector*) NULL,
                                                      initial_a_vp,
                                                      initial_u_mp,
                                                      initial_var_mp,
                                                      a_vpp, u_mpp, var_mpp,  
                                                      P_mpp, 
                                                      &log_likelihood,
                                                      &held_out_log_likelihood,
                                                      &good_num_iterations);

            result = good_num_clusters;

        }

        if (fs_model_selection_held_out_max_membership == TRUE)
        {
           double max_held_out_max_membership_sum = 0.0;
           int max_held_out_max_membership_sum_index = 0;
 
           ERE(sum_matrix_cols(&held_out_max_membership_sum_vp, held_out_max_membership_mp));

           for (i = 0; i < fs_num_cluster_count_samples; i++)
           {
               pso("Held out max membership sum with %d clusters is %f.\n",
                   num_clusters_vp->elements[i], held_out_max_membership_sum_vp->elements[i]);

                if (held_out_max_membership_sum_vp->elements[i] > max_held_out_max_membership_sum)
                {
                    max_held_out_max_membership_sum       = held_out_max_membership_sum_vp->elements[i];
                    max_held_out_max_membership_sum_index = i;
                }
            }
            
            /* Retrain using the entire data set as training set, number of clusters
             * set to good_num_clusters and maximum number of iterations set to good_num_iterations. */
           /*           TBD         */
           
        }
    }

    if ((result != ERROR) && (fs_plot_log_likelihood_vs_num_clusters))
    {
        result = plot_log_likelihood(log_likelihood_sum_vp, counts_vp,
                                     min_log_likelihood_vp, 
                                     max_log_likelihood_vp,
                                     (const char*)NULL,
                                     (const char*)NULL); 
    }

    if ((result != ERROR) && (fs_normalize_data) && (independent_flag))
    {
        result = ow_un_normalize_cluster_data((u_mpp == NULL) ? NULL : *u_mpp, 
                                              (var_mpp == NULL) ? NULL : *var_mpp, 
                                              feature_mean_vp, 
                                              feature_var_vp, 
                                              norm_stdev); 
    }

    free_vector(a_vp);
    free_matrix(u_mp);
    free_matrix(var_mp);
    free_matrix_vector(S_mvp);
    free_matrix(local_P_mp);

    free_matrix(norm_feature_mp);
    free_vector(feature_var_vp); 
    free_vector(feature_mean_vp); 
    free_vector(log_likelihood_sum_vp); 
    free_vector(min_log_likelihood_vp); 
    free_vector(max_log_likelihood_vp); 
    free_vector(counts_vp); 

    free_matrix(perturbed_feature_mp);

    free_int_matrix(held_out_indicator_mp);
    free_int_vector(held_out_indicator_vp);
    free_int_vector(num_clusters_vp);
    free_matrix(held_out_log_likelihood_mp);
    free_int_matrix(num_iterations_mp);
    free_vector(held_out_log_likelihood_sum_vp);

    free_vector(aux_a_vp);
    free_matrix(aux_u_mp);
    free_matrix(aux_var_mp);
    free_matrix(aux_P_mp);
    free_matrix(held_out_corr_diff_mp);
    free_vector(held_out_corr_diff_sum_vp);
    free_vector(held_out_P_vp);
    free_vector(training_P_vp);
    free_vector(held_out_aux_P_vp);
    free_vector(training_aux_P_vp);

    free_matrix(held_out_max_membership_mp);
    free_vector(held_out_max_membership_sum_vp);
    
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* =============================================================================
 *                 get_full_GMM
 *                               
 * Finds a Gaussian mixture model (GMM) for the correlated features.
 *
 * This routine finds a Gaussian mixture model (GMM) for the data under the
 * assumption that the features are not independent. The model is fit with EM. Some
 * features are controlled via the set facility. 
 *
 * The argument num_clusters is the number of requested mixture compoenent
 * (clusters), K. 
 *
 * The data matrix feature_mp is an N by M matrix where N is the number of data
 * points, and M is the number of features. 
 * 
 * The covariance matrix 
 *
 * The model parameters are put into *a_vpp, *u_mpp, and *S_mvpp. Any of
 * a_vpp, u_mpp, or S_mvpp is NULL if that value is not needed.
 *
 * If P_mpp, is not NULL, then the soft clustering (cluster membership) for each
 * data point is returned. In that case, *P_mpp will be N by K. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with an error message being set. Otherwise NO_ERROR is returned. 
 *
 * Index: clustering, EM, GMM
 *
 * -----------------------------------------------------------------------------
*/
int get_full_GMM
(
    int             num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp
)
{
    int num_points   = feature_mp->num_rows;
    int num_features = feature_mp->num_cols;
    int j;
    double max_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    Matrix*        u_mp    = NULL;
    Matrix_vector* S_mvp = NULL;
    Vector*        a_vp    = NULL;
    Matrix*        perturbed_feature_mp = NULL;
    Matrix*        P_mp = NULL;
    double         encoding_cost;
    int            result          = NO_ERROR;


    UNTESTED_CODE();  /* Since start of changes towards industrial version. */

    if ((result != ERROR) && (fs_data_perturbation > 10.0 * DBL_EPSILON))
    {
        result = perturb_unary_cluster_data(&perturbed_feature_mp, 
                                            feature_mp, fs_data_perturbation,
                                            (Int_vector*)NULL); 
        feature_mp = perturbed_feature_mp; 
    }

    if (fs_normalize_data)
    {
        warn_pso("Normalizing data does not make sense for full GMM.\n");
        warn_pso("Ignoring normalize option.\n");
    }

    if (fs_num_tries_per_cluster_count <= 1)
    {
        /*
         * Basic simple case. Doing it separately saves memory.
        */
        result = get_full_GMM_2(num_clusters, 
                                feature_mp, 
                                covariance_mask_mp, 
                                a_vpp, 
                                u_mpp,
                                S_mvpp, 
                                P_mpp, 
                                (double*)NULL); 
    }
    else
    {
        UNTESTED_CODE(); 

        for (j = 0; j < fs_num_tries_per_cluster_count; j++)
        {
            double log_likelihood;

            if (result == ERROR) { NOTE_ERROR(); break; }

            num_clusters = get_full_GMM_2(num_clusters, 
                                          feature_mp, 
                                          covariance_mask_mp, 
                                          (a_vpp == NULL) ? NULL: &a_vp, 
                                          (u_mpp == NULL) ? NULL : &u_mp, 
                                          (S_mvpp == NULL) ? NULL : &S_mvp, 
                                          (P_mpp == NULL) ? NULL : &P_mp, 
                                          &log_likelihood); 
            if (num_clusters == ERROR)
            {
                result = ERROR;
                break; 
            }

            verbose_pso(2, "Raw log likelihood with %d clusters is %.5e,\n", 
                        num_clusters, log_likelihood);
            encoding_cost = bic(num_clusters, num_features, num_points, 
                                FALSE, fs_tie_var, fs_tie_feature_var);

            log_likelihood -= encoding_cost; 

            verbose_pso(2,
                    "Adjusted log likelihood with %d clusters is %.5e,\n",
                    num_clusters, log_likelihood);

            if (log_likelihood > max_log_likelihood)
            {
                max_log_likelihood = log_likelihood; 

                if ((result != ERROR) && (u_mpp != NULL))
                {
                    result = copy_matrix(u_mpp, u_mp); 
                }

                if ((result != ERROR) && (S_mvpp != NULL))
                {
                    result = copy_matrix_vector(S_mvpp, S_mvp); 
                }

                if ((result != ERROR) && (a_vpp != NULL))
                {
                    result = copy_vector(a_vpp, a_vp); 
                }

                if ((result != ERROR) && (P_mpp != NULL))
                {
                    result = copy_matrix(P_mpp, P_mp); 
                }

            }

        }
    }

    free_matrix(P_mp);
    free_vector(a_vp);
    free_matrix(u_mp);
    free_matrix_vector(S_mvp);
    free_matrix(perturbed_feature_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ??? covariance_mask_mp ??? */
int get_full_GMM_2
(
    int             num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp, 
    Matrix**        u_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp,
    double*         log_likelihood_ptr 
)
{
    int           num_points   = feature_mp->num_rows;
    int           num_features = feature_mp->num_cols;
    Matrix*       perturbed_feature_mp = NULL;
    Matrix*        local_u_mp        = NULL;
    Matrix*  u_mp        = NULL;
    Matrix_vector* local_S_mvp       = NULL;
    Matrix_vector* S_mvp       = NULL;
    Matrix*        local_P_mp        = NULL;
    Matrix*  P_mp        = NULL;
    Vector*  a_vp        = NULL;
    Vector*        local_a_vp        = NULL;
    int            cluster;
    int            it, i, j;
    Matrix*        I_mp        = NULL;
    double         det;
    Vector*        sqrt_det_vp = NULL;
    Vector*        p_sum_vp    = NULL;
    Matrix_vector* S_inv_mvp   = NULL;
    Matrix*        S_mp        = NULL;
    Vector*        x_vp        = NULL;
    Vector*        d_vp        = NULL;
    Vector*        row_vp      = NULL;
    double         dot_product;
    double         log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double         prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double         diff;
    Matrix*        U_mp       = NULL;
    Vector*        u_vp       = NULL;
    Matrix*        V_trans_mp = NULL;
    Vector*        sigma_vp   = NULL;
    int            rank;
    Vector*        temp_vp    = NULL;
    int            num_good_points;
    int            result = NO_ERROR;


    /*
     * Need to make it so that we are iterating over points. 
     * 
     * Likely want to rethink the back-off. 
     *
     * Also want better initialization.
     *
     * Lots of testing! 
    */

    UNTESTED_CODE();

    num_good_points = num_points; 

    /* Kobus. We need to improve the following initialization of I_mp. */
    if (    (get_random_matrix(&I_mp, num_points, num_clusters)   == ERROR) 
         || (get_target_vector(&sqrt_det_vp, num_clusters)        == ERROR) 
         || (get_target_vector(&p_sum_vp, num_clusters)           == ERROR) 
         || (get_target_matrix_vector(&S_inv_mvp, num_clusters)   == ERROR)
       )
    {
        result = ERROR;
    }

    if ((result != ERROR) && (P_mpp == NULL))
    {
        if (get_zero_matrix(&local_P_mp, num_points, num_clusters) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            P_mp = local_P_mp;
        }
    }
    else 
    {
        if (get_zero_matrix(P_mpp, num_points, num_clusters) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            P_mp = *P_mpp;
        }
    }

    if ((result != ERROR) && (a_vpp == NULL))
    {
        if (get_target_vector(&local_a_vp, num_clusters) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            a_vp = local_a_vp;
        }
    }
    else 
    {
        if (get_target_vector(a_vpp, num_clusters) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            a_vp = *a_vpp;
        }
    }

    if ((result != ERROR) && (u_mpp == NULL))
    {
        if (get_target_matrix(&local_u_mp, num_clusters, num_features) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            u_mp = local_u_mp;
        }
    }
    else 
    {
        if (get_target_matrix(u_mpp, num_clusters, num_features) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            u_mp = *u_mpp;
        }
    }

    if ((result != ERROR) && (S_mvpp == NULL))
    {
        if (get_target_matrix_vector(&local_S_mvp, num_clusters) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            S_mvp = local_S_mvp;
        }
    }
    else 
    {
        if (get_target_matrix_vector(S_mvpp, num_clusters) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            S_mvp = *S_mvpp;
        }
    }

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        int backed_off_during_this_iteration = FALSE; 

        if (result == ERROR) { NOTE_ERROR(); break; }
        
        /* Compute point responsibilities */
        for (i = 0; i<num_points; i++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (I_mp->elements[ i ][ 0 ] >= 0.0)
            {
                double p_sum = sum_matrix_row_elements(I_mp, i);

                if (IS_ZERO_DBL(p_sum))  /* This is taken to be an outlier. */
                {
                    dbw(); 
                    for (j = 0; j<num_clusters; j++)
                    {
                        I_mp->elements[ i ][ j ] = DBL_NOT_SET;
                        P_mp->elements[ i ][ j ] = DBL_NOT_SET;
                    }
                    num_good_points--; 
                }
                else 
                {
                    result = ow_divide_matrix_row_by_scalar(I_mp, p_sum, i);
                }
            }
        }

        /*CONSTCOND*/
        while (TRUE)    /* We try to back of each cluster. */
        {   
            int back_off = NOT_SET; 

            if (result == ERROR) { NOTE_ERROR(); break; }

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double meat = 0.0; 

                for (i = 0; i < num_points; i++)
                {
                    if (I_mp->elements[ i ][ cluster ] >= 0.0)
                    {
                        meat += I_mp->elements[ i ][ cluster ]; 
                    }
                }

                if (meat < 1.5)
                {
                    back_off = cluster; 
                    break; 
                }
            }

            if (KJB_IS_SET(back_off))
            {
                int bad_cluster = back_off; 

                backed_off_during_this_iteration = TRUE; 

                verbose_pso(3, "Backing off cluster %d.\n", bad_cluster); 

                num_clusters--;
                
                if (num_clusters == 0)
                {
                    set_bug("All clusters have been removed."); 
                    result = ERROR; 
                    break; 
                }

                result = remove_matrix_col((Vector**)NULL, I_mp, bad_cluster);
                if (result == ERROR) { NOTE_ERROR(); break; } 

                result = remove_matrix_col((Vector**)NULL, P_mp, bad_cluster);
                if (result == ERROR) { NOTE_ERROR(); break; } 

                (a_vp->length)--;
                (p_sum_vp->length)--;
                (sqrt_det_vp->length)--; 

                for (i = 0; i<num_points; i++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    if (I_mp->elements[ i ][ 0 ] >= 0.0)
                    {
                        double p_sum = sum_matrix_row_elements(I_mp, i);

                        if (IS_ZERO_DBL(p_sum))  /* Taken to be an outlier. */
                        {
                            dbw(); 

                            for (j = 0; j<num_clusters; j++)
                            {
                                I_mp->elements[ i ][ j ] = DBL_NOT_SET;
                                P_mp->elements[ i ][ j ] = DBL_NOT_SET;
                            }
                            num_good_points--; 
                        }
                        else 
                        {
                            result = ow_divide_matrix_row_by_scalar(I_mp, p_sum,
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

        if (result == ERROR) { NOTE_ERROR(); break; }

        /* M-Step */

        result = get_zero_matrix(&u_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        /*
        // Calculate the means. 
        */
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            p_sum_vp->elements[ cluster ] = 0.0; 

            for (i = 0; i < num_points; i++)
            {
                double  p = I_mp->elements[ i ][ cluster ];

                if (p >= 0.0)
                {
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

        if (ow_divide_matrix_by_col_vector(u_mp, p_sum_vp)
            == ERROR)
        {
            db_rv(p_sum_vp); 
            result = ERROR;
            break;
        }

        /*
        // Calculate the new variances/covariances. We can't merge it with the
        // above because it depends on the calculation of the means.
        */
                
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_zero_matrix(&(S_mvp->elements[ cluster ]),
                                     num_features, num_features);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_matrix_row(&row_vp, u_mp, cluster); 
            if (result == ERROR) { NOTE_ERROR(); break; } 

            for (i = 0; i < num_points; i++)
            {
                double  p = I_mp->elements[ i ][ cluster ];

                if (p >= 0.0)
                {
                    /* get x_n */
                    result = get_matrix_row(&x_vp, feature_mp, i);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    /* get x_n - mu_k (the mean for the given cluster k) */
                    result = subtract_vectors(&d_vp, x_vp, row_vp); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    /* get (x_n - mu_k)^2 */ 
                    result = get_vector_outer_product(&S_mp, d_vp, d_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    /* multiply the resulting product 
                       by the responsibility of each cluster */
                    result = ow_multiply_matrix_by_scalar(S_mp, p); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    /* update the sum for the given cluster with the
                       calculated sum */
                    result = ow_add_matrices(S_mvp->elements[ cluster ],
                                             S_mp);
                    if (result == ERROR) { NOTE_ERROR(); break; } 
                }
            }

            if (result == ERROR) { NOTE_ERROR(); break; } 

            /*
            // Normalize the cluster variance sums by the total weight.  We
            // also add in some fancy stuff courtesy of A. Doucet. It is not
            // clear if it helps yet, and so I have only coded in the
            // independent case. In the non-independent case we have to work a
            // bit harder to get the inverse of the covariance matrix and its
            // determinant.
            */

            /* divide the covariance by the sum of responsibilities for 
               all points for that cluster */
            if (ow_divide_matrix_by_scalar(S_mvp->elements[ cluster ],
                                           p_sum_vp->elements[ cluster ])
                == ERROR)

            {
                dbe(p_sum_vp->elements[ cluster ]);
                result = ERROR; 
                break; 
            }

            for(j = 0; j < num_features; ++j)
            {
                S_mvp->elements[ cluster ]->elements[j][j] += fs_var_offset;
            }

            if (covariance_mask_mp != NULL)
            {
                result = ow_multiply_matrices_ew(S_mvp->elements[ cluster],
                                                 covariance_mask_mp); 
                if (result == ERROR) { NOTE_ERROR(); break; } 
            }

            result = do_svd(S_mvp->elements[ cluster ], 
                            &U_mp, &sigma_vp, &V_trans_mp, &rank);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if (rank < S_mvp->elements[ cluster ]->num_rows)
            {
                set_error("Covariance matrix does not have full rank.");
                result = ERROR; 
                break; 
            }

            det = multiply_vector_elements(sigma_vp);

            if (do_svd_matrix_inversion_2(&(S_inv_mvp->elements[ cluster]), 
                                          U_mp, sigma_vp, V_trans_mp)
                == ERROR)
            {
                dbe(det); 
                result = ERROR; 
                break; 
            }

            if (det < 1e-300) 
            {
                dbe(det);
                det = 1e-300; 
            }

            sqrt_det_vp->elements[ cluster ]= sqrt(det);

        }

        /* E Step */

        for (cluster = 0; cluster<num_clusters; cluster++)
        {
            double factor = a_vp->elements[ cluster ] /
                                            sqrt_det_vp->elements[ cluster ];
            Matrix* S_inv_mp = S_inv_mvp->elements[ cluster ];

            if (result == ERROR) { NOTE_ERROR(); break; } 

            result = get_matrix_row(&u_vp, u_mp, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }

            for (i = 0; i < num_points; i++)
            {
                if (P_mp->elements[ i ][ cluster ] >= 0.0)
                {
                    result = get_matrix_row(&x_vp, feature_mp, i);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = subtract_vectors(&d_vp, x_vp, u_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = multiply_matrix_and_vector(&temp_vp, S_inv_mp, 
                                                        d_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = get_dot_product(d_vp, temp_vp, &dot_product);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    if (dot_product < 0.0)
                    {
                        dbe(dot_product);
                        dot_product = 0.0; 
                    }

                    P_mp->elements[ i ][ cluster ]=  
                                            factor * exp(-0.5 * dot_product);

                    ASSERT_IS_FINITE_DBL(P_mp->elements[ i ][ cluster ]); 
                    /*
                    // If we wanted to put the constant in for some reason.
                    //
                    P_mp->elements[ i ][ cluster ] /= 
                              pow(2.0 * M_PI, ((double)num_features) / 2.0);
                    */
                }
                else
                {
                    dbe(P_mp->elements[ i ][ cluster ]);
                }
            }
        }

        verify_matrix(P_mp, NULL); 

        /* Compute the responsibilities for each point */
        for (i = 0; i < num_points; i++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if (P_mp->elements[ i ][ 0 ] >= 0.0)
            {
                double p_sum = sum_matrix_row_elements(P_mp, i);

                if (IS_ZERO_DBL(p_sum))  /* This is taken to be an outlier. */
                {
                    dbw();

                    for (j = 0; j<num_clusters; j++)
                    {
                        I_mp->elements[ i ][ j ] = DBL_NOT_SET;
                        P_mp->elements[ i ][ j ] = DBL_NOT_SET;
                    }
                    num_good_points--; 
                }
                else 
                {
                    /* pi_k*normal(...) / sum over all clusters*/
/*                    result = divide_matrix_by_scalar(&I_mp, P_mp, p_sum); */
                    result = copy_matrix_row(I_mp, i, P_mp, i);
                    if(result != ERROR)
                    {
                        result = ow_divide_matrix_row_by_scalar(I_mp, p_sum, i);
                    }
                }
            }
        }

        log_likelihood = 0.0; 

        if (result != ERROR)
        {
            for (i = 0; i < num_points; i++)
            {
                if (P_mp->elements[ i ][ 0 ] >= 0.0)
                {
                    double p_sum = sum_matrix_row_elements(P_mp, i); 

                    ASSERT(p_sum > 0.0);
                    log_likelihood += log(p_sum); 
                    ASSERT_IS_NUMBER_DBL(log_likelihood); 
                }
            }
        }

        if (     ( ! backed_off_during_this_iteration )
              && (IS_LESSER_DBL(log_likelihood, prev_log_likelihood))
           )
        {
            warn_pso("Log likelihood %.6e is less than previous %.6e.\n",
                     log_likelihood, prev_log_likelihood);
        }

        diff = log_likelihood - prev_log_likelihood;

        diff *= 2.0;
        diff /= (ABS_OF(log_likelihood + prev_log_likelihood));

        verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e\n", 
                    it + 1, log_likelihood, diff); 

        if (ABS_OF(diff) < fs_iteration_tolerance) break; 

        prev_log_likelihood = log_likelihood; 
    }

    verbose_pso(2, "%d points used as outliers.\n", 
                num_points - num_good_points);

    if (log_likelihood_ptr != NULL)
    {
        *log_likelihood_ptr = log_likelihood;
    }

    free_vector(sqrt_det_vp);
    free_vector(p_sum_vp);

    free_matrix_vector(S_inv_mvp);

    free_matrix(I_mp); 
    free_matrix(S_mp); 
    free_vector(x_vp); 
    free_vector(d_vp); 
    free_vector(row_vp); 
    free_vector(temp_vp); 
    free_vector(u_vp); 

    free_vector(sigma_vp); 
    free_matrix(U_mp); 
    free_matrix(V_trans_mp); 

    free_matrix(perturbed_feature_mp);

    free_matrix(local_u_mp);
    free_matrix(local_P_mp);
    free_matrix_vector(local_S_mvp);
    free_vector(local_a_vp);

    if (result == ERROR) return ERROR; else return num_clusters;
}

#endif  /* End of code block that is removed for export. */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* =============================================================================
 *                 get_full_GMM_3
 *                               
 * Finds a Gaussian mixture model (GMM) for the correlated features.
 *
 * This version handles held out data and initial model parameters BUT 
 * likelihood and cluster membership are computed using the DIAGONAL model.
 * This has two consequences:
 *  
 *  |1. learned covariances and means will not be as good as with get_full_GMM 
 *  |   or get_full_GMM_2, because point classifications will be wrong in some cases.
 *  |2. classification results returned in a_vpp will not take advantage of the 
 *  |   full diagonal model.
 *
 * If held out data isn't needed, prefer get_full_GMM_2.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with an error message being set. Otherwise NO_ERROR is returned. 
 *
 * Index: clustering, EM, GMM
 *
 * -----------------------------------------------------------------------------
*/
int get_full_GMM_3
(
    int             num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**            a_vpp, 
    Matrix**            u_mpp,
    Matrix_vector**     S_mvpp,
    Matrix**            P_mpp,
    double*             log_likelihood_ptr,
    const Int_vector*   held_out_indicator_vp,
    const Vector*       initial_a_vp,
    const Matrix*       initial_means_mp,
    const Matrix*       initial_var_mp,
    double*             held_out_log_likelihood_ptr,
    int*                num_iterations_ptr
)
{
    int           num_points          = feature_mp->num_rows;
    int           num_features        = feature_mp->num_cols;
    int           cluster;
    int           feature;
    int           it, i, rank;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double        var_offset = 0.0;
#else
    double        var_offset = fs_var_offset; 
#endif 
    double        det;
    Vector*       I_vp       = NULL;
    Vector*       a_vp       = NULL;
    Vector*       p_sum_vp   = NULL;
    Vector*       p_square_sum_vp = NULL;
    Vector*       x_vp       = NULL;
    Vector*       x2_vp      = NULL;
    Vector*       u_vp    = NULL;
    Vector*       var_vp  = NULL;
    Vector*       sigma_vp   = NULL;
    Vector*       d_vp        = NULL;
    Vector*       sqrt_det_vp = NULL;
    Vector*       row_vp      = NULL;
    
    Matrix*       P_mp       = NULL;
    Matrix*       var_mp     = NULL;
    Matrix*       new_var_mp = NULL;
    Matrix*       new_u_mp   = NULL;
    Matrix*       u_mp    = NULL;
    Matrix*       S_mp        = NULL;
    Matrix*       U_mp       = NULL;
    Matrix*       V_trans_mp = NULL;
    
    Matrix_vector* S_mvp       = NULL;
    Matrix_vector* S_inv_mvp   = NULL;
    double        log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        held_out_log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_held_out_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        diff, held_out_diff;
    int           result       = NO_ERROR;
#ifdef DONT_USE_GAUSS_ROUTINE
    Vector*       log_sqrt_det_vp  = NULL;
#endif 
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    Matrix* I_mp          = NULL;
#endif 
    

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    ERE(get_cluster_random_matrix(&I_mp, num_points, num_clusters));
#endif 

    if (P_mpp != NULL)
    {
        ERE(get_target_matrix(P_mpp, num_points, num_clusters));
    }
if (held_out_indicator_vp != NULL)
    {
        if (held_out_indicator_vp->length != num_points)
        {
            result = ERROR;
        }
    }
    
    if ( (get_target_vector(&sqrt_det_vp, num_clusters)== ERROR) 
         || (get_target_matrix_vector(&S_inv_mvp, num_clusters)   == ERROR)
         ||   (get_target_matrix(&P_mp, num_points, num_clusters) == ERROR)
         || (get_target_matrix_vector(&S_mvp, num_clusters) == ERROR)
         ||  (get_target_vector(&I_vp, num_clusters) == ERROR) 
         || (get_target_vector(&a_vp, num_clusters) == ERROR) 
#ifdef DONT_USE_GAUSS_ROUTINE
         || (get_target_vector(&log_sqrt_det_vp, num_clusters) == ERROR) 
#endif 
       )
    {
        result = ERROR;
    }

    dbi(num_clusters); 

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        log_likelihood = 0.0;

        held_out_log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_square_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_u_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_var_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
                
        if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
        {
            if (initial_means_mp != NULL)
            {   
                ERE(get_target_matrix(&u_mp, num_clusters, num_features));
                for (i = 0; i < num_clusters; i++)
                {
                    ERE(copy_matrix_row(u_mp, i, initial_means_mp, i));
                }
            }
            else
            {
                 set_bug("Initial cluster means matrix is NULL.\n");
                 return ERROR;
                 
                /*
                warn_pso("Initial cluster means matrix is NULL: Using random means.\n");
                ERE(get_random_matrix(&u_mp, num_clusters, num_features));
                */
            }

            if (initial_var_mp != NULL)
            {
                ERE(get_target_matrix(&var_mp, num_clusters, num_features));
                for (i = 0; i < num_clusters; i++)
                {
                    ERE(copy_matrix_row(var_mp, i, initial_var_mp, i));
                }
            }
            else
            {
                set_bug("Initial cluster variances matrix is NULL.\n");
                return ERROR;
                
                /*
                warn_pso("Initial cluster variances matrix is NULL: Using random variances.\n");
                ERE(get_random_matrix(&var_mp, num_clusters, num_features));
                */
            }
            for (i = 0; i < num_clusters; i++)
            {
                ERE(ow_add_scalar_to_matrix_row(var_mp, var_offset, i));
            }

            if (initial_a_vp != NULL)
            {
                ERE(get_target_vector(&a_vp, num_clusters));
                for (i = 0; i < num_clusters; i++)
                {
                    a_vp->elements[i] = initial_a_vp->elements[i];
                }
                ERE(ow_normalize_vector(a_vp, NORMALIZE_BY_SUM));
            }
            else
            {
                set_bug("Initial cluster priors vector is NULL.\n");
                return ERROR;
                
                /*
                warn_pso("Initial cluster priors vector is NULL: Using random priors.\n");
                for (i = 0; i < num_clusters; i++)
                {
                    a_vp->elements[i] = kjb_rand();
                }
                ERE(ow_add_scalar_to_vector(a_vp, 0.2 * kjb_rand() / num_clusters)); 
                ERE(ow_normalize_vector(a_vp, NORMALIZE_BY_SUM));
                */
            }

#ifdef DONT_USE_GAUSS_ROUTINE
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double temp = 0.0;

                if (result == ERROR) { NOTE_ERROR(); break; }

                for (feature = 0; feature < num_features; feature++)
                {
                    double var = var_mp->elements[ cluster ][ feature ]; 

                    temp += SAFE_LOG(var);
                }

                log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
            }
            verify_vector(log_sqrt_det_vp, NULL);
#endif 
        }
        
        for (i = 0; i<num_points; i++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_matrix_row(&x_vp, feature_mp, i);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors != TRUE) )
            {
                /* E Step init */

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
                result = get_matrix_row(&I_vp, I_mp, i);
#else
                /* ??? this has already been done at the beginning */
                result = get_target_vector(&I_vp, num_clusters); 
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double r = kjb_rand();
                    double p = pow(r, 5.0); /* Perhaps should be an option. */

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    I_vp->elements[ cluster ] = p;

                    if (kjb_rand() < 1.0 / num_clusters) 
                    {
                        I_vp->elements[ cluster ] += 0.5; 
                    }
                }

                result = ow_scale_vector_by_sum(I_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_to_vector(I_vp, 0.2 * kjb_rand() / num_clusters); 
                if (result == ERROR) { NOTE_ERROR(); break; }
#endif 

                result = ow_scale_vector_by_sum(I_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            
            if ( (it > 0) || (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
            {
                /* E Step */

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double log_a = SAFE_LOG(a_vp->elements[ cluster ]);
#ifdef DONT_USE_GAUSS_ROUTINE
                    double* x_ptr = x_vp->elements;
                    double* u_ptr = u_mp->elements[ cluster ];
                    double* v_ptr = var_mp->elements[ cluster ];
                    double temp, dev;
                    double d = 0.0;
#else 

                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = get_matrix_row(&u_vp, u_mp, cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&var_vp, var_mp, cluster);
#endif 
                    if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef DONT_USE_GAUSS_ROUTINE
                    for (feature = 0; feature < num_features; feature++)
                    {
                        dev = *u_ptr - *x_ptr;

                        ASSERT_IS_NUMBER_DBL(dev); 
                        ASSERT_IS_FINITE_DBL(dev); 

                        temp = (dev / (*v_ptr)) * dev;

                        d += temp; 

                        ASSERT_IS_NUMBER_DBL(d); 
                        ASSERT_IS_FINITE_DBL(d); 

                        u_ptr++;
                        v_ptr++;
                        x_ptr++;
                    }

                    ASSERT_IS_NUMBER_DBL(d); 
                    ASSERT_IS_FINITE_DBL(d); 

                    I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ]; 
#else
                    result = get_log_gaussian_density(x_vp, u_vp, var_vp, 
                                                      &(I_vp->elements[ cluster ]));
                    if (result == ERROR) { NOTE_ERROR(); break; }
                    I_vp->elements[ cluster ] += log_a; 
#endif 

#ifndef DONT_USE_GAUSS_ROUTINE
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS_LL
                    I_vp->elements[ cluster ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
#endif 
#endif 
                }

                if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
                {
                    held_out_log_likelihood += ow_exp_scale_by_sum_log_vector(I_vp);
                }
                else
                {
                    log_likelihood += ow_exp_scale_by_sum_log_vector(I_vp);
                }
            }

            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_vp), 1.0, 0.00001);

            if (result == ERROR) { NOTE_ERROR(); break; }

            result = put_matrix_row(P_mp, I_vp, i);
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (P_mpp != NULL)
            {
                result = put_matrix_row(*P_mpp, I_vp, i);
            }

            /* M-Step */
            
            if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
            {
                /* Do not do the M-step for held out points */
            }
            else
            {
                result = multiply_vectors(&x2_vp, x_vp, x_vp);
                if (result == ERROR) { NOTE_ERROR(); break; } 

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double  p = I_vp->elements[ cluster ];

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_add_scalar_times_vector_to_matrix_row(new_u_mp, 
                                                                      x_vp, p,
                                                                      cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; } 
    
                    result = ow_add_scalar_times_vector_to_matrix_row(new_var_mp, 
                                                                      x2_vp, p,
                                                                      cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    p_sum_vp->elements[ cluster ] += p; 

                    p_square_sum_vp->elements[ cluster ] += (p * p);
                }
            }
        }

        /* M-Step cleanup */

        /* Calculating the means */
        if (divide_matrix_by_col_vector(&u_mp, new_u_mp, p_sum_vp) == ERROR)
        {
            db_rv(p_sum_vp); 
            result = ERROR;
            break; 
        }


        /*
        // Calculate the new variances/covariances. We can't merge it with the
        // above because it depends on the calculation of the means.
        */

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            result = get_zero_matrix(&(S_mvp->elements[ cluster ]),
                                     num_features, num_features);
            if (result == ERROR) { NOTE_ERROR(); break; }
            
            result = get_matrix_row(&row_vp, u_mp, cluster); 
            if (result == ERROR) { NOTE_ERROR(); break; } 

            for (i = 0; i < num_points; i++)
            {
                double  p = P_mp->elements[ i ][ cluster ];

                if (p >= 0.0)
                {
                    /* get x_n */
                    result = get_matrix_row(&x_vp, feature_mp, i);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    /* get x_n - mu_k (the mean for the given cluster k) */
                    result = subtract_vectors(&d_vp, x_vp, row_vp); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    /* get (x_n - mu_k)^2 */ 
                    result = get_vector_outer_product(&S_mp, d_vp, d_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    /* multiply the resulting product 
                       by the responsibility of each cluster */
                    result = ow_multiply_matrix_by_scalar(S_mp, p); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    /* update the sum for the given cluster with the
                       calculated sum */
                    result = ow_add_matrices(S_mvp->elements[ cluster ],
                                             S_mp);
                    if (result == ERROR) { NOTE_ERROR(); break; } 
                }
            }

            if (result == ERROR) { NOTE_ERROR(); break; } 

  /*
            // Normalize the cluster variance sums by the total weight.  We
            // also add in some fancy stuff courtesy of A. Doucet. It is not
            // clear if it helps yet, and so I have only coded in the
            // independent case. In the non-independent case we have to work a
            // bit harder to get the inverse of the covariance matrix and its
            // determinant.
            */

            /* divide the covariance by the sum of responsibilities for 
               all points for that cluster */
            if (ow_divide_matrix_by_scalar(S_mvp->elements[ cluster ],
                                           p_sum_vp->elements[ cluster ])
                == ERROR)

            {
                dbe(p_sum_vp->elements[ cluster ]);
                result = ERROR; 
                break; 
            }

            if (covariance_mask_mp != NULL)
            {
                result = ow_multiply_matrices_ew(S_mvp->elements[ cluster],
                                                 covariance_mask_mp); 
                if (result == ERROR) { NOTE_ERROR(); break; } 
            }

            result = do_svd(S_mvp->elements[ cluster ], 
                            &U_mp, &sigma_vp, &V_trans_mp, &rank);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if (rank < S_mvp->elements[ cluster ]->num_rows)
            {
                set_error("Covariance matrix does not have full rank.");
                result = ERROR; 
                break; 
            }

            det = multiply_vector_elements(sigma_vp);

            if (do_svd_matrix_inversion_2(&(S_inv_mvp->elements[ cluster]), 
                                          U_mp, sigma_vp, V_trans_mp)
                == ERROR)
            {
                dbe(det); 
                result = ERROR; 
                break; 
            }

            if (det < 1e-300) 
            {
                dbe(det);
                det = 1e-300; 
            }

            sqrt_det_vp->elements[ cluster ]= sqrt(det);
        
        }

        result = get_initialized_matrix(&var_mp, num_clusters, num_features, 
                                        DBL_NOT_SET);
        if (result == ERROR) { NOTE_ERROR(); break; }
        for(cluster = 0; cluster < num_clusters; cluster++)
        {
            for(feature = 0; feature < num_features; feature++)
            {
                var_mp->elements[cluster][feature] = S_mvp->elements[cluster]->elements[feature][feature];
            }
        }

        if (fs_tie_var == TRUE)
        {
            double tie_var = 0.0;
            
            for (feature = 0; feature < num_features; feature++)
            {
                tie_var = 0.0;
                
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    tie_var = tie_var + var_mp->elements[ cluster ][ feature ];
                }

                tie_var = tie_var/num_clusters;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_var;
                }
            }
        }
        else if (fs_tie_feature_var == TRUE)
        {
            double tie_feature_var = 0.0;

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    tie_feature_var = tie_feature_var + var_mp->elements[ cluster ][ feature ];
                }
            }

            tie_feature_var = tie_feature_var/(num_clusters * num_features);

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_feature_var;
                }
            }
        }
            
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            result = ow_add_scalar_to_matrix_row(var_mp, var_offset, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }
        
#ifdef DONT_USE_GAUSS_ROUTINE
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double temp = 0.0;

            if (result == ERROR) { NOTE_ERROR(); break; }

            for (feature = 0; feature < num_features; feature++)
            {
                double var = var_mp->elements[ cluster ][ feature ]; 

                temp += SAFE_LOG(var);
            }

            log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
        }
        verify_vector(log_sqrt_det_vp, NULL);
#endif 

        result = scale_vector_by_sum(&a_vp, p_sum_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; } 

        if (it > 0) 
        {
            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood +  prev_log_likelihood));

            verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e ", 
                        it + 1, log_likelihood, diff); 


            held_out_diff = held_out_log_likelihood - prev_held_out_log_likelihood;

            held_out_diff *= 2.0;
            held_out_diff /= (ABS_OF(held_out_log_likelihood + prev_held_out_log_likelihood));
            
            verbose_pso(3, "| Held out is %12e  |  %10e\n",
                        held_out_log_likelihood, held_out_diff); 

            if (fs_EM_stop_criterion_training_LL == TRUE)
            {
                if (ABS_OF(diff) < fs_iteration_tolerance) break; 
            }
            else if (fs_EM_stop_criterion_held_out_LL == TRUE)
            {
                if (ABS_OF(held_out_diff) < fs_iteration_tolerance) break;
            }

            prev_log_likelihood = log_likelihood;

            prev_held_out_log_likelihood = held_out_log_likelihood;
        }
    }

    if (result != ERROR)
    {
        if (log_likelihood_ptr != NULL)
        {
            *log_likelihood_ptr = log_likelihood;
        }

        if (held_out_log_likelihood_ptr != NULL)
        {
            *held_out_log_likelihood_ptr = held_out_log_likelihood;
        }

        if (num_iterations_ptr != NULL)
        {
            *num_iterations_ptr = it;
        }

        if (a_vpp != NULL) 
        {
            result = copy_vector(a_vpp, a_vp);
        }

        if (u_mpp != NULL)
        {
            result = copy_matrix(u_mpp, u_mp); 
        }

        if (S_mvpp != NULL)
        {
            result = copy_matrix_vector(S_mvpp, S_mvp); 
        }
    }
   

    free_vector(I_vp); 
    free_vector(a_vp);
    free_vector(p_sum_vp);
    free_vector(p_square_sum_vp);
    free_vector(x_vp); 
    free_vector(x2_vp); 
    free_vector(u_vp); 
    free_vector(var_vp); 
    free_vector(sigma_vp); 
    free_vector(d_vp); 
    free_vector(sqrt_det_vp); 
    free_vector(row_vp); 

    free_matrix(P_mp); 
    free_matrix(var_mp); 
    free_matrix(new_var_mp); 
    free_matrix(new_u_mp); 
    free_matrix(u_mp); 
    free_matrix(S_mp);
    free_matrix(U_mp);
    free_matrix(V_trans_mp);

    free_matrix_vector(S_mvp);
    free_matrix_vector(S_inv_mvp);

#ifdef DONT_USE_GAUSS_ROUTINE
    free_vector(log_sqrt_det_vp);
#endif 
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
     free_matrix(I_mp); 
#endif 

    if (result == ERROR) 
    {
        return ERROR; 
    }
    else 
    {
        return num_clusters;
    }
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                 get_independent_GMM
 *                               
 * Finds a Gaussian mixture model (GMM) for independent features.
 *
 * This routine finds a Gaussian mixture model (GMM) for the data under the
 * assumption that the features are independent. The model is fit with EM. Some
 * features are controlled via the set facility. 
 *
 * In particular, it fits:
 * |         p(x) = sum  a-sub-i *  g(u-sub-i, v-sub-i, x)
 * |                 i
 * where a-sub-i is the prior probability for the mixuture compoenent (cluster),
 * u-sub-i is the mean vector for component i, v-sub-i is the variance for the
 * component, and g(u,v,x) is a Gaussian with diagonal covariance (i.e., the
 * features are assumed to be independent, given the cluster). 
 *
 * The argument num_clusters is the number of requested mixture compoenent
 * (clusters), K. 
 *
 * The data matrix feature_mp is an N by M matrix where N is the number of data
 * points, and M is the number of features. 
 *
 * The model parameters are put into *a_vpp (prior), *u_mpp (mean), and 
 * *v_mpp (variance). Any of a_vpp, u_mpp, or v_mpp is NULL if that value is not needed.
 *
 * Both u-sub-i and v-sub-i are vectors, and they are put into the i'th row of
 * *u_mpp and *v_ppp, respectively. The matrices are thus K by M. 
 * 
 * If P_mpp, is not NULL, then the soft clustering (cluster membership) for each
 * data point is returned. In that case, *P_mpp will be N by K. 
 *
 * If missing data is enabled (i.e. enable_respect_missing_values() was called), 
 * features with value DBL_MISSING do not contribute to sufficient statistics or 
 * likelihood computations.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with an error message being set. Otherwise NO_ERROR is returned. 
 *
 * Related:
 *      enable_respect_missing_values, respect_missing_values
 *  
 * Index: clustering, EM, GMM
 *
 * -----------------------------------------------------------------------------
*/
int get_independent_GMM
(
    int           num_clusters,
    const Matrix* feature_mp,
    Vector**      a_vpp,
    Matrix**      u_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp
)
{
    int        num_points   = feature_mp->num_rows;
    int        num_features = feature_mp->num_cols;
    int        j;
    double     max_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    Matrix*    u_mp = NULL;
    Matrix*    var_mp = NULL;
    Vector*    a_vp = NULL; 
    Matrix*    P_mp = NULL; 
    Matrix*    perturbed_feature_mp = NULL;
    double     encoding_cost;
    int        result        = NO_ERROR;
    Matrix*        norm_feature_mp = NULL;
    Vector*        feature_mean_vp = NULL;
    Vector*        feature_var_vp  = NULL;
    const double norm_stdev = 1.0; 

    if(num_clusters <= 0)
    {
        set_error("num_clusters must be at least 1");
        return ERROR;
    }


    UNTESTED_CODE();  /* Since start of changes towards industrial version. */

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

    if (fs_num_tries_per_cluster_count <= 1)
    {
        /*
         * Basic simple case. Doing it separately saves memory.
        */
        if(respect_missing_values())
        {
            result = get_independent_GMM_2_with_missing_data(
                                           num_clusters, 
                                           feature_mp, 
                                           a_vpp, 
                                           u_mpp,
                                           var_mpp, 
                                           P_mpp, 
                                           (double*)NULL); 
        }
        else
        {
            result = get_independent_GMM_2(num_clusters, 
                                           feature_mp, 
                                           a_vpp, 
                                           u_mpp,
                                           var_mpp, 
                                           P_mpp, 
                                           (double*)NULL); 
        }

    }
    else
    {
        UNTESTED_CODE(); 

        for (j = 0; j < fs_num_tries_per_cluster_count; j++)
        {
            double log_likelihood;

            if (result == ERROR) { NOTE_ERROR(); break; }

            if( respect_missing_values() )
            {
                num_clusters = get_independent_GMM_2_with_missing_data(
                                                    num_clusters, 
                                                    feature_mp, 
                                                    (a_vpp == NULL) ? NULL: &a_vp, 
                                                    (u_mpp == NULL) ? NULL : &u_mp, 
                                                    (var_mpp == NULL) ? NULL : &var_mp, 
                                                    (P_mpp == NULL) ? NULL : &P_mp, 
                                                    &log_likelihood); 
            }
            else
            {
                num_clusters = get_independent_GMM_2(num_clusters, 
                                                     feature_mp, 
                                                     (a_vpp == NULL) ? NULL: &a_vp, 
                                                     (u_mpp == NULL) ? NULL : &u_mp, 
                                                     (var_mpp == NULL) ? NULL : &var_mp, 
                                                     (P_mpp == NULL) ? NULL : &P_mp, 
                                                     &log_likelihood); 
            }
            if (num_clusters == ERROR)
            {
                result = ERROR;
                break; 
            }

            verbose_pso(2, "Raw log likelihood with %d clusters is %.5e,\n", 
                        num_clusters, log_likelihood);

            encoding_cost = bic(num_clusters, num_features, num_points, 
                                TRUE, fs_tie_var, fs_tie_feature_var);
            log_likelihood -= encoding_cost; 

            verbose_pso(2,
                    "Adjusted log likelihood with %d clusters is %.5e,\n",
                    num_clusters, log_likelihood);

            if (log_likelihood > max_log_likelihood)
            {
                max_log_likelihood = log_likelihood; 

                if ((result != ERROR) && (u_mpp != NULL))
                {
                    result = copy_matrix(u_mpp, u_mp); 
                }

                if ((result != ERROR) && (var_mpp != NULL))
                {
                    result = copy_matrix(var_mpp, var_mp); 
                }
                                                                                                    
                if ((result != ERROR) && (a_vpp != NULL))
                {
                    result = copy_vector(a_vpp, a_vp); 
                }
                                                                                                    
                if ((result != ERROR) && (P_mpp != NULL))
                {
                    result = copy_matrix(P_mpp, P_mp); 
                }
            }

        }
    }

    if ((result != ERROR) && (fs_normalize_data))
    {
        result = ow_un_normalize_cluster_data((u_mpp == NULL) ? NULL : *u_mpp, 
                                              (var_mpp == NULL) ? NULL : *var_mpp, 
                                              feature_mean_vp, 
                                              feature_var_vp, 
                                              norm_stdev); 
    }

    free_matrix(norm_feature_mp);
    free_vector(feature_var_vp); 
    free_vector(feature_mean_vp); 
    free_vector(a_vp);
    free_matrix(u_mp);
    free_matrix(var_mp);
    free_matrix(P_mp);
    free_matrix(perturbed_feature_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                 get_independent_GMM_2
 * This version doesn't apply optional preprocessing
 * (e.g. data perturbation, normalization), or multiple runs.  If needed, see 
 * get_independent_GMM().
 *
 * This version does not respect missing data; if needed, see 
 * get_independent_GMM_2_with_missing_data().
 *
 * Related:
 *      get_independent_GMM,  get_independent_GMM_2_with_missing_data
 *
 * Index: clustering, EM, GMM
 *
 * -----------------------------------------------------------------------------
*/
int get_independent_GMM_2
(
    int           num_clusters,
    const Matrix* feature_mp,
    Vector**      a_vpp, 
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp,
    double*       log_likelihood_ptr 
)
{
    int           num_points          = feature_mp->num_rows;
    int           num_features        = feature_mp->num_cols;
    int           cluster;
    int           feature;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double        var_offset = 0.0;
#else
    double        var_offset = fs_var_offset; 
#endif 
    int           it, i;
    Vector*       I_vp        = NULL;
    Vector*       a_vp        = NULL;
    Vector*       p_sum_vp    = NULL;
    Matrix*       var_mp        = NULL;
    Matrix*       new_var_mp        = NULL;
    Matrix*       new_u_mp        = NULL;
    Vector*       x_vp        = NULL;
    Vector*       x2_vp        = NULL;
    double        log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        diff;
    Matrix*       u_mp    = NULL;
    Vector*       u_vp    = NULL;
    Vector*       var_vp  = NULL;
    int           result       = NO_ERROR;
#ifdef DONT_USE_GAUSS_ROUTINE
    Vector*       log_sqrt_det_vp  = NULL;
#endif 
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    Matrix* I_mp = NULL; 
#endif 
#ifndef REGRESS_NO_HACK_FOR_ZERO_MEMBERSHIP
    int first_null_cluster_spotted = TRUE; 
#endif 

    if(num_clusters <= 0)
    {
        set_error("num_clusters must be at least 1");
        return ERROR;
    }

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    ERE(get_cluster_random_matrix(&I_mp, num_points, num_clusters)); 
#endif 

    if (P_mpp != NULL)
    {
        ERE(get_target_matrix(P_mpp, num_points, num_clusters));
    }

    if (    (get_target_vector(&I_vp, num_clusters) == ERROR) 
         || (get_target_vector(&a_vp, num_clusters)             == ERROR) 
#ifdef DONT_USE_GAUSS_ROUTINE
         || (get_target_vector(&log_sqrt_det_vp, num_clusters)      == ERROR) 
#endif 
       )
    {
        result = ERROR;
    }

    dbi(num_clusters); 

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_u_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_var_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
                
        for (i = 0; i<num_points; i++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_matrix_row(&x_vp, feature_mp, i);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if (it == 0)
            {
                /* E Step init */

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
                result = get_matrix_row(&I_vp, I_mp, i);
#else

#ifdef HOW_IT_WAS_FEB_4_07
                result = get_target_vector(&I_vp, num_clusters);
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double r = kjb_rand();
                    double p = pow(r, 5.0); /* Perhaps should be an option. */

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    I_vp->elements[ cluster ] = p;

                    if (kjb_rand() < 1.0 / num_clusters) 
                    {
                        I_vp->elements[ cluster ] += 0.5; 
                    }
                }
#else
                /*
                 * Simple initilaization: One cluster per point, but also make
                 * it so that every cluster gets a tiny bit of weight due to
                 * this point, so that we know that every cluster has some
                 * weight. 
                */
                result = get_random_vector(&I_vp, num_clusters);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_divide_vector_by_scalar(I_vp, 20.0 * (double)num_points);
                if (result == ERROR) { NOTE_ERROR(); break; }

                cluster = (int)(((double)num_clusters) * kjb_rand());
                if (cluster == num_clusters) cluster--; 
                I_vp->elements[ cluster ] = 1.0; 
                
#endif 

                result = ow_scale_vector_by_sum(I_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_to_vector(I_vp, 0.2 * kjb_rand() / num_clusters); 
                if (result == ERROR) { NOTE_ERROR(); break; }
#endif 

                result = ow_scale_vector_by_sum(I_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            else
            {
                /* E Step */

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double log_a = SAFE_LOG(a_vp->elements[ cluster ]);
#ifdef DONT_USE_GAUSS_ROUTINE
                    double* x_ptr = x_vp->elements;
                    double* u_ptr = u_mp->elements[ cluster ];
                    double* v_ptr = var_mp->elements[ cluster ];
                    double temp, dev;
                    double d = 0.0;
#else 

                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = get_matrix_row(&u_vp, u_mp, cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&var_vp, var_mp, cluster);
#endif 
                    if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef DONT_USE_GAUSS_ROUTINE
                    for (feature = 0; feature < num_features; feature++)
                    {
                        dev = *u_ptr - *x_ptr;

                        ASSERT_IS_NUMBER_DBL(dev); 
                        ASSERT_IS_FINITE_DBL(dev); 

                        temp = (dev / (*v_ptr)) * dev;

                        d += temp; 

                        ASSERT_IS_NUMBER_DBL(d); 
                        ASSERT_IS_FINITE_DBL(d); 

                        u_ptr++;
                        v_ptr++;
                        x_ptr++;
                    }

                    ASSERT_IS_NUMBER_DBL(d); 
                    ASSERT_IS_FINITE_DBL(d); 

                    I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ]; 
#else
                    result = get_log_gaussian_density(x_vp, u_vp, var_vp, 
                                                      &(I_vp->elements[ cluster ]));
                    if (result == ERROR) { NOTE_ERROR(); break; }
                    I_vp->elements[ cluster ] += log_a; 
#endif 

#ifndef DONT_USE_GAUSS_ROUTINE
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS_LL
                    I_vp->elements[ cluster ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
#endif 
#endif 
                }

                log_likelihood += ow_exp_scale_by_sum_log_vector(I_vp);
            }

            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_vp), 1.0, 0.00001);

            if (result == ERROR) { NOTE_ERROR(); break; }

            if (P_mpp != NULL)
            {
                result = put_matrix_row(*P_mpp, I_vp, i);
            }

            /* M-Step */

            result = multiply_vectors(&x2_vp, x_vp, x_vp);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double  p = I_vp->elements[ cluster ];

                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_times_vector_to_matrix_row(new_u_mp, 
                                                                  x_vp, p,
                                                                  cluster);
                if (result == ERROR) { NOTE_ERROR(); break; } 

                result = ow_add_scalar_times_vector_to_matrix_row(new_var_mp, 
                                                                  x2_vp, p,
                                                                  cluster);
                if (result == ERROR) { NOTE_ERROR(); break; } 

                p_sum_vp->elements[ cluster ] += p; 
            }
        }

        /* M-Step cleanup */

#ifdef REGRESS_NO_HACK_FOR_ZERO_MEMBERSHIP
        if (divide_matrix_by_col_vector(&u_mp, new_u_mp, p_sum_vp) == ERROR)
        {
            NOTE_ERROR();
            db_rv(p_sum_vp); 
            result = ERROR;
            break; 
        }
#else
        if (copy_matrix(&u_mp, new_u_mp) == ERROR)
        {
            db_rv(p_sum_vp); 
            result = ERROR;
            break; 
        }

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double s = p_sum_vp->elements[ cluster ];

            if (s > 10.0 * DBL_MIN)
            {
                result = ow_divide_matrix_row_by_scalar(u_mp, s, cluster);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            else if (first_null_cluster_spotted)
            {
                first_null_cluster_spotted = FALSE;
                warn_pso("At least one cluster has no members.\n"); 
            }
        }
#endif 

        result = get_initialized_matrix(&var_mp, num_clusters, num_features, 
                                        DBL_NOT_SET);
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double s = p_sum_vp->elements[ cluster ];

            for (feature = 0; feature < num_features; feature++)
            {
                double u = u_mp->elements[ cluster ][ feature ];
                double u2 = s * u * u;
                double var = new_var_mp->elements[ cluster ][ feature ] - u2;

                if (var  < 0.0) 
                {
                    /* 
                    // This does happen! The calculation of variance
                    // this way is not numerically stable. 
                    */
                    var = 0.0;
                }

                var_mp->elements[ cluster ][ feature ] = var;
            }

#ifndef REGRESS_NO_HACK_FOR_ZERO_MEMBERSHIP
            if (s > 10.0 * DBL_MIN)
            {
#endif 
                result = ow_divide_matrix_row_by_scalar(var_mp, s, cluster);
                if (result == ERROR) { NOTE_ERROR(); break; }
#ifndef REGRESS_NO_HACK_FOR_ZERO_MEMBERSHIP
            }
#endif 

            verify_matrix(var_mp, NULL); 

            /* Moved to later due to the added options of tying variances:
            result = ow_add_scalar_to_matrix_row(var_mp, var_offset, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }
            */
        }
        
        if (fs_tie_var == TRUE)
        {
            double tie_var = 0.0;
            
            for (feature = 0; feature < num_features; feature++)
            {
                tie_var = 0.0;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    tie_var += var_mp->elements[ cluster ][ feature ];
                }

                tie_var /= num_clusters;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_var;
                }
            }
        }
        else if (fs_tie_feature_var == TRUE)
        {
            double tie_feature_var = 0.0;

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    tie_feature_var += var_mp->elements[ cluster ][ feature ];
                }
            }

            tie_feature_var /= (num_clusters * num_features);

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_feature_var;
                }
            }
        }
            
        else if (fs_tie_cluster_var == TRUE)
        {
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double tie_cluster_var = 0.0;

                for (feature = 0; feature < num_features; feature++)
                {
                    tie_cluster_var += var_mp->elements[ cluster ][ feature ];
                }

                tie_cluster_var /= num_features;

                for (feature = 0; feature < num_features; feature++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_cluster_var;
                }
            }
        }
            
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            result = ow_add_scalar_to_matrix_row(var_mp, var_offset, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }

#ifdef DONT_USE_GAUSS_ROUTINE
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double temp = 0.0;

            if (result == ERROR) { NOTE_ERROR(); break; }

            for (feature = 0; feature < num_features; feature++)
            {
                double var = var_mp->elements[ cluster ][ feature ]; 

                temp += SAFE_LOG(var);
            }

            log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
        }
        verify_vector(log_sqrt_det_vp, NULL);
#endif

        result = scale_vector_by_sum(&a_vp, p_sum_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; } 

        if (it > 0) 
        {
            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood +  prev_log_likelihood));

            verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e\n", 
                        it + 1, log_likelihood, diff); 

            if (ABS_OF(diff) < fs_iteration_tolerance) break; 

            prev_log_likelihood = log_likelihood; 
        }
    }

    if (result != ERROR)
    {
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
        result = copy_matrix(var_mpp, var_mp);
    }

    if ((result != ERROR) && (means_mpp != NULL))
    {
        result = copy_matrix(means_mpp, u_mp);
    }

    free_vector(I_vp); 
    free_vector(p_sum_vp);
    free_vector(a_vp);
    free_matrix(new_u_mp); 
    free_matrix(u_mp); 
    free_matrix(new_var_mp); 
    free_matrix(var_mp); 
#ifdef DONT_USE_GAUSS_ROUTINE
    free_vector(log_sqrt_det_vp);
#endif 

    free_vector(x2_vp); 
    free_vector(x_vp); 
    free_vector(var_vp); 
    free_vector(u_vp); 

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
     free_matrix(I_mp); 
#endif 

    if (result == ERROR) return ERROR; else return num_clusters;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                 get_independent_GMM_3
 *
 * Similar to get_independent_GMM_2, but adds support for held out data and 
 * pre-initialized clusters.
 * 
 * Initial means and variances are used only if option
 * fs_use_initialized_cluster_means_variances_and_priors is set.  
 *
 * Miscellaneous difference that have crept in over time:
 *  
 * |1. Default  intitialization scheme differs slightly .
 * |2. This version doesn't handle empty clusters gracefully
 * |3. This version doesn't handle fs_tie_cluster_var (only fs_tie_var is handled)
 *
 *
 * Related:
 *      get_independent_GMM_2, get_independent_GMM_2_with_missing_data
 *
 * Index: clustering, EM, GMM
 *
 * -----------------------------------------------------------------------------
*/

/* Kobus. */
#define REPORT_PER_POINT

int get_independent_GMM_3
(
    int           num_clusters,
    const Matrix* feature_mp,
    const Int_vector* held_out_indicator_vp,
    const Vector* initial_a_vp,
    const Matrix* initial_means_mp,
    const Matrix* initial_var_mp,
    Vector**      a_vpp, 
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp,
    double*       log_likelihood_ptr,
    double*       held_out_log_likelihood_ptr,
    int*          num_iterations_ptr
)
{
    int           num_points          = feature_mp->num_rows;
    int           num_features        = feature_mp->num_cols;
    int           cluster;
    int           feature;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double        var_offset = 0.0;
#else
    double        var_offset = fs_var_offset; 
#endif 
    int           it, i;
    Vector*       I_vp          = NULL;
    Vector*       a_vp        = NULL;
    Vector*       p_sum_vp    = NULL;
    Matrix*       var_mp        = NULL;
    Matrix*       new_var_mp        = NULL;
    Matrix*       new_u_mp        = NULL;
    Vector*       x_vp        = NULL;
    Vector*       x2_vp        = NULL;
    double        log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        held_out_log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_held_out_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        diff, held_out_diff;
    Matrix*       u_mp    = NULL;
    Vector*       u_vp    = NULL;
    Vector*       var_vp  = NULL;
    int           result       = NO_ERROR;
#ifdef DONT_USE_GAUSS_ROUTINE
    Vector*       log_sqrt_det_vp  = NULL;
#endif 
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    Matrix* I_mp          = NULL;
#endif 
    Vector*       p_square_sum_vp = NULL;
#ifdef REPORT_PER_POINT
    int num_training_points = 0;
    int num_held_out_points = 0;
#endif

    dbp("This is the version of get_independent_GMM_3() in lib/r2.\n"); 

    if(num_clusters <= 0)
    {
        set_error("num_clusters must be at least 1");
        return ERROR;
    }

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    ERE(get_cluster_random_matrix(&I_mp, num_points, num_clusters));
#endif 

    if (P_mpp != NULL)
    {
        ERE(get_target_matrix(P_mpp, num_points, num_clusters));
    }

    if (held_out_indicator_vp != NULL)
    {
        if (held_out_indicator_vp->length != num_points)
        {
            result = ERROR;
        }
    }
            
    if (    (get_target_vector(&I_vp, num_clusters) == ERROR) 
         || (get_target_vector(&a_vp, num_clusters) == ERROR) 
#ifdef DONT_USE_GAUSS_ROUTINE
         || (get_target_vector(&log_sqrt_det_vp, num_clusters) == ERROR) 
#endif 
       )
    {
        result = ERROR;
    }

    dbi(num_clusters); 

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        log_likelihood = 0.0;

        held_out_log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_square_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_u_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_var_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
                
        if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
        {
            if (initial_means_mp != NULL)
            {   
                ERE(get_target_matrix(&u_mp, num_clusters, num_features));
                for (i = 0; i < num_clusters; i++)
                {
                    ERE(copy_matrix_row(u_mp, i, initial_means_mp, i));
                }
            }
            else
            {
                 set_bug("Initial cluster means matrix is NULL.\n");
                 return ERROR;
                 
                /*
                warn_pso("Initial cluster means matrix is NULL: Using random means.\n");
                ERE(get_random_matrix(&u_mp, num_clusters, num_features));
                */
            }

            if (initial_var_mp != NULL)
            {
                ERE(get_target_matrix(&var_mp, num_clusters, num_features));
                for (i = 0; i < num_clusters; i++)
                {
                    ERE(copy_matrix_row(var_mp, i, initial_var_mp, i));
                }
            }
            else
            {
                set_bug("Initial cluster variances matrix is NULL.\n");
                return ERROR;
                
                /*
                warn_pso("Initial cluster variances matrix is NULL: Using random variances.\n");
                ERE(get_random_matrix(&var_mp, num_clusters, num_features));
                */
            }
            for (i = 0; i < num_clusters; i++)
            {
                ERE(ow_add_scalar_to_matrix_row(var_mp, var_offset, i));
            }

            if (initial_a_vp != NULL)
            {
                ERE(get_target_vector(&a_vp, num_clusters));
                for (i = 0; i < num_clusters; i++)
                {
                    a_vp->elements[i] = initial_a_vp->elements[i];
                }
                ERE(ow_normalize_vector(a_vp, NORMALIZE_BY_SUM));
            }
            else
            {
                set_bug("Initial cluster priors vector is NULL.\n");
                return ERROR;
                
                /*
                warn_pso("Initial cluster priors vector is NULL: Using random priors.\n");
                for (i = 0; i < num_clusters; i++)
                {
                    a_vp->elements[i] = kjb_rand();
                }
                ERE(ow_add_scalar_to_vector(a_vp, 0.2 * kjb_rand() / num_clusters)); 
                ERE(ow_normalize_vector(a_vp, NORMALIZE_BY_SUM));
                */
            }

#ifdef DONT_USE_GAUSS_ROUTINE
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double temp = 0.0;

                if (result == ERROR) { NOTE_ERROR(); break; }

                for (feature = 0; feature < num_features; feature++)
                {
                    double var = var_mp->elements[ cluster ][ feature ]; 

                    temp += SAFE_LOG(var);
                }

                log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
            }
            verify_vector(log_sqrt_det_vp, NULL);
#endif 
        }
        
        for (i = 0; i<num_points; i++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_matrix_row(&x_vp, feature_mp, i);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors != TRUE) )
            {
                /* E Step init */

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
                result = get_matrix_row(&I_vp, I_mp, i);
#else
                /* ??? this has already been done at the beginning */
                result = get_target_vector(&I_vp, num_clusters); 
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double r = kjb_rand();
                    double p = pow(r, 5.0); /* Perhaps should be an option. */

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    I_vp->elements[ cluster ] = p;

                    if (kjb_rand() < 1.0 / num_clusters) 
                    {
                        I_vp->elements[ cluster ] += 0.5; 
                    }
                }

                result = ow_scale_vector_by_sum(I_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_to_vector(I_vp, 0.2 * kjb_rand() / num_clusters); 
                if (result == ERROR) { NOTE_ERROR(); break; }
#endif 

                result = ow_scale_vector_by_sum(I_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            
            if ( (it > 0) || (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
            {
                /* E Step */

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double log_a = SAFE_LOG(a_vp->elements[ cluster ]);
#ifdef DONT_USE_GAUSS_ROUTINE
                    double* x_ptr = x_vp->elements;
                    double* u_ptr = u_mp->elements[ cluster ];
                    double* v_ptr = var_mp->elements[ cluster ];
                    double temp, dev;
                    double d = 0.0;
#else 

                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = get_matrix_row(&u_vp, u_mp, cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&var_vp, var_mp, cluster);
#endif 
                    if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef DONT_USE_GAUSS_ROUTINE
                    for (feature = 0; feature < num_features; feature++)
                    {
                        dev = *u_ptr - *x_ptr;

                        ASSERT_IS_NUMBER_DBL(dev); 
                        ASSERT_IS_FINITE_DBL(dev); 

                        temp = (dev / (*v_ptr)) * dev;

                        d += temp; 

                        ASSERT_IS_NUMBER_DBL(d); 
                        ASSERT_IS_FINITE_DBL(d); 

                        u_ptr++;
                        v_ptr++;
                        x_ptr++;
                    }

                    ASSERT_IS_NUMBER_DBL(d); 
                    ASSERT_IS_FINITE_DBL(d); 

                    I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ]; 
#else
                    result = get_log_gaussian_density(x_vp, u_vp, var_vp, 
                                                      &(I_vp->elements[ cluster ]));
                    if (result == ERROR) { NOTE_ERROR(); break; }
                    I_vp->elements[ cluster ] += log_a; 
#endif 

#ifndef DONT_USE_GAUSS_ROUTINE
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS_LL
                    I_vp->elements[ cluster ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
#endif 
#endif 
                }

                if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
                {
                    held_out_log_likelihood += ow_exp_scale_by_sum_log_vector(I_vp);
#ifdef REPORT_PER_POINT
                    if (it == 1) num_held_out_points++; 
#endif 
                }
                else
                {
                    log_likelihood += ow_exp_scale_by_sum_log_vector(I_vp);
#ifdef REPORT_PER_POINT
                    if (it == 1) num_training_points++; 
#endif 
                }
            }

            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_vp), 1.0, 0.00001);

            if (result == ERROR) { NOTE_ERROR(); break; }

            if (P_mpp != NULL)
            {
                result = put_matrix_row(*P_mpp, I_vp, i);
            }

            /* M-Step */
            
            if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
            {
                /* Do not do the M-step for held out points */
            }
            else
            {
                result = multiply_vectors(&x2_vp, x_vp, x_vp);
                if (result == ERROR) { NOTE_ERROR(); break; } 

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double  p = I_vp->elements[ cluster ];

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_add_scalar_times_vector_to_matrix_row(new_u_mp, 
                                                                      x_vp, p,
                                                                      cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; } 
    
                    result = ow_add_scalar_times_vector_to_matrix_row(new_var_mp, 
                                                                      x2_vp, p,
                                                                      cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    p_sum_vp->elements[ cluster ] += p; 

                    p_square_sum_vp->elements[ cluster ] += (p * p);
                }
            }
        }

        /* M-Step cleanup */

        if (divide_matrix_by_col_vector(&u_mp, new_u_mp, p_sum_vp) == ERROR)
        {
            db_rv(p_sum_vp); 
            result = ERROR;
            break; 
        }

        result = get_initialized_matrix(&var_mp, num_clusters, num_features, 
                                        DBL_NOT_SET);
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double s = p_sum_vp->elements[ cluster ];

            for (feature = 0; feature < num_features; feature++)
            {
                double u = u_mp->elements[ cluster ][ feature ];
                double u2 = s * u * u;
                double var = new_var_mp->elements[ cluster ][ feature ] - u2;

                if (var  < 0.0) 
                {
                    /* 
                    // This does happen! The calculation of variance
                    // this way is not numerically stable. 
                    */
                    var = 0.0;
                }

                var_mp->elements[ cluster ][ feature ] = var;
            }

            result = ow_divide_matrix_row_by_scalar(var_mp, s, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (fs_use_unbiased_var_estimate_in_M_step == TRUE)
            {
                double p_square_sum      = p_square_sum_vp->elements[ cluster ];
                double norm_p_square_sum = p_square_sum / (s * s);
                double norm_factor       = 1.0 / (1.0 - norm_p_square_sum);

                result = ow_multiply_matrix_row_by_scalar(var_mp, norm_factor, cluster);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            
            verify_matrix(var_mp, NULL); 

            /* Moved to later due to the added options of tying variances:
            result = ow_add_scalar_to_matrix_row(var_mp, var_offset, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }
            */
        }

        if (fs_tie_var == TRUE)
        {
            double tie_var = 0.0;
            
            for (feature = 0; feature < num_features; feature++)
            {
                tie_var = 0.0;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    tie_var = tie_var + var_mp->elements[ cluster ][ feature ];
                }

                tie_var = tie_var/num_clusters;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_var;
                }
            }
        }
        else if (fs_tie_feature_var == TRUE)
        {
            double tie_feature_var = 0.0;

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    tie_feature_var = tie_feature_var + var_mp->elements[ cluster ][ feature ];
                }
            }

            tie_feature_var = tie_feature_var/(num_clusters * num_features);

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_feature_var;
                }
            }
        }
            
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            result = ow_add_scalar_to_matrix_row(var_mp, var_offset, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }
        
#ifdef DONT_USE_GAUSS_ROUTINE
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double temp = 0.0;

            if (result == ERROR) { NOTE_ERROR(); break; }

            for (feature = 0; feature < num_features; feature++)
            {
                double var = var_mp->elements[ cluster ][ feature ]; 

                temp += SAFE_LOG(var);
            }

            log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
        }
        verify_vector(log_sqrt_det_vp, NULL);
#endif 

        result = scale_vector_by_sum(&a_vp, p_sum_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; } 

        if (it > 0) 
        {
            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood +  prev_log_likelihood));

#ifdef REPORT_PER_POINT
            verbose_pso(3, "%-3d: Log likelihood per point is %12e  |  %10e ", 
                        it + 1, log_likelihood / num_training_points, diff / num_training_points); 
#else 
            verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e ", 
                        it + 1, log_likelihood, diff); 
#endif 


            held_out_diff = held_out_log_likelihood - prev_held_out_log_likelihood;

            held_out_diff *= 2.0;
            held_out_diff /= (ABS_OF(held_out_log_likelihood + prev_held_out_log_likelihood));
            
#ifdef REPORT_PER_POINT
            verbose_pso(3, "| Held out per point is %12e  |  %10e\n",
                        held_out_log_likelihood / num_held_out_points, 
                        held_out_diff / num_held_out_points); 
#else 
            verbose_pso(3, "| Held out is %12e  |  %10e\n",
                        held_out_log_likelihood, held_out_diff); 
#endif 

            if (fs_EM_stop_criterion_training_LL == TRUE)
            {
                if (ABS_OF(diff) < fs_iteration_tolerance) break; 
            }
            else if (fs_EM_stop_criterion_held_out_LL == TRUE)
            {
                if (ABS_OF(held_out_diff) < fs_iteration_tolerance) break;
            }

            prev_log_likelihood = log_likelihood;

            prev_held_out_log_likelihood = held_out_log_likelihood;
        }
    }

    if (result != ERROR)
    {
        if (log_likelihood_ptr != NULL)
        {
            *log_likelihood_ptr = log_likelihood;
        }

        if (held_out_log_likelihood_ptr != NULL)
        {
            *held_out_log_likelihood_ptr = held_out_log_likelihood;
        }

        if (num_iterations_ptr != NULL)
        {
            *num_iterations_ptr = it;
        }

        if (a_vpp != NULL) 
        {
            result = copy_vector(a_vpp, a_vp);
        }
    }
    
    if ((result != ERROR) && (var_mpp != NULL))
    {
        result = copy_matrix(var_mpp, var_mp);
    }

    if ((result != ERROR) && (means_mpp != NULL))
    {
        result = copy_matrix(means_mpp, u_mp);
    }

    free_vector(I_vp); 
    free_vector(p_sum_vp);
    free_vector(a_vp);
    free_matrix(new_u_mp); 
    free_matrix(u_mp); 
    free_matrix(new_var_mp); 
    free_matrix(var_mp); 
#ifdef DONT_USE_GAUSS_ROUTINE
    free_vector(log_sqrt_det_vp);
#endif 

    free_vector(x2_vp); 
    free_vector(x_vp); 
    free_vector(var_vp); 
    free_vector(u_vp); 

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
     free_matrix(I_mp); 
#endif 
     free_vector(p_square_sum_vp);

    if (result == ERROR) 
    {
        return ERROR; 
    }
    else 
    {
        return num_clusters;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                 get_independent_GMM_2_with_missing_data
 *
 * Clone of get_independent_GMM_2, but respects missing data.  Marginally higher
 * memory footprint and running time relative to the get_independent_GMM_2.
 *
 * Index: clustering, EM, GMM
 *
 * -----------------------------------------------------------------------------
*/
int get_independent_GMM_2_with_missing_data
(
    int           num_clusters,
    const Matrix* feature_mp,
    Vector**      a_vpp, 
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp,
    double*       log_likelihood_ptr 
)
{
    int           num_points          = feature_mp->num_rows;
    int           num_features        = feature_mp->num_cols;
    int           cluster;
    int           feature;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double        var_offset = 0.0;
#else
    double        var_offset = fs_var_offset; 
#endif 
    int           it, i;
    Vector*       I_vp        = NULL;
    Vector*       a_vp        = NULL;
    Vector*       p_sum_vp    = NULL;
    Matrix*       p_sum_mp    = NULL;
    Matrix*       var_mp        = NULL;
    Matrix*       new_var_mp        = NULL;
    Matrix*       new_u_mp        = NULL;
    Matrix*       tmp_mp        = NULL;
    Vector*       x_vp        = NULL;
    Vector*       x2_vp        = NULL;
    double        log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        diff;
    Matrix*       u_mp    = NULL;
    Vector*       u_vp    = NULL;
    Vector*       var_vp  = NULL;
    int           result       = NO_ERROR;
    Vector*       log_sqrt_det_vp  = NULL;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    Matrix* I_mp = NULL; 
#endif 
#ifndef REGRESS_NO_HACK_FOR_ZERO_MEMBERSHIP
    int first_null_cluster_spotted = TRUE; 
#endif 

    if(num_clusters <= 0)
    {
        set_error("num_clusters must be at least 1");
        return ERROR;
    }

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    ERE(get_cluster_random_matrix(&I_mp, num_points, num_clusters)); 
#endif 

    if (P_mpp != NULL)
    {
        ERE(get_target_matrix(P_mpp, num_points, num_clusters));
    }

    if (    (get_target_vector(&I_vp, num_clusters) == ERROR) 
         || (get_target_vector(&a_vp, num_clusters)             == ERROR) 
         || (get_target_vector(&log_sqrt_det_vp, num_clusters)      == ERROR) 
       )
    {
        result = ERROR;
    }

    dbi(num_clusters); 

    EGC(get_zero_matrix(&var_mp, num_clusters, num_features));

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }
        result = get_zero_matrix(&p_sum_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_u_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_var_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
                
        for (i = 0; i<num_points; i++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_matrix_row(&x_vp, feature_mp, i);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if (it == 0)
            {
                /* E Step init */


                /*
                 * Simple initilaization: One cluster per point, but also make
                 * it so that every cluster gets a tiny bit of weight due to
                 * this point, so that we know that every cluster has some
                 * weight. 
                */
                result = get_random_vector(&I_vp, num_clusters);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_divide_vector_by_scalar(I_vp, 20.0 * (double)num_points);
                if (result == ERROR) { NOTE_ERROR(); break; }

                cluster = (int)(((double)num_clusters) * kjb_rand());
                if (cluster == num_clusters) cluster--; 
                I_vp->elements[ cluster ] = 1.0; 
                

                result = ow_scale_vector_by_sum(I_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_to_vector(I_vp, 0.2 * kjb_rand() / num_clusters); 
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_scale_vector_by_sum(I_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            else
            {
                /* E Step */

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double log_a = SAFE_LOG(a_vp->elements[ cluster ]);
                    double* x_ptr = x_vp->elements;
                    double* u_ptr = u_mp->elements[ cluster ];
                    double* v_ptr = var_mp->elements[ cluster ];
                    double temp, dev;
                    double d = 0.0;

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (feature = 0; feature < num_features; feature++)
                    {
                        if(IS_NOT_MISSING_DBL(*x_ptr))
                        {

                            dev = *u_ptr - *x_ptr;

                            ASSERT_IS_NUMBER_DBL(dev); 
                            ASSERT_IS_FINITE_DBL(dev); 

                            temp = (dev / (*v_ptr)) * dev;

                            d += temp; 

                            ASSERT_IS_NUMBER_DBL(d); 
                            ASSERT_IS_FINITE_DBL(d);
                        }

                        u_ptr++;
                        v_ptr++;
                        x_ptr++;
                    }

                    ASSERT_IS_NUMBER_DBL(d); 
                    ASSERT_IS_FINITE_DBL(d); 

                    I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ]; 
                }
                if (result == ERROR)  break;

                log_likelihood += ow_exp_scale_by_sum_log_vector(I_vp);
            }

            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_vp), 1.0, 0.00001);

            if (result == ERROR) { NOTE_ERROR(); break; }

            if (P_mpp != NULL)
            {
                result = put_matrix_row(*P_mpp, I_vp, i);
            }

            /* M-Step */

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double  p = I_vp->elements[ cluster ];
                double* x_ptr = x_vp->elements;
                double* u_ptr = new_u_mp->elements[ cluster ];
                double* v_ptr = new_var_mp->elements[ cluster ];
                double* p_sum_ptr = p_sum_mp->elements[ cluster ];
                double tmp;

                /* keep track of total weight, for computing cluster weights later */ 
                p_sum_vp->elements[ cluster ] += p; 

                for (feature = 0; feature < num_features; feature++)
                {
                    if(IS_NOT_MISSING_DBL(x_vp->elements[ feature ]))
                    {

                        /* keep track of contributed weight, for normalization.
                         *  Missing featurs are skipped, since they didn't contribute 
                         *  to the sums above.  */
                        *p_sum_ptr += p; 
                    
                        tmp = *x_ptr * p;
                        *u_ptr += tmp;
                        *v_ptr += *x_ptr * tmp;
                    }

                    x_ptr++;
                    u_ptr++;
                    v_ptr++;
                    p_sum_ptr++;
                }
            }
        }
        if (result == ERROR)  break;

        /* M-Step cleanup */

        /* COMPUTE MEANS */
        result = copy_matrix(&u_mp, new_u_mp);
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (cluster = 0; cluster < num_clusters; cluster++)
        for (feature = 0; feature < num_features; feature++)
        {
            double s = p_sum_mp->elements[ cluster ][ feature ];
            if (s <= 10.0 * DBL_MIN)
            {
                p_sum_mp->elements[ cluster ][ feature ] = 1;

                if(first_null_cluster_spotted)
                {
                    first_null_cluster_spotted = FALSE;
                    warn_pso("At least one cluster has no members.\n"); 
                }
            }
        }

        result = divide_matrices_ew(&u_mp, new_u_mp, p_sum_mp);
            if (result == ERROR) { NOTE_ERROR(); break; }

        /* COMPUTE VARIANCES */
        /*   var = (var - u .* u .* p_sum) ./ p_sum      */
        result = multiply_matrices_ew(&tmp_mp, u_mp, u_mp);
            if (result == ERROR) { NOTE_ERROR(); break; }
        result = ow_multiply_matrices_ew(tmp_mp, p_sum_mp);
            if (result == ERROR) { NOTE_ERROR(); break; }
        result = ow_subtract_matrices(new_var_mp, tmp_mp);
            if (result == ERROR) { NOTE_ERROR(); break; }
        result = ow_divide_matrices_ew(new_var_mp, p_sum_mp);
            if (result == ERROR) { NOTE_ERROR(); break; }
        /* 
        // Handle var < 0.
        // This does happen! The calculation of variance
        // this way is not numerically stable. 
        */
        result = ow_min_thresh_matrix(new_var_mp, 0.0);
            if (result == ERROR) { NOTE_ERROR(); break; }

        verify_matrix(new_var_mp, NULL); 
        
        /* HANDLE TIED VARIANCES */
        if (fs_tie_var == TRUE)
        {
            double tie_var = 0.0;
            
            for (feature = 0; feature < num_features; feature++)
            {
                tie_var = 0.0;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    tie_var += new_var_mp->elements[ cluster ][ feature ];
                }

                tie_var /= num_clusters;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    new_var_mp->elements[ cluster ][ feature ] = tie_var;
                }
            }
        }
        else if (fs_tie_feature_var == TRUE)
        {
            double tie_feature_var = 0.0;

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    tie_feature_var += new_var_mp->elements[ cluster ][ feature ];
                }
            }

            tie_feature_var /= (num_clusters * num_features);

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    new_var_mp->elements[ cluster ][ feature ] = tie_feature_var;
                }
            }
        }
            
        else if (fs_tie_cluster_var == TRUE)
        {
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double tie_cluster_var = 0.0;

                for (feature = 0; feature < num_features; feature++)
                {
                    tie_cluster_var += new_var_mp->elements[ cluster ][ feature ];
                }

                tie_cluster_var /= num_features;

                for (feature = 0; feature < num_features; feature++)
                {
                    new_var_mp->elements[ cluster ][ feature ] = tie_cluster_var;
                }
            }
        }
        
        /* ADD VARIANCE OFFSETS */
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            result = ow_add_scalar_to_matrix_row(new_var_mp, var_offset, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }
        if (result == ERROR)  break;

        /* Compute log(sqrt(det(Sigma))) (for likelihood evaluation) */
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double temp = 0.0;

            if (result == ERROR) { NOTE_ERROR(); break; }

            for (feature = 0; feature < num_features; feature++)
            {
                double var = new_var_mp->elements[ cluster ][ feature ]; 

                temp += SAFE_LOG(var);
            }

            log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
        }
        if (result == ERROR)  break;
        verify_vector(log_sqrt_det_vp, NULL);

        result = scale_vector_by_sum(&a_vp, p_sum_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; }

        SWAP_MATRICES(new_var_mp, var_mp);

        if (it > 0) 
        {
            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood +  prev_log_likelihood));

            verbose_pso(3, "%-3d:%-3d  Log likelihood is %12e  |  %10e | %10e \n", 
                        it + 1, fs_max_num_iterations, log_likelihood, diff, fs_iteration_tolerance); 

            if (ABS_OF(diff) < fs_iteration_tolerance) break; 

            prev_log_likelihood = log_likelihood; 
        }

    }

    if (result != ERROR)
    {
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
        result = copy_matrix(var_mpp, var_mp);
    }

    if ((result != ERROR) && (means_mpp != NULL))
    {
        result = copy_matrix(means_mpp, u_mp);
    }

cleanup:
    free_vector(I_vp); 
    free_vector(p_sum_vp);
    free_matrix(p_sum_mp);
    free_vector(a_vp);
    free_matrix(u_mp); 
    free_vector(var_vp); 
    free_vector(u_vp); 
    free_matrix(new_var_mp); 
    free_matrix(new_u_mp); 
    free_matrix(tmp_mp);
    free_matrix(var_mp); 

    free_vector(x_vp); 
    free_vector(x2_vp); 
    free_vector(log_sqrt_det_vp);

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
     free_matrix(I_mp); 
#endif 

    if (result == ERROR) return ERROR; else return num_clusters;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Multithreaded EM */
#ifdef KJB_HAVE_PTHREAD

int get_independent_GMM_3_mt
(
    int           num_threads,
    int           num_clusters,
    const Matrix* feature_mp,
    const Int_vector* held_out_indicator_vp,
    const Vector* initial_a_vp,
    const Matrix* initial_means_mp,
    const Matrix* initial_var_mp,
    Vector**      a_vpp, 
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp,
    double*       log_likelihood_ptr,
    double*       held_out_log_likelihood_ptr,
    int*          num_iterations_ptr
)
{
    int           num_points          = feature_mp->num_rows;
    int           num_features        = feature_mp->num_cols;
    int           cluster;
    int           feature;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double        var_offset = 0.0;
#else
    double        var_offset = fs_var_offset; 
#endif 
    int           it, i;
    Vector*       a_vp        = NULL;
    Vector*       p_sum_vp    = NULL;
    Matrix*       var_mp      = NULL;
    Matrix*       u_mp    = NULL;
    Matrix*       new_var_mp  = NULL;
    Matrix*       new_u_mp    = NULL;
    Vector_vector* I_vps      = NULL;
    Vector_vector* x_vps      = NULL;
    Vector_vector* x2_vps     = NULL;
    double        log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        held_out_log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double        prev_held_out_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double        diff, held_out_diff;
    Vector*       u_vp    = NULL;
    Vector*       var_vp  = NULL;
    int           result       = NO_ERROR;
#ifdef DONT_USE_GAUSS_ROUTINE
    Vector*       log_sqrt_det_vp  = NULL;
#endif 
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    Matrix*       I_mp = NULL; 
#endif 
    Vector*       p_square_sum_vp = NULL;

    /* Used for multithreaded */
    int thread, end_index, thread_r, block_size; 
    int start_index;
    kjb_pthread_attr_t attr;
    kjb_pthread_t* threads;
    struct thread_GMM_data* data;

    kjb_pthread_mutex_t mutexsum;

    if(num_clusters <= 0)
    {
        set_error("num_clusters must be at least 1");
        return ERROR;
    }
   
    warn_pso("Warning: multi-threaded GMM doesn't currently pass tests.\n");

    if (P_mpp != NULL)
    {
        ERE(get_target_matrix(P_mpp, num_points, num_clusters));
    }

    /* Create the threads */
    threads = N_TYPE_MALLOC(kjb_pthread_t, num_threads);
    data = N_TYPE_MALLOC(struct thread_GMM_data, num_threads);

    if (held_out_indicator_vp != NULL)
    {
        if (held_out_indicator_vp->length != num_points)
        {
            result = ERROR;
        }
    }
            
    if ( (get_target_vector_vector(&I_vps, num_threads)==ERROR) 
         || (get_target_vector_vector(&x_vps, num_threads)==ERROR) 
         || (get_target_vector_vector(&x2_vps, num_threads)==ERROR) 
         || (get_target_vector(&a_vp, num_clusters) == ERROR) 
#ifdef DONT_USE_GAUSS_ROUTINE
         || (get_target_vector(&log_sqrt_det_vp, num_clusters) == ERROR) 
#endif 
       )
    {
        result = ERROR;
    }

    dbi(num_clusters); 

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        start_index = 0;
        
        log_likelihood = 0.0;

        held_out_log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_square_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_u_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_var_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
                
        if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
        {
            if (initial_means_mp != NULL)
            {   
                ERE(get_target_matrix(&u_mp, num_clusters, num_features));
                for (i = 0; i < num_clusters; i++)
                {
                    ERE(copy_matrix_row(u_mp, i, initial_means_mp, i));
                }
            }
            else
            {
                 set_bug("Initial cluster means matrix is NULL.\n");
                 return ERROR;
                 
                /*
                warn_pso("Initial cluster means matrix is NULL: Using random means.\n");
                ERE(get_random_matrix(&u_mp, num_clusters, num_features));
                */
            }

            if (initial_var_mp != NULL)
            {
                ERE(get_target_matrix(&var_mp, num_clusters, num_features));
                for (i = 0; i < num_clusters; i++)
                {
                    ERE(copy_matrix_row(var_mp, i, initial_var_mp, i));
                }
            }
            else
            {
                set_bug("Initial cluster variances matrix is NULL.\n");
                return ERROR;
                
                /*
                warn_pso("Initial cluster variances matrix is NULL: Using random variances.\n");
                ERE(get_random_matrix(&var_mp, num_clusters, num_features));
                */
            }
            for (i = 0; i < num_clusters; i++)
            {
                ERE(ow_add_scalar_to_matrix_row(var_mp, var_offset, i));
            }

            if (initial_a_vp != NULL)
            {
                ERE(get_target_vector(&a_vp, num_clusters));
                for (i = 0; i < num_clusters; i++)
                {
                    a_vp->elements[i] = initial_a_vp->elements[i];
                }
                ERE(ow_normalize_vector(a_vp, NORMALIZE_BY_SUM));
            }
            else
            {
                set_bug("Initial cluster priors vector is NULL.\n");
                return ERROR;
                
                /*
                warn_pso("Initial cluster priors vector is NULL: Using random priors.\n");
                for (i = 0; i < num_clusters; i++)
                {
                    a_vp->elements[i] = kjb_rand();
                }
                ERE(ow_add_scalar_to_vector(a_vp, 0.2 * kjb_rand() / num_clusters)); 
                ERE(ow_normalize_vector(a_vp, NORMALIZE_BY_SUM));
                */
            }

#ifdef DONT_USE_GAUSS_ROUTINE
            kjb_pthread_mutex_lock (&mutexsum);
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double temp = 0.0;

                if (result == ERROR) { NOTE_ERROR(); break; }

                for (feature = 0; feature < num_features; feature++)
                {
                    double var = var_mp->elements[ cluster ][ feature ]; 

                    temp += SAFE_LOG(var);
                }

                log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
            }
            verify_vector(log_sqrt_det_vp, NULL);
            kjb_pthread_mutex_unlock (&mutexsum);
#endif 
        }
       
        /* Create threads based on the number of points and the number of CPUs 
         * We need to split the data into chunks, do Expectation step on each 
         * of the chunks using different thread. After all the threads are 
         * finished, We will do Maximization step on all the data 
         */
       
        /* Initialize the mutex for the shared data */
        kjb_pthread_mutex_init(&mutexsum, NULL);
        kjb_pthread_attr_init(&attr);
        kjb_pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        block_size = num_points/num_threads;

        for(thread = 0; thread < num_threads; thread++)
        {
            if(thread == num_threads-1)
            {
                end_index = num_points-1;
            }
            else
            {
                end_index = start_index+block_size-1; 
            }
           
            /*verbose_pso(3, "Thread %2d: start index: %3d, end index: %3d \n", thread, 
                    start_index, end_index);*/
            data[thread].num_clusters = num_clusters;
            data[thread].num_iteration = it;
            data[thread].start_index = start_index;
            data[thread].end_index = end_index;
            data[thread].feature_mp = feature_mp;
            data[thread].held_out_indicator_vp = held_out_indicator_vp;
            data[thread].a_vp = a_vp;
            data[thread].x_vpp = &(x_vps->elements[thread]);
            data[thread].x2_vpp = &(x2_vps->elements[thread]);
            data[thread].I_vpp = &(I_vps->elements[thread]);
            data[thread].u_mp = u_mp;
            data[thread].var_mp = var_mp;
            data[thread].new_u_mp = new_u_mp;
            data[thread].new_var_mp = new_var_mp;

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
            data[thread].I_mp = I_mp;
#else 

            data[thread].I_mp = NULL;
#endif
            data[thread].P_mpp = P_mpp;
            data[thread].log_likelihood_ptr= &log_likelihood;
            data[thread].held_out_log_likelihood_ptr = &held_out_log_likelihood;
            data[thread].p_sum_vpp = &p_sum_vp;
            data[thread].p_square_sum_vpp = &p_square_sum_vp;
            data[thread].log_sqrt_det_vpp = &log_sqrt_det_vp;
            data[thread].mutexsum_p = &mutexsum;
            
            /*verbose_pso(3, "create thread %2d\n", thread);*/
            thread_r = kjb_pthread_create(&threads[thread], &attr, create_independent_GMM_thread, (void*) &data[thread]);
           
            if(thread_r)
            {
                result = ERROR;
                NOTE_ERROR();
                break;
            }
            start_index = end_index+1;
        }
        
        kjb_pthread_attr_destroy(&attr);
        /* Wait until all the threads finish, then do the cleaning up step */
        
        for(thread = 0; thread < num_threads; thread++)
        {
            /*verbose_pso(3, "finish thread %2d\n", thread);*/
            kjb_pthread_join(threads[thread], NULL);
            /*free_vector(I_vps->elements[thread]);
            free_vector(x_vps->elements[thread]);
            free_vector(x2_vps->elements[thread]);*/
        }
        /*clean the memory */ 

        /* M-Step cleanup */

        if (divide_matrix_by_col_vector(&u_mp, new_u_mp, p_sum_vp) == ERROR)
        {
            db_rv(p_sum_vp); 
            result = ERROR;
            break; 
        }

        result = get_initialized_matrix(&var_mp, num_clusters, num_features, 
                                        DBL_NOT_SET);
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double s = p_sum_vp->elements[ cluster ];

            for (feature = 0; feature < num_features; feature++)
            {
                double u = u_mp->elements[ cluster ][ feature ];
                double u2 = s * u * u;
                double var = new_var_mp->elements[ cluster ][ feature ] - u2;

                if (var  < 0.0) 
                {
                    /* 
                    // This does happen! The calculation of variance
                    // this way is not numerically stable. 
                    */
                    var = 0.0;
                }

                var_mp->elements[ cluster ][ feature ] = var;
                }

            result = ow_divide_matrix_row_by_scalar(var_mp, s, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (fs_use_unbiased_var_estimate_in_M_step == TRUE)
            {
                double p_square_sum      = p_square_sum_vp->elements[ cluster ];
                double norm_p_square_sum = p_square_sum / (s * s);
                double norm_factor       = 1.0 / (1.0 - norm_p_square_sum);

                result = ow_multiply_matrix_row_by_scalar(var_mp, norm_factor, cluster);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            
            verify_matrix(var_mp, NULL); 

            /* Moved to later due to the added options of tying variances:
            result = ow_add_scalar_to_matrix_row(var_mp, var_offset, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }
            */
        }

        if (fs_tie_var == TRUE)
        {
            double tie_var = 0.0;
            
            for (feature = 0; feature < num_features; feature++)
            {
                tie_var = 0.0;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    tie_var = tie_var + var_mp->elements[ cluster ][ feature ];
                }

                tie_var = tie_var/num_clusters;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_var;
                }
            }
        }
        else if (fs_tie_feature_var == TRUE)
        {
            double tie_feature_var = 0.0;

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    tie_feature_var = tie_feature_var + var_mp->elements[ cluster ][ feature ];
                }
            }

            tie_feature_var = tie_feature_var/(num_clusters * num_features);

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (feature = 0; feature < num_features; feature++)
                {
                    var_mp->elements[ cluster ][ feature ] = tie_feature_var;
                }
            }
        }
            
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            result = ow_add_scalar_to_matrix_row(var_mp, var_offset, cluster);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }
        
#ifdef DONT_USE_GAUSS_ROUTINE
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double temp = 0.0;

            if (result == ERROR) { NOTE_ERROR(); break; }

            for (feature = 0; feature < num_features; feature++)
            {
                double var = var_mp->elements[ cluster ][ feature ]; 

                temp += SAFE_LOG(var);
            }

            log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
        }
        verify_vector(log_sqrt_det_vp, NULL);
#endif 

        result = scale_vector_by_sum(&a_vp, p_sum_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; } 

        if (it > 0) 
        {
            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood +  prev_log_likelihood));

            /*verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e ",  
                        it + 1, log_likelihood, diff); */


            held_out_diff = held_out_log_likelihood - prev_held_out_log_likelihood;

            held_out_diff *= 2.0;
            held_out_diff /= (ABS_OF(held_out_log_likelihood + prev_held_out_log_likelihood));
            
            /*verbose_pso(3, "| Held out is %12e  |  %10e\n",
                        held_out_log_likelihood, held_out_diff);  */

            if (fs_EM_stop_criterion_training_LL == TRUE)
            {
                if (ABS_OF(diff) < fs_iteration_tolerance) break; 
            }
            else if (fs_EM_stop_criterion_held_out_LL == TRUE)
            {
                if (ABS_OF(held_out_diff) < fs_iteration_tolerance) break;
            }

            prev_log_likelihood = log_likelihood;

            prev_held_out_log_likelihood = held_out_log_likelihood;
        }
    }

    if (result != ERROR)
    {
        if (log_likelihood_ptr != NULL)
        {
            *log_likelihood_ptr = log_likelihood;
        }

        if (held_out_log_likelihood_ptr != NULL)
        {
            *held_out_log_likelihood_ptr = held_out_log_likelihood;
        }

        if (num_iterations_ptr != NULL)
        {
            *num_iterations_ptr = it;
        }

        if (a_vpp != NULL) 
        {
            result = copy_vector(a_vpp, a_vp);
        }
    }
    
    if ((result != ERROR) && (var_mpp != NULL))
    {
        result = copy_matrix(var_mpp, var_mp);
    }

    if ((result != ERROR) && (means_mpp != NULL))
    {
        result = copy_matrix(means_mpp, u_mp);
    }

    kjb_free(threads);
    kjb_free(data);
    free_vector(a_vp);
    free_vector(p_sum_vp);
    free_matrix(var_mp); 
    free_matrix(u_mp); 
    free_matrix(new_var_mp); 
    free_matrix(new_u_mp); 
    free_vector_vector(I_vps); 
    free_vector_vector(x_vps); 
    free_vector_vector(x2_vps); 
    free_vector(u_vp); 
    free_vector(var_vp); 

#ifdef DONT_USE_GAUSS_ROUTINE
    free_vector(log_sqrt_det_vp);
#endif 

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
     free_matrix(I_mp); 
#endif 
     free_vector(p_square_sum_vp);

    if (result == ERROR) 
    {
        return ERROR; 
    }
    else 
    {
        return num_clusters;
    }
}

/* Running EM on a subset of the data on a thread */
void *create_independent_GMM_thread (void *arg)
{
    int i, cluster, feature;
    struct thread_GMM_data * data = (struct thread_GMM_data *)arg;
    int           num_clusters = data->num_clusters;
    int           num_iteration = data->num_iteration;
    int           start_index = data->start_index;
    int           end_index = data->end_index;
    const Matrix* feature_mp = data->feature_mp;
    const Int_vector* held_out_indicator_vp = data->held_out_indicator_vp;
    Vector*       a_vp = data->a_vp;
    Matrix*       u_mp = data->u_mp;
    Matrix*       var_mp = data->var_mp;
    Matrix*       new_u_mp = data->new_u_mp;
    Matrix*       new_var_mp = data->new_var_mp;
    Matrix*       I_mp = data->I_mp;
    Matrix**      P_mpp = data->P_mpp;
    double*       log_likelihood_ptr = data->log_likelihood_ptr;
    double*       held_out_log_likelihood_ptr = data->held_out_log_likelihood_ptr;
    Vector*       p_sum_vp = *(data->p_sum_vpp);
    Vector*       p_square_sum_vp = *(data->p_square_sum_vpp);
    Vector**      x_vpp = data->x_vpp;
    Vector**      x2_vpp = data->x2_vpp;
    Vector**      I_vpp = data->I_vpp;
    int num_features = feature_mp->num_cols;
    int result = NO_ERROR;
    int it = num_iteration;
    double log_likelihood = 0;
    double held_out_log_likelihood = 0;
    Vector* I_vp = NULL;

#ifdef DONT_USE_GAUSS_ROUTINE
    Vector*       log_sqrt_det_vp  = *(data->log_sqrt_det_vpp);
#endif 
   
    
    if(get_target_vector(I_vpp, num_clusters)==ERROR)
    { 
        NOTE_ERROR();
    }

    for (i = start_index; i < end_index; i++)
    {
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_matrix_row(x_vpp, feature_mp, i);
        if (result == ERROR) { NOTE_ERROR(); break; } 

        if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors != TRUE) )
        {
            /* E Step init */

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
            result = get_matrix_row(I_vpp, I_mp, i);
#else
            if (result == ERROR) { NOTE_ERROR(); break; }
            
            I_vp  = *I_vpp;

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double r = kjb_rand();
                double p = pow(r, 5.0); /* Perhaps should be an option. */

                if (result == ERROR) { NOTE_ERROR(); break; }

                I_vp->elements[ cluster ] = p;

                if (kjb_rand() < 1.0 / num_clusters) 
                {
                    I_vp->elements[ cluster ] += 0.5; 
                }
            }

            result = ow_scale_vector_by_sum(I_vp);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = ow_add_scalar_to_vector(I_vp, 0.2 * kjb_rand() / num_clusters); 
            if (result == ERROR) { NOTE_ERROR(); break; }
#endif 

            result = ow_scale_vector_by_sum(I_vp);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }
        
        if ( (it > 0) || (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
        {
            Vector *I_vp  = *I_vpp;
            /* E Step */
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double log_a = SAFE_LOG(a_vp->elements[ cluster ]);
#ifdef DONT_USE_GAUSS_ROUTINE
                double* x_ptr = (*x_vpp)->elements;
                double* u_ptr = u_mp->elements[ cluster ];
                double* v_ptr = var_mp->elements[ cluster ];
                double temp, dev;
                double d = 0.0;
#else 

                if (result == ERROR) { NOTE_ERROR(); break; } 

                result = get_matrix_row(&u_vp, u_mp, cluster);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = get_matrix_row(&var_vp, var_mp, cluster);
#endif 
                if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef DONT_USE_GAUSS_ROUTINE
                for (feature = 0; feature < num_features; feature++)
                {
                    dev = *u_ptr - *x_ptr;

                    ASSERT_IS_NUMBER_DBL(dev); 
                    ASSERT_IS_FINITE_DBL(dev); 

                    temp = (dev / (*v_ptr)) * dev;

                    d += temp; 

                    ASSERT_IS_NUMBER_DBL(d); 
                    ASSERT_IS_FINITE_DBL(d); 

                    u_ptr++;
                    v_ptr++;
                    x_ptr++;
                }

                ASSERT_IS_NUMBER_DBL(d); 
                ASSERT_IS_FINITE_DBL(d); 

                I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ]; 
#else
                result = get_log_gaussian_density(*x_vpp, u_vp, var_vp, 
                                                  &(I_vp->elements[ cluster ]));
                if (result == ERROR) { NOTE_ERROR(); break; }
                I_vp->elements[ cluster ] += log_a; 
#endif 

#ifndef DONT_USE_GAUSS_ROUTINE
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS_LL
                I_vp->elements[ cluster ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
#endif 
#endif 
            }

            if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
            {
                held_out_log_likelihood += ow_exp_scale_by_sum_log_vector(I_vp);
            }
            else
            {
                log_likelihood += ow_exp_scale_by_sum_log_vector(I_vp);
            }
        }

        ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(*I_vpp), 1.0, 0.00001);

        if (result == ERROR) { NOTE_ERROR(); break; }

        /*
        Lock a mutex prior to updating the value in the shared
        structure, and unlock it upon updating.
        */
        kjb_pthread_mutex_lock ((data->mutexsum_p));

        if (P_mpp != NULL)
        {
            result = put_matrix_row(*P_mpp, *I_vpp, i);
        }

        *log_likelihood_ptr += log_likelihood;
        *held_out_log_likelihood_ptr += held_out_log_likelihood;

        kjb_pthread_mutex_unlock ((data->mutexsum_p));

        /* M-Step */
        
        if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
        {
            /* Do not do the M-step for held out points */
        }
        else
        {
            Vector *I_vp  = *I_vpp;
            result = multiply_vectors(x2_vpp, *x_vpp, *x_vpp);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double  p = I_vp->elements[ cluster ];

                /*
                Lock a mutex prior to updating the value in the shared
                structure, and unlock it upon updating.
                */
                kjb_pthread_mutex_lock (data->mutexsum_p);
               
                if (result == ERROR) {
                    NOTE_ERROR();
                    kjb_pthread_mutex_unlock (data->mutexsum_p);
                    break;
                }

                result = ow_add_scalar_times_vector_to_matrix_row(new_u_mp, 
                                                                  *x_vpp, p,
                                                                  cluster);
                if (result == ERROR) 
                { 
                    NOTE_ERROR();
                    kjb_pthread_mutex_unlock (data->mutexsum_p);
                    break;
                } 

                result = ow_add_scalar_times_vector_to_matrix_row(new_var_mp, 
                                                                  *x2_vpp, p,
                                                                  cluster);
                if (result == ERROR) 
                { 
                    NOTE_ERROR();
                    kjb_pthread_mutex_unlock (data->mutexsum_p);
                    break;
                } 

                p_sum_vp->elements[ cluster ] += p; 

                p_square_sum_vp->elements[ cluster ] += (p * p);
                kjb_pthread_mutex_unlock (data->mutexsum_p);
            }
        }
    }

   /* pthread_mutex_lock (&mutexsum);*/
    /*pthread_mutex_lock (&mutexsum);*/
    /*free(I_vp);    
    free(x_vp);
    free(x2_vp);*/
    kjb_pthread_exit(NULL);

    /* NOTREACHED */
    return NULL;
}
#endif /* KJB_HAVE_PTHREAD*/


/** 
* Legacy version of Bayes information criterion.
* 
* This implemnentation has several issues:
*  
*  1. encoding costs assume independent GMM;  using these for 
*     full GMM will give incorrect values.
*  2. Results differs from true BIC by a constant factor.  Not 
*     strictly important for model selection, but some statisticians
*     use rules of thumb when reading BIC, which won't apply to the
*     values reported here (See wikipedia article on BIC)
*  3. Number of observations should be used, but instead number of
*     observed dimensions is used.
*/
double old_bic(int num_clusters, int num_features, int num_observations, int independent, int tie_var, int tie_feature_var)
{
    double encoding_cost;

    if (tie_var == TRUE)
    {
        encoding_cost = 0.5 * ((num_clusters * (num_features +  1.0) - 1.0) + num_features);
    }
    else if (fs_tie_feature_var == TRUE)
    {
        encoding_cost = 0.5 * ((num_clusters * (num_features +  1.0) - 1.0) + 1.0);
    }
    else
    {
        encoding_cost = 0.5 * (num_clusters * ((2.0 * num_features) +  1.0) - 1.0);
    }

    encoding_cost *= log( (double) num_observations*num_features ); 
    return encoding_cost;
}

/** 
* Update version of Bayes information criterion.
*
* 1. Full-covariance GMMs are handled correctly.
* 2. Number of observations are counted correctly.
*
* tie_var and tie_feature_var are ignored if independent=false.
*/
double bic(int num_clusters, int num_features, int num_observations, int independent, int tie_var, int tie_feature_var)
{
    int num_params;
    double encoding_cost;

    if(independent)
    {
        if (tie_var == TRUE)
        {
            num_params = ((num_clusters * (num_features +  1.0) - 1.0) + num_features);
        }
        else if (fs_tie_feature_var == TRUE)
        {
            num_params = ((num_clusters * (num_features +  1.0) - 1.0) + 1.0);
        }
        else
        {
            num_params = (num_clusters * ((2.0 * num_features) +  1.0) - 1.0);
        }
    }
    else
    {
        num_params = (num_clusters * (num_features * (num_features + 1))/2 + /* number of covariance DoF */
                        num_clusters * num_features +                               /* number of mean DoF */
                        num_clusters - 1);                                          /* number of weight DoF */

    }

    encoding_cost = 0.5 * num_params * log(num_observations); 
    return encoding_cost;
}

#ifdef __cplusplus
}
#endif
