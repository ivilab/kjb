
/* $Id: r_cluster.c 21596 2017-07-30 23:33:36Z kobus $ */

/* =========================================================================== *
|  
|  Copyright (c) by the UA computer vision group (representing affiliated
|  authors and institutions).  
|  
|  Authors of this code include:
|      Prasad Gabbur
|      Kobus Barnard
| 
|  This code is under development and is currently restricted to use within the
|  UA computer vision group except when granted on a case by case basis.
|  (Granting of permission is implicit if a vision group member explicitly
|  provides access to the code, perhaps by E-mailing a tar file). Permission for
|  further distribution of the code is not granted.  Permission for use is
|  further subject to the following general conditions for distribution of UA
|  computer vision code.
| 
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
| 
|  For other use contact kobus AT cs DOT arizona DOT edu.
| 
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or
|  fitness for any particular task. Nonetheless, we are interested in hearing
|  about problems that you encounter. 
|
*  ========================================================================== */


#include "m/m_incl.h"      /*  Only safe if first #include in a ".c" file  */

#include "n/n_invert.h"  
#include "n/n_svd.h"  

#include "r/r_cluster.h"  
#include "r/r_cluster_lib.h"  

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */
/*
#define REGRESS_DO_FIXED_IND_CON_EM_GUTS
#define REGRESS_DO_FIXED_IND_CON_EM_GUTS_LL
#define REGRESS_NO_HACK_FOR_ZERO_MEMBERSHIP
*/
#endif  /* End of code block that is removed for export. */

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
/*
#define SHORT_CUT_POINTERS
*/

/* Question--does this affect the scaling of output parameters? If so, it could
// screw up searches! */
/* #define RESCALE  */

#define DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT  
    
/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */
static int select_GMM_helper
(
    int             max_num_clusters,
    int             independent_flag,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix**        var_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp 
);
#endif  /* End of code block that is removed for export. */

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */
static int get_full_GMM_2
(
    int             num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp,
    double*         log_likelihood_ptr 
);
#endif  /* End of code block that is removed for export. */

#ifdef NOT_CURRENTLY_USED
static int get_discrete_KL_divergence
(
    const Vector* true_distribution_vp,
    const Vector* assumed_distribution_vp,
    double*       KL_divergence_ptr
);
#endif 

static int get_discrete_KL_divergence_log_densities
(
    /*const*/ Vector* true_log_distribution_vp,
    /*const*/ Vector* assumed_log_distribution_vp,
    double*       KL_divergence_ptr
);

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */
#ifdef ENABLE_SHIFT_MODELS
static int get_independent_GMM_with_shift_3
(
    int           min_shift,
    int           max_shift,
    int           num_clusters,
    const Matrix* feature_mp,
    const Int_vector* held_out_indicator_vp,
    const Vector* initial_delta_vp,
    const Vector* initial_a_vp,
    const Matrix* initial_means_mp,
    const Matrix* initial_var_mp,
    Vector**      delta_vpp,
    Matrix**      P_shift_mpp,
    Vector**      a_vpp, 
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_cluster_mpp,
    double*       log_likelihood_ptr,
    double*       held_out_log_likelihood_ptr,
    int*          num_iterations_ptr
);

static int get_independent_GMM_with_shift_4
(
    int           max_left_shift,
    int           max_right_shift,
    int           num_clusters,
    const Matrix* feature_mp,
    const Int_vector* held_out_indicator_vp,
    const Vector* initial_delta_vp,
    const Vector* initial_a_vp,
    const Matrix* initial_means_mp,
    const Matrix* initial_var_mp,
    Vector**      delta_vpp,
    Matrix**      P_shift_mpp,
    Vector**      a_vpp, 
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_cluster_mpp,
    double*       log_likelihood_ptr,
    double*       held_out_log_likelihood_ptr,
    int*          num_iterations_ptr
);

/*
static int shift_point_cyclic
(
    Vector**      shifted_point_vp,
    const Vector* point_vp,
    const int     shift
);

static int shift_point_with_extension
(
    Vector**      shifted_point_vp,
    const Vector* point_vp,
    const int     shift
);

static int shift_point_with_zero_padding
(
    Vector**      shifted_point_vp,
    const Vector* point_vp,
    const int     shift
);
*/
#endif 

/*
static int get_GMM_blk_compound_sym_cov
(
    const Int_vector_vector* block_diag_sizes_vvp,
    const Matrix*            feature_mp,
    const Vector*            held_out_indicator_vp,
    const Vector*            initial_a_vp,
    const double             initial_mu,
    const double             initial_sig_sqr,
    const double             tau_sig_sqr_ratio,
    Vector**                 a_vpp, 
    double*                  mu_ptr,
    double*                  sig_sqr_ptr,
    Matrix**                 P_mpp,
    double*                  log_likelihood_ptr,
    double*                  held_out_log_likelihood_ptr,
    int*                     num_iterations_ptr
);
*/

#endif  /* End of code block that is removed for export. */

/* -------------------------------------------------------------------------- */

/* =================  set_em_cluster_options  ======================
 *
 * | cluster-data-perturbation
 * |     Cluster data perturbation.
 * | cluster-discrete-threshold
 * |     Cluster threshold for excluding discrete probabilities. 
 * | cluster-num-tries-per-cluster-count 
 * |     When trying different cluster counts, this option specifies the number number of EM tries each number of clusters gets (must be at least one) (default 1)
 * | cluster-num-cluster-count-samples
 * |     Number of log-spaced EM clusters counts to try (must be at least one) (default 30).
 * | cluster-max-num-iterations
 * |     The maximum number of EM iterations to use (default 20).
 * | cluster-iteration-tolerance
 * |     EM iteration tolerance (default 1e-6).
 * | cluster-plot-log-likelihood
 * |     Plot EM log likelihood vs num clusters. (Boolean, default false)
 * | cluster-norm-data
 * |     Normalize Cluster data. (boolean, default false)
 * | cluster-var-offset
 * |     Cluster variance offset (default 1e-4)
 * | cluster-use-unbiased-var-estimate-in-M-step
 * |     Unbiased estimate for variance %s used in the M step. (Boolean, default false)
 * | cluster-held-out-data-fraction
 * |     Held out data fraction.3e. (default 1/10)
 * | cluster-tie-var
 * |     Use cluster tie variance (Boolean, default false)
 * | cluster-tie-feature-var
 * |     Use cluster tie feature variance (Boolean, default false)
 * | cluster-tie-cluster-var
 * |     Use cluster tie cluster variance. (Boolean, default false)
 * | cluster-model-selection-training-MDL
 * |     Use training data MDL criterion for model selection. (Boolean, default false)
 * | cluster-model-selection-held-out-LL
 * |     Use held out data log likelihood criterion for model selection. (Boolean, default True)
 * | cluster-model-selection-held-out-MDL
 * |     Use held out data MDL criterion for model selection. (Boolean, default false)
 * | cluster-model-selection-held-out-corr-diff
 * |     Use held out data correlation difference criterion for model selection. (Boolean, default true)
 * | cluster-model-selection-held-out-max-membership
 * |     Use held out data maximum membership criterion for model selection. (Boolean, default true).
 * | cluster-EM-stop-criterion-training-LL
 * |     Use training data log likelihood as EM stopping criterion. (Boolean, default true).
 * | cluster-EM-stop-criterion-held-out-LL
 * |     Use held out data log likelihood as EM stopping criterion. (Boolean, default false).
 * | cluster-use-initialized-cluster-means-variances-and-priors
 * |     Use initialized cluster means and variances in the EM algorithm. (Boolean, default false).
 * | cluster-max-num-CEM-iterations
 * |     The maximum of N CEM iterations are used. (default 100)
 * | cluster-write-CEM-intermediate-results
 * |     Whether to write CEM intermediate results. (default true)
 * | cluster-force-equal-prob-for-CEM-split-and-merge
 * |     Whether the probabilities of choosing a CEM split or a merge operation are forced to be equal. (default true)
 * | cluster-crop-feature-dimensions
 * |     Crop some feature dimensions before clustering  (default false)
 * | cluster-crop-num-feature-dimensions-left
 * |     Number of feature dimensions to the left to crop. (default 0)
 * | cluster-crop-num-feature-dimensions-right
 * |     Number of feature dimensions to the right to crop. (default 0)
 * */
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

            if (num_clusters > max_num_clusters - 1)
            {
                num_clusters--; 
            }

            verbose_pso(2, "Trying %d clusters for %d points.\n", num_clusters,
                        num_points); 

            if (independent_flag)
            {
                num_clusters = get_independent_GMM_2(num_clusters, 
                                                     feature_mp, 
                                                     &a_vp, &u_mp, &var_mp,  
                                                     local_P_mpp, 
                                                     &log_likelihood); 
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

            if (fs_tie_var == TRUE)
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
            encoding_cost *= log( (double) num_points*num_features ); 

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

            encoding_cost = 0.5 * (num_clusters * (num_features + (num_features * (num_features + 1.0) / 2.0) + 1.0) - 1.0); 
            encoding_cost *= log( (double) num_points*num_features ); 

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

static int get_full_GMM_2
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
     * Also need to add fs_var_offset to the diagonal of the covariance matrix. 
     *
     * Likely want to rethink the back-off. 
     *
     * Also want better initialization.
     *
     * Lots of testing! 
    */

    UNTESTED_CODE();

    num_good_points = num_points; 

    /* Koubs. We need to improve the following initialization of I_mp. */
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
                    result = get_matrix_row(&x_vp, feature_mp, i);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = subtract_vectors(&d_vp, x_vp, row_vp); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = get_vector_outer_product(&S_mp, d_vp, d_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = ow_multiply_matrix_by_scalar(S_mp, p); 
                    if (result == ERROR) { NOTE_ERROR(); break; } 

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

/* =============================================================================
 *                 get_independent_GMM
 *                               
 * Finds a Gaussian mixture model (GMM).
 *
 * This routine finds a Gaussian mixture model (GMM) for the data on the
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
 * The model parameters are put into *a_vpp, *u_mpp, and *v_mpp. Any of
 * a_vpp, u_mpp, or v_mpp is NULL if that value is not needed.
 *
 * Both u-sub-i and v-sub-i are vectors, and they are put into the i'th row of
 * *u_mpp and *v_ppp, respectively. The matrices are thus K by M. 
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
        result = get_independent_GMM_2(num_clusters, 
                                       feature_mp, 
                                       a_vpp, 
                                       u_mpp,
                                       var_mpp, 
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

            num_clusters = get_independent_GMM_2(num_clusters, 
                                                 feature_mp, 
                                                 (a_vpp == NULL) ? NULL: &a_vp, 
                                                 (u_mpp == NULL) ? NULL : &u_mp, 
                                                 (var_mpp == NULL) ? NULL : &var_mp, 
                                                 (P_mpp == NULL) ? NULL : &P_mp, 
                                                 &log_likelihood); 
            if (num_clusters == ERROR)
            {
                result = ERROR;
                break; 
            }

            verbose_pso(2, "Raw log likelihood with %d clusters is %.5e,\n", 
                        num_clusters, log_likelihood);

            if (fs_tie_var == TRUE)
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
            encoding_cost *= log( (double) num_points*num_features ); 

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

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */

int select_multinomial_mixture_model
(
    int               max_num_clusters,
    int               num_categories,
    const Int_matrix* item_mp,
    const Matrix*     multiplicity_mp, 
    Vector**          a_vpp, 
    Matrix**          P_i_c_mpp,
    Matrix**          P_c_p_mpp
)
{
    Vector* a_vp           = NULL;
    Matrix* P_i_c_mp       = NULL;
    Matrix* P_c_p_mp       = NULL;
    int     num_points     = item_mp->num_rows;
    int     num_parameters;
    int     num_P_i_c_parameters;
    int     i, j;
    int     num_clusters;
    double  d_num_clusters;
    double  d_min_num_clusters;
    double  d_max_num_clusters;
    Vector* log_likelihood_sum_vp = NULL;
    Vector* min_log_likelihood_vp = NULL;
    Vector* max_log_likelihood_vp = NULL;
    double  f;
    double  max_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    Vector* counts_vp     = NULL;
    double  encoding_cost;
    int     result        = NO_ERROR;
    int     slightly_below_count = 0;
    int     downward_trend       = FALSE;


    /*
    // Thresholding distinctly increases the possibility of zero probability
    // documents. These also occur often with Gaussian clusters, so simply not
    // thresholding for this is not a very good option, as this problem needs to
    // be dealt with anyway. To deal with this problem, there are several
    // possibilities.  1. Backoff documents, creating outliers 2. Returning
    // -infinity for log likelihood, forcing the driver to increase the number
    // of clusters or try again with a different starting point.
    // 
    // Lets try 2 ... 
    //
    */

#define MAX_NUM_IMPOSSIBLE_MODELS 5

    d_min_num_clusters = 2.0;
    d_max_num_clusters = max_num_clusters; 

    f = exp(log(d_max_num_clusters / d_min_num_clusters) / 
                                      (double)(fs_num_cluster_count_samples - 1));

    d_num_clusters = d_min_num_clusters;

    /* Make room for bucket sort. */
    max_num_clusters += 1; 

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
                                        DBL_HALF_MOST_POSITIVE / 2.0); 
    }

    if (result != ERROR)
    {
        result = get_initialized_vector(&max_log_likelihood_vp,
                                        max_num_clusters, 
                                        DBL_HALF_MOST_NEGATIVE / 2.0); 
    }

    for (i = 0; i < fs_num_cluster_count_samples; i++)
    {
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (j = 0; j < fs_num_tries_per_cluster_count; j++)
        {
            double log_likelihood;

            if (result == ERROR) { NOTE_ERROR(); break; }

            num_clusters = kjb_rint(d_num_clusters);

            if (num_clusters > max_num_clusters - 1)
            {
                num_clusters--; 
            }

            verbose_pso(2, "Trying %d clusters for %d points.\n", num_clusters,
                        num_points); 

            num_clusters = get_multinomial_mixture_model(num_clusters, 
                                                            num_categories,
                                                            item_mp, 
                                                            multiplicity_mp,
                                                            (a_vpp == NULL) ? NULL : &a_vp,
                                                            (P_i_c_mpp == NULL) ? NULL: &P_i_c_mp, 
                                                            (P_c_p_mpp == NULL) ? NULL: &P_c_p_mp, 
                                                            &log_likelihood); 

            if (num_clusters == ERROR)
            {
                result = ERROR;
                break; 
            }

            if (log_likelihood < DBL_HALF_MOST_NEGATIVE / 2.0)
            {
                continue;
            }

            verbose_pso(2, "Raw log likelihood with %d clusters is %.5e,\n", 
                        num_clusters, log_likelihood);

            /*
            // In some ways the number of parameters here is arbitrary. We could
            // cut off the P(i|c) at some threshold, thus ignoring small P(i|c),
            // and end up with way few parameters. At first glance it would seem
            // than that the number of parameters per cluster would be twice the
            // number these probabilities because we would then have to store
            // the probability AND the word it applies to, but I don't think the
            // point is to count bytes, but rather the degrees of freedom. Thus,
            // if a threshold was used, we would simply count the number of
            // non-zero probabilities. 
            //
            // In the first crack I did the following which does not really make
            // sense but does give a nice set of clusters. 
            //          num_P_i_c_parameters = num_clusters * 5.0; 
            */

            if (fs_dis_item_prob_threshold > 10.0 * DBL_MIN)
            {
                int ii, jj;

                num_P_i_c_parameters = 0;

                for (ii = 0; ii < num_clusters; ii++)
                {
                    for (jj = 0; jj < num_categories; jj++)
                    {
                        if (P_i_c_mp->elements[ ii ][ jj ] > fs_dis_item_prob_threshold / 2.0)
                        {
                            num_P_i_c_parameters++;
                        }
                    }
                }
            }
            else 
            {
                num_P_i_c_parameters = num_clusters * num_categories; 
            }

            num_parameters = num_P_i_c_parameters + num_clusters; 

            /* MDL log-likelihood penalty */
            encoding_cost = 0.5 * num_parameters * log((double)num_points); 

            log_likelihood -= encoding_cost; 

            verbose_pso(2,"Adjusted log likelihood with %d clusters is %.5e,\n",
                        num_clusters, log_likelihood);

            if (log_likelihood > max_log_likelihood)
            {
                verbose_pso(2,"This is better than the previous max %.5e\n",
                            max_log_likelihood);

                max_log_likelihood = log_likelihood; 

                verbose_pso(2,"Setting max log likelihood to %.5e\n",
                            max_log_likelihood);

                if ((result != ERROR) && (a_vpp != NULL))
                {
                    result = copy_vector(a_vpp, a_vp);
                }

                if ((result != ERROR) && (P_i_c_mpp != NULL))
                {
                    result = copy_matrix(P_i_c_mpp, P_i_c_mp);
                }

                if (P_c_p_mpp != NULL)
                {
                    result = copy_matrix(P_c_p_mpp, P_c_p_mp);
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

            if (result == ERROR) { NOTE_ERROR(); break; } 

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

            if (downward_trend) break; 
        }

        if (downward_trend) break; 

        d_num_clusters *= f; 
    }

    if ((result != ERROR) && (fs_plot_log_likelihood_vs_num_clusters))
    {
        result = plot_log_likelihood(log_likelihood_sum_vp, counts_vp,
                                     min_log_likelihood_vp, 
                                     max_log_likelihood_vp,
                                     (const char*)NULL, (const char*)NULL); 
    }

    free_vector(log_likelihood_sum_vp); 
    free_vector(min_log_likelihood_vp); 
    free_vector(max_log_likelihood_vp); 
    free_vector(counts_vp); 

    free_vector(a_vp); 
    free_matrix(P_c_p_mp);
    free_matrix(P_i_c_mp);

    return result;
}
#endif  /* End of code block that is removed for export. */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */

int get_multinomial_mixture_model
(
    int               num_clusters,
    int               num_categories,
    const Int_matrix* item_mp,
    const Matrix*     multiplicity_mp, 
    Vector**          a_vpp, 
    Matrix**          P_i_c_mpp,
    Matrix**          P_c_p_mpp,
    double*           log_likelihood_ptr 
)
{
    int num_impossible_models = 0;
    int     num_points     = item_mp->num_rows;
    int     num_items      = item_mp->num_cols;
    int     cluster;
    int     point;
    int     item;
    int     category;
    Matrix* local_P_i_c_mp = NULL;
    Matrix* local_P_c_p_mp       = NULL;
    Vector* local_a_vp           = NULL;
    Matrix* P_c_p_mp       = NULL;
    Matrix* P_i_c_mp = NULL;
    Vector* P_sum_vp       = NULL;
    int     it;
    Vector* a_vp           = NULL;
    double  log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double  prev_log_likelihood;
    double  diff;
    int     result   = NO_ERROR;
    double  log_s; 
    double  log_p; 


    /* Even worse now, that we have started to make it ready for prime time.  */
    UNTESTED_CODE();  /* In bad need of a check. Hacked for a paper deadline. */
                      /* Scan for CHECK. */


    if ((result != ERROR) && (P_c_p_mpp == NULL))
    {
        if (get_target_matrix(&local_P_c_p_mp, num_points, num_clusters) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            P_c_p_mp = local_P_c_p_mp;
        }
    }
    else 
    {
        if (get_target_matrix(P_c_p_mpp, num_points, num_clusters) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            P_c_p_mp = *P_c_p_mpp;
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

    if ((result != ERROR) && (P_i_c_mpp == NULL))
    {
        if (get_target_matrix(&local_P_i_c_mp, num_clusters, num_categories) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            P_i_c_mp = local_P_i_c_mp;
        }
    }
    else 
    {
        if (get_target_matrix(P_i_c_mpp, num_clusters, num_categories) == ERROR) 
        {
            result = ERROR; 
        }
        else
        {
            P_i_c_mp = *P_i_c_mpp;
        }
    }

    if (result != ERROR)
    {
        result = get_target_vector(&P_sum_vp, num_clusters); 
    }

try_again:

    prev_log_likelihood = DBL_HALF_MOST_NEGATIVE; 

    if (result != ERROR)
    {
        result = get_random_matrix(&P_c_p_mp, num_points, num_clusters); 
    }

    if (result != ERROR)
    {
        result = ow_multiply_matrix_by_scalar(P_c_p_mp, 0.5);
    }

    if (result != ERROR)
    {
        result = ow_add_scalar_to_matrix(P_c_p_mp, 1.0);
    }

    if (result != ERROR)
    {
        result = ow_scale_matrix_rows_by_sums(P_c_p_mp);
    }

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        int changed_number_of_parameters = FALSE; 

        if (result == ERROR) { NOTE_ERROR(); break; }
        
        /*
        //                           M Step
        // 
        // Calculate P(i|c) and P(c)
        //
        //          sum P(i|p)P(c|p)       (over points)
        // P(i|c) = -----------------------------------
        //          sum P(c|p)             (over points)
        //
        // More pedantically, 
        //                         
        // P(i|c) = P(i|p)P(p|c) = sum P(i|p)P(c|p)P(p)/P(c)  (over points)
        //
        //
        //                         sum P(i|p)P(c|p)P(p)  (over points)
        //                       = ---------------------------------
        //                         sum P(c|p)P(p)        (over points)
        //
        //
        //                         sum P(i|p)P(c|p)  (over points)
        //                       = ---------------------------------
        //                         sum P(c|p)        (over points)
        //
        // P(i|p) is simply the count (0 or 1, here), divided by the number of
        // items. 
        //
        // P(c) = sum P(c|p)P(p)         (over points)
        //
        // Here P(p) is simply taken to be 1/N, N=number of points.
        //
        */

        result = get_zero_matrix(&P_i_c_mp, num_clusters, num_categories);
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            P_sum_vp->elements[ cluster ] = 0.0; 

            for (point = 0; point < num_points; point++)
            {
                double P_c_p = P_c_p_mp->elements[ point ][ cluster ];

                for (item = 0; item < num_items; item++)
                {
                    double weight = P_c_p;

                    if (multiplicity_mp != NULL)
                    {
                        weight *= multiplicity_mp->elements[ point ][ item ];
                    }

                    category = item_mp->elements[ point ][ item ]; 

                    if (category < 0) continue;

                    /*
                    // Could be buggy. CHECK.
                    */
                    ASSERT(category >= 1); 

                    P_i_c_mp->elements[ cluster ][ category - 1 ] += weight;
                }

                /* CHECK */
                P_sum_vp->elements[ cluster ] += P_c_p; 
            }
        }

        if (result == ERROR) { NOTE_ERROR(); break; } 

        result = scale_vector_by_sum(&a_vp, P_sum_vp);
        if (result == ERROR) { NOTE_ERROR(); break; } 

        if (ABS_OF(sum_vector_elements(a_vp) - 1.0) > 0.00001) 
        {
            warn_pso("Sum of mixing weights is %f.\n",
                     sum_vector_elements(a_vp));
        }

        /* CHECK */  /* Is this and the next norm correct. Correct order? */
        if (ow_divide_matrix_by_col_vector(P_i_c_mp, P_sum_vp)
            == ERROR)
        {
            db_rv(P_sum_vp); 
            result = ERROR;
            break;
        }

        /* CHECK */  /* Is this and the previous correct. Correct order? */
        if (ow_scale_matrix_rows_by_sums(P_i_c_mp) == ERROR)
        {
            result = ERROR;
            break;
        }

        if (fs_dis_item_prob_threshold > 10.0 * DBL_MIN)
        {
            int ii, jj;

            for (ii = 0; ii < num_clusters; ii++)
            {
                for (jj = 0; jj < num_categories; jj++)
                {
                    if (     (P_i_c_mp->elements[ ii ][ jj ] > 10.0 * DBL_MIN)
                         &&  (P_i_c_mp->elements[ ii ][ jj ] < fs_dis_item_prob_threshold)
                       )
                    {
                        P_i_c_mp->elements[ ii ][ jj ] = 0.0; 
                        changed_number_of_parameters = TRUE; 
                    }
                }
            }

            if (changed_number_of_parameters)
            {
                result = ow_scale_matrix_rows_by_sums(P_i_c_mp);
                if (result == ERROR) { NOTE_ERROR(); break; } 
            }
        }

        /* 
        //                          E Step 
        //             
        //  Calculate P(c|p)
        //
        //           P(p|c)P(c)     P(p|c)P(c)
        //  P(c|p) = ----------  =  ----------------
        //           P(p)           sum P(p|c')P(c')     (over clusters)
        //
        //
        //  P(p|c) = product P(w|c)       (over words in point)
        // 
        //  P(c) is in a_vp
        */          
        log_likelihood = 0.0; 

        for (point = 0; point < num_points; point++)
        {
            double weight = 1.0; 

            if (result == ERROR) { NOTE_ERROR(); break; } 

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                log_p = SAFE_LOG(a_vp->elements[ cluster ]);

                if (result == ERROR) { NOTE_ERROR(); break; } 

                for (item = 0; item < num_items; item++)
                {
                    category = item_mp->elements[ point ][ item ]; 

                    if (category < 0) continue;

                    ASSERT(category > 0);

                    if (multiplicity_mp != NULL)
                    {
                        weight = multiplicity_mp->elements[ point ][ item ];
                    }

                    log_p += weight * SAFE_LOG(P_i_c_mp->elements[ cluster ][ category - 1 ]);
                }

                P_c_p_mp->elements[ point ][ cluster ] = log_p; 
            }

            log_s = ow_exp_scale_by_sum_log_matrix_row(P_c_p_mp, point);

            log_likelihood += log_s; 

            if (log_likelihood < DBL_HALF_MOST_NEGATIVE / 10.0)
            {
                log_likelihood = DBL_HALF_MOST_NEGATIVE;
                break; 
            }
        }

        if (log_likelihood < DBL_HALF_MOST_NEGATIVE/2.0)
        {

            if (num_impossible_models < MAX_NUM_IMPOSSIBLE_MODELS)
            {
                warn_pso("Model is impossible. Trying again.\n"); 
                num_impossible_models++; 
                goto try_again;
            }
            else 
            {
                warn_pso("Model is impossible. Have tried %d times. ",
                         num_impossible_models); 
                warn_pso("Giving up.\n"); 
                break; 
            }
        }

        if (result != ERROR)
        {
            if (    (IS_LESSER_DBL(log_likelihood, prev_log_likelihood))
                 && ( ! changed_number_of_parameters )
               )
            {
                warn_pso("Log likelihood %.8e is less than previous value of %.8e.\n",
                         log_likelihood, prev_log_likelihood);
            }

            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood +  prev_log_likelihood));

            verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e\n", 
                        it + 1, log_likelihood, diff); 

            if (ABS_OF(diff) < fs_iteration_tolerance) break; 

            prev_log_likelihood = log_likelihood; 
        }
    }

    if ((result != ERROR) && (log_likelihood_ptr != NULL))
    {
        *log_likelihood_ptr = log_likelihood;
    }

    free_vector(P_sum_vp);

    free_matrix(local_P_c_p_mp); 
    free_matrix(local_P_i_c_mp); 
    free_vector(local_a_vp);


    if (result == ERROR) return ERROR; else return num_clusters;
}
#endif  /* End of code block that is removed for export. */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */

int select_GMM_helper_2
(
    int             independent_flag,
    int             max_num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    const Vector*   initial_a_vp,
    const Matrix*   initial_u_mp,
    const Matrix*   initial_var_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix**        var_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp 
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
                            if (fs_tie_var == TRUE)
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
                            encoding_cost *= log( (double) max_num_held_out_points*num_features ); 

                            held_out_log_likelihood -= encoding_cost;
                        }

                        held_out_log_likelihood_mp->elements[i][j] = held_out_log_likelihood;
                        
                        num_iterations_mp->elements[i][j] = num_iterations;
                    }
                
                
                    if (fs_model_selection_held_out_corr_diff == TRUE)
                    {
                        num_clusters_vp->elements[i] = num_clusters;

                        num_iterations_mp->elements[i][j] = num_iterations;

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

            verbose_pso(2, "Raw training log likelihood with %d clusters is %.5e.\n", 
                        num_clusters, log_likelihood);

            if (fs_tie_var == TRUE)
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
            encoding_cost *= log( (double) (num_points - max_num_held_out_points)*num_features ); 

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

#endif  /* End of code block that is removed for export. */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

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
 *                 get_independent_GMM_using_CEM
 *                               
 * Finds a Gaussian mixture model (GMM) by automatically choosing an optimal
 * number of clusters.
 *
 * This routine finds a Gaussian mixture model (GMM) for the data on the
 * assumption that the features are independent. It starts with an initial
 * number of clusters and possibly proceeds through a series of split-merge
 * operations on the clusters to choose an optimal number of clusters. The
 * optimizing criterion for choosing the final number of clusters is based on a
 * trade-off between the log-likelihood and an information theoretic criterion
 * as described in the following paper:
 * 
 * Baibo Zhang, Changshui Zhang, Xing Yi, "Competitive EM Algorithm for Finite
 * Mixture Models", Pattern Recognition, PR(37), No. 1, Jan. 2004, pp. 131-144.
 * 
 * Some features of this routine are controlled via the set facility. 
 *
 * In particular, it fits:
 * |         p(x) = sum  a-sub-i *  g(u-sub-i, v-sub-i, x)
 * |                 i
 * where a-sub-i is the prior probability for the mixuture component (cluster),
 * u-sub-i is the mean vector for component i, v-sub-i is the variance for the
 * component, and g(u,v,x) is a Gaussian with diagonal covariance (i.e., the
 * features are assumed to be independent, given the cluster). 
 *
 * The argument initial_num_clusters is the initial number of mixture components
 * (clusters). 
 *
 * The data matrix feature_mp is an N by M matrix where N is the number of data
 * points, and M is the number of features. 
 *
 * The means, variances and priors of the initial clusters can be specified
 * through the arguments initial_u_mp, initial_var_mp, and initial_a_vp
 * respectively. If they are all NULL, then a random initialization method is used
 * for the clusters.
 *
 * The model parameters are put into *a_vpp, *u_mpp, and *v_mpp. Any of
 * a_vpp, u_mpp, or v_mpp is NULL if that value is not needed.
 *
 * Both u-sub-i and v-sub-i are vectors, and they are put into the i'th row of
 * *u_mpp and *v_mpp, respectively. The matrices are thus K by M, where K is the
 * final number of clusters.
 * 
 * If P_mpp, is not NULL, then the soft clustering (cluster membership) for each
 * data point is returned. In that case, *P_mpp will be N by K. 
 *
 * The parameters gamma and beta control the cluster split-merge operations.
 * These need to be determined experimentally. However, the following
 * observations might help as a rough guide in the selection of values for these:
 * 
 * The higher the value of gamma, the greater is the frequency of split-merge
 * operations and hence greater is the ability to jump in the parameter space (see
 * reference). A value that is of the order of 0.1L or L, where L is the
 * adjusted log-likelihood, has been observed to give reasonable results. The
 * order of L can be determined from the logs printed out by the routine.
 *
 * The higher the value of beta, the greater is the chance of merge operations
 * being chosen relative to split operations (see reference) while proceeding
 * through the split-merge sequence. A value in the range of 0.1R - 10R, where,
 * R = min(KL_divergence_merge^2, KL_divergence_split^2), has been observed to give
 * reasonable results. R has to be determined experimentally by observing the KL
 * divergence values printed out by the routine.
 *
 * Warning:
 *    This routine might lead to memory leaks if fails to execute completely.
 *
 * Returns:
 *    If the routine fails, then ERROR is returned with an error message being 
 *    set. Otherwise NO_ERROR is returned. 
 *
 * Index: clustering, EM, GMM
 *
 * -----------------------------------------------------------------------------
*/

int get_independent_GMM_using_CEM
(
    int             initial_num_clusters,
    const Matrix*   feature_mp,
    const Vector*   initial_a_vp,
    const Matrix*   initial_u_mp,
    const Matrix*   initial_var_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix**        var_mpp,
    Matrix**        P_mpp,
    double          beta,
    double          gamma
)
{
    int        num_points   = feature_mp->num_rows;
    int        num_features = feature_mp->num_cols;
    int        i, j, k, l, m, n, cluster, feature, point;
    int        num_clusters, new_num_clusters;
    
    Vector*    a_vp = NULL; 
    Matrix*    u_mp = NULL;
    Matrix*    var_mp = NULL;
    Matrix*    local_P_mp = NULL; 
    Matrix**   local_P_mpp = (P_mpp == NULL) ? NULL : &local_P_mp; 
    Matrix*    perturbed_feature_mp = NULL;
    
    Matrix*    norm_feature_mp = NULL; 
    Vector*    feature_mean_vp = NULL;
    Vector*    feature_var_vp = NULL;
    
    const double norm_stdev = 1.0;

    int    num_E_M_iterations, max_num_E_M_iterations;
    int    use_initialization_E_M;
    double log_likelihood, held_out_log_likelihood, adjusted_log_likelihood;
    double new_log_likelihood, new_held_out_log_likelihood, adjusted_new_log_likelihood;
    double max_adjusted_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    
    int    num_possible_split_merge_operations;
    double K_L_divergence;
    
    Int_matrix* split_merge_cluster_indices_mp         = NULL;
    Vector*     split_merge_probabilities_vp           = NULL;
    Vector*     split_merge_cumulative_distribution_vp = NULL;
    Vector*     weighted_empirical_distribution_vp     = NULL;
    Vector*     weighted_log_empirical_distribution_vp = NULL;
    Vector*     log_sqrt_det_vp                        = NULL;
    Vector*     x_vp                                   = NULL;
    Matrix*     log_cluster_distributions_mp           = NULL;
    Vector*     cluster_distribution_vp                = NULL;
    Vector*     log_cluster_distribution_vp            = NULL;

    double temp;
    
    double split_merge_a, split_merge_log_sqrt_det;
    
    Vector*     split_merge_u_vp   = NULL;
    Vector*     split_merge_var_vp = NULL;

    Vector*     u_1_vp   = NULL;
    Vector*     u_2_vp   = NULL;
    Vector*     var_1_vp = NULL;
    Vector*     var_2_vp = NULL;

    Vector*     merge_log_cluster_distribution_vp = NULL;
    Matrix*     merge_P_mp                        = NULL;
    Vector*     temp_vp                           = NULL;
    
    double random_number;

    int split_cluster_index, max_var_index;
    int merge_cluster_index_1, merge_cluster_index_2;
    double max_var;
    
    Vector*     local_initial_a_vp   = NULL;
    Matrix*     local_initial_u_mp   = NULL;
    Matrix*     local_initial_var_mp = NULL;
    
    double acceptance_prob;

    int    num_parameters_per_cluster;
    int    num_clusters_annihilate;
    double total_mass_annihilate;
 
    Matrix_vector* intermediate_a_mvp   = NULL;
    Matrix_vector* intermediate_u_mvp   = NULL;
    Matrix_vector* intermediate_var_mvp = NULL;

    Matrix* intermediate_a_mp   = NULL;
    Matrix* intermediate_u_mp   = NULL;
    Matrix* intermediate_var_mp = NULL;
    
    int intermediate_result_index;

    double total_split_KL_divergence, total_merge_inverse_KL_divergence;
    
    if ( fs_data_perturbation > 10.0 * DBL_EPSILON )
    {
        ERE(perturb_unary_cluster_data(&perturbed_feature_mp, feature_mp, fs_data_perturbation, (Int_vector*)NULL)); 
        feature_mp = perturbed_feature_mp; 
    }

    /* Normalization to unit variance in all dimensions. */
    if (fs_normalize_data)
    {
        ERE(copy_matrix(&norm_feature_mp, feature_mp));
        
        ERE(ow_normalize_cluster_data(norm_feature_mp,
                                      (Matrix*)NULL,
                                      &feature_mean_vp, 
                                      &feature_var_vp,
                                      norm_stdev,
                                      (Int_vector*)NULL));
        
        db_rv(feature_var_vp); 
        
        feature_mp = norm_feature_mp; 
    }
    
    num_parameters_per_cluster = num_features + num_features + 1;
    
    /* Compute a first local maxima in the parameter space starting from initial
     * parameter values. */
    num_clusters = initial_num_clusters;

    ERE(copy_vector(&local_initial_a_vp, initial_a_vp));
    ERE(copy_matrix(&local_initial_u_mp, initial_u_mp));
    ERE(copy_matrix(&local_initial_var_mp, initial_var_mp)); 
    pso("First run of EM using %d clusters.\n", num_clusters);

    max_num_E_M_iterations = fs_max_num_iterations;
    use_initialization_E_M = fs_use_initialized_cluster_means_variances_and_priors; 

    /* Keep track of intermediate results. */
    if (fs_write_CEM_intermediate_results == TRUE)
    {
        /* Allocate memory to store intermediate CEM results:
         * Assume that 5 times the total number of CEM iterations will be
         * the maximum number of intermediate results that will be stored.
         * Since the intermediate results being stored include configurations:
         * 1. before and after each EM iteration.
         * 2. before and after every cluster annihilation step.
         * at feasible points in time, the total number of such results cannot
         * be predicted beforehand.
         * 
         * Therefore the above assumption of 5 times the total number of CEM
         * iterations might lead to accessing unallocated memory!!!!!!!!!!!!!!!
         */
        
        ERE(get_target_matrix_vector(&intermediate_a_mvp, 5*fs_max_num_CEM_iterations));
        ERE(get_target_matrix_vector(&intermediate_u_mvp, 5*fs_max_num_CEM_iterations));
        ERE(get_target_matrix_vector(&intermediate_var_mvp, 5*fs_max_num_CEM_iterations));
    }
    
    intermediate_result_index = 0;
    
    /* Keep track of intermediate results. */
    if (fs_write_CEM_intermediate_results == TRUE)
    {
        if ( (local_initial_a_vp != NULL) && (local_initial_u_mp != NULL) && (local_initial_var_mp != NULL) )
        {
            ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], num_clusters, 1));
            ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], num_clusters, num_features));
            ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], num_clusters, num_features));

            intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
            intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
            intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                intermediate_a_mp->elements[cluster][0] = local_initial_a_vp->elements[cluster];
                ERE(copy_matrix_row(intermediate_u_mp, cluster, local_initial_u_mp, cluster));
                ERE(copy_matrix_row(intermediate_var_mp, cluster, local_initial_var_mp, cluster));
            }

            intermediate_result_index = intermediate_result_index + 1; 
        }
    }
    
    do
    {
        ERE(num_clusters = get_independent_GMM_3(num_clusters, 
                                                 feature_mp,
                                                 (Int_vector*) NULL,
                                                 local_initial_a_vp, 
                                                 local_initial_u_mp,
                                                 local_initial_var_mp,
                                                 &a_vp, &u_mp, &var_mp,  
                                                 local_P_mpp, 
                                                 &log_likelihood,
                                                 &held_out_log_likelihood,
                                                 &num_E_M_iterations));

        fs_max_num_iterations = max_num_E_M_iterations;
        fs_use_initialized_cluster_means_variances_and_priors = use_initialization_E_M;

        /* Keep track of intermediate results. */
        if (fs_write_CEM_intermediate_results == TRUE)
        {
            if ( (a_vp != NULL) && (u_mp != NULL) && (var_mp != NULL) )
            {
                ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], num_clusters, 1));
                ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], num_clusters, num_features));
                ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], num_clusters, num_features));

                intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
                intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
                intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    intermediate_a_mp->elements[cluster][0] = a_vp->elements[cluster];
                    ERE(copy_matrix_row(intermediate_u_mp, cluster, u_mp, cluster));
                    ERE(copy_matrix_row(intermediate_var_mp, cluster, var_mp, cluster));
                }

                intermediate_result_index = intermediate_result_index + 1; 
            }
        }

        /* Cluster annihilation. */
        num_clusters_annihilate = 0;
        total_mass_annihilate   = 0.0;
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            if ( (a_vp->elements[cluster] * num_points) < num_parameters_per_cluster )
            {
                ERE(remove_matrix_row((Vector**) NULL, u_mp, (cluster - num_clusters_annihilate)));
                ERE(remove_matrix_row((Vector**) NULL, var_mp, (cluster - num_clusters_annihilate)));
                
                pso("Annihilating cluster %d with posterior %f.\n", cluster, a_vp->elements[cluster]);
                num_clusters_annihilate = num_clusters_annihilate + 1;
                total_mass_annihilate = total_mass_annihilate + a_vp->elements[cluster];
            }
        }

        ERE(get_target_vector(&temp_vp, (num_clusters - num_clusters_annihilate)));
        for (cluster = 0, m = 0; cluster < num_clusters; cluster++)
        {
            if ( (a_vp->elements[cluster] * num_points) >= num_parameters_per_cluster )
            {
                temp_vp->elements[m] = a_vp->elements[cluster] + (total_mass_annihilate/(num_clusters - num_clusters_annihilate));
                m = m+1;
            }
        }
        ERE(scale_vector_by_sum(&a_vp, temp_vp));

        num_clusters = num_clusters - num_clusters_annihilate;

        if (num_clusters_annihilate > 0)
        {
            ERE(copy_vector(&local_initial_a_vp, a_vp));
            ERE(copy_matrix(&local_initial_u_mp, u_mp));
            ERE(copy_matrix(&local_initial_var_mp, var_mp));
        
            /* Keep track of intermediate results. */
            if (fs_write_CEM_intermediate_results == TRUE)
            {
                if ( (a_vp != NULL) && (u_mp != NULL) && (var_mp != NULL) )
                {
                    ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], num_clusters, 1));
                    ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], num_clusters, num_features));
                    ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], num_clusters, num_features));

                    intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
                    intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
                    intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

                    for (cluster = 0; cluster < num_clusters; cluster++)
                    {
                        intermediate_a_mp->elements[cluster][0] = a_vp->elements[cluster];
                        ERE(copy_matrix_row(intermediate_u_mp, cluster, u_mp, cluster));
                        ERE(copy_matrix_row(intermediate_var_mp, cluster, var_mp, cluster));
                    }

                    intermediate_result_index = intermediate_result_index + 1; 
                }
            }

            /* This is necessary because the first run of EM might not have used
               initialized means, variances and priors. */
            fs_use_initialized_cluster_means_variances_and_priors = TRUE;
            
            pso("Running EM for %d clusters.\n", num_clusters);
        }
        
    } while (num_clusters_annihilate > 0);
    
    ERE(get_target_vector(&log_sqrt_det_vp, num_clusters));
    for (cluster = 0; cluster < num_clusters; cluster++)
    {
        temp = 0.0;

        for (feature = 0; feature < num_features; feature++)
        {
            double var = var_mp->elements[ cluster ][ feature ]; 
            temp += SAFE_LOG(var);
        }

        log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
    }
    verify_vector(log_sqrt_det_vp, NULL);


    for (i = 0; i < fs_max_num_CEM_iterations; i++)
    {
        num_possible_split_merge_operations = num_clusters + ((num_clusters * (num_clusters - 1)) / 2);

        ERE(get_target_int_matrix(&split_merge_cluster_indices_mp, 2, num_possible_split_merge_operations));
        ERE(get_target_vector(&split_merge_probabilities_vp, num_possible_split_merge_operations));

        ERE(get_target_matrix(&log_cluster_distributions_mp, num_points, num_clusters));

        for (point = 0; point < num_points; point++)
        {
            ERE(get_matrix_row(&x_vp, feature_mp, point));

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                double log_a = SAFE_LOG(a_vp->elements[ cluster ]);

                double* x_ptr = x_vp->elements;
                double* u_ptr = u_mp->elements[ cluster ];
                double* v_ptr = var_mp->elements[ cluster ];

                double dev;
                double d = 0.0;

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

                log_cluster_distributions_mp->elements[ point ][ cluster ] = 0.0 - (0.5*d) - log_sqrt_det_vp->elements[ cluster ];
                (*local_P_mpp)->elements[ point ][ cluster ]  = log_a + log_cluster_distributions_mp->elements[ point ][ cluster ];

                ASSERT_IS_NUMBER_DBL(log_cluster_distributions_mp->elements[point][cluster]);
                ASSERT_IS_FINITE_DBL(log_cluster_distributions_mp->elements[point][cluster]);
                ASSERT_IS_NUMBER_DBL((*local_P_mpp)->elements[point][cluster]);
                ASSERT_IS_FINITE_DBL((*local_P_mpp)->elements[point][cluster]);
                
                log_cluster_distributions_mp->elements[ point ][ cluster ] -= ((double)num_features / 2.0) * SAFE_LOG(2.0 * M_PI); 

                ASSERT_IS_NUMBER_DBL(log_cluster_distributions_mp->elements[point][cluster]);
                ASSERT_IS_FINITE_DBL(log_cluster_distributions_mp->elements[point][cluster]);
            }
            
            ERE(get_matrix_row(&temp_vp, *local_P_mpp, point));
            temp = ow_exp_scale_by_sum_log_vector(temp_vp);
            ASSERT_IS_NUMBER_DBL(temp);
            ASSERT_IS_FINITE_DBL(temp);
            ERE(put_matrix_row(*local_P_mpp, temp_vp, point));
        }

        for (j = 0, n = num_clusters; j < num_clusters; j++)
        {
            /*
            ERE(get_matrix_col(&weighted_empirical_distribution_vp, *local_P_mpp, j));
            ERE(ow_scale_vector_by_sum(weighted_empirical_distribution_vp));

            ERE(get_matrix_col(&cluster_distribution_vp, log_cluster_distributions_mp, j));
            ow_exp_scale_by_sum_log_vector(cluster_distribution_vp);

            ERE(get_discrete_KL_divergence(weighted_empirical_distribution_vp, cluster_distribution_vp, &K_L_divergence));
            */

            ERE(get_matrix_col(&weighted_empirical_distribution_vp, *local_P_mpp, j));
            ERE(log_vector_2(&weighted_log_empirical_distribution_vp, weighted_empirical_distribution_vp, DBL_HALF_MOST_NEGATIVE));

            ERE(get_matrix_col(&log_cluster_distribution_vp, log_cluster_distributions_mp, j));
            
            ERE(get_discrete_KL_divergence_log_densities(weighted_log_empirical_distribution_vp, log_cluster_distribution_vp, &K_L_divergence));
            /* The following is a hack to deal with the fact that the
             * weighted empirical distribution is discrete and hence has an
             * out of bound differential entropy. This would ideally lead to
             * infinite KL divergence between it and a continuous
             * distribution. We convert it into a continuous distribution by 
             * spreading the point probability masses into very small
             * hypercubes around the discrete points.
            K_L_divergence = K_L_divergence + (DBL_HALF_MOST_POSITIVE / num_possible_split_merge_operations);
            */

            pso("KL divergence for splitting component %d with posterior %1.5f : %f.\n", j, a_vp->elements[j], K_L_divergence); 

            split_merge_cluster_indices_mp->elements[0][j] = j;
            split_merge_cluster_indices_mp->elements[1][j] = j;

            split_merge_probabilities_vp->elements[j] = K_L_divergence;

            for (k = j+1; k < num_clusters; k++)
            {
                split_merge_a = a_vp->elements[j] + a_vp->elements[k];

                ASSERT(split_merge_a > 0.0);

                ERE(get_zero_vector(&split_merge_u_vp, num_features));
                ERE(get_matrix_row(&u_1_vp, u_mp, j));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_u_vp, u_1_vp, (a_vp->elements[j]/split_merge_a)));
                ERE(get_matrix_row(&u_2_vp, u_mp, k));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_u_vp, u_2_vp, (a_vp->elements[k]/split_merge_a)));

                ERE(get_zero_vector(&split_merge_var_vp, num_features));
                ERE(get_matrix_row(&var_1_vp, var_mp, j));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_var_vp, var_1_vp, (a_vp->elements[j]/split_merge_a)));
                ERE(subtract_vectors(&var_1_vp, u_1_vp, split_merge_u_vp));
                ERE(ow_multiply_vectors(var_1_vp, var_1_vp));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_var_vp, var_1_vp, (a_vp->elements[j]/split_merge_a)));
                ERE(get_matrix_row(&var_2_vp, var_mp, k));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_var_vp, var_2_vp, (a_vp->elements[k]/split_merge_a)));
                ERE(subtract_vectors(&var_2_vp, u_2_vp, split_merge_u_vp));
                ERE(ow_multiply_vectors(var_2_vp, var_2_vp));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_var_vp, var_2_vp, (a_vp->elements[k]/split_merge_a)));
         
                /*
                   Though the following is theoretically correct, it is not
                   feasible to do in very high dimensions. This is because even
                   small values of variance in each dimension multiply to very
                   large values not allowed by the available precision. Instead,
                   do log(var) and then add.
                   
                db_rv(split_merge_var_vp);
                split_merge_log_sqrt_det = multiply_vector_elements(split_merge_var_vp);
                ASSERT_IS_NUMBER_DBL(split_merge_log_sqrt_det);
                ASSERT_IS_FINITE_DBL(split_merge_log_sqrt_det); 
                split_merge_log_sqrt_det = SAFE_LOG(split_merge_log_sqrt_det);
                */

                split_merge_log_sqrt_det = 0.0;

                for (feature = 0; feature < num_features; feature++)
                {
                    split_merge_log_sqrt_det += SAFE_LOG(split_merge_var_vp->elements[feature]);
                }

                ASSERT_IS_NUMBER_DBL(split_merge_log_sqrt_det);
                ASSERT_IS_FINITE_DBL(split_merge_log_sqrt_det); 
                split_merge_log_sqrt_det = split_merge_log_sqrt_det / 2.0;

                ERE(get_target_vector(&merge_log_cluster_distribution_vp, num_points));
                ERE(get_target_matrix(&merge_P_mp, num_points, (num_clusters-1)));

                for (point = 0; point < num_points; point++)
                {
                    ERE(get_matrix_row(&x_vp, feature_mp, point));

                    {
                        double log_a = SAFE_LOG(split_merge_a);
                        double* x_ptr = x_vp->elements;
                        double* u_ptr = split_merge_u_vp->elements;
                        double* v_ptr = split_merge_var_vp->elements;
                        double dev;
                        double d = 0.0;

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

                        merge_log_cluster_distribution_vp->elements[point] = 0.0 - (0.5*d) - split_merge_log_sqrt_det;
                        merge_P_mp->elements[point][0] = log_a + merge_log_cluster_distribution_vp->elements[point];

                        ASSERT_IS_NUMBER_DBL(merge_log_cluster_distribution_vp->elements[point]);
                        ASSERT_IS_FINITE_DBL(merge_log_cluster_distribution_vp->elements[point]);
                        ASSERT_IS_NUMBER_DBL(merge_P_mp->elements[point][0]);
                        ASSERT_IS_FINITE_DBL(merge_P_mp->elements[point][0]);

                        merge_log_cluster_distribution_vp->elements[point] -= ((double)num_features / 2.0) * SAFE_LOG(2.0 * M_PI);

                        ASSERT_IS_NUMBER_DBL(merge_log_cluster_distribution_vp->elements[point]);
                        ASSERT_IS_FINITE_DBL(merge_log_cluster_distribution_vp->elements[point]);

                        for (l = 0, m = 1; l < num_clusters; l++)
                        {
                            if ( (l != j) && (l != k) )
                            {
                                log_a = SAFE_LOG(a_vp->elements[l]);

                                x_ptr = x_vp->elements;
                                u_ptr = u_mp->elements[l];
                                v_ptr = var_mp->elements[l];

                                d = 0.0;

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

                                merge_P_mp->elements[point][m] = log_a - (0.5*d) - log_sqrt_det_vp->elements[l];

                                ASSERT_IS_NUMBER_DBL(merge_P_mp->elements[point][m]);
                                ASSERT_IS_FINITE_DBL(merge_P_mp->elements[point][m]);

                                m = m+1;
                            }
                        }

                        ERE(get_matrix_row(&temp_vp, merge_P_mp, point));
                        temp = ow_exp_scale_by_sum_log_vector(temp_vp);
                        ASSERT_IS_NUMBER_DBL(temp);
                        ASSERT_IS_FINITE_DBL(temp);
                        ERE(put_matrix_row(merge_P_mp, temp_vp, point));
                    }
                }

                /*
                ERE(get_matrix_col(&weighted_empirical_distribution_vp, merge_P_mp, 0));
                ERE(ow_scale_vector_by_sum(weighted_empirical_distribution_vp));

                ERE(copy_vector(&cluster_distribution_vp, merge_log_cluster_distribution_vp));
                ow_exp_scale_by_sum_log_vector(cluster_distribution_vp);

                ERE(get_discrete_KL_divergence(weighted_empirical_distribution_vp, cluster_distribution_vp, &K_L_divergence));
                */

                ERE(get_matrix_col(&weighted_empirical_distribution_vp, merge_P_mp, 0));
                ERE(log_vector_2(&weighted_log_empirical_distribution_vp, weighted_empirical_distribution_vp, DBL_HALF_MOST_NEGATIVE));

                ERE(copy_vector(&log_cluster_distribution_vp, merge_log_cluster_distribution_vp));

                ERE(get_discrete_KL_divergence_log_densities(weighted_log_empirical_distribution_vp, log_cluster_distribution_vp, &K_L_divergence)); 

                /* The following is a hack to deal with the fact that the
                 * weighted empirical distribution is discrete and hence has an
                 * out of bound differential entropy. This would ideally lead to
                 * infinite KL divergence between it and a continuous
                 * distribution. We convert it into a continuous distribution by 
                 * spreading the point probability masses into very small
                 * hypercubes around the discrete points.
                K_L_divergence = K_L_divergence + (DBL_HALF_MOST_POSITIVE / num_possible_split_merge_operations);
                */

                pso("KL divergence for merging components %d and %d : %f.\n", j, k, K_L_divergence);

                split_merge_cluster_indices_mp->elements[0][n] = j;
                split_merge_cluster_indices_mp->elements[1][n] = k;

                if (fs_force_equal_prob_for_CEM_split_and_merge == TRUE)
                {
                    split_merge_probabilities_vp->elements[n] = 1.0 / K_L_divergence;
                }
                else
                {
                    split_merge_probabilities_vp->elements[n] = beta / K_L_divergence;
                }

                n = n+1;
            }
        }

        ASSERT(n == num_possible_split_merge_operations);

        if (fs_force_equal_prob_for_CEM_split_and_merge == TRUE)
        {
            total_split_KL_divergence         = 0;
            total_merge_inverse_KL_divergence = 0;

            for (k = 0; k < num_clusters; k++)
            {
                total_split_KL_divergence = total_split_KL_divergence + split_merge_probabilities_vp->elements[k];
            }

            for (k = num_clusters; k < num_possible_split_merge_operations; k++)
            {
                total_merge_inverse_KL_divergence = total_merge_inverse_KL_divergence + split_merge_probabilities_vp->elements[k];
            }

            /* Compute beta online so that the total probability of all split
             * choices equals the total probability of all merge choices after
             * normalization. */ 
            beta = total_split_KL_divergence / total_merge_inverse_KL_divergence;

            for (k = num_clusters; k < num_possible_split_merge_operations; k++)
            {
                split_merge_probabilities_vp->elements[k] = beta * split_merge_probabilities_vp->elements[k];
            }
        }

        ERE(ow_scale_vector_by_sum(split_merge_probabilities_vp));
        db_rv(split_merge_probabilities_vp);

        ERE(get_zero_vector(&split_merge_cumulative_distribution_vp, num_possible_split_merge_operations));
        for (k = 0, temp = 0; k < num_possible_split_merge_operations; k++)
        {
            temp = temp + split_merge_probabilities_vp->elements[k];
            split_merge_cumulative_distribution_vp->elements[k] = temp;

            dbf(split_merge_cumulative_distribution_vp->elements[k]); 
        }

        ASSERT(split_merge_cumulative_distribution_vp->elements[num_possible_split_merge_operations-1] > 0.999);

        do
        {
            
            /* Operation sampling. */
            random_number = kjb_rand();
            for (k = 0; k < num_possible_split_merge_operations; k++)
            {
                if ( random_number <= split_merge_cumulative_distribution_vp->elements[k] )
                {
                    pso("Choosing %s operation with probability %f.\n",\
                            ((k < num_clusters) ? "split" : "merge"), split_merge_probabilities_vp->elements[k]);

                    break;
                }
            }

            ASSERT(k < num_possible_split_merge_operations);

            if (split_merge_cluster_indices_mp->elements[0][k] == split_merge_cluster_indices_mp->elements[1][k])
            {
                /* Split operation:
                 * Obtain the split means, variances and priors. */

                pso("Computing new parameters due to splitting component %d.\n",\
                        split_merge_cluster_indices_mp->elements[0][k]); 

                ERE(get_target_vector(&local_initial_a_vp, num_clusters+1));
                ERE(get_target_matrix(&local_initial_u_mp, num_clusters+1, num_features));
                ERE(get_target_matrix(&local_initial_var_mp, num_clusters+1, num_features));

                split_cluster_index = split_merge_cluster_indices_mp->elements[0][k];

                for (l = 0, m = 0; l < num_clusters; l++)
                {
                    if (l != split_cluster_index)
                    {
                        local_initial_a_vp->elements[m] = a_vp->elements[l];
                        ERE(copy_matrix_row(local_initial_u_mp, m, u_mp, l));
                        ERE(copy_matrix_row(local_initial_var_mp, m, var_mp, l));

                        m = m+1;
                    }
                }

                ERE(get_matrix_row(&temp_vp, var_mp, split_cluster_index));
                ERE(max_var_index = get_max_vector_element(temp_vp, &max_var));

                ERE(get_matrix_row(&u_1_vp, u_mp, split_cluster_index));
                ERE(get_matrix_row(&u_2_vp, u_mp, split_cluster_index));
                ERE(get_matrix_row(&var_1_vp, var_mp, split_cluster_index));
                ERE(get_matrix_row(&var_2_vp, var_mp, split_cluster_index));

                u_1_vp->elements[max_var_index] = u_1_vp->elements[max_var_index] - sqrt(max_var);
                u_2_vp->elements[max_var_index] = u_2_vp->elements[max_var_index] + sqrt(max_var);

                var_1_vp->elements[max_var_index] = 0.25 * max_var;
                var_2_vp->elements[max_var_index] = 0.25 * max_var;

                local_initial_a_vp->elements[m] = 0.5 * a_vp->elements[split_cluster_index];
                ERE(put_matrix_row(local_initial_u_mp, u_1_vp, m));
                ERE(put_matrix_row(local_initial_var_mp, var_1_vp, m));
                m = m+1;

                local_initial_a_vp->elements[m] = 0.5 * a_vp->elements[split_cluster_index];
                ERE(put_matrix_row(local_initial_u_mp, u_2_vp, m));
                ERE(put_matrix_row(local_initial_var_mp, var_2_vp, m));

                /* Hack: Store a copy of the standard user specified options 
                 * and call the EM module using initial means, variances and
                 * priors for one iteration. Then restore back the options.
                 */
                max_num_E_M_iterations = fs_max_num_iterations;
                use_initialization_E_M = fs_use_initialized_cluster_means_variances_and_priors; 

                /*
                   fs_max_num_iterations = 1;
                 */
                fs_use_initialized_cluster_means_variances_and_priors = TRUE;

                pso("Determining log likelihood due to split resulting in %d clusters.\n", num_clusters+1);
                ERE(new_num_clusters = get_independent_GMM_3((num_clusters+1), 
                            feature_mp,
                            (Int_vector*) NULL,
                            local_initial_a_vp, 
                            local_initial_u_mp,
                            local_initial_var_mp,
                            (Vector**) NULL, 
                            (Matrix**) NULL,
                            (Matrix**) NULL,  
                            (Matrix**) NULL, 
                            &new_log_likelihood,
                            &new_held_out_log_likelihood,
                            &num_E_M_iterations));                

                fs_max_num_iterations = max_num_E_M_iterations;
                fs_use_initialized_cluster_means_variances_and_priors = use_initialization_E_M;
            }
            else
            {
                /* Merge operation:
                 * Obtain the merged means, variances and priors. */

                pso("Computing new parameters due to merging components %d and %d.\n",\
                        split_merge_cluster_indices_mp->elements[0][k], split_merge_cluster_indices_mp->elements[1][k]); 

                ERE(get_target_vector(&local_initial_a_vp, num_clusters-1));
                ERE(get_target_matrix(&local_initial_u_mp, num_clusters-1, num_features));
                ERE(get_target_matrix(&local_initial_var_mp, num_clusters-1, num_features));

                merge_cluster_index_1 = split_merge_cluster_indices_mp->elements[0][k];
                merge_cluster_index_2 = split_merge_cluster_indices_mp->elements[1][k];

                for (l = 0, m = 0; l < num_clusters; l++)
                {
                    if ( (l != merge_cluster_index_1) && (l != merge_cluster_index_2) )
                    {
                        local_initial_a_vp->elements[m] = a_vp->elements[l];
                        ERE(copy_matrix_row(local_initial_u_mp, m, u_mp, l));
                        ERE(copy_matrix_row(local_initial_var_mp, m, var_mp, l));

                        m = m+1;
                    }
                }

                split_merge_a = a_vp->elements[merge_cluster_index_1] + a_vp->elements[merge_cluster_index_2];

                ASSERT(split_merge_a > 0.0);

                ERE(get_zero_vector(&split_merge_u_vp, num_features));
                ERE(get_matrix_row(&u_1_vp, u_mp, merge_cluster_index_1));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_u_vp, u_1_vp, (a_vp->elements[merge_cluster_index_1]/split_merge_a)));
                ERE(get_matrix_row(&u_2_vp, u_mp, merge_cluster_index_2));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_u_vp, u_2_vp, (a_vp->elements[merge_cluster_index_2]/split_merge_a)));

                ERE(get_zero_vector(&split_merge_var_vp, num_features));
                ERE(get_matrix_row(&var_1_vp, var_mp, merge_cluster_index_1));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_var_vp, var_1_vp, (a_vp->elements[merge_cluster_index_1]/split_merge_a)));
                ERE(subtract_vectors(&var_1_vp, u_1_vp, split_merge_u_vp));
                ERE(ow_multiply_vectors(var_1_vp, var_1_vp));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_var_vp, var_1_vp, (a_vp->elements[merge_cluster_index_1]/split_merge_a)));
                ERE(get_matrix_row(&var_2_vp, var_mp, merge_cluster_index_2));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_var_vp, var_2_vp, (a_vp->elements[merge_cluster_index_2]/split_merge_a)));
                ERE(subtract_vectors(&var_2_vp, u_2_vp, split_merge_u_vp));
                ERE(ow_multiply_vectors(var_2_vp, var_2_vp));
                ERE(ow_add_scalar_times_vector_to_vector(split_merge_var_vp, var_2_vp, (a_vp->elements[merge_cluster_index_2]/split_merge_a)));

                local_initial_a_vp->elements[m] = split_merge_a;
                ERE(put_matrix_row(local_initial_u_mp, split_merge_u_vp, m));
                ERE(put_matrix_row(local_initial_var_mp, split_merge_var_vp, m)); 

                /* Hack: Store a copy of the standard user specified options 
                 * and call the EM module using initial means, variances and
                 * priors for one iteration. Then restore back the options.
                 */
                max_num_E_M_iterations = fs_max_num_iterations;
                use_initialization_E_M = fs_use_initialized_cluster_means_variances_and_priors; 

                /*
                   fs_max_num_iterations = 1;
                 */
                fs_use_initialized_cluster_means_variances_and_priors = TRUE;

                pso("Determining log likelihood due to merge resulting in %d clusters.\n", num_clusters-1);
                ERE(new_num_clusters = get_independent_GMM_3((num_clusters-1), 
                            feature_mp,
                            (Int_vector*) NULL,
                            local_initial_a_vp, 
                            local_initial_u_mp,
                            local_initial_var_mp,
                            (Vector**) NULL, 
                            (Matrix**) NULL, 
                            (Matrix**) NULL,  
                            (Matrix**) NULL, 
                            &new_log_likelihood,
                            &new_held_out_log_likelihood,
                            &num_E_M_iterations));                

                fs_max_num_iterations = max_num_E_M_iterations;
                fs_use_initialized_cluster_means_variances_and_priors = use_initialization_E_M;
            }

            /* Acceptance sampling. */
            adjusted_log_likelihood = log_likelihood - (0.5 * num_clusters * SAFE_LOG(num_points / 12.0)); 
            adjusted_log_likelihood = adjusted_log_likelihood - (0.5 * num_clusters * (num_parameters_per_cluster + 1.0));
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                adjusted_log_likelihood = adjusted_log_likelihood - (0.5 * num_parameters_per_cluster * SAFE_LOG(num_points * a_vp->elements[cluster] / 12.0));
            }

            /* Keep track of the best model based on maximum adjusted log
             * likelihood. */
            if (adjusted_log_likelihood > max_adjusted_log_likelihood)
            {
                max_adjusted_log_likelihood = adjusted_log_likelihood;

                if (a_vpp != NULL)
                {
                    ERE(copy_vector(a_vpp, a_vp));
                }

                if (u_mpp != NULL)
                {
                    ERE(copy_matrix(u_mpp, u_mp));
                }

                if (var_mpp != NULL)
                {
                    ERE(copy_matrix(var_mpp, var_mp));
                }

                if (P_mpp != NULL)
                {
                    ERE(copy_matrix(P_mpp, local_P_mp));
                }

                /* Keep track of intermediate results. */
                if (fs_write_CEM_intermediate_results == TRUE)
                {
                    if ( (a_vp != NULL) && (u_mp != NULL) && (var_mp != NULL) )
                    {
                        ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], num_clusters, 1));
                        ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], num_clusters, num_features));
                        ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], num_clusters, num_features));

                        intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
                        intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
                        intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

                        for (cluster = 0; cluster < num_clusters; cluster++)
                        {
                            intermediate_a_mp->elements[cluster][0] = a_vp->elements[cluster];
                            ERE(copy_matrix_row(intermediate_u_mp, cluster, u_mp, cluster));
                            ERE(copy_matrix_row(intermediate_var_mp, cluster, var_mp, cluster));
                        }

                        intermediate_result_index = intermediate_result_index + 1; 
                    }
                }
            }

            adjusted_new_log_likelihood = new_log_likelihood - (0.5 * new_num_clusters * SAFE_LOG(num_points / 12.0)); 
            adjusted_new_log_likelihood = adjusted_new_log_likelihood - (0.5 * new_num_clusters * (num_parameters_per_cluster + 1.0));
            for (cluster = 0; cluster < new_num_clusters; cluster++)
            {
                adjusted_new_log_likelihood = adjusted_new_log_likelihood - (0.5 * num_parameters_per_cluster * SAFE_LOG(num_points * local_initial_a_vp->elements[cluster] / 12.0));
            }

            pso("%-3d: Old log likelihood with %d clusters: %12e.\n", i+1, num_clusters, log_likelihood);
            pso("%-3d: New log likelihood with %d clusters: %12e.\n", i+1, new_num_clusters, new_log_likelihood);
            pso("%-3d: Adjusted old log likelihood with %d clusters: %12e.\n", i+1, num_clusters, adjusted_log_likelihood);
            pso("%-3d: Adjusted new log likelihood with %d clusters: %12e.\n", i+1, new_num_clusters, adjusted_new_log_likelihood);

            acceptance_prob = exp((adjusted_new_log_likelihood - adjusted_log_likelihood)/gamma);

            if (kjb_rand() < acceptance_prob)
            {
                break;
            }

            i = i+1;

            if (i >= fs_max_num_CEM_iterations)
            {
                break;
            }
            
        } while (1);
        

        if (i >= fs_max_num_CEM_iterations)
        {
            break;
        }
        
        /*
        if (kjb_rand() < acceptance_prob)
        */
        {
            pso("Accepting %s operation with probability %10e.\n",\
            ((split_merge_cluster_indices_mp->elements[0][k] ==\
            split_merge_cluster_indices_mp->elements[1][k]) ? "split" :\
            "merge"), acceptance_prob);

            /* Keep track of intermediate results. */
            if (fs_write_CEM_intermediate_results == TRUE)
            {
                if ( (local_initial_a_vp != NULL) && (local_initial_u_mp != NULL) && (local_initial_var_mp != NULL) )
                {
                    ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], new_num_clusters, 1));
                    ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], new_num_clusters, num_features));
                    ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], new_num_clusters, num_features));

                    intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
                    intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
                    intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

                    for (cluster = 0; cluster < new_num_clusters; cluster++)
                    {
                        intermediate_a_mp->elements[cluster][0] = local_initial_a_vp->elements[cluster];
                        ERE(copy_matrix_row(intermediate_u_mp, cluster, local_initial_u_mp, cluster));
                        ERE(copy_matrix_row(intermediate_var_mp, cluster, local_initial_var_mp, cluster));
                    }

                    intermediate_result_index = intermediate_result_index + 1; 
                }
            }

            do
            {
                /* Hack: Store a copy of the standard user specified EM
                 * initialization option and call the EM module using initial 
                 * means, variances and priors. Then restore back the options.
                 */
                use_initialization_E_M = fs_use_initialized_cluster_means_variances_and_priors; 

                fs_use_initialized_cluster_means_variances_and_priors = TRUE;

                ERE(num_clusters = get_independent_GMM_3(new_num_clusters, 
                                                         feature_mp,
                                                         (Int_vector*) NULL,
                                                         local_initial_a_vp, 
                                                         local_initial_u_mp,
                                                         local_initial_var_mp,
                                                         &a_vp, &u_mp, &var_mp,  
                                                         local_P_mpp, 
                                                         &log_likelihood,
                                                         &held_out_log_likelihood,
                                                         &num_E_M_iterations));                

                fs_use_initialized_cluster_means_variances_and_priors = use_initialization_E_M;

                pso("%-3d: Log likelihood with %d clusters after EM: %12e.\n", i+1, num_clusters, log_likelihood);

                /* Keep track of intermediate results. */
                if (fs_write_CEM_intermediate_results == TRUE)
                {
                    if ( (a_vp != NULL) && (u_mp != NULL) && (var_mp != NULL) )
                    {
                        ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], num_clusters, 1));
                        ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], num_clusters, num_features));
                        ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], num_clusters, num_features));

                        intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
                        intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
                        intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

                        for (cluster = 0; cluster < num_clusters; cluster++)
                        {
                            intermediate_a_mp->elements[cluster][0] = a_vp->elements[cluster];
                            ERE(copy_matrix_row(intermediate_u_mp, cluster, u_mp, cluster));
                            ERE(copy_matrix_row(intermediate_var_mp, cluster, var_mp, cluster));
                        }

                        intermediate_result_index = intermediate_result_index + 1; 
                    }
                }

                /* Cluster annihilation. */
                num_clusters_annihilate = 0;
                total_mass_annihilate   = 0.0;
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    if ( (a_vp->elements[cluster] * num_points) < num_parameters_per_cluster )
                    {
                        ERE(remove_matrix_row((Vector**) NULL, u_mp, (cluster - num_clusters_annihilate)));
                        ERE(remove_matrix_row((Vector**) NULL, var_mp, (cluster - num_clusters_annihilate)));
                        
                        pso("Annihilating cluster %d with posterior %f.\n", cluster, a_vp->elements[cluster]);
                        num_clusters_annihilate = num_clusters_annihilate + 1;
                        total_mass_annihilate = total_mass_annihilate + a_vp->elements[cluster];
                    }
                }

                ERE(get_target_vector(&temp_vp, (num_clusters - num_clusters_annihilate)));
                for (cluster = 0, m = 0; cluster < num_clusters; cluster++)
                {
                    if ( (a_vp->elements[cluster] * num_points) >= num_parameters_per_cluster )
                    {
                        temp_vp->elements[m] = a_vp->elements[cluster] + (total_mass_annihilate/(num_clusters - num_clusters_annihilate));
                        m = m+1;
                    }
                }
                ERE(scale_vector_by_sum(&a_vp, temp_vp));

                num_clusters     = num_clusters - num_clusters_annihilate;
                new_num_clusters = num_clusters;

                if (num_clusters_annihilate > 0)
                {
                    ERE(copy_vector(&local_initial_a_vp, a_vp));
                    ERE(copy_matrix(&local_initial_u_mp, u_mp));
                    ERE(copy_matrix(&local_initial_var_mp, var_mp));

                    /* Keep track of intermediate results. */
                    if (fs_write_CEM_intermediate_results == TRUE)
                    {
                        if ( (a_vp != NULL) && (u_mp != NULL) && (var_mp != NULL) )
                        {
                            ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], num_clusters, 1));
                            ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], num_clusters, num_features));
                            ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], num_clusters, num_features));

                            intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
                            intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
                            intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

                            for (cluster = 0; cluster < num_clusters; cluster++)
                            {
                                intermediate_a_mp->elements[cluster][0] = a_vp->elements[cluster];
                                ERE(copy_matrix_row(intermediate_u_mp, cluster, u_mp, cluster));
                                ERE(copy_matrix_row(intermediate_var_mp, cluster, var_mp, cluster));
                            }

                            intermediate_result_index = intermediate_result_index + 1; 
                        }
                    }

                    pso("Running EM for %d clusters.\n", num_clusters);
                }

            } while (num_clusters_annihilate > 0);
            
            ERE(get_target_vector(&log_sqrt_det_vp, num_clusters));
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                temp = 0.0;

                for (feature = 0; feature < num_features; feature++)
                {
                    double var = var_mp->elements[ cluster ][ feature ]; 
                    temp += SAFE_LOG(var);
                }

                log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
            }
            verify_vector(log_sqrt_det_vp, NULL);
        }
    }

    adjusted_log_likelihood = log_likelihood - (0.5 * num_clusters * SAFE_LOG(num_points / 12.0)); 
    adjusted_log_likelihood = adjusted_log_likelihood - (0.5 * num_clusters * (num_parameters_per_cluster + 1.0));
    for (cluster = 0; cluster < num_clusters; cluster++)
    {
        adjusted_log_likelihood = adjusted_log_likelihood - (0.5 * num_parameters_per_cluster * SAFE_LOG(num_points * a_vp->elements[cluster] / 12.0));
    }

    if (adjusted_log_likelihood > max_adjusted_log_likelihood)
    {
        max_adjusted_log_likelihood = adjusted_log_likelihood;

        if (a_vpp != NULL)
        {
            ERE(copy_vector(a_vpp, a_vp));
        }

        if (u_mpp != NULL)
        {
            ERE(copy_matrix(u_mpp, u_mp));
        }

        if (var_mpp != NULL)
        {
            ERE(copy_matrix(var_mpp, var_mp));
        }

        if (P_mpp != NULL)
        {
            ERE(copy_matrix(P_mpp, local_P_mp));
        }

        /* Keep track of intermediate results. */
        if (fs_write_CEM_intermediate_results == TRUE)
        {
            if ( (a_vp != NULL) && (u_mp != NULL) && (var_mp != NULL) )
            {
                ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], num_clusters, 1));
                ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], num_clusters, num_features));
                ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], num_clusters, num_features));

                intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
                intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
                intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    intermediate_a_mp->elements[cluster][0] = a_vp->elements[cluster];
                    ERE(copy_matrix_row(intermediate_u_mp, cluster, u_mp, cluster));
                    ERE(copy_matrix_row(intermediate_var_mp, cluster, var_mp, cluster));
                }

                intermediate_result_index = intermediate_result_index + 1; 
            }
        }
    }
        
    if (fs_normalize_data)
    {
        ERE(ow_un_normalize_cluster_data((u_mpp == NULL) ? NULL : *u_mpp, 
                                         (var_mpp == NULL) ? NULL : *var_mpp, 
                                         feature_mean_vp, 
                                         feature_var_vp, 
                                         norm_stdev)); 
    }

    /* Copying the final result at the end. */
    if (fs_write_CEM_intermediate_results == TRUE)
    {
        if ( (a_vpp != NULL) && (u_mpp != NULL) && (var_mpp != NULL) )
        {
            if ( (*a_vpp != NULL) && (*u_mpp != NULL) && (*var_mpp != NULL) )
            {
                ERE(get_target_matrix(&intermediate_a_mvp->elements[intermediate_result_index], (*a_vpp)->length, 1));
                ERE(get_target_matrix(&intermediate_u_mvp->elements[intermediate_result_index], (*a_vpp)->length, num_features));
                ERE(get_target_matrix(&intermediate_var_mvp->elements[intermediate_result_index], (*a_vpp)->length, num_features));

                intermediate_a_mp = intermediate_a_mvp->elements[intermediate_result_index];
                intermediate_u_mp = intermediate_u_mvp->elements[intermediate_result_index];
                intermediate_var_mp = intermediate_var_mvp->elements[intermediate_result_index];

                for (cluster = 0; cluster < (*a_vpp)->length; cluster++)
                {
                    intermediate_a_mp->elements[cluster][0] = (*a_vpp)->elements[cluster];
                    ERE(copy_matrix_row(intermediate_u_mp, cluster, *u_mpp, cluster));
                    ERE(copy_matrix_row(intermediate_var_mp, cluster, *var_mpp, cluster));
                }

                intermediate_result_index = intermediate_result_index + 1; 
            }
        }
    }

    /* Write all the intermediate and final results to disk. */
    if (fs_write_CEM_intermediate_results == TRUE)
    {
        ERE(write_matrix_vector(intermediate_a_mvp, "intermediate_a_mvp"));
        ERE(write_matrix_vector(intermediate_u_mvp, "intermediate_u_mvp"));
        ERE(write_matrix_vector(intermediate_var_mvp, "intermediate_var_mvp"));
    }
    
    free_vector(a_vp);
    free_matrix(u_mp);
    free_matrix(var_mp);
    free_matrix(local_P_mp);

    free_matrix(norm_feature_mp);
    free_vector(feature_var_vp); 
    free_vector(feature_mean_vp); 

    free_matrix(perturbed_feature_mp);

    free_int_matrix(split_merge_cluster_indices_mp);
    free_vector(split_merge_probabilities_vp);
    free_vector(split_merge_cumulative_distribution_vp);
    free_vector(log_sqrt_det_vp);
    free_vector(x_vp);
    free_matrix(log_cluster_distributions_mp);
    free_vector(weighted_empirical_distribution_vp);
    free_vector(weighted_log_empirical_distribution_vp);
    free_vector(cluster_distribution_vp);
    free_vector(log_cluster_distribution_vp);

    free_vector(split_merge_u_vp);
    free_vector(split_merge_var_vp);

    free_vector(u_1_vp);
    free_vector(u_2_vp);
    free_vector(var_1_vp);
    free_vector(var_2_vp);

    free_vector(merge_log_cluster_distribution_vp);
    free_matrix(merge_P_mp);
    free_vector(temp_vp);

    free_vector(local_initial_a_vp);
    free_matrix(local_initial_u_mp);
    free_matrix(local_initial_var_mp);
    
    free_matrix_vector(intermediate_a_mvp);
    free_matrix_vector(intermediate_u_mvp);
    free_matrix_vector(intermediate_var_mvp);

    /*
    free_matrix(intermediate_a_mp);
    free_matrix(intermediate_u_mp);
    free_matrix(intermediate_var_mp);
    */

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef NOT_CURRENTLY_USED
static int get_discrete_KL_divergence
(
    const Vector* true_distribution_vp,
    const Vector* assumed_distribution_vp,
    double*       K_L_divergence_ptr
)
{
    int num_values;
    int i;

    if (true_distribution_vp == NULL)
    {
        set_bug("True distribution vector is NULL.");
        return ERROR;
    }

    if (assumed_distribution_vp == NULL)
    {
        set_bug("Assumed distribution vector is NULL.");
        return ERROR;
    }
    
    num_values = true_distribution_vp->length;
    
    ASSERT(num_values == assumed_distribution_vp->length);

    *K_L_divergence_ptr = 0;
    
    for (i = 0; i < num_values; i++)
    {
        if (assumed_distribution_vp->elements[i] == 0)
        {
            *K_L_divergence_ptr = 1000;
            return NO_ERROR;
            
            /*
            set_bug("Support of the assumed distribution is smaller than that of the true distribution.");
            return ERROR;
            */
        }
        else if (true_distribution_vp->elements[i] == 0)
        {
            *K_L_divergence_ptr = *K_L_divergence_ptr + 0;
        }
        else
        {
            *K_L_divergence_ptr = *K_L_divergence_ptr + (true_distribution_vp->elements[i] * (SAFE_LOG(true_distribution_vp->elements[i]) - SAFE_LOG(assumed_distribution_vp->elements[i])));
        }
    }
    
    return NO_ERROR;    
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Kobus: This changes true_log_distribution_vp and assumed_log_distribution_vp
 * due to the call to normalization. 
*/
static int get_discrete_KL_divergence_log_densities
(
    Vector* true_log_distribution_vp,
    Vector* assumed_log_distribution_vp,
    double*       K_L_divergence_ptr
)
{
    int num_values;
    int i;

    Vector* true_distribution_vp = NULL;

    if (true_log_distribution_vp == NULL)
    {
        set_bug("True distribution vector is NULL.");
        return ERROR;
    }

    if (assumed_log_distribution_vp == NULL)
    {
        set_bug("Assumed distribution vector is NULL.");
        return ERROR;
    }
    
    num_values = true_log_distribution_vp->length;
    
    ASSERT(num_values == assumed_log_distribution_vp->length);

    *K_L_divergence_ptr = 0;
    
    /* Making sure the input log densities are normalized */

    /* It is only necessary to make sure the true distribution is normalized.
     * This is because the assumed distribution might be in a continuous space
     * in which case the sum of the point density values over the input points
     * need not sum to 1. */
    ERE(ow_normalize_log_prob_vp(true_log_distribution_vp));
    ERE(ow_normalize_log_prob_vp(assumed_log_distribution_vp)); 

    ERE(copy_vector(&true_distribution_vp, true_log_distribution_vp));
    ow_exp_scale_by_sum_log_vector(true_distribution_vp);

    for (i = 0; i < num_values; i++)
    {
        /* Sanity checks */
        ASSERT(true_distribution_vp->elements[i] >= 0.0);

        ASSERT_IS_NUMBER_DBL(true_log_distribution_vp->elements[i]);
        ASSERT_IS_NUMBER_DBL(assumed_log_distribution_vp->elements[i]);
    
        /*
        ASSERT_IS_FINITE_DBL(true_log_distribution_vp->elements[i]);
        */

        ASSERT_IS_FINITE_DBL(assumed_log_distribution_vp->elements[i]);

        if (assumed_log_distribution_vp->elements[i] <= DBL_HALF_MOST_NEGATIVE)
        {
            /*
            *K_L_divergence_ptr = 1000;
            return NO_ERROR;
            */

            /*
            set_bug("Support of the assumed distribution is smaller than that of the true distribution.");
            return ERROR;
            */
        }
        else if (true_distribution_vp->elements[i] == 0)
        {
            *K_L_divergence_ptr = *K_L_divergence_ptr + 0;
        }
        else
        {
            *K_L_divergence_ptr = *K_L_divergence_ptr + (true_distribution_vp->elements[i] * (true_log_distribution_vp->elements[i] - assumed_log_distribution_vp->elements[i]));
        }
    }
    
    free_vector(true_distribution_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef SKIP_FOR_EXPORT /* The following code is removed for export. */
#ifdef ENABLE_SHIFT_MODELS

/* =============================================================================
 *                 get_independent_GMM_with_shift
 *                               
 * Finds a Gaussian mixture model (GMM) for data possibly containing discrete
 * random global shifts in feature dimensions.
 *
 * This routine finds a Gaussian mixture model (GMM) for the data on the
 * assumption that the features are independent. It allows for the possibility
 * of a data point being shifted by a random discrete amount after having been
 * generated from its Gaussian. The shifts are assumed to be independent of the
 * Gaussians from which the data points are generated. Also the shifts are
 * assumed to occur with wrap-arounds. The model is fit with EM. Some features
 * are controlled via the set facility.
 *
 * In particular, it fits:
 * |         p(x) = sum sum  a-sub-i * delta-sub-j * g(u-sub-i, v-sub-i, x(- s-sub-j))
 * |                 i   j
 * where a-sub-i is the prior probability for the mixuture component (cluster),
 * u-sub-i is the mean vector for component i, v-sub-i is the variance for the
 * component, and g(u,v,x) is a Gaussian with diagonal covariance (i.e., the
 * features are assumed to be independent, given the cluster). delta-sub-j is
 * the prior probability of shift j and x(- s-sub-j) indicates a global reverse
 * (negative sign) shift of x by the amount corresponding to s-sub-j. 
 *
 * min_shift and max_shift specify the minimum and maximum amount of global
 * discrete random shift a data point can experience after being generated from
 * its Gaussian. Negative values for the shift indicate a shift in the left
 * direction w.r.t. the assumed ordering of feature dimensions and positive values
 * indicate a shift in right direction. Then the total number of possible
 * shifts for any data point is S = (max_shift - min_shift + 1).
 *
 * The argument num_clusters is the number of requested mixture components
 * (clusters), K. 
 *
 * The data matrix feature_mp is an N by M matrix where N is the number of data
 * points, and M is the number of features. 
 *
 * The model parameters are put into *delta_vpp, *a_vpp, *u_mpp, and *var_mpp. Any of
 * delta_vpp, a_vpp, u_mpp, or var_mpp is NULL if that value is not needed.
 *
 * The vector *delta_vpp contains the inferred probability distribution over
 * shifts computed using all the training data points. It is of size S. The
 * elements of *delta_vpp can be viewed as shift priors. Similarly the vector
 * *a_vpp contains the inferred cluster priors. It is of size K.
 *
 * Both u-sub-i and v-sub-i are vectors, and they are put into the i'th row of
 * *u_mpp and *var_mpp, respectively. The matrices are thus K by M. 
 * 
 * If P_cluster_mpp, is not NULL, then the soft clustering (cluster membership) for each
 * data point is returned. In that case, *P_cluster_mpp will be N by K. 
 *
 * If P_shift_mpp is not NULL, then the posterior probability distribution over
 * the possible discrete shifts for each data point is returned. In that case,
 * *P_shift_mpp will be N by S.
 *
 * Initial values of the parameters to be used as the starting values for the EM
 * iterations can be specified using initial_delta_vp, initial_a_vp,
 * initial_u_mp and initial_var_mp. If they are all NULL, then a random
 * initialization scheme is used.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with an error message being set. Otherwise NO_ERROR is returned. 
 *
 * Index: clustering, EM, GMM, shift
 *
 * Authors: Prasad Gabbur, Kobus Barnard. 
 * -----------------------------------------------------------------------------
*/

int get_independent_GMM_with_shift
(
    int           min_shift,
    int           max_shift,
    int           num_clusters,
    const Matrix* feature_mp,
    const Vector* initial_delta_vp,
    const Vector* initial_a_vp,
    const Matrix* initial_u_mp,
    const Matrix* initial_var_mp,
    Vector**      delta_vpp,
    Matrix**      P_shift_mpp,
    Vector**      a_vpp,
    Matrix**      u_mpp,
    Matrix**      var_mpp,
    Matrix**      P_cluster_mpp
)
{
    int        num_points           = feature_mp->num_rows;
    int        num_features         = feature_mp->num_cols;
    int        j;
    double     max_log_likelihood   = DBL_HALF_MOST_NEGATIVE;
    Matrix*    u_mp                 = NULL;
    Matrix*    var_mp               = NULL;
    Vector*    a_vp                 = NULL;
    Vector*    delta_vp             = NULL;
    Matrix*    P_cluster_mp         = NULL;
    Matrix*    P_shift_mp           = NULL;
    Matrix*    perturbed_feature_mp = NULL;
    double     encoding_cost;
    int        result               = NO_ERROR;
    Matrix*    norm_feature_mp      = NULL;
    Vector*    feature_mean_vp      = NULL;
    Vector*    feature_var_vp       = NULL;
    const double norm_stdev         = 1.0;
    Matrix*    cropped_feature_mp   = NULL;
    int        cropped_row_offset;
    int        cropped_col_offset;
    int        cropped_num_rows;
    int        cropped_num_cols;

    UNTESTED_CODE();  /* Since start of changes towards industrial version. */

    if (fs_crop_feature_dimensions)
    {
        cropped_row_offset = 0;
        cropped_col_offset = fs_crop_num_feature_dimensions_left;

        cropped_num_rows = num_points;
        cropped_num_cols = num_features - (fs_crop_num_feature_dimensions_left + fs_crop_num_feature_dimensions_right);

        result = copy_matrix_block(&cropped_feature_mp,
                                   feature_mp,
                                   cropped_row_offset,
                                   cropped_col_offset,
                                   cropped_num_rows,
                                   cropped_num_cols);

        if (result != ERROR)
        {
            feature_mp = cropped_feature_mp;
        }
    }

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
        result = get_independent_GMM_with_shift_3(min_shift,
                                                  max_shift,
                                                  num_clusters, 
                                                  feature_mp,
                                                  (Int_vector*) NULL,
                                                  initial_delta_vp,
                                                  initial_a_vp,
                                                  initial_u_mp,
                                                  initial_var_mp,
                                                  delta_vpp,
                                                  P_shift_mpp,
                                                  a_vpp,
                                                  u_mpp,
                                                  var_mpp, 
                                                  P_cluster_mpp, 
                                                  (double*) NULL,
                                                  (double*) NULL,
                                                  (int*)    NULL); 
    }
    else
    {
        UNTESTED_CODE(); 

        for (j = 0; j < fs_num_tries_per_cluster_count; j++)
        {
            double log_likelihood;

            if (result == ERROR) { NOTE_ERROR(); break; }

            num_clusters = get_independent_GMM_with_shift_3(min_shift,
                                                            max_shift,
                                                            num_clusters, 
                                                            feature_mp,
                                                            (Int_vector*) NULL,
                                                            initial_delta_vp,
                                                            initial_a_vp,
                                                            initial_u_mp,
                                                            initial_var_mp,
                                                            (delta_vpp == NULL) ? NULL : &delta_vp,
                                                            (P_shift_mpp == NULL) ? NULL : &P_shift_mp,
                                                            (a_vpp == NULL) ? NULL : &a_vp,
                                                            (u_mpp == NULL) ? NULL : &u_mp,
                                                            (var_mpp == NULL) ? NULL : & var_mp, 
                                                            (P_cluster_mpp == NULL) ? NULL : & P_cluster_mp, 
                                                            &log_likelihood,
                                                            (double*) NULL,
                                                            (int*)    NULL); 

            if (num_clusters == ERROR)
            {
                result = ERROR;
                break; 
            }

            verbose_pso(2, "Raw log likelihood with %d clusters is %.5e,\n", 
                        num_clusters, log_likelihood);

            if (fs_tie_var == TRUE)
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
            encoding_cost *= log( (double) num_points*num_features ); 

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
                                                                                                    
                if ((result != ERROR) && (delta_vpp != NULL))
                {
                    result = copy_vector(delta_vpp, delta_vp); 
                }

                if ((result != ERROR) && (P_cluster_mpp != NULL))
                {
                    result = copy_matrix(P_cluster_mpp, P_cluster_mp); 
                }

                if ((result != ERROR) && (P_shift_mpp != NULL))
                {
                    result = copy_matrix(P_shift_mpp, P_shift_mp); 
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
    free_vector(delta_vp);
    free_vector(a_vp);
    free_matrix(u_mp);
    free_matrix(var_mp);
    free_matrix(P_shift_mp);
    free_matrix(P_cluster_mp);
    free_matrix(perturbed_feature_mp);
    free_matrix(cropped_feature_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                 get_independent_GMM_with_shift_2
 *                               
 * Finds a Gaussian mixture model (GMM) for data possibly containing discrete
 * random global shifts in feature dimensions.
 *
 * This routine finds a Gaussian mixture model (GMM) for the data on the
 * assumption that the features are independent. It allows for the possibility
 * of a data point being shifted by a random discrete amount after having been
 * generated from its Gaussian. The shifts are assumed to be independent of the
 * Gaussians from which the data points are generated. Unlike the counterpart
 * routine, the shifts are not necessarily assumed to occur with wrap-arounds.
 * Instead, the shifts could result in any arbitrary values into the feature
 * dimensions that free up due to the shift. The model is fit with EM. Some
 * features are controlled via the set facility.
 *
 * This routine performs subspace clustering in that it considers only a subset
 * of feature dimensions that are guaranteed to be not prone to corruption by
 * noise due to the assumed nature of shift. The subspace of feature dimensions
 * is determined by the max_left_shift and max_right_shift parameters as
 * explained below.
 *
 * In particular, it fits:
 * |         p(x) = sum sum  a-sub-i * delta-sub-j * g(u-sub-i, v-sub-i, x(- s-sub-j))
 * |                 i   j
 * where a-sub-i is the prior probability for the mixuture component (cluster),
 * u-sub-i is the mean vector for component i, v-sub-i is the variance for the
 * component, and g(u,v,x) is a Gaussian with diagonal covariance (i.e., the
 * features are assumed to be independent, given the cluster). delta-sub-j is
 * the prior probability of shift j and x(- s-sub-j) indicates a global reverse
 * (negative sign) shift of x by the amount corresponding to s-sub-j. 
 *
 * max_left_shift and max_right_shift specify the maximum amount of global
 * discrete random left and right shift respectively a data point can experience
 * after being generated from its Gaussian. Unlike the counterpart routine, each
 * of these parameters can have only non-negative values. The total number of
 * possible shifts for any data point is S = (max_left_shift + max_right_shift + 1) 
 * including the zero shift.
 *
 * Based on max_left_shift and max_right_shift, a subspace of the entire
 * feature space exists that is guaranteed to be unaffected by the arbitrary
 * noise that a random shift introduces. It is of dimension 
 * T = M - (max_left_shift + max_right_shift), where M is the dimensionality of
 * the full feature space. So, the EM procedure determines clusters in this
 * subspace rather than the full space. 
 *  
 * The argument num_clusters is the number of requested mixture components
 * (clusters), K. 
 *
 * The data matrix feature_mp is an N by M matrix where N is the number of data
 * points, and M is the number of features. 
 *
 * The model parameters are put into *delta_vpp, *a_vpp, *u_mpp, and *var_mpp. Any of
 * delta_vpp, a_vpp, u_mpp, or var_mpp is NULL if that value is not needed.
 *
 * The vector *delta_vpp contains the inferred probability distribution over
 * shifts computed using all the training data points. It is of size S. The
 * elements of *delta_vpp can be viewed as shift priors. The assumed order of
 * shifts in this vector or any other output pertaining to shifts is:
 * (max_left_shift, max_left_shift-1,...., 0,...., max_right_shift-1, max_right_shift)
 *
 * The vector *a_vpp contains the inferred cluster priors. It is of size K.
 *
 * Both u-sub-i and v-sub-i are vectors, and they are put into the i'th row of
 * *u_mpp and *var_mpp, respectively. The matrices are thus K by T. 
 * 
 * If P_cluster_mpp, is not NULL, then the soft clustering (cluster membership) for each
 * data point is returned. In that case, *P_cluster_mpp will be N by K. 
 *
 * If P_shift_mpp is not NULL, then the posterior probability distribution over
 * the possible discrete shifts for each data point is returned. In that case,
 * *P_shift_mpp will be N by S.
 *
 * Initial values of the parameters to be used as the starting values for the EM
 * iterations can be specified using initial_delta_vp, initial_a_vp,
 * initial_u_mp and initial_var_mp.  If they are all NULL, then a
 * random initialization scheme is used. It is assumed that the initial
 * parameters are specified either in the full feature space or the reduced
 * space in which the final clusters are sought. In case of full space, the
 * routine retrieves the parameters corresponding to the target subspace.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with an error message being set. Otherwise NO_ERROR is returned. 
 *
 * Index: clustering, EM, GMM, shift
 *
 * Authors: Prasad Gabbur, Kobus Barnard. 
 * -----------------------------------------------------------------------------
*/

int get_independent_GMM_with_shift_2
(
    int           max_left_shift,
    int           max_right_shift,
    int           num_clusters,
    const Matrix* feature_mp,
    const Vector* initial_delta_vp,
    const Vector* initial_a_vp,
    const Matrix* initial_u_mp,
    const Matrix* initial_var_mp,
    Vector**      delta_vpp,
    Matrix**      P_shift_mpp,
    Vector**      a_vpp,
    Matrix**      u_mpp,
    Matrix**      var_mpp,
    Matrix**      P_cluster_mpp
)
{
    int        num_points           = feature_mp->num_rows;
    int        num_features         = feature_mp->num_cols;
    int        j;
    double     max_log_likelihood   = DBL_HALF_MOST_NEGATIVE;
    Matrix*    u_mp                 = NULL;
    Matrix*    var_mp               = NULL;
    Vector*    a_vp                 = NULL;
    Vector*    delta_vp             = NULL;
    Matrix*    P_cluster_mp         = NULL;
    Matrix*    P_shift_mp           = NULL;
    Matrix*    perturbed_feature_mp = NULL;
    double     encoding_cost;
    int        result               = NO_ERROR;
    Matrix*    norm_feature_mp      = NULL;
    Vector*    feature_mean_vp      = NULL;
    Vector*    feature_var_vp       = NULL;
    const double norm_stdev         = 1.0; 
    Matrix*    cropped_feature_mp   = NULL;
    int        cropped_row_offset;
    int        cropped_col_offset;
    int        cropped_num_rows;
    int        cropped_num_cols;

    UNTESTED_CODE();  /* Since start of changes towards industrial version. */

    if (fs_crop_feature_dimensions)
    {
        cropped_row_offset = 0;
        cropped_col_offset = fs_crop_num_feature_dimensions_left;

        cropped_num_rows = num_points;
        cropped_num_cols = num_features - (fs_crop_num_feature_dimensions_left + fs_crop_num_feature_dimensions_right);

        result = copy_matrix_block(&cropped_feature_mp,
                                   feature_mp,
                                   cropped_row_offset,
                                   cropped_col_offset,
                                   cropped_num_rows,
                                   cropped_num_cols);

        if (result != ERROR)
        {
            feature_mp = cropped_feature_mp;
        }
    }

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
        result = get_independent_GMM_with_shift_4(max_left_shift,
                                                  max_right_shift,
                                                  num_clusters, 
                                                  feature_mp,
                                                  (Int_vector*) NULL,
                                                  initial_delta_vp,
                                                  initial_a_vp,
                                                  initial_u_mp,
                                                  initial_var_mp,
                                                  delta_vpp,
                                                  P_shift_mpp,
                                                  a_vpp,
                                                  u_mpp,
                                                  var_mpp, 
                                                  P_cluster_mpp, 
                                                  (double*) NULL,
                                                  (double*) NULL,
                                                  (int*)    NULL); 
    }
    else
    {
        UNTESTED_CODE(); 

        for (j = 0; j < fs_num_tries_per_cluster_count; j++)
        {
            double log_likelihood;

            if (result == ERROR) { NOTE_ERROR(); break; }

            num_clusters = get_independent_GMM_with_shift_4(max_left_shift,
                                                            max_right_shift,
                                                            num_clusters, 
                                                            feature_mp,
                                                            (Int_vector*) NULL,
                                                            initial_delta_vp,
                                                            initial_a_vp,
                                                            initial_u_mp,
                                                            initial_var_mp,
                                                            (delta_vpp == NULL) ? NULL : &delta_vp,
                                                            (P_shift_mpp == NULL) ? NULL : &P_shift_mp,
                                                            (a_vpp == NULL) ? NULL : &a_vp,
                                                            (u_mpp == NULL) ? NULL : &u_mp,
                                                            (var_mpp == NULL) ? NULL : & var_mp, 
                                                            (P_cluster_mpp == NULL) ? NULL : & P_cluster_mp, 
                                                            &log_likelihood,
                                                            (double*) NULL,
                                                            (int*)    NULL); 

            if (num_clusters == ERROR)
            {
                result = ERROR;
                break; 
            }

            verbose_pso(2, "Raw log likelihood with %d clusters is %.5e,\n", 
                        num_clusters, log_likelihood);

            if (fs_tie_var == TRUE)
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
            encoding_cost *= log( (double) num_points*num_features ); 

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
                                                                                                    
                if ((result != ERROR) && (delta_vpp != NULL))
                {
                    result = copy_vector(delta_vpp, delta_vp); 
                }

                if ((result != ERROR) && (P_cluster_mpp != NULL))
                {
                    result = copy_matrix(P_cluster_mpp, P_cluster_mp); 
                }

                if ((result != ERROR) && (P_shift_mpp != NULL))
                {
                    result = copy_matrix(P_shift_mpp, P_shift_mp); 
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
    free_vector(delta_vp);
    free_vector(a_vp);
    free_matrix(u_mp);
    free_matrix(var_mp);
    free_matrix(P_shift_mp);
    free_matrix(P_cluster_mp);
    free_matrix(perturbed_feature_mp);
    free_matrix(cropped_feature_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_independent_GMM_with_shift_3
(
    int           min_shift,
    int           max_shift,
    int           num_clusters,
    const Matrix* feature_mp,
    const Int_vector* held_out_indicator_vp,
    const Vector* initial_delta_vp,
    const Vector* initial_a_vp,
    const Matrix* initial_means_mp,
    const Matrix* initial_var_mp,
    Vector**      delta_vpp,
    Matrix**      P_shift_mpp,
    Vector**      a_vpp, 
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_cluster_mpp,
    double*       log_likelihood_ptr,
    double*       held_out_log_likelihood_ptr,
    int*          num_iterations_ptr
)
{
    int num_points   = feature_mp->num_rows;
    int num_features = feature_mp->num_cols;
    int num_shifts   = max_shift - min_shift + 1;
   
    int           shift;
    int           negative_shift;
    int           cluster;
    int           feature;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double        var_offset = 0.0;
#else
    double        var_offset = fs_var_offset; 
#endif 
    int           it, i;
    Matrix*       log_cluster_shift_density_mp = NULL;
    Vector*       I_shift_vp   = NULL;
    Vector*       I_cluster_vp          = NULL;
    Vector*       delta_vp = NULL;
    Vector*       p_sum_shift_vp = NULL;
    Vector*       a_vp        = NULL;
    Vector*       p_sum_cluster_vp    = NULL;
    Matrix*       var_mp        = NULL;
    Matrix*       new_var_mp        = NULL;
    Matrix*       new_u_mp        = NULL;
    Vector*       x_vp        = NULL;
    Vector*       shifted_x_vp = NULL;
    Vector*       shifted_x2_vp        = NULL;
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
    Matrix*       I_shift_mp          = NULL;
    Matrix*       I_cluster_mp        = NULL;
#endif 
    Vector*       temp_log_prob_vp = NULL;
#ifdef DONT_USE_GAUSS_ROUTINE
    double* x_ptr;
    double* u_ptr;
    double* v_ptr;
    double temp, dev;
    double d;
#endif
    double log_delta;
    double log_a;

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    ERE(get_cluster_random_matrix(&I_shift_mp, num_points, num_shifts));
    ERE(get_cluster_random_matrix(&I_cluster_mp, num_points, num_clusters));
#endif 

    if (P_shift_mpp != NULL)
    {
        ERE(get_target_matrix(P_shift_mpp, num_points, num_shifts));
    }
    
    if (P_cluster_mpp != NULL)
    {
        ERE(get_target_matrix(P_cluster_mpp, num_points, num_clusters));
    }

    if (held_out_indicator_vp != NULL)
    {
        if (held_out_indicator_vp->length != num_points)
        {
            result = ERROR;
        }
    }
            
    if (    (get_target_matrix(&log_cluster_shift_density_mp, num_clusters, num_shifts) == ERROR)
         || (get_target_vector(&I_shift_vp, num_shifts) == ERROR)
         || (get_target_vector(&I_cluster_vp, num_clusters) == ERROR)
         || (get_target_vector(&delta_vp, num_shifts) == ERROR)
         || (get_target_vector(&a_vp, num_clusters) == ERROR) 
#ifdef DONT_USE_GAUSS_ROUTINE
         || (get_target_vector(&log_sqrt_det_vp, num_clusters) == ERROR) 
#endif 
       )
    {
        result = ERROR;
    }

    dbi(num_shifts);
    dbi(num_clusters); 

    for (it = 0; it < fs_max_num_iterations; it++)
    {

        log_likelihood = 0.0;

        held_out_log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_shift_vp, num_shifts);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_cluster_vp, num_clusters);
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

            if (initial_delta_vp != NULL)
            {
                ERE(get_target_vector(&delta_vp, num_shifts));
                for (i = 0; i < num_shifts; i++)
                {
                    delta_vp->elements[i] = initial_delta_vp->elements[i];
                }
                ERE(ow_normalize_vector(delta_vp, NORMALIZE_BY_SUM));
            }
            else
            {
                set_bug("Initial shift priors vector is NULL.\n");
                return ERROR;
            }
                
#ifdef DONT_USE_GAUSS_ROUTINE
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                temp = 0.0;

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

        for (i = 0; i < num_points; i++)
        {

            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_matrix_row(&x_vp, feature_mp, i);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors != TRUE) )
            {
                /* E Step init */

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
                result = get_matrix_row(&I_shift_vp, I_shift_mp, i);
                result = get_matrix_row(&I_cluster_vp, I_cluster_mp, i);
#else
                result = get_target_vector(&I_shift_vp, num_shifts);
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                for (shift = 0; shift < num_shifts; shift++)
                {
                    double r = kjb_rand();
                    double p = pow(r, 5.0); /* Perhaps should be an option. */

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    I_shift_vp->elements[ shift ] = p;

                    if (kjb_rand() < 1.0 / num_shifts) 
                    {
                        I_shift_vp->elements[ shift ] += 0.5; 
                    }
                }

                result = ow_scale_vector_by_sum(I_shift_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_to_vector(I_shift_vp, 0.2 * kjb_rand() / num_shifts); 
                if (result == ERROR) { NOTE_ERROR(); break; }
 

                result = get_target_vector(&I_cluster_vp, num_clusters);
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double r = kjb_rand();
                    double p = pow(r, 5.0); /* Perhaps should be an option. */

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    I_cluster_vp->elements[ cluster ] = p;

                    if (kjb_rand() < 1.0 / num_clusters) 
                    {
                        I_cluster_vp->elements[ cluster ] += 0.5; 
                    }
                }

                result = ow_scale_vector_by_sum(I_cluster_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_to_vector(I_cluster_vp, 0.2 * kjb_rand() / num_clusters); 
                if (result == ERROR) { NOTE_ERROR(); break; }
#endif 

                result = ow_scale_vector_by_sum(I_shift_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                result = ow_scale_vector_by_sum(I_cluster_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }

            if ( (it > 0) || (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
            {
                /* E Step */

                for (cluster = 0; cluster < num_clusters; cluster++)
                {                    
                    /*double log_a = SAFE_LOG(a_vp->elements[ cluster ]);*/
                 
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (shift = 0; shift < num_shifts; shift++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        negative_shift = -(min_shift + shift);
                        
                        result = shift_point_with_extension(&shifted_x_vp, x_vp, negative_shift);
                        if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef DONT_USE_GAUSS_ROUTINE
                        x_ptr = shifted_x_vp->elements;
                        u_ptr = u_mp->elements[ cluster ];
                        v_ptr = var_mp->elements[ cluster ];
                        
                        /*
                        int feature; 
                        double temp, dev;
                        */
                        d = 0.0;
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

                        log_cluster_shift_density_mp->elements[ cluster ][ shift ]  = - (0.5*d) - log_sqrt_det_vp->elements[ cluster ];
                        
                        /*
                        I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ];
                        */ 
#else
                        result = get_log_gaussian_density(shifted_x_vp, u_vp, var_vp, 
                                                          &(log_cluster_shift_density_mp->elements[ cluster ][ shift ]));

                        /*
                        result = get_log_gaussian_density(x_vp, u_vp, var_vp, 
                                                        &(I_vp->elements[ cluster ]));
                        */
                        if (result == ERROR) { NOTE_ERROR(); break; }
                        
                        /*
                        I_vp->elements[ cluster ] += log_a; 
                        */
#endif 

#ifndef DONT_USE_GAUSS_ROUTINE
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS_LL

                        log_cluster_shift_density_mp->elements[ cluster ][ shift ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
                        /*
                        I_vp->elements[ cluster ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
                        */
#endif 
#endif 
                    } /* for (shift = 0; shift < num_shifts; shift++) */

                } /* for (cluster = 0; cluster < num_clusters; cluster++) */

                for (shift = 0; shift < num_shifts; shift++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    log_delta = SAFE_LOG(delta_vp->elements[ shift ]);

                    result = get_target_vector(&temp_log_prob_vp, num_clusters);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (cluster = 0; cluster < num_clusters; cluster++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        log_a = SAFE_LOG(a_vp->elements[ cluster ]);

                        temp_log_prob_vp->elements[ cluster ] = log_a + log_cluster_shift_density_mp->elements[ cluster ][ shift ]; 
                    }

                    I_shift_vp->elements[ shift ]  = ow_exp_scale_by_sum_log_vector(temp_log_prob_vp);
                    I_shift_vp->elements[ shift ] += log_delta;
                }

                /* The following normalization is necessary but the resulting likelihood is not useful (?) */
                temp = ow_exp_scale_by_sum_log_vector(I_shift_vp);

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }
 
                    log_a = SAFE_LOG(a_vp->elements[ cluster ]);

                    result = get_target_vector(&temp_log_prob_vp, num_shifts);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (shift = 0; shift < num_shifts; shift++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        log_delta = SAFE_LOG(delta_vp->elements[ shift ]);

                        temp_log_prob_vp->elements[ shift ] = log_delta + log_cluster_shift_density_mp->elements[ cluster ][ shift ]; 
                    }

                    I_cluster_vp->elements[ cluster ]  = ow_exp_scale_by_sum_log_vector(temp_log_prob_vp);
                    I_cluster_vp->elements[ cluster ] += log_a;
                }

                if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
                {
                    held_out_log_likelihood += ow_exp_scale_by_sum_log_vector(I_cluster_vp);
                }
                else
                {
                    log_likelihood += ow_exp_scale_by_sum_log_vector(I_cluster_vp);
                }
            }

            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_shift_vp), 1.0, 0.00001);
            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_cluster_vp), 1.0, 0.00001);

            if (result == ERROR) { NOTE_ERROR(); break; }

            if (P_shift_mpp != NULL)
            {
                result = put_matrix_row(*P_shift_mpp, I_shift_vp, i);
            }

            if (P_cluster_mpp != NULL)
            {
                result = put_matrix_row(*P_cluster_mpp, I_cluster_vp, i);
            }

            /* M-Step */
            
            if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
            {
                /* Do not do the M-step for held out points */
            }
            else
            {
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double  p_cluster = I_cluster_vp->elements[ cluster ];

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (shift = 0; shift < num_shifts; shift++)
                    {
                        double p_shift     = I_shift_vp->elements[ shift ];
                        double p           = p_cluster * p_shift;

                        negative_shift     = -(min_shift + shift);
                    
                        if (result == ERROR) { NOTE_ERROR(); break; }
                        
                        result = shift_point_with_extension(&shifted_x_vp, x_vp, negative_shift);
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        result = multiply_vectors(&shifted_x2_vp, shifted_x_vp, shifted_x_vp);
                        if (result == ERROR) { NOTE_ERROR(); break; } 
                        
                        result = ow_add_scalar_times_vector_to_matrix_row(new_u_mp, 
                                                                          shifted_x_vp, p,
                                                                          cluster);
                        if (result == ERROR) { NOTE_ERROR(); break; } 

                        result = ow_add_scalar_times_vector_to_matrix_row(new_var_mp, 
                                                                          shifted_x2_vp, p,
                                                                          cluster);
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        p_sum_shift_vp->elements[ shift ] += p_shift;
                    }

                    p_sum_cluster_vp->elements[ cluster ] += p_cluster; 
                }
            }

        } /* for (i = 0; i < num_points; i++) */

        /* M-Step cleanup */

        if (divide_matrix_by_col_vector(&u_mp, new_u_mp, p_sum_cluster_vp) == ERROR)
        {
            db_rv(p_sum_cluster_vp); 
            result = ERROR;
            break; 
        }

        result = get_initialized_matrix(&var_mp, num_clusters, num_features, 
                                        DBL_NOT_SET);
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double s = p_sum_cluster_vp->elements[ cluster ];

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

            /* Still need to figure out the expression for unbiased estimate in this case.
            if (fs_use_unbiased_var_estimate_in_M_step == TRUE)
            {
                double p_square_sum      = p_square_sum_vp->elements[ cluster ];
                double norm_p_square_sum = p_square_sum / (s * s);
                double norm_factor       = 1.0 / (1.0 - norm_p_square_sum);

                result = ow_multiply_matrix_row_by_scalar(var_mp, norm_factor, cluster);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            */

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
            temp = 0.0;

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

        result = scale_vector_by_sum(&delta_vp, p_sum_shift_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; } 
        
        result = scale_vector_by_sum(&a_vp, p_sum_cluster_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; } 

        if (it > 0) 
        {
            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood +  prev_log_likelihood));

            verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e\n", 
                        it + 1, log_likelihood, diff); 


            held_out_diff = held_out_log_likelihood - prev_held_out_log_likelihood;

            held_out_diff *= 2.0;
            held_out_diff /= (ABS_OF(held_out_log_likelihood + prev_held_out_log_likelihood));
            
            verbose_pso(3, "%-3d: Held out log likelihood is %12e  |  %10e\n",
                        it + 1, held_out_log_likelihood, held_out_diff); 

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

    } /* for (it = 0; it < fs_max_num_iterations; it++) */

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

        if (delta_vpp != NULL) 
        {
            result = copy_vector(delta_vpp, delta_vp);
        }
    }
    
    if ((result != ERROR) && (a_vpp != NULL)) 
    {
        result = copy_vector(a_vpp, a_vp);
    }

    if ((result != ERROR) && (var_mpp != NULL))
    {
        result = copy_matrix(var_mpp, var_mp);
    }

    if ((result != ERROR) && (means_mpp != NULL))
    {
        result = copy_matrix(means_mpp, u_mp);
    }

    free_matrix(log_cluster_shift_density_mp);
    free_vector(I_shift_vp);
    free_vector(I_cluster_vp);
    free_vector(p_sum_shift_vp);
    free_vector(p_sum_cluster_vp);
    free_vector(delta_vp);
    free_vector(a_vp);
    free_matrix(new_u_mp); 
    free_matrix(u_mp); 
    free_matrix(new_var_mp); 
    free_matrix(var_mp); 
#ifdef DONT_USE_GAUSS_ROUTINE
    free_vector(log_sqrt_det_vp);
#endif 

    free_vector(shifted_x2_vp); 
    free_vector(x_vp); 
    free_vector(shifted_x_vp);
    free_vector(var_vp); 
    free_vector(u_vp); 

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    free_matrix(I_shift_mp);
    free_matrix(I_cluster_mp);
#endif
    
    free_vector(temp_log_prob_vp);

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

static int get_independent_GMM_with_shift_4
(
    int           max_left_shift,
    int           max_right_shift,
    int           num_clusters,
    const Matrix* feature_mp,
    const Int_vector* held_out_indicator_vp,
    const Vector* initial_delta_vp,
    const Vector* initial_a_vp,
    const Matrix* initial_means_mp,
    const Matrix* initial_var_mp,
    Vector**      delta_vpp,
    Matrix**      P_shift_mpp,
    Vector**      a_vpp, 
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_cluster_mpp,
    double*       log_likelihood_ptr,
    double*       held_out_log_likelihood_ptr,
    int*          num_iterations_ptr
)
{
    int num_points         = feature_mp->num_rows;
    int total_num_features = feature_mp->num_cols;
    int num_features       = total_num_features - (max_left_shift + max_right_shift);
    int num_shifts         = max_left_shift + max_right_shift + 1;
   
    int           shift;
    /*int           negative_shift;*/
    int           cluster;
    int           feature;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double        var_offset = 0.0;
#else
    double        var_offset = fs_var_offset; 
#endif 
    int           it, i, j, k;
    Matrix*       log_cluster_shift_density_mp = NULL;
    Vector*       I_shift_vp   = NULL;
    Vector*       I_cluster_vp          = NULL;
    Vector*       delta_vp = NULL;
    Vector*       p_sum_shift_vp = NULL;
    Vector*       a_vp        = NULL;
    Vector*       p_sum_cluster_vp    = NULL;
    Matrix*       var_mp        = NULL;
    Matrix*       new_var_mp        = NULL;
    Matrix*       new_u_mp        = NULL;
    Vector*       x_vp        = NULL;
    Vector*       shifted_x_vp = NULL;
    Vector*       shifted_x2_vp        = NULL;
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
    Matrix*       I_shift_mp          = NULL;
    Matrix*       I_cluster_mp        = NULL;
#endif 
    Vector*       temp_log_prob_vp = NULL;
#ifdef DONT_USE_GAUSS_ROUTINE
    double* x_ptr;
    double* u_ptr;
    double* v_ptr;
    double temp, dev;
    double d;
#endif
    double log_delta;
    double log_a;

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    ERE(get_cluster_random_matrix(&I_shift_mp, num_points, num_shifts));
    ERE(get_cluster_random_matrix(&I_cluster_mp, num_points, num_clusters));
#endif 

    if (max_left_shift >= total_num_features)
    {
        set_bug("Maximum left shift is greater than or equal to feature dimensionality.\n");
        return ERROR;
    }

    if (max_right_shift >= total_num_features)
    {
        set_bug("Maximum right shift is greater than or equal to feature dimensionality.\n");
        return ERROR;
    }
   
    if ((max_left_shift + max_right_shift) >= total_num_features)
    {
        set_bug("Total number of shifts is greater than or equal to feature dimensionality.\n");
        return ERROR;
    }

    if (P_shift_mpp != NULL)
    {
        ERE(get_target_matrix(P_shift_mpp, num_points, num_shifts));
    }
    
    if (P_cluster_mpp != NULL)
    {
        ERE(get_target_matrix(P_cluster_mpp, num_points, num_clusters));
    }

    if (held_out_indicator_vp != NULL)
    {
        if (held_out_indicator_vp->length != num_points)
        {
            result = ERROR;
        }
    }
            
    if (    (get_target_matrix(&log_cluster_shift_density_mp, num_clusters, num_shifts) == ERROR)
         || (get_target_vector(&I_shift_vp, num_shifts) == ERROR)
         || (get_target_vector(&I_cluster_vp, num_clusters) == ERROR)
         || (get_target_vector(&delta_vp, num_shifts) == ERROR)
         || (get_target_vector(&a_vp, num_clusters) == ERROR) 
#ifdef DONT_USE_GAUSS_ROUTINE
         || (get_target_vector(&log_sqrt_det_vp, num_clusters) == ERROR) 
#endif 
       )
    {
        result = ERROR;
    }

    dbi(num_shifts);
    dbi(num_clusters); 

    for (it = 0; it < fs_max_num_iterations; it++)
    {

        log_likelihood = 0.0;

        held_out_log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_shift_vp, num_shifts);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_cluster_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_u_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_var_mp, num_clusters, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
                
        if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
        {
            if (initial_means_mp != NULL)
            {  
                if (initial_means_mp->num_cols == num_features)
                {
                    ERE(get_target_matrix(&u_mp, num_clusters, num_features));
                    for (i = 0; i < num_clusters; i++)
                    {
                        ERE(copy_matrix_row(u_mp, i, initial_means_mp, i));
                    }
                }
                else if (initial_means_mp->num_cols == total_num_features)
                {
                    ERE(get_target_matrix(&u_mp, num_clusters, num_features));
                    for (i = 0; i < num_clusters; i++)
                    {
                        for (j = max_left_shift, k= 0; j < (total_num_features - max_right_shift); j++, k++)
                        {
                            u_mp->elements[i][k] = initial_means_mp->elements[i][j];
                        }
                    }
                }
                else
                {
                    set_bug("Initial cluster means matrix is of wrong dimensions.\n");
                    return ERROR;
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
                if (initial_var_mp->num_cols == num_features)
                {
                    ERE(get_target_matrix(&var_mp, num_clusters, num_features));
                    for (i = 0; i < num_clusters; i++)
                    {
                        ERE(copy_matrix_row(var_mp, i, initial_var_mp, i));
                    }
                }
                else if (initial_var_mp->num_cols == total_num_features)
                {
                    ERE(get_target_matrix(&var_mp, num_clusters, num_features));
                    for (i = 0; i < num_clusters; i++)
                    {
                        for (j = max_left_shift, k= 0; j < (total_num_features - max_right_shift); j++, k++)
                        {
                            var_mp->elements[i][k] = initial_var_mp->elements[i][j];
                        }
                    }
                }
                else
                {
                    set_bug("Initial cluster variances matrix is of wrong dimensions.\n");
                    return ERROR;
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

            if (initial_delta_vp != NULL)
            {
                ERE(get_target_vector(&delta_vp, num_shifts));
                for (i = 0; i < num_shifts; i++)
                {
                    delta_vp->elements[i] = initial_delta_vp->elements[i];
                }
                ERE(ow_normalize_vector(delta_vp, NORMALIZE_BY_SUM));
            }
            else
            {
                set_bug("Initial shift priors vector is NULL.\n");
                return ERROR;
            }
                
#ifdef DONT_USE_GAUSS_ROUTINE
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                temp = 0.0;

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

        for (i = 0; i < num_points; i++)
        {

            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_matrix_row(&x_vp, feature_mp, i);
            if (result == ERROR) { NOTE_ERROR(); break; } 

            if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors != TRUE) )
            {
                /* E Step init */

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
                result = get_matrix_row(&I_shift_vp, I_shift_mp, i);
                result = get_matrix_row(&I_cluster_vp, I_cluster_mp, i);
#else
                result = get_target_vector(&I_shift_vp, num_shifts);
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                for (shift = 0; shift < num_shifts; shift++)
                {
                    double r = kjb_rand();
                    double p = pow(r, 5.0); /* Perhaps should be an option. */

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    I_shift_vp->elements[ shift ] = p;

                    if (kjb_rand() < 1.0 / num_shifts) 
                    {
                        I_shift_vp->elements[ shift ] += 0.5; 
                    }
                }

                result = ow_scale_vector_by_sum(I_shift_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_to_vector(I_shift_vp, 0.2 * kjb_rand() / num_shifts); 
                if (result == ERROR) { NOTE_ERROR(); break; }
 

                result = get_target_vector(&I_cluster_vp, num_clusters);
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double r = kjb_rand();
                    double p = pow(r, 5.0); /* Perhaps should be an option. */

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    I_cluster_vp->elements[ cluster ] = p;

                    if (kjb_rand() < 1.0 / num_clusters) 
                    {
                        I_cluster_vp->elements[ cluster ] += 0.5; 
                    }
                }

                result = ow_scale_vector_by_sum(I_cluster_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_add_scalar_to_vector(I_cluster_vp, 0.2 * kjb_rand() / num_clusters); 
                if (result == ERROR) { NOTE_ERROR(); break; }
#endif 

                result = ow_scale_vector_by_sum(I_shift_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
                
                result = ow_scale_vector_by_sum(I_cluster_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }

            if ( (it > 0) || (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
            {
                /* E Step */

                for (cluster = 0; cluster < num_clusters; cluster++)
                {                    
                    /*double log_a = SAFE_LOG(a_vp->elements[ cluster ]);*/
                 
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (shift = 0; shift < num_shifts; shift++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        /*
                        negative_shift = -(min_shift + shift);
                        
                        result = shift_point_with_extension(&shifted_x_vp, x_vp, negative_shift);
                        if (result == ERROR) { NOTE_ERROR(); break; }
                        */

                        ERE(get_target_vector(&shifted_x_vp, num_features));
                        for (feature = 0; feature < num_features; feature++)
                        {
                            shifted_x_vp->elements[feature] = x_vp->elements[feature + shift];
                        }

#ifdef DONT_USE_GAUSS_ROUTINE
                        x_ptr = shifted_x_vp->elements;
                        u_ptr = u_mp->elements[ cluster ];
                        v_ptr = var_mp->elements[ cluster ];
                        
                        /*
                        int feature; 
                        double temp, dev;
                        */
                        d = 0.0;
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

                        log_cluster_shift_density_mp->elements[ cluster ][ shift ]  = - (0.5*d) - log_sqrt_det_vp->elements[ cluster ];
                        
                        /*
                        I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ];
                        */ 
#else
                        result = get_log_gaussian_density(shifted_x_vp, u_vp, var_vp, 
                                                          &(log_cluster_shift_density_mp->elements[ cluster ][ shift ]));

                        /*
                        result = get_log_gaussian_density(x_vp, u_vp, var_vp, 
                                                        &(I_vp->elements[ cluster ]));
                        */
                        if (result == ERROR) { NOTE_ERROR(); break; }
                        
                        /*
                        I_vp->elements[ cluster ] += log_a; 
                        */
#endif 

#ifndef DONT_USE_GAUSS_ROUTINE
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS_LL

                        log_cluster_shift_density_mp->elements[ cluster ][ shift ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
                        /*
                        I_vp->elements[ cluster ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
                        */
#endif 
#endif 
                    } /* for (shift = 0; shift < num_shifts; shift++) */

                } /* for (cluster = 0; cluster < num_clusters; cluster++) */

                for (shift = 0; shift < num_shifts; shift++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    log_delta = SAFE_LOG(delta_vp->elements[ shift ]);

                    result = get_target_vector(&temp_log_prob_vp, num_clusters);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (cluster = 0; cluster < num_clusters; cluster++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        log_a = SAFE_LOG(a_vp->elements[ cluster ]);

                        temp_log_prob_vp->elements[ cluster ] = log_a + log_cluster_shift_density_mp->elements[ cluster ][ shift ]; 
                    }

                    I_shift_vp->elements[ shift ]  = ow_exp_scale_by_sum_log_vector(temp_log_prob_vp);
                    I_shift_vp->elements[ shift ] += log_delta;
                }

                /* The following normalization is necessary but the resulting likelihood is not useful (?) */
                temp = ow_exp_scale_by_sum_log_vector(I_shift_vp);

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }
 
                    log_a = SAFE_LOG(a_vp->elements[ cluster ]);

                    result = get_target_vector(&temp_log_prob_vp, num_shifts);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (shift = 0; shift < num_shifts; shift++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        log_delta = SAFE_LOG(delta_vp->elements[ shift ]);

                        temp_log_prob_vp->elements[ shift ] = log_delta + log_cluster_shift_density_mp->elements[ cluster ][ shift ]; 
                    }

                    I_cluster_vp->elements[ cluster ]  = ow_exp_scale_by_sum_log_vector(temp_log_prob_vp);
                    I_cluster_vp->elements[ cluster ] += log_a;
                }

                if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
                {
                    held_out_log_likelihood += ow_exp_scale_by_sum_log_vector(I_cluster_vp);
                }
                else
                {
                    log_likelihood += ow_exp_scale_by_sum_log_vector(I_cluster_vp);
                }
            }

            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_shift_vp), 1.0, 0.00001);
            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_cluster_vp), 1.0, 0.00001);

            if (result == ERROR) { NOTE_ERROR(); break; }

            if (P_shift_mpp != NULL)
            {
                result = put_matrix_row(*P_shift_mpp, I_shift_vp, i);
            }

            if (P_cluster_mpp != NULL)
            {
                result = put_matrix_row(*P_cluster_mpp, I_cluster_vp, i);
            }

            /* M-Step */
            
            if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
            {
                /* Do not do the M-step for held out points */
            }
            else
            {
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    double  p_cluster = I_cluster_vp->elements[ cluster ];

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (shift = 0; shift < num_shifts; shift++)
                    {
                        double p_shift     = I_shift_vp->elements[ shift ];
                        double p           = p_cluster * p_shift;

                        /*negative_shift     = -(min_shift + shift);*/
                    
                        if (result == ERROR) { NOTE_ERROR(); break; }
                        
                        /*
                        result = shift_point_with_extension(&shifted_x_vp, x_vp, negative_shift);
                        if (result == ERROR) { NOTE_ERROR(); break; }
                        */

                        ERE(get_target_vector(&shifted_x_vp, num_features));
                        for (feature = 0; feature < num_features; feature++)
                        {
                            shifted_x_vp->elements[feature] = x_vp->elements[feature + shift];
                        }

                        result = multiply_vectors(&shifted_x2_vp, shifted_x_vp, shifted_x_vp);
                        if (result == ERROR) { NOTE_ERROR(); break; } 
                        
                        result = ow_add_scalar_times_vector_to_matrix_row(new_u_mp, 
                                                                          shifted_x_vp, p,
                                                                          cluster);
                        if (result == ERROR) { NOTE_ERROR(); break; } 

                        result = ow_add_scalar_times_vector_to_matrix_row(new_var_mp, 
                                                                          shifted_x2_vp, p,
                                                                          cluster);
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        p_sum_shift_vp->elements[ shift ] += p_shift;
                    }

                    p_sum_cluster_vp->elements[ cluster ] += p_cluster; 
                }
            }

        } /* for (i = 0; i < num_points; i++) */

        /* M-Step cleanup */

        if (divide_matrix_by_col_vector(&u_mp, new_u_mp, p_sum_cluster_vp) == ERROR)
        {
            db_rv(p_sum_cluster_vp); 
            result = ERROR;
            break; 
        }

        result = get_initialized_matrix(&var_mp, num_clusters, num_features, 
                                        DBL_NOT_SET);
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            double s = p_sum_cluster_vp->elements[ cluster ];

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

            /* Still need to figure out the expression for unbiased estimate in this case.
            if (fs_use_unbiased_var_estimate_in_M_step == TRUE)
            {
                double p_square_sum      = p_square_sum_vp->elements[ cluster ];
                double norm_p_square_sum = p_square_sum / (s * s);
                double norm_factor       = 1.0 / (1.0 - norm_p_square_sum);

                result = ow_multiply_matrix_row_by_scalar(var_mp, norm_factor, cluster);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
            */

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
            temp = 0.0;

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

        result = scale_vector_by_sum(&delta_vp, p_sum_shift_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; } 
        
        result = scale_vector_by_sum(&a_vp, p_sum_cluster_vp); 
        if (result == ERROR) { NOTE_ERROR(); break; } 

        if (it > 0) 
        {
            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood + prev_log_likelihood));

            verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e\n", 
                        it + 1, log_likelihood, diff); 


            held_out_diff = held_out_log_likelihood - prev_held_out_log_likelihood;

            held_out_diff *= 2.0;
            held_out_diff /= (ABS_OF(held_out_log_likelihood + prev_held_out_log_likelihood));
            
            verbose_pso(3, "%-3d: Held out log likelihood is %12e  |  %10e\n",
                        it + 1, held_out_log_likelihood, held_out_diff); 

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

    } /* for (it = 0; it < fs_max_num_iterations; it++) */

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

        if (delta_vpp != NULL) 
        {
            result = copy_vector(delta_vpp, delta_vp);
        }
    }
    
    if ((result != ERROR) && (a_vpp != NULL)) 
    {
        result = copy_vector(a_vpp, a_vp);
    }

    if ((result != ERROR) && (var_mpp != NULL))
    {
        result = copy_matrix(var_mpp, var_mp);
    }

    if ((result != ERROR) && (means_mpp != NULL))
    {
        result = copy_matrix(means_mpp, u_mp);
    }

    free_matrix(log_cluster_shift_density_mp);
    free_vector(I_shift_vp);
    free_vector(I_cluster_vp);
    free_vector(p_sum_shift_vp);
    free_vector(p_sum_cluster_vp);
    free_vector(delta_vp);
    free_vector(a_vp);
    free_matrix(new_u_mp); 
    free_matrix(u_mp); 
    free_matrix(new_var_mp); 
    free_matrix(var_mp); 
#ifdef DONT_USE_GAUSS_ROUTINE
    free_vector(log_sqrt_det_vp);
#endif 

    free_vector(shifted_x2_vp); 
    free_vector(x_vp); 
    free_vector(shifted_x_vp);
    free_vector(var_vp); 
    free_vector(u_vp); 

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    free_matrix(I_shift_mp);
    free_matrix(I_cluster_mp);
#endif
    
    free_vector(temp_log_prob_vp);

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

int shift_point_cyclic
(
    Vector**      shifted_point_vpp,
    const Vector* point_vp,
    const int     shift
)
{
    int i, j; 
    int num_features;

    Vector* shifted_point_vp = NULL;

    num_features = point_vp->length;
    
    ERE(get_target_vector(shifted_point_vpp, num_features));

    shifted_point_vp = *shifted_point_vpp;

    for (i = 0; i < num_features; i++)
    {
        /* Kobus:
         * 
         * This does not make sense because fmod() works on doubles, not
         * integers. Please check the fix! 
         *
           j = kjb_rint(fmod(i-shift, num_features));
        */
        j = (i-shift) % num_features; 
        
        if (j < 0)
        {
            j = j + num_features;
        }

        ASSERT_IS_POSITIVE_INT(j+1);
        ASSERT(j < num_features);

        shifted_point_vp->elements[i] = point_vp->elements[j]; 
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int shift_point_with_extension
(
    Vector**      shifted_point_vpp,
    const Vector* point_vp,
    const int     shift
)
{
    int i, j; 
    int num_features;

    Vector* shifted_point_vp = NULL;

    num_features = point_vp->length;
    
    ERE(get_target_vector(shifted_point_vpp, num_features));

    shifted_point_vp = *shifted_point_vpp;

    for (i = 0; i < num_features; i++)
    {
        j = i-shift;
        
        if (j < 0)
        {
            j = 0;
        }

        if (j >= num_features)
        {
            j = num_features - 1;
        }

        shifted_point_vp->elements[i] = point_vp->elements[j]; 
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int shift_point_with_zero_padding
(
    Vector**      shifted_point_vpp,
    const Vector* point_vp,
    const int     shift
)
{
    int i, j; 
    int num_features;

    Vector* shifted_point_vp = NULL;

    num_features = point_vp->length;
    
    ERE(get_target_vector(shifted_point_vpp, num_features));

    shifted_point_vp = *shifted_point_vpp;

    for (i = 0; i < num_features; i++)
    {
        j = i-shift;
        
        if ( (j < 0) || (j >= num_features) )
        {
            shifted_point_vp->elements[i] = 0;
        }
        else
        {
            shifted_point_vp->elements[i] = point_vp->elements[j]; 
        }
    }

    return NO_ERROR;
}
#endif  /* End of code for ENABLE_SHIFT_MODELS. */
#endif  /* End of code block that is removed for export. */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *            get_GMM_blk_compound_sym_cov
 *                               
 * Finds a Gaussian mixture model (GMM) where the Gaussians have block compound
 * symmetrical covariances with shared parameters. Specifically, each feature
 * has the same mean (mu) and variance (sig^2 + tau^2). The covariance
 * between any pair of features is either (tau^2) or 0. This holds for all the
 * Gaussians in the mixture. However the block diagonal structure of the
 * covariance matrices of the different Gaussians in the mixture is different.
 * Further it is assumed that the ratio (tau^2 / sig^2) is known beforehand.
 * This amounts to reducing one degree of freedom in fitting the model but it
 * enables closed form solutions for optimum parameter values (mu, sig^2) in
 * the M-step of EM. A suitable value for the ratio can be estimated using
 * cross-validation.
 *
 * In particular, it fits:
 * |         p(x) = sum  a-sub-i *  g(mu-vec, cov-sub-i, x)
 * |                 i
 * where a-sub-i is the prior probability for the mixuture compoenent (cluster),
 * mu-vec is the mean vector with all components equal to mu, cov-sub-i is the
 * covariance matrix for the component i, and g(mu,cov,x) is a Gaussian with
 * mean mu and covariance cov. 
 *
 * The data matrix feature_mp is an N by M matrix where N is the number of data
 * points, and M is the number of features. 
 *
 * The argument block_diag_sizes_vvp specifies the block diagonal structures of
 * the covariances of the Gaussian components. The number of vectors in this
 * argument is equal to the number of mixture compoenents (clusters), K. Each
 * vector is a list of sizes of the block diagonals from top to bottom in the
 * corresponding covariance matrix. For eg., the vector corresponding to a
 * Gaussian component with all independent features would consist of 1 as all
 * its elements and the number of elements in the vector equal to M. And a
 * Gaussian component with two block diagonals of the same size in its
 * covariance matrix would be specified by a vector with two elements, each
 * equal to M/2.
 *
 * The argument tau_sig_sqr specifies the ratio as mentioned above.
 *
 * initial_a_vp, initial_mu and initial_sig_sqr can be used to specify the
 * initial values of the parameters for EM.
 *
 * The model parameters are put into *a_vpp, *mu_ptr, and *sig_sqr_ptr. Any
 * of a_vpp, mu_ptr, or sig_sqr_ptr is NULL if that value is not needed.
 *
 * If P_mpp is not NULL, then the soft clustering (cluster membership) for each
 * data point is returned. In that case, *P_mpp will be N by K. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with an error message being set. Otherwise NO_ERROR is returned. 
 *
 * Index: clustering, EM, GMM
 *
 * Note: The covariance structure assumed in this routine is often referred to
 * as "block compound-symmetry" structure especially in the mixed-models ANOVA
 * literature. It is useful in modeling data with repeated measures in ANOVA
 * using mixed-models. For example see:
 * http://www.asu.edu/sas/sasdoc/sashtml/stat/chap41/sect23.htm
 * http://www.tufts.edu/~gdallal/repeat2.htm 
 *
 * Authors: Prasad Gabbur, Kobus Barnard.
 * -----------------------------------------------------------------------------
*/

int get_GMM_blk_compound_sym_cov
(
    const Int_vector_vector* block_diag_sizes_vvp,
    const Matrix*            feature_mp,
    const Int_vector*            held_out_indicator_vp,
    const Vector*            initial_a_vp,
    const double             initial_mu,
    const double             initial_sig_sqr,
    const double             tau_sig_sqr_ratio,
    Vector**                 a_vpp, 
    double*                  mu_ptr,
    double*                  sig_sqr_ptr,
    Matrix**                 P_mpp,
    double*                  log_likelihood_ptr,
    double*                  held_out_log_likelihood_ptr,
    int*                     num_iterations_ptr
)
{
    int num_clusters = block_diag_sizes_vvp->length;
    int num_points   = feature_mp->num_rows;
    int num_features = feature_mp->num_cols;
   
    Int_vector*  block_diag_sizes_vp = NULL;
    
    int      cluster;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double   var_offset = 0.0;
#else
    double   var_offset = fs_var_offset; 
#endif 
    int      it, i, j, k, l;
    double   mu, sig_sqr;
    double   new_mu, new_sig_sqr;
    Vector*  I_vp          = NULL;
    Vector*  a_vp          = NULL;
    Vector*  p_sum_vp      = NULL;
    Vector*  x_vp          = NULL;
    Vector*  u_vp          = NULL;
    Vector*  centered_x_vp = NULL;
    Vector*  ones_vp       = NULL;
    Vector*  temp_vp       = NULL;
    double   log_likelihood               = DBL_HALF_MOST_NEGATIVE;
    double   prev_log_likelihood          = DBL_HALF_MOST_NEGATIVE;
    double   held_out_log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double   prev_held_out_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double   diff, held_out_diff;

    Matrix_vector* S_tilda_inv_mvp = NULL;
    Matrix*        S_tilda_inv_mp  = NULL;
    Matrix*        S_inv_mp        = NULL;

    double zero_order_stat;
    double first_order_stat;
    double second_order_stat;

    int     result = NO_ERROR;
#ifdef DONT_USE_GAUSS_ROUTINE
    Vector* log_sqrt_det_vp = NULL;
#endif 
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    Matrix* I_mp = NULL;
#endif 

#ifdef DONT_USE_GAUSS_ROUTINE
    double temp;
#endif
    double p, d;
    double log_a;
    int    num_block_diags;
    int    block_diag_size;
    int    sum_block_sizes;

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

    if (result != ERROR)
    {
        result = get_target_matrix_vector(&S_tilda_inv_mvp, num_clusters);
    }

    for (cluster = 0; cluster < num_clusters; cluster++)
    {
        if (result == ERROR) { NOTE_ERROR(); break; }

        sum_block_sizes = 0;

        /*
        S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

        result = get_zero_matrix(&S_tilda_inv_mp, num_features, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
        */

        result = get_zero_matrix(&(S_tilda_inv_mvp->elements[ cluster ]), num_features, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
        
        S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

        block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

        db_irv(block_diag_sizes_vp);

        num_block_diags = block_diag_sizes_vp->length;

        for (i = 0, j = 0, k = 0; i < num_block_diags; i++)
        {
            block_diag_size = block_diag_sizes_vp->elements[i];

            sum_block_sizes = sum_block_sizes + block_diag_size;

            l = j;

            for (j = l; j < (l + block_diag_size); j++)
            {
                for (k = l; k < (l + block_diag_size); k++)
                {
                    if (j == k)
                    {
                        S_tilda_inv_mp->elements[j][k] = 1 + ((block_diag_size - 1) * tau_sig_sqr_ratio);
                        S_tilda_inv_mp->elements[j][k] = S_tilda_inv_mp->elements[j][k] / \
                                                         (1 + (block_diag_size * tau_sig_sqr_ratio));
                    }
                    else
                    {
                        S_tilda_inv_mp->elements[j][k] = -tau_sig_sqr_ratio / \
                                                         (1 + (block_diag_size * tau_sig_sqr_ratio));
                    }
                }
            }
        }

        if (sum_block_sizes != num_features)
        {
            set_bug("Block diagonal sizes for cluster %d do not sum \
                    to the feature dimensionality:", (cluster + 1));
            return ERROR;
        }
    }

    for (it = 0; it < fs_max_num_iterations; it++)
    {
        log_likelihood = 0.0;

        held_out_log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        new_mu            = 0.0;
        new_sig_sqr  = 0.0;
        zero_order_stat   = 0.0;
        first_order_stat  = 0.0;
        second_order_stat = 0.0;

        if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
        {
            mu = initial_mu;

            sig_sqr = initial_sig_sqr + var_offset; 

            if (initial_a_vp != NULL)
            {
                ERE(get_target_vector(&a_vp, num_clusters));

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    a_vp->elements[ cluster ] = initial_a_vp->elements[ cluster ];
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
                temp = 0.0;

                if (result == ERROR) { NOTE_ERROR(); break; }

                block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

                num_block_diags = block_diag_sizes_vp->length;

                for (i = 0; i < num_block_diags; i++)
                {
                    block_diag_size = block_diag_sizes_vp->elements[i];

                    temp += block_diag_size * SAFE_LOG(sig_sqr);
                    temp += SAFE_LOG(1.0 + (block_diag_size * tau_sig_sqr_ratio));
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
                    log_a = SAFE_LOG(a_vp->elements[ cluster ]);

#ifdef DONT_USE_GAUSS_ROUTINE                    
                    S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];
                    
                    result = multiply_matrix_by_scalar(&S_inv_mp, S_tilda_inv_mp, (1.0 / sig_sqr));
                    if (result == ERROR) { NOTE_ERROR(); break; }
#else 
                    result = ERROR;
                    set_error("No Gauss routine for this case exists yet, sorry!");
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /*
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = get_matrix_row(&u_vp, u_mp, cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&var_vp, var_mp, cluster);
                    */
#endif 

#ifdef DONT_USE_GAUSS_ROUTINE

                    result = get_unity_vector(&ones_vp, num_features);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = multiply_vector_by_scalar(&u_vp, ones_vp, mu);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = subtract_vectors(&centered_x_vp, x_vp, u_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = multiply_matrix_and_vector(&temp_vp, S_inv_mp, centered_x_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_dot_product(centered_x_vp, temp_vp, &d);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    ASSERT_IS_NUMBER_DBL(d); 
                    ASSERT_IS_FINITE_DBL(d); 

                    I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ]; 
#else
                    result = ERROR;
                    set_error("No Gauss routine for this case exists yet, sorry!");
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /*
                    result = get_log_gaussian_density(x_vp, u_vp, var_vp, 
                                                      &(I_vp->elements[ cluster ]));
                    if (result == ERROR) { NOTE_ERROR(); break; }
                    I_vp->elements[ cluster ] += log_a; 
                    */
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
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    p = I_vp->elements[ cluster ];

                    S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

                    result = get_unity_vector(&ones_vp, num_features);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /* Compute zero-order statistic */
                    result = multiply_matrix_and_vector(&temp_vp, S_tilda_inv_mp, ones_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_dot_product(ones_vp, temp_vp, &d);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    zero_order_stat += (p * d);

                    /* Compute first-order statistic */
                    result = multiply_matrix_and_vector(&temp_vp, S_tilda_inv_mp, x_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_dot_product(ones_vp, temp_vp, &d);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    first_order_stat += (p * d);

                    new_mu = first_order_stat;

                    /* Compute second-order statistic */
                    result = multiply_matrix_and_vector(&temp_vp, S_tilda_inv_mp, x_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_dot_product(x_vp, temp_vp, &d);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    second_order_stat += (p * d);
                    
                    new_sig_sqr = second_order_stat;

                    p_sum_vp->elements[ cluster ] += p; 
                }
            }    
        }

        /* M-Step cleanup */

        ASSERT_IS_NOT_ZERO_DBL(zero_order_stat);
        mu = new_mu / zero_order_stat;

        sig_sqr = new_sig_sqr - (2.0 * mu * first_order_stat);
        sig_sqr = sig_sqr + (mu * mu * zero_order_stat);
        sig_sqr = sig_sqr / (sum_vector_elements(p_sum_vp));
        sig_sqr = sig_sqr / num_features;

#ifdef DONT_USE_GAUSS_ROUTINE
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            temp = 0.0;

            if (result == ERROR) { NOTE_ERROR(); break; }

            block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

            num_block_diags = block_diag_sizes_vp->length;

            for (i = 0; i < num_block_diags; i++)
            {
                block_diag_size = block_diag_sizes_vp->elements[i];

                temp += block_diag_size * SAFE_LOG(sig_sqr);
                temp += SAFE_LOG(1.0 + (block_diag_size * tau_sig_sqr_ratio));
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
            diff /= (ABS_OF(log_likelihood + prev_log_likelihood));

            verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e\n", 
                        it + 1, log_likelihood, diff); 


            held_out_diff = held_out_log_likelihood - prev_held_out_log_likelihood;

            held_out_diff *= 2.0;
            held_out_diff /= (ABS_OF(held_out_log_likelihood + prev_held_out_log_likelihood));
            
            verbose_pso(3, "%-3d: Held out log likelihood is %12e  |  %10e\n",
                        it + 1, held_out_log_likelihood, held_out_diff); 

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
    
    if ((result != ERROR) && (mu_ptr != NULL))
    {
        *mu_ptr = mu;
    }

    if ((result != ERROR) && (sig_sqr_ptr != NULL))
    {
        *sig_sqr_ptr = sig_sqr;
    }

    free_vector(I_vp); 
    free_vector(p_sum_vp);
    free_vector(a_vp);
#ifdef DONT_USE_GAUSS_ROUTINE
    free_vector(log_sqrt_det_vp);
#endif 

    free_vector(x_vp); 
    free_vector(u_vp); 
    free_vector(centered_x_vp);
    free_vector(ones_vp);
    free_vector(temp_vp);

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    free_matrix(I_mp); 
#endif

    free_matrix_vector(S_tilda_inv_mvp);
    free_matrix(S_inv_mp);

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
 *            get_GMM_blk_compound_sym_cov_1
 * 
 * This routine is the same as the get_GMM_blk_compound_sym_cov with the added
 * feature of estimating the tau_sig_sqr_ratio parameter automatically through
 * gradient ascent of the expected complete log likelihood in the M step.
 *
 * Finds a Gaussian mixture model (GMM) where the Gaussians have block compound
 * symmetrical covariances with shared parameters. Specifically, each feature
 * has the same mean (mu) and variance (sig^2 + tau^2). The covariance
 * between any pair of features is either (tau^2) or 0. This holds for all the
 * Gaussians in the mixture. However the block diagonal structure of the
 * covariance matrices of the different Gaussians in the mixture is different.
 * Here it is not assumed that the ratio (tau^2 / sig^2) is known beforehand
 * unlike the parent routine get_GMM_blk_compound_sym_cov.
 *
 * In particular, it fits:
 * |         p(x) = sum  a-sub-i *  g(mu-vec, cov-sub-i, x)
 * |                 i
 * where a-sub-i is the prior probability for the mixuture compoenent (cluster),
 * mu-vec is the mean vector with all components equal to mu, cov-sub-i is the
 * covariance matrix for the component i, and g(mu,cov,x) is a Gaussian with
 * mean mu and covariance cov. 
 *
 * The data matrix feature_mp is an N by M matrix where N is the number of data
 * points, and M is the number of features. 
 *
 * The argument block_diag_sizes_vvp specifies the block diagonal structures of
 * the covariances of the Gaussian components. The number of vectors in this
 * argument is equal to the number of mixture compoenents (clusters), K. Each
 * vector is a list of sizes of the block diagonals from top to bottom in the
 * corresponding covariance matrix. For eg., the vector corresponding to a
 * Gaussian component with all independent features would consist of 1 as all
 * its elements and the number of elements in the vector equal to M. And a
 * Gaussian component with two block diagonals of the same size in its
 * covariance matrix would be specified by a vector with two elements, each
 * equal to M/2.
 *
 * initial_a_vp, initial_mu, initial_sig_sqr and initial_tau_sig_sqr_ratio can
 * be used to specify the initial values of the parameters for EM.
 *
 * The model parameters are put into *a_vpp, *mu_ptr, *sig_sqr_ptr and
 * *tau_sig_sqr_ratio_ptr. Any of a_vpp, mu_ptr, sig_sqr_ptr or
 * tau_sig_sqr_ratio_ptr is NULL if that value is not needed.
 *
 * If P_mpp is not NULL, then the soft clustering (cluster membership) for each
 * data point is returned. In that case, *P_mpp will be N by K. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with an error message being set. Otherwise NO_ERROR is returned. 
 *
 * Index: clustering, EM, GMM
 *
 * Note: The covariance structure assumed in this routine is often referred to
 * as "block compound-symmetry" structure especially in the mixed-models ANOVA
 * literature. It is useful in modeling data with repeated measures in ANOVA
 * using mixed-models. For example see:
 * http://www.asu.edu/sas/sasdoc/sashtml/stat/chap41/sect23.htm
 * http://www.tufts.edu/~gdallal/repeat2.htm 
 *
 * Authors: Prasad Gabbur, Kobus Barnard.
 * -----------------------------------------------------------------------------
*/
int get_GMM_blk_compound_sym_cov_1
(
    const Int_vector_vector* block_diag_sizes_vvp,
    const Matrix*            feature_mp,
    const Int_vector*        held_out_indicator_vp,
    const Vector*            initial_a_vp,
    const double             initial_mu,
    const double             initial_sig_sqr,
    const double             initial_tau_sig_sqr_ratio,
    Vector**                 a_vpp, 
    double*                  mu_ptr,
    double*                  sig_sqr_ptr,
    double*                  tau_sig_sqr_ratio_ptr,
    Matrix**                 P_mpp,
    double*                  log_likelihood_ptr,
    double*                  held_out_log_likelihood_ptr,
    int*                     num_iterations_ptr
)
{
    int num_clusters = block_diag_sizes_vvp->length;
    int num_points   = feature_mp->num_rows;
    int num_features = feature_mp->num_cols;
   
    Int_vector*  block_diag_sizes_vp = NULL;
    
    int      cluster;
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS
    double   var_offset = 0.0;
#else
    double   var_offset = fs_var_offset; 
#endif 
    int      it, i, j, k, l, m;
    int      it_grad_ascent, it_M_step;
    double   mu, sig_sqr;
    double   new_mu, new_sig_sqr;
    double   prev_mu           = DBL_NOT_SET;
    double   prev_sig_sqr      = DBL_NOT_SET;
    double   diff_mu, diff_sig_sqr;
    double   tau_sig_sqr_ratio;
    double   new_tau_sig_sqr_ratio;
    double   prev_tau_sig_sqr_ratio = DBL_NOT_SET;
    double   diff_tau_sig_sqr_ratio;
    Vector*  I_vp          = NULL;
    Vector*  a_vp          = NULL;
    Vector*  p_sum_vp      = NULL;
    Vector*  x_vp          = NULL;
    Vector*  x_sub_vp      = NULL;
    Vector*  u_vp          = NULL;
    Vector*  centered_x_vp = NULL;
    Vector*  ones_vp       = NULL;
    Vector*  temp_vp       = NULL;
    double   log_likelihood               = DBL_HALF_MOST_NEGATIVE;
    double   prev_log_likelihood          = DBL_HALF_MOST_NEGATIVE;
    double   held_out_log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    double   prev_held_out_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double   diff, held_out_diff;

    Matrix_vector* S_tilda_inv_mvp = NULL;
    Matrix*        S_tilda_inv_mp  = NULL;
    Matrix*        S_inv_mp        = NULL;
    Matrix*        temp_mp         = NULL;

    double zero_order_stat;
    double first_order_stat;
    double second_order_stat;

    double zero_order_stat_for_grad;
    double first_order_stat_for_grad;
    double second_order_stat_for_grad;
    double tau_sig_sqr_ratio_grad;
    double prev_tau_sig_sqr_ratio_grad;

    int     result = NO_ERROR;
#ifdef DONT_USE_GAUSS_ROUTINE
    Vector* log_sqrt_det_vp = NULL;
#endif 
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    Matrix* I_mp = NULL;
#endif
    Matrix* I_1_mp = NULL;

#ifndef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT    
    double  exp_comp_log_likelihood;
    double  pt_exp_comp_log_likelihood;

    Vector* plot_tau_sig_sqr_ratio_grad_vp  = NULL;
    Vector* plot_log_likelihood_vp          = NULL;
    Vector* log_p_vp                        = NULL;
    Vector* plot_exp_comp_log_likelihood_vp = NULL;
#endif

#ifdef DONT_USE_GAUSS_ROUTINE
    double temp;
#endif
    double p, d;
    double log_a;
    double inv_S_blk_grad;
    int    num_block_diags;
    int    block_diag_size;
    int    sum_block_sizes;

    double rate_grad_ascent = 1.0;
    double prev_abs_change  = 1.0;
    int    update_mu_sig_sqr;

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    ERE(get_cluster_random_matrix(&I_mp, num_points, num_clusters));
#endif 

    if (P_mpp != NULL)
    {
        ERE(get_target_matrix(P_mpp, num_points, num_clusters));
    }

    ERE(get_target_matrix(&I_1_mp, num_points, num_clusters));

#ifndef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT    
    ERE(get_target_vector(&plot_tau_sig_sqr_ratio_grad_vp, 1000/0.01));
#endif

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

    if (result != ERROR)
    {
        result = get_target_matrix_vector(&S_tilda_inv_mvp, num_clusters);
    }

    tau_sig_sqr_ratio = initial_tau_sig_sqr_ratio;

    for (cluster = 0; cluster < num_clusters; cluster++)
    {
        if (result == ERROR) { NOTE_ERROR(); break; }

        sum_block_sizes = 0;

        /*
        S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

        result = get_zero_matrix(&S_tilda_inv_mp, num_features, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
        */

        result = get_zero_matrix(&(S_tilda_inv_mvp->elements[ cluster ]), num_features, num_features);
        if (result == ERROR) { NOTE_ERROR(); break; }
        
        S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

        block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

        db_irv(block_diag_sizes_vp);

        num_block_diags = block_diag_sizes_vp->length;

        for (i = 0, j = 0, k = 0; i < num_block_diags; i++)
        {
            block_diag_size = block_diag_sizes_vp->elements[i];

            sum_block_sizes = sum_block_sizes + block_diag_size;

            l = j;

            for (j = l; j < (l + block_diag_size); j++)
            {
                for (k = l; k < (l + block_diag_size); k++)
                {
                    if (j == k)
                    {
                        S_tilda_inv_mp->elements[j][k] = 1 + ((block_diag_size - 1) * tau_sig_sqr_ratio);
                        S_tilda_inv_mp->elements[j][k] = S_tilda_inv_mp->elements[j][k] / \
                                                         (1 + (block_diag_size * tau_sig_sqr_ratio));
                    }
                    else
                    {
                        S_tilda_inv_mp->elements[j][k] = -tau_sig_sqr_ratio / \
                                                         (1 + (block_diag_size * tau_sig_sqr_ratio));
                    }
                }
            }
        }

        if (sum_block_sizes != num_features)
        {
            set_bug("Block diagonal sizes for cluster %d do not sum \
                    to the feature dimensionality:", (cluster + 1));
            return ERROR;
        }
    }

    for (it = 0; it < fs_max_num_iterations; it++)
    {
#ifndef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT

        ERE(get_target_vector(&log_p_vp, num_clusters));
        ERE(get_target_vector(&plot_log_likelihood_vp, 1000/0.01));
        ERE(get_target_vector(&plot_exp_comp_log_likelihood_vp, 1000/0.01));

        it_grad_ascent   = 0;
        tau_sig_sqr_ratio = initial_tau_sig_sqr_ratio;

        do
        {
        dbi(it_grad_ascent);

        exp_comp_log_likelihood = 0.0;
#endif
        log_likelihood = 0.0;

        held_out_log_likelihood = 0.0;

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&p_sum_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        if ( (it == 0) && (fs_use_initialized_cluster_means_variances_and_priors == TRUE) )
        {
            mu                = initial_mu;
            sig_sqr           = initial_sig_sqr + var_offset;
#ifdef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT           
            tau_sig_sqr_ratio = initial_tau_sig_sqr_ratio;
#endif
            prev_mu                = mu;
            prev_sig_sqr           = sig_sqr;
            prev_tau_sig_sqr_ratio = tau_sig_sqr_ratio;

            if (initial_a_vp != NULL)
            {
                ERE(get_target_vector(&a_vp, num_clusters));

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    a_vp->elements[ cluster ] = initial_a_vp->elements[ cluster ];
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
                temp = 0.0;

                if (result == ERROR) { NOTE_ERROR(); break; }

                block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

                num_block_diags = block_diag_sizes_vp->length;

                for (i = 0; i < num_block_diags; i++)
                {
                    block_diag_size = block_diag_sizes_vp->elements[i];

                    temp += block_diag_size * SAFE_LOG(sig_sqr);
                    temp += SAFE_LOG(1.0 + (block_diag_size * tau_sig_sqr_ratio));
                }

                log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
            }
            verify_vector(log_sqrt_det_vp, NULL);
#endif 
        }
        
        /* E Step */
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
                    log_a = SAFE_LOG(a_vp->elements[ cluster ]);

#ifdef DONT_USE_GAUSS_ROUTINE                    
                    S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];
                    
                    result = multiply_matrix_by_scalar(&S_inv_mp, S_tilda_inv_mp, (1.0 / sig_sqr));
                    if (result == ERROR) { NOTE_ERROR(); break; }
#else 
                    result = ERROR;
                    set_error("No Gauss routine for this case exists yet, sorry!");
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /*
                    if (result == ERROR) { NOTE_ERROR(); break; } 

                    result = get_matrix_row(&u_vp, u_mp, cluster);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&var_vp, var_mp, cluster);
                    */
#endif 

#ifdef DONT_USE_GAUSS_ROUTINE

                    result = get_unity_vector(&ones_vp, num_features);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = multiply_vector_by_scalar(&u_vp, ones_vp, mu);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = subtract_vectors(&centered_x_vp, x_vp, u_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = multiply_matrix_and_vector(&temp_vp, S_inv_mp, centered_x_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_dot_product(centered_x_vp, temp_vp, &d);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    ASSERT_IS_NUMBER_DBL(d); 
                    ASSERT_IS_FINITE_DBL(d); 

                    I_vp->elements[ cluster ] = log_a - (0.5*d) - log_sqrt_det_vp->elements[ cluster ]; 
#else
                    result = ERROR;
                    set_error("No Gauss routine for this case exists yet, sorry!");
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /*
                    result = get_log_gaussian_density(x_vp, u_vp, var_vp, 
                                                      &(I_vp->elements[ cluster ]));
                    if (result == ERROR) { NOTE_ERROR(); break; }
                    I_vp->elements[ cluster ] += log_a; 
                    */
#endif 

#ifndef DONT_USE_GAUSS_ROUTINE
#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS_LL
                    I_vp->elements[ cluster ] += (((double)num_features) / 2.0) * log(2.0 * M_PI); 
#endif 
#endif 

#ifndef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT
                    log_p_vp->elements[ cluster ] =  - (0.5*d) - log_sqrt_det_vp->elements[ cluster ];
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

#ifndef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT
                ERE(get_dot_product(log_p_vp, I_vp, &pt_exp_comp_log_likelihood));
                exp_comp_log_likelihood += pt_exp_comp_log_likelihood; 
#endif

            }

            ASSERT_IS_NEARLY_EQUAL_DBL(sum_vector_elements(I_vp), 1.0, 0.00001);

            if (result == ERROR) { NOTE_ERROR(); break; }

            if (P_mpp != NULL)
            {
                result = put_matrix_row(*P_mpp, I_vp, i);
            }

            if (result == ERROR) { NOTE_ERROR(); break; }

            result = put_matrix_row(I_1_mp, I_vp, i);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }

#ifndef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT

        verbose_pso(3, "%-3d: tau_sig_sqr_ratio is %12e  |  %10e\n",
                            it_grad_ascent + 1, tau_sig_sqr_ratio, diff_tau_sig_sqr_ratio);

        plot_log_likelihood_vp->elements[it_grad_ascent]          = log_likelihood;
        plot_exp_comp_log_likelihood_vp->elements[it_grad_ascent] = exp_comp_log_likelihood;

        tau_sig_sqr_ratio                                 = tau_sig_sqr_ratio + 0.01;

        it_grad_ascent = it_grad_ascent + 1;
        
        /* Form S_tilda_mvp from the new value of tau_sig_sqr_ratio */
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_zero_matrix(&(S_tilda_inv_mvp->elements[ cluster ]), num_features, num_features);
            if (result == ERROR) { NOTE_ERROR(); break; }

            S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

            block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

            num_block_diags = block_diag_sizes_vp->length;

            for (i = 0, j = 0, k = 0; i < num_block_diags; i++)
            {
                block_diag_size = block_diag_sizes_vp->elements[i];

                l = j;

                for (j = l; j < (l + block_diag_size); j++)
                {
                    for (k = l; k < (l + block_diag_size); k++)
                    {
                        if (j == k)
                        {
                            S_tilda_inv_mp->elements[j][k] = 1 + ((block_diag_size - 1) * tau_sig_sqr_ratio);
                            S_tilda_inv_mp->elements[j][k] = S_tilda_inv_mp->elements[j][k] / \
                                                             (1 + (block_diag_size * tau_sig_sqr_ratio));
                        }
                        else
                        {
                            S_tilda_inv_mp->elements[j][k] = -tau_sig_sqr_ratio / \
                                                             (1 + (block_diag_size * tau_sig_sqr_ratio));
                        }
                    }
                }
            }
        }

        } while (tau_sig_sqr_ratio < 1000);

        ERE(write_col_vector(plot_log_likelihood_vp, "log_likelihood_vp"));
        ERE(write_col_vector(plot_log_likelihood_vp, "exp_comp_log_likelihood_vp"));

        tau_sig_sqr_ratio = initial_tau_sig_sqr_ratio;

        /* Form S_tilda_mvp from the new value of tau_sig_sqr_ratio */
        for (cluster = 0; cluster < num_clusters; cluster++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_zero_matrix(&(S_tilda_inv_mvp->elements[ cluster ]), num_features, num_features);
            if (result == ERROR) { NOTE_ERROR(); break; }

            S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

            block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

            num_block_diags = block_diag_sizes_vp->length;

            for (i = 0, j = 0, k = 0; i < num_block_diags; i++)
            {
                block_diag_size = block_diag_sizes_vp->elements[i];

                l = j;

                for (j = l; j < (l + block_diag_size); j++)
                {
                    for (k = l; k < (l + block_diag_size); k++)
                    {
                        if (j == k)
                        {
                            S_tilda_inv_mp->elements[j][k] = 1 + ((block_diag_size - 1) * tau_sig_sqr_ratio);
                            S_tilda_inv_mp->elements[j][k] = S_tilda_inv_mp->elements[j][k] / \
                                                             (1 + (block_diag_size * tau_sig_sqr_ratio));
                        }
                        else
                        {
                            S_tilda_inv_mp->elements[j][k] = -tau_sig_sqr_ratio / \
                                                             (1 + (block_diag_size * tau_sig_sqr_ratio));
                        }
                    }
                }
            }
        }

#endif
        
        /* M-Step */
#ifdef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT
        update_mu_sig_sqr = TRUE;
#else
        dbf(mu);
        dbf(sig_sqr);
        update_mu_sig_sqr = FALSE;
#endif

        it_M_step = 0;

        do
        {
            it_grad_ascent              = 0;
            prev_tau_sig_sqr_ratio_grad = DBL_NOT_SET;
            rate_grad_ascent            = 1.0;

            do /*for (it_grad_ascent = 0; it_grad_ascent < 1000; it_grad_ascent++)*/
            {
                new_mu            = 0.0;
                new_sig_sqr       = 0.0;
                zero_order_stat   = 0.0;
                first_order_stat  = 0.0;
                second_order_stat = 0.0;

                tau_sig_sqr_ratio_grad     = 0.0;
                zero_order_stat_for_grad   = 0.0;
                first_order_stat_for_grad  = 0.0;
                second_order_stat_for_grad = 0.0;

                result = get_zero_vector(&p_sum_vp, num_clusters);
                if (result == ERROR) { NOTE_ERROR(); break; }

                for (i = 0; i<num_points; i++)
                {     
                    if ( (held_out_indicator_vp != NULL) && (held_out_indicator_vp->elements[i] == 1) )
                    {
                        /* Do not do the M-step for held out points */
                    } 
                    else
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; } 

                        result = get_matrix_row(&x_vp, feature_mp, i);
                        if (result == ERROR) { NOTE_ERROR(); break; } 

                        result = get_matrix_row(&I_vp, I_1_mp, i);
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        for (cluster = 0; cluster < num_clusters; cluster++)
                        {
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            p = I_vp->elements[ cluster ];

                            S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

                            /* Compute a gradient term */
                            block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

                            num_block_diags = block_diag_sizes_vp->length;

                            for (j = 0; j < num_block_diags; j++)
                            {
                                block_diag_size = block_diag_sizes_vp->elements[j];

                                tau_sig_sqr_ratio_grad -= (0.5 * p * block_diag_size) /\
                                                               (1.0 + (block_diag_size * tau_sig_sqr_ratio));
                            }

                            /* Compute zero-order statistics */
                            result = get_unity_vector(&ones_vp, num_features);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = multiply_matrix_and_vector(&temp_vp, S_tilda_inv_mp, ones_vp);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = get_dot_product(ones_vp, temp_vp, &d);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            zero_order_stat += (p * d);

                            block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

                            num_block_diags = block_diag_sizes_vp->length;

                            for (j = 0; j < num_block_diags; j++)
                            {
                                block_diag_size = block_diag_sizes_vp->elements[j];

                                inv_S_blk_grad  = 1.0/(1.0 +  (block_diag_size*tau_sig_sqr_ratio));
                                inv_S_blk_grad  = inv_S_blk_grad*inv_S_blk_grad; 
                                result = get_initialized_matrix(&temp_mp, block_diag_size, block_diag_size, inv_S_blk_grad);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                result = get_unity_vector(&ones_vp, block_diag_size);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                result = multiply_matrix_and_vector(&temp_vp, temp_mp, ones_vp);                    
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                result = get_dot_product(ones_vp, temp_vp, &d);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                zero_order_stat_for_grad += (p * d);
                            }

                            /* Compute first-order statistics */
                            result = multiply_matrix_and_vector(&temp_vp, S_tilda_inv_mp, x_vp);                    
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = get_unity_vector(&ones_vp, num_features);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = get_dot_product(ones_vp, temp_vp, &d);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            first_order_stat += (p * d);

                            new_mu = first_order_stat;

                            block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

                            num_block_diags = block_diag_sizes_vp->length;

                            for (j = 0, k = 0; j < num_block_diags; j++)
                            {
                                block_diag_size = block_diag_sizes_vp->elements[j];

                                result = get_target_vector(&x_sub_vp, block_diag_size);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                l = k;

                                for (k = l, m = 0; k < (l + block_diag_size); k++, m++)
                                {
                                    x_sub_vp->elements[m] = x_vp->elements[k];
                                }

                                inv_S_blk_grad  = 1.0/(1.0 +  (block_diag_size*tau_sig_sqr_ratio));
                                inv_S_blk_grad  = inv_S_blk_grad*inv_S_blk_grad; 
                                result = get_initialized_matrix(&temp_mp, block_diag_size, block_diag_size, inv_S_blk_grad);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                result = multiply_matrix_and_vector(&temp_vp, temp_mp, x_sub_vp);                    
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                result = get_unity_vector(&ones_vp, block_diag_size);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                result = get_dot_product(ones_vp, temp_vp, &d);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                first_order_stat_for_grad += (p * d);
                            }

                            /* Compute second-order statistics */
                            result = multiply_matrix_and_vector(&temp_vp, S_tilda_inv_mp, x_vp);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = get_dot_product(x_vp, temp_vp, &d);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            second_order_stat += (p * d);

                            new_sig_sqr = second_order_stat;

                            block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

                            num_block_diags = block_diag_sizes_vp->length;

                            for (j = 0, k = 0; j < num_block_diags; j++)
                            {
                                block_diag_size = block_diag_sizes_vp->elements[j];

                                result = get_target_vector(&x_sub_vp, block_diag_size);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                l = k;

                                for (k = l, m = 0; k < (l + block_diag_size); k++, m++)
                                {
                                    x_sub_vp->elements[m] = x_vp->elements[k];
                                }

                                inv_S_blk_grad = (1.0/(1.0 +  (block_diag_size*tau_sig_sqr_ratio)));
                                inv_S_blk_grad = inv_S_blk_grad*inv_S_blk_grad; 
                                result = get_initialized_matrix(&temp_mp, block_diag_size, block_diag_size, inv_S_blk_grad);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                result = multiply_matrix_and_vector(&temp_vp, temp_mp, x_sub_vp);                    
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                result = get_dot_product(x_sub_vp, temp_vp, &d);
                                if (result == ERROR) { NOTE_ERROR(); break; }

                                second_order_stat_for_grad += (p * d);
                            }

                            p_sum_vp->elements[ cluster ] += p; 
                        }
                    }    
                }

                /* M-Step cleanup */
                if (update_mu_sig_sqr == TRUE)
                {
                    ASSERT_IS_NOT_ZERO_DBL(zero_order_stat);
                    mu = new_mu / zero_order_stat;

                    sig_sqr = new_sig_sqr - (2.0 * mu * first_order_stat);
                    sig_sqr = sig_sqr + (mu * mu * zero_order_stat);
                    sig_sqr = sig_sqr / (sum_vector_elements(p_sum_vp));
                    sig_sqr = sig_sqr / num_features;
            
                    diff_mu      = ABS_OF(prev_mu - mu);
                    diff_sig_sqr = ABS_OF(prev_sig_sqr - sig_sqr); 

                    verbose_pso(3, "%-3d, %-3d, %-3d: mu is %12e  |  %10e\n",
                                it + 1, it_M_step + 1, it_grad_ascent + 1, mu, diff_mu);
            
                    verbose_pso(3, "%-3d, %-3d, %-3d: sig_sqr is %12e  |  %10e\n",
                                it + 1, it_M_step + 1, it_grad_ascent + 1, sig_sqr, diff_sig_sqr); 
                }

                ASSERT_IS_NOT_ZERO_DBL(sig_sqr);
                tau_sig_sqr_ratio_grad += (mu * mu * zero_order_stat_for_grad) / (2.0 * sig_sqr);
                tau_sig_sqr_ratio_grad -= (2.0 * mu * first_order_stat_for_grad) / (2.0 * sig_sqr);
                tau_sig_sqr_ratio_grad += second_order_stat_for_grad / (2.0 * sig_sqr);

                verbose_pso(3, "%-3d, %-3d, %-3d: tau_sig_sqr_ratio_grad is %12e\n", 
                            it + 1, it_M_step + 1, it_grad_ascent + 1, tau_sig_sqr_ratio_grad);

#ifdef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT                
                /* tau_sig_sqr_ratio gets updated here using gradient ascent */
                new_tau_sig_sqr_ratio = tau_sig_sqr_ratio + \
                                        (rate_grad_ascent * SIGN_OF(tau_sig_sqr_ratio_grad));

                verbose_pso(3, "%-3d, %-3d, %-3d: new_tau_sig_sqr_ratio is %12e\n", 
                            it + 1, it_M_step + 1, it_grad_ascent + 1, new_tau_sig_sqr_ratio);

                /* The following is a hack to avoid going below zero for the
                 * tau_sig_sqr_ratio parameter. When we get close to zero,
                 * restart from the initial value(?) for this parameter. Also
                 * reduce the rate of gradient ascent in order to explore the
                 * space in between. The assumption is that the likelihood
                 * maxima lies somewhere between zero and the initial value. */
                if (new_tau_sig_sqr_ratio <= DBL_EPSILON)
                {
                    /*
                    tau_sig_sqr_ratio = initial_tau_sig_sqr_ratio;
                    */
                    rate_grad_ascent  = rate_grad_ascent / 2.0;

                    dbe(rate_grad_ascent);

                    /*
                    if ( ABS_OF(rate_grad_ascent * tau_sig_sqr_ratio_grad) >= 0.00001 )
                    */
                    if ( rate_grad_ascent >= 0.00001 )
                    {
                        continue;
                    }
                }
                else
                {
                    tau_sig_sqr_ratio = new_tau_sig_sqr_ratio;
                }

                diff_tau_sig_sqr_ratio = ABS_OF(tau_sig_sqr_ratio - prev_tau_sig_sqr_ratio);
                prev_tau_sig_sqr_ratio = tau_sig_sqr_ratio;
                prev_abs_change = diff_tau_sig_sqr_ratio;

                /* Mocking simulated annealing by reducing the rate of gradient
                 * ascent when two successive gradients are of the opposite
                 * sign. */
                if ( prev_tau_sig_sqr_ratio_grad != DBL_NOT_SET )
                {
                    if (    (prev_tau_sig_sqr_ratio_grad <= 0 && tau_sig_sqr_ratio_grad > 0) 
                         || (prev_tau_sig_sqr_ratio_grad >= 0 && tau_sig_sqr_ratio_grad < 0) )
                    {
                        rate_grad_ascent = rate_grad_ascent / 2.0;

                        dbe(rate_grad_ascent);
                    }
                }
                prev_tau_sig_sqr_ratio_grad = tau_sig_sqr_ratio_grad;
#else
                plot_tau_sig_sqr_ratio_grad_vp->elements[it_grad_ascent] = tau_sig_sqr_ratio_grad;
                tau_sig_sqr_ratio                                        = tau_sig_sqr_ratio + 0.01;
#endif
                verbose_pso(3, "%-3d, %-3d, %-3d: tau_sig_sqr_ratio is %12e  |  %10e\n",
                            it + 1, it_M_step + 1, it_grad_ascent + 1, tau_sig_sqr_ratio, diff_tau_sig_sqr_ratio);

                /* Form S_tilda_mvp from the new value of tau_sig_sqr_ratio */
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_zero_matrix(&(S_tilda_inv_mvp->elements[ cluster ]), num_features, num_features);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    S_tilda_inv_mp = S_tilda_inv_mvp->elements[ cluster ];

                    block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

                    num_block_diags = block_diag_sizes_vp->length;

                    for (i = 0, j = 0, k = 0; i < num_block_diags; i++)
                    {
                        block_diag_size = block_diag_sizes_vp->elements[i];

                        l = j;

                        for (j = l; j < (l + block_diag_size); j++)
                        {
                            for (k = l; k < (l + block_diag_size); k++)
                            {
                                if (j == k)
                                {
                                    S_tilda_inv_mp->elements[j][k] = 1 + ((block_diag_size - 1) * tau_sig_sqr_ratio);
                                    S_tilda_inv_mp->elements[j][k] = S_tilda_inv_mp->elements[j][k] / \
                                                                     (1 + (block_diag_size * tau_sig_sqr_ratio));
                                }
                                else
                                {
                                    S_tilda_inv_mp->elements[j][k] = -tau_sig_sqr_ratio / \
                                                                     (1 + (block_diag_size * tau_sig_sqr_ratio));
                                }
                            }
                        }
                    }
                }

#ifdef DONT_USE_GAUSS_ROUTINE
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    temp = 0.0;

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    block_diag_sizes_vp = block_diag_sizes_vvp->elements[ cluster ];

                    num_block_diags = block_diag_sizes_vp->length;

                    for (i = 0; i < num_block_diags; i++)
                    {
                        block_diag_size = block_diag_sizes_vp->elements[i];

                        temp += block_diag_size * SAFE_LOG(sig_sqr);
                        temp += SAFE_LOG(1.0 + (block_diag_size * tau_sig_sqr_ratio));
                    }

                    log_sqrt_det_vp->elements[ cluster ] = temp / 2.0;
                }
                verify_vector(log_sqrt_det_vp, NULL);
#endif 

                result = scale_vector_by_sum(&a_vp, p_sum_vp); 
                if (result == ERROR) { NOTE_ERROR(); break; } 

#ifdef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT                     
                if ( diff_tau_sig_sqr_ratio < 0.00001 )
                {
                    update_mu_sig_sqr = TRUE;
                    break;
                }
#endif                
                update_mu_sig_sqr = FALSE;

                it_grad_ascent = it_grad_ascent + 1;

            }
#ifdef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT     
            while (1); 
#else
            while (tau_sig_sqr_ratio < 1000);
#endif

            if (update_mu_sig_sqr == TRUE)
            {
                if (    diff_mu      < 0.00001 
                     && diff_sig_sqr < 0.00001 
                     && prev_mu      != DBL_NOT_SET
                     && prev_sig_sqr != DBL_NOT_SET )
                {
                    update_mu_sig_sqr = FALSE;
                }
            }

            prev_mu      = mu;
            prev_sig_sqr = sig_sqr;

            it_M_step = it_M_step + 1;

        } while (update_mu_sig_sqr == TRUE);

#ifndef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT
        /*
        int plot_id = plot_open();
        ERE(plot_vector(plot_id, plot_tau_sig_sqr_ratio_grad_vp, 0.0, 0.1, NULL));
        */
        ERE(write_col_vector(plot_tau_sig_sqr_ratio_grad_vp, "tau_sig_sqr_ratio_grad_vp"));
        break;
#endif
        
        if (it > 0) 
        {
            diff = log_likelihood - prev_log_likelihood;

            diff *= 2.0;
            diff /= (ABS_OF(log_likelihood + prev_log_likelihood));

            verbose_pso(3, "%-3d: Log likelihood is %12e  |  %10e\n", 
                        it + 1, log_likelihood, diff); 


            held_out_diff = held_out_log_likelihood - prev_held_out_log_likelihood;

            held_out_diff *= 2.0;
            held_out_diff /= (ABS_OF(held_out_log_likelihood + prev_held_out_log_likelihood));
            
            verbose_pso(3, "%-3d: Held out log likelihood is %12e  |  %10e\n",
                        it + 1, held_out_log_likelihood, held_out_diff); 

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
    
    if ((result != ERROR) && (mu_ptr != NULL))
    {
        *mu_ptr = mu;
    }

    if ((result != ERROR) && (sig_sqr_ptr != NULL))
    {
        *sig_sqr_ptr = sig_sqr;
    }

    if ((result != ERROR) && (tau_sig_sqr_ratio_ptr != NULL))
    {
        *tau_sig_sqr_ratio_ptr = tau_sig_sqr_ratio;
    }
    
    free_vector(I_vp); 
    free_vector(p_sum_vp);
    free_vector(a_vp);
#ifdef DONT_USE_GAUSS_ROUTINE
    free_vector(log_sqrt_det_vp);
#endif 

    free_vector(x_vp); 
    free_vector(x_sub_vp);
    free_vector(u_vp); 
    free_vector(centered_x_vp);
    free_vector(ones_vp);
    free_vector(temp_vp);

#ifdef REGRESS_DO_FIXED_IND_CON_EM_GUTS 
    free_matrix(I_mp); 
#endif
    free_matrix(I_1_mp);

#ifndef DONT_PLOT_GRAD_BUT_DO_GRAD_ASCENT   
    free_vector(plot_tau_sig_sqr_ratio_grad_vp);
    free_vector(plot_log_likelihood_vp);
    free_vector(log_p_vp);
    free_vector(plot_exp_comp_log_likelihood_vp);
#endif

    free_matrix_vector(S_tilda_inv_mvp);
    free_matrix(S_inv_mp);
    free_matrix(temp_mp);

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

#ifdef __cplusplus
}
#endif

