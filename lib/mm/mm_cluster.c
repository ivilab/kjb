
/* $Id: mm_cluster.c 22174 2018-07-01 21:49:18Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 2000-2008 by Kobus Barnard (author), UCB, and  UA. Currently
|  the use of this code is retricted to the UA and UCB image and text projects.
|
* =========================================================================== */

#ifdef TEST
/*
#    define TRACK_OVERRUN_BUG
*/
#endif


/*
 * TODO --- Technical
 *   Try adjustment to variance computation due to weighting
 *   Play more with clustering based on new topology.
 *
// TODO --- Lots of cleanup, etc.
//   Check things that are no good, and put them in #defines which have prefix
//   XXX_ which will indicate that the option is dead (or perhaps ARCHIVE_).
//   (USE VIM FOLDING !!!). Ones to check:
//       fs_double_depend
//       fs_multiply_vertical_prior_weights
//
//       fs_trim* (WAS fs_null*). At least seperate con from dis! Also, I don't
//       think the calculationsare correct. We really only need P(b) and P(w) to
//       make a trim decision. HOWEVER, what we are trying to accomplish is more
//       or less already accomplished with the statistical formulation, except
//       in the correspondence case. If P(b) is small, then P(
//
//    ------------------------------------------------------------------------
//
//    fs_max_con_norm_var is a misnomer; it is the max normalized distance
//    squared.
//
//    Revisit annealing (check that it works!). Can it be an option without
//    creating too much expense? Does it work?
//
//    Perhaps turf min_con_log_prob. Currently it is not used. It would have to
//    be turfed from the model typedef.
//
//    If we track con_score_cutoff, then we should track dis_score_cutoff for
//    completeness, although it is not as important.
//
//    As for tracking the cutoff--since we want to cut based on conditional,
//    then we need to verify the relationship between p(b) and p(w|b, NULL), and
//    perhaps identify a cutoff which gives a better prediction than the
//    empirical distribution?
//
//    Rationalize normalizing ll by num points througout (held out, saved with
//    model, etc).
//
//    Better job of ll in the case of nulls (including con_log_norm)! For
//    example, when doing nulls and correspondance, there is a big dip in the
//    ll when nulls start.
//
//    Likely want to remove the hacks to implement Nando's Gaussian model (this
//    was never finished, and he now has an implementation).
//
//    Modularize?
//
//    Loose "em" from names. The caller does not care how it is done. Tommorow
//    it may be MCMC (by option).
//
//    Need to setup and test simple init strategies (modularization would help
//    here).
//
//    Need to revisit and test annealing.
//
//    Need to try stochastic.
//
//    Need to revisit and test Bayes.
//
//    If possible, get_cluster_membership_2, would be better as a special case
//    of the main clustering code.
//
//    Eventually get rid of phase stuff, once more powerful stuff is in place
//    and works (e.g., NULLs, topology adjustments, etc.).
//
//    Topology learner:
//        In the long run, we would like to have an arbitrary tree, with node
//        creation, deletion, and splitting. If nulls work, then increasing the
//        percentage to be modeled would correspond to creating nodes at the
//        leafs across the board.
//
//
*/


#include "m/m_gen.h"      /* Only safe if first #include in a ".c" file  */

#include "m/m_stat.h"
#include "m/m_mat_metric.h"
#include "m/m_vec_metric.h"

#include "p/p_plot.h"

#include "r/r_cluster_lib.h"
#include "mm/mm_cluster.h"
#include "mm/mm_cluster_lib.h"
#include "mm/mm_compute_con_prob.h"

/*
#define XXX_SUPPORT_TRIMMING    (buggy?)
#define XXX_XXX_OLD_PAIR_NULL_METHOD

#define HOW_IT_WAS_OCT_22_02
#define OLD_INFERENCE

#define USE_STANDARD_GUASS_NORM_CONSTANT
*/

/*
#define XXX_SUPPORT_LIMITING_NUM_FEATURES
*/

/*
*/

/*
#ifdef TEST
#    ifdef PROGRAMMER_IS_kobus
#        define REPORT_MEMORY_USE
#    endif
#endif
*/


#include "m/m_missing.h"

#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------------------------------------------------------- */


/*
#define SIMPLE_CLUSTER_INIT
*/
#define STANDARD_CLUSTER_INIT


/*
// This verifies that using norm_stdev = 1, gives the same result as the
// one used for most work (1/e). The log likelihood is different, but the model
// output is exactly the same. The likelihood jumps arround differently--I
// assume this just reflect the change in the relative contributions of text
// and blob data.
//
//
*/

/*
#define CHECK_NORM_STDEV
*/

#define REPORT_CPU_TIME

/*
#define TEST_WEIGHT_DATA
*/


/*
#define SUPPORT_IGNORE_DIS_PROB
*/

/*
// If this is set too small, then likely we need to use logs for discrete
// probabilities which is slow and has been relagated to the archives because
// it was getting too hard to synchronize it with other changes.
*/
#define MIN_DIS_PROB_FOR_OFFSET (0.000001)
/*
*/

/*
#define SUPPORT_ANNEALING
*/


#define MIN_LOG_PROB_CLUSTER_GIVEN_POINT ((-1e-10)*DBL_MAX)
/*
*/


#define MIN_HORIZONTAL_INDICATOR (1000.0 * DBL_EPSILON)

/*
 * This must be small enough that
 *        MIN_LEVEL_INDICATOR * num_levels << 1.0
*/
#define MIN_LEVEL_INDICATOR      (1000.0 * DBL_EPSILON)

/*
#define XXX_LIMIT_MIN_CON_PROB
*/

/* -------------------------------------------------------------------------- */

typedef struct Training_options
{
    Convergence_criterion convergence_criterion;
    int                   initial_model_is_valid;
    int                   initial_model_is_subset;
    int                   num_limited_clusters;
    int                   uniform_vertical_dist;
    int                   cluster_dependent_vertical_dist;
    int                   model_correspondence;
    int                   share_matches;
    int                   sample_matches;
    int                   last_sample;
    double                trim_fraction; /* Only used if XXX_SUPPORT_TRIMMING */
}
Training_options;

/* -------------------------------------------------------------------------- */

static int* fs_vector_feature_counts_ptr = NULL;
static int  fs_vector_feature_counts_set = FALSE;

#ifdef XXX_SUPPORT_LIMITING_NUM_FEATURES
static int    fs_max_num_con_features  = NOT_SET;
#endif

static int    fs_min_num_levels    = NOT_SET;
#ifdef XXX_SUPPORT_TRIMMING
static double fs_min_trim_fraction = DBL_NOT_SET;
static double fs_max_trim_fraction = DBL_NOT_SET;
#endif

#ifdef SUPPORT_IGNORE_DIS_PROB
/*
// This was a start at an early attempt to model a global percentage of the
// words which are emmitted as a the result of a noise process. Doing so
// involves estimating at any given iteration the prob that a given word was
// emmitted in this way. I never figured out a good way to do this, an thus
// this option remains disabled.
*/
static double fs_ignore_dis_prob_fraction = 0.0;
#endif

static int fs_multiply_vertical_prior_weights = FALSE;   /* OBSOLETE ?? */
static int fs_multiply_vertical_depend_weights = FALSE;   /* OBSOLETE ?? */

static int fs_initialize_method = 2;
static int fs_sample_hack = 1;

#ifdef XXX_SUPPORT_TRIMMING
static int fs_trim_hack = 2;
#endif

static int fs_discrete_tie_count = 1;
static int fs_tie_var = FALSE;
static int fs_weight_data = FALSE;
static int fs_add_discrete_nulls = FALSE;
static int fs_save_model_data = FALSE;

static int fs_phase_one_model_correspondence = 0;
static int fs_phase_two_model_correspondence = 0;
static int fs_phase_one_last_sample = NOT_SET;
static int fs_phase_two_last_sample = NOT_SET;
static int fs_phase_one_sample_matches = FALSE;
static int fs_phase_two_sample_matches = FALSE;
static int fs_phase_one_share_matches = FALSE;
static int fs_phase_two_share_matches = FALSE;

static int fs_phase_one_num_iterations    = 0;
static int fs_phase_two_num_iterations    = 1;
static int fs_phase_one_uniform_vertical_dist = FALSE;
static int fs_phase_two_uniform_vertical_dist = FALSE;
static int fs_phase_one_cluster_dependent_vertical_dist = FALSE;
static int fs_phase_two_cluster_dependent_vertical_dist = FALSE;

static Convergence_criterion fs_phase_one_convergence_criterion = MAX_HELD_OUT_LL;

static int fs_phase_one_held_out_data_is_fixed = FALSE;

static Convergence_criterion fs_phase_two_convergence_criterion = MAX_DATA_LL;

static int    fs_double_depend            = FALSE;
static int    fs_pair_depend              = FALSE;
static int    fs_max_depend               = FALSE;

static int    fs_norm_depend              = TRUE;
static int    fs_normalize_data           = FALSE;
static int    fs_norm_con_prob            = TRUE;
#ifdef XXX_SUPPORT_TRIMMING
static int    fs_trim_mach_two            = TRUE;
#endif
static int    fs_norm_vec_prob            = TRUE;
static double fs_var_offset               = 0.0001;
static double fs_var_offset_2             = 0.0;
static double fs_max_con_norm_var         = DBL_NOT_SET;
#ifdef XXX_LIMIT_MIN_CON_PROB
static double fs_min_con_log_prob_cutoff  = DBL_NOT_SET;
#endif
static double fs_data_perturbation        = DBL_NOT_SET;
static int    fs_num_tries_per_topology   = 1;
static int    fs_max_num_em_iterations    = 10;
static int    fs_min_num_em_iterations    = NOT_SET;
static double fs_iteration_tolerance      = 1.0e-05;
static double fs_dis_item_prob_threshold  = DBL_NOT_SET;
static int    fs_num_limited_clusters     = NOT_SET;
#ifdef DEF_OUT
static int    fs_num_passes               = 1;
static int    fs_max_num_pass_one_points  = 2000;
static double fs_pass_one_fraction        = 0.1;
#endif
static double fs_phase_one_train_fraction    = 0.7;
static double fs_phase_one_held_out_fraction = 0.3;
static double fs_dis_prob_boost_factor    = 1.0;
static double fs_con_prob_boost_factor    = 1.0;
static double fs_vec_prob_boost_factor    = 1.0;
static double fs_his_prob_boost_factor    = 1.0;
static int    fs_norm_items               = TRUE;
static int    fs_norm_continuous_features = FALSE;
static int    fs_do_map                   = FALSE;
static double fs_map_alpha                = 1.0;
static double fs_map_beta_minus_one       = 1.0;
static double fs_map_kappa                = 0.0;
static double fs_map_delta                = 1.0;
static double fs_map_r_minus_n_g          = 0.0;

#ifdef I_FORGET_WHAT_THIS_IS_FOR
static int    fs_compute_uniform_vertical_log_likelihood = FALSE;
#endif

static double fs_max_ave_limited_cluster_error = 0.000001;

#ifdef  SUPPORT_ANNEALING
static int    fs_num_cooling_iterations   = NOT_SET;
static double fs_min_temperature          = 0.2;
static double fs_max_temperature          = 1.0;
static int fs_watch_temperature = TRUE;
#endif


static int fs_watch = TRUE;
#ifdef TEST
static int fs_watch_2 = TRUE;
#else
static int fs_watch_2 = FALSE;
#endif
static int fs_watch_max_con_prob = FALSE;
#ifdef VEC_AND_HIS
static int fs_watch_max_vec_prob = FALSE;
#endif
static int fs_watch_max_parm_diff = TRUE;

/* Can't support these for a bit. */
static int fs_watch_max_cluster_given_point_diff = FALSE;
static int fs_watch_rms_cluster_given_point_diff = FALSE;

static int fs_watch_ave_limited_error = TRUE;
static int fs_watch_max_limited_error = TRUE;

static int fs_use_level_prior  = TRUE;
static int fs_use_blob_prior  = TRUE;

static double***       fs_cached_level_indicators_ppp = NULL;
static Indexed_vector* fs_cached_cluster_index_vp  = NULL;
static V_v_v*          fs_con_prob_cache_vvvp         = NULL;
static Int_vector*     fs_level_counts_vp          = NULL;

static int fs_weight_con_items = FALSE;

static int fs_min_it_for_con_items = NOT_SET;
static int fs_min_it_for_dis_items = NOT_SET;
static int fs_sample_count = NOT_SET;

static int fs_max_num_level_weights = 100;

static double fs_limit_gaussian_deviation = DBL_NOT_SET;

static Int_vector* fs_sub_group_vp = NULL;
static int fs_use_specified_clusters = FALSE;
static int fs_one_to_one_match = FALSE;
static int fs_use_contiguous_assignment = FALSE;
static int fs_duplicate_words_for_matching = FALSE;

static int fs_emulate_uniform_discrete_prob = FALSE;

/* -------------------------------------------------------------------------- */

static int do_multi_modal_clustering_2
(
    const Topology*     topology_ptr,
    const Int_vector*   dis_item_level_vp,
    const Cluster_data* data_ptr,
    const Cluster_data* held_out_data_ptr,
    Multi_modal_model* model_ptr,
    int                 (*output_model_fn)(const Multi_modal_model*, int, const char*),
    const char*         output_dir,
    const Vector*       norm_mean_vp,
    const Vector*       norm_var_vp,
    double              norm_stdev,
    double              (*dis_item_loss_fn) (int, const double*)
);

static void set_default_training_options
(
    Training_options* training_options_ptr
);

static int do_multi_modal_clustering_3
(
    const Topology*     topology_ptr,
    const Int_vector*   dis_item_level_vp,
    const Cluster_data* data_ptr,
    const Cluster_data* held_out_data_ptr,
    Multi_modal_model* model_ptr,
    const Training_options* options_ptr,
    int                 (*output_model_fn) (const Multi_modal_model *, int, const char *),
    const char*         output_dir,
    const Vector*       norm_mean_vp,
    const Vector*       norm_var_vp,
    double              norm_stdev,
    int                 max_num_em_iterations,
    double              (*dis_item_loss_fn)(int, const double*)
);

static int do_multi_modal_clustering_4
(
    const Topology*         topology_ptr,
    const Int_vector*       dis_item_level_vp,
    const Cluster_data*     data_ptr,
    const Cluster_data*     held_out_data_ptr,
    Multi_modal_model*     model_ptr,
    const Training_options* options_ptr,
    int                     (*output_model_fn) (const Multi_modal_model*,                                                               int, const char*),
    const char*             output_dir,
    const Vector*           norm_mean_vp,
    const Vector*           norm_var_vp,
    double                  norm_stdev,
    int                     max_num_em_iterations,
    double                  (*dis_item_loss_fn) (int, const double*)


);

static int do_fixed_discrete_multi_modal_clustering
(
    const Topology*         topology_ptr,
    const Cluster_data*     data_ptr,
    const Cluster_data*     held_out_data_ptr,
    Multi_modal_model*     model_ptr,
    const Training_options* options_ptr,
    int                     (*output_model_fn) (const Multi_modal_model *, int, const char *),
    const char*             output_dir,
    const Vector*           norm_mean_vp,
    const Vector*           norm_var_vp,
    double                  norm_stdev,
    int                     max_num_em_iterations
);

static int get_cluster_membership_2
(
    Matrix**             P_c_p_mpp,
    Matrix_vector**      P_l_p_c_mvpp,
    Int_matrix_vector**  cluster_level_index_mvpp,
    Matrix**             P_l_p_mpp,
    Int_matrix**         level_index_mpp,
    Vector**             fit_vp_ptr,
    const Topology*      topology_ptr,
    const Vector*        a_vp,
    const Matrix*        V_mp,
    const Matrix*        input_P_c_p_mp,
    const Matrix_vector* input_P_l_p_c_mvp,
    const Matrix*        input_P_l_p_mp,
    const Matrix_vector* P_i_n_mvp,
    const Matrix_vector* mean_mvp,
    const Matrix_vector* var_mvp,
    const Vector_vector* con_log_sqrt_det_vvp,
    double               con_log_norm,
    const Int_vector*    con_feature_enable_vp,
    const Int_vector*    con_feature_group_vp,
    double               min_con_log_prob,
    double               max_con_norm_var,
    double               con_score_cutoff,
    int                  weight_con_items,
    int                  norm_depend,
    int                  model_correspondence,
    const Cluster_data*  data_ptr,
    const Int_vector*    points_to_use_vp,
    const Vector*        dis_factor_vp,
    const Vector*        con_factor_vp,
    int                  norm_continuous_features,
    double               iteration_tolerance,
    double               prev_log_likelihood,
    double*              log_likelihood_ptr,
    const Matrix*        test_P_c_p_mp,
    int                  test_data_flag,
    Vector_vector*       con_score_vpp
);

static int initialize_vector_feature_counts
(
    const Int_vector* con_feature_enable_vp,
    const Int_vector* con_feature_group_vp
);

static int get_dis_factors
(
    Vector**          factor_vp_ptr,
    const Int_matrix* item_mp,
    const Matrix*     dis_item_multiplicity_mp,
    double            max_num_dis_items_with_multiplicity,
    int               norm_items,
    double            prob_boost_factor
);

static int get_con_factors
(
    Vector**             factor_vp_ptr,
    const Matrix_vector* item_mvp,
    int                  max_num_items,
    int                  norm_items,
    double               prob_boost_factor
);

#ifdef VEC_AND_HIS
static int get_vec_factors
(
    Vector**       factor_vp_ptr,
    const V_v_v_v* item_vvvvp,
    int            max_num_items,
    int            norm_items,
    double         prob_boost_factor
);

static int get_his_factors
(
    Vector**       factor_vp_ptr,
    const V_v_v_v* item_vvvvp,
    int            max_num_items,
    int            norm_items,
    double         prob_boost_factor
);
#endif

static int initialize_con_prob_cache
(
    int               max_num_con_items,
    int               first_level,
    int               last_level_num,
    int               num_levels,
    const Int_vector* level_counts_vp
);

static int initialize_level_indicators_cache
(
    int num_levels,
    int num_clusters,
    int num_items
);

static int get_limited_cluster_membership
(
    Int_matrix*   cluster_index_mp,
    const Vector* P_c_p_vp,
    int           point,
    int           num_limited_clusters,
    double*       error_ptr
);

#ifdef XXX_SUPPORT_TRIMMING
static void sum_collected_scores
(
    int               num_items,
    int               base_count,
    const Int_vector* score_counts_vp,
    const Matrix*     score_mp,
    Vector*           score_vp
);
#endif

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

/* -------------------------------------------------------------------------- */

int set_multi_modal_cluster_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int    result             = NOT_FOUND;
    int    temp_int_value;
    int    temp_boolean_value;
    double temp_double_value;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

#ifdef NOT_FINISHED
    /*
     * Should we switch to something else?
    */

    /*
     * Legacy spellings.
    */
    if (HEAD_CMP_EQ(lc_option, "hc-"))
    {
        static int first_time = TRUE;

        if (first_time)
        {
            warn_pso("Options begining with \"hc-\" should be changed to \"mm-\"\n");
            first_time = FALSE;
        }

        lc_option[ 0 ] = 'm';
        lc_option[ 1 ] = 'm';
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-weight-con-items")
          /* Legacy spellings. */
          || match_pattern(lc_option, "hc-weight-by-area")
          || match_pattern(lc_option, "hc-weight-con-items-by-first-feature")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC con items %s weighted.\n",
                    fs_weight_con_items ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-weight-con-items = %s\n",
                    fs_weight_con_items ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_weight_con_items = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-mult-vert-depend-weights")
          || match_pattern(lc_option, "hc-multiply-vertical-depend-weights")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC vertical depend weights %s multiplied.\n",
                    fs_multiply_vertical_depend_weights ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-multiply-vertical-depend-weights = %s\n",
                    fs_multiply_vertical_depend_weights ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_multiply_vertical_depend_weights = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-mult-vert-prior-weights")
          || match_pattern(lc_option, "hc-multiply-vertical-prior-weights")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC vertical prior weights %s multiplied.\n",
                    fs_multiply_vertical_prior_weights ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-multiply-vertical-prior-weights = %s\n",
                    fs_multiply_vertical_prior_weights ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_multiply_vertical_prior_weights = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-one-model-correspondence")
          || match_pattern(lc_option, "hc-phase-one-correspondence")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Phase one correspondence learning parameter is %d.\n",
                    fs_phase_one_model_correspondence));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-one-correspondence = %d\n",
                    fs_phase_one_model_correspondence));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));

            if ((temp_int_value >= 0) && (temp_int_value < 6))
            {
                fs_phase_one_model_correspondence = temp_int_value;
            }
            else
            {
                set_error("%q is an invalid correspondence learning specification.",
                          value);
                return ERROR;
            }
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-two-model-correspondence")
          || match_pattern(lc_option, "hc-phase-two-correspondence")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Phase two correspondence learning parameter is %d.\n",
                    fs_phase_two_model_correspondence));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-two-correspondence = %d\n",
                    fs_phase_two_model_correspondence));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));

            if ((temp_int_value >= 0) && (temp_int_value < 6))
            {
                fs_phase_two_model_correspondence = temp_int_value;
            }
            else
            {
                set_error("%q is an invalid correspondence learning specification.",
                          value);
                return ERROR;
            }
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "phase-one-last-sample")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_phase_one_last_sample > 0)
            {
                ERE(pso("HC correspondence learing phase one -- sampling is not used after iteration %d.\n",
                        fs_phase_one_last_sample));
            }
            else if (fs_phase_one_last_sample < 0)
            {
                ERE(pso("HC correspondence learing phase one -- sampling is not stopped.\n"));
            }
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("phase-one-last-sample = %d\n",
                    fs_phase_one_last_sample));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));
            fs_phase_one_last_sample = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "phase-two-last-sample")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_phase_two_last_sample > 0)
            {
                ERE(pso("HC correspondence learing phase two -- sampling is not used after iteration %d.\n",
                        fs_phase_two_last_sample));
            }
            else if (fs_phase_two_last_sample < 0)
            {
                ERE(pso("HC correspondence learing phase two -- sampling is not stopped.\n"));
            }
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("phase-two-last-sample = %d\n",
                    fs_phase_two_last_sample));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));
            fs_phase_two_last_sample = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-sub-groups")
       )
    {
#ifdef TRACK_MEMORY_ALLOCATION
        prepare_memory_cleanup();
#endif

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            db_irv(fs_sub_group_vp);
        }
        else if (value[ 0 ] == '?')
        {
            db_irv(fs_sub_group_vp);
        }
        else
        {
            ERE(sget_positive_int_vector(&fs_sub_group_vp, value));
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-use-specified-clusters")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC specified clusters %s used.\n",
                    fs_use_specified_clusters ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-use-specified-clusters = %s\n",
                    fs_use_specified_clusters ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_use_specified_clusters = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-use-contiguous-assignment")
          || match_pattern(lc_option, "hc-use-contig-assignment")
          || match_pattern(lc_option, "hc-use-contig")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC contiguous assignment %s used.\n",
                    fs_use_contiguous_assignment ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-use-contiguous-assignment = %s\n",
                    fs_use_contiguous_assignment ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_use_contiguous_assignment = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-one-to-one-match")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC one to one matched %s used.\n",
                    fs_one_to_one_match ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-one-to-one-match = %s\n",
                    fs_one_to_one_match ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_one_to_one_match = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-initialize-method")
          || match_pattern(lc_option, "hc-initialization-method")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC initialize method is %d.\n",
                    fs_initialize_method));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-initialize-method = %d\n", fs_initialize_method));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));
            fs_initialize_method = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-sample-hack")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC sample hack is %d.\n",
                    fs_sample_hack));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-sample-hack = %d\n", fs_sample_hack));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));

            if ((temp_int_value < 0) || (temp_int_value > 2))
            {
                set_error("HC sample hack must be between 0 and 2.\n");
                return ERROR;
            }
            else
            {
                fs_sample_hack = temp_int_value;
            }
        }
        result = NO_ERROR;
    }

#ifdef XXX_SUPPORT_TRIMMING
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-trim-hack")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC trim hack is %d.\n",
                    fs_trim_hack));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-trim-hack = %d\n", fs_sample_hack));
        }
        else
        {
            ERE(ss1i(value, &temp_int_value));

            if ((temp_int_value < 0) || (temp_int_value > 2))
            {
                set_error("HC trim hack must be between 0 and 2.\n");
                return ERROR;
            }
            else
            {
                fs_trim_hack = temp_int_value;
            }
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-one-sample-matches")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC correspondence learing phase one -- matches %s sampled.\n",
                    fs_phase_one_sample_matches ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-one-sample-matches = %s\n",
                    fs_phase_one_sample_matches ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_phase_one_sample_matches = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-two-sample-matches")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC correspondence learing phase two -- matches %s sampled.\n",
                    fs_phase_two_sample_matches ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-two-sample-matches = %s\n",
                    fs_phase_two_sample_matches ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_phase_two_sample_matches = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-one-share-matches")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC correspondence learing phase one -- matches %s shared.\n",
                    fs_phase_one_share_matches ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-one-share-matches = %s\n",
                    fs_phase_one_share_matches ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_phase_one_share_matches = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-two-share-matches")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC correspondence learing phase two -- matches %s shared.\n",
                    fs_phase_two_share_matches ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-two-share-matches = %s\n",
                    fs_phase_two_share_matches ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_phase_two_share_matches = temp_boolean_value;

            if ( ! fs_phase_two_share_matches)
            {
                warn_pso("Option to not share matches is dubious!\n");
            }
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-watch")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC iterations %s displayed.\n",
                    fs_watch ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-watch = %s\n",
                    fs_watch ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_watch = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-watch-2")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC sub-iterations %s displayed.\n",
                    fs_watch_2 ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-watch = %s\n",
                    fs_watch_2 ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_watch_2 = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-map-alpha")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC MAP alpha is %.3e.\n", fs_map_alpha));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-map-alpha = %.3e\n", fs_map_alpha));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_map_alpha = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-map-beta-minus-one")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC MAP beta_minus_one is %.3e.\n", fs_map_beta_minus_one));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-map-beta-minus-one = %.3e\n", fs_map_beta_minus_one));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_map_beta_minus_one = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-map-kappa")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC MAP kappa is %.3e.\n", fs_map_kappa));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-map-kappa = %.3e\n", fs_map_kappa));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_map_kappa = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-map-delta")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC MAP delta is %.3e.\n", fs_map_delta));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-map-delta = %.3e\n", fs_map_delta));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_map_delta = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-map-r-minus-n-g")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC MAP r minus number of dimensions is %.3e.\n",
                    fs_map_r_minus_n_g));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-map-r-minus-n-g = %.3e\n", fs_map_r_minus_n_g));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_map_r_minus_n_g = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-var-offset")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC variance offset is %.3e.\n", fs_var_offset));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-var-offset = %.3e\n", fs_var_offset));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_var_offset = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-var-offset-2")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC variance offset 2 is %.3e.\n", fs_var_offset_2));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-var-offset-2 = %.3e\n", fs_var_offset_2));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_var_offset_2 = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-con-norm-var")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_con_norm_var <= 0.0)
            {
                ERE(pso("HC max con norm var is not used.\n"));
            }
            else
            {
                ERE(pso("HC max con norm var is %.3e.\n",
                        fs_max_con_norm_var));
            }
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_con_norm_var <= 0.0)
            {
                ERE(pso("hc-max-con-norm-var = off\n"));
            }
            else
            {
                ERE(pso("hc-max-con-norm-var = %.3e\n",
                        fs_max_con_norm_var));
            }
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_max_con_norm_var = temp_double_value;
        }
        result = NO_ERROR;
    }

#ifdef XXX_LIMIT_MIN_CON_PROB
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-min-log-con-prob-factor")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC min log con prob cutoff is %.3e.\n",
                    fs_min_con_log_prob_cutoff));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-min-log-con-prob-curoff = %.3e\n",
                    fs_min_con_log_prob_cutoff));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_min_con_log_prob_cutoff = temp_double_value;
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-depend")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC max depend %s used.\n",
                    fs_max_depend ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-max-depend = %s\n",
                    fs_max_depend ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_max_depend = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-pair-depend")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC pair depend %s used.\n",
                    fs_pair_depend ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-pair-depend = %s\n",
                    fs_pair_depend ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_pair_depend = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-double-depend")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC double depend %s used.\n",
                    fs_double_depend ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-double-depend = %s\n",
                    fs_double_depend ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_double_depend = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-norm-depend")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC norm depend %s used.\n",
                    fs_norm_depend ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-norm-depend = %s\n",
                    fs_norm_depend ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_norm_depend = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-norm-data")
          || match_pattern(lc_option, "hc-norm-data-flag")
          || match_pattern(lc_option, "hc-normalize-data")
          || match_pattern(lc_option, "hc-normalize-data-flag")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC data %s normalized.\n",
                    fs_normalize_data ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-normalize-data = %s\n",
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
          || match_pattern(lc_option, "hc-discrete-tie-count")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC discrete tie count is %d.\n",
                    fs_discrete_tie_count));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-discrete-tie-count = %d\n",
                    fs_discrete_tie_count));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));

            if (temp_int_value == 0)
            {
                set_error("Discrete tie count must be at least one.");
                return ERROR;
            }
            fs_discrete_tie_count = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-tie-var")
          || match_pattern(lc_option, "hc-tie-var-flag")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC var %s tied.\n",
                    fs_tie_var ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-tie-var = %s\n",
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
          || match_pattern(lc_option, "hc-weight-data")
          || match_pattern(lc_option, "hc-weight-data-flag")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC data %s weighted.\n",
                    fs_weight_data ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-weight-data = %s\n",
                    fs_weight_data ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_weight_data = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-add-discrete-nulls")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC discrete nulls %s added.\n",
                    fs_add_discrete_nulls ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-add-discrete-nulls = %s\n",
                    fs_add_discrete_nulls ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_add_discrete_nulls = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-save-model-data")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC model data %s saveed.\n",
                    fs_save_model_data ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-save-model-data = %s\n",
                    fs_save_model_data ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_save_model_data = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-norm-con-prob")
          || match_pattern(lc_option, "hc-norm-con-probs")
          || match_pattern(lc_option, "hc-normalize-continuous-probabilities")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC continuous probabilities %s normalized.\n",
                    fs_norm_con_prob ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-norm-con-prob = %s\n",
                    fs_norm_con_prob ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_norm_con_prob = temp_boolean_value;
        }
        result = NO_ERROR;
    }

#ifdef XXX_SUPPORT_TRIMMING
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-trim-mach-two")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC trim mach two %s used.\n",
                    fs_trim_mach_two ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-trim-mach-two = %s\n",
                    fs_trim_mach_two ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_trim_mach_two = temp_boolean_value;
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-norm-vec-prob")
          || match_pattern(lc_option, "hc-norm-vec-probs")
          || match_pattern(lc_option, "hc-normalize-vector-probabilities")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC vector probabilities %s normalized.\n",
                    fs_norm_vec_prob ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-norm-vec-prob = %s\n",
                    fs_norm_vec_prob ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_norm_vec_prob = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-data-perturbation")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC data perturbation is %.3e.\n", fs_data_perturbation));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-data-perturbation = %.3e\n", fs_data_perturbation));
        }
        else if (is_no_value_word(value))
        {
            fs_data_perturbation = DBL_NOT_SET;
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_data_perturbation = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-discrete-threshold")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC threshold for excluding discrete probabilities %.3e.\n",
                    fs_dis_item_prob_threshold));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-discrete-threshold = %.3e\n",
                    fs_dis_item_prob_threshold));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_dis_item_prob_threshold = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-num-tries-per-topology")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-num-tries-per-toplogy = %d\n",
                    fs_num_tries_per_topology));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Each toplogy gets %d EM tries.\n",
                    fs_num_tries_per_topology));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            if (temp_int_value == 0)
            {
                set_error("Number of tries per cluster count must be at least one.");
                return ERROR;
            }
            fs_num_tries_per_topology = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-cluster-membership")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_num_limited_clusters > 0)
            {
                ERE(pso("hc-max-cluster-membership = %d\n",
                        fs_num_limited_clusters));
            }
            else
            {
                ERE(pso("hc-max-cluster-membership = off\n"));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_num_limited_clusters > 0)
            {
                ERE(pso("Cluster membership is limited to %d clusters.\n",
                        fs_num_limited_clusters));
            }
            else
            {
                ERE(pso("Cluster membership is not limited.\n"));
            }
        }
        else
        {
            ERE(ss1pi_2(value, &temp_int_value));
            fs_num_limited_clusters = temp_int_value;
        }
        result = NO_ERROR;
    }

#ifdef DEF_OUT
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-number-of-passes")
          || match_pattern(lc_option, "hc-num-passes")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-num-passes = %d\n",
                    fs_num_passes));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC clustering uses %d pass%s.\n",
                    fs_num_passes, (fs_num_passes == 2) ? "es" : ""));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));

            if ((temp_int_value != 1) && (temp_int_value != 2))
            {
                set_error("Currently the number of passes must be 1 or 2.\n");
                return ERROR;
            }
            fs_num_passes = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-num-pass-one-points")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-max-num-pass-one-points = %d\n",
                    fs_max_num_pass_one_points));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("A maximum of %d points is used in HC pass one.\n",
                    fs_max_num_pass_one_points));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));

            if (temp_int_value < 0)
            {
                set_error("The max number of pass one points must be non-negative.\n");
                return ERROR;
            }
            fs_max_num_pass_one_points = temp_int_value;
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-one-hold-out-fraction")
          || match_pattern(lc_option, "hc-phase-one-held-out-fraction")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d%% of the points are held out in phase one training.\n",
                    (int)(fs_phase_one_held_out_fraction * 100)));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-one-hold-out-fraction = %d%%\n",
                    (int)(fs_phase_one_held_out_fraction * 100)));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));

            if (temp_double_value < 0.0)
            {
                set_error("Phase one hold out fraction must be positive.");
                return ERROR;
            }
            fs_phase_one_held_out_fraction = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-one-train-fraction")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d%% of the points are used in phase one training.\n",
                    (int)(fs_phase_one_train_fraction * 100)));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-one-train-fraction = %d%%\n",
                    (int)(fs_phase_one_train_fraction * 100)));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));

            if (temp_double_value < 0.0)
            {
                set_error("Phase one training fraction must be positive.");
                return ERROR;
            }
            fs_phase_one_train_fraction = temp_double_value;
        }
        result = NO_ERROR;
    }

#ifdef SUPPORT_IGNORE_DIS_PROB
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-ignore-dis-prob-fraction")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d%% of the discrete probability is ignored.\n",
                    (int)(fs_ignore_dis_prob_fraction * 100)));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-ignore-dis-item-fraction = %d%%\n",
                    (int)(fs_ignore_dis_prob_fraction * 100)));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));

            if ((temp_double_value < 0.0) || (temp_double_value >= 1.0))
            {
                set_error("Ignore discrete prob fraction must be in [0,1).");
                return ERROR;
            }
            fs_ignore_dis_prob_fraction = temp_double_value;
        }
        result = NO_ERROR;
    }
#endif

#ifdef DEF_OUT
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-pass-one-fraction")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d%% of the points are fitted in pass one.\n",
                    (int)(fs_pass_one_fraction * 100)));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-pass-one-fraction = %d%%\n",
                    (int)(fs_pass_one_fraction * 100)));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));

            if (temp_double_value <= 0.0)
            {
                set_error("HC pass one fraction must be positive.");
                return ERROR;
            }
            fs_pass_one_fraction = temp_double_value;
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-num-iterations")
          || match_pattern(lc_option, "hc-max-num-em-iterations")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-max-num-em-iterations = %d\n",
                    fs_max_num_em_iterations));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("A maximum of %d EM iterations are used.\n",
                    fs_max_num_em_iterations));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_max_num_em_iterations = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-min-num-iterations")
          || match_pattern(lc_option, "hc-min-num-em-iterations")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-min-num-em-iterations = %d\n",
                    fs_min_num_em_iterations));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("A minimum of %d EM iterations are used.\n",
                    fs_min_num_em_iterations));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_min_num_em_iterations = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-one-num-iterations")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-one-num-iterations = %d\n",
                    fs_phase_one_num_iterations));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d phase one iterations are used in hierarchical clustering.\n",
                    fs_phase_one_num_iterations));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_phase_one_num_iterations = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-two-num-iterations")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-two-num-iterations = %d\n",
                    fs_phase_two_num_iterations));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d phase two iterations are used in hierarchical clustering.\n",
                    fs_phase_two_num_iterations));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_phase_two_num_iterations = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-discrete-prob-boost-factor")
          || match_pattern(lc_option, "hc-dis-prob-boost-factor")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-discrete-prob-boost-factor = %.5f\n",
                    fs_dis_prob_boost_factor));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Discrete probs are boosted by a factor of %.5f.\n",
                    fs_dis_prob_boost_factor));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));
            fs_dis_prob_boost_factor = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-continuous-prob-boost-factor")
          || match_pattern(lc_option, "hc-con-prob-boost-factor")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-continuous-prob-boost-factor = %.5f\n",
                    fs_con_prob_boost_factor));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Continuous probs are boosted by a factor of %.5f.\n",
                    fs_con_prob_boost_factor));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));
            fs_con_prob_boost_factor = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-vector-prob-boost-factor")
          || match_pattern(lc_option, "hc-vec-prob-boost-factor")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-vector-prob-boost-factor = %.5f\n",
                    fs_vec_prob_boost_factor));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC vector probs are boosted by a factor of %.5f.\n",
                    fs_vec_prob_boost_factor));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));
            fs_vec_prob_boost_factor = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-histogram-prob-boost-factor")
          || match_pattern(lc_option, "hc-his-prob-boost-factor")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-histogram-prob-boost-factor = %.5f\n",
                    fs_his_prob_boost_factor));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC histogram probs are boosted by a factor of %.5f.\n",
                    fs_his_prob_boost_factor));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));
            fs_his_prob_boost_factor = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-iteration-tolerance")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC iteration tolerance is %.3e.\n",
                    fs_iteration_tolerance));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-iteration-tolerance = %.3e\n", fs_iteration_tolerance));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_iteration_tolerance = temp_double_value;
        }
        result = NO_ERROR;
    }

#ifdef SUPPORT_ANNEALING
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-num-cooling-iterations")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-num-cooling-iterations = %d\n",
                    fs_num_cooling_iterations));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("%d HC iteratations are annealed.\n",
                    fs_num_cooling_iterations));
        }
        else
        {
            ERE(ss1pi_2(value, &temp_int_value));
            fs_num_cooling_iterations= temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-min-temperature")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC min temperature is %.3e.\n", fs_min_temperature));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-min-temperature = %.3e\n", fs_min_temperature));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_min_temperature = temp_double_value;
        }
        result = NO_ERROR;
    }

#ifdef DONT_ALLOW_CHANGES
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-temperature")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC max temperature is %.3e.\n", fs_max_temperature));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-max-temperature = %.3e\n", fs_max_temperature));
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_max_temperature = temp_double_value;
        }
        result = NO_ERROR;
    }
#endif
#endif

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-ave-limited-cluster-error")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_ave_limited_cluster_error >= 0.0)
            {
                ERE(pso("HC max ave limited cluster error is %.3e.\n",
                        fs_max_ave_limited_cluster_error));
            }
            else
            {
                ERE(pso("HC max ave limited cluster error is not set.\n"));

            }
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_ave_limited_cluster_error >= 0.0)
            {
                ERE(pso("hc-max-ave-limited-cluster-error = %.3e\n",
                        fs_max_ave_limited_cluster_error));
            }
            else
            {
                ERE(pso("hc-max-ave-limited-cluster-error = off\n"));
            }
        }
        else
        {
            ERE(ss1snd(value, &temp_double_value));
            fs_max_ave_limited_cluster_error = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-norm-item")
          || match_pattern(lc_option, "hc-norm-item-flag")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC item probability contributions %s normalized.\n",
                    fs_norm_items ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-norm-item-flag = %s\n",
                    fs_norm_items ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_norm_items = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-norm-con-features")
          || match_pattern(lc_option, "hc-norm-continuous-features")
          || match_pattern(lc_option, "hc-norm-con-feature-flag")
          || match_pattern(lc_option, "hc-norm-continuous-feature-flag")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC continuous feature probability contributrions %s normalized.\n",
                    fs_norm_continuous_features ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-norm-continuous-features = %s\n",
                    fs_norm_continuous_features ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_norm_continuous_features = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-one-uniform-vertical-dist")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Clusters %s fitted with average vertical distributions for phase one.\n",
                    fs_phase_one_uniform_vertical_dist ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-one-uniform-vertical-dist = %s\n",
                    fs_phase_one_uniform_vertical_dist ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_phase_one_uniform_vertical_dist = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option,
                           "hc-phase-one-cluster-dependent-vertical-dist")
          || match_pattern(lc_option,
                           "hc-phase-one-cluster-vertical-dist")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Clusters %s fitted with cluster dependent vertical distributions for phase one.\n",
                    fs_phase_one_cluster_dependent_vertical_dist ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-one-cluster-dependent-vertical-dist = %s\n",
                    fs_phase_one_cluster_dependent_vertical_dist ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_phase_one_cluster_dependent_vertical_dist = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-phase-two-uniform-vertical-dist")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Clusters %s fitted with average vertical distributions for phase two.\n",
                    fs_phase_two_uniform_vertical_dist ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-two-uniform-vertical-dist = %s\n",
                    fs_phase_two_uniform_vertical_dist ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_phase_two_uniform_vertical_dist = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option,
                           "hc-phase-two-cluster-dependent-vertical-dist")
          || match_pattern(lc_option,
                           "hc-phase-two-cluster-vertical-dist")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Clusters %s fitted with cluster dependent vertical distributions for phase two.\n",
                    fs_phase_two_cluster_dependent_vertical_dist ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-phase-two-cluster-dependent-vertical-dist = %s\n",
                    fs_phase_two_cluster_dependent_vertical_dist ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_phase_two_cluster_dependent_vertical_dist = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-do-map")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Bayesian priors %s used.\n",
                    fs_do_map ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-do-map = %s\n", fs_do_map ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_do_map = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-use-blob-prior")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Blob priors %s used if available.\n",
                    fs_use_blob_prior ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-use-blob-and-priors = %s\n",
                    fs_use_blob_prior ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_use_blob_prior = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-use-level-priors")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Level priors %s used if available.\n",
                    fs_use_level_prior ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-use-level-and-priors = %s\n",
                    fs_use_level_prior ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_use_level_prior = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-min-num-levels")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-min-num-levels = %d\n",
                    fs_min_num_levels));
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_min_num_levels > 0)
            {
                ERE(pso("HC training begins with %d levels.\n",
                        fs_min_num_levels));
            }
            else
            {
                ERE(pso("HC training only uses all levels.\n"));
            }
        }
        else
        {
            ERE(ss1pi_2(value, &temp_int_value));
            fs_min_num_levels = temp_int_value;
        }
        result = NO_ERROR;
    }

#ifdef XXX_SUPPORT_TRIMMING
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-min-trim-fraction")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_min_trim_fraction > 0.0)
            {
                ERE(pso("HC training begins with %d%% trims.\n",
                        (int)(fs_min_trim_fraction * 100)));
            }
            else
            {
                ERE(pso("hc-min-trim-fraction = off\n"));
            }
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_min_trim_fraction > 0.0)
            {
                ERE(pso("hc-min-trim-fraction = %d%%\n",
                        (int)(fs_min_trim_fraction * 100)));
            }
            else
            {
                ERE(pso("hc-min-trim-fraction = off\n"));
            }
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));
            fs_min_trim_fraction = temp_double_value;
        }
        result = NO_ERROR;
    }
#endif

#ifdef XXX_SUPPORT_TRIMMING
    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-trim-fraction")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_trim_fraction > 0.0)
            {
                ERE(pso("HC training ends with %d%% trims.\n",
                        (int)(fs_max_trim_fraction * 100)));
            }
            else
            {
                ERE(pso("hc-max-trim-fraction = off\n"));
            }
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_trim_fraction > 0.0)
            {
                ERE(pso("hc-max-trim-fraction = %d%%\n",
                        (int)(fs_max_trim_fraction * 100)));
            }
            else
            {
                ERE(pso("hc-max-trim-fraction = off\n"));
            }
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));
            fs_max_trim_fraction = temp_double_value;
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-sample-count")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_sample_count > 0)
            {
                ERE(pso("hc-sample-count = %d\n",
                        fs_sample_count));
            }
            else
            {
                ERE(pso("hc-sample-count = off\n"));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_sample_count > 0)
            {
                ERE(pso("HC sample count is %d .\n",
                        fs_sample_count));
            }
            else
            {
                ERE(pso("HC sampling (stochastic EM) is not used.\n"));
            }
        }
        else
        {
            ERE(ss1pi_2(value, &temp_int_value));
            fs_sample_count= temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-min-it-for-dis-items")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_min_it_for_dis_items > 0)
            {
                ERE(pso("hc-min-it-for-dis-items = %d\n",
                        fs_min_it_for_dis_items));
            }
            else
            {
                ERE(pso("hc-min-it-for-dis-items = off\n"));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_min_it_for_dis_items > 0)
            {
                ERE(pso("HC min iteration for dis items is %d .\n",
                        fs_min_it_for_dis_items));
            }
            else
            {
                ERE(pso("HC dis items are used on all iterations.\n"));
            }
        }
        else
        {
            ERE(ss1pi_2(value, &temp_int_value));
            fs_min_it_for_dis_items= temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-min-it-for-con-items")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_min_it_for_con_items > 0)
            {
                ERE(pso("hc-min-it-for-con-items = %d\n",
                        fs_min_it_for_con_items));
            }
            else
            {
                ERE(pso("hc-min-it-for-con-items = off\n"));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_min_it_for_con_items > 0)
            {
                ERE(pso("HC min iteration for con items is %d .\n",
                        fs_min_it_for_con_items));
            }
            else
            {
                ERE(pso("HC con items are used on all iterations.\n"));
            }
        }
        else
        {
            ERE(ss1pi_2(value, &temp_int_value));
            fs_min_it_for_con_items= temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-max-num-level-weights")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-max-num-level-weights = %d\n",
                    fs_max_num_level_weights));
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_num_level_weights > 0)
            {
                ERE(pso("The maximum number of level weights used is %d.\n",
                        fs_max_num_level_weights));
            }
            else
            {
                ERE(pso("All level weights are used.\n"));
            }
        }
        else
        {
            ERE(ss1pi_2(value, &temp_int_value));
            fs_max_num_level_weights = temp_int_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-limit-gaussian-deviation")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_limit_gaussian_deviation > 0.0)
            {
                ERE(pso("Gaussian distribution deviations are not limited.\n"));
            }
            else
            {
                ERE(pso("Gaussian distribution deviations are limited to %.2f.\n",
                        fs_limit_gaussian_deviation));
            }
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_limit_gaussian_deviation > 0.0)
            {
                ERE(pso("hc-limit-gaussian-deviation = %.2f\n",
                        fs_limit_gaussian_deviation));
            }
            else
            {
                ERE(pso("hc-limit-gaussian-deviation = off\n"));
            }

        }
        else if (is_no_value_word(value))
        {
            fs_limit_gaussian_deviation = DBL_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));
            fs_limit_gaussian_deviation = temp_double_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-duplicate-words-for-matching")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC correspondence learing words %s duplicated.\n",
                    fs_duplicate_words_for_matching ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-duplicate-words-for-matching = %s\n",
                    fs_duplicate_words_for_matching ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_duplicate_words_for_matching = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hc-emulate-uniform-discrete-prob")
          || match_pattern(lc_option, "hc-emulate-uniform-word-prob")
          || match_pattern(lc_option, "hc-emulate-uniform-discrete-probability")
          || match_pattern(lc_option, "hc-emulate-uniform-word-probability")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HC uniform discrete item probability %s emulated.\n",
                    fs_emulate_uniform_discrete_prob ? "is" : "is not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hc-emulate-uniform-word-prob = %s\n",
                    fs_emulate_uniform_discrete_prob ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_emulate_uniform_discrete_prob = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define MAX_NUM_IMPOSSIBLE_MODELS 5

int do_multi_modal_clustering
(
    const Topology_vector* topology_vp,
    const Int_vector*      dis_item_level_vp,
    const Cluster_data*    data_ptr,
    const Cluster_data*    held_out_data_ptr,
    Multi_modal_model*     model_ptr,
    int                    (*output_model_fn) (const Multi_modal_model *, int, const char *),
    const char*            output_dir,
    double                 (*dis_item_loss_fn)(int, const double*)
)
{
    int           j;
    int           result = NO_ERROR;
    Cluster_data*       perturbed_data_ptr = NULL;
    Cluster_data*       normalized_data_ptr = NULL;
    Cluster_data*       normalized_held_out_data_ptr = NULL;
    Cluster_data*       data_with_nulls_ptr = NULL;
    Cluster_data*       held_out_data_with_nulls_ptr = NULL;
    const Cluster_data* derived_data_ptr = data_ptr;
    const Cluster_data* derived_held_out_data_ptr = held_out_data_ptr;
    int                 num_topologies = topology_vp->num_topologies;
    int                 count;
    Vector* mean_vp = NULL;
    Vector* var_vp = NULL;
#ifdef USE_STANDARD_GUASS_NORM_CONSTANT
    double norm_stdev = 1.0 / (sqrt(2.0 * M_PI) *  M_E);
#else
#ifdef HOW_IT_WAS_FEB_26_2003
    double norm_stdev = 1.0 / M_E;
#else
#endif
    double norm_stdev = 1.0;
#endif
#ifdef REPORT_MEMORY_USE
    unsigned long total_bytes_used;
#endif


#ifdef REPORT_MEMORY_USE
    if (get_allocated_memory(&total_bytes_used) != ERROR)
    {
        dbu(total_bytes_used);
    }
#endif

#ifdef CHECK_NORM_STDEV
    norm_stdev = 100;  /* Whatever! */
#endif

    if (num_topologies <= 0)
    {
        set_error("No topology for hierarchical clustering.");
        return ERROR;
    }

    if (fs_normalize_data)
    {
        verbose_pso(2, "Normalizing data before training.\n");

        ERE(copy_cluster_data(&normalized_data_ptr, derived_data_ptr));
        ERE(copy_cluster_data(&normalized_held_out_data_ptr,
                              derived_held_out_data_ptr));

        if (ow_normalize_composite_cluster_data(normalized_data_ptr->con_item_mvp,
                                                (normalized_held_out_data_ptr == NULL) ? NULL : normalized_held_out_data_ptr->con_item_mvp,
                                                &mean_vp, &var_vp,
                                                norm_stdev,
                                                /*
                                                 * We don't normalize grouped
                                                 * features, because this might
                                                 * destroy relations among them. 
                                                */
                                                model_ptr->con_feature_group_vp)  
            == ERROR)
        {
            free_cluster_data(normalized_data_ptr);
            free_cluster_data(normalized_held_out_data_ptr);
            return ERROR;
        }

        derived_data_ptr = normalized_data_ptr;
        derived_held_out_data_ptr = normalized_held_out_data_ptr;
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (fs_data_perturbation > 10.0 * DBL_MIN)
    {
        if ( ! fs_normalize_data)
        {
            warn_pso("Perturbing data without normalization.\n");
            warn_pso("This may not be what you want.\n");
        }

        ERE(copy_cluster_data(&perturbed_data_ptr, derived_data_ptr));
        ERE(ow_perturb_composite_cluster_data(perturbed_data_ptr->con_item_mvp,
                                              fs_data_perturbation,
                                              /*
                                               * We don't normalize grouped
                                               * features, because this might
                                               * destroy relations among them. 
                                              */
                                              model_ptr->con_feature_group_vp));
        TEST_PSO(("Data perturbation still needs vec and his.\n"));
        derived_data_ptr = perturbed_data_ptr;
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (fs_add_discrete_nulls)
    {
        ERE(add_discrete_nulls_to_data(&data_with_nulls_ptr,
                                       derived_data_ptr));
        ERE(add_discrete_nulls_to_data(&held_out_data_with_nulls_ptr,
                                       derived_held_out_data_ptr));

        derived_data_ptr = data_with_nulls_ptr;
        derived_held_out_data_ptr = held_out_data_with_nulls_ptr;
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

#ifdef TEST
    verify_cluster_data(derived_data_ptr);
    verify_cluster_data(derived_held_out_data_ptr);
#endif

    if (num_topologies * fs_num_tries_per_topology == 1)
    {
        result = do_multi_modal_clustering_2(topology_vp->topologies[ 0 ],
                                             dis_item_level_vp,
                                             derived_data_ptr,
                                             derived_held_out_data_ptr,
                                             model_ptr, output_model_fn, output_dir,
                                             mean_vp, var_vp, norm_stdev,
                                             dis_item_loss_fn);

        if (result == ERROR)
        {
            NOTE_ERROR();
        }
#ifdef REPORT_MEMORY_USE
        else if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            dbu(total_bytes_used);
        }
#endif
    }
    else
    {
        double mdl_log_likelihood;
        double max_log_likelihood = DBL_MOST_NEGATIVE;
        double max_topology_log_likelihood = DBL_MOST_NEGATIVE;
        Multi_modal_model* new_model_ptr = allocate_multi_modal_model();

        UNTESTED_CODE();  /* Not recently, anyway. */

        verbose_pso(2, "Using %d topologies, and %d tries per topologoy.\n",
                    num_topologies, fs_num_tries_per_topology);

        if (new_model_ptr == NULL) result = ERROR;

        for (count = 0; count < num_topologies; count++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            for (j = 0; j < fs_num_tries_per_topology; j++)
            {
                int impossible_model;

                if (result == ERROR) { NOTE_ERROR(); break; }

                db_irv(topology_vp->topologies[ count ]->fan_out_vp);

                for (impossible_model = 0;
                     impossible_model < MAX_NUM_IMPOSSIBLE_MODELS;
                     impossible_model++
                    )
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = do_multi_modal_clustering_2
                             (
                                 topology_vp->topologies[ count ],
                                 dis_item_level_vp,
                                 derived_data_ptr,
                                 derived_held_out_data_ptr,
                                 new_model_ptr, NULL, NULL,
                                 mean_vp, var_vp, norm_stdev,
                                 dis_item_loss_fn
                             );
                }

                if (result == ERROR) { NOTE_ERROR(); break; }

                verbose_pso(1, "Raw log likelihood with topology %d is %.5e.\n",
                            count + 1,
                            new_model_ptr->raw_log_likelihood);

                verbose_pso(1,"Adjusted log likelihood with topology %d is %.5e.\n",
                            count + 1,
                            new_model_ptr->mdl_log_likelihood);

                mdl_log_likelihood = new_model_ptr->mdl_log_likelihood;

                if (mdl_log_likelihood > max_topology_log_likelihood)
                {
                    max_topology_log_likelihood = mdl_log_likelihood;
                }

                if (mdl_log_likelihood > max_log_likelihood)
                {
                    verbose_pso(1,"This is better than the previous max %.5e\n",
                                max_log_likelihood);

                    max_log_likelihood = mdl_log_likelihood;

                    verbose_pso(1,"Setting max log likelihood to %.5e\n",
                                max_log_likelihood);

                    result = copy_multi_modal_model(&model_ptr, new_model_ptr);
                    if (result == ERROR) { NOTE_ERROR(); }
                }
            }

#ifdef REPORT_MEMORY_USE
            if (    (result != ERROR)
                 && (get_allocated_memory(&total_bytes_used) != ERROR)
               )
            {
                dbu(total_bytes_used);
            }
#endif
        }

        free_multi_modal_model(new_model_ptr);
    }

    if ((result != ERROR) && (fs_normalize_data))
    {
        result = ow_un_normalize_multi_modal_model(model_ptr, mean_vp, var_vp,
                                                    norm_stdev);
        if (result == ERROR) { NOTE_ERROR(); }
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if ((result != ERROR) && (fs_add_discrete_nulls))
    {
        result = subtract_discrete_nulls_from_model(model_ptr);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    free_cluster_data(held_out_data_with_nulls_ptr);
    free_cluster_data(data_with_nulls_ptr);
    free_cluster_data(perturbed_data_ptr);
    free_cluster_data(normalized_data_ptr);
    free_cluster_data(normalized_held_out_data_ptr);

    free_vector(mean_vp);
    free_vector(var_vp);

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef THIS_IS_THE_NEW_MULTI_MODAL_CLUSTERING_3

static int do_multi_modal_clustering_2
(
    const Topology*     topology_ptr,
    const Int_vector*   dis_item_level_vp,
    const Cluster_data* data_ptr,
    const Cluster_data* held_out_data_ptr,
    Multi_modal_model* model_ptr,
    int                 (*output_model_fn) (const Multi_modal_model *, int, const char *),
    const char*         output_dir,
    const Vector*       norm_mean_vp,
    const Vector*       norm_var_vp,
    double              norm_stdev,
    double              (*dis_item_loss_fn)(int, const double*)
)
{
    int           result            = NO_ERROR;
    Training_options options;
    Topology* sub_topology_ptr = NULL;
    int min_num_levels;
    int max_num_levels;
    int num_levels;
    int level_range;
#ifdef XXX_SUPPORT_TRIMMING
    double trim_fraction = DBL_NOT_SET;
#endif
#ifdef REPORT_MEMORY_USE
    unsigned long total_bytes_used;
#endif


#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    /*
     * FIXME: It is not clear why we cannot do withou id_vp
    */
    if (    (data_ptr == NULL)
         || (data_ptr->num_points == 0)
         || (data_ptr->id_vp == NULL)
         || (data_ptr->id_vp->elements == 0)
       )
    {
        set_error("No training data is available.");
        return ERROR;
    }

    set_default_training_options(&options);

    /*
    // Set the option oriented model fields. We set them here so that if we need
    // to output the model, then the fields reflect the state under which the
    // model is being trained.
    */
    model_ptr->weight_con_items = fs_weight_con_items;
    model_ptr->discrete_tie_count = fs_discrete_tie_count;
    model_ptr->norm_depend = fs_norm_depend;
    model_ptr->phase_one_num_iterations = 0;  /* Ignore phase one. */
    model_ptr->phase_two_num_iterations = 1;  /* Not sure why we would want anything else. */
    model_ptr->phase_one_uniform_vertical_dist = fs_phase_one_uniform_vertical_dist;
    model_ptr->phase_two_uniform_vertical_dist = fs_phase_two_uniform_vertical_dist;
    model_ptr->phase_one_cluster_dependent_vertical_dist = fs_phase_one_cluster_dependent_vertical_dist;
    model_ptr->phase_two_cluster_dependent_vertical_dist = fs_phase_two_cluster_dependent_vertical_dist;

    model_ptr->phase_one_sample_matches = fs_phase_one_sample_matches;
    model_ptr->phase_one_share_matches = fs_phase_one_share_matches;
    model_ptr->phase_one_model_correspondence = fs_phase_one_model_correspondence;
    model_ptr->phase_one_last_sample = fs_phase_one_last_sample;
    model_ptr->phase_two_sample_matches = fs_phase_two_sample_matches;
    model_ptr->phase_two_share_matches = fs_phase_two_share_matches;
    model_ptr->phase_two_model_correspondence = fs_phase_two_model_correspondence;
    model_ptr->phase_two_last_sample = fs_phase_two_last_sample;

    model_ptr->dis_item_prob_threshold  = fs_dis_item_prob_threshold;
    model_ptr->norm_items               = fs_norm_items;
    model_ptr->norm_continuous_features = fs_norm_continuous_features;
    model_ptr->dis_prob_boost_factor    = fs_dis_prob_boost_factor;
    model_ptr->con_prob_boost_factor    = fs_con_prob_boost_factor;
    model_ptr->var_offset = fs_var_offset;
#ifdef SUPPORT_IGNORE_DIS_PROB
    model_ptr->ignore_dis_prob_fraction = fs_ignore_dis_prob_fraction;
#endif


    options.convergence_criterion = fs_phase_two_convergence_criterion;
    options.uniform_vertical_dist = fs_phase_two_uniform_vertical_dist;
    options.cluster_dependent_vertical_dist = fs_phase_two_cluster_dependent_vertical_dist;
    options.sample_matches = fs_phase_two_sample_matches;
    options.share_matches = fs_phase_two_share_matches;
    options.last_sample = fs_phase_two_last_sample;

#ifdef OBSOLETE /* Superseeded by separate word model. */
    /* IE, see do_fixed_discrete_multi_modal_clustering(). */
    if (topology_ptr->hack_nando_gaussian)
    {
        if (fs_phase_two_model_correspondence != 1)
        {
            warn_pso("Forcing correspondence model 1 for ");
            warn_pso("hacking Nando's Gaussian method.\n");
        }

        options.model_correspondence = 1;
    }
    else
    {
        options.model_correspondence = fs_phase_two_model_correspondence;
    }
#else
    options.model_correspondence = fs_phase_two_model_correspondence;
#endif

    dbi(topology_ptr->num_levels);

    max_num_levels = topology_ptr->num_levels;

    dbi(fs_min_num_levels);
    dbi(MAX_OF(fs_min_num_levels, topology_ptr->first_level));

    if (fs_min_num_levels <= 0)
    {
        min_num_levels = max_num_levels;
    }
    else
    {
        min_num_levels = MIN_OF(max_num_levels,
                                MAX_OF(fs_min_num_levels,
                                       topology_ptr->first_level));
    }

    level_range = max_num_levels - min_num_levels;

    dbi(max_num_levels);
    dbi(min_num_levels);
    dbi(level_range);
    dbi(topology_ptr->fan_out_vp->elements[ 0 ])

    num_levels = min_num_levels;

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    while (num_levels <= max_num_levels)
    {
        if (num_levels != max_num_levels)
        {
            dbp("++++++++++++++++++++++++++++++++++++++++++++++++");
            dbi(num_levels);
            dbi(max_num_levels);
            dbp("++++++++++++++++++++++++++++++++++++++++++++++++");
        }

        /* Untested since addition of last level parameter in topology. */
        /* Dec 10, 2004 */
        UNTESTED_CODE();

        result = copy_topology(&sub_topology_ptr,
                               topology_ptr);
        sub_topology_ptr->num_levels = num_levels;

        if (num_levels < sub_topology_ptr->last_level_num)
        {
            sub_topology_ptr->last_level_num = num_levels;
        }

        if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef XXX_SUPPORT_TRIMMING
        if (    (fs_min_trim_fraction > 0.0)
             && (fs_max_trim_fraction > 0.0)
           )
        {
            double trim_fraction_range = fs_max_trim_fraction - fs_min_trim_fraction;
            double trim_fraction_done = 0;

            if (level_range > 0)
            {
                trim_fraction_done = (double)(num_levels - min_num_levels) / (double)level_range;
            }

            trim_fraction = fs_max_trim_fraction - trim_fraction_done * trim_fraction_range
        }

        dbe(trim_fraction);

        options.trim_fraction = trim_fraction;
#endif

        result  = do_multi_modal_clustering_3(sub_topology_ptr, dis_item_level_vp,
                                       data_ptr, held_out_data_ptr,
                                       model_ptr, &options,
                                       output_model_fn,
                                       output_dir,
                                       norm_mean_vp, norm_var_vp,
                                       norm_stdev,
                                       fs_max_num_em_iterations,
                                       dis_item_loss_fn);

        if (result == ERROR)
        {
            NOTE_ERROR(); break;
        }

#ifdef REPORT_MEMORY_USE
        if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            dbu(total_bytes_used);
        }
#endif

        options.initial_model_is_valid = FALSE;
        options.initial_model_is_subset = TRUE;

        num_levels++;
    }

#ifdef TEST
    if (result != ERROR)
    {
        ASSERT_IS_EQUAL_INT(sub_topology_ptr->num_levels,
                            topology_ptr->num_levels);
    }
#endif

    free_topology(sub_topology_ptr);

    return result;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * We had this routine #ifdefed out in preference for the above because of some
 * possible conflict with training with nulls---long forgotten!
*/

static int do_multi_modal_clustering_2
(
    const Topology*     topology_ptr,
    const Int_vector*   dis_item_level_vp,
    const Cluster_data* data_ptr,
    const Cluster_data* held_out_data_ptr,
    Multi_modal_model* model_ptr,
    int                 (*output_model_fn) (const Multi_modal_model *, int, const char *),
    const char*         output_dir,
    const Vector*       norm_mean_vp,
    const Vector*       norm_var_vp,
    double              norm_stdev,
    double              (*dis_item_loss_fn)(int, const double*)
)
{
    int           result            = NO_ERROR;
    int it;
    Training_options options;
#ifdef REPORT_MEMORY_USE
    unsigned long total_bytes_used;
#endif


#ifdef REPORT_MEMORY_USE
    if (get_allocated_memory(&total_bytes_used) != ERROR)
    {
        dbu(total_bytes_used);
    }
#endif

    if (    (data_ptr == NULL)
         || (data_ptr->num_points == 0)
         || (data_ptr->id_vp == NULL)
         || (data_ptr->id_vp->elements == 0)
       )
    {
        set_error("No training data is available.");
        return ERROR;
    }

    set_default_training_options(&options);

    /*
    // Set the option oriented model fields. We set them here so that if we need
    // to output the model, then the fields reflect the state under which the
    // model is being trained.
    */
    model_ptr->weight_con_items = fs_weight_con_items;
    model_ptr->discrete_tie_count = fs_discrete_tie_count;
    model_ptr->norm_depend = fs_norm_depend;
    model_ptr->phase_one_num_iterations = fs_phase_one_num_iterations;
    model_ptr->phase_two_num_iterations = fs_phase_two_num_iterations;
    model_ptr->phase_one_uniform_vertical_dist = fs_phase_one_uniform_vertical_dist;
    model_ptr->phase_two_uniform_vertical_dist = fs_phase_two_uniform_vertical_dist;
    model_ptr->phase_one_cluster_dependent_vertical_dist = fs_phase_one_cluster_dependent_vertical_dist;
    model_ptr->phase_two_cluster_dependent_vertical_dist = fs_phase_two_cluster_dependent_vertical_dist;

    model_ptr->phase_one_sample_matches = fs_phase_one_sample_matches;
    model_ptr->phase_one_share_matches = fs_phase_one_share_matches;
    model_ptr->phase_one_model_correspondence = fs_phase_one_model_correspondence;
    model_ptr->phase_one_last_sample = fs_phase_one_last_sample;
    model_ptr->phase_two_sample_matches = fs_phase_two_sample_matches;
    model_ptr->phase_two_share_matches = fs_phase_two_share_matches;
    model_ptr->phase_two_model_correspondence = fs_phase_two_model_correspondence;
    model_ptr->phase_two_last_sample = fs_phase_two_last_sample;

    model_ptr->dis_item_prob_threshold  = fs_dis_item_prob_threshold;
    model_ptr->norm_items               = fs_norm_items;
    model_ptr->norm_continuous_features = fs_norm_continuous_features;
    model_ptr->dis_prob_boost_factor    = fs_dis_prob_boost_factor;
    model_ptr->con_prob_boost_factor    = fs_con_prob_boost_factor;
    model_ptr->var_offset = fs_var_offset;
#ifdef SUPPORT_IGNORE_DIS_PROB
    model_ptr->ignore_dis_prob_fraction = fs_ignore_dis_prob_fraction;
#endif

    if (fs_phase_one_num_iterations > 0)
    {
        int                 i;
        int                 num_points               = data_ptr->num_points;
        int                 count;
        Cluster_data*       subset_data_ptr          = allocate_cluster_data();
        Cluster_data*       subset_held_out_data_ptr = NULL;
        const Cluster_data* used_held_out_data_ptr;
        Indexed_vector*     point_key_vp             = NULL;
        int                 train_count              = kjb_rint((double)num_points * fs_phase_one_train_fraction);
        int                 held_out_count           = kjb_rint((double)num_points * fs_phase_one_held_out_fraction);
        int                 point;


#ifdef REPORT_MEMORY_USE
        if (    (result != ERROR)
             && (get_allocated_memory(&total_bytes_used) != ERROR)
           )
        {
            dbu(total_bytes_used);
        }
#endif

        if (train_count < 1)
        {
            set_error("No training points for phase one.");
            result = ERROR;
            NOTE_ERROR();
            goto mmc2_phase_one_cleanup;
        }

        if (held_out_count < 1)
        {
            set_error("No held out points for phase one.");
            result = ERROR;
            NOTE_ERROR();
            goto mmc2_phase_one_cleanup;
        }


        if (subset_data_ptr == NULL)
        {
            result = ERROR;
            NOTE_ERROR();
            goto mmc2_phase_one_cleanup;
        }

        if (    (fs_phase_one_held_out_data_is_fixed)
             && (    (held_out_data_ptr == NULL)
                  || (held_out_data_ptr->id_vp == NULL)
                  || (held_out_data_ptr->id_vp->length <= 0)
                )
            )
        {
            set_error("Phase one held out data is fixed but there is no held out data.");
            result = ERROR;
            NOTE_ERROR();
            goto mmc2_phase_one_cleanup;
        }

        options.convergence_criterion = fs_phase_one_convergence_criterion;
        options.uniform_vertical_dist = fs_phase_one_uniform_vertical_dist;
        options.cluster_dependent_vertical_dist = fs_phase_one_cluster_dependent_vertical_dist;
        options.sample_matches = fs_phase_one_sample_matches;
        options.share_matches = fs_phase_one_share_matches;
        options.last_sample = fs_phase_one_last_sample;

#ifdef OBSOLETE /* Superseeded by separate word model. */
        /* IE, see do_fixed_discrete_multi_modal_clustering(). */
        if (topology_ptr->hack_nando_gaussian)
        {
            if (fs_phase_one_model_correspondence != 1)
            {
                warn_pso("Forcing correspondence model 1 for ");
                warn_pso("hacking Nando's Gaussian method.\n");
            }

            options.model_correspondence = 1;
        }
        else
        {
            options.model_correspondence = fs_phase_one_model_correspondence;
        }
#else
        options.model_correspondence = fs_phase_one_model_correspondence;
#endif

        if (train_count + held_out_count > num_points)
        {
            warn_pso("Online training and held out counts exceed the number ");
            warn_pso("of points.\n");
            warn_pso("Making adjustments.\n");

            held_out_count = MIN_OF(held_out_count, num_points/2);
            train_count = num_points - held_out_count;
        }

        if (    (held_out_count < 1)
             && (fs_phase_one_convergence_criterion != MAX_DATA_LL)
           )
        {
            set_error("No phase one held out data.");
            add_error("Even though convergence criterion requres it.");
            result = ERROR;
            NOTE_ERROR();
        }

        if ( ! fs_phase_one_held_out_data_is_fixed)
        {
            if ((subset_held_out_data_ptr = allocate_cluster_data()) == NULL)
            {
                result = ERROR;
                NOTE_ERROR();
                goto mmc2_phase_one_cleanup;
            }
            else
            {
                used_held_out_data_ptr = subset_held_out_data_ptr;
            }
        }
        else
        {
            used_held_out_data_ptr = held_out_data_ptr;
        }

        for (i = 0; i < fs_phase_one_num_iterations; i++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_random_indexed_vector(&point_key_vp, num_points);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = descend_sort_indexed_vector(point_key_vp);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_target_cluster_data_like_source(
                                                     &subset_held_out_data_ptr,
                                                     data_ptr, NOT_SET);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_target_cluster_data_like_source(&subset_data_ptr,
                                                         data_ptr, NOT_SET);
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (subset_held_out_data_ptr != NULL)
            {
                for (count = 0; count < held_out_count; count++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    point = point_key_vp->elements[ count ].index;

                    subset_held_out_data_ptr->id_vp->elements[ count ] = data_ptr->id_vp->elements[ point ];

                    if (data_ptr->weight_vp != NULL)
                    {
                        subset_held_out_data_ptr->weight_vp->elements[ count ] =
                                        data_ptr->weight_vp->elements[ point ];
                    }

                    if (data_ptr->con_item_mvp != NULL)
                    {
                        result = copy_matrix(&(subset_held_out_data_ptr->con_item_mvp->elements[ count ]),
                                             data_ptr->con_item_mvp->elements[ point ]);
                        if (result == ERROR) { NOTE_ERROR(); break; }
                    }

                    if (data_ptr->vec_item_vvvvp != NULL)
                    {
                        result = copy_v3(&(subset_held_out_data_ptr->vec_item_vvvvp->elements[ count ]),
                                         data_ptr->vec_item_vvvvp->elements[ point ]);
                        if (result == ERROR) { NOTE_ERROR(); break; }
                    }

                    if (data_ptr->his_item_vvvvp != NULL)
                    {
                        result = copy_v3(&(subset_held_out_data_ptr->his_item_vvvvp->elements[ count ]),
                                         data_ptr->his_item_vvvvp->elements[ point ]);
                        if (result == ERROR) { NOTE_ERROR(); break; }
                    }

                    if (data_ptr->dis_item_mp != NULL)
                    {
                        result = copy_int_matrix_row(subset_held_out_data_ptr->dis_item_mp,
                                                     count,
                                                     data_ptr->dis_item_mp,
                                                     point);
                        if (result == ERROR) { NOTE_ERROR(); break; }
                    }

                    if (data_ptr->dis_item_multiplicity_mp != NULL)
                    {
                        result = copy_matrix_row(subset_held_out_data_ptr->dis_item_multiplicity_mp,
                                                 count,
                                                 data_ptr->dis_item_multiplicity_mp,
                                                 point);
                        if (result == ERROR) { NOTE_ERROR(); break; }
                    }
                }

                subset_held_out_data_ptr->num_points = held_out_count;
                subset_held_out_data_ptr->id_vp->length = held_out_count;

                if (data_ptr->weight_vp != NULL)
                {
                    subset_held_out_data_ptr->weight_vp->length = held_out_count;
                }

                if (subset_held_out_data_ptr->dis_item_mp != NULL)
                {
                    subset_held_out_data_ptr->dis_item_mp->num_rows = held_out_count;
                }

                if (subset_held_out_data_ptr->dis_item_mp != NULL)
                {
                    subset_held_out_data_ptr->dis_item_multiplicity_mp->num_rows = held_out_count;
                }

                if (subset_held_out_data_ptr->con_item_mvp != NULL)
                {
                    subset_held_out_data_ptr->con_item_mvp->length = held_out_count;
                }

                if (subset_held_out_data_ptr->vec_item_vvvvp != NULL)
                {
                    subset_held_out_data_ptr->vec_item_vvvvp->length = held_out_count;
                }

                if (subset_held_out_data_ptr->his_item_vvvvp != NULL)
                {
                    subset_held_out_data_ptr->his_item_vvvvp->length = held_out_count;
                }

                result = set_max_num_cluster_data_items(subset_held_out_data_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }

                if (data_ptr->max_num_dis_items == 0)
                {
                    subset_held_out_data_ptr->max_num_dis_items = 0;
                }

                if (data_ptr->max_num_con_items == 0)
                {
                    subset_held_out_data_ptr->max_num_con_items = 0;
                }
            }

            for (count = 0; count < train_count; count++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }

                point = point_key_vp->elements[ held_out_count + count ].index;
                subset_data_ptr->id_vp->elements[  count ] = data_ptr->id_vp->elements[ point ];

                if (data_ptr->weight_vp != NULL)
                {
                    subset_data_ptr->weight_vp->elements[ count ] =
                                    data_ptr->weight_vp->elements[ point ];
                }

                if (data_ptr->con_item_mvp != NULL)
                {
                    result = copy_matrix(&(subset_data_ptr->con_item_mvp->elements[  count ]),
                                         data_ptr->con_item_mvp->elements[ point ]);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }

                if (data_ptr->vec_item_vvvvp != NULL)
                {
                    result = copy_v3(&(subset_data_ptr->vec_item_vvvvp->elements[  count ]),
                                      data_ptr->vec_item_vvvvp->elements[ point ]);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }

                if (data_ptr->his_item_vvvvp != NULL)
                {
                    result = copy_v3(&(subset_data_ptr->his_item_vvvvp->elements[  count ]),
                                      data_ptr->his_item_vvvvp->elements[ point ]);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }

                if (data_ptr->dis_item_mp != NULL)
                {
                    result = copy_int_matrix_row(subset_data_ptr->dis_item_mp,
                                                 count,
                                                 data_ptr->dis_item_mp,
                                                 point);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }

                if (data_ptr->dis_item_multiplicity_mp != NULL)
                {
                    result = copy_matrix_row(subset_data_ptr->dis_item_multiplicity_mp,
                                             count,
                                             data_ptr->dis_item_multiplicity_mp,
                                             point);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }

            }

            if (result == ERROR) { NOTE_ERROR(); break; }

            subset_data_ptr->num_points = train_count;
            subset_data_ptr->id_vp->length = train_count;

            if (data_ptr->weight_vp != NULL)
            {
                subset_data_ptr->weight_vp->length = train_count;
            }

            if (subset_data_ptr->dis_item_mp != NULL)
            {
                subset_data_ptr->dis_item_mp->num_rows = train_count;
            }

            if (subset_data_ptr->dis_item_mp != NULL)
            {
                subset_data_ptr->dis_item_multiplicity_mp->num_rows = train_count;
            }

            if (subset_data_ptr->con_item_mvp != NULL)
            {
                subset_data_ptr->con_item_mvp->length = train_count;
            }

            if (subset_data_ptr->vec_item_vvvvp != NULL)
            {
                subset_data_ptr->vec_item_vvvvp->length = train_count;
            }

            if (subset_data_ptr->his_item_vvvvp != NULL)
            {
                subset_data_ptr->his_item_vvvvp->length = train_count;
            }

            result = set_max_num_cluster_data_items(subset_data_ptr);
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (data_ptr->max_num_dis_items == 0)
            {
                subset_data_ptr->max_num_dis_items = 0;
            }

            if (data_ptr->max_num_con_items == 0)
            {
                subset_data_ptr->max_num_con_items = 0;
            }

#ifdef REPORT_MEMORY_USE
            if (get_allocated_memory(&total_bytes_used) != ERROR)
            {
                dbu(total_bytes_used);
            }
#endif

            result = do_multi_modal_clustering_3(topology_ptr,
                                                 dis_item_level_vp,
                                                 subset_data_ptr,
                                                 used_held_out_data_ptr,
                                                 model_ptr,
                                                 &options, NULL, NULL,
                                                 norm_mean_vp, norm_var_vp,
                                                 norm_stdev,
                          (fs_phase_one_convergence_criterion == THREE_ITERATIONS) ? 3 : fs_max_num_em_iterations,
                                                 dis_item_loss_fn);

            if (result == ERROR)
            {
                NOTE_ERROR(); break;
            }
#ifdef REPORT_MEMORY_USE
            else if (get_allocated_memory(&total_bytes_used) != ERROR)
            {
                dbu(total_bytes_used);
            }
#endif

            options.initial_model_is_valid = TRUE;

            /*
            // Regardless of what we are doing, we never consider the levels to
            // be part of a model to pass on.
            */
            free_matrix_vector(model_ptr->P_l_p_c_mvp);
            model_ptr->P_l_p_c_mvp = NULL;
        }

mmc2_phase_one_cleanup:

        free_cluster_data(subset_held_out_data_ptr);
        free_cluster_data(subset_data_ptr);
        free_indexed_vector(point_key_vp);
    }

    if (result == ERROR) return ERROR;

#ifdef REPORT_MEMORY_USE
    if (get_allocated_memory(&total_bytes_used) != ERROR)
    {
        dbu(total_bytes_used);
    }
#endif

    if (fs_phase_two_num_iterations <= 0)
    {
        UNTESTED_CODE();   /* Not excecuted recently anyway. */

        /*
        // Special case. Here we only going in for one iteration to clean up the
        // data from phase one. We therefore use phase one options.
        */
        options.convergence_criterion = SINGLE_ITERATION;
        options.uniform_vertical_dist = fs_phase_one_uniform_vertical_dist;
        options.cluster_dependent_vertical_dist = fs_phase_one_cluster_dependent_vertical_dist;
        options.sample_matches = fs_phase_one_sample_matches;
        options.share_matches = fs_phase_one_share_matches;
        options.last_sample = fs_phase_one_last_sample;

#ifdef OBSOLETE /* Superseeded by separate word model. */
        /* IE, see do_fixed_discrete_multi_modal_clustering(). */
        if (topology_ptr->hack_nando_gaussian)
        {
            if (fs_phase_one_model_correspondence != 1)
            {
                warn_pso("Forcing correspondence model 1 for ");
                warn_pso("hacking Nando's Gaussian method.\n");
            }

            options.model_correspondence = 1;
        }
        else
        {
            options.model_correspondence = fs_phase_one_model_correspondence;
        }
#else
        options.model_correspondence = fs_phase_one_model_correspondence;
#endif

        result = do_multi_modal_clustering_3(topology_ptr, dis_item_level_vp,
                                             data_ptr, held_out_data_ptr,
                                             model_ptr, &options, NULL, NULL,
                                             norm_mean_vp, norm_var_vp,
                                             norm_stdev, 1,
                                             dis_item_loss_fn);

        if (result == ERROR)
        {
            NOTE_ERROR();
        }
        else
        {
            options.initial_model_is_valid = TRUE;

#ifdef REPORT_MEMORY_USE
            if (get_allocated_memory(&total_bytes_used) != ERROR)
            {
                dbu(total_bytes_used);
            }
#endif
        }

    }
    else
    {
        options.convergence_criterion = fs_phase_two_convergence_criterion;
        options.uniform_vertical_dist = fs_phase_two_uniform_vertical_dist;
        options.cluster_dependent_vertical_dist = fs_phase_two_cluster_dependent_vertical_dist;
        options.sample_matches = fs_phase_two_sample_matches;
        options.share_matches = fs_phase_two_share_matches;
        options.last_sample = fs_phase_two_last_sample;

#ifdef OBSOLETE /* Superseeded by separate word model. */
        /* IE, see do_fixed_discrete_multi_modal_clustering(). */
        if (topology_ptr->hack_nando_gaussian)
        {
            if (fs_phase_two_model_correspondence != 1)
            {
                warn_pso("Forcing correspondence model 1 for ");
                warn_pso("hacking Nando's Gaussian method.\n");
            }

            options.model_correspondence = 1;
        }
        else
        {
            options.model_correspondence = fs_phase_two_model_correspondence;
        }
#else
        options.model_correspondence = fs_phase_two_model_correspondence;
#endif

        for (it = 0; it < fs_phase_two_num_iterations; it++)
        {
            if (result == ERROR)
            {
                NOTE_ERROR(); break;
            }
#ifdef REPORT_MEMORY_USE
            else if (get_allocated_memory(&total_bytes_used) != ERROR)
            {
                dbu(total_bytes_used);
            }
#endif

            result  = do_multi_modal_clustering_3(topology_ptr, dis_item_level_vp,
                                                  data_ptr, held_out_data_ptr,
                                                  model_ptr, &options,
                                                  (it < fs_phase_two_num_iterations - 1) ? NULL : output_model_fn,
                                                  output_dir,
                                                  norm_mean_vp, norm_var_vp,
                                                  norm_stdev,
                                                  fs_max_num_em_iterations,
                                                  dis_item_loss_fn);

            if (result == ERROR)
            {
                NOTE_ERROR();
            }
            else
            {
#ifdef REPORT_MEMORY_USE
                if (get_allocated_memory(&total_bytes_used) != ERROR)
                {
                    dbu(total_bytes_used);
                }
#endif

                options.initial_model_is_valid = TRUE;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int do_multi_modal_clustering_3
(
    const Topology*     topology_ptr,
    const Int_vector*   dis_item_level_vp,
    const Cluster_data* data_ptr,
    const Cluster_data* held_out_data_ptr,
    Multi_modal_model* model_ptr,
    const Training_options* options_ptr,
    int                 (*output_model_fn) (const Multi_modal_model *, int, const char *),
    const char*         output_dir,
    const Vector*       norm_mean_vp,
    const Vector*       norm_var_vp,
    double              norm_stdev,
    int                 max_num_em_iterations,
    double              (*dis_item_loss_fn)(int, const double*)
)
{
    Training_options options = *options_ptr;
    int           result            = NO_ERROR;
    Topology* sub_topology_ptr = NULL;
    int min_num_levels;
    int max_num_levels;
    int num_levels;
    int level_range;
#ifdef XXX_SUPPORT_TRIMMING
    double trim_fraction = DBL_NOT_SET;
#endif
#ifdef REPORT_MEMORY_USE
    unsigned long total_bytes_used;
#endif


#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif


    if (topology_ptr->num_levels_per_category > 0)
    {
        /*
         * This means that we want a model based on a fixed set of nodes for
         * each word.
        */
        return do_fixed_discrete_multi_modal_clustering(
                                              topology_ptr,
                                              data_ptr, held_out_data_ptr,
                                              model_ptr, &options,
                                              output_model_fn,
                                              output_dir,
                                              norm_mean_vp, norm_var_vp,
                                              norm_stdev,
                                              max_num_em_iterations);

    }


    dbi(topology_ptr->num_levels);

    max_num_levels = topology_ptr->num_levels;

    dbi(fs_min_num_levels);
    dbi(MAX_OF(fs_min_num_levels, topology_ptr->first_level));

    if (fs_min_num_levels <= 0)
    {
        min_num_levels = max_num_levels;
    }
    else
    {
        min_num_levels = MIN_OF(max_num_levels,
                                MAX_OF(fs_min_num_levels,
                                       topology_ptr->first_level));
    }

    level_range = max_num_levels - min_num_levels;

    dbi(max_num_levels);
    dbi(min_num_levels);
    dbi(level_range);
    dbi(topology_ptr->fan_out_vp->elements[ 0 ])

    num_levels = min_num_levels;

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    while (num_levels <= max_num_levels)
    {
        if (num_levels != max_num_levels)
        {
            dbp("++++++++++++++++++++++++++++++++++++++++++++++++");
            dbi(num_levels);
            dbi(max_num_levels);
            dbp("++++++++++++++++++++++++++++++++++++++++++++++++");
        }

        /* Untested since addition of last level parameter in topology. */
        /* Dec 10, 2004 */
        UNTESTED_CODE();

        result = copy_topology(&sub_topology_ptr,
                               topology_ptr);
        sub_topology_ptr->num_levels = num_levels;

        if (num_levels < sub_topology_ptr->last_level_num)
        {
            sub_topology_ptr->last_level_num = num_levels;
        }

        if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef XXX_SUPPORT_TRIMMING
        if (    (fs_min_trim_fraction > 0.0)
             && (fs_max_trim_fraction > 0.0)
           )
        {
            double trim_fraction_range = fs_max_trim_fraction - fs_min_trim_fraction;
            double trim_fraction_done = 0;

            if (level_range > 0)
            {
                trim_fraction_done = (double)(num_levels - min_num_levels) / (double)level_range;
            }

            trim_fraction = fs_max_trim_fraction - trim_fraction_done * trim_fraction_range
        }

        dbe(trim_fraction);

        options.trim_fraction = trim_fraction;
#endif

        result  = do_multi_modal_clustering_4(sub_topology_ptr, dis_item_level_vp,
                                              data_ptr, held_out_data_ptr,
                                              model_ptr, &options,
                                              output_model_fn,
                                              output_dir,
                                              norm_mean_vp, norm_var_vp,
                                              norm_stdev,
                                              max_num_em_iterations,
                                              dis_item_loss_fn);

        if (result == ERROR)
        {
            NOTE_ERROR(); break;
        }

#ifdef REPORT_MEMORY_USE
        if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            dbu(total_bytes_used);
        }
#endif

        options.initial_model_is_valid = FALSE;
        options.initial_model_is_subset = TRUE;

        num_levels++;
    }

#ifdef TEST
    if (result != ERROR)
    {
        ASSERT_IS_EQUAL_INT(sub_topology_ptr->num_levels,
                            topology_ptr->num_levels);
    }
#endif

    free_topology(sub_topology_ptr);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void set_default_training_options
(
    Training_options* training_options_ptr
)
{

    training_options_ptr->convergence_criterion = MAX_DATA_LL;
    training_options_ptr->initial_model_is_valid = FALSE;
    training_options_ptr->initial_model_is_subset = FALSE;
    training_options_ptr->num_limited_clusters = NOT_SET;
    training_options_ptr->model_correspondence = NOT_SET;
    training_options_ptr->share_matches = FALSE;
    training_options_ptr->sample_matches = FALSE;
    training_options_ptr->last_sample = NOT_SET;
#ifdef XXX_SUPPORT_TRIMMING
    training_options_ptr->trim_fraction = DBL_NOT_SET;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Search for "head" */

static int do_multi_modal_clustering_4
(
    const Topology*         topology_ptr,
    const Int_vector*       dis_item_level_vp,
    const Cluster_data*     data_ptr,
    const Cluster_data*     held_out_data_ptr,
    Multi_modal_model*     model_ptr,
    const Training_options* options_ptr,
    int                     (*output_model_fn) (const Multi_modal_model *, int, const char *),
    const char*             output_dir,
    const Vector*           norm_mean_vp,
    const Vector*           norm_var_vp,
    double                  norm_stdev,
    int                     max_num_em_iterations,
    double                  (*dis_item_loss_fn)(int, const double*)
)
{
    IMPORT volatile Bool    iteration_atn_flag;
    const int               num_real_levels   = topology_ptr->num_levels;
    const int               first_level       = topology_ptr->first_level;
    const int               last_level_num    = topology_ptr->last_level_num;
    const Int_vector*       level_counts_vp   = topology_ptr->level_counts_vp;
    const Int_matrix*       node_mp           = topology_ptr->node_mp;
    const int               num_points        = data_ptr->num_points;
    const int               num_categories    = data_ptr->num_categories;
    const int               max_num_dis_items = data_ptr->max_num_dis_items;
    const double            max_num_dis_items_with_multiplicity = data_ptr->max_num_dis_items_with_multiplicity;
    const int               max_num_con_items = data_ptr->max_num_con_items;
    const int               max_num_vec_items = data_ptr->max_num_vec_items;
    const int               max_num_his_items = data_ptr->max_num_his_items;
    const int               max_num_held_out_dis_items = (held_out_data_ptr == NULL) ? 0 : held_out_data_ptr->max_num_dis_items;
    const double            max_num_held_out_dis_items_with_multiplicity = (held_out_data_ptr == NULL) ? 0 : held_out_data_ptr->max_num_dis_items_with_multiplicity;
    const int               max_num_held_out_con_items = (held_out_data_ptr == NULL) ? 0 : held_out_data_ptr->max_num_con_items;
    const Int_matrix*       dis_item_mp = (max_num_dis_items == 0) ? NULL : data_ptr->dis_item_mp;
    const Matrix*           dis_item_multiplicity_mp    = (max_num_dis_items == 0) ? NULL : data_ptr->dis_item_multiplicity_mp;
#ifdef XXX_NEVER_USED
    const Matrix*           dis_category_level_prior_mp = (max_num_dis_items == 0) ? NULL : data_ptr->dis_category_level_prior_mp;
#endif
    const Matrix_vector*    con_item_mvp         = (max_num_con_items == 0) ? NULL : data_ptr->con_item_mvp;
#ifdef VEC_AND_HIS
    const V_v_v_v*          vec_item_vvvvp       = (max_num_vec_items == 0) ? NULL : data_ptr->vec_item_vvvvp;
    const V_v_v_v*          his_item_vvvvp       = (max_num_his_items == 0) ? NULL : data_ptr->his_item_vvvvp;
#endif
    const Int_matrix*       held_out_dis_item_mp = (max_num_held_out_dis_items == 0) ? NULL : held_out_data_ptr->dis_item_mp;
    const Matrix*           held_out_dis_item_multiplicity_mp = (max_num_held_out_dis_items == 0) ? NULL : held_out_data_ptr->dis_item_multiplicity_mp;
    const Matrix_vector*    held_out_con_item_mvp   = (max_num_held_out_con_items == 0) ? NULL : held_out_data_ptr->con_item_mvp;
#ifdef VECTOR_ATTRIBUTE_SUPPORT
    const V_v_v_v*          held_out_vec_item_vvvvp = (max_num_held_out_vec_items == 0) ? NULL : held_out_data_ptr->vec_item_vvvvp;
#endif
#ifdef HISTOGRAM_ATTRIBUTE_SUPPORT
    const V_v_v_v*          held_out_his_item_vvvvp = (max_num_held_out_his_items == 0) ? NULL : held_out_data_ptr->his_item_vvvvp;
#endif
    const int               max_num_items         = max_num_dis_items + max_num_con_items + max_num_vec_items + max_num_his_items;
    const int               num_real_clusters      = topology_ptr->num_clusters;
    const int               num_con_features  = data_ptr->num_con_features;
    const Int_vector*       con_feature_enable_vp = model_ptr->con_feature_enable_vp;
    const int               num_used_con_features = (con_feature_enable_vp == NULL) ? num_con_features : sum_int_vector_elements(con_feature_enable_vp);
    const int               num_sub_levels        = model_ptr->topology_ptr->num_levels;
    int               num_sub_clusters      = model_ptr->topology_ptr->num_clusters;
#ifdef VEC_AND_HIS
    const int               num_vec_features  = data_ptr->num_vec_features;
    const Int_vector*       vec_num_dim_vp    = data_ptr->vec_num_dim_vp;
#endif
    const Int_vector* con_feature_group_vp = model_ptr->con_feature_group_vp;
    const Vector_vector* con_item_weight_vvp = data_ptr->con_item_weight_vvp;
    const Matrix_vector* blob_word_prior_mvp = data_ptr->blob_word_prior_mvp;
    int num_levels = num_real_levels;
    int num_clusters = num_real_clusters;
    int num_limited_clusters = num_clusters;
    int num_clusters_used = num_clusters;
#ifdef VEC_AND_HIS
    int                     num_vec_items;
    int                     num_his_items;
#endif
    int                     cluster = NOT_SET;
    int                     point;
    int                     dis_item;
    int                     con_item;
#ifdef VEC_AND_HIS
    int                     vec_item;
    int                     his_item;
#endif
    int                     node;
    int                     category;
    int                     con_feature;
#ifdef VEC_AND_HIS
    int                     vec_feature;
#endif
    int                     item;
    Matrix*                 P_i_n_mp;
    int                     it;
    Vector*                 map_alpha_vp      = NULL;
    Vector*                 map_r_minus_n_g_vp = NULL;
    double                  map_r_minus_n_g;
    V_v_v*                  map_beta_minus_one_vvvp     = NULL;
    Vector_vector*          map_beta_minus_one_vvp;
    Vector*                 map_beta_minus_one_vp;
    Matrix_vector*          delta_inverse_mvp         = NULL;
    Matrix*                 delta_inverse_mp = NULL;
    Matrix_vector*          omega_mvp         = NULL;
    Matrix* omega_mp = NULL;
    Vector*                 new_a_vp              = NULL;
    Vector_vector*          dis_P_sum_vvp         = NULL;
    Vector*                 dis_P_sum_vp;
    Vector_vector*          con_P_sum_vvp         = NULL;
    V_v_v*                  missing_con_P_sum_vvvp         = NULL;
    Vector*                 P_sum_vp;
    double                  log_likelihood = 0.0;
    double                  prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double                  ll_rel_diff = DBL_NOT_SET;
    double                  held_out_ll_diff = DBL_NOT_SET;
    int                     result        = NO_ERROR;
    Matrix_vector*          held_out_P_l_p_c_mvp = NULL;
    Matrix*                 held_out_P_l_p_mp = NULL;
    Matrix_vector*          new_P_i_n_mvp      = NULL;
    V_v_v*                  con_prob_cache_vvvp = NULL;
    Vector_vector*          new_con_log_sqrt_det_vvp = NULL;
    Matrix_vector*          new_mean_mvp       = NULL;
    Matrix_vector*          new_var_mvp        = NULL;
    Vector_vector*          his_sqrt_det_vvp     = NULL;
    double***               level_indicators_ppp = NULL;
    double**                level_indicators_pp;
    double*                 level_indicators_p;
    double***               pair_probs_1_ppp = NULL;
    double**                pair_probs_1_pp;
    double*                 pair_probs_1_p;
    double***               pair_probs_2_ppp = NULL;
    double**                pair_probs_2_pp;
    double*                 pair_probs_2_p;
    double***               log_pair_probs_1_ppp = NULL;
    double**                log_pair_probs_1_pp;
    double*                 log_pair_probs_1_p;
    double***               log_pair_probs_2_ppp = NULL;
    double**                log_pair_probs_2_pp;
    double*                 log_pair_probs_2_p;
    int                     level;
    Vector*                 temp_vp     = NULL;
    Vector*                 var_vp      = NULL;
    Vector*                 x_vp        = NULL;
    Vector*                 u_vp        = NULL;
    Vector*                 w_vp        = NULL;
    Vector*                 d_vp        = NULL;
    double                  held_out_log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    /*
    // Use DBL_HALF_MOST_NEGATIVE divided by two for
    // held_out_prev_log_likelihood so that a return of DBL_HALF_MOST_NEGATIVE
    // looks like a decrease.
    */
    double                  held_out_prev_log_likelihood = DBL_HALF_MOST_NEGATIVE / 2.0;
    Matrix*                 new_V_mp = NULL;
    Matrix*                 new_con_V_mp = NULL;
    Matrix*                 new_dis_V_mp = NULL;
    int                     need_held_out_log_likelihood = FALSE;
    double                  max_combined_log_likelihood  = DBL_HALF_MOST_NEGATIVE;
    double                  max_held_out_log_likelihood  = DBL_HALF_MOST_NEGATIVE;
    int                     first_negative_held_out_ll_diff = FALSE;
    double                  a_vp_normalization;
    double                  con_log_norm       = 0.0;
    double                  max_con_log_prob   = 0.0;
    double                  ll_con_correction_norm_sum = 0.0;
#ifdef VEC_AND_HIS
    double                  vec_log_norm       = 0.0;
    double                  max_vec_log_prob   = 0.0;
    double                  ll_vec_correction_norm_sum = 0.0;
#endif
    Vector*                 P_c_p_vp         = NULL;
    Vector*                 ll_P_c_p_vp      = NULL;
    Vector*                 log_prob_vp          = NULL;
    Vector*                 prob_vp          = NULL;
    Matrix*                 prev_P_c_p_mp    = NULL;
    double                  max_parm_diff;
    Vector*                 dis_factor_vp    = NULL;
    Vector*                 con_factor_vp    = NULL;
    Vector*                 vec_factor_vp    = NULL;
    Vector*                 his_factor_vp    = NULL;
    Vector*                 held_out_dis_factor_vp = NULL;
    Vector*                 held_out_con_factor_vp = NULL;
    double                  dis_item_multiplicity;
#ifdef SUPPORT_ANNEALING
    Vector*                 annealed_prob_vp = NULL;
    Vector*                 annealed_log_prob_vp = NULL;
    double                  temperature = 1.0;
    double                  temperature_factor           = 1.0;
    V_v_v*                  annealed_con_prob_cache_vvvp = NULL;
#endif
#ifdef REPORT_CPU_TIME
    long                    cpu_time_offset;
    long                    initialization_cpu;
    long                    loop_initialization_cpu     = 0;
    long                    vertical_weight_caching_cpu   = 0;
    long                    pair_item_prob_update_cpu   = 0;
    long                    con_prob_cache_cpu      = 0;
    long                    dis_item_prob_update_cpu      = 0;
    long                    con_item_prob_update_cpu      = 0;
    long                    dis_item_parameter_update_cpu = 0;
    long                    con_item_parameter_update_cpu = 0;
    long                    vertical_weight_update_cpu    = 0;
    long                    q_calculation_cpu = 0;
    long                    log_likelihood_cpu            = 0;
    long                    held_out_cpu        = 0;
    long                    cluster_membership_cpu        = 0;
    long                    loop_cleanup_cpu     = 0;
    long                    limited_cluster_cpu        = 0;
    long                    cleanup_cpu         = 0;
    long                    pair_prob_table_cpu         = 0;
    long                    raw_pair_prob_table_cpu         = 0;
    long                    pair_match_cpu         = 0;
#ifdef TEST
    long                    test_extra_cpu         = 0;
#endif
    long                    total_recorded_cpu  = 0;
    long                    all_cpu_offset = 0;
    long                    all_cpu = 0;
#endif
    double limited_cluster_error;
    double ave_limited_cluster_error = DBL_NOT_SET;
    double max_limited_cluster_error = DBL_MOST_NEGATIVE;
    int valid_cluster_dependent_aspects = FALSE;
    int valid_aspects = FALSE;
    int valid_cluster_index = FALSE;
    int compute_aspects = FALSE;
    int compute_cluster_dependent_aspects = FALSE;
    Int_matrix* cluster_index_mp = NULL;
    int cluster_index;
    Matrix*           V_mp = model_ptr->V_mp;
    Matrix*           P_c_p_mp         = model_ptr->P_c_p_mp;
    Matrix_vector*    P_l_p_c_mvp   = model_ptr->P_l_p_c_mvp;
    Matrix*    P_l_p_mp   = model_ptr->P_l_p_mp;
    Vector_vector*    con_log_sqrt_det_vvp = model_ptr->con_log_sqrt_det_vvp;
    Matrix_vector*    mean_mvp       = model_ptr->mean_mvp;
    Matrix_vector*    var_mvp        = model_ptr->var_mvp;
    Matrix_vector*    P_i_n_mvp      = model_ptr->P_i_n_mvp;
    Matrix_vector*    log_P_i_n_mvp = NULL;
    Vector*                 a_vp              = model_ptr->a_vp;
    Multi_modal_model* save_model_ptr = NULL;
    Vector*            fit_vp = model_ptr->fit_vp;
    int            dis_item_level;
    double         rel_level;
    double         level_diff;
    int            num_nodes_this_level;
    int            head_len;
    int            changed_number_of_parameters;
    double         var_offset;
    double         s;
    double         dis_factor;
    double         con_factor;
    int            num_con_items;
    int            num_dis_items = NOT_SET;
    int*           cluster_index_ptr;
    double         p;
    double*        save_x_ptr;
    Vector_vector* con_prob_cache_vvp;
#ifdef SUPPORT_ANNEALING
    Vector_vector* annealed_con_prob_cache_vvp;
    double*        annealed_con_prob_cache_ptr;
    double         annealed_log_prob;
    double         annealed_log_p;
    double         annealed_log_product;
    double         annealed_max_log_prob;
    double         annealed_prob;
    double         annealed_prob_sum;
    double         annealed_log_prob_sum;
    double         annealed_log_nu_times_prob;
    double         annealed_nu_times_prob;
#endif
    double*        con_prob_cache_ptr;
    double         d;
    double*        u_ptr;
    double*        v_ptr;
    double*        x_ptr;
    double         log_prob;
    double         log_con_prob;
    double*        max_log_ptr;
    int            dis_item_found;
    double         log_p;
    double         log_product;
    double         nu;
    double         max_log_prob;
    double         nu_times_prob;
    double         prob_sum;
    double         log_prob_sum;
    double         log_nu;
    double         log_nu_times_prob;
    double         prob;
    double         sum;
    double         fit;
    double         temp;
    double         level_sum;
    double*        q_ptr;
    double         q;
    double*        P_sum_ptr;
    double*        P_i_n_ptr;
    int*           dis_item_ptr;
    double*        dis_item_multiplicity_ptr;
    Matrix*        mean_mp;
    Matrix*        var_mp;
    Vector*        con_log_sqrt_det_vp;
    double         var;
    double         u;
    double         u2;
    Matrix*        limited_P_c_p_mp             = NULL;
#ifdef HOW_IT_WAS_05_01_29
    double         e;
    double         norm;
    double         dev;
    double         v;
    double         x;
#endif
#ifdef MIN_LEVEL_INDICATOR
    /* When adding to quantities that sum to one */
    const double delta_to_ensure_min_level_indicator = MIN_LEVEL_INDICATOR / (1.0 - MIN_LEVEL_INDICATOR * (last_level_num - first_level));
    const double min_level_indicator_norm =
                      (1.0 + delta_to_ensure_min_level_indicator * (last_level_num - first_level));
#endif
#ifdef TEST
    double         sum_nu;
    int l;
#endif
    char user_id[ 1000 ];
    char halt_file_name[ MAX_FILE_NAME_SIZE ];
    double min_con_log_prob = DBL_HALF_MOST_NEGATIVE;
#ifdef XXX_LIMIT_MIN_CON_PROB
    Vector* g_prob_vp = NULL;
    Indexed_vector* sorted_g_prob_vp = NULL;
    int max_num_g_probs;
    int num_g_probs;
#ifdef PLOT_MIN_CON_PROB
    int plot_id;
#endif
    int num_g_prob_resets;
#endif
    Vector* log_pair_level_priors_vp = NULL;   /* Perhaps should be over clusters? */
    Matrix_vector* log_pair_level_probs_mvp = NULL;   /* Perhaps should be over clusters? */
    Int_matrix* match_mp = NULL;   /* Perhaps should be over clusters? */
    Matrix* log_pair_probs_mp = NULL;   /* Perhaps should be over clusters? */
    int match, num_matches;
    Int_vector* dis_item_match_count_vp = NULL;
    Int_vector* con_item_match_count_vp = NULL;
    Vector* con_item_max_log_vp = NULL;
    Vector* dis_item_max_log_vp = NULL;
    Vector* mf_dis_item_max_log_vp = NULL;
    Vector* mf_con_item_max_log_vp = NULL;
    double dis_max_log_prob;
    double con_max_log_prob;
    double dis_prob, con_prob;
#ifdef DEF_OUT
    int min_ll_diff_exp = 0;
    int ll_diff_exp;
#endif

    Vector* log_nu_vp                 = NULL;
    Vector* nu_vp                     = NULL;
    Vector* nu_for_dependent_model_vp = NULL;
    Vector* nu_based_on_dis_items_vp  = NULL;
    Vector* nu_based_on_pairs_vp      = NULL;
    Vector* nu_with_prior_vp = NULL;
    Matrix* nu_with_prior_mp = NULL;
    Vector* word_prior_vp = NULL;

#ifdef SUPPORT_IGNORE_DIS_PROB
    double extra_dis_prob = 0.0;
    int    dis_prob_count = 0;
    double total_dis_prob = 0.0;
    Matrix* p_use_dis_item_mp = NULL;
    double* p_use_dis_item_ptr;
#endif
    double extra_log_con_prob_for_original_data_space = 0;
    const Vector* weight_vp = data_ptr->weight_vp;
#ifdef TEST_WEIGHT_DATA
    Vector* test_weight_vp = NULL;
#endif
    int current_model_is_valid = FALSE;
    int i,c;
    double temp_double;
    int initialize_remaining_nodes;
    int con_item_count = NOT_SET, dis_item_count = NOT_SET;
#ifdef XXX_SUPPORT_TRIMMING
    Vector* con_score_vp = NULL;
    Vector* dis_score_vp = NULL;
    Vector* new_con_score_vp = NULL;
    Vector* new_dis_score_vp = NULL;
    Matrix* con_score_mp = NULL;   /* Used to hack NULLs. Stores for each con_item, the association with every dis_item, for each node. */
    Matrix* dis_score_mp = NULL;
    Indexed_vector* sorted_con_score_vp = NULL;
    Indexed_vector* sorted_dis_score_vp = NULL;
    int apply_score_cutoff = FALSE;
    double con_score_cutoff = DBL_HALF_MOST_NEGATIVE;
    double dis_score_cutoff = DBL_HALF_MOST_NEGATIVE;
    Int_vector* con_score_counts_vp = NULL;
    Int_vector* dis_score_counts_vp = NULL;
#ifdef DEBUG_TRIMMING
    int dis_items_used;
    int total_dis_items;
    int con_items_used;
    int total_con_items;
#endif
    int num_runs = 1, run;
#else
    const double con_score_cutoff = DBL_HALF_MOST_NEGATIVE;
#endif
    int num_nulls = 0;
#ifdef USE_STANDARD_GUASS_NORM_CONSTANT
    double log_scale_factor_sqrd = (double)num_used_con_features * log(2.0 * M_PI);
#else
    double log_scale_factor_sqrd = 0.0;
#endif
#ifdef REPORT_MEMORY_USE
    unsigned long total_bytes_used;
#endif
    Int_vector_vector* missing_index_vvp = NULL;
    Vector* dis_item_empirical_vp = NULL;


    CHECK_FOR_HEAP_OVERRUNS();

#ifdef REPORT_MEMORY_USE
    if (get_allocated_memory(&total_bytes_used) != ERROR)
    {
        dbu(total_bytes_used);
    }
#endif


#ifdef REPORT_CPU_TIME
    cpu_time_offset = get_cpu_time();
    all_cpu_offset  = cpu_time_offset;
#endif

    BUFF_GET_USER_ID(user_id);
    BUFF_CPY(halt_file_name, TEMP_DIR);
    BUFF_CAT(halt_file_name, DIR_STR);
    BUFF_CAT(halt_file_name, user_id);
    BUFF_CAT(halt_file_name, DIR_STR);
    BUFF_CAT(halt_file_name, "halt-hc");


    /* Value for initialization. Later this variable holds the current number
    // of limited clusters.
    */
    if (KJB_IS_SET(options_ptr->num_limited_clusters))
    {
        if (options_ptr->initial_model_is_subset)
        {
            set_error("Subset models with cluster limits not implemented yet.\n");
            add_error("(Not checked, anyway).\n");
            return ERROR;
        }

        num_limited_clusters = MIN_OF(options_ptr->num_limited_clusters,
                                      num_clusters);
    }

    if (    (options_ptr->initial_model_is_subset)
         && (options_ptr->initial_model_is_valid)
       )
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }


    if (    (options_ptr != NULL)
         && (    (options_ptr->convergence_criterion == MAX_HELD_OUT_LL)
              || (options_ptr->convergence_criterion == MAX_COMBINED_LL)
              || (options_ptr->convergence_criterion == FIRST_HELD_OUT_LL_MAX)
            )
       )
    {
        if (    (held_out_data_ptr == NULL)
             || (held_out_data_ptr->id_vp == NULL)
             || (held_out_data_ptr->id_vp->length <= 0)
           )
        {
            set_error("Phase two needs held out data but there is none.");
            return ERROR;
        }

        need_held_out_log_likelihood = TRUE;
    }

    if (options_ptr->initial_model_is_valid)
    {
        con_log_norm = model_ptr->con_log_norm;
        min_con_log_prob = model_ptr->min_con_log_prob;
    }
    else
    {
        /* One iteration is not stable unless the model is already valid. */
        max_num_em_iterations = MAX_OF(2, max_num_em_iterations);
    }

    /*
     * FIXME
     *
     * Eventually, this will not be true, and a more complicated
     * specification will be required based on there being at least some
     * dependent items.
     *
     */

#ifdef TRY_WITHOUT
    if (    (options_ptr->model_correspondence > 0)
         && (    (max_num_dis_items <= 0)
              || (max_num_con_items <= 0)
            )
       )
    {
        set_error("Modeling correspondence requires both dis and con items.");
        return ERROR;
    }
#endif 

    if (    (    (options_ptr->initial_model_is_valid)
              || (options_ptr->initial_model_is_subset)
            )
         && ((options_ptr->model_correspondence  > 0) && (max_num_dis_items > 0)) 
       )
    {
        /* Untested since addition of last level parameter in topology. */
        /* Dec 10, 2004 */
        UNTESTED_CODE();

        result = copy_matrix_vector(&log_P_i_n_mvp, model_ptr->P_i_n_mvp);
        if (result == ERROR) { NOTE_ERROR(); }

        for (level = first_level; level < log_P_i_n_mvp->length; level++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = ow_log_matrix_elements(log_P_i_n_mvp->elements[ level ]);
        }

        verify_matrix_vector(log_P_i_n_mvp, NULL);
    }

    if (norm_var_vp != NULL)
    {
        /* Aug 30, 2004; switched from num_con_features to
         * num_used_con_features.
        */
        extra_log_con_prob_for_original_data_space += num_used_con_features * log(norm_stdev);

        for (con_feature = 0; con_feature < num_con_features; con_feature++)
        {
            /* Aug 30, 2004; only use enabled features. */
            if ((con_feature_enable_vp == NULL) || (con_feature_enable_vp->elements[ con_feature ]))
            {
                extra_log_con_prob_for_original_data_space -=
                    0.5 * log(norm_var_vp->elements[ con_feature ]);
            }
        }
    }

    if (result != ERROR)
    {
        result = initialize_vector_feature_counts(con_feature_enable_vp, con_feature_group_vp);
    }

    if (    (result != ERROR)
         && (fs_emulate_uniform_discrete_prob)
         && (dis_item_mp != NULL)
       )
    {
        result = get_zero_vector(&dis_item_empirical_vp,
                                 num_categories);

        if (result != ERROR)
        {
            for (point = 0; point < num_points; point++)
            {
                for (dis_item = 0; dis_item < max_num_dis_items; dis_item++)
                {
                    category = dis_item_mp->elements[ point ][ dis_item ];

                    if (category < 0) break;

                    if (dis_item_multiplicity_mp != NULL)
                    {
                        dis_item_empirical_vp->elements[ category ] += dis_item_multiplicity_mp->elements[ point ][ dis_item ];
                    }
                    else
                    {
                        dis_item_empirical_vp->elements[ category ] += 1.0;
                    }
                }
            }

            result = ow_scale_vector_by_sum(dis_item_empirical_vp);
        }
    }

#ifdef SUPPORT_ANNEALING
    if ((result != ERROR) && (fs_num_cooling_iterations > 0))
    {
        temperature = fs_min_temperature;
        temperature_factor = exp(log(fs_max_temperature/fs_min_temperature)/
                                            (double)fs_num_cooling_iterations);
    }
#endif


#ifdef TEST_WEIGHT_DATA
    if ((result != ERROR) && (fs_weight_data) && (weight_vp == NULL))
    {
        ERE(get_random_vector(&test_weight_vp, num_points));
        weight_vp = test_weight_vp;
        write_col_vector(weight_vp, "weight_vp");
    }
#endif

    if (result != ERROR)
    {
        dbp("");
        dbp("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
        dbp("");
        dbp("       Hierarchical clustering parameters.");
        dbp("");
#ifdef DEFINE_ASSERTS
        dbp("Asserts are defined.");
#else
        dbp("Asserts are NOT defined.");
#endif
        dbp("");
        dbi(num_points);
        dbi(num_categories);
        dbi(num_clusters);
        dbi(max_num_items);
        dbi(num_levels);
        dbi(last_level_num);
        dbi(first_level);
        dbi(num_limited_clusters);
        dbi(num_con_features);
        dbi(num_used_con_features);
        dbp(" ");
        dbf(max_num_dis_items_with_multiplicity);
        dbi(max_num_dis_items);
        dbi(max_num_con_items);
        dbp(" ");
        dbi(options_ptr->uniform_vertical_dist);
        dbi(options_ptr->cluster_dependent_vertical_dist);
        dbi(options_ptr->initial_model_is_valid);
        dbi(options_ptr->model_correspondence);
        dbi(options_ptr->share_matches);
        dbp("");
#ifdef XXX_SUPPORT_TRIMMING
        dbe(options_ptr->trim_fraction);
#endif
        dbf(con_log_norm);
        dbe(extra_log_con_prob_for_original_data_space);
        dbp("");
        dbp("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
        dbp("");

        ASSERT_IS_POSITIVE_INT(num_clusters);

        model_ptr->num_clusters     = num_clusters;
        model_ptr->num_categories   = num_categories;

        /* This should already be set, if called from "it". */
        model_ptr->num_con_features = num_con_features;

        /* This gets updated if we are using con prob norm. */
        model_ptr->con_log_norm = con_log_norm;
        model_ptr->min_con_log_prob = min_con_log_prob;
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    /*
    // In the case of "correspondence", it would be nice to write out a value
    // where word prediction goes down, if there is such a thing (it should not
    // have much of an effect on the other models, except in the case where we
    // want to say that we cannot predict anything at all). However, for calls
    // to get_cluster_membership_2 from *this* routine, con_score_cutoff should
    // be set so that it does NOT get used.
    */
    model_ptr->con_score_cutoff = con_score_cutoff;

    if (result != ERROR)
    {
        result = index_matrix_vector_rows_with_missing(&missing_index_vvp, con_item_mvp);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = copy_topology(&(model_ptr->topology_ptr), topology_ptr);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&nu_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&log_nu_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

#ifdef SUPPORT_ANNEALING
    if (result != ERROR)
    {
        result = get_zero_vector(&annealed_prob_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }
    if (result != ERROR)
    {
        result = get_zero_vector(&annealed_log_prob_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }
#endif

    if (result != ERROR)
    {
        result = get_zero_vector(&log_prob_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&prob_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_initialized_vector(&(model_ptr->fit_vp), num_points, 2.0 * LOG_ZERO);
        if (result == ERROR)
        {
            NOTE_ERROR();
        }
        else
        {
            fit_vp = model_ptr->fit_vp;
        }
    }

    if ((result != ERROR) && (max_num_dis_items > 0))
    {
        result = get_target_matrix_vector(&new_P_i_n_mvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (    (result != ERROR)
         && (    (fs_watch_max_cluster_given_point_diff)
              || (fs_watch_rms_cluster_given_point_diff)
           )
       )
    {
        result = get_zero_matrix(&prev_P_c_p_mp, num_points, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
#ifdef TEST
        /*
         * Invalid memory created while developing code to switch the number of
         * clusters in mid-stream.
        */
        result = get_initialized_vector(&ll_P_c_p_vp, num_clusters, DBL_NOT_SET);
#else
        result = get_target_vector(&ll_P_c_p_vp, num_clusters);
#endif
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if ((result != ERROR) && (max_num_con_items > 0))
    {
        result = get_target_matrix_vector(&new_mean_mvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if ((result != ERROR) && (max_num_con_items > 0))
    {
        result = get_target_matrix_vector(&new_var_mvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if ((result != ERROR) && (max_num_con_items > 0))
    {
        result = get_target_v3(&con_prob_cache_vvvp,
                               max_num_con_items);
        if (result == ERROR) { NOTE_ERROR(); }

        /* Untested since addition of last level parameter in topology. */
        /* Dec 10, 2004 */
        UNTESTED_CODE();

        for (con_item = 0; con_item < max_num_con_items; con_item++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_target_vector_vector(&(con_prob_cache_vvvp->elements[ con_item ]),
                                              num_levels);
            if (result == ERROR) { NOTE_ERROR(); }

            for (level = first_level; level < last_level_num; level++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }

                num_nodes_this_level = level_counts_vp->elements[ level ];

#ifdef TEST
                /*
                 * Invalid memory created while developing code to switch the number of
                 * clusters in mid-stream.
                */
                result = get_initialized_vector(&(con_prob_cache_vvvp->elements[ con_item ]->elements[ level ]),
                                                num_nodes_this_level, DBL_NOT_SET);
#else
                result = get_target_vector(&(con_prob_cache_vvvp->elements[ con_item ]->elements[ level ]),
                                           num_nodes_this_level);
#endif
#ifdef TRACK_OVERRUN_BUG
                dbi(con_item);
                dbi(level);
                dbi(num_nodes_this_level);
                dbx(con_prob_cache_vvvp->elements[ con_item ]->elements[ level ]);
                dbx(con_prob_cache_vvvp->elements[ con_item ]->elements[ level ]->elements);
#endif
            }
        }
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

#ifdef SUPPORT_ANNEALING
    if ((result != ERROR) && (max_num_con_items > 0))
    {
        result = get_target_v3(&annealed_con_prob_cache_vvvp,
                               max_num_con_items);
        if (result == ERROR) { NOTE_ERROR(); }

        /* Untested since addition of last level parameter in topology. */
        /* Dec 10, 2004 */
        UNTESTED_CODE();

        for (con_item = 0; con_item < max_num_con_items; con_item++)
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = get_target_vector_vector(&(annealed_con_prob_cache_vvvp->elements[ con_item ]),
                                              num_levels);

            for (level = first_level; level < last_level_num; level++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }

                num_nodes_this_level = level_counts_vp->elements[ level ];

                result = get_target_vector(&(annealed_con_prob_cache_vvvp->elements[ con_item ]->elements[ level ]),
                                           num_nodes_this_level);
            }
        }
    }
#endif

#ifdef VEC_AND_HIS
XXX if ((result != ERROR) && (max_num_vec_items > 0))
XXX {
XXX     result = get_target_v4(&vec_mean_vvvvp, num_levels);
XXX }
XXX
XXX if ((result != ERROR) && (max_num_vec_items > 0))
XXX {
XXX     result = get_target_v3(&vec_var_vvvp, num_levels);
XXX }
XXX
XXX if ((result != ERROR) && (max_num_his_items > 0))
XXX {
XXX     result = get_target_v4(&his_mean_vvvvp, num_levels);
XXX }
XXX
XXX if ((result != ERROR) && (max_num_his_items > 0))
XXX {
XXX     result = get_target_v3(&his_var_vvvp, num_levels);
XXX }
#endif

    if (result != ERROR)
    {
        result = get_target_vector_vector(&dis_P_sum_vvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_target_vector_vector(&con_P_sum_vvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_target_v3(&missing_con_P_sum_vvvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if ((result != ERROR) && (max_num_con_items > 0) && (fs_do_map))
    {
        result = get_target_matrix_vector(&omega_mvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if ((result != ERROR) && (max_num_con_items > 0) && (fs_do_map))
    {
        result = get_target_matrix_vector(&delta_inverse_mvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (result != ERROR)
    {
        level_indicators_ppp = allocate_3D_double_array(num_levels,
                                                        num_clusters,
                                                        max_num_items);
        if (level_indicators_ppp == NULL)
        {
            result = ERROR;
            NOTE_ERROR();
        }
#ifdef REPORT_MEMORY_USE
        else if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            dbu(total_bytes_used);
        }
#endif
    }

    if (    (result != ERROR)
         && (options_ptr->model_correspondence > 1)
       )
    {
        ASSERT_IS_POSITIVE_INT(max_num_dis_items);
        ASSERT_IS_POSITIVE_INT(max_num_con_items);

        result = get_target_matrix_vector(&log_pair_level_probs_mvp,
                                          num_levels);
        if (result == ERROR) { NOTE_ERROR(); }

        if (options_ptr->share_matches)
        {
            if (result != ERROR)
            {
                pair_probs_1_ppp = allocate_3D_double_array(num_clusters,
                                                            num_levels,
                                                            max_num_dis_items);

                if (pair_probs_1_ppp == NULL)
                {
                    result = ERROR;
                    NOTE_ERROR();
                }
            }

            if (result != ERROR)
            {
                pair_probs_2_ppp = allocate_3D_double_array(num_clusters,
                                                            num_levels,
                                                            max_num_con_items);

                if (pair_probs_2_ppp == NULL)
                {
                    result = ERROR;
                    NOTE_ERROR();
                }
            }

            if (result != ERROR)
            {
                log_pair_probs_1_ppp = allocate_3D_double_array(num_clusters,
                                                                num_levels,
                                                                max_num_dis_items);

                if (log_pair_probs_1_ppp == NULL)
                {
                    result = ERROR;
                    NOTE_ERROR();
                }
            }

            if (result != ERROR)
            {
                log_pair_probs_2_ppp = allocate_3D_double_array(num_clusters,
                                                                num_levels,
                                                                max_num_con_items);

                if (log_pair_probs_2_ppp == NULL)
                {
                    result = ERROR;
                    NOTE_ERROR();
                }
            }
        }
        else
        {
            for (level = first_level; level < last_level_num; level++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = get_target_matrix(&(log_pair_level_probs_mvp->elements[ level ]),
                                           max_num_con_items, max_num_dis_items);
            }
        }

        if (result != ERROR)
        {
#ifdef TEST
            result = get_zero_matrix(&log_pair_probs_mp,
                                     max_num_con_items, max_num_dis_items);
#else
            result = get_target_matrix(&log_pair_probs_mp,
                                       max_num_con_items, max_num_dis_items);
#endif
            if (result == ERROR) { NOTE_ERROR(); }
        }

#ifdef OBSOLETE
        if (result != ERROR)
        {
            result = get_target_int_matrix(&match_mp, max_num_matches, 2);
            if (result == ERROR) { NOTE_ERROR(); }
        }
#endif
    }

    if (    (result != ERROR)
         && (options_ptr->model_correspondence == 2)
       )
    {
        if (result != ERROR)
        {
            result = get_target_int_vector(&dis_item_match_count_vp,
                                           max_num_dis_items);
            if (result == ERROR) { NOTE_ERROR(); }
        }

        if (result != ERROR)
        {
            result = get_target_int_vector(&con_item_match_count_vp,
                                           max_num_con_items);
            if (result == ERROR) { NOTE_ERROR(); }
        }

        if (result != ERROR)
        {
            result = get_target_vector(&dis_item_max_log_vp,
                                       max_num_dis_items);
            if (result == ERROR) { NOTE_ERROR(); }
        }

        if (result != ERROR)
        {
            result = get_target_vector(&con_item_max_log_vp,
                                       max_num_con_items);
            if (result == ERROR) { NOTE_ERROR(); }
        }

        if (result != ERROR)
        {
            result = get_target_vector(&mf_dis_item_max_log_vp, max_num_dis_items);
            if (result == ERROR) { NOTE_ERROR(); }
        }

        if (result != ERROR)
        {
            result = get_target_vector(&mf_con_item_max_log_vp, max_num_con_items);
            if (result == ERROR) { NOTE_ERROR(); }
        }
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (    (result != ERROR)
         && (options_ptr->model_correspondence == 3)
       )
    {
        result = get_target_vector(&log_pair_level_priors_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

#ifdef TEST
    if (max_num_dis_items > 0)
    {
        ASSERT_IS_POSITIVE_INT(num_categories);
    }
#endif

    if ((result != ERROR) && (max_num_dis_items > 0) && (fs_do_map))
    {
        result = get_initialized_vector(&map_alpha_vp, num_clusters,
                                        fs_map_alpha);

        if (result != ERROR)
        {
            result = get_target_v3(&map_beta_minus_one_vvvp, num_levels);
        }
        else
        {
            if (result == ERROR) { NOTE_ERROR(); }
        }

        for (level = first_level; level < last_level_num; level++)
        {
            dbi(level);

            if (result == ERROR) { NOTE_ERROR(); break; }

            num_nodes_this_level = level_counts_vp->elements[ level ];

            result = get_target_vector_vector(
                                          &(map_beta_minus_one_vvvp->elements[ level ]),
                                          num_nodes_this_level);
            if (result == ERROR) { NOTE_ERROR(); }

            map_beta_minus_one_vvp = map_beta_minus_one_vvvp->elements[ level ];

#define MACH_THREE

            for (node = 0; node < num_nodes_this_level; node++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef MACH_TWO
                result = get_zero_vector(&(map_beta_minus_one_vvp->elements[ node ]),
                                         num_categories);
                if (level < (num_levels - 1)) continue;
#else
#ifdef MACH_THREE
                result = get_zero_vector(&(map_beta_minus_one_vvp->elements[ node ]),
                                        num_categories);
#else
                result = get_initialized_vector(
                                        &(map_beta_minus_one_vvp->elements[ node ]),
                                        num_categories, fs_map_beta_minus_one);
#endif
#endif

                if (result == ERROR) { NOTE_ERROR(); break; }

                map_beta_minus_one_vp = map_beta_minus_one_vvp->elements[ node ];

                if ((fs_use_level_prior) && (dis_item_level_vp != NULL))
                {
                    for (category = 0; category < num_categories; category++)
                    {
                        dis_item_level = dis_item_level_vp->elements[ category ];

                        if (dis_item_level > 0)
                        {
#ifdef DEF_OUT
                            int temp_level;
                            double temp_level_diff;
#endif

                            /*
                            // Account for the fact that we skip the first one.
                            */
                            dis_item_level--;
                            if (dis_item_level < 0) dis_item_level = 0;

                            rel_level = ((double)dis_item_level - 1.0) / 8.0;
                            rel_level *= (double)(num_levels -1.0);
                            rel_level = MIN_OF(rel_level, (double)(num_levels - 1.0));

                            level_diff = ABS_OF(rel_level - (double)level);

#ifdef MACH_ONE
                            map_beta_minus_one_vp->elements[ category ] /= ((level_diff + 1.0) * (level_diff + 1.0));
                            map_beta_minus_one_vp->elements[ category ] *= 2.0;
#endif
#ifdef MACH_TWO
                            map_beta_minus_one_vp->elements[ category ] = fs_map_beta_minus_one *
                                                                                     exp(-level_diff);
#endif
#ifdef MACH_THREE
                            map_beta_minus_one_vp->elements[ category ] = fs_map_beta_minus_one *
                                                                                     exp(-2.0 * level_diff);
#endif

#ifdef DEF_OUT
                            for (temp_level = first_level; temp_level < last_level_num; temp_level++)
                            {
                                temp_level_diff = ABS_OF(rel_level - (double)temp_level);

                                if (temp_level_diff < level_diff)
                                {
                                    map_beta_minus_one_vp->elements[ category ] = 1.0;
                                    break;
                                }
                            }
#endif
                        }
                    }
                }

                if (node == 0)
                {
                    db_rv(map_beta_minus_one_vp);
                }
            }
        }
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if ((result != ERROR) && (max_num_con_items > 0) && (fs_do_map))
    {
        /* Untested since addition of last level parameter in topology. */
        /* Dec 10, 2004 */
        UNTESTED_CODE();

        result = get_zero_vector(&map_r_minus_n_g_vp, num_levels);

        level = first_level + 1;

        while (level < last_level_num)
        {
            map_r_minus_n_g = fs_map_r_minus_n_g /
                                        (double)(last_level_num - level);
            level++;
        }
    }

    if ((result != ERROR) && (max_num_dis_items > 0))
    {
        result = get_dis_factors(&dis_factor_vp,
                                 dis_item_mp,
                                 dis_item_multiplicity_mp,
                                 max_num_dis_items_with_multiplicity,
                                 fs_norm_items,
                                 fs_dis_prob_boost_factor);
        verify_vector(dis_factor_vp, NULL);
    }

    if (    (result != ERROR)
         && (max_num_dis_items > 0)
         && (need_held_out_log_likelihood)
       )
    {
        result = get_dis_factors(&held_out_dis_factor_vp,
                                 held_out_dis_item_mp,
                                 held_out_dis_item_multiplicity_mp,
                                 max_num_held_out_dis_items_with_multiplicity,
                                 fs_norm_items,
                                 fs_dis_prob_boost_factor);
        verify_vector(held_out_dis_factor_vp, NULL);
    }

    if ((result != ERROR) && (max_num_con_items > 0))
    {
        result = get_con_factors(&con_factor_vp, con_item_mvp,
                                 max_num_con_items,
                                 fs_norm_items,
                                 fs_con_prob_boost_factor);
        verify_vector(con_factor_vp, NULL);
    }

    if (    (result != ERROR)
         && (max_num_con_items > 0)
         && (need_held_out_log_likelihood)
       )
    {
        result = get_con_factors(&held_out_con_factor_vp,
                                 held_out_con_item_mvp,
                                 max_num_held_out_con_items,
                                 fs_norm_items,
                                 fs_con_prob_boost_factor);
        verify_vector(held_out_con_factor_vp, NULL);
    }


#ifdef VEC_AND_HIS
XXX if ((result != ERROR) && (max_num_vec_items > 0))
XXX {
XXX     result = get_vec_factors(&vec_factor_vp, vec_item_vvvvp,
XXX                              max_num_vec_items,
XXX                              fs_norm_items,
XXX                              fs_vec_prob_boost_factor);
XXX }
XXX
XXX if ((result != ERROR) && (max_num_his_items > 0))
XXX {
XXX     result = get_his_factors(&his_factor_vp, his_item_vvvvp,
XXX                              max_num_his_items,
XXX                              fs_norm_items,
XXX                              fs_his_prob_boost_factor);
XXX }
#endif

    if (result != ERROR)
    {
#ifdef REPORT_MEMORY_USE
        if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            dbu(total_bytes_used);
        }
#endif

        result = get_initialized_matrix(&(model_ptr->P_c_p_mp), 
                                        num_points, num_clusters,
                                        DBL_NOT_SET);
    }

    if (result != ERROR)
    {
        P_c_p_mp = model_ptr->P_c_p_mp;
        /* Must init for subset model. */
        result = get_zero_vector(&P_c_p_vp, num_clusters);
    }

    if (    (result != ERROR)
         && (max_num_con_items > 0)
       )
    {
        result = get_target_vector_vector(&new_con_log_sqrt_det_vvp,
                                          num_levels);

        for (level = first_level; level < last_level_num; level++)
        {
            num_nodes_this_level = level_counts_vp->elements[ level ];

            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef TEST
            result = get_initialized_vector(&(new_con_log_sqrt_det_vvp->elements[ level ]),
                                            num_nodes_this_level,
                                            DBL_NOT_SET);
#else
            result = get_target_vector(&(new_con_log_sqrt_det_vvp->elements[ level ]),
                                       num_nodes_this_level);
#endif
        }
    }

    if (    (result != ERROR)
         && (max_num_con_items > 0)
         && (options_ptr->initial_model_is_valid )
         && (fs_do_map)
       )
    {
        result = copy_matrix_vector(&omega_mvp, model_ptr->mean_mvp);

        if (result != ERROR)
        {
            result = copy_matrix_vector(&delta_inverse_mvp,
                                        model_ptr->var_mvp);
        }
    }

    if (    (result != ERROR)
         && (num_limited_clusters != num_clusters)
       )
    {
        result = get_target_int_matrix(&cluster_index_mp, num_points,
                                       num_limited_clusters);
    }

#ifdef XXX_SUPPORT_TRIMMING
    if (options_ptr->trim_fraction > 0.0)
    {
        if (result != ERROR)
        {
            /*
            // For joint, we need room for each pair, thus the
            // max_num_con_items.
            */
            result = get_target_matrix(&dis_score_mp, max_num_dis_items,
                                       num_levels * num_clusters * max_num_con_items);
        }

        if (result != ERROR)
        {
            /*
            // For joint, we need room for each pair, thus the
            // max_num_dis_items.
            */
            result = get_target_matrix(&con_score_mp, max_num_con_items,
                                       num_levels * num_clusters * max_num_dis_items);
        }

        if (result != ERROR)
        {
            result = get_target_int_vector(&dis_score_counts_vp,
                                           max_num_dis_items);
        }

        if (result != ERROR)
        {
            result = get_target_int_vector(&con_score_counts_vp,
                                           max_num_con_items);
        }

        if (result != ERROR)
        {
            /* Set to zero so verification during TEST works. */
            result = get_zero_vector(&new_dis_score_vp, max_num_dis_items * num_points);
        }

        if (result != ERROR)
        {
            /* Set to zero so verification during TEST works. */
            result = get_zero_vector(&new_con_score_vp, max_num_con_items * num_points);
        }

        if (result != ERROR)
        {
            result = get_target_indexed_vector(&sorted_dis_score_vp, max_num_dis_items * num_points);
        }

        if (result != ERROR)
        {
            result = get_target_indexed_vector(&sorted_con_score_vp, max_num_con_items * num_points);
        }
    }
#endif

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if ((result != ERROR) && fs_watch)
    {
        head_len = 30;

        if (need_held_out_log_likelihood) head_len += 15;
        if (need_held_out_log_likelihood) head_len += 13;
#ifdef SUPPORT_ANNEALING
        if (fs_watch_temperature) head_len += 13;
#endif
        if (fs_watch_max_con_prob) head_len += 12;
#ifdef VEC_AND_HIS
        if (fs_watch_max_vec_prob) head_len += 12;
#endif
        if (fs_watch_max_cluster_given_point_diff) head_len += 12;
        if (fs_watch_rms_cluster_given_point_diff) head_len += 12;
        if (fs_watch_max_parm_diff) head_len += 12;
        if (fs_watch_ave_limited_error) head_len += 12;
        if (fs_watch_max_limited_error) head_len += 12;

        rep_print(stdout, '=', head_len);
        pso("\n");

        pso(" it  |    %s     |   diff   ", fs_do_map ? "LP/N" : "LL/N");

        if (need_held_out_log_likelihood) pso(" | Held out LL");
        if (need_held_out_log_likelihood) pso(" |   diff   ");
#ifdef SUPPORT_ANNEALING
        if (fs_watch_temperature) pso(" |   temp    ");
#endif
        if (fs_watch_max_con_prob) pso(" | max con P");
#ifdef VEC_AND_HIS
        if (fs_watch_max_vec_prob) pso(" | max vec P");
#endif
        if (fs_watch_rms_cluster_given_point_diff) pso(" | rms P c  ");
        if (fs_watch_max_cluster_given_point_diff) pso(" | max P c  ");
        if (fs_watch_max_parm_diff) pso(" | max parm ");
        if (fs_watch_ave_limited_error) pso(" | ave lim  ");
        if (fs_watch_max_limited_error) pso(" | max lim  ");
        pso("\n");
        rep_print(stdout, '-', head_len);
        pso("\n");
    }

#ifdef REPORT_CPU_TIME
    initialization_cpu = get_cpu_time() - cpu_time_offset;
#endif

    set_iteration_atn_trap();

    /*
    //  -------------------------- Main EM Loop -------------------------------
    */

    CHECK_FOR_HEAP_OVERRUNS();

#ifdef SUPPORT_NUM_CLUSTER_SWITCH     /* Not finished. Should it be? */
    dbp("************************************************************");
    dbp("************************************************************");
    dbp("************************************************************");
    db_irv(level_counts_vp);

    if (    ( ! fs_use_specified_clusters)
         && (last_level_num == (num_levels - 1))
         && (num_levels >= 2)
         && (level_counts_vp->elements[ num_levels - 2 ] == 1)
       )
    {
        num_clusters = 1;
        num_clusters_used = 1;
        num_limited_clusters = 1;
        num_sub_clusters = 1;
    }

    dbp("************************************************************");
    dbp("************************************************************");
    dbp("************************************************************");
#endif


    for (it = 0; it < max_num_em_iterations; it++)
    {
        int num_good_points = 0;


        CHECK_FOR_HEAP_OVERRUNS();

#ifdef REPORT_MEMORY_USE
        if (    (result != ERROR)
             && (get_allocated_memory(&total_bytes_used) != ERROR)
           )
        {
            dbu(total_bytes_used);
        }
#endif

#ifdef DEBUG_TRIMMING
        dis_items_used = 0;
        total_dis_items = 0;
        con_items_used = 0;
        total_con_items = 0;

        dbe(con_log_norm);
#endif

        changed_number_of_parameters = FALSE;
        var_offset = fs_var_offset + fs_var_offset_2 / (1.0 + it/5);
        var_offset *= (norm_stdev * norm_stdev);

#ifdef REPORT_CPU_TIME
        cpu_time_offset = get_cpu_time();
#endif

#ifdef XXX_LIMIT_MIN_CON_PROB
        max_num_g_probs = num_points * max_num_con_items * num_levels * num_clusters;
        num_g_probs = 0;
        num_g_prob_resets = 0;

        ERE(get_target_vector(&g_prob_vp, max_num_g_probs));
#endif

        /*
        // ------------------  E Step Initialization --------------------------
        */

        if (    ( ! valid_cluster_index) /* Only flip the switch once. */
             && ((it != 0) || (options_ptr->initial_model_is_valid))
             && (num_limited_clusters != num_clusters)
             && (    (fs_max_ave_limited_cluster_error < 0.0)  /* Not set */
                  || (    (ave_limited_cluster_error > 0.0)
                       && (ave_limited_cluster_error < fs_max_ave_limited_cluster_error)
                     )
                )
           )
        {
            verbose_pso(2, "Using limited cluster membership.\n");

            valid_cluster_index = TRUE;
            num_clusters_used = num_limited_clusters;

            P_c_p_mp->num_cols  = num_limited_clusters;
            P_c_p_vp->length    = num_limited_clusters;
            ll_P_c_p_vp->length = num_limited_clusters;
        }

        if (    ( ! compute_cluster_dependent_aspects)  /* Only flip the switch once. */
             && ( ! options_ptr->uniform_vertical_dist)
             && ( options_ptr->cluster_dependent_vertical_dist)
             && (    (num_clusters_used == num_limited_clusters)
                  || (valid_cluster_index)
                )
            )
        {
            compute_cluster_dependent_aspects = TRUE;

            result = get_target_matrix_vector(&(model_ptr->P_l_p_c_mvp), num_levels);

            if (result != ERROR)
            {
                P_l_p_c_mvp = model_ptr->P_l_p_c_mvp;
            }

            for (level = first_level; level < last_level_num; level++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }
#ifdef TEST
                result = get_initialized_matrix(&(P_l_p_c_mvp->elements[ level ]),
                                                num_points, num_clusters_used,
                                                DBL_NOT_SET);
#else
                result = get_target_matrix(&(P_l_p_c_mvp->elements[ level ]),
                                           num_points, num_clusters_used);
#endif
            }
            verbose_pso(2, "Computing cluster dependent aspects.\n");
        }

        if (    ( ! compute_aspects)  /* Only flip the switch once. */
             && ( ! options_ptr->uniform_vertical_dist)
             && ( ! options_ptr->cluster_dependent_vertical_dist)
             && (    (num_clusters_used == num_limited_clusters)
                  || (valid_cluster_index)
                )
            )
        {
            compute_aspects = TRUE;

#ifdef TEST
            result = get_initialized_matrix(&(model_ptr->P_l_p_mp),
                                            num_levels, num_points,
                                            DBL_NOT_SET);
#else
            result = get_target_matrix(&(model_ptr->P_l_p_mp),
                                       num_levels, num_points);
#endif

            if (result != ERROR)
            {
                P_l_p_mp = model_ptr->P_l_p_mp;
            }

            verbose_pso(2, "Computing standard aspects.\n");
        }

        if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef SUPPORT_IGNORE_DIS_PROB
        result = get_unity_matrix(&p_use_dis_item_mp, num_clusters_used,
                                  max_num_dis_items);
        if (result == ERROR) { NOTE_ERROR(); break; }
#endif

#ifdef SUPPORT_IGNORE_DIS_PROB
        if (dis_prob_count > 0)
        {
            extra_dis_prob = fs_ignore_dis_prob_fraction * total_dis_prob / dis_prob_count;
            dis_prob_count = 0;
            total_dis_prob = 0.0;
        }
#endif

        /*
        //                   V Step initializations
        */

        result = get_zero_matrix(&new_V_mp, num_levels, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        if (max_num_con_items > 0)
        {
            result = get_zero_matrix(&new_con_V_mp, num_levels,
                                     num_clusters);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }

        if (max_num_dis_items > 0)
        {
            result = get_zero_matrix(&new_dis_V_mp, num_levels,
                                     num_clusters);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }


        CHECK_FOR_HEAP_OVERRUNS();

        /*
        //                   M Step initializations
        */

        result = get_zero_vector(&new_a_vp, num_clusters);
        if (result == ERROR) { NOTE_ERROR(); break; }

        if (max_num_dis_items > 0)
        {
            for (level = first_level; level < last_level_num; level++)
            {
                num_nodes_this_level = level_counts_vp->elements[ level ];

                result = get_zero_vector(&(dis_P_sum_vvp->elements[ level]),
                                         num_nodes_this_level);
                if (result == ERROR) { NOTE_ERROR(); break; }

                if (map_beta_minus_one_vvvp == NULL)
                {
                    result = get_zero_matrix(
                                        &(new_P_i_n_mvp->elements[ level ]),
                                        num_nodes_this_level,
                                        num_categories);
                }
                else
                {
                    result = get_zero_matrix(&(new_P_i_n_mvp->elements[ level ]),
                                             num_nodes_this_level,
                                             num_categories);

                    map_beta_minus_one_vvp = map_beta_minus_one_vvvp->elements[ level ];
                    dis_P_sum_vp = dis_P_sum_vvp->elements[ level ];

                    for (node = 0; node < num_nodes_this_level; node++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        s = sum_vector_elements(map_beta_minus_one_vvp->elements[ node ]);

                        dis_P_sum_vp->elements[ node ] += s;

                        result = ow_add_vector_to_matrix_row(
                                             new_P_i_n_mvp->elements[ level ],
                                             map_beta_minus_one_vvp->elements[ node ],
                                             node);
                    }
                }
            }
        }

        if (max_num_con_items > 0)
        {
            for (level = first_level; level < last_level_num; level++)
            {
                num_nodes_this_level = level_counts_vp->elements[ level ];

                result = get_zero_vector(&(con_P_sum_vvp->elements[ level ]),
                                         num_nodes_this_level);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = get_target_vector_vector(&(missing_con_P_sum_vvvp->elements[ level ]),
                                                  num_nodes_this_level);
                if (result == ERROR) { NOTE_ERROR(); break; }

                for (node = 0; node < num_nodes_this_level; node++)
                {
                    result = get_zero_vector(&(missing_con_P_sum_vvvp->elements[ level ]->elements[ node ]),
                                             num_con_features);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }


                result = get_zero_matrix(&(new_mean_mvp->elements[ level ]),
                                         num_nodes_this_level,
                                         num_con_features);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = get_zero_matrix(&(new_var_mvp->elements[ level ]),
                                         num_nodes_this_level,
                                         num_con_features);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
        }

        if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
        loop_initialization_cpu += (get_cpu_time() - cpu_time_offset);
#endif

        verify_matrix_vector(new_var_mvp, NULL);
        verify_matrix_vector(new_mean_mvp, NULL);
        verify_matrix_vector(new_P_i_n_mvp, NULL);

#ifdef XXX_SUPPORT_TRIMMING
        /*
        //  In trimming, we would do a dry run, and use that to set the trim
        //  level, and then do a second run.
        */
        if (    (options_ptr->trim_fraction > 0.0)
             && (it > 3)
             && (current_model_is_valid)
#ifndef XXX_OLD_PAIR_NULL_METHOD
             && (options_ptr->model_correspondence < 2)
#endif
            )
        {
                num_runs = 2;
        }

        apply_score_cutoff = FALSE;

        for (run = 0; run < num_runs; run++)
        {
            int temp_good_con_item_count = 0;
            int temp_good_dis_item_count = 0;
            int temp_total_con_item_count = 0;
            int temp_total_dis_item_count = 0;
#endif  /* XXX_SUPPORT_TRIMMING */

        log_likelihood = 0.0;

        if (max_num_con_items > 0)
        {
            ll_con_correction_norm_sum = 0.0;
            max_con_log_prob = DBL_MOST_NEGATIVE;
        }

#ifdef VEC_AND_HIS
XXX     if (max_num_vec_items > 0)
XXX     {
XXX         ll_vec_correction_norm_sum = 0.0;
XXX         max_vec_log_prob = DBL_MOST_NEGATIVE;
XXX     }
#endif

#ifdef XXX_SUPPORT_TRIMMING
#ifdef DEBUG_TRIMMING
        dbp("*********************************************************");
        dbi(run);
        dbe(con_log_norm);
#endif

#ifdef REPORT_MEMORY_USE
        if (    (result != ERROR)
             && (get_allocated_memory(&total_bytes_used) != ERROR)
           )
        {
            dbu(total_bytes_used);
        }
#endif

        if (run == 1)
        {
            /*
            // Get cutoffs from the score vectors. Note that in the case of
            // pair emmisions, we essentially compute two very similar numbers.
            // In fact, if we do not allow duplicate matches, the numbers
            // should be identical. However, we calculate them independently so
            // that this code applies to all cases.
            */
            int cutoff_index;
            double score;

            int good_con_item_count = 0;
            int good_dis_item_count = 0;


#ifdef DEBUG_TRIMMING
            dbp("---------------------------------");
            dbi(dis_item_count);
#endif

            new_dis_score_vp->length = dis_item_count;

            for (i = 0; i < dis_item_count; i++)
            {
                score = new_dis_score_vp->elements[ i ];

                if (score > DBL_HALF_MOST_NEGATIVE)
                {
                    sorted_dis_score_vp->elements[ good_dis_item_count ].element = score;

                    /* Index is not really needed. */
                    sorted_dis_score_vp->elements[ good_dis_item_count ].index = good_dis_item_count;

                    good_dis_item_count++;
                }
            }

#ifdef DEBUG_TRIMMING
            dbi(good_dis_item_count);
#endif

            sorted_dis_score_vp->length = good_dis_item_count;

            if (ascend_sort_indexed_vector(sorted_dis_score_vp) == ERROR)
            {
                NOTE_ERROR(); break;
            }

            cutoff_index = (int)((double)good_dis_item_count * options_ptr->trim_fraction);

#ifdef DEBUG_TRIMMING
            dbi(cutoff_index);
#endif
            dis_score_cutoff = sorted_dis_score_vp->elements[ cutoff_index ].element;

            result = copy_vector(&dis_score_vp, new_dis_score_vp);
            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef DEBUG_TRIMMING
            dbe(dis_score_cutoff);
#endif

            /* ------------------------------------------------ */

#ifdef XXX_SUPPORT_TRIMMING
#ifdef DEBUG_TRIMMING
            dbi(con_item_count);
#endif
#endif

            new_con_score_vp->length = con_item_count;

            for (i = 0; i < con_item_count; i++)
            {
                score = new_con_score_vp->elements[ i ];

                if (score > DBL_HALF_MOST_NEGATIVE)
                {
                    sorted_con_score_vp->elements[ good_con_item_count ].element = score;

                    /* Index is not really needed. */
                    sorted_con_score_vp->elements[ good_con_item_count ].index = good_con_item_count;

                    good_con_item_count++;
                }
            }

#ifdef DEBUG_TRIMMING
            dbi(good_con_item_count);
#endif

            sorted_con_score_vp->length = good_con_item_count;

            if (ascend_sort_indexed_vector(sorted_con_score_vp) == ERROR)
            {
                NOTE_ERROR(); break;
            }

            cutoff_index = (int)((double)good_con_item_count * options_ptr->trim_fraction);

#ifdef DEBUG_TRIMMING
            dbi(cutoff_index);
#endif

            con_score_cutoff = sorted_con_score_vp->elements[ cutoff_index ].element;

            result = copy_vector(&con_score_vp, new_con_score_vp);
            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef DEBUG_TRIMMING
            dbe(con_score_cutoff);
#endif

            apply_score_cutoff = TRUE;

        }  /* End trimming wet run stuff. */
#endif

        con_item_count = 0;
        dis_item_count = 0;

#ifdef DEBUG_TRIMMING
        dbi(apply_score_cutoff);
#endif

#ifdef REPORT_MEMORY_USE
        if (    (result != ERROR)
             && (get_allocated_memory(&total_bytes_used) != ERROR)
           )
        {
            dbu(total_bytes_used);
        }
#endif
        CHECK_FOR_HEAP_OVERRUNS();


        for (point = 0; point < num_points; point++)
        {
            double point_weight = 1.0;

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif

            dis_factor    = (dis_factor_vp == NULL) ? 1.0 : dis_factor_vp->elements[ point ];
            con_factor    = (con_factor_vp == NULL) ? 1.0 : con_factor_vp->elements[ point ];
            num_con_items = (max_num_con_items == 0) ? 0 : con_item_mvp->elements[ point ]->num_rows;
            cluster_index_ptr = valid_cluster_index ? cluster_index_mp->elements[ point ] : NULL;
            num_dis_items = 0;

            /*
             * A big of a HACK
             *
             * Needs reworking for generality.
             */
            if (    (options_ptr->model_correspondence == 0)
                 && (max_num_dis_items > 0)
                 && (it < fs_min_it_for_con_items)
               )
            {
                num_con_items = 0;
            }


            if (result == ERROR) { NOTE_ERROR(); break; }

            /* dbi(point); */

            if ((fs_weight_data) && (weight_vp != NULL))
            {
                point_weight = weight_vp->elements[ point ];
            }

            for (dis_item = 0; dis_item < max_num_dis_items; dis_item++)
            {
                category = dis_item_mp->elements[ point ][ dis_item ];

                if (category < 0) break;

                num_dis_items++;
            }

#ifdef TRY_WITHOUT
            if (options_ptr->model_correspondence > 0)
            {
                if ((num_dis_items <= 0) || (num_con_items <= 0))
                {
                    set_error("Point without either a dis or con item found.");
                    add_error("Currently not supported while modeling correspondence.");

                    result = ERROR;
                }

            }

            if (result == ERROR) { NOTE_ERROR(); break; }
#endif 

            ave_limited_cluster_error = DBL_NOT_SET;
            max_limited_cluster_error = DBL_MOST_NEGATIVE;

#ifdef TEST
#ifdef REPORT_CPU_TIME
/* TEST */  cpu_time_offset = get_cpu_time();
#endif
/* TEST */  for (level = 0; level < num_levels; level++)
/* TEST */  {
/* TEST */      level_indicators_pp = level_indicators_ppp[ level ];
/* TEST */
/* TEST */      for (cluster = 0; cluster < num_clusters; cluster++)
/* TEST */      {
/* TEST */          level_indicators_p = level_indicators_pp[ cluster ];
/* TEST */
/* TEST */          for (item = 0; item < max_num_items; item++)
/* TEST */          {
                        if (    (fs_use_specified_clusters)
                             && (data_ptr->group_vp != NULL)
                           )
                        {
                            /* We want to use the indicators for one cluster per
                             * document, so the that ensures use them all will
                             * fail.
                            */
                            *level_indicators_p = 0.0;
                        }
                        else
                        {
/* TEST */                  *level_indicators_p = DBL_NOT_SET;
                        }
/* TEST */              level_indicators_p++;
/* TEST */          }
/* TEST */      }
/* TEST */  }
/* TEST */
/* TEST */  /* Breaks shared correspondence */
/* TEST */  /* result = ow_set_vector(P_c_p_vp, DBL_NOT_SET); */
#ifdef REPORT_CPU_TIME
/* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif

            initialize_remaining_nodes = FALSE;

            /*
             * If the initial model is a subset to be expanded, then we do the
             * E-step, hit a goto, and come here to fill in the rest of the
             * nodes. Once we have done the logical thing and have made the
             * E-step a sub-routine, this can be simplified. It is also not
             * clear how useful this subset business is, but lets leave it for a
             * bit longer.
             */
once_more:

            /*
            // Initialize. Note that we essentially do the M step first, via the
            // goto below.
            */
            if (    (it == 0)
                 && (    (    ( ! options_ptr->initial_model_is_valid )
                           && ( ! options_ptr->initial_model_is_subset )
                         )
                      || (initialize_remaining_nodes)
                    )
               )
            {
                int     temp_first_level = first_level;
                int     temp_num_sub_clusters = num_real_clusters;
                Vector* sub_P_c_p_vp     = NULL;
                int     sub_ratio        = 1;
                int     sub_cluster;
                double  level_indicator_factor = 1.0;


#ifdef TEST
#ifdef REPORT_CPU_TIME
/* TEST */      cpu_time_offset = get_cpu_time();
#endif
/* TEST */      for (l = 0; l< num_levels; l++)
/* TEST */      {
/* TEST */          level_indicators_pp = level_indicators_ppp[ l];
/* TEST */
/* TEST */          for (c = 0; c< num_clusters; c++)
/* TEST */          {
/* TEST */              level_indicators_p = level_indicators_pp[ c];
/* TEST */
/* TEST */              for (i= 0; i< max_num_items; i++)
/* TEST */              {
/* TEST */                  if (    (*level_indicators_p < 0.0)
/* TEST */                       && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
/* TEST */                        || (*level_indicators_p < DBL_NOT_SET - 0.001)
/* TEST */                          )
/* TEST */                     )
/* TEST */                  {
/* TEST */                      dbe(*level_indicators_p);
/* TEST */                      dbi(l);
/* TEST */                      dbi(c);
/* TEST */                      dbi(i);
/* TEST */
/* TEST */                      SET_CANT_HAPPEN_BUG();
/* TEST */
/* TEST */                      result = ERROR;
/* TEST */                      break;
/* TEST */                  }
/* TEST */                  level_indicators_p++;
/* TEST */              }
/* TEST */          }
/* TEST */      }
/* TEST */
#ifdef REPORT_CPU_TIME
/* TEST */      test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif

                num_levels = num_real_levels;

#ifdef SUPPORT_NUM_CLUSTER_SWITCH     /* Not finished. Should it be? */
                if (    ( ! fs_use_specified_clusters)
                     && (last_level_num == (num_levels - 1))
                     && (num_levels >= 2)
                     && (level_counts_vp->elements[ num_levels - 2 ] == 1)
                   )
                {
                    num_clusters = 1;
                    num_clusters_used = 1;
                    num_limited_clusters = 1;
                    num_sub_clusters = 1;
                    temp_num_sub_clusters = 1;
                }
                else
                {
#endif
                    num_clusters = num_real_clusters;
                    num_clusters_used = num_clusters;
                    num_limited_clusters = num_clusters;
#ifdef SUPPORT_NUM_CLUSTER_SWITCH     /* Not finished. Should it be? */
                }
#endif

                node_mp = topology_ptr->node_mp;
                level_counts_vp = topology_ptr->level_counts_vp;

                if (initialize_remaining_nodes)
                {
                    temp_first_level = num_sub_levels;
                    temp_num_sub_clusters = num_sub_clusters;

                    if (copy_vector_segment(&sub_P_c_p_vp, P_c_p_vp, 0, num_sub_clusters) == ERROR)
                    {
                        result = ERROR;
                        break;
                    }

                    verify_vector(sub_P_c_p_vp, NULL);

                    sub_ratio = num_real_clusters / num_sub_clusters;

                    /* Untested since addition of last level parameter in topology. */
                    /* Dec 10, 2004 */
                    UNTESTED_CODE();

                    level_indicator_factor = (double)(last_level_num - temp_first_level) / (double)temp_first_level;
                }

#ifdef HOW_IT_WAS_NOON_JULY_6_02

#ifdef SIMPLE_CLUSTER_INIT
                TEST_PSO(("Reverting to simple cluster initialization.\n"));
#endif

                result = ow_set_vector(P_c_p_vp,
#ifdef SIMPLE_CLUSTER_INIT
                                       0.1
#else
#ifdef STANDARD_CLUSTER_INIT
                                       0.001
#else
                                       0.0
#endif
#endif
                                       );
#else

                if (fs_initialize_method <= 0)
                {
                    result = ow_set_vector(P_c_p_vp, 0.001);
                }
                else
                {
                    result = ow_zero_vector(P_c_p_vp);

                }
#endif

                verify_vector(P_c_p_vp, NULL);

                for (sub_cluster = 0; sub_cluster < temp_num_sub_clusters; sub_cluster++)
                {
                    for (i = 0; i < sub_ratio; i++)
                    {
                        c = sub_ratio * sub_cluster + i;

                        temp_double = kjb_rand();

#ifdef HOW_IT_WAS_NOON_JULY_6_02

#ifdef SIMPLE_CLUSTER_INIT
                        P_c_p_vp->elements[ c ] += temp_double;
#else
#ifdef STANDARD_CLUSTER_INIT
                        cluster = (int)((double)num_clusters * temp_double);

                        if (cluster == num_clusters) cluster--;

                        P_c_p_vp->elements[ cluster ] += 1.0 / (double)(c + 1);
#else
                        P_c_p_vp->elements[ c ] += (temp_double * temp_double);
#endif
#endif

#else
                        if (fs_initialize_method <= 0)
                        {
                            cluster = (int)((double)num_clusters * temp_double);

                            if (cluster == num_clusters) cluster--;

                            P_c_p_vp->elements[ cluster ] += 1.0 / (double)(c + 1);
                        }
                        else
                        {
                            P_c_p_vp->elements[ c ] += pow(temp_double,
                                                           (double)fs_initialize_method);
                        }
#endif

#ifdef MIN_HORIZONTAL_INDICATOR
                        /*
                         * Since the quantities are less than one, this should
                         * ensure that upon normalization, each level weight is
                         * at least the minimum. There could be problems if
                         * MIN_HORIZONTAL_INDICATOR is non-neglible, as then
                         * this would overwhelm the initialization, making it
                         * near uniform.
                        */
                        P_c_p_vp->elements[ c ] += MIN_HORIZONTAL_INDICATOR * num_clusters;
#endif
                        ASSERT_IS_LESS_INT(c, num_clusters );
                    }

                    verify_vector(P_c_p_vp, NULL);

                    if (options_ptr->initial_model_is_subset)
                    {
                        sum = 0.0;

                        for (i = 0; i < sub_ratio; i++)
                        {
                            c = sub_ratio * sub_cluster + i;
                            ASSERT_IS_LESS_INT(c, num_clusters );
                            sum += P_c_p_vp->elements[ c ];
                        }

                        ASSERT_IS_NUMBER_DBL(sum);
                        ASSERT_IS_FINITE_DBL(sum);
                        ASSERT_IS_GREATER_DBL(sum, 1e5 * DBL_MIN);

                        for (i = 0; i < sub_ratio; i++)
                        {
                            c = sub_ratio * sub_cluster + i;

                            ASSERT_IS_LESS_INT(c, num_clusters );
                            ASSERT_IS_LESS_INT(sub_cluster, sub_P_c_p_vp->length);

                            P_c_p_vp->elements[ c ] /= sum;
                            P_c_p_vp->elements[ c ] *= sub_P_c_p_vp->elements[ sub_cluster ];

                            ASSERT_IS_NUMBER_DBL(P_c_p_vp->elements[ c ]);
                            ASSERT_IS_FINITE_DBL(P_c_p_vp->elements[ c ]);
                        }
                    }

                    verify_vector(P_c_p_vp, NULL);
                }

                verify_vector(P_c_p_vp, NULL);

                free_vector(sub_P_c_p_vp);

                result = ow_scale_vector_by_sum(P_c_p_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                if (initialize_remaining_nodes)
                {
                    /*
                    // Go backwards to avoid over-writing.
                    */
                    for (c = num_sub_clusters - 1; c >= 0; c--)
                    {
                        for (i = sub_ratio - 1; i >= 0; i--)
                        {
                            cluster = c * sub_ratio + i;

                            for (item = 0; item < max_num_items; item++)
                            {
                                for (level = first_level; level < temp_first_level; level++)
                                {
                                    level_indicators_ppp[ level ][ cluster ][ item ] =
                                                      level_indicators_ppp[ level ][ c ][ item ];
                                }
                            }
                        }
                    }
                }

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
#ifdef HOW_IT_WAS_OCT_22_02
                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        level_sum = 0.0;

#ifdef TEST
                        if (options_ptr->initial_model_is_subset)
                        {
                            for (level = first_level; level < temp_first_level; level++)
                            {
                                ASSERT_IS_POSITIVE_DBL(level_indicators_ppp[ level ][ cluster ][ dis_item ]);
                            }
                        }
#endif

                        for (level = temp_first_level; level < last_level_num; level++)
                        {
#ifdef HOW_IT_WAS_NOON_JULY_6_02
                            p = kjb_rand() + 0.1;
#else
                            p = kjb_rand();

                            if (fs_initialize_method <= 0)
                            {
                                p += 0.1;
                            }
                            else
                            {
                                p = pow(p, (double)fs_initialize_method);

                            }
#endif

#ifdef MIN_LEVEL_INDICATOR
                            /*
                             * delta_to_ensure_min_level_indicator is for
                             * quanties that sum to one. The sum here is bounded by
                             * num_levels.
                            */
                            p += delta_to_ensure_min_level_indicator * num_levels;
#endif

                            ASSERT_IS_NUMBER_DBL(p);
                            ASSERT_IS_NUMBER_DBL(level_indicator_factor);
                            level_indicators_ppp[ level ][ cluster ][ dis_item ] = p * level_indicator_factor;
                            ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster ][ dis_item ]);
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            temp_double = level_indicators_ppp[ level ][ cluster ][ dis_item ];

                            if (temp_double > 0.0)
                            {
                                level_sum += temp_double;
                            }
                        }

                        if (level_sum > 0.0)
                        {
                            for (level = first_level; level < last_level_num; level++)
                            {
                                temp_double = level_indicators_ppp[ level ][ cluster ][ dis_item ];

                                if (temp_double > 0.0)
                                {
                                    ASSERT_IS_NUMBER_DBL(level_sum);
                                    level_indicators_ppp[ level ][ cluster ][ dis_item ] /= level_sum;
                                    ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster ][ dis_item ]);
                                }
                            }
                        }
                    }
#endif
                    for (con_item = 0; con_item < num_con_items; con_item++)
                    {
                        level_sum = 0.0;

                        for (level = temp_first_level; level < last_level_num; level++)
                        {
#ifdef HOW_IT_WAS_NOON_JULY_6_02
                            p = kjb_rand() + 0.1;
#else
                            p = kjb_rand();

                            if (fs_initialize_method <= 0)
                            {
                                p += 0.1;
                            }
                            else
                            {
                                p = pow(p, (double)fs_initialize_method);

                            }
#endif
#ifdef MIN_LEVEL_INDICATOR
                            /*
                             * delta_to_ensure_min_level_indicator is for
                             * quanties that sum to one. The sum here is bounded by
                             * num_levels.
                            */
                            p += delta_to_ensure_min_level_indicator * num_levels;
#endif

                            level_indicators_ppp[ level ][ cluster ][ max_num_dis_items + con_item ] = p * level_indicator_factor;
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            temp_double = level_indicators_ppp[ level ][ cluster ][ max_num_dis_items + con_item ];

                            if (temp_double > 0.0)
                            {
                                level_sum += temp_double;
                            }
                        }

                        if (level_sum > 0.0)
                        {
                            for (level = first_level; level < last_level_num; level++)
                            {
                                temp_double = level_indicators_ppp[ level ][ cluster ][ max_num_dis_items + con_item ];

                                if (temp_double > 0.0)
                                {
                                    level_indicators_ppp[ level ][ cluster ][ max_num_dis_items + con_item ] /= level_sum;
                                }
                            }
                        }
                    }

/* "NOT", so this code is generally active. */
#ifndef HOW_IT_WAS_OCT_22_02
                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        level_sum = 0.0;

                        for (level = temp_first_level; level < last_level_num; level++)
                        {
                            if (    (num_con_items > 0)
                                 && (options_ptr->model_correspondence > 0)
                               )
                            {
                                p = 0.0;

                                for (con_item = 0; con_item < num_con_items; con_item++)
                                {
                                    p += level_indicators_ppp[ level ][ cluster ][ max_num_dis_items + con_item ];
                                }

                                p *= (1.0 + 0.01 * kjb_rand());
                            }
                            else
                            {
#ifdef HOW_IT_WAS_NOON_JULY_6_02
                                p = kjb_rand() + 0.1;
#else
                                p = kjb_rand();

                                if (fs_initialize_method <= 0)
                                {
                                    p += 0.1;
                                }
                                else
                                {
                                    p = pow(p, (double)fs_initialize_method);

                                }
                            }
#endif

#ifdef MIN_LEVEL_INDICATOR
                            /*
                             * delta_to_ensure_min_level_indicator is for
                             * quanties that sum to one. The sum here is bounded by
                             * num_levels.
                            */
                            p += delta_to_ensure_min_level_indicator * num_levels;
#endif

                            ASSERT_IS_NUMBER_DBL(p);
                            ASSERT_IS_NUMBER_DBL(level_indicator_factor);
                            level_indicators_ppp[ level ][ cluster ][ dis_item ] = p * level_indicator_factor;
                            ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster ][ dis_item ]);
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            temp_double = level_indicators_ppp[ level ][ cluster ][ dis_item ];

                            if (temp_double > 0.0)
                            {
                                level_sum += temp_double;
                            }
                        }

                        if (level_sum > 0.0)
                        {
                            for (level = first_level; level < last_level_num; level++)
                            {
                                temp_double = level_indicators_ppp[ level ][ cluster ][ dis_item ];

                                if (temp_double > 0.0)
                                {
                                    ASSERT_IS_NUMBER_DBL(level_sum);
                                    level_indicators_ppp[ level ][ cluster ][ dis_item ] /= level_sum;
                                    ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster ][ dis_item ]);
                                }
                            }
                        }
                    }
#endif

                }

                verify_vector(P_c_p_vp, NULL);

/*
                This broke correlations?

#define BASIC_WAY_AS_THIS_IS_GETTING_COMPLICATED
*/


#ifdef BASIC_WAY_AS_THIS_IS_GETTING_COMPLICATED
                result = get_zero_vector(&P_c_p_vp, num_clusters);
                if (result == ERROR) { NOTE_ERROR(); break; }

                for (c=0; c < num_clusters; c++)
                {
                    /* Underlying small bit of uniform distribution for
                     * stability.
                     */
                    P_c_p_vp->elements[ c ] = (0.0001 + kjb_rand()) / ((double)num_clusters);

                    if (kjb_rand() < (1.0 / sqrt((double)num_clusters)))
                    {
                         P_c_p_vp->elements[ c ] = 1.0;
                    }
                }

                result = ow_scale_vector_by_sum(P_c_p_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
#endif

#ifdef TEST
#ifdef REPORT_CPU_TIME
/* TEST */      cpu_time_offset = get_cpu_time();
#endif
/* TEST */      for (l = 0; l< num_levels; l++)
/* TEST */      {
/* TEST */          level_indicators_pp = level_indicators_ppp[ l];
/* TEST */
/* TEST */          for (c = 0; c< num_clusters; c++)
/* TEST */          {
/* TEST */              level_indicators_p = level_indicators_pp[ c];
/* TEST */
/* TEST */              for (i= 0; i< max_num_items; i++)
/* TEST */              {
/* TEST */                  if (    (*level_indicators_p < 0.0)
/* TEST */                       && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
/* TEST */                        || (*level_indicators_p < DBL_NOT_SET - 0.001)
/* TEST */                          )
/* TEST */                     )
/* TEST */                  {
/* TEST */                      dbe(*level_indicators_p);
/* TEST */                      dbi(l);
/* TEST */                      dbi(c);
/* TEST */                      dbi(i);
/* TEST */
/* TEST */                      SET_CANT_HAPPEN_BUG();
/* TEST */
/* TEST */                      result = ERROR;
/* TEST */                      break;
/* TEST */                  }
/* TEST */                  level_indicators_p++;
/* TEST */              }
/* TEST */          }
/* TEST */      }
/* TEST */
#ifdef REPORT_CPU_TIME
/* TEST */      test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif
                /*
                 * Too dangerous to integrate this with the above mess. Just
                 * overwrite P_c_p_vp with what it has to be.
                */
                if (    (fs_use_specified_clusters)
                     && (data_ptr->group_vp != NULL)
                   )
                {
                    result = ow_zero_vector(P_c_p_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    ASSERT_IS_NON_NEGATIVE_INT(data_ptr->group_vp->elements[ point ]);

                    if (data_ptr->group_vp->elements[ point ] >= num_clusters)
                    {
                        set_bug("The group index (%d) for point %d is not less than the number of clusters (%d)",
                                data_ptr->group_vp->elements[ point ],
                                point,
                                num_clusters);
                        return ERROR;
                    }
                    P_c_p_vp->elements[ data_ptr->group_vp->elements[ point ] ] = 1.0;
                }

                /*
                 * We want to start with an M-Step, so we jump to it, skiping
                 * the first E-Step iteration.
                */

                goto v_step;
            }
            else if ((it == 0) && (options_ptr->initial_model_is_subset))
            {
                /*
                // Temporarily reduce the number of levels and clusters so that
                // the cluster membership and level_indicators can be computed
                // based on the model parameters.
                */

                dbw();

                num_levels = num_sub_levels;
                num_clusters = num_sub_clusters;
                num_clusters_used = num_sub_clusters;
                num_limited_clusters = num_sub_clusters;
                node_mp = model_ptr->topology_ptr->node_mp;
                level_counts_vp = model_ptr->topology_ptr->level_counts_vp;
            }
            else if ((it > 0) || (options_ptr->initial_model_is_valid))
            {
                current_model_is_valid = TRUE;
            }

            /*
            //                          E Step
            */

            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif

#ifdef TEST
#ifdef REPORT_CPU_TIME
/* TEST */  cpu_time_offset = get_cpu_time();
#endif
/* TEST */  for (level = 0; level < num_levels; level++)
/* TEST */  {
/* TEST */      level_indicators_pp = level_indicators_ppp[ level ];
/* TEST */
/* TEST */      for (cluster = 0; cluster < num_clusters; cluster++)
/* TEST */      {
/* TEST */          level_indicators_p = level_indicators_pp[ cluster ];
/* TEST */
/* TEST */          for (item = 0; item < max_num_items; item++)
/* TEST */          {
/* TEST */              if (    (*level_indicators_p < 0.0)
/* TEST */                   && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
/* TEST */                        || (*level_indicators_p < DBL_NOT_SET - 0.001)
/* TEST */                      )
/* TEST */                 )
/* TEST */              {
/* TEST */                  dbe(*level_indicators_p);
/* TEST */                  dbi(level);
/* TEST */                  dbi(cluster);
/* TEST */                  dbi(item);
/* TEST */
/* TEST */                  SET_CANT_HAPPEN_BUG();
/* TEST */
/* TEST */                  result = ERROR;
/* TEST */                  break;
/* TEST */              }
/* TEST */              level_indicators_p++;
/* TEST */          }
/* TEST */      }
/* TEST */  }
/* TEST */
/* TEST */  /* Breaks shared correspondence */
/* TEST */  /* result = ow_set_vector(P_c_p_vp, DBL_NOT_SET); */
#ifdef REPORT_CPU_TIME
/* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif
#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif

            /*
            // Cache the probability calculations so that they are done only
            // once per node as opposed to num_levels*num_cluster times.
            */
#ifdef REPORT_CPU_TIME
            cpu_time_offset = get_cpu_time();
#endif
            for (con_item = 0; con_item < num_con_items; con_item++)
            {
                int missing_data = missing_index_vvp->elements[ point ]->elements[ con_item ];

                save_x_ptr = con_item_mvp->elements[ point ]->elements[ con_item ];
                con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];
#ifdef SUPPORT_ANNEALING
                annealed_con_prob_cache_vvp = annealed_con_prob_cache_vvvp->elements[ con_item ];
#endif

                for (level = first_level; level < last_level_num; level++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    num_nodes_this_level = level_counts_vp->elements[ level ];
                    con_prob_cache_ptr = con_prob_cache_vvp->elements[ level ]->elements;
#ifdef SUPPORT_ANNEALING
                    annealed_con_prob_cache_ptr = annealed_con_prob_cache_vvp->elements[ level ]->elements;
#endif

                    for (node = 0; node < num_nodes_this_level; node++)
                    {
                        ASSERT_IS_LESS_INT(level, last_level_num);
                        ASSERT_IS_NOT_LESS_INT(level, first_level);
                        ASSERT_IS_NON_NEGATIVE_INT(first_level);
                        ASSERT_IS_NON_NEGATIVE_INT(node);
                        ASSERT_IS_LESS_INT(node, mean_mvp->elements[ level ]->num_rows);
                        ASSERT_IS_LESS_INT(node, var_mvp->elements[ level ]->num_rows);
                        ASSERT_IS_LESS_INT(node, con_log_sqrt_det_vvp->elements[ level ]->length);

                        if (missing_data)
                        {
                            result = compute_con_prob(con_feature_enable_vp,
                                                      con_feature_group_vp,
                                                      save_x_ptr,
                                                      mean_mvp->elements[ level ]->elements[ node ],
                                                      var_mvp->elements[ level ]->elements[ node ],
                                                      con_log_sqrt_det_vvp->elements[ level ]->elements[ node ],
                                                      fs_norm_continuous_features,
                                                      fs_limit_gaussian_deviation,
                                                      fs_max_con_norm_var,
                                                      fs_vector_feature_counts_ptr,
                                                      con_prob_cache_ptr,
                                                      &max_con_log_prob, TRUE);
                        }
                        else
                        {
                            result = compute_con_prob(con_feature_enable_vp,
                                                      con_feature_group_vp,
                                                      save_x_ptr,
                                                      mean_mvp->elements[ level ]->elements[ node ],
                                                      var_mvp->elements[ level ]->elements[ node ],
                                                      con_log_sqrt_det_vvp->elements[ level ]->elements[ node ],
                                                      fs_norm_continuous_features,
                                                      fs_limit_gaussian_deviation,
                                                      fs_max_con_norm_var,
                                                      fs_vector_feature_counts_ptr,
                                                      con_prob_cache_ptr,
                                                      &max_con_log_prob, FALSE);
                        }

                        if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef SUPPORT_ANNEALING
                        UNTESTED_CODE();
                        /*
                        // Possible BUG!
                        //
                        // We may never get back to annealing in preference to
                        // other ways to fit the model. So I have not dealt with
                        // the slight problem that the weighting by region area
                        // likely should occur AFTER annealing---of course,
                        // annealing being a hack, this is not for sure!
                        */

/* ANNEAL */            *annealed_log_prob_ptr = (*con_prob_cache_ptr);
/* ANNEAL */
/* ANNEAL */            if (it < fs_num_cooling_iterations)
/* ANNEAL */            {
/* ANNEAL */                (*annealed_log_prob_ptr) *= temperature;
/* ANNEAL */            }
#endif

                        con_prob_cache_ptr++;
#ifdef SUPPORT_ANNEALING
/* ANNEAL */            annealed_con_prob_cache_ptr++;
#endif

                    }
                }
            }
#ifdef REPORT_CPU_TIME
            con_prob_cache_cpu += (get_cpu_time() - cpu_time_offset);
            cpu_time_offset = get_cpu_time();
#endif

#ifdef XXX_SUPPORT_TRIMMING
            if (options_ptr->trim_fraction > 0.0)
            {
                result = ow_zero_int_vector(dis_score_counts_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_zero_int_vector(con_score_counts_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
#endif

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif

            if (    (fs_use_specified_clusters)
                 && (data_ptr->group_vp != NULL)
               )
            {
                result = ow_set_vector(P_c_p_vp, DBL_HALF_MOST_NEGATIVE);
                if (result == ERROR) { NOTE_ERROR(); break; }
                result = ow_set_vector(ll_P_c_p_vp, DBL_HALF_MOST_NEGATIVE);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }

            /*
            // It would be easy to make some cases overlap, *except* if we
            // want to keep the ANNEAL stuff. So for the moment, I will just
            // copy the code and hack the correspondence case.
            */
            if (options_ptr->model_correspondence < 2)
            {
                /*
                // Compute level breakdowns and
                //
                // p(c|d) = p(d|c)p(c) / NORM
                */

                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {
#ifdef XXX_SUPPORT_TRIMMING
                    int num_good_con_items = 0;
                    int num_good_dis_items = 0;
#endif
                    double log_a = DBL_MOST_NEGATIVE;

                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                    if (    (fs_use_specified_clusters)
                         && (data_ptr->group_vp != NULL)
                       )
                    {
                        if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    }
#ifdef TEST
/* TEST */          if (valid_cluster_index)
/* TEST */          {
/* TEST */              ASSERT_IS_EQUAL_INT(cluster, cluster_index_mp->elements[ point ][ cluster_index ]);
/* TEST */          }
/* TEST */          ASSERT_IS_NON_NEGATIVE_INT(cluster_index);
/* TEST */          ASSERT_IS_LESS_INT(cluster_index, num_clusters);
#endif

                    dis_item_found = FALSE;

                    p = a_vp->elements[ cluster ];

                    if (p < 5.0 * DBL_MIN)
                    {
                        TEST_PSO(("Cluster (%d) with no meat spotted in E step.\n",
                                  cluster + 1));
                        dbe(p);
                        log_a = LOG_ZERO;
                    }
                    else
                    {
                        log_a = log(p);
                    }

#ifdef TEST
                    sum_nu = 0.0;
#endif
                    log_p = log_a;
#ifdef SUPPORT_ANNEALING
                    annealed_log_p = log_p;
#endif

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /*
                    // Cache the level weights, taking logs if we are using logs for
                    // probs.
                    */
#ifdef REPORT_CPU_TIME
                    cpu_time_offset = get_cpu_time();
#endif

                    for (level = first_level; level < last_level_num; level++)
                    {
                        if (valid_cluster_dependent_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_c_mvp->length);
                            ASSERT_IS_LESS_INT(point , P_l_p_c_mvp->elements[ level ]->num_rows);
                            ASSERT_IS_LESS_INT(cluster_index , P_l_p_c_mvp->elements[ level ]->num_cols);

                            nu = P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else if (valid_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_mp->num_rows);
                            ASSERT_IS_LESS_INT(point , P_l_p_mp->num_cols);

                            nu = P_l_p_mp->elements[ level ][ point ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else
                        {
                            nu = V_mp->elements[ level ][ cluster ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }

#ifdef TEST
                        sum_nu += nu;
#endif

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);

                        nu_vp->elements[ level ] = nu;

                        if (nu < 5.0 * DBL_MIN)
                        {
                            log_nu_vp->elements[ level ] = LOG_ZERO;
                        }
                        else
                        {
                            log_nu_vp->elements[ level ] = log(nu);
                        }
                    }

#ifdef TEST
                    ASSERT_IS_NUMBER_DBL(sum_nu);
                    ASSERT_IS_FINITE_DBL(sum_nu);
                    ASSERT_IS_LESS_DBL(sum_nu , 1.0001);
                    ASSERT_IS_GREATER_DBL(sum_nu, 0.9999);
#endif

#ifdef REPORT_CPU_TIME
                    vertical_weight_caching_cpu += (get_cpu_time() - cpu_time_offset);
                    cpu_time_offset = get_cpu_time();
#endif

                    /*
                    // Now do con_items first to support word emmission
                    // conditioned on blobs (March 1, 2002). Note that this
                    // means that we now fill in the second half of some arrays
                    // first.
                    */

                    log_product = 0.0;
#ifdef SUPPORT_ANNEALING
    /* ANNEAL */    annealed_log_product = 0.0;
#endif
                    if (options_ptr->model_correspondence == 1)
                    {
                        if (fs_pair_depend)
                        {
                                               /* FIX */
                            UNTESTED_CODE();   /* Will be broken for cases other than word depends on blob. */

                            result = get_zero_vector(&nu_based_on_pairs_vp,
                                                     num_levels);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            for (con_item = 0; con_item < num_con_items; con_item++)
                            {
                                prob_sum = 0.0;
                                max_log_prob = DBL_MOST_NEGATIVE;
                                con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];

                                if (result == ERROR) { NOTE_ERROR(); break; }

                                for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                                {
                                    category = dis_item_mp->elements[ point ][ dis_item ];
                                    dis_item_multiplicity = dis_item_multiplicity_mp->elements[ point ][ dis_item ];

                                    ASSERT_IS_NON_NEGATIVE_INT(category);

                                    for (level = first_level; level < last_level_num; level++)
                                    {
                                        log_nu       = log_nu_vp->elements[ level ];
                                        node         = node_mp->elements[ cluster ][ level ];
                                        log_prob     = con_prob_cache_vvp->elements[ level ]->elements[ node ];
                                        log_prob += (log_P_i_n_mvp->elements[ level ]->elements[ node ][ category ] * dis_item_multiplicity);

                                        log_nu_times_prob = log_prob + log_nu;

                                        ASSERT_IS_NUMBER_DBL(log_nu_times_prob);
                                        ASSERT_IS_FINITE_DBL(log_nu_times_prob);

                                        log_prob_vp->elements[ level ] = log_nu_times_prob;

                                        max_log_prob = MAX_OF(max_log_prob, log_nu_times_prob);
                                    }


                                    ASSERT_IS_NUMBER_DBL(max_log_prob);
                                    ASSERT_IS_FINITE_DBL(max_log_prob);
                                    ASSERT_IS_NOT_LESS_DBL(max_log_prob, DBL_HALF_MOST_NEGATIVE);

                                    for (level = first_level; level < last_level_num; level++)
                                    {
                                        log_prob = log_prob_vp->elements[ level ] - max_log_prob;
                                        prob = exp(log_prob);

                                        ASSERT_IS_NUMBER_DBL(log_prob_vp->elements[ level ]);
                                        ASSERT_IS_FINITE_DBL(log_prob_vp->elements[ level ]);
                                        ASSERT_IS_NUMBER_DBL(log_prob);
                                        ASSERT_IS_FINITE_DBL(log_prob);
                                        ASSERT_IS_NUMBER_DBL(prob);
                                        ASSERT_IS_FINITE_DBL(prob);

                                        prob_vp->elements[ level ] = prob;

                                        prob_sum += prob;

                                        ASSERT_IS_NUMBER_DBL(prob_sum);
                                        ASSERT_IS_FINITE_DBL(prob_sum);
                                    }

                                    for (level = first_level; level < last_level_num; level++)
                                    {
                                        if (model_ptr->norm_depend)
                                        {
                                            if (fs_max_depend)
                                            {
                                                nu_based_on_pairs_vp->elements[ level ] =
                                                    MAX_OF(nu_based_on_pairs_vp->elements[ level ],
                                                           prob_vp->elements[ level ] / prob_sum);
                                            }
                                            else
                                            {
                                                nu_based_on_pairs_vp->elements[ level ] += (prob_vp->elements[ level ] / prob_sum);
                                            }
                                        }
                                        else
                                        {
                                            if (fs_max_depend)
                                            {
                                                nu_based_on_pairs_vp->elements[ level ] =
                                                    MAX_OF(nu_based_on_pairs_vp->elements[ level ],
                                                           prob_vp->elements[ level ]);
                                            }
                                            else
                                            {
                                                nu_based_on_pairs_vp->elements[ level ] += (prob_vp->elements[ level ]);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            if ((fs_use_blob_prior) && (blob_word_prior_mvp != NULL))
                            {
                                result = get_zero_matrix(&nu_with_prior_mp,
                                                         num_levels, num_con_items);
                            }
                            else
                            {
                                result = get_zero_vector(&nu_for_dependent_model_vp,
                                                         num_levels);
                            }

                            if (result == ERROR) { NOTE_ERROR(); break; }

                            for (con_item = 0; con_item < num_con_items; con_item++)
                            {
                                double con_item_weight = 1.0;

                                if ((model_ptr->weight_con_items) && (con_item_weight_vvp != NULL))
                                {
                                    static int first_time = TRUE;

                                    if (first_time)
                                    {
                                        first_time = FALSE;
                                        dbp("Weighting continous features.\n");
                                    }

                                    con_item_weight = con_item_weight_vvp->elements[point]->elements[con_item];
                                }

                                prob_sum = 0.0;
                                max_log_prob = DBL_MOST_NEGATIVE;
                                con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];

                                if (result == ERROR) { NOTE_ERROR(); break; }

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    log_nu       = log_nu_vp->elements[ level ];
                                    node         = node_mp->elements[ cluster ][ level ];
                                    log_prob     = con_prob_cache_vvp->elements[ level ]->elements[ node ];

                                    log_nu_times_prob = log_prob + log_nu;

                                    ASSERT_IS_NUMBER_DBL(log_nu_times_prob);
                                    ASSERT_IS_FINITE_DBL(log_nu_times_prob);

                                    log_prob_vp->elements[ level ] = log_nu_times_prob;

                                    max_log_prob = MAX_OF(max_log_prob, log_nu_times_prob);
                                }


                                ASSERT_IS_NUMBER_DBL(max_log_prob);
                                ASSERT_IS_FINITE_DBL(max_log_prob);
                                ASSERT_IS_NOT_LESS_DBL(max_log_prob, DBL_HALF_MOST_NEGATIVE);

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    log_prob = log_prob_vp->elements[ level ] - max_log_prob;
                                    prob = exp(log_prob);

                                    ASSERT_IS_NUMBER_DBL(log_prob_vp->elements[ level ]);
                                    ASSERT_IS_FINITE_DBL(log_prob_vp->elements[ level ]);
                                    ASSERT_IS_NUMBER_DBL(log_prob);
                                    ASSERT_IS_FINITE_DBL(log_prob);
                                    ASSERT_IS_NUMBER_DBL(prob);
                                    ASSERT_IS_FINITE_DBL(prob);

                                    prob_vp->elements[ level ] = prob;

                                    prob_sum += prob;

                                    ASSERT_IS_NUMBER_DBL(prob_sum);
                                    ASSERT_IS_FINITE_DBL(prob_sum);
                                }

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    if ((fs_use_blob_prior) && (blob_word_prior_mvp != NULL))
                                    {
                                        if (model_ptr->norm_depend) /* Usually true, might not make a difference. */
                                        {
                                            if (fs_max_depend) /* Usually false. */
                                            {
                                                nu_with_prior_mp->elements[ level ][ con_item ] =
                                                    MAX_OF(nu_with_prior_mp->elements[ level ][ con_item ],
                                                           con_item_weight * prob_vp->elements[ level ] / prob_sum);
                                            }
                                            else
                                            {
                                                nu_with_prior_mp->elements[ level ][ con_item ] =
                                                    con_item_weight * (prob_vp->elements[ level ] / prob_sum);
                                            }
                                        }
                                        else
                                        {
                                            if (fs_max_depend) /* Usually false. */
                                            {
                                                nu_with_prior_mp->elements[ level ][ con_item ] =
                                                    MAX_OF(nu_with_prior_mp->elements[ level ][ con_item ],
                                                           con_item_weight * prob_vp->elements[ level ]);
                                            }
                                            else
                                            {
                                                nu_with_prior_mp->elements[ level ][ con_item ] =
                                                    con_item_weight * (prob_vp->elements[ level ]);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (model_ptr->norm_depend) /* Usually true, might not make a difference. */
                                        {
                                            if (fs_max_depend) /* Usually false. */
                                            {
                                                nu_for_dependent_model_vp->elements[ level ] =
                                                    MAX_OF(nu_for_dependent_model_vp->elements[ level ],
                                                           con_item_weight * prob_vp->elements[ level ] / prob_sum);
                                            }
                                            else
                                            {
                                                nu_for_dependent_model_vp->elements[ level ] +=
                                                    con_item_weight * (prob_vp->elements[ level ] / prob_sum);
                                            }
                                        }
                                        else
                                        {
                                            if (fs_max_depend) /* Usually false. */
                                            {
                                                nu_for_dependent_model_vp->elements[ level ] =
                                                    MAX_OF(nu_for_dependent_model_vp->elements[ level ],
                                                           con_item_weight * prob_vp->elements[ level ]);
                                            }
                                            else
                                            {
                                                nu_for_dependent_model_vp->elements[ level ] +=
                                                    con_item_weight * (prob_vp->elements[ level ]);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (fs_double_depend)   /* Usually false, perhaps a dead option. */
                        {
                                               /* FIX */
                            UNTESTED_CODE();   /* Will be broken for cases other than word depends on blob. */

                            result = get_zero_vector(&nu_based_on_dis_items_vp,
                                                     num_levels);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                            {
                                prob_sum = 0.0;

                                category = dis_item_mp->elements[ point ][ dis_item ];

                                ASSERT_IS_NON_NEGATIVE_INT(category);

                                dis_item_found = TRUE;

                                dis_item_multiplicity = dis_item_multiplicity_mp->elements[ point ][ dis_item ];

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    node  = node_mp->elements[ cluster ][ level ];
                                    prob = P_i_n_mvp->elements[ level ]->elements[ node ][ category ];

                                    nu = nu_vp->elements[ level ];

                                    ASSERT_IS_NUMBER_DBL(prob);
                                    ASSERT_IS_FINITE_DBL(prob);
                                    ASSERT_IS_NON_NEGATIVE_DBL(prob);

                                    ASSERT_IS_NUMBER_DBL(nu);
                                    ASSERT_IS_FINITE_DBL(nu);
                                    ASSERT_IS_NON_NEGATIVE_DBL(nu);

                                    nu_times_prob = nu * prob;

                                    ASSERT_IS_NUMBER_DBL(nu_times_prob);
                                    ASSERT_IS_FINITE_DBL(nu_times_prob);
                                    ASSERT_IS_NON_NEGATIVE_DBL(nu_times_prob);

                                    prob_sum += nu_times_prob;
                                    prob_vp->elements[ level ] = nu_times_prob;

                                    ASSERT_IS_NUMBER_DBL(prob_sum);
                                    ASSERT_IS_FINITE_DBL(prob_sum);
                                    ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);
                                }

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    if (model_ptr->norm_depend)
                                    {
                                        nu_based_on_dis_items_vp->elements[ level ] += (prob_vp->elements[ level ] / prob_sum);
                                    }
                                    else
                                    {
                                        nu_based_on_dis_items_vp->elements[ level ] += (prob_vp->elements[ level ]);
                                    }
                                }

                            }

                            if (fs_multiply_vertical_depend_weights)
                            {
                                double temp_sum = 0;

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    nu_based_on_dis_items_vp->elements[ level ] *= nu_for_dependent_model_vp->elements[ level ];
                                    temp_sum += nu_based_on_dis_items_vp->elements[ level ];
                                }

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    nu_based_on_dis_items_vp->elements[ level ] /= temp_sum;
                                    nu_for_dependent_model_vp->elements[ level ] = nu_based_on_dis_items_vp->elements[ level ];
                                }
                            }
                        }
                    }

                    for (con_item = 0; con_item < num_con_items; con_item++)
                    {
                        double con_item_weight = 1.0;

                        if ((model_ptr->weight_con_items) && (con_item_weight_vvp != NULL))
                        {
                            static int first_time = TRUE;

                            if (first_time)
                            {
                                first_time = FALSE;
                                dbp("Weighting continous features by area.\n");
                            }

                            con_item_weight = con_item_weight_vvp->elements[point]->elements[con_item];
                        }

                        prob_sum = 0.0;
                        max_log_prob = DBL_MOST_NEGATIVE;
                        con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];
#ifdef SUPPORT_ANNEALING
    /* ANNEAL */        annealed_prob_sum = 0.0;
    /* ANNEAL */        annealed_max_log_prob = DBL_MOST_NEGATIVE;
    /* ANNEAL */        annealed_con_prob_cache_vvp = annealed_con_prob_cache_vvvp->elements[ con_item ];
#endif

                        if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef TEST
#ifdef SUPPORT_ANNEALING
    /* TEST */          result = ow_set_vector(annealed_log_prob_vp, DBL_NOT_SET);
    /* TEST */          if (result == ERROR) { NOTE_ERROR(); break; }
    /* TEST */
    /* TEST */          result = ow_set_vector(annealed_prob_vp, DBL_NOT_SET);
    /* TEST */          if (result == ERROR) { NOTE_ERROR(); break; }
#endif
    /* TEST */
    /* TEST */          result = ow_set_vector(log_prob_vp, DBL_NOT_SET);
    /* TEST */          if (result == ERROR) { NOTE_ERROR(); break; }
    /* TEST */
    /* TEST */          result = ow_set_vector(prob_vp, DBL_NOT_SET);
    /* TEST */          if (result == ERROR) { NOTE_ERROR(); break; }
    /* TEST */
#endif
                        for (level = first_level; level < last_level_num; level++)
                        {
                            if (    (options_ptr->model_correspondence == 1)
                                 && (fs_pair_depend)
                               )
                            {
                                log_nu = log(nu_based_on_pairs_vp->elements[ level ]);
                            }
                            else if (    (options_ptr->model_correspondence == 1)
                                      && (fs_double_depend)
                                    )
                            {
                                log_nu = log(nu_based_on_dis_items_vp->elements[ level ]);
                            }
                            else
                            {
                                log_nu = log_nu_vp->elements[ level ];
                            }

                            node         = node_mp->elements[ cluster ][ level ];
                            log_prob     = con_prob_cache_vvp->elements[ level ]->elements[ node ];
                            log_nu_times_prob = log_prob + log_nu;
#ifdef SUPPORT_ANNEALING
                            annealed_log_prob = annealed_con_prob_cache_vvp->elements[ level ]->elements[ node ];
                            annealed_log_nu_times_prob = annealed_log_prob + log_nu;
#endif

                            ASSERT_IS_NUMBER_DBL(log_nu_times_prob);
                            ASSERT_IS_FINITE_DBL(log_nu_times_prob);

                            log_prob_vp->elements[ level ] = log_nu_times_prob;

                            max_log_prob = MAX_OF(max_log_prob, log_nu_times_prob);

#ifdef XXX_SUPPORT_TRIMMING
                            /*
                            // Scores for trim determination.
                            //
                            // FIX -- this is SLOW -- could easily be made
                            // faster.
                            */
                            if (options_ptr->trim_fraction > 0.0)
                            {
                                for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                                {
                                    category = dis_item_mp->elements[ point ][ dis_item ];

                                    i = con_score_counts_vp->elements[ con_item ];

                                    con_score_mp->elements[ con_item ][ i ] =
                                            log_nu_times_prob
                                          + log(P_i_n_mvp->elements[ level ]->elements[ node ][ category ])
                                          - log((double)num_dis_items);

                                    if (fs_trim_hack == 1)
                                    {
                                        (con_score_mp->elements[ con_item ][ i ]) += log_a;
                                        dbw();   /* Suspect! */
                                    }
                                    else if (fs_trim_hack == 2)
                                    {
                                        (con_score_mp->elements[ con_item ][ i ]) += log(P_c_p_mp->elements[ point ][ cluster ]);
                                    }
                                    else
                                    {
                                         dbw();   /* Suspect! */
                                    }

                                    (con_score_counts_vp->elements[ con_item ])++;
                                }
                            }
#endif


#ifdef SUPPORT_ANNEALING
    /* ANNEAL */            ASSERT_IS_NUMBER_DBL(annealed_log_nu_times_prob);
    /* ANNEAL */            ASSERT_IS_FINITE_DBL(annealed_log_nu_times_prob);
    /* ANNEAL */
    /* ANNEAL */            annealed_log_prob_vp->elements[ level ] = annealed_log_nu_times_prob;
    /* ANNEAL */
    /* ANNEAL */            annealed_max_log_prob = MAX_OF(annealed_max_log_prob,
    /* ANNEAL */                                           annealed_log_nu_times_prob);
#endif
                        }


                        ASSERT_IS_NUMBER_DBL(max_log_prob);
                        ASSERT_IS_FINITE_DBL(max_log_prob);
                        ASSERT_IS_NOT_LESS_DBL(max_log_prob, DBL_HALF_MOST_NEGATIVE);

                        for (level = first_level; level < last_level_num; level++)
                        {
                            log_prob = log_prob_vp->elements[ level ] - max_log_prob;
                            prob = exp(log_prob);
#ifdef SUPPORT_ANNEALING
                            annealed_log_prob = annealed_log_prob_vp->elements[ level ] - annealed_max_log_prob;
                            annealed_prob = exp(annealed_log_prob);

                            /*
                            // Theoretically possible, but highly unlikely.
                            */
                            ASSERT_IS_NOT_EQUAL_DBL(annealed_log_prob, DBL_NOT_SET);

                            annealed_prob_sum += annealed_prob;
                            annealed_prob_vp->elements[ level ] = annealed_prob;
#else
                            ASSERT_IS_NUMBER_DBL(log_prob_vp->elements[ level ]);
                            ASSERT_IS_FINITE_DBL(log_prob_vp->elements[ level ]);
                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);
                            ASSERT_IS_NUMBER_DBL(prob);
                            ASSERT_IS_FINITE_DBL(prob);

                            prob_vp->elements[ level ] = prob;
#endif

                            prob_sum += prob;

                            ASSERT_IS_NUMBER_DBL(prob_sum);
                            ASSERT_IS_FINITE_DBL(prob_sum);
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];

#ifdef SUPPORT_ANNEALING
                            level_indicators_pp[ cluster_index ][ max_num_dis_items + con_item ] = annealed_prob_vp->elements[ level ] / annealed_prob_sum;
#else
                            ASSERT_IS_NUMBER_DBL(prob_vp->elements[ level ]);
                            ASSERT_IS_FINITE_DBL(prob_vp->elements[ level ]);
                            ASSERT_IS_NON_NEGATIVE_DBL(prob_vp->elements[ level ]);

                            ASSERT_IS_NUMBER_DBL(prob_sum);
                            ASSERT_IS_FINITE_DBL(prob_sum);
                            ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);
                            ASSERT_IS_NOT_LESS_DBL(prob_sum, 10.0 * DBL_MIN);

                            level_indicators_pp[ cluster_index ][ max_num_dis_items + con_item ] = prob_vp->elements[ level ] / prob_sum;
#endif
                            ASSERT_IS_NUMBER_DBL(level_indicators_pp[ cluster_index ][ max_num_dis_items + con_item ]);
                            ASSERT_IS_FINITE_DBL(level_indicators_pp[ cluster_index ][ max_num_dis_items + con_item ]);
                            ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_pp[ cluster_index ][ max_num_dis_items + con_item ]);
                            ASSERT_IS_PROB_DBL(level_indicators_pp[ cluster_index ][ max_num_dis_items + con_item ]);
                        }

#ifdef MIN_LEVEL_INDICATOR
                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];
                            level_indicators_p = level_indicators_pp[ cluster_index ] + max_num_dis_items + con_item;

                            ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                            ASSERT_IS_FINITE_DBL(*level_indicators_p);
                            ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                            ASSERT_IS_PROB_DBL(*level_indicators_p);

                            *level_indicators_p += delta_to_ensure_min_level_indicator;
                            *level_indicators_p /= min_level_indicator_norm;

                            ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                            ASSERT_IS_FINITE_DBL(*level_indicators_p);
                            ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                            ASSERT_IS_PROB_DBL(*level_indicators_p);
                        }
#endif

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */        annealed_log_prob_sum = log(annealed_prob_sum) + annealed_max_log_prob;
    /* ANNEAL */
    /* ANNEAL */        ASSERT_IS_NUMBER_DBL(annealed_log_prob_sum);
    /* ANNEAL */        ASSERT_IS_FINITE_DBL(annealed_log_prob_sum);
#endif

                        ASSERT_IS_NUMBER_DBL(prob_sum);
                        ASSERT_IS_FINITE_DBL(prob_sum);

                        if (prob_sum < 5.0 * DBL_MIN)
                        {
                            warn_pso("Zero prob spotted. Making amends.\n");
                            dbe(prob_sum);
                            dbe(max_log_prob);

                            log_prob_sum = LOG_ZERO + max_log_prob;
                        }
                        else
                        {
                            log_prob_sum = log(prob_sum) + max_log_prob;
                        }

                        ASSERT_IS_NUMBER_DBL(log_prob_sum);
                        ASSERT_IS_FINITE_DBL(log_prob_sum);

                        ASSERT_IS_NUMBER_DBL(log_prob_sum);
                        ASSERT_IS_FINITE_DBL(log_prob_sum);

#ifdef XXX_SUPPORT_TRIMMING
                        if (fs_trim_mach_two)
                        {
                            if (    ( ! apply_score_cutoff)
                                 || (con_score_vp->elements[ con_item_count + con_item ] > con_score_cutoff)
                               )
                            {
                                log_product += log_prob_sum;
                                num_good_con_items++;
                                temp_good_con_item_count++;
                            }
                            temp_total_con_item_count++;
                        }
                        else
                        {
#endif
                            log_product += log_prob_sum * con_item_weight;
#ifdef XXX_SUPPORT_TRIMMING
                        }
#endif

                        ASSERT_IS_NUMBER_DBL(log_product);
                        ASSERT_IS_FINITE_DBL(log_product);

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */        annealed_log_product += annealed_log_prob_sum;
    /* ANNEAL */
    /* ANNEAL */        ASSERT_IS_NUMBER_DBL(annealed_log_product);
    /* ANNEAL */        ASSERT_IS_FINITE_DBL(annealed_log_product);
#endif
                    }

                    if (options_ptr->model_correspondence == 1)
                    {
                        /*
                         * This normalization might only affect the computation
                         * of the likelihood. If model_ptr->norm_depend then
                         * this vector is already  normalized on a region by
                         * region basis. The normalization that follows should
                         * not be confused with that user controlled
                         * normalization.
                        */
                        if (nu_for_dependent_model_vp != NULL)
                        {
                            result = ow_scale_vector_by_sum(nu_for_dependent_model_vp);
                            if (result == ERROR) { NOTE_ERROR(); break; }
                        }
#ifdef TEST
                        else
                        {
                            ASSERT(nu_with_prior_mp != NULL);
                        }
#endif
                    }

#ifdef XXX_SUPPORT_TRIMMING
                    if (fs_trim_mach_two)
                    {
                        /*
                        // FIX
                        //
                        // Must update for con_log_norm?
                        */
                        if (num_good_con_items > 0)
                        {
                            log_p += ((double)max_num_con_items /(double)num_good_con_items) * log_product;

#ifdef SUPPORT_ANNEALING
                        BARF  /* Not finished */
#endif
                        }
                    }
                    else
                    {
#endif
                        if (num_con_items > 0)
                        {
                            ASSERT_IS_NUMBER_DBL(con_factor);
                            ASSERT_IS_FINITE_DBL(con_factor);
                            ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

                            /*
                            // The multiplication by con_factor can probably be moved
                            // outside the loop over clusters.
                            */
                            log_p += con_factor * log_product;
#ifdef SUPPORT_ANNEALING
        /* ANNEAL */        annealed_log_p += con_factor * annealed_log_product;
#endif

                        }
#ifdef XXX_SUPPORT_TRIMMING
                    }
#endif

                    ASSERT_IS_NUMBER_DBL(log_p);
                    ASSERT_IS_FINITE_DBL(log_p);

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */    ASSERT_IS_NUMBER_DBL(annealed_log_p);
    /* ANNEAL */    ASSERT_IS_FINITE_DBL(annealed_log_p);
#endif

#ifdef REPORT_CPU_TIME
                    con_item_prob_update_cpu += get_cpu_time() - cpu_time_offset;
                    cpu_time_offset = get_cpu_time();
#endif

                    /* Compute level break downs for discrete items. */

                    log_product = 0.0;
#ifdef SUPPORT_ANNEALING
                    annealed_log_product = 0.0;
#endif
                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
#ifdef SUPPORT_ANNEALING
                        annealed_prob_sum = 0.0;
#endif
                        prob_sum = 0.0;

                        category = dis_item_mp->elements[ point ][ dis_item ];

                        ASSERT_IS_NON_NEGATIVE_INT(category);

                        dis_item_found = TRUE;

#ifdef TEST
#ifdef SUPPORT_ANNEALING
                        result = ow_set_vector(annealed_prob_vp, DBL_NOT_SET);
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        result = ow_set_vector(annealed_log_prob_vp, DBL_NOT_SET);
                        if (result == ERROR) { NOTE_ERROR(); break; }
#endif
                        result = ow_set_vector(log_prob_vp, DBL_NOT_SET);
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        result = ow_set_vector(prob_vp, DBL_NOT_SET);
                        if (result == ERROR) { NOTE_ERROR(); break; }
#endif
                        dis_item_multiplicity = dis_item_multiplicity_mp->elements[ point ][ dis_item ];

                        if ((fs_use_blob_prior) && (blob_word_prior_mvp != NULL))
                        {
                            result = get_matrix_col(&word_prior_vp,
                                                    blob_word_prior_mvp->elements[ point ],
                                                    category);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = multiply_matrix_and_vector(&nu_with_prior_vp,
                                                                nu_with_prior_mp,
                                                                word_prior_vp);
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = ow_scale_vector_by_sum(nu_with_prior_vp);
                            if (result == ERROR) { NOTE_ERROR(); break; }
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            if (result == ERROR) { NOTE_ERROR(); break; }

                            node  = node_mp->elements[ cluster ][ level ];

                            if (dis_item_loss_fn != NULL)
                            {
                                prob = dis_item_loss_fn(category,
                                                        P_i_n_mvp->elements[ level ]->elements[ node ]);
                            }
                            else
                            {
                                prob = P_i_n_mvp->elements[ level ]->elements[ node ][ category ];
                            }

                            if (nu_with_prior_vp != NULL)
                            {
                                nu = nu_with_prior_vp->elements[ level ];
                            }
                            else if (    (options_ptr->model_correspondence == 1)
                                      && (fs_pair_depend)
                                    )
                            {
                                nu = nu_based_on_pairs_vp->elements[ level ];
                            }
                            else if (options_ptr->model_correspondence == 1)
                            {
                                nu = nu_for_dependent_model_vp->elements[ level ];
                            }
                            else
                            {
                                nu = nu_vp->elements[ level ];
                            }

                            ASSERT_IS_NUMBER_DBL(prob);
                            ASSERT_IS_FINITE_DBL(prob);
                            ASSERT_IS_NON_NEGATIVE_DBL(prob);

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);

                            nu_times_prob = nu * prob;

                            ASSERT_IS_NUMBER_DBL(nu_times_prob);
                            ASSERT_IS_FINITE_DBL(nu_times_prob);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu_times_prob);

#ifdef XXX_SUPPORT_TRIMMING
                            /*
                            // Scores for null determination.
                            */
                            if (options_ptr->trim_fraction > 0.0)
                            {
                                for (con_item = 0; con_item < num_con_items; con_item++)
                                {
                                    i = dis_score_counts_vp->elements[ dis_item ];

                                    dis_score_mp->elements[ dis_item ][ i ] =
                                            log(nu_times_prob)
                                          + con_prob_cache_vvvp->elements[ con_item ]->elements[ level ]->elements[ node ]
                                          - log((double)num_con_items);

                                    if (fs_trim_hack == 1)
                                    {
                                        (dis_score_mp->elements[ dis_item ][ i ]) += log_a;
                                    }
                                    else if (fs_trim_hack == 2)
                                    {
                                        (dis_score_mp->elements[ dis_item ][ i ]) += log(P_c_p_mp->elements[ point ][ cluster ]);
                                    }

                                    (dis_score_counts_vp->elements[ dis_item ])++;
                                }
                            }
#endif

                            level_indicators_pp = level_indicators_ppp[ level ];

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */            if (it < fs_num_cooling_iterations)
    /* ANNEAL */            {
    /* ANNEAL */                annealed_prob = pow(prob, temperature);
    /* ANNEAL */                annealed_nu_times_prob = nu * annealed_prob;
    /* ANNEAL */            }
    /* ANNEAL */            else
    /* ANNEAL */            {
    /* ANNEAL */                annealed_prob = prob;
    /* ANNEAL */                annealed_nu_times_prob = nu_times_prob;
    /* ANNEAL */            }
    /* ANNEAL */
    /* ANNEAL */            ASSERT_IS_NUMBER_DBL(annealed_nu_times_prob);
    /* ANNEAL */            ASSERT_IS_FINITE_DBL(annealed_nu_times_prob);
    /* ANNEAL */            ASSERT_IS_NON_NEGATIVE_DBL(annealed_nu_times_prob);
    /* ANNEAL */
    /* ANNEAL */            level_indicators_pp[ cluster_index ][ dis_item ] = annealed_nu_times_prob;
    /* ANNEAL */
    /* ANNEAL */            annealed_prob_sum += annealed_nu_times_prob;
    /* ANNEAL */
    /* ANNEAL */            ASSERT_IS_NUMBER_DBL(annealed_prob_sum);
    /* ANNEAL */            ASSERT_IS_FINITE_DBL(annealed_prob_sum);
    /* ANNEAL */            ASSERT_IS_NON_NEGATIVE_DBL(annealed_prob_sum);
#else
                            level_indicators_pp[ cluster_index ][ dis_item ] = nu_times_prob;
#endif

                            prob_sum += nu_times_prob;

                            ASSERT_IS_NUMBER_DBL(prob_sum);
                            ASSERT_IS_FINITE_DBL(prob_sum);
                            ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);
                        }

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */        if (ABS_OF(annealed_prob_sum) < 3.0 * DBL_MIN)
    /* ANNEAL */        {
    /* ANNEAL */            dbe(annealed_prob_sum);
    /* ANNEAL */            dbi(annealed_prob_sum == 0.0);
    /* ANNEAL */            annealed_log_prob_sum = LOG_ZERO;
    /* ANNEAL */        }
    /* ANNEAL */        else
    /* ANNEAL */        {
    /* ANNEAL */            annealed_log_prob_sum = log(annealed_prob_sum);
    /* ANNEAL */        }
    /* ANNEAL */
    /* ANNEAL */        if (annealed_prob_sum < 1.0e-50)
    /* ANNEAL */        {
    /* ANNEAL */            dbe(annealed_prob_sum);
    /* ANNEAL */        }
#endif
                        if (prob_sum < 1.0e-50)
                        {
                            dbe(prob_sum);
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];

                            ASSERT_IS_NUMBER_DBL(level_indicators_pp[ cluster_index ][ dis_item ]);
                            ASSERT_IS_FINITE_DBL(level_indicators_pp[ cluster_index ][ dis_item ]);
                            ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_pp[ cluster_index ][ dis_item ]);

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */            ASSERT_IS_NUMBER_DBL(annealed_prob_sum);
    /* ANNEAL */            ASSERT_IS_FINITE_DBL(annealed_prob_sum);
    /* ANNEAL */            ASSERT_IS_NON_NEGATIVE_DBL(annealed_prob_sum);
    /* ANNEAL */
    /* ANNEAL */            level_indicators_pp[ cluster_index ][ dis_item ] /= annealed_prob_sum;
#else
                            ASSERT_IS_NUMBER_DBL(prob_sum);
                            ASSERT_IS_FINITE_DBL(prob_sum);
                            ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);
                            ASSERT_IS_NOT_LESS_DBL(prob_sum, 10.0 * DBL_MIN);

                            level_indicators_pp[ cluster_index ][ dis_item ] /= prob_sum;

                            ASSERT_IS_NUMBER_DBL(level_indicators_pp[ cluster_index ][ dis_item ]);
                            ASSERT_IS_FINITE_DBL(level_indicators_pp[ cluster_index ][ dis_item ]);
                            ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_pp[ cluster_index ][ dis_item ]);
                            ASSERT_IS_PROB_DBL(level_indicators_pp[ cluster_index ][ dis_item ]);
#endif
                        }


#ifdef MIN_LEVEL_INDICATOR
                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];

                            level_indicators_pp[ cluster_index ][ dis_item ] +=
                                                          delta_to_ensure_min_level_indicator;
                            level_indicators_pp[ cluster_index ][ dis_item ] /=
                                                          min_level_indicator_norm;
                        }
#endif


#ifdef SUPPORT_IGNORE_DIS_PROB
                        total_dis_prob += prob_sum;
                        dis_prob_count++;

                        p_use_dis_item_mp->elements[ cluster_index ][ dis_item ] =
                                                       prob_sum / (prob_sum + extra_dis_prob);
                        prob_sum += extra_dis_prob;
#endif

                        if (prob_sum < 5.0 * DBL_MIN)
                        {
                            db_rv(nu_vp);
                            dbi(num_dis_items);
                            dbe(prob_sum);
                            dbi(prob_sum == 0.0);
                            log_prob_sum = LOG_ZERO;
                        }
                        else
                        {
                            log_prob_sum = log(prob_sum);
                        }

#ifdef XXX_SUPPORT_TRIMMING
                        if (fs_trim_mach_two)
                        {
                            if (    ( ! apply_score_cutoff)
                                 || (dis_score_vp->elements[ dis_item_count + dis_item ] > dis_score_cutoff)
                               )
                            {
                                log_product += (log_prob_sum * dis_item_multiplicity);
                                num_good_dis_items++;
                                temp_good_dis_item_count++;
                            }

                            temp_total_dis_item_count++;
                        }
                        else
                        {
#endif
                            log_product += (log_prob_sum * dis_item_multiplicity);
#ifdef XXX_SUPPORT_TRIMMING
                        }
#endif

                        ASSERT_IS_NUMBER_DBL(log_product);
                        ASSERT_IS_FINITE_DBL(log_product);

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */        annealed_log_product += (annealed_log_prob_sum * dis_item_multiplicity);
    /* ANNEAL */        ASSERT_IS_NUMBER_DBL(annealed_log_product);
    /* ANNEAL */        ASSERT_IS_FINITE_DBL(annealed_log_product);
#endif
                    }


#ifdef XXX_SUPPORT_TRIMMING
                    if (fs_trim_mach_two)
                    {
                        if (num_good_dis_items > 0)
                        {
                            log_p += ((double)max_num_dis_items /(double)num_good_dis_items) * log_product;

#ifdef SUPPORT_ANNEALING
                        BARF  /* Not finished */
#endif
                        }
                    }
                    else
                    {
#endif
                        /*
                         * A big of a HACK
                         *
                         * Needs reworking for generality.
                         *
                         * We skip the effect of the dis item computation if we
                         * want to cluster only on con items at this point.
                         * While it would be more natural to simply exclude the
                         * computation of the dis item expectation, doing it
                         * that way has lots of reprocussions in the code below
                         * (with respect to the q factors for the multi-level
                         * case).
                         */
                        if (    (options_ptr->model_correspondence == 0)
                             && (max_num_con_items > 0)
                             && (it < fs_min_it_for_dis_items)
                           )
                        {
                            /*EMPTY*/
                            ; /* Do nothing. */
                        }
                        else if (dis_item_found)
                        {
                            ASSERT_IS_NUMBER_DBL(dis_factor);
                            ASSERT_IS_FINITE_DBL(dis_factor);
                            ASSERT_IS_NON_NEGATIVE_DBL(dis_factor);
                            ASSERT_IS_GREATER_DBL(dis_factor, 1e-10);

                            log_p += log_product * dis_factor;
#ifdef SUPPORT_ANNEALING
        /* ANNEAL */        annealed_log_p += annealed_log_product * dis_factor;
#endif
                        }
#ifdef XXX_SUPPORT_TRIMMING
                    }
#endif

                    ASSERT_IS_NUMBER_DBL(log_p);
                    ASSERT_IS_FINITE_DBL(log_p);

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */    ASSERT_IS_NUMBER_DBL(annealed_log_p);
    /* ANNEAL */    ASSERT_IS_FINITE_DBL(annealed_log_p);
#endif


#ifdef REPORT_CPU_TIME
                    dis_item_prob_update_cpu += get_cpu_time() - cpu_time_offset;
#endif

                    ASSERT_IS_NUMBER_DBL(log_p);
                    ASSERT_IS_FINITE_DBL(log_p);

#ifdef XXX_SUPPORT_TRIMMING
                    if (fs_trim_mach_two)
                    {
                        if (num_good_dis_items + num_good_con_items > 0)
                        {
                            ll_P_c_p_vp->elements[ cluster_index ] = log_p;
                        }
                        else
                        {
                            ll_P_c_p_vp->elements[ cluster_index ] = DBL_MOST_NEGATIVE;
                        }
                    }
                    else
                    {
#endif
                        ll_P_c_p_vp->elements[ cluster_index ] = log_p;
#ifdef XXX_SUPPORT_TRIMMING
                    }
#endif

#ifdef XXX_SUPPORT_TRIMMING
                    if (fs_trim_mach_two)
                    {
#ifdef SUPPORT_ANNEALING
                        BARF  /* not done */
#else
                        if (num_good_dis_items + num_good_con_items > 0)
                        {
                            P_c_p_vp->elements[ cluster_index ] = log_p;
                        }
                        else
                        {
                            P_c_p_vp->elements[ cluster_index ] = DBL_MOST_NEGATIVE;
                        }
#endif
                    }
                    else
                    {
#endif
#ifdef SUPPORT_ANNEALING
                        ASSERT_IS_NUMBER_DBL(annealed_log_p);
                        ASSERT_IS_FINITE_DBL(annealed_log_p);

                        P_c_p_vp->elements[ cluster_index ] = annealed_log_p;
#else
                        P_c_p_vp->elements[ cluster_index ] = log_p;

                        ASSERT_IS_NUMBER_DBL(P_c_p_vp->elements[ cluster_index ]);
                        ASSERT_IS_FINITE_DBL(P_c_p_vp->elements[ cluster_index ]);
#endif
#ifdef XXX_SUPPORT_TRIMMING
                    }
#endif
                }

#ifdef TEST
#ifdef REPORT_CPU_TIME
    /* CPU */   cpu_time_offset = get_cpu_time();
#endif
    /* TEST */
    /* TEST */  for (level = first_level; level < last_level_num; level++)
    /* TEST */  {
    /* TEST */      level_indicators_pp = level_indicators_ppp[ level ];
    /* TEST */
    /* TEST */      for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
    /* TEST */      {
    /* TEST */          level_indicators_p = level_indicators_pp[ cluster_index ];
    /* TEST */
    /* TEST */          for (item = 0; item < max_num_items; item++)
    /* TEST */          {
    /* TEST */              ASSERT_IS_FINITE_DBL(level_indicators_p[ item ]);
    /* TEST */              ASSERT_IS_NUMBER_DBL(level_indicators_p[ item ]);
    /* TEST */
                            /* This verifies that all elements got touched.
                            // However, some could have been set to negative. We
                            // want to keep them negative to verfify that those
                            // elements are never used.
                            */
    /* TEST */              if (level_indicators_p[ item ] < 0.0)
    /* TEST */              {
    /* TEST */                  ASSERT_IS_EQUAL_DBL(level_indicators_p[ item ],
    /* TEST */                                      DBL_NOT_SET);
    /* TEST */
    /* TEST */              }
    /* TEST */              else
    /* TEST */              {
    /* TEST */                  ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_p[ item ]);
    /* TEST */                  ASSERT_IS_NOT_GREATER_DBL(level_indicators_p[ item ], 1.0);
    /* TEST */              }
    /* TEST */          }
    /* TEST */      }
    /* TEST */
    /* TEST */      for (cluster_index = num_clusters_used; cluster_index < num_clusters; cluster_index++)
    /* TEST */      {
    /* TEST */          level_indicators_p = level_indicators_pp[ cluster_index ];
    /* TEST */
    /* TEST */          for (item = 0; item < max_num_items; item++)
    /* TEST */          {
    /* TEST */              ASSERT_IS_FINITE_DBL(level_indicators_p[ item ]);
    /* TEST */              ASSERT_IS_NUMBER_DBL(level_indicators_p[ item ]);
    /* TEST */              ASSERT_IS_EQUAL_DBL(level_indicators_p[ item ],
    /* TEST */                                  DBL_NOT_SET);
    /* TEST */          }
    /* TEST */      }
    /* TEST */  }
#ifdef REPORT_CPU_TIME
                test_extra_cpu += (get_cpu_time() - cpu_time_offset);
                cpu_time_offset = get_cpu_time();
#endif
#endif
            }
            else if (    (options_ptr->model_correspondence == 2)
                      && (options_ptr->share_matches)
                    )
            {
                double log_a = DBL_MOST_NEGATIVE;

#ifdef REPORT_CPU_TIME
                cpu_time_offset = get_cpu_time();
#endif

                result = ow_set_vector(mf_dis_item_max_log_vp,
                                       DBL_MOST_NEGATIVE);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_set_vector(mf_con_item_max_log_vp,
                                       DBL_MOST_NEGATIVE);
                if (result == ERROR) { NOTE_ERROR(); break; }

                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                    if (    (fs_use_specified_clusters)
                         && (data_ptr->group_vp != NULL)
                       )
                    {
                        if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    }

                    log_pair_probs_1_pp = log_pair_probs_1_ppp[ cluster ];
                    log_pair_probs_2_pp = log_pair_probs_2_ppp[ cluster ];
#ifdef TEST
                    sum_nu = 0.0;
#endif

#ifdef TEST
                    if (valid_cluster_index)
                    {
                        ASSERT_IS_EQUAL_INT(cluster, cluster_index_mp->elements[ point ][ cluster_index ]);
                    }
#endif
                    ASSERT_IS_NON_NEGATIVE_INT(cluster_index);
                    ASSERT_IS_LESS_INT(cluster_index , num_clusters);

                     /* dbi(cluster);  */

                    /*
                    // Cache the level weights, taking logs if we are using logs for
                    // probs.
                    */

                    for (level = first_level; level < last_level_num; level++)
                    {
                        if (valid_cluster_dependent_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_c_mvp->length);
                            ASSERT_IS_LESS_INT(point , P_l_p_c_mvp->elements[ level ]->num_rows);
                            ASSERT_IS_LESS_INT(cluster_index , P_l_p_c_mvp->elements[ level ]->num_cols);

                            nu = P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else if (valid_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_mp->num_rows);
                            ASSERT_IS_LESS_INT(point , P_l_p_mp->num_cols);

                            nu = P_l_p_mp->elements[ level ][ point ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else
                        {
                            nu = V_mp->elements[ level ][ cluster ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
#ifdef TEST
                        sum_nu += nu;
#endif

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);

                        nu_vp->elements[ level ] = nu;

                        if (nu < 5.0 * DBL_MIN)
                        {
                            log_nu_vp->elements[ level ] = LOG_ZERO;
                        }
                        else
                        {
                            log_nu_vp->elements[ level ] = log(nu);
                        }
                    }

#ifdef TEST
                    ASSERT_IS_NUMBER_DBL(sum_nu);
                    ASSERT_IS_FINITE_DBL(sum_nu);
                    ASSERT_IS_LESS_DBL(sum_nu , 1.0001);
                    ASSERT_IS_GREATER_DBL(sum_nu, 0.9999);
#endif

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /* Set up pair prob by level table */

                    for (level = first_level; level < last_level_num; level++)
                    {
                        double* log_dis_prob_ptr;

                        if (result == ERROR) { NOTE_ERROR(); break; }

                        log_pair_probs_1_p = log_pair_probs_1_pp[ level ];
                        log_pair_probs_2_p = log_pair_probs_2_pp[ level ];

                        node   = node_mp->elements[ cluster ][ level ];
                        log_nu = log_nu_vp->elements[ level ];

                        log_dis_prob_ptr = log_P_i_n_mvp->elements[ level ]->elements[ node ];

                         /* dbi(level);  */

                        for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                        {
                            category = dis_item_mp->elements[ point ][ dis_item ];

                            ASSERT_IS_NON_NEGATIVE_INT(category);

#ifdef IT_WAS_MORE_LIKE_THIS_BEFORE_REWRITE
                            /*
                            // CHECK this HACK
                            //
                            // Note that we are currently ignoring
                            // multiplicity, which gets tricky when doing
                            // strict matching.
                            */
                            dis_item_multiplicity = dis_item_multiplicity_mp->elements[ point ][ dis_item ];

                            ASSERT_IS_NUMBER_DBL(dis_factor);
                            ASSERT_IS_NON_NEGATIVE_DBL(dis_factor);
                            ASSERT_IS_NUMBER_DBL(dis_item_multiplicity);
                            ASSERT_IS_NON_NEGATIVE_DBL(dis_item_multiplicity);
#endif

                            ASSERT_IS_NUMBER_DBL(log_dis_prob_ptr[ category ]);

#ifdef IT_WAS_MORE_LIKE_THIS_BEFORE_REWRITE
                            log_prob = log_nu + dis_factor * dis_item_multiplicity * log_dis_prob_ptr[ category ];
#endif
                            log_prob = log_nu + log_dis_prob_ptr[ category ];

                            ASSERT_IS_NUMBER_DBL(log_prob);

                            log_pair_probs_1_p[ dis_item ] = log_prob;

                            max_log_ptr = mf_dis_item_max_log_vp->elements + dis_item;
                            *max_log_ptr = MAX_OF(log_prob, *max_log_ptr);

                            ASSERT_IS_NUMBER_DBL(mf_dis_item_max_log_vp->elements[ dis_item ]);
                            ASSERT_IS_FINITE_DBL(mf_dis_item_max_log_vp->elements[ dis_item ]);
                            ASSERT_IS_NOT_LESS_DBL(mf_dis_item_max_log_vp->elements[ dis_item ], log_prob);
                        }

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];
                            log_prob     = con_prob_cache_vvp->elements[ level ]->elements[ node ];
                            /*
                            // CHECK this HACK
                            //
                            // Note that we are currently ignoring
                            // multiplicity, which gets tricky when doing
                            // strict matching.
                            //
                            // Note that any changes here need to be reflected
                            // in the computation of ll_con_correction_norm_sum.
                            */
#ifdef IT_WAS_MORE_LIKE_THIS_BEFORE_REWRITE
                            log_prob *= con_factor;
#endif

                            ASSERT_IS_NUMBER_DBL(log_prob);

                            log_pair_probs_2_p[ con_item ] = log_prob;
                            max_log_ptr = mf_con_item_max_log_vp->elements + con_item;
                            *max_log_ptr = MAX_OF(log_prob, *max_log_ptr);

                            ASSERT_IS_NOT_LESS_DBL(mf_con_item_max_log_vp->elements[ con_item ], log_prob);
                            ASSERT_IS_NUMBER_DBL(mf_con_item_max_log_vp->elements[ con_item ]);
                            ASSERT_IS_FINITE_DBL(mf_con_item_max_log_vp->elements[ con_item ]);
                        }
                    }
                }

#ifdef TEST
                if (result == ERROR) { NOTE_ERROR(); break; }

                for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                {
                    ASSERT_IS_NUMBER_DBL(mf_dis_item_max_log_vp->elements[ dis_item ]);
                    ASSERT_IS_GREATER_DBL(mf_dis_item_max_log_vp->elements[ dis_item ], DBL_HALF_MOST_NEGATIVE);
                }
                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    ASSERT_IS_NUMBER_DBL(mf_con_item_max_log_vp->elements[ con_item ]);
                    ASSERT_IS_GREATER_DBL(mf_con_item_max_log_vp->elements[ con_item ], DBL_HALF_MOST_NEGATIVE);
                }
#endif

                if (result == ERROR) { NOTE_ERROR(); break; }

                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                    pair_probs_1_pp = pair_probs_1_ppp[ cluster ];
                    pair_probs_2_pp = pair_probs_2_ppp[ cluster ];
                    log_pair_probs_1_pp = log_pair_probs_1_ppp[ cluster ];
                    log_pair_probs_2_pp = log_pair_probs_2_ppp[ cluster ];

#ifdef TEST
                    if (valid_cluster_index)
                    {
                        ASSERT_IS_EQUAL_INT(cluster, cluster_index_mp->elements[ point ][ cluster_index ]);
                    }
#endif
                    ASSERT_IS_NON_NEGATIVE_INT(cluster_index);
                    ASSERT_IS_LESS_INT(cluster_index , num_clusters);

                     /* dbi(cluster);  */

                    for (level = first_level; level < last_level_num; level++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        pair_probs_1_p = pair_probs_1_pp[ level ];
                        pair_probs_2_p = pair_probs_2_pp[ level ];
                        log_pair_probs_1_p = log_pair_probs_1_pp[ level ];
                        log_pair_probs_2_p = log_pair_probs_2_pp[ level ];

                        for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                        {
                            pair_probs_1_p[ dis_item ] = exp(log_pair_probs_1_p[ dis_item ] - mf_dis_item_max_log_vp->elements[ dis_item ]);
                        }

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            pair_probs_2_p[ con_item ] = exp(log_pair_probs_2_p[ con_item ] - mf_con_item_max_log_vp->elements[ con_item ]);
                        }
                    }
                }

                if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
                raw_pair_prob_table_cpu += (get_cpu_time() - cpu_time_offset);
                cpu_time_offset = get_cpu_time();
#endif

                /* Compute pair probs. */

                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        prob_sum = 0.0;

                        for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                        {
                            cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                            pair_probs_1_pp = pair_probs_1_ppp[ cluster ];
                            pair_probs_2_pp = pair_probs_2_ppp[ cluster ];

                            level_sum = 0.0;

                            for (level = first_level; level < last_level_num; level++)
                            {
                                double p1 = pair_probs_1_pp[ level ][ dis_item ];
                                double p2 = pair_probs_2_pp[ level ][ con_item ];

                                level_sum += p1 * p2;

                                ASSERT_IS_NUMBER_DBL(level_sum);
                                ASSERT_IS_NON_NEGATIVE_DBL(level_sum);
                            }

                            if (fs_sample_hack == 0)
                            {
                                prob_sum += level_sum;
                            }
                            else if (fs_sample_hack == 1)
                            {
                                prob_sum += level_sum * a_vp->elements[ cluster ];
                            }
                            else if (fs_sample_hack == 2)
                            {
                                prob_sum += level_sum * P_c_p_mp->elements[ point ][ cluster ];
                            }
                            else
                            {
                                SET_CANT_HAPPEN_BUG();
                                result = ERROR;
                                break;
                            }

                            ASSERT_IS_NUMBER_DBL(prob_sum);
                            ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);
                        }

                        ASSERT_IS_NUMBER_DBL(mf_con_item_max_log_vp->elements[ con_item ]);
                        ASSERT_IS_FINITE_DBL(mf_con_item_max_log_vp->elements[ con_item ]);
                        ASSERT_IS_NUMBER_DBL(mf_dis_item_max_log_vp->elements[ dis_item ]);
                        ASSERT_IS_FINITE_DBL(mf_dis_item_max_log_vp->elements[ dis_item ]);

                        log_pair_probs_mp->elements[ con_item ][ dis_item ] =
                            log(prob_sum) + mf_con_item_max_log_vp->elements[ con_item ] +
                                                       mf_dis_item_max_log_vp->elements[ dis_item ];

                        ASSERT_IS_NUMBER_DBL(log_pair_probs_mp->elements[ con_item ][ dis_item ]);
                        ASSERT_IS_FINITE_DBL(log_pair_probs_mp->elements[ con_item ][ dis_item ]);
                    }
                }

#ifdef REPORT_CPU_TIME
                pair_prob_table_cpu += (get_cpu_time() - cpu_time_offset);
                cpu_time_offset = get_cpu_time();
#endif
                log_pair_probs_mp->num_rows = num_con_items;
                log_pair_probs_mp->num_cols = num_dis_items;

                verify_matrix(log_pair_probs_mp, NULL);

#ifdef XXX_SUPPORT_TRIMMING
#ifndef XXX_OLD_PAIR_NULL_METHOD
                if (options_ptr->trim_fraction > 0.0)
                {
                    num_nulls = ((double)num_con_items * options_ptr->trim_fraction) + 0.5;

                    if (num_nulls >= num_con_items)
                    {
                        num_nulls = num_con_items - 1;
                    }
                }
#endif
#endif
                result = get_matches(&match_mp, &num_matches,
                                     log_pair_probs_mp,
                                        (options_ptr->sample_matches)
                                     && (    (options_ptr->last_sample < 0)
                                          || (it <= options_ptr->last_sample)
                                        ),
                                     num_nulls,
                                     fs_duplicate_words_for_matching);

                log_pair_probs_mp->num_rows = max_num_con_items;
                log_pair_probs_mp->num_cols = max_num_dis_items;

                if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
                pair_match_cpu += (get_cpu_time() - cpu_time_offset);
                cpu_time_offset = get_cpu_time();
#endif

#ifdef XXX_SUPPORT_TRIMMING
#ifdef XXX_OLD_PAIR_NULL_METHOD
                /*
                // Scores for null determination.
                */
                if (options_ptr->trim_fraction > 0.0)
                {
                    for (match = 0; match < num_matches; match++)
                    {
                        double log_pair_prob;

                        con_item = match_mp->elements[ match ][ 0 ];
                        dis_item = match_mp->elements[ match ][ 1 ];

                        log_pair_prob = log_pair_probs_mp->elements[ con_item ][ dis_item ];

                        i = dis_score_counts_vp->elements[ dis_item ];
                        dis_score_mp->elements[ dis_item ][ i ] = log_pair_prob;
                        (dis_score_counts_vp->elements[ dis_item ])++;

                        i = con_score_counts_vp->elements[ con_item ];
                        con_score_mp->elements[ con_item ][ i ] = log_pair_prob;
                        (con_score_counts_vp->elements[ con_item ])++;
                    }
                }
#endif
#endif
                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {
#ifdef XXX_SUPPORT_TRIMMING
                    int num_good_pairs = 0;
#endif

                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                    p = a_vp->elements[ cluster ];

                    if (p < 5.0 * DBL_MIN)
                    {
                        TEST_PSO(("Cluster (%d) with no meat spotted in E step.\n",
                                  cluster + 1));
                        dbe(p);
                        log_a = LOG_ZERO;
                    }
                    else
                    {
                        log_a = log(p);
                    }

                    log_pair_probs_1_pp = log_pair_probs_1_ppp[ cluster ];
                    log_pair_probs_2_pp = log_pair_probs_2_ppp[ cluster ];

                    /* Update--Dry Run */

                    result = ow_zero_int_vector(dis_item_match_count_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_zero_int_vector(con_item_match_count_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_set_vector(dis_item_max_log_vp,
                                           DBL_MOST_NEGATIVE);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_set_vector(con_item_max_log_vp,
                                           DBL_MOST_NEGATIVE);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (match = 0; match < num_matches; match++)
                    {
                        con_item = match_mp->elements[ match ][ 0 ];
                        dis_item = match_mp->elements[ match ][ 1 ];

                        ASSERT_IS_NON_NEGATIVE_INT(con_item);
                        ASSERT_IS_LESS_INT(con_item , num_con_items);
                        ASSERT_IS_NON_NEGATIVE_INT(dis_item);
                        ASSERT_IS_LESS_INT(dis_item , num_dis_items);

                        (dis_item_match_count_vp->elements[ dis_item ])++;
                        (con_item_match_count_vp->elements[ con_item ])++;

                        for (level = first_level; level < last_level_num; level++)
                        {
                            log_pair_probs_1_p = log_pair_probs_1_pp[ level ];
                            log_pair_probs_2_p = log_pair_probs_2_pp[ level ];

                            log_prob = log_pair_probs_1_p[ dis_item ] + log_pair_probs_2_p[ con_item ];

                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

                            con_item_max_log_vp->elements[ con_item ] =
                                MAX_OF(log_prob,
                                       con_item_max_log_vp->elements[ con_item ]);

                            ASSERT_IS_GREATER_DBL(con_item_max_log_vp->elements[ con_item ], DBL_MOST_NEGATIVE / 100.0);

                            dis_item_max_log_vp->elements[ dis_item ] =
                                MAX_OF(log_prob,
                                       dis_item_max_log_vp->elements[ dis_item ]);

                            ASSERT_IS_GREATER_DBL(dis_item_max_log_vp->elements[ dis_item ], DBL_MOST_NEGATIVE / 100.0);
                        }
                    }

                    for (level = first_level; level < last_level_num; level++)
                    {
                        level_indicators_p = level_indicators_ppp[ level ][ cluster_index ];

                        for (item = 0; item < max_num_items; item++)
                        {
                            *level_indicators_p = 0.0;
                            level_indicators_p++;
                        }
                    }

                    log_p = 0.0;

                    for (match = 0; match < num_matches; match++)
                    {
                        con_item = match_mp->elements[ match ][ 0 ];
                        dis_item = match_mp->elements[ match ][ 1 ];

                        ASSERT_IS_NON_NEGATIVE_INT(con_item);
                        ASSERT_IS_LESS_INT(con_item , num_con_items);
                        ASSERT_IS_NON_NEGATIVE_INT(dis_item);
                        ASSERT_IS_LESS_INT(dis_item , num_dis_items);

                        prob_sum = 0.0;

                        dis_max_log_prob = dis_item_max_log_vp->elements[ dis_item ];
                        con_max_log_prob = con_item_max_log_vp->elements[ con_item ];

                        ASSERT_IS_NUMBER_DBL(dis_max_log_prob);
                        ASSERT_IS_NUMBER_DBL(con_max_log_prob);

                        ASSERT_IS_GREATER_DBL(dis_max_log_prob, DBL_MOST_NEGATIVE / 100.0);
                        ASSERT_IS_GREATER_DBL(con_max_log_prob, DBL_MOST_NEGATIVE / 100.0);

                        for (level = first_level; level < last_level_num; level++)
                        {
                            log_pair_probs_1_p = log_pair_probs_1_pp[ level ];
                            log_pair_probs_2_p = log_pair_probs_2_pp[ level ];

                            log_prob = log_pair_probs_1_p[ dis_item ] + log_pair_probs_2_p[ con_item ];


                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);


                            level_indicators_p = level_indicators_ppp[ level ][ cluster_index ];

                            /*
                            // Because of pair emmision, dis_prob and
                            // con_prob are essentially the same. They only
                            // have different normalizations.
                            */

                            dis_prob = exp(log_prob - dis_max_log_prob);
                            level_indicators_p[ dis_item ] += dis_prob;

                            ASSERT_IS_NUMBER_DBL(level_indicators_p[ dis_item ]);
                            ASSERT_IS_FINITE_DBL(level_indicators_p[ dis_item ]);

                            con_prob = exp(log_prob - con_max_log_prob);
                            level_indicators_p[ max_num_dis_items + con_item ] += con_prob;

                            ASSERT_IS_NUMBER_DBL(level_indicators_p[ max_num_dis_items + con_item ]);
                            ASSERT_IS_FINITE_DBL(level_indicators_p[ max_num_dis_items + con_item ]);

                            /*
                            // Note: con_prob has both con and dis prob; it
                            // is only normalized for con probs.
                            */
                            prob_sum += con_prob;

                            ASSERT_IS_NUMBER_DBL(prob_sum);
                            ASSERT_IS_FINITE_DBL(prob_sum);
                            ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);
                        }

                        ASSERT_IS_NUMBER_DBL(log_p);
                        ASSERT_IS_FINITE_DBL(log_p);

                        ASSERT_IS_NUMBER_DBL(prob_sum);
                        ASSERT_IS_FINITE_DBL(prob_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);

                        ASSERT_IS_NUMBER_DBL(con_max_log_prob);
                        ASSERT_IS_FINITE_DBL(con_max_log_prob);
                        ASSERT_IS_GREATER_DBL(con_max_log_prob, DBL_MOST_NEGATIVE / 100.0);

#ifdef XXX_SUPPORT_TRIMMING
                        if (fs_trim_mach_two)
                        {
                            if (    ( ! apply_score_cutoff)
                                 || (    (dis_score_vp->elements[ dis_item_count + dis_item ] > dis_score_cutoff)
                                      && (con_score_vp->elements[ con_item_count + con_item ] > con_score_cutoff)
                                    )
                               )
                            {
                                log_p += (log(prob_sum) + con_max_log_prob);
                                num_good_pairs++;
                            }
                        }
                        else
                        {
#endif
                            log_p += (log(prob_sum) + con_max_log_prob);
#ifdef XXX_SUPPORT_TRIMMING
                        }
#endif

                        ASSERT_IS_NUMBER_DBL(log_p);
                        ASSERT_IS_FINITE_DBL(log_p);
                    }

                    ASSERT_IS_NUMBER_DBL(log_p);
                    ASSERT_IS_FINITE_DBL(log_p);

                    /* Geometric mean over matches? A hack, but should be OK? */
                    /*
                    // Note that any changes here need to be reflected
                    // in the computation of ll_con_correction_norm_sum.
                    */
#ifdef XXX_SUPPORT_TRIMMING
                    if (fs_trim_mach_two)
                    {
                        if (num_good_pairs > 0)
                        {
                            log_p /= num_good_pairs;
                        }
                    }
                    else
                    {
#endif
                        ASSERT_IS_POSITIVE_INT(num_matches);
                        log_p /= num_matches;
#ifdef XXX_SUPPORT_TRIMMING
                    }
#endif

                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        if (dis_item_match_count_vp->elements[ dis_item ] == 0)
                        {
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_ppp[ level ][ cluster_index ][ dis_item ] = DBL_NOT_SET;
                            }
                        }
                        else
                        {
                            level_sum = 0.0;

                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_sum += level_indicators_ppp[ level ][ cluster_index ][ dis_item ];
                            }

                            ASSERT_IS_GREATER_DBL(level_sum, 1000.0 * DBL_MIN);

                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_ppp[ level ][ cluster_index ][ dis_item ] /= level_sum;
                            }

#ifdef MIN_LEVEL_INDICATOR
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_pp = level_indicators_ppp[ level ];

                                level_indicators_pp[ cluster_index ][ dis_item ] +=
                                                                   delta_to_ensure_min_level_indicator;
                                level_indicators_pp[ cluster_index ][ dis_item ] /=
                                                              min_level_indicator_norm;
                            }
#endif
                        }

                    }

                    for (con_item = 0; con_item < num_con_items; con_item++)
                    {
                        item = con_item + max_num_dis_items;

                        if (con_item_match_count_vp->elements[ con_item ] == 0)
                        {
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_ppp[ level ][ cluster_index ][ item ] = DBL_NOT_SET;
                            }
                        }
                        else
                        {
                            level_sum = 0.0;

                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_sum += level_indicators_ppp[ level ][ cluster_index ][ item ];
                            }

                            ASSERT_IS_GREATER_DBL(level_sum, 1000.0 * DBL_MIN);

                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_ppp[ level ][ cluster_index ][ item ] /= level_sum;
                            }

#ifdef MIN_LEVEL_INDICATOR
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_pp = level_indicators_ppp[ level ];
                                level_indicators_p = level_indicators_pp[ cluster_index ] + max_num_dis_items + con_item;

                                *level_indicators_p += delta_to_ensure_min_level_indicator;
                                *level_indicators_p /= min_level_indicator_norm;
                            }
#endif
                        }
                    }

#ifdef XXX_SUPPORT_TRIMMING
                    if (fs_trim_mach_two)
                    {
                        if (num_good_pairs > 0)
                        {
                            log_p += log_a;
                            ASSERT_IS_NUMBER_DBL(log_p);
                            ASSERT_IS_FINITE_DBL(log_p);

                            ll_P_c_p_vp->elements[ cluster_index ] = log_p;

                            P_c_p_vp->elements[ cluster_index ] = log_p;
                        }
                        else
                        {
                            ll_P_c_p_vp->elements[ cluster_index ] = DBL_NOT_SET;

                            P_c_p_vp->elements[ cluster_index ] = DBL_NOT_SET;

                        }
                    }
                    else
                    {
#endif
                        ASSERT_IS_NUMBER_DBL(log_p);
                        ASSERT_IS_FINITE_DBL(log_p);

                        log_p += log_a;

                        ASSERT_IS_NUMBER_DBL(log_p);
                        ASSERT_IS_FINITE_DBL(log_p);

                        ll_P_c_p_vp->elements[ cluster_index ] = log_p;

                        P_c_p_vp->elements[ cluster_index ] = log_p;
#ifdef XXX_SUPPORT_TRIMMING
                    }
#endif

#ifdef REPORT_CPU_TIME
                    pair_item_prob_update_cpu += get_cpu_time() - cpu_time_offset;
                    cpu_time_offset = get_cpu_time();
#endif

                }
            }
            else if (    (options_ptr->model_correspondence == 2)
                      && ( ! options_ptr->share_matches)
                    )
            {
                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {
                    double log_a = DBL_MOST_NEGATIVE;
#ifdef XXX_SUPPORT_TRIMMING
                    int num_good_pairs = 0;
#endif

                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                    if (    (fs_use_specified_clusters)
                         && (data_ptr->group_vp != NULL)
                       )
                    {
                        if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    }
#ifdef TEST
                    sum_nu = 0.0;
#endif

#ifdef TEST
                    if (valid_cluster_index)
                    {
                        ASSERT_IS_EQUAL_INT(cluster, cluster_index_mp->elements[ point ][ cluster_index ]);
                    }
#endif
                    ASSERT_IS_NON_NEGATIVE_INT(cluster_index);
                    ASSERT_IS_LESS_INT(cluster_index , num_clusters);

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    p = a_vp->elements[ cluster ];

                    if (p < 5.0 * DBL_MIN)
                    {
                        TEST_PSO(("Cluster (%d) with no meat spotted in E step.\n",
                                  cluster + 1));
                        dbe(p);
                        log_a = LOG_ZERO;
                    }
                    else
                    {
                        log_a = log(p);
                    }

#ifdef REPORT_CPU_TIME
                    cpu_time_offset = get_cpu_time();
#endif

                    /*
                    // Cache the level weights, taking logs if we are using logs for
                    // probs.
                    */
                    for (level = first_level; level < last_level_num; level++)
                    {
                        if (valid_cluster_dependent_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_c_mvp->length);
                            ASSERT_IS_LESS_INT(point , P_l_p_c_mvp->elements[ level ]->num_rows);
                            ASSERT_IS_LESS_INT(cluster_index , P_l_p_c_mvp->elements[ level ]->num_cols);

                            nu = P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else if (valid_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_mp->num_rows);
                            ASSERT_IS_LESS_INT(point , P_l_p_mp->num_cols);

                            nu = P_l_p_mp->elements[ level ][ point ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else
                        {
                            nu = V_mp->elements[ level ][ cluster ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
#ifdef TEST
                        sum_nu += nu;
#endif

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);

                        nu_vp->elements[ level ] = nu;

                        if (nu < 5.0 * DBL_MIN)
                        {
                            log_nu_vp->elements[ level ] = LOG_ZERO;
                        }
                        else
                        {
                            log_nu_vp->elements[ level ] = log(nu);
                        }
                    }

#ifdef TEST
                    ASSERT_IS_NUMBER_DBL(sum_nu);
                    ASSERT_IS_FINITE_DBL(sum_nu);
                    ASSERT_IS_LESS_DBL(sum_nu , 1.0001);
                    ASSERT_IS_GREATER_DBL(sum_nu, 0.9999);
#endif

#ifdef REPORT_CPU_TIME
                    vertical_weight_caching_cpu += (get_cpu_time() - cpu_time_offset);
                    cpu_time_offset = get_cpu_time();
#endif

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /* Set up pair prob by level table */
                    for (level = first_level; level < last_level_num; level++)
                    {
                        double* log_dis_prob_ptr;


                        if (result == ERROR) { NOTE_ERROR(); break; }

                        node   = node_mp->elements[ cluster ][ level ];
                        log_nu = log_nu_vp->elements[ level ];

                        log_dis_prob_ptr = log_P_i_n_mvp->elements[ level ]->elements[ node ];

                         /* dbi(level);  */

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];

                            if (result == ERROR) { NOTE_ERROR(); break; }

                            log_prob     = con_prob_cache_vvp->elements[ level ]->elements[ node ];

                            /*
                            // CHECK this HACK
                            //
                            // Note that we are currently ignoring
                            // multiplicity, which gets tricky when doing
                            // strict matching.
                            //
                            // Note that any changes here need to be reflected
                            // in the computation of ll_con_correction_norm_sum.
                            */
                            log_nu_times_prob = log_prob + log_nu;
#ifdef IT_WAS_MORE_LIKE_THIS_BEFORE_REWRITE
                            log_nu_times_prob = con_factor * log_prob + log_nu;
#endif

                            for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                            {
                                category = dis_item_mp->elements[ point ][ dis_item ];

                                ASSERT_IS_NON_NEGATIVE_INT(category);

#ifdef IT_WAS_MORE_LIKE_THIS_BEFORE_REWRITE
                                dis_item_multiplicity = dis_item_multiplicity_mp->elements[ point ][ dis_item ];
                                ASSERT_IS_NUMBER_DBL(dis_item_multiplicity);
                                ASSERT_IS_NON_NEGATIVE_DBL(dis_item_multiplicity);
#endif

                                ASSERT_IS_NUMBER_DBL(log_nu_times_prob);
                                ASSERT_IS_NUMBER_DBL(dis_factor);
                                ASSERT_IS_NON_NEGATIVE_DBL(dis_factor);
                                ASSERT_IS_NUMBER_DBL(log_dis_prob_ptr[ category ]);

                                /*
                                // CHECK this HACK
                                //
                                // Note that we are currently ignoring
                                // multiplicity, which gets tricky when doing
                                // strict matching.
                                */
                                log_pair_level_probs_mvp->elements[ level ]->elements[ con_item ][ dis_item ] =
                                    log_nu_times_prob + log_dis_prob_ptr[ category ];

#ifdef IT_WAS_MORE_LIKE_THIS_BEFORE_REWRITE
                                log_pair_level_probs_mvp->elements[ level ]->elements[ con_item ][ dis_item ] =
                                    log_nu_times_prob + dis_factor * dis_item_multiplicity * log_dis_prob_ptr[ category ];
#endif
                            }
                        }
                    }

                    if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
                    pair_prob_table_cpu += (get_cpu_time() - cpu_time_offset);
                    cpu_time_offset = get_cpu_time();
#endif

                    log_pair_probs_mp->num_rows = num_con_items;
                    log_pair_probs_mp->num_cols = num_dis_items;

                    if (result != ERROR)
                    {
                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                            {
                                max_log_prob = DBL_MOST_NEGATIVE;

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    max_log_prob = MAX_OF(max_log_prob,
                                                          log_pair_level_probs_mvp->elements[ level ]->elements[ con_item ][ dis_item ]);
                                }

                                sum = 0.0;

                                for (level = first_level; level < last_level_num; level++)
                                {
                                    log_prob = log_pair_level_probs_mvp->elements[ level ]->elements[ con_item ][ dis_item ];
                                    log_prob -= max_log_prob;

                                    sum += exp(log_prob);
                                }

                                log_pair_probs_mp->elements[ con_item ][ dis_item ] = log(sum) + max_log_prob;
                            }
                        }
                    }

#ifdef XXX_SUPPORT_TRIMMING
#ifndef XXX_OLD_PAIR_NULL_METHOD
                    if (options_ptr->trim_fraction > 0.0)
                    {
                        num_nulls = ((double)num_con_items * options_ptr->trim_fraction) + 0.5;

                        if (num_nulls >= num_con_items)
                        {
                            num_nulls = num_con_items - 1;
                        }
                    }
#endif
#endif

                    if (MIN_OF(num_con_items, num_dis_items) > 2)
                    {
                        num_nulls = 1;
                    }
                    else
                    {
                        num_nulls = 0;
                    }

                    result = get_matches(&match_mp, &num_matches,
                                         log_pair_probs_mp,
                                            (options_ptr->sample_matches)
                                         && (    (options_ptr->last_sample < 0)
                                              || (it <= options_ptr->last_sample)),
                                         num_nulls,
                                         fs_duplicate_words_for_matching);

                    if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
                    pair_match_cpu += (get_cpu_time() - cpu_time_offset);
                    cpu_time_offset = get_cpu_time();
#endif

                    /* Update--Dry Run */

                    result = ow_zero_int_vector(dis_item_match_count_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_zero_int_vector(con_item_match_count_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_set_vector(dis_item_max_log_vp,
                                           DBL_MOST_NEGATIVE);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_set_vector(con_item_max_log_vp,
                                           DBL_MOST_NEGATIVE);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (match = 0; match < num_matches; match++)
                    {
                        con_item = match_mp->elements[ match ][ 0 ];
                        dis_item = match_mp->elements[ match ][ 1 ];

                        ASSERT_IS_NON_NEGATIVE_INT(con_item);
                        ASSERT_IS_LESS_INT(con_item , num_con_items);
                        ASSERT_IS_NON_NEGATIVE_INT(dis_item);
                        ASSERT_IS_LESS_INT(dis_item , num_dis_items);

                        (dis_item_match_count_vp->elements[ dis_item ])++;
                        (con_item_match_count_vp->elements[ con_item ])++;

                        for (level = first_level; level < last_level_num; level++)
                        {
                            log_prob = log_pair_level_probs_mvp->elements[ level ]->elements[ con_item ][ dis_item ];

                            con_item_max_log_vp->elements[ con_item ] =
                                MAX_OF(log_prob,
                                       con_item_max_log_vp->elements[ con_item ]);

                            dis_item_max_log_vp->elements[ dis_item ] =
                                MAX_OF(log_prob,
                                       dis_item_max_log_vp->elements[ dis_item ]);

                        }
                    }

                    for (level = first_level; level < last_level_num; level++)
                    {
                        level_indicators_p = level_indicators_ppp[ level ][ cluster_index ];

                        for (item = 0; item < max_num_items; item++)
                        {
                            *level_indicators_p = 0.0;
                            level_indicators_p++;
                        }
                    }

                    /* Update--Wet Run */

                    log_p = 0.0;

                    for (match = 0; match < num_matches; match++)
                    {
                        con_item = match_mp->elements[ match ][ 0 ];
                        dis_item = match_mp->elements[ match ][ 1 ];

                        ASSERT_IS_NON_NEGATIVE_INT(con_item);
                        ASSERT_IS_LESS_INT(con_item , num_con_items);
                        ASSERT_IS_NON_NEGATIVE_INT(dis_item);
                        ASSERT_IS_LESS_INT(dis_item , num_dis_items);

                        prob_sum = 0.0;

                        dis_max_log_prob = dis_item_max_log_vp->elements[ dis_item ];
                        con_max_log_prob = con_item_max_log_vp->elements[ con_item ];

                        for (level = first_level; level < last_level_num; level++)
                        {
                            log_prob = log_pair_level_probs_mvp->elements[ level ]->elements[ con_item ][ dis_item ];

                            level_indicators_p = level_indicators_ppp[ level ][ cluster_index ];

                            dis_prob = exp(log_prob - dis_max_log_prob);
                            level_indicators_p[ dis_item ] += dis_prob;

                            con_prob = exp(log_prob - con_max_log_prob);
                            level_indicators_p[ max_num_dis_items + con_item ] += con_prob;

                            /*
                            // con_prob and dis_prob are the same
                            // animal--only the normalizations are
                            // different. Both are pair probs. Use con_prob for
                            // likelihood estimate.
                            */
                            prob_sum += con_prob;
                        }

                        ASSERT_IS_NUMBER_DBL(prob_sum);
                        ASSERT_IS_FINITE_DBL(prob_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);

                        log_prob_sum = log(prob_sum) + con_max_log_prob;

#ifdef XXX_SUPPORT_TRIMMING
                        if (fs_trim_mach_two)
                        {
                            if (    ( ! apply_score_cutoff)
                                 || (    (dis_score_vp->elements[ dis_item_count + dis_item ] > dis_score_cutoff)
                                      && (con_score_vp->elements[ con_item_count + con_item ] > con_score_cutoff)
                                    )
                               )
                            {
                                log_p += log_prob_sum;
                                num_good_pairs++;
                            }
                        }
                        else
                        {
#endif
                            log_p += log_prob_sum;
#ifdef XXX_SUPPORT_TRIMMING
                        }
#endif


                        ASSERT_IS_NUMBER_DBL(log_p);
                        ASSERT_IS_FINITE_DBL(log_p);
                    }

                    ASSERT_IS_POSITIVE_INT(num_matches);

                    /* Geometric mean over matches? A hack, but should be OK? */
                    /*
                    // If this normalization is changed, then the adjustment in
                    // the likelihood for con_log_norm needs to change also.
                    */
#ifdef XXX_SUPPORT_TRIMMING
                    if (fs_trim_mach_two)
                    {
                        if (num_good_pairs > 0)
                        {
                            log_p /= num_good_pairs;
                        }
                    }
                    else
                    {
#endif
                        log_p /= num_matches;
#ifdef XXX_SUPPORT_TRIMMING
                    }
#endif

#ifdef TEST
#ifdef REPORT_CPU_TIME
        /* TEST */  cpu_time_offset = get_cpu_time();
#endif
        /* TEST */  for (l = 0; l< num_levels; l++)
        /* TEST */  {
        /* TEST */      level_indicators_pp = level_indicators_ppp[ l];
        /* TEST */
        /* TEST */      for (c = 0; c< num_clusters; c++)
        /* TEST */      {
        /* TEST */          level_indicators_p = level_indicators_pp[ c];
        /* TEST */
        /* TEST */          for (i= 0; i< max_num_items; i++)
        /* TEST */          {
                                if (    (*level_indicators_p < 0.0)
                                     && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
                                          || (*level_indicators_p < DBL_NOT_SET - 0.001)
                                        )
                                   )
                                {
                                    dbe(*level_indicators_p);
                                    dbi(l);
                                    dbi(c);
                                    dbi(i);

                                    SET_CANT_HAPPEN_BUG();

                                    result = ERROR;
                                    break;
                                }
        /* TEST */              level_indicators_p++;
        /* TEST */          }
        /* TEST */      }
        /* TEST */  }
        /* TEST */
        /* TEST */  /* Breaks shared correspondence */
        /* TEST */  /* result = ow_set_vector(P_c_p_vp, DBL_NOT_SET); */
#ifdef REPORT_CPU_TIME
        /* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif
                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        if (dis_item_match_count_vp->elements[ dis_item ] == 0)
                        {
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_ppp[ level ][ cluster_index ][ dis_item ] = DBL_NOT_SET;
                            }
                        }
                        else
                        {
                            level_sum = 0.0;

                            for (level = first_level; level < last_level_num; level++)
                            {
                                ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster_index ][ dis_item ]);

                                if (level_indicators_ppp[ level ][ cluster_index ][ dis_item ] > 0.0)
                                {
                                    level_sum += level_indicators_ppp[ level ][ cluster_index ][ dis_item ];
                                }
                            }

                            if (level_sum > 0.0)
                            {
                                for (level = first_level; level < last_level_num; level++)
                                {
                                    ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster_index ][ dis_item ]);

                                    if (level_indicators_ppp[ level ][ cluster_index ][ dis_item ] > 0.0)
                                    {
                                        level_indicators_ppp[ level ][ cluster_index ][ dis_item ] /= level_sum;
                                        ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster_index ][ dis_item ]);
                                        ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_ppp[ level ][ cluster_index ][ dis_item ]);
                                    }
                                }
                            }

#ifdef MIN_LEVEL_INDICATOR
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_pp = level_indicators_ppp[ level ];

                                level_indicators_pp[ cluster_index ][ dis_item ] +=
                                                              delta_to_ensure_min_level_indicator;
                                level_indicators_pp[ cluster_index ][ dis_item ] /=
                                                              min_level_indicator_norm;
                            }
#endif
                        }
                    }
#ifdef TEST
#ifdef REPORT_CPU_TIME
        /* TEST */  cpu_time_offset = get_cpu_time();
#endif
        /* TEST */  for (l = 0; l< num_levels; l++)
        /* TEST */  {
        /* TEST */      level_indicators_pp = level_indicators_ppp[ l];
        /* TEST */
        /* TEST */      for (c = 0; c< num_clusters; c++)
        /* TEST */      {
        /* TEST */          level_indicators_p = level_indicators_pp[ c];
        /* TEST */
        /* TEST */          for (i= 0; i< max_num_items; i++)
        /* TEST */          {
                                if (    (*level_indicators_p < 0.0)
                                     && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
                                          || (*level_indicators_p < DBL_NOT_SET - 0.001)
                                        )
                                   )
                                {
                                    dbe(*level_indicators_p);
                                    dbi(l);
                                    dbi(c);
                                    dbi(i);

                                    SET_CANT_HAPPEN_BUG();

                                    result = ERROR;
                                    break;
                                }
        /* TEST */              level_indicators_p++;
        /* TEST */          }
        /* TEST */      }
        /* TEST */  }
        /* TEST */
        /* TEST */  /* Breaks shared correspondence */
        /* TEST */  /* result = ow_set_vector(P_c_p_vp, DBL_NOT_SET); */
#ifdef REPORT_CPU_TIME
        /* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif

                    for (con_item = 0; con_item < num_con_items; con_item++)
                    {
                        item = con_item + max_num_dis_items;

                        if (con_item_match_count_vp->elements[ con_item ] == 0)
                        {
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_ppp[ level ][ cluster_index ][ item ] = DBL_NOT_SET;
                            }
                        }
                        else
                        {
                            level_sum = 0.0;

                            for (level = first_level; level < last_level_num; level++)
                            {
                                ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster_index ][ dis_item ]);

                                if (level_indicators_ppp[ level ][ cluster_index ][ dis_item ] > 0.0)
                                {
                                    level_sum += level_indicators_ppp[ level ][ cluster_index ][ item ];
                                }
                            }

                            if (level_sum > 0.0)
                            {
                                for (level = first_level; level < last_level_num; level++)
                                {
                                    ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster_index ][ item ]);

                                    if (level_indicators_ppp[ level ][ cluster_index ][ item ] > 0.0)
                                    {
                                        level_indicators_ppp[ level ][ cluster_index ][ item ] /= level_sum;

                                        ASSERT_IS_NUMBER_DBL(level_indicators_ppp[ level ][ cluster_index ][ item ]);
                                        ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_ppp[ level ][ cluster_index ][ item ]);
                                    }
                                }
                            }

#ifdef MIN_LEVEL_INDICATOR
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_pp = level_indicators_ppp[ level ];
                                level_indicators_p = level_indicators_pp[ cluster_index ] + max_num_dis_items + con_item;

                                *level_indicators_p += delta_to_ensure_min_level_indicator;
                                *level_indicators_p /= min_level_indicator_norm;
                            }
#endif
                        }
                    }
#ifdef TEST
#ifdef REPORT_CPU_TIME
        /* TEST */  cpu_time_offset = get_cpu_time();
#endif
        /* TEST */  for (l = 0; l< num_levels; l++)
        /* TEST */  {
        /* TEST */      level_indicators_pp = level_indicators_ppp[ l];
        /* TEST */
        /* TEST */      for (c = 0; c< num_clusters; c++)
        /* TEST */      {
        /* TEST */          level_indicators_p = level_indicators_pp[ c];
        /* TEST */
        /* TEST */          for (i= 0; i< max_num_items; i++)
        /* TEST */          {
                                if (    (*level_indicators_p < 0.0)
                                     && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
                                          || (*level_indicators_p < DBL_NOT_SET - 0.001)
                                        )
                                   )
                                {
                                    dbe(*level_indicators_p);
                                    dbi(l);
                                    dbi(c);
                                    dbi(i);

                                    SET_CANT_HAPPEN_BUG();

                                    result = ERROR;
                                    break;
                                }
        /* TEST */              level_indicators_p++;
        /* TEST */          }
        /* TEST */      }
        /* TEST */  }
        /* TEST */
        /* TEST */  /* Breaks shared correspondence */
        /* TEST */  /* result = ow_set_vector(P_c_p_vp, DBL_NOT_SET); */
#ifdef REPORT_CPU_TIME
        /* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif

#ifdef XXX_SUPPORT_TRIMMING
#ifdef XXX_OLD_PAIR_NULL_METHOD
                    /*
                    // Scores for null determination.
                    */
                    if (options_ptr->trim_fraction > 0.0)
                    {
                        for (match = 0; match < num_matches; match++)
                        {
                            double log_pair_prob;

                            con_item = match_mp->elements[ match ][ 0 ];
                            dis_item = match_mp->elements[ match ][ 1 ];

                            log_pair_prob = log_pair_probs_mp->elements[ con_item ][ dis_item ];

                            if (fs_trim_hack == 1)
                            {
                                log_pair_prob += log_a;
                            }
                            else if (fs_trim_hack == 2)
                            {
                                log_pair_prob += log(P_c_p_mp->elements[ point ][ cluster ]);
                            }

                            i = dis_score_counts_vp->elements[ dis_item ];
                            dis_score_mp->elements[ dis_item ][ i ] = log_pair_prob;
                            (dis_score_counts_vp->elements[ dis_item ])++;

                            i = con_score_counts_vp->elements[ con_item ];
                            con_score_mp->elements[ con_item ][ i ] = log_pair_prob;
                            (con_score_counts_vp->elements[ con_item ])++;
                        }
                    }
#endif
#endif

#ifdef TEST
#ifdef REPORT_CPU_TIME
        /* TEST */  cpu_time_offset = get_cpu_time();
#endif
        /* TEST */  for (l = 0; l< num_levels; l++)
        /* TEST */  {
        /* TEST */      level_indicators_pp = level_indicators_ppp[ l];
        /* TEST */
        /* TEST */      for (c = 0; c< num_clusters; c++)
        /* TEST */      {
        /* TEST */          level_indicators_p = level_indicators_pp[ c];
        /* TEST */
        /* TEST */          for (i= 0; i< max_num_items; i++)
        /* TEST */          {
                                if (    (*level_indicators_p < 0.0)
                                     && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
                                          || (*level_indicators_p < DBL_NOT_SET - 0.001)
                                        )
                                   )
                                {
                                    dbe(*level_indicators_p);
                                    dbi(l);
                                    dbi(c);
                                    dbi(i);

                                    SET_CANT_HAPPEN_BUG();

                                    result = ERROR;
                                    break;
                                }
        /* TEST */              level_indicators_p++;
        /* TEST */          }
        /* TEST */      }
        /* TEST */  }
        /* TEST */
        /* TEST */  /* Breaks shared correspondence */
        /* TEST */  /* result = ow_set_vector(P_c_p_vp, DBL_NOT_SET); */
#ifdef REPORT_CPU_TIME
        /* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif

#ifdef XXX_SUPPORT_TRIMMING
                    if (fs_trim_mach_two)
                    {
                        if (num_good_pairs > 0)
                        {
                            log_p += log_a;
                            ASSERT_IS_NUMBER_DBL(log_p);
                            ASSERT_IS_FINITE_DBL(log_p);

                            ll_P_c_p_vp->elements[ cluster_index ] = log_p;

                            P_c_p_vp->elements[ cluster_index ] = log_p;
                        }
                        else
                        {
                            ll_P_c_p_vp->elements[ cluster_index ] = DBL_NOT_SET;

                            P_c_p_vp->elements[ cluster_index ] = DBL_NOT_SET;

                        }
                    }
                    else
                    {
#endif
                        log_p += log_a;
                        ASSERT_IS_NUMBER_DBL(log_p);
                        ASSERT_IS_FINITE_DBL(log_p);

                        ll_P_c_p_vp->elements[ cluster_index ] = log_p;

                        P_c_p_vp->elements[ cluster_index ] = log_p;
#ifdef XXX_SUPPORT_TRIMMING
                    }
#endif

#ifdef REPORT_CPU_TIME
                    pair_item_prob_update_cpu += get_cpu_time() - cpu_time_offset;
                    cpu_time_offset = get_cpu_time();
#endif

                }

            }
            /*
             * Comment: June 10, 2005: Something along thise lines might make
             * sense. This code might provide a template. Keep for now.
             *
             * JUNK ?
             *
             * Comment after the fact: This perhaps should be junked. We could
             * check to see if performance is OK, but I don't recall that it
             * was, and I think the model does not make any sense. Basically, we
             * are doing P(l|w)=P(w|l)P(l|W,B), where P(l|W,B) is estimated by
             * marginalizing over all the (w,b) pairs.
            */
            else if (options_ptr->model_correspondence == 3)
            {
                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {
                    double log_a = DBL_MOST_NEGATIVE;
#ifdef XXX_SUPPORT_TRIMMING
                    int num_good_pairs = 0;
#endif

                    if (    (fs_use_specified_clusters)
                         && (data_ptr->group_vp != NULL)
                       )
                    {
                        if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    }

                    log_p = 0.0;


                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];
#ifdef TEST
                    sum_nu = 0.0;
#endif

#ifdef TEST
                    if (valid_cluster_index)
                    {
                        ASSERT_IS_EQUAL_INT(cluster, cluster_index_mp->elements[ point ][ cluster_index ]);
                    }
#endif
                    ASSERT_IS_NON_NEGATIVE_INT(cluster_index);
                    ASSERT_IS_LESS_INT(cluster_index , num_clusters);

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    p = a_vp->elements[ cluster ];

                    if (p < 5.0 * DBL_MIN)
                    {
                        TEST_PSO(("Cluster (%d) with no meat spotted in E step.\n",
                                  cluster + 1));
                        dbe(p);
                        log_a = LOG_ZERO;
                    }
                    else
                    {
                        log_a = log(p);
                    }

                    /*
                    // Cache the level weights, taking logs if we are using logs for
                    // probs.
                    */
#ifdef REPORT_CPU_TIME
                    cpu_time_offset = get_cpu_time();
#endif

                    for (level = first_level; level < last_level_num; level++)
                    {
                        if (valid_cluster_dependent_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_c_mvp->length);
                            ASSERT_IS_LESS_INT(point , P_l_p_c_mvp->elements[ level ]->num_rows);
                            ASSERT_IS_LESS_INT(cluster_index , P_l_p_c_mvp->elements[ level ]->num_cols);

                            nu = P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else if (valid_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_mp->num_rows);
                            ASSERT_IS_LESS_INT(point , P_l_p_mp->num_cols);

                            nu = P_l_p_mp->elements[ level ][ point ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else
                        {
                            nu = V_mp->elements[ level ][ cluster ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
#ifdef TEST
                        sum_nu += nu;
#endif

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);

                        nu_vp->elements[ level ] = nu;

                        if (nu < 5.0 * DBL_MIN)
                        {
                            log_nu_vp->elements[ level ] = LOG_ZERO;
                        }
                        else
                        {
                            log_nu_vp->elements[ level ] = log(nu);
                        }
                    }

#ifdef TEST
                    ASSERT_IS_NUMBER_DBL(sum_nu);
                    ASSERT_IS_FINITE_DBL(sum_nu);
                    ASSERT_IS_LESS_DBL(sum_nu , 1.0001);
                    ASSERT_IS_GREATER_DBL(sum_nu, 0.9999);
#endif

#ifdef REPORT_CPU_TIME
                    vertical_weight_caching_cpu += (get_cpu_time() - cpu_time_offset);
                    cpu_time_offset = get_cpu_time();
#endif

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /* Set up pair prob by level table */
                    for (level = first_level; level < last_level_num; level++)
                    {
                        double* log_dis_prob_ptr;


                        if (result == ERROR) { NOTE_ERROR(); break; }

                        node   = node_mp->elements[ cluster ][ level ];
                        /*
                        log_nu = log_nu_vp->elements[ level ];
                        */

                        log_dis_prob_ptr = log_P_i_n_mvp->elements[ level ]->elements[ node ];

                         /* dbi(level);  */

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];

                            if (result == ERROR) { NOTE_ERROR(); break; }

                            log_con_prob = con_prob_cache_vvp->elements[ level ]->elements[ node ];

                            /*
                            log_nu_times_con_prob = log_con_prob + log_nu;
                            */

                            for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                            {
                                category = dis_item_mp->elements[ point ][ dis_item ];

                                ASSERT_IS_NON_NEGATIVE_INT(category);
                                ASSERT_IS_NUMBER_DBL(log_dis_prob_ptr[ category ]);

                                log_pair_level_probs_mvp->elements[ level ]->elements[ con_item ][ dis_item ] =
                                    log_con_prob /* log_nu_times_con_prob */ + log_dis_prob_ptr[ category ];
                            }
                        }

                        log_pair_level_priors_vp->elements[ level ] =
                            log_sum_log_matrix_elements(log_pair_level_probs_mvp->elements[ level ]);
                    }

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (level = first_level; level < last_level_num; level++)
                    {
                        log_pair_level_priors_vp->elements[ level ] += log_nu_vp->elements[ level ];
                    }

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = ow_normalize_log_prob_vp(log_pair_level_priors_vp);

#ifdef REPORT_CPU_TIME
                    pair_prob_table_cpu += (get_cpu_time() - cpu_time_offset);
                    cpu_time_offset = get_cpu_time();
#endif

                    for (level = first_level; level < last_level_num; level++)
                    {
                        level_indicators_p = level_indicators_ppp[ level ][ cluster_index ];

                        for (item = 0; item < max_num_items; item++)
                        {
                            *level_indicators_p = 0.0;
                            level_indicators_p++;
                        }
                    }

                    for (level = first_level; level < last_level_num; level++)
                    {
                        node   = node_mp->elements[ cluster ][ level ];
                        level_indicators_p = level_indicators_ppp[ level ][ cluster_index ];

                        for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                        {
                            category = dis_item_mp->elements[ point ][ dis_item ];

                            log_prob = log_pair_level_priors_vp->elements[ level ];
                            log_prob += log_P_i_n_mvp->elements[ level ]->elements[ node ][ category ];

                            level_indicators_p[ dis_item ] = log_prob;

                            log_p += log_prob;  /* For LL (Do we want adjustments for counts?) */
                        }

                        level_indicators_p = level_indicators_ppp[ level ][ cluster_index ] + max_num_dis_items;

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];

                            log_prob = log_pair_level_priors_vp->elements[ level ];
                            log_prob += con_prob_cache_vvp->elements[ level ]->elements[ node ];

                            level_indicators_p[ con_item ] = log_prob;

                            log_p += log_prob; /* For LL (Do we want adjustments for counts?)  */
                        }
                    }

                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        max_log_prob = DBL_MOST_NEGATIVE;
                        sum = 0.0;

                        for (level = first_level; level < last_level_num; level++)
                        {
                            max_log_prob = MAX_OF(max_log_prob,
                                                  level_indicators_ppp[ level ][ cluster_index ][ dis_item ]);
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            log_prob = level_indicators_ppp[ level ][ cluster_index ][ dis_item ];
                            prob = exp(log_prob - max_log_prob);
                            level_indicators_ppp[ level ][ cluster_index ][ dis_item ] = prob;
                            sum += prob;
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_ppp[ level ][ cluster_index ][ dis_item ] /= sum;
                        }
                    }


                    for (con_item = 0; con_item < max_num_con_items; con_item++)
                    {
                        max_log_prob = DBL_MOST_NEGATIVE;
                        sum = 0.0;

                        for (level = first_level; level < last_level_num; level++)
                        {
                            max_log_prob = MAX_OF(max_log_prob,
                                                  level_indicators_ppp[ level ][ cluster_index ][ max_num_dis_items + con_item ]);
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            log_prob = level_indicators_ppp[ level ][ cluster_index ][ max_num_dis_items + con_item ];
                            prob = exp(log_prob - max_log_prob);
                            level_indicators_ppp[ level ][ cluster_index ][ max_num_dis_items + con_item ] = prob;
                            sum += prob;
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_ppp[ level ][ cluster_index ][ max_num_dis_items + con_item ] /= sum;
                        }
                    }

                    log_p += log_a;
                    ASSERT_IS_NUMBER_DBL(log_p);
                    ASSERT_IS_FINITE_DBL(log_p);

                    ll_P_c_p_vp->elements[ cluster_index ] = log_p;

                    P_c_p_vp->elements[ cluster_index ] = log_p;

#ifdef REPORT_CPU_TIME
                    pair_item_prob_update_cpu += get_cpu_time() - cpu_time_offset;
                    cpu_time_offset = get_cpu_time();
#endif

                }

            }
#ifdef NEW_STUFF   /* HERE, TODO, CURRENT */
            /*
             * This is a model where there is no correspondence issues. It
             * implements a pure multi-modal mixture model. It is equivalent to
             * some uses of the program with one con_item and one dis_item per
             * document, but it is explicit about the correspondence. The main
             * reason for having it is for document clustering. For example, in
             * the psych study, we pretend we have correspondence of data over a
             * long run of time, but sometimes want to cluster the people or
             * pairs based on the many data items.
            */
            else if (options_ptr->model_correspondence == 4)
            {
                int num_items;
                double con_item_weight = 1.0;

                if ((model_ptr->weight_con_items) && (con_item_weight_vvp != NULL))
                {
                    static int first_time = TRUE;

                    if (first_time)
                    {
                        first_time = FALSE;
                        dbp("Weighting continous features.\n");
                    }

                    con_item_weight = con_item_weight_vvp->elements[point]->elements[con_item];
                }

                if (num_con_items != num_dis_items)
                {
                    set_error("Correspondence model 4 requires equal numbers of con items and dis items.");
                    result = ERROR;
                    NOTE_ERROR();
                    break;
                }

                num_items = num_con_items;

                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {
                    double log_a = DBL_MOST_NEGATIVE;
#ifdef XXX_SUPPORT_TRIMMING
                    int num_good_pairs = 0;
#endif

                    log_product = 0.0;

                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                    if (    (fs_use_specified_clusters)
                         && (data_ptr->group_vp != NULL)
                       )
                    {
                        if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    }
#ifdef TEST
                    sum_nu = 0.0;
#endif

#ifdef TEST
                    if (valid_cluster_index)
                    {
                        ASSERT_IS_EQUAL_INT(cluster, cluster_index_mp->elements[ point ][ cluster_index ]);
                    }
#endif
                    ASSERT_IS_NON_NEGATIVE_INT(cluster_index);
                    ASSERT_IS_LESS_INT(cluster_index , num_clusters);

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    p = a_vp->elements[ cluster ];

                    if (p < 5.0 * DBL_MIN)
                    {
                        TEST_PSO(("Cluster (%d) with no meat spotted in E step.\n",
                                  cluster + 1));
                        dbe(p);
                        log_a = LOG_ZERO;
                    }
                    else
                    {
                        log_a = log(p);
                    }

#ifdef REPORT_CPU_TIME
                    cpu_time_offset = get_cpu_time();
#endif

                    /*
                    // Cache the level weights, taking logs if we are using logs for
                    // probs.
                    */
                    for (level = first_level; level < last_level_num; level++)
                    {
                        if (valid_cluster_dependent_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_c_mvp->length);
                            ASSERT_IS_LESS_INT(point , P_l_p_c_mvp->elements[ level ]->num_rows);
                            ASSERT_IS_LESS_INT(cluster_index , P_l_p_c_mvp->elements[ level ]->num_cols);

                            nu = P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else if (valid_aspects)
                        {
                            ASSERT_IS_LESS_INT(level , P_l_p_mp->num_rows);
                            ASSERT_IS_LESS_INT(point , P_l_p_mp->num_cols);

                            nu = P_l_p_mp->elements[ level ][ point ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
                        else
                        {
                            nu = V_mp->elements[ level ][ cluster ];

                            ASSERT_IS_NUMBER_DBL(nu);
                            ASSERT_IS_FINITE_DBL(nu);
                            ASSERT_IS_NON_NEGATIVE_DBL(nu);
                        }
#ifdef TEST
                        sum_nu += nu;
#endif

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);

                        nu_vp->elements[ level ] = nu;

                        if (nu < 5.0 * DBL_MIN)
                        {
                            log_nu_vp->elements[ level ] = LOG_ZERO;
                        }
                        else
                        {
                            log_nu_vp->elements[ level ] = log(nu);
                        }
                    }

#ifdef TEST
                    ASSERT_IS_NUMBER_DBL(sum_nu);
                    ASSERT_IS_FINITE_DBL(sum_nu);
                    ASSERT_IS_LESS_DBL(sum_nu , 1.0001);
                    ASSERT_IS_GREATER_DBL(sum_nu, 0.9999);
#endif

#ifdef REPORT_CPU_TIME
                    vertical_weight_caching_cpu += (get_cpu_time() - cpu_time_offset);
                    cpu_time_offset = get_cpu_time();
#endif

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (item = 0; item < num_items; item++)
                    {
                        double max_log_prob = DBL_MOST_NEGATIVE;
                        double prob_sum = 0.0;

                        con_prob_cache_vvp = con_prob_cache_vvvp->elements[ item ];

                        for (level = first_level; level < last_level_num; level++)
                        {
                            double log_prob = log_nu_vp->elements[ level ];

                            node   = node_mp->elements[ cluster ][ level ];

                            log_prob += log_P_i_n_mvp->elements[ level ]->elements[ node ][ item ];
                            log_prob += con_prob_cache_vvp->elements[ level ]->elements[ node ];

                            level_indicators_ppp[ level ][ cluster_index ][ item ] = log_prob;

                            max_log_prob = MAX_OF(max_log_prob, log_prob);
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            double log_prob = level_indicators_ppp[ level ][ cluster_index ][ item ];

                            prob = exp(log_prob = max_log_prob);

                            level_indicators_ppp[ level ][ cluster_index ][ item ] = prob;

                            prob_sum += prob;
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_ppp[ level ][ cluster_index ][ item ] /= prob_sum;
                        }

#ifdef MIN_LEVEL_INDICATOR
                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];

                            level_indicators_pp[ cluster_index ][ item ] += delta_to_ensure_min_level_indicator;
                            level_indicators_pp[ cluster_index ][ item ] /= min_level_indicator_norm;
                        }
#endif

                        if (prob_sum < 5.0 * DBL_MIN)
                        {
                            warn_pso("Zero prob spotted. Making amends.\n");
                            dbe(prob_sum);
                            dbe(max_log_prob);

                            log_prob_sum = LOG_ZERO + max_log_prob;
                        }
                        else
                        {
                            log_prob_sum = log(prob_sum) + max_log_prob;
                        }

                        ASSERT_IS_NUMBER_DBL(log_prob_sum);
                        ASSERT_IS_FINITE_DBL(log_prob_sum);

                        log_product += log_prob_sum * con_item_weight;

                        ASSERT_IS_NUMBER_DBL(log_product);
                        ASSERT_IS_FINITE_DBL(log_product);

#ifdef SUPPORT_ANNEALING
    /* ANNEAL */        annealed_log_product += annealed_log_prob_sum;
    /* ANNEAL */
    /* ANNEAL */        ASSERT_IS_NUMBER_DBL(annealed_log_product);
    /* ANNEAL */        ASSERT_IS_FINITE_DBL(annealed_log_product);
#endif

                        if (num_items > 0)
                        {
                            ASSERT_IS_NUMBER_DBL(con_factor);
                            ASSERT_IS_FINITE_DBL(con_factor);
                            ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

                            /*
                            // The multiplication by con_factor can probably be moved
                            // outside the loop over clusters.
                            */
                            log_p += con_factor * log_product;
#ifdef SUPPORT_ANNEALING
        /* ANNEAL */        annealed_log_p += con_factor * annealed_log_product;
#endif
                        }
                    }

#ifdef REPORT_CPU_TIME
                    /*
                     * Con and dis are done together so split the cpu time
                     * between them.
                    */
                    con_item_prob_update_cpu += (get_cpu_time() - cpu_time_offset) / 2;
                    dis_item_prob_update_cpu += (get_cpu_time() - cpu_time_offset) / 2;
                    cpu_time_offset = get_cpu_time();
#endif

#ifdef TEST
#ifdef REPORT_CPU_TIME
        /* TEST */  cpu_time_offset = get_cpu_time();
#endif
        /* TEST */  for (l = 0; l< num_levels; l++)
        /* TEST */  {
        /* TEST */      level_indicators_pp = level_indicators_ppp[ l];
        /* TEST */
        /* TEST */      for (c = 0; c< num_clusters; c++)
        /* TEST */      {
        /* TEST */          level_indicators_p = level_indicators_pp[ c];
        /* TEST */
        /* TEST */          for (i= 0; i< max_num_items; i++)
        /* TEST */          {
                                if (    (*level_indicators_p < 0.0)
                                     && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
                                          || (*level_indicators_p < DBL_NOT_SET - 0.001)
                                        )
                                   )
                                {
                                    dbe(*level_indicators_p);
                                    dbi(l);
                                    dbi(c);
                                    dbi(i);

                                    SET_CANT_HAPPEN_BUG();

                                    result = ERROR;
                                    break;
                                }
        /* TEST */              level_indicators_p++;
        /* TEST */          }
        /* TEST */      }
        /* TEST */  }
        /* TEST */
        /* TEST */  /* Breaks shared correspondence */
        /* TEST */  /* result = ow_set_vector(P_c_p_vp, DBL_NOT_SET); */
#ifdef REPORT_CPU_TIME
        /* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif


                        log_p += log_a;
                        ASSERT_IS_NUMBER_DBL(log_p);
                        ASSERT_IS_FINITE_DBL(log_p);

                        ll_P_c_p_vp->elements[ cluster_index ] = log_p;

                        P_c_p_vp->elements[ cluster_index ] = log_p;

#ifdef REPORT_CPU_TIME
                    pair_item_prob_update_cpu += get_cpu_time() - cpu_time_offset;
                    cpu_time_offset = get_cpu_time();
#endif

                }
            }
#endif
            else if (options_ptr->model_correspondence == 5)
            {
                /*
                 * This is based on hard assignment of levels. We really need to
                 * sample to get an estimate of the expection.
                */
                Int_matrix* concept_for_pair_probs_mp = NULL;
                int block;
                int num_blocks = (last_level_num - first_level) / fs_discrete_tie_count;
                Matrix_vector* con_item_merge_prior_mvp = data_ptr->con_item_merge_prior_mvp;
                Matrix* con_item_merge_prior_mp = (con_item_merge_prior_mvp == NULL) ? NULL :  con_item_merge_prior_mvp->elements[ point ];

                log_p = 0.0;

                if (num_clusters > 1)
                {
                    set_error("Correspondence model 5 does not support clusters.");
                    result = ERROR;
                    NOTE_ERROR();
                    break;
                }

                /*
                // Cache the level weights, taking logs if we are using logs for
                // probs.
                */
                for (level = first_level; level < last_level_num; level++)
                {
                    if (valid_cluster_dependent_aspects)
                    {
                        ASSERT_IS_LESS_INT(level , P_l_p_c_mvp->length);
                        ASSERT_IS_LESS_INT(point , P_l_p_c_mvp->elements[ level ]->num_rows);

                        nu = P_l_p_c_mvp->elements[ level ]->elements[ point ][ 0 ];

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);
                    }
                    else if (valid_aspects)
                    {
                        ASSERT_IS_LESS_INT(level , P_l_p_mp->num_rows);
                        ASSERT_IS_LESS_INT(point , P_l_p_mp->num_cols);

                        nu = P_l_p_mp->elements[ level ][ point ];

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);
                    }
                    else
                    {
                        nu = V_mp->elements[ level ][ 0 ];

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);
                    }
#ifdef TEST
                    sum_nu += nu;
#endif

                    ASSERT_IS_NUMBER_DBL(nu);
                    ASSERT_IS_FINITE_DBL(nu);
                    ASSERT_IS_NON_NEGATIVE_DBL(nu);

                    nu_vp->elements[ level ] = nu;

                    if (nu < 5.0 * DBL_MIN)
                    {
                        log_nu_vp->elements[ level ] = LOG_ZERO;
                    }
                    else
                    {
                        log_nu_vp->elements[ level ] = log(nu);
                    }

                    ASSERT_IS_NUMBER_DBL(log_nu_vp->elements[ level ]);
                    ASSERT_IS_FINITE_DBL(log_nu_vp->elements[ level ]);
                }

#ifdef TEST
                for (level = first_level; level < last_level_num; level++)
                {
                    level_indicators_p = level_indicators_ppp[ level ][ 0 ];

                    for (dis_item = 0; dis_item < max_num_dis_items; dis_item++)
                    {
                        level_indicators_p[ dis_item ] = DBL_NOT_SET;
                    }
                    for (con_item = 0; con_item < max_num_con_items; con_item++)
                    {
                        level_indicators_p[ max_num_dis_items + con_item ] = DBL_NOT_SET;
                    }
                }
#endif
                for (level = first_level; level < last_level_num; level++)
                {
                    level_indicators_p = level_indicators_ppp[ level ][ 0 ];

                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        level_indicators_p[ dis_item ] = 0.0;
                    }
                    for (con_item = 0; con_item < num_con_items; con_item++)
                    {
                        level_indicators_p[ max_num_dis_items + con_item ] = 0.0;
                    }
                }

                result = get_target_int_matrix(&concept_for_pair_probs_mp,
                                               num_con_items, num_dis_items);
                if (result == ERROR) { NOTE_ERROR(); break; }

                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];

                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        int block_for_max_prob = NOT_SET;
                        double max_block_log_prob = DBL_MOST_NEGATIVE;

                        category = dis_item_mp->elements[ point ][ dis_item ];

                        for (block = 0; block < num_blocks; block++)
                        {
                            max_log_prob = DBL_MOST_NEGATIVE;

                            for (i = 0; i < fs_discrete_tie_count; i++)
                            {
                                level = first_level + block * fs_discrete_tie_count + i;

                                log_prob = con_prob_cache_vvp->elements[ level ]->elements[ 0 ];
                                ASSERT_IS_NUMBER_DBL(log_prob);
                                ASSERT_IS_FINITE_DBL(log_prob);

                                log_prob_vp->elements[ level ] = log_prob;
                                max_log_prob = MAX_OF(max_log_prob, log_prob);
                           }

                            ASSERT_IS_NUMBER_DBL(max_log_prob);
                            ASSERT_IS_FINITE_DBL(max_log_prob);

                            prob_sum = 0;

                            for (i = 0; i < fs_discrete_tie_count; i++)
                            {
                                level = first_level + block * fs_discrete_tie_count + i;

                                prob = exp(log_prob_vp->elements[ level ] - max_log_prob);
                                ASSERT_IS_NUMBER_DBL(prob);
                                ASSERT_IS_FINITE_DBL(prob);
                                prob_sum += prob;
                                ASSERT_IS_NUMBER_DBL(prob_sum);
                                ASSERT_IS_FINITE_DBL(prob_sum);
                            }

                            log_prob = SAFE_LOG(prob_sum) + max_log_prob;
                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

                            log_prob += log_P_i_n_mvp->elements[ level ]->elements[ 0 ][ category ];
                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

                            log_prob += log_nu_vp->elements[ level ];
                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

                            if (log_prob > max_block_log_prob)
                            {
                                max_block_log_prob = log_prob;
                                block_for_max_prob = block;
                            }
                            ASSERT_IS_NUMBER_DBL(max_block_log_prob);
                            ASSERT_IS_FINITE_DBL(max_block_log_prob);
                        }

                        ASSERT_IS_NON_NEGATIVE_INT(block_for_max_prob);
                        ASSERT_IS_NUMBER_DBL(max_block_log_prob);
                        ASSERT_IS_FINITE_DBL(max_block_log_prob);

                        log_pair_probs_mp->elements[ con_item ][ dis_item ] = max_block_log_prob;
                        concept_for_pair_probs_mp->elements[ con_item ][ dis_item ] = block_for_max_prob;
                    }
                }

                log_pair_probs_mp->num_rows = num_con_items;
                log_pair_probs_mp->num_cols = num_dis_items;

                verify_matrix(log_pair_probs_mp, NULL);

                result = get_matches(&match_mp, &num_matches, log_pair_probs_mp,
                                     FALSE,   /* No sampling. */
                                     0,       /* Forget NULLS. */
                                     FALSE);  /* Don't duplicate words. */
                if (result == ERROR) { NOTE_ERROR(); break; }

                ASSERT(num_matches == MIN_OF(num_dis_items, num_con_items));

#ifdef REPORT_CPU_TIME
                pair_match_cpu += (get_cpu_time() - cpu_time_offset);
                cpu_time_offset = get_cpu_time();
#endif

                if (result == ERROR) { NOTE_ERROR(); break; }

                if ((! fs_one_to_one_match) && (num_con_items > num_dis_items))
                {
                    Int_vector* dis_item_for_con_item_vp = NULL;

                    result = get_initialized_int_vector(&dis_item_for_con_item_vp,
                                                        num_con_items, NOT_SET);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    /*
                     * Cache dis item assignments and rebuild concept assignment
                     * table based on the constrained that the discrete items
                     * drive the concepts.
                    */
                    for (match = 0; match < num_matches; match++)
                    {
                        con_item = match_mp->elements[ match ][ 0 ];
                        dis_item = match_mp->elements[ match ][ 1 ];
                        block = concept_for_pair_probs_mp->elements[ con_item ][ dis_item ];

                        ASSERT(dis_item_for_con_item_vp->elements[ con_item ] < 0);

                        dis_item_for_con_item_vp->elements[ con_item ] = dis_item;

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            concept_for_pair_probs_mp->elements[ con_item ][ dis_item ] = block;
                        }
                    }

                    /*
                     * Rebuild log_pair_probs_mp with dis item based levels
                     * found for max assign.
                    */
                    for (con_item = 0; con_item < num_con_items; con_item++)
                    {
                        if (KJB_IS_SET(dis_item_for_con_item_vp->elements[ con_item ]))
                        {
                            continue;
                        }

                        con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];

                        for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                        {
                            max_log_prob = DBL_MOST_NEGATIVE;

                            category = dis_item_mp->elements[ point ][ dis_item ];
                            block = concept_for_pair_probs_mp->elements[ con_item ][ dis_item ];

                            for (i = 0; i < fs_discrete_tie_count; i++)
                            {
                                level = first_level + block * fs_discrete_tie_count + i;

                                log_prob = con_prob_cache_vvp->elements[ level ]->elements[ 0 ];
                                ASSERT_IS_NUMBER_DBL(log_prob);
                                ASSERT_IS_FINITE_DBL(log_prob);

                                log_prob_vp->elements[ level ] = log_prob;
                                max_log_prob = MAX_OF(max_log_prob, log_prob);
                            }

                            ASSERT_IS_NUMBER_DBL(max_log_prob);
                            ASSERT_IS_FINITE_DBL(max_log_prob);

                            prob_sum = 0;

                            for (i = 0; i < fs_discrete_tie_count; i++)
                            {
                                level = first_level + block * fs_discrete_tie_count + i;

                                prob = exp(log_prob_vp->elements[ level ] - max_log_prob);
                                ASSERT_IS_NUMBER_DBL(prob);
                                ASSERT_IS_FINITE_DBL(prob);
                                prob_sum += prob;
                                ASSERT_IS_NUMBER_DBL(prob_sum);
                                ASSERT_IS_FINITE_DBL(prob_sum);
                            }

                            log_prob = SAFE_LOG(prob_sum) + max_log_prob;
                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

                            log_prob += log_P_i_n_mvp->elements[ level ]->elements[ 0 ][ category ];
                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

                            log_prob += log_nu_vp->elements[ level ];
                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

                            log_pair_probs_mp->elements[ con_item ][ dis_item ] = log_prob;
                        }
                    }

                    /*
                     * Find best match for left over con items using greedy and
                     * contiguity prior. Originally contiguity was forced, and
                     * it is easy to switch back.
                    */
                    for (i = 0; i < (num_con_items - num_dis_items); i++)
                    {
                        int best_dis_item = NOT_SET;
                        int best_con_item = NOT_SET;
                        int enforce_contiguous = (fs_use_contiguous_assignment && con_item_merge_prior_mp != NULL);
                        int num_tries = enforce_contiguous ? 2 : 1;
                        int try_count = 0;
                        double contig_factor = 1.0;

                        max_log_prob = DBL_MOST_NEGATIVE;

                        if (enforce_contiguous)
                        {
                            static int first_time = TRUE;

                            if (first_time)
                            {
                                first_time = FALSE;
                                dbp("Using contiguity.\n");
                            }

                            contig_factor = MM_CONTIGUOUS_PRIOR;
                        }

                        while ((try_count < num_tries) && (best_dis_item == NOT_SET))
                        {
                            for (con_item = 0; con_item < num_con_items; con_item++)
                            {
                                if (KJB_IS_SET(dis_item_for_con_item_vp->elements[ con_item ]))
                                {
                                    continue;
                                }

                                for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                                {
                                    if (contig_factor * log_pair_probs_mp->elements[ con_item ][ dis_item ] > max_log_prob)
                                    {
                                        int valid_match = TRUE;

                                        if (enforce_contiguous && (try_count == 0))
                                        {
                                            int con_item_2;

                                            valid_match = FALSE;

                                            for (con_item_2 = 0; con_item_2 < num_con_items; con_item_2++)
                                            {
                                                if (    (con_item_merge_prior_mp->elements[ con_item ][ con_item_2 ])
                                                     && (dis_item_for_con_item_vp->elements[ con_item_2 ] == dis_item)
                                                   )
                                                {
                                                    valid_match = TRUE;
                                                    break;
                                                }
                                            }
                                        }

                                        if ((valid_match) && (contig_factor * log_pair_probs_mp->elements[ con_item ][ dis_item ] > max_log_prob))
                                        {
                                            max_log_prob = contig_factor * log_pair_probs_mp->elements[ con_item ][ dis_item ];
                                            best_dis_item = dis_item;
                                            best_con_item = con_item;
                                        }
                                        else if (log_pair_probs_mp->elements[ con_item ][ dis_item ] > max_log_prob)
                                        {
                                            max_log_prob = log_pair_probs_mp->elements[ con_item ][ dis_item ];
                                            best_dis_item = dis_item;
                                            best_con_item = con_item;
                                        }
                                    }
                                }
                            }

                            try_count++;
                        }

                        ASSERT_IS_GREATER_DBL(max_log_prob, DBL_HALF_MOST_NEGATIVE);
                        ASSERT_IS_NOT_LESS_INT(best_con_item, 0);
                        ASSERT_IS_NOT_LESS_INT(best_dis_item, 0);

                        match_mp->elements[ num_matches ][ 0 ] = best_con_item;
                        match_mp->elements[ num_matches ][ 1 ] = best_dis_item;
                        dis_item_for_con_item_vp->elements[ best_con_item ] = best_dis_item;

                        num_matches++;
                    }

                    free_int_vector(dis_item_for_con_item_vp);

                    ASSERT(num_matches == num_con_items);
                }

                /*
                 * Now compute indicators and log likelihood.
                */
                for (match = 0; match < num_matches; match++)
                {
                    con_item = match_mp->elements[ match ][ 0 ];
                    dis_item = match_mp->elements[ match ][ 1 ];
                    block = concept_for_pair_probs_mp->elements[ con_item ][ dis_item ];

                    con_prob_cache_vvp = con_prob_cache_vvvp->elements[ con_item ];

                    max_log_prob = DBL_MOST_NEGATIVE;

                    log_prob_sum = 0;

                    for (i = 0; i < fs_discrete_tie_count; i++)
                    {
                        level = first_level + block * fs_discrete_tie_count + i;

                        log_prob = log_nu_vp->elements[ level ];
                        log_prob += con_prob_cache_vvp->elements[ level ]->elements[ 0 ];
                        max_log_prob = MAX_OF(max_log_prob, log_prob);
                        log_prob_vp->elements[ level ] = log_prob;
                    }

                    prob_sum = 0;

                    for (i = 0; i < fs_discrete_tie_count; i++)
                    {
                        level = first_level + block * fs_discrete_tie_count + i;

                        prob = exp(log_prob_vp->elements[ level ] - max_log_prob);
                        prob_sum += prob;
                        prob_vp->elements[ level ] = prob;
                    }

                    for (i = 0; i < fs_discrete_tie_count; i++)
                    {
                        level = first_level + block * fs_discrete_tie_count + i;

                        level_indicators_p = level_indicators_ppp[ level ][ 0 ];

                        ASSERT_IS_EQUAL_DBL(0.0, level_indicators_p[ max_num_dis_items + con_item ]);
                        level_indicators_p[ max_num_dis_items + con_item ] = prob_vp->elements[ level ] / prob_sum;

                        level_indicators_p[ dis_item ] += 1.0;
                    }

                    log_p += log_pair_probs_mp->elements[ con_item ][ dis_item ];

                }

                for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                {
                    prob_sum = 0.0;

                    for (level = first_level; level < last_level_num; level++)
                    {
                        level_indicators_p = level_indicators_ppp[ level ][ 0 ];
                        prob_sum += level_indicators_p[ dis_item ];
                    }

                    if (prob_sum < 0.5)
                    {
                        ASSERT((num_dis_items > num_con_items) || ((fs_one_to_one_match) && (num_dis_items < num_con_items)));
                        continue;
                    }

                    for (level = first_level; level < last_level_num; level++)
                    {
                        level_indicators_p = level_indicators_ppp[ level ][ 0 ];
                        level_indicators_p[ dis_item ] /= prob_sum;
                    }
                }

#ifdef MIN_LEVEL_INDICATOR
                for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                {
                    for (level = first_level; level < last_level_num; level++)
                    {
                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ 0 ] + dis_item;

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);

                        *level_indicators_p += delta_to_ensure_min_level_indicator;
                        *level_indicators_p /= min_level_indicator_norm;

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);
                    }
                }
                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    for (level = first_level; level < last_level_num; level++)
                    {
                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ 0 ] + max_num_dis_items + con_item;

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);

                        *level_indicators_p += delta_to_ensure_min_level_indicator;
                        *level_indicators_p /= min_level_indicator_norm;

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);
                    }
                }
#endif

#ifdef TEST
    /* TEST */
    /* TEST */  for (level = first_level; level < last_level_num; level++)
    /* TEST */  {
                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ 0 ];
    /* TEST */
    /* TEST */          for (dis_item = 0; dis_item < num_dis_items; dis_item++)
    /* TEST */          {
    /* TEST */              ASSERT_IS_FINITE_DBL(level_indicators_p[ dis_item ]);
    /* TEST */              ASSERT_IS_NUMBER_DBL(level_indicators_p[ dis_item ]);
    /* TEST */              ASSERT_IS_PROB_DBL(level_indicators_p[ dis_item ]);
    /* TEST */          }
    /* TEST */          for (dis_item = num_dis_items; dis_item < max_num_dis_items; dis_item++)
    /* TEST */          {
    /* TEST */              ASSERT_IS_FINITE_DBL(level_indicators_p[ dis_item ]);
    /* TEST */              ASSERT_IS_NUMBER_DBL(level_indicators_p[ dis_item ]);
    /* TEST */              ASSERT_IS_EQUAL_DBL(level_indicators_p[ dis_item ],
    /* TEST */                                  DBL_NOT_SET);
    /* TEST */          }
    /* TEST */          for (con_item = 0; con_item < num_con_items; con_item++)
    /* TEST */          {
    /* TEST */              ASSERT_IS_FINITE_DBL(level_indicators_p[ max_num_dis_items + con_item ]);
    /* TEST */              ASSERT_IS_NUMBER_DBL(level_indicators_p[ max_num_dis_items + con_item ]);
    /* TEST */              ASSERT_IS_PROB_DBL(level_indicators_p[ max_num_dis_items + con_item ]);
    /* TEST */          }
    /* TEST */          for (con_item = num_con_items; con_item < max_num_con_items; con_item++)
    /* TEST */          {
    /* TEST */              ASSERT_IS_FINITE_DBL(level_indicators_p[ max_num_dis_items + con_item ]);
    /* TEST */              ASSERT_IS_NUMBER_DBL(level_indicators_p[ max_num_dis_items + con_item ]);
    /* TEST */              ASSERT_IS_EQUAL_DBL(level_indicators_p[ max_num_dis_items + con_item ],
    /* TEST */                                  DBL_NOT_SET);
    /* TEST */          }
    /* TEST */  }
#ifdef REPORT_CPU_TIME
                test_extra_cpu += (get_cpu_time() - cpu_time_offset);
                cpu_time_offset = get_cpu_time();
#endif
#endif

                ASSERT_IS_NUMBER_DBL(log_p);
                ASSERT_IS_FINITE_DBL(log_p);

                log_p /= num_matches;

                ASSERT_IS_NUMBER_DBL(log_p);
                ASSERT_IS_FINITE_DBL(log_p);

                ll_P_c_p_vp->elements[ 0 ] = log_p;

                P_c_p_vp->elements[ 0 ] = log_p;

#ifdef REPORT_CPU_TIME
                pair_item_prob_update_cpu += get_cpu_time() - cpu_time_offset;
                cpu_time_offset = get_cpu_time();
#endif
                free_int_matrix(concept_for_pair_probs_mp);
            }
            else
            {
                SET_CANT_HAPPEN_BUG();
                result = ERROR;
                break;
            }

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif

            /*
            // I'm not sure if this is where we want the correspondence and
            // non-correspondence cases to merge.
            */

            /* --------------------------------------------------------- */

#ifdef XXX_SUPPORT_TRIMMING
            /*
            // Scores for trimming.
            */
            if (    (options_ptr->trim_fraction > 0.0)
#ifndef XXX_OLD_PAIR_NULL_METHOD
                 && (options_ptr->model_correspondence < 2)
#endif
               )
            {
                sum_collected_scores(num_dis_items, dis_item_count,
                                     dis_score_counts_vp,
                                     dis_score_mp, new_dis_score_vp);

                sum_collected_scores(num_con_items, con_item_count,
                                     con_score_counts_vp,
                                     con_score_mp, new_con_score_vp);
            }
#endif

#ifdef XXX_SUPPORT_TRIMMING
            if (    (current_model_is_valid)
                 && (run == 0)
                 && (num_runs == 2)
               )
            {
                /*
                // HACK
                //
                // We skip this update which must occur at specific places in
                // run 2, so we need to do it here.
                */
                dis_item_count += num_dis_items;
                con_item_count += num_con_items;

                continue;
            }
#endif

#ifdef REPORT_CPU_TIME
            cpu_time_offset = get_cpu_time();
#endif
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (num_con_items > 0)
            {
                ASSERT_IS_NUMBER_DBL(con_factor);
                ASSERT_IS_FINITE_DBL(con_factor);
                ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

#ifdef IT_WAS_MORE_LIKE_THIS_BEFORE_REWRITE
                ll_con_correction_norm_sum += (double)num_con_items * con_factor;
#else
                if (options_ptr->model_correspondence < 2)
                {
                    ll_con_correction_norm_sum += (double)num_con_items * con_factor;
                }
                else
                {
                    /*
                    // This adjustment depends on the normalization for the
                    // number of matches, and naive handling of multiplicity.
                    */
                    ll_con_correction_norm_sum += 1.0;
                }
#endif
            }


            max_log_prob = DBL_MOST_NEGATIVE;
            sum = 0.0;

            for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
            {
#ifdef XXX_SUPPORT_TRIMMING
                if (fs_trim_mach_two)
                {
                    if (ll_P_c_p_vp->elements[ cluster_index ] > DBL_MOST_NEGATIVE /2.0)
                    {
                        max_log_prob = MAX_OF(max_log_prob, ll_P_c_p_vp->elements[ cluster_index ]);
                    }
                }
                else
                {
#endif
                    max_log_prob = MAX_OF(max_log_prob, ll_P_c_p_vp->elements[ cluster_index ]);
#ifdef XXX_SUPPORT_TRIMMING
                }
#endif
            }

            for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
            {
#ifdef XXX_SUPPORT_TRIMMING
                if (fs_trim_mach_two)
                {
                    if (ll_P_c_p_vp->elements[ cluster_index ] > DBL_MOST_NEGATIVE /2.0)
                    {
                        temp = ll_P_c_p_vp->elements[ cluster_index ] - max_log_prob;
                        sum += exp(temp);
                    }
                }
                else
                {
#endif
                    temp = ll_P_c_p_vp->elements[ cluster_index ] - max_log_prob;
                    sum += exp(temp);
#ifdef XXX_SUPPORT_TRIMMING
                }
#endif

            }

            if (sum < 5.0 * DBL_MIN)
            {
#ifdef XXX_SUPPORT_TRIMMING
                if (fs_trim_mach_two)
                {
                    fit = 0.0;
                }
                else
                {
#endif
                    dbw();
                    fit = LOG_ZERO + max_log_prob;
                    num_good_points++;
#ifdef XXX_SUPPORT_TRIMMING
                }
#endif
            }
            else
            {
                fit = log(sum) + max_log_prob;
                num_good_points++;
            }

            log_likelihood += fit * point_weight;

            /*
            // CHECK
            //
            // Perhaps we want the point_weight in the fit vector?
            */
            fit_vp->elements[ point ] = fit;

            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
/* CPU  */  log_likelihood_cpu += (get_cpu_time() - cpu_time_offset);
/* CPU  */  cpu_time_offset = get_cpu_time();
#endif

            max_log_prob = DBL_MOST_NEGATIVE;

            for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
            {
#ifdef XXX_SUPPORT_TRIMMING
                if (fs_trim_mach_two)
                {
                    if (P_c_p_vp->elements[ cluster_index ] > DBL_MOST_NEGATIVE /2.0)
                    {
                        max_log_prob = MAX_OF(max_log_prob, P_c_p_vp->elements[ cluster_index ]);
                    }
                }
                else
                {
#endif
                    max_log_prob = MAX_OF(max_log_prob, P_c_p_vp->elements[ cluster_index ]);
#ifdef XXX_SUPPORT_TRIMMING
                }
#endif
            }

            for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
            {
#ifdef XXX_SUPPORT_TRIMMING
                if (fs_trim_mach_two)
                {
                    if (P_c_p_vp->elements[ cluster_index ] > DBL_MOST_NEGATIVE /2.0)
                    {
                         P_c_p_vp->elements[ cluster_index ] -= max_log_prob;
                    }
                }
                else
                {
#endif
                    P_c_p_vp->elements[ cluster_index ] -= max_log_prob;

#ifdef TEST
/* TEST */          ASSERT_IS_LESS_DBL(P_c_p_vp->elements[ cluster_index ], 700.0);
#endif
#ifdef XXX_SUPPORT_TRIMMING
                }
#endif

            }

            for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
            {
#ifdef XXX_SUPPORT_TRIMMING
                if (fs_trim_mach_two)
                {
                    if (P_c_p_vp->elements[ cluster_index ] > DBL_MOST_NEGATIVE /2.0)
                    {
                        P_c_p_vp->elements[ cluster_index ] = exp(P_c_p_vp->elements[ cluster_index ]);
                    }
                    else
                    {
#ifdef HOW_IT_WAS_NOON_JULY_1
                        P_c_p_vp->elements[ cluster_index ] = 1000.0 * DBL_MIN;
#else
                        cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];
                        P_c_p_vp->elements[ cluster_index ] = a_vp->elements[ cluster ];
#endif
                    }
                }
                else
                {
#endif
                    P_c_p_vp->elements[ cluster_index ] = exp(P_c_p_vp->elements[ cluster_index ]);
#ifdef XXX_SUPPORT_TRIMMING
                }
#endif
            }

#ifdef TEST
/* TEST */  for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
/* TEST */  {
/* TEST */      ASSERT_IS_NUMBER_DBL(P_c_p_vp->elements[ cluster_index ]);
/* TEST */      ASSERT_IS_NON_NEGATIVE_DBL(P_c_p_vp->elements[ cluster_index ]);
/* TEST */  }
/* TEST */
/* TEST */  sum = 0.0;
/* TEST */
/* TEST */  for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
/* TEST */  {
/* TEST */      sum += P_c_p_vp->elements[ cluster_index ];
/* TEST */  }
/* TEST */
/* TEST */  ASSERT_IS_NUMBER_DBL(sum);
/* TEST */  ASSERT_IS_NON_NEGATIVE_DBL(sum);
/* TEST */  ASSERT_IS_FINITE_DBL(sum);
#endif

            result = ow_scale_vector_by_sum_2(P_c_p_vp, (double*)NULL);
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (fs_sample_count > 0)
            {
                int count;
                Vector* cum_P_c_p_vp = NULL;
                Vector* new_P_c_p_vp = NULL;


                if (it < 5)
                {
                    for (i = 0; i < num_clusters; i++)
                    {
                        P_c_p_vp->elements[ i ] = pow(P_c_p_vp->elements[ i ],
                                                      1.0 / (6.0 - it));
                    }
                    result = ow_scale_vector_by_sum_2(P_c_p_vp, (double*)NULL);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }

                ERE(get_zero_vector(&cum_P_c_p_vp, num_clusters));
                ERE(get_zero_vector(&new_P_c_p_vp, num_clusters));

                cum_P_c_p_vp->elements[ 0 ] = P_c_p_vp->elements[ 0 ];

                for (i = 1; i < num_clusters; i++)
                {
                    cum_P_c_p_vp->elements[ i ] =
                        cum_P_c_p_vp->elements[ i - 1 ] + P_c_p_vp->elements[ i ];
                }

                ASSERT_IS_NEARLY_EQUAL_DBL(cum_P_c_p_vp->elements[ num_clusters - 1 ], 1.0,
                                           0.0001 / ((double)num_clusters));

                /* Force to 1.0 to deal with round off error. */
                cum_P_c_p_vp->elements[ num_clusters - 1 ] = 1.0;

                for (count = 0; count < fs_sample_count; count++)
                {
                    double r = kjb_rand();

                    for (i = 0; i < num_clusters; i++)
                    {
                        if (r <= cum_P_c_p_vp->elements[ i ])
                        {
                            new_P_c_p_vp->elements[ i ] += 1.0;
                            break;
                        }
                    }
                }

                result = copy_vector(&P_c_p_vp, new_P_c_p_vp);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_scale_vector_by_sum_2(P_c_p_vp, (double*)NULL);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }

#ifdef TEST
/* TEST */  for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
/* TEST */  {
/* TEST */      ASSERT_IS_NUMBER_DBL(P_c_p_vp->elements[ cluster_index ]);
/* TEST */      ASSERT_IS_NON_NEGATIVE_DBL(P_c_p_vp->elements[ cluster_index ]);
/* TEST */  }
#endif

#ifdef TEST
#ifdef REPORT_CPU_TIME
/* TEST */  cpu_time_offset = get_cpu_time();
#endif
/* TEST */  for (l = 0; l< num_levels; l++)
/* TEST */  {
/* TEST */      level_indicators_pp = level_indicators_ppp[ l];
/* TEST */
/* TEST */      for (c = 0; c< num_clusters; c++)
/* TEST */      {
/* TEST */          level_indicators_p = level_indicators_pp[ c];
/* TEST */
/* TEST */          for (i= 0; i< max_num_items; i++)
/* TEST */          {
/* TEST */              if (    (*level_indicators_p < 0.0)
/* TEST */                   && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
/* TEST */                    || (*level_indicators_p < DBL_NOT_SET - 0.001)
/* TEST */                      )
/* TEST */                 )
/* TEST */              {
/* TEST */                  dbe(*level_indicators_p);
/* TEST */                  dbi(l);
/* TEST */                  dbi(c);
/* TEST */                  dbi(i);
/* TEST */
/* TEST */                  SET_CANT_HAPPEN_BUG();
/* TEST */
/* TEST */                  result = ERROR;
/* TEST */                  break;
/* TEST */              }
/* TEST */              level_indicators_p++;
/* TEST */          }
/* TEST */      }
/* TEST */  }
/* TEST */
#ifdef REPORT_CPU_TIME
/* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif

            /*
             * If the initial model is a subset to be expanded, then having done
             * one iteration of the E-step to fit the indicators, we go back and
             * do the initialization.
             *
             * Once we have done the logical thing and have made the
             * E-step a sub-routine, this can be simplified. It is also not
             * clear how useful this subset business is, but lets leave it for a
             * bit longer.
            */
            if (    (it == 0)
                 && (options_ptr->initial_model_is_subset)
                 && ( ! initialize_remaining_nodes )
               )
            {
                initialize_remaining_nodes = TRUE;
                goto once_more;
            }

v_step:

#ifdef SUPPORT_NUM_CLUSTER_SWITCH     /* Not finished. Should it be? */
            if (    (result != ERROR)
                 && (it == 3)   /* HERE */
                 && ( ! fs_use_specified_clusters)
                 && (last_level_num == (num_levels - 1))
                 && (num_levels >= 2)
                 && (level_counts_vp->elements[ num_levels - 2 ] == 1)
               )
            {
                dbp("************************************************************");
                dbp("************************************************************");
                dbp("************************************************************");

                num_clusters = num_real_clusters;
                num_clusters_used = num_real_clusters;
                num_limited_clusters = num_real_clusters;

                result = get_random_vector(&P_c_p_vp, num_real_clusters);
                if (result == ERROR) { NOTE_ERROR(); break; }

                for (l = 0; l< num_levels; l++)
                {
                    level_indicators_pp = level_indicators_ppp[ l];

                    for (c = 1; c< num_clusters; c++)
                    {
                        level_indicators_p = level_indicators_pp[ c];

                        for (i= 0; i< max_num_items; i++)
                        {
                            level_indicators_p[ i ] = level_indicators_pp[ 0 ][ i ];
                        }
                    }
                }

                dbp("************************************************************");
                dbp("************************************************************");
                dbp("************************************************************");
            }
#endif

            verify_vector(P_c_p_vp, NULL);


#ifdef MIN_HORIZONTAL_INDICATOR
            result = ow_add_scalar_to_vector(P_c_p_vp,
                                             MIN_HORIZONTAL_INDICATOR);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = ow_scale_vector_by_sum_2(P_c_p_vp, (double*)NULL);
            if (result == ERROR) { NOTE_ERROR(); break; }
#endif

            verify_vector(P_c_p_vp, NULL);
/*
            Not valid for testing switch of num clusters.

            ASSERT_IS_EQUAL_INT(num_clusters_used, P_c_p_vp->length);
*/

#ifdef TEST
/* TEST */  for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
/* TEST */  {
/* TEST */      ASSERT_IS_NUMBER_DBL(P_c_p_vp->elements[ cluster_index ]);
/* TEST */      ASSERT_IS_FINITE_DBL(P_c_p_vp->elements[ cluster_index ]);
/* TEST */      ASSERT_IS_PROB_DBL(P_c_p_vp->elements[ cluster_index ]);
/* TEST */  }
#endif

            result = put_matrix_row(P_c_p_mp, P_c_p_vp, point);
            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
/* CPU  */  cluster_membership_cpu += (get_cpu_time() - cpu_time_offset);
/* CPU  */  cpu_time_offset = get_cpu_time();
#endif

            /*
            // Compute the q's in Hofmann's paper which are the probability that
            // a specified node/cluster combination explains an observation.
            */

            for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
            {
                if (    (fs_use_specified_clusters)
                     && (data_ptr->group_vp != NULL)
                   )
                {
                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];
                    if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    ASSERT_IS_GREATER_DBL(P_c_p_vp->elements[ cluster_index ], 0.99999);
                    ASSERT_IS_LESS_DBL(P_c_p_vp->elements[ cluster_index ], 1.00001);
                }

                p = P_c_p_vp->elements[ cluster_index ];

                ASSERT_IS_NUMBER_DBL(p);
                ASSERT_IS_FINITE_DBL(p);
                ASSERT_IS_NON_NEGATIVE_DBL(p);
                ASSERT_IS_PROB_DBL(p);
#ifdef MIN_HORIZONTAL_INDICATOR
                ASSERT_IS_NOT_LESS_DBL(p, MIN_HORIZONTAL_INDICATOR / 2.0);
#endif

                for (level = first_level; level < last_level_num; level++)
                {
                    level_indicators_pp = level_indicators_ppp[ level ];
                    level_indicators_p = level_indicators_pp[ cluster_index ];

                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
#ifdef TEST
                        category = dis_item_mp->elements[ point ][ dis_item ];
                        ASSERT_IS_LESS_INT(category , num_categories);
                        ASSERT_IS_NON_NEGATIVE_INT(category);
#endif

#ifdef LOOKS_BUGGY
                        /* Looks a bit BUGGY? Why no state change before
                        // continue? */
                        /* How it was before mods for NULL. */
                        if (*level_indicators_p < DBL_NOT_SET / 2.0)
                        {
                            ASSERT_IS_LESS_DBL(*level_indicators_p, DBL_NOT_SET + 000.1);
                            ASSERT_IS_GREATER_DBL(*level_indicators_p, DBL_NOT_SET - 000.1);
                            continue;
                        }
#else
                        if (*level_indicators_p < 0.0)
                        {
                            if (options_ptr->model_correspondence < 2)
                            {
                                SET_CANT_HAPPEN_BUG();
                                result = ERROR;
                                break;
                            }

                            ASSERT_IS_LESS_DBL(*level_indicators_p, DBL_NOT_SET + 000.1);
                            ASSERT_IS_GREATER_DBL(*level_indicators_p, DBL_NOT_SET - 000.1);

                            level_indicators_p++;
                            continue;
                        }
#endif

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);

                        *level_indicators_p *= p;

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);

                        level_indicators_p++;
                    }

                    level_indicators_p = level_indicators_pp[ cluster_index ];
                    level_indicators_p += max_num_dis_items;

                    for (con_item = 0; con_item < num_con_items; con_item++)
                    {
#ifdef LOOKS_BUGGY
                        /*
                        // Looks a bit BUGGY? Why no state change before
                        // continue? How it was before mods for NULL.
                        */
                        if (*level_indicators_p < DBL_NOT_SET / 2.0)
                        {
                            ASSERT_IS_LESS_DBL(*level_indicators_p, DBL_NOT_SET + 000.1);
                            ASSERT_IS_GREATER_DBL(*level_indicators_p, DBL_NOT_SET - 000.1);
                            continue;
                        }
#else
                        if (*level_indicators_p < 0.0)
                        {
                            if (options_ptr->model_correspondence < 2)
                            {
                                SET_CANT_HAPPEN_BUG();
                                result = ERROR;
                                break;
                            }

                            ASSERT_IS_LESS_DBL(*level_indicators_p, DBL_NOT_SET + 000.1);
                            ASSERT_IS_GREATER_DBL(*level_indicators_p, DBL_NOT_SET - 000.1);

                            level_indicators_p++;
                            continue;
                        }
#endif

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);
#ifdef MIN_LEVEL_INDICATOR
#ifdef TEST
                        if (! fs_use_specified_clusters)
                        {
                            ASSERT_IS_NOT_LESS_DBL(*level_indicators_p,
                                                  MIN_LEVEL_INDICATOR / 2.0);
                        }
#endif
#endif

                        *level_indicators_p *= p;

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);

#ifdef MIN_LEVEL_INDICATOR
#ifdef TEST
                        if (! fs_use_specified_clusters)
                        {
                            ASSERT_IS_NOT_LESS_DBL(*level_indicators_p,
                                                  MIN_HORIZONTAL_INDICATOR * MIN_LEVEL_INDICATOR / 4.0);
                        }
#endif
#endif


                        level_indicators_p++;
                    }
                }
            }


#ifdef REPORT_CPU_TIME
/* CPU  */  q_calculation_cpu += (get_cpu_time() - cpu_time_offset);
#endif

            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef TEST
#ifdef REPORT_CPU_TIME
/* TEST */  cpu_time_offset = get_cpu_time();
#endif
/* TEST */  for (l = 0; l< num_levels; l++)
/* TEST */  {
/* TEST */      level_indicators_pp = level_indicators_ppp[ l];
/* TEST */
/* TEST */      for (c = 0; c< num_clusters; c++)
/* TEST */      {
/* TEST */          level_indicators_p = level_indicators_pp[ c];
/* TEST */
/* TEST */          for (i= 0; i< max_num_items; i++)
/* TEST */          {
/* TEST */              if (    (*level_indicators_p < 0.0)
/* TEST */                   && (    (*level_indicators_p > DBL_NOT_SET + 0.001)
/* TEST */                    || (*level_indicators_p < DBL_NOT_SET - 0.001)
/* TEST */                      )
/* TEST */                 )
/* TEST */              {
/* TEST */                  dbe(*level_indicators_p);
/* TEST */                  dbi(l);
/* TEST */                  dbi(c);
/* TEST */                  dbi(i);
/* TEST */
/* TEST */                  SET_CANT_HAPPEN_BUG();
/* TEST */
/* TEST */                  result = ERROR;
/* TEST */                  break;
/* TEST */              }
/* TEST */              level_indicators_p++;
/* TEST */          }
/* TEST */      }
/* TEST */  }
/* TEST */
#ifdef REPORT_CPU_TIME
/* TEST */  test_extra_cpu += (get_cpu_time() - cpu_time_offset);
#endif
#endif

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif

            /*
            //
            //                           V Step
            */

            /*
            // Compute the normalizations of the vertical weights.
            */

#ifdef REPORT_CPU_TIME
/* CPU  */  cpu_time_offset = get_cpu_time();
#endif

            if (compute_cluster_dependent_aspects)
            {
                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {
                    cluster   = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];
                    level_sum = 0.0;

                    ASSERT_IS_NON_NEGATIVE_INT(cluster_index);
                    ASSERT_IS_LESS_INT(cluster_index , num_clusters);

                    if (    (fs_use_specified_clusters)
                         && (data_ptr->group_vp != NULL)
                       )
                    {
                        if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    }

                    for (level = first_level; level < last_level_num; level++)
                    {
                        double dis_sum = 0.0;
                        double con_sum = 0.0;

                        dis_item_ptr  = (dis_item_mp == NULL) ? NULL : dis_item_mp->elements[ point ];
                        dis_item_multiplicity_ptr = (dis_item_multiplicity_mp == NULL) ? NULL : dis_item_multiplicity_mp->elements[ point ];

                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ cluster_index ];

                        if (max_num_dis_items > 0)
                        {
                            q_ptr = level_indicators_p;

                            for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                            {
                                ASSERT_IS_NUMBER_DBL(dis_sum);
                                ASSERT_IS_FINITE_DBL(dis_sum);
                                ASSERT_IS_NON_NEGATIVE_DBL(dis_sum);

#ifdef TEST
                                category = *dis_item_ptr;
                                ASSERT_IS_EQUAL_INT(category, dis_item_mp->elements[ point ][ dis_item ]);
                                ASSERT_IS_NON_NEGATIVE_INT(category);
#endif
                                ASSERT_IS_EQUAL_INT(*dis_item_multiplicity_ptr, dis_item_multiplicity_mp->elements[ point ][ dis_item ]);

#ifdef LOOKS_BUGGY
                                /*
                                // Looks a bit BUGGY? Why no state change
                                // before continue? How it was before mods for
                                // NULL.
                                */
                                if (*q_ptr < DBL_NOT_SET / 2.0)
                                {
                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);
                                    continue;
                                }
#else
                                if (*q_ptr < 0.0)
                                {
                                    if (options_ptr->model_correspondence < 2)
                                    {
                                        SET_CANT_HAPPEN_BUG();
                                        result = ERROR;
                                        break;
                                    }

                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);

                                    q_ptr++;
                                    dis_item_ptr++;
                                    dis_item_multiplicity_ptr++;
                                    continue;
                                }
#endif


                                ASSERT_IS_NUMBER_DBL(*q_ptr);
                                ASSERT_IS_FINITE_DBL(*q_ptr);
                                ASSERT_IS_NON_NEGATIVE_DBL(*q_ptr);
                                ASSERT_IS_PROB_DBL(*q_ptr);

                                ASSERT_IS_NUMBER_DBL(dis_factor);
                                ASSERT_IS_FINITE_DBL(dis_factor);
                                ASSERT_IS_NON_NEGATIVE_DBL(dis_factor);
                                ASSERT_IS_GREATER_DBL(dis_factor, 1e-10);

                                ASSERT_IS_NUMBER_DBL(*dis_item_multiplicity_ptr);
                                ASSERT_IS_NON_NEGATIVE_DBL(*dis_item_multiplicity_ptr);

                                q = (*q_ptr) * dis_factor * (*dis_item_multiplicity_ptr);

                                ASSERT_IS_NUMBER_DBL(q);
                                ASSERT_IS_NON_NEGATIVE_DBL(q);

                                dis_sum += q;

                                ASSERT_IS_NUMBER_DBL(dis_sum);
                                ASSERT_IS_FINITE_DBL(dis_sum);
                                ASSERT_IS_NON_NEGATIVE_DBL(dis_sum);

                                q_ptr++;
                                dis_item_ptr++;
                                dis_item_multiplicity_ptr++;
                            }
                        }

                        level_indicators_p += max_num_dis_items;

                        if (num_con_items > 0)
                        {
                            q_ptr = level_indicators_p;

                            ASSERT_IS_NUMBER_DBL(con_factor);
                            ASSERT_IS_FINITE_DBL(con_factor);
                            ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

                            for (con_item = 0; con_item < num_con_items; con_item++)
                            {
                                ASSERT_IS_NUMBER_DBL(con_factor);
                                ASSERT_IS_FINITE_DBL(con_factor);
                                ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

#ifdef LOOKS_BUGGY
                                /*
                                // Looks a bit BUGGY? Why no state change
                                // before continue? How it was before mods for
                                // NULL.
                                */
                                if (*q_ptr < DBL_NOT_SET / 2.0)
                                {
                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);
                                    continue;
                                }
#else
                                if (*q_ptr < 0.0)
                                {
                                    if (options_ptr->model_correspondence < 2)
                                    {
                                        SET_CANT_HAPPEN_BUG();
                                        result = ERROR;
                                        break;
                                    }

                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);

                                    q_ptr++;
                                    continue;
                                }
#endif

                                ASSERT_IS_NUMBER_DBL(*q_ptr);
                                ASSERT_IS_PROB_DBL(*q_ptr);

                                q = (*q_ptr) * con_factor;

                                ASSERT_IS_NUMBER_DBL(q);
                                ASSERT_IS_NON_NEGATIVE_DBL(q);

                                con_sum += q;
                                q_ptr++;

                                ASSERT_IS_NUMBER_DBL(con_sum);
                                ASSERT_IS_FINITE_DBL(con_sum);
                                ASSERT_IS_NON_NEGATIVE_DBL(con_sum);
                            }
                        }

                        ASSERT_IS_NUMBER_DBL(con_sum);
                        ASSERT_IS_FINITE_DBL(con_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(con_sum);

                        /*
                        // Multiplying does not make much sense because there
                        // are then two nu's in the product!
                        */
                        if (fs_multiply_vertical_prior_weights)
                        {
                            sum = dis_sum * con_sum;
                        }
                        else
                        {
                            sum = dis_sum + con_sum;
                        }

                        ASSERT_IS_NUMBER_DBL(sum);
                        ASSERT_IS_FINITE_DBL(sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(sum);

                        P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ] = sum;
                        level_sum += sum;

                        ASSERT_IS_NUMBER_DBL(level_sum);
                        ASSERT_IS_FINITE_DBL(level_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(level_sum);

                        ASSERT_IS_NOT_LESS_INT(level, first_level);
                        ASSERT_IS_LESS_INT(level , last_level_num);
                        ASSERT_IS_LESS_INT(level , new_V_mp->num_rows);
                        ASSERT_IS_NON_NEGATIVE_INT(cluster);
                        ASSERT_IS_LESS_INT(cluster , num_clusters);
                        ASSERT_IS_LESS_INT(cluster , new_V_mp->num_cols);

                        new_V_mp->elements[ level ][ cluster ] += sum * point_weight;

                        if (new_dis_V_mp != NULL)
                        {
                            new_dis_V_mp->elements[ level ][ cluster ] += dis_sum * point_weight;
                        }

                        if (new_con_V_mp != NULL)
                        {
                            new_con_V_mp->elements[ level ][ cluster ] += con_sum * point_weight;
                        }
                    }

                    if (level_sum > 10.0 * DBL_MIN)
                    {
                        for (level = first_level; level < last_level_num; level++)
                        {
                            P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ] /= level_sum;

                            ASSERT_IS_NUMBER_DBL(P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ]);
                            ASSERT_IS_FINITE_DBL(P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ]);
                            ASSERT_IS_NON_NEGATIVE_DBL(P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ]);

                        }
                    }
                    else
                    {
                        /* This does happen. */

                        for (level = first_level; level < last_level_num; level++)
                        {
                            P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ] = 1.0 / (last_level_num - first_level);

                            ASSERT_IS_NUMBER_DBL(P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ]);
                            ASSERT_IS_FINITE_DBL(P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ]);
                            ASSERT_IS_NON_NEGATIVE_DBL(P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster_index ]);
                        }
                    }
                }
            }
            else if (compute_aspects)
            {
                level_sum = 0.0;

                for (level = first_level; level < last_level_num; level++)
                {
                    double cluster_sum = 0.0;

                    for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                    {
                        double dis_sum = 0.0;
                        double con_sum = 0.0;

                        cluster   = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                        ASSERT_IS_NON_NEGATIVE_INT(cluster_index);
                        ASSERT_IS_LESS_INT(cluster_index , num_clusters);

                        if (    (fs_use_specified_clusters)
                             && (data_ptr->group_vp != NULL)
                           )
                        {
                            if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                        }

                        dis_item_ptr  = (dis_item_mp == NULL) ? NULL : dis_item_mp->elements[ point ];
                        dis_item_multiplicity_ptr = (dis_item_multiplicity_mp == NULL) ? NULL : dis_item_multiplicity_mp->elements[ point ];

                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ cluster_index ];

                        if (max_num_dis_items > 0)
                        {
                            q_ptr = level_indicators_p;

                            for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                            {
                                ASSERT_IS_NUMBER_DBL(dis_sum);
                                ASSERT_IS_FINITE_DBL(dis_sum);
                                ASSERT_IS_NON_NEGATIVE_DBL(dis_sum);

#ifdef TEST
                                category = *dis_item_ptr;
                                ASSERT_IS_EQUAL_INT(category, dis_item_mp->elements[ point ][ dis_item ]);
                                ASSERT_IS_NON_NEGATIVE_INT(category);
#endif

                                ASSERT_IS_EQUAL_INT(*dis_item_multiplicity_ptr, dis_item_multiplicity_mp->elements[ point ][ dis_item ]);

#ifdef LOOKS_BUGGY
                                /*
                                // Looks a bit BUGGY? Why no state change
                                // before continue? How it was before mods for
                                // NULL.
                                */
                                if (*q_ptr < DBL_NOT_SET / 2.0)
                                {
                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);
                                    continue;
                                }
#else
                                if (*q_ptr < 0.0)
                                {
                                    if (options_ptr->model_correspondence < 2)
                                    {
                                        SET_CANT_HAPPEN_BUG();
                                        result = ERROR;
                                        break;
                                    }

                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);

                                    q_ptr++;
                                    dis_item_ptr++;
                                    dis_item_multiplicity_ptr++;
                                    continue;
                                }
#endif

                                ASSERT_IS_NUMBER_DBL(*q_ptr);
                                ASSERT_IS_FINITE_DBL(*q_ptr);
                                ASSERT_IS_NON_NEGATIVE_DBL(*q_ptr);
                                ASSERT_IS_PROB_DBL(*q_ptr);

                                ASSERT_IS_NUMBER_DBL(dis_factor);
                                ASSERT_IS_FINITE_DBL(dis_factor);
                                ASSERT_IS_NON_NEGATIVE_DBL(dis_factor);
                                ASSERT_IS_GREATER_DBL(dis_factor, 1e-10);

                                ASSERT_IS_NUMBER_DBL(*dis_item_multiplicity_ptr);
                                ASSERT_IS_NON_NEGATIVE_DBL(*dis_item_multiplicity_ptr);

                                q = (*q_ptr) * dis_factor * (*dis_item_multiplicity_ptr);

                                ASSERT_IS_NUMBER_DBL(q);
                                ASSERT_IS_NON_NEGATIVE_DBL(q);

                                dis_sum += q;

                                ASSERT_IS_NUMBER_DBL(dis_sum);
                                ASSERT_IS_FINITE_DBL(dis_sum);
                                ASSERT_IS_NON_NEGATIVE_DBL(dis_sum);

                                q_ptr++;
                                dis_item_ptr++;
                                dis_item_multiplicity_ptr++;
                            }
                        }

                        level_indicators_p += max_num_dis_items;

                        if (num_con_items > 0)
                        {
                            q_ptr = level_indicators_p;

                            ASSERT_IS_NUMBER_DBL(con_factor);
                            ASSERT_IS_FINITE_DBL(con_factor);
                            ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

                            for (con_item = 0; con_item < num_con_items; con_item++)
                            {
                                ASSERT_IS_NUMBER_DBL(con_factor);
                                ASSERT_IS_FINITE_DBL(con_factor);
                                ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

#ifdef LOOKS_BUGGY
                                /*
                                // Looks a bit BUGGY? Why no state change
                                // before continue? How it was before mods for
                                // NULL.
                                */
                                if (*q_ptr < DBL_NOT_SET / 2.0)
                                {
                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);
                                    continue;
                                }
#else
                                if (*q_ptr < 0.0)
                                {
                                    if (options_ptr->model_correspondence < 2)
                                    {
                                        SET_CANT_HAPPEN_BUG();
                                        result = ERROR;
                                        break;
                                    }

                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);

                                    q_ptr++;
                                    continue;
                                }
#endif


                                ASSERT_IS_NUMBER_DBL(*q_ptr);
                                ASSERT_IS_PROB_DBL(*q_ptr);

                                q = (*q_ptr) * con_factor;

                                ASSERT_IS_NUMBER_DBL(q);
                                ASSERT_IS_NON_NEGATIVE_DBL(q);

                                con_sum += q;
                                q_ptr++;

                                ASSERT_IS_NUMBER_DBL(con_sum);
                                ASSERT_IS_FINITE_DBL(con_sum);
                                ASSERT_IS_NON_NEGATIVE_DBL(con_sum);
                            }
                        }

                        ASSERT_IS_NUMBER_DBL(con_sum);
                        ASSERT_IS_FINITE_DBL(con_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(con_sum);

                        /*
                        // Multiplying does not make much sense because there
                        // are then two nu's in the product!
                        */
                        if (fs_multiply_vertical_prior_weights)
                        {
                            sum = dis_sum * con_sum;
                        }
                        else
                        {
                            sum = dis_sum + con_sum;
                        }

                        ASSERT_IS_NUMBER_DBL(sum);
                        ASSERT_IS_FINITE_DBL(sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(sum);

                        cluster_sum += sum;

                        ASSERT_IS_NUMBER_DBL(cluster_sum);
                        ASSERT_IS_FINITE_DBL(cluster_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(cluster_sum);

                        ASSERT_IS_NOT_LESS_INT(level, first_level);
                        ASSERT_IS_LESS_INT(level , last_level_num);
                        ASSERT_IS_LESS_INT(level , new_V_mp->num_rows);
                        ASSERT_IS_NON_NEGATIVE_INT(cluster);
                        ASSERT_IS_LESS_INT(cluster , num_clusters);
                        ASSERT_IS_LESS_INT(cluster , new_V_mp->num_cols);

                        new_V_mp->elements[ level ][ cluster ] += sum * point_weight;

                        if (new_dis_V_mp != NULL)
                        {
                            new_dis_V_mp->elements[ level ][ cluster ] += dis_sum * point_weight;
                        }

                        if (new_con_V_mp != NULL)
                        {
                            new_con_V_mp->elements[ level ][ cluster ] += con_sum * point_weight;
                        }
                    }

                    level_sum += cluster_sum;

                    ASSERT_IS_NUMBER_DBL(level_sum);
                    ASSERT_IS_FINITE_DBL(level_sum);
                    ASSERT_IS_NON_NEGATIVE_DBL(level_sum);

                    P_l_p_mp->elements[ level ][ point ] = cluster_sum;

                }

                if (level_sum > 1000000.0 * DBL_MIN)
                {
                    for (level = first_level; level < last_level_num; level++)
                    {
                        P_l_p_mp->elements[ level ][ point ] /= level_sum;

                        ASSERT_IS_NUMBER_DBL(P_l_p_mp->elements[ level ][ point ]);
                        ASSERT_IS_FINITE_DBL(P_l_p_mp->elements[ level ][ point ]);
                        ASSERT_IS_NON_NEGATIVE_DBL(P_l_p_mp->elements[ level ][ point ]);

                    }
                }
                else
                {
                    /* This does happen. */

                    for (level = first_level; level < last_level_num; level++)
                    {
                        P_l_p_mp->elements[ level ][ point ] = 1.0 / (last_level_num - first_level);

                        ASSERT_IS_NUMBER_DBL(P_l_p_mp->elements[ level ][ point ]);
                        ASSERT_IS_FINITE_DBL(P_l_p_mp->elements[ level ][ point ]);
                        ASSERT_IS_NON_NEGATIVE_DBL(P_l_p_mp->elements[ level ][ point ]);
                    }
                }
            }
            else  /* Case we only want the vertical average follows. */
            {
                for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                {
                    cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                    if (    (fs_use_specified_clusters)
                         && (data_ptr->group_vp != NULL)
                       )
                    {
                        if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    }

                    for (level = first_level; level < last_level_num; level++)
                    {
                        double dis_sum       = 0.0;
                        double con_sum       = 0.0;

                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p  = level_indicators_pp[ cluster_index ];

                        if (max_num_dis_items > 0)
                        {
                            q_ptr = level_indicators_p;

                            for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                            {
#ifdef TEST
                                category = dis_item_mp->elements[ point ][ dis_item ];
                                ASSERT_IS_LESS_INT(category , num_categories);
                                ASSERT_IS_NON_NEGATIVE_INT(category);
#endif

#ifdef LOOKS_BUGGY
                                /*
                                // Looks a bit BUGGY? Why no state change
                                // before continue? How it was before mods for
                                // NULL.
                                */
                                if (*q_ptr < DBL_NOT_SET / 2.0)
                                {
                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);
                                    continue;
                                }
#else
                                if (*q_ptr < 0.0)
                                {
                                    if (options_ptr->model_correspondence < 2)
                                    {
                                        SET_CANT_HAPPEN_BUG();
                                        result = ERROR;
                                        break;
                                    }

                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);

                                    q_ptr++;
                                    continue;
                                }
#endif

                                ASSERT_IS_NUMBER_DBL(*q_ptr);
                                ASSERT_IS_FINITE_DBL(*q_ptr);
                                ASSERT_IS_NON_NEGATIVE_DBL(*q_ptr);
                                ASSERT_IS_PROB_DBL(*q_ptr);

                                dis_item_multiplicity = dis_item_multiplicity_mp->elements[ point ][ dis_item ];

                                dis_sum += dis_item_multiplicity * dis_factor * (*q_ptr);
                                q_ptr++;
                            }
                        }

                        level_indicators_p += max_num_dis_items;

                        if (max_num_con_items > 0)
                        {
                            q_ptr = level_indicators_p;

                            for (con_item = 0; con_item < num_con_items; con_item++)
                            {
#ifdef LOOKS_BUGGY
                                /* Looks a bit BUGGY? Why no state change
                                // before continue? */
                                /* How it was before mods for NULL. */
                                if (*q_ptr < DBL_NOT_SET / 2.0)
                                {
                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);
                                    continue;
                                }
#else
                                if (*q_ptr < 0.0)
                                {
                                    if (options_ptr->model_correspondence < 2)
                                    {
                                        SET_CANT_HAPPEN_BUG();
                                        result = ERROR;
                                        break;
                                    }

                                    ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                    ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);

                                    q_ptr++;
                                    continue;
                                }
#endif


                                ASSERT_IS_NUMBER_DBL(*q_ptr);
                                ASSERT_IS_FINITE_DBL(*q_ptr);
                                ASSERT_IS_NON_NEGATIVE_DBL(*q_ptr);
                                ASSERT_IS_PROB_DBL(*q_ptr);

                                con_sum += con_factor * (*q_ptr);

                                q_ptr++;
                            }
                        }

                        ASSERT_IS_NOT_LESS_INT(level, first_level);
                        ASSERT_IS_LESS_INT(level , last_level_num);
                        ASSERT_IS_LESS_INT(level , new_V_mp->num_rows);
                        ASSERT_IS_NON_NEGATIVE_INT(cluster);
                        ASSERT_IS_LESS_INT(cluster , num_clusters);
                        ASSERT_IS_LESS_INT(cluster , new_V_mp->num_cols);

                        /*
                        // Multiplying does not make much sense because there
                        // are then two nu's in the product!
                        */
                        if (fs_multiply_vertical_prior_weights)
                        {
                            new_V_mp->elements[ level ][ cluster ] += (dis_sum * con_sum) * point_weight;
                        }
                        else
                        {
                            new_V_mp->elements[ level ][ cluster ] += (dis_sum + con_sum) * point_weight;
                        }

                        if (new_con_V_mp != NULL)
                        {
                            new_con_V_mp->elements[ level ][ cluster ] += con_sum * point_weight;
                        }

                        if (new_dis_V_mp != NULL)
                        {
                            new_dis_V_mp->elements[ level ][ cluster ] += dis_sum * point_weight;
                        }
                    }
                }
            }


            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
/* CPU  */  vertical_weight_update_cpu += (get_cpu_time() - cpu_time_offset);
#endif


/*
            Not valid for testing switch of num clusters.

            ASSERT_IS_EQUAL_INT(num_clusters, num_real_clusters);
*/
            ASSERT_IS_EQUAL_INT(num_levels, num_real_levels);

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif

#ifdef REPORT_CPU_TIME
            /*
            //                           M Step
            */
/* CPU  */  cpu_time_offset = get_cpu_time();
#endif
            for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
            {
                cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                if (    (fs_use_specified_clusters)
                     && (data_ptr->group_vp != NULL)
                   )
                {
                    if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                    ASSERT_IS_GREATER_DBL(P_c_p_vp->elements[ cluster_index ], 0.99999);
                    ASSERT_IS_LESS_DBL(P_c_p_vp->elements[ cluster_index ], 1.00001);
                }
                new_a_vp->elements[ cluster ] += P_c_p_vp->elements[ cluster_index ];
            }

#ifdef OBSOLETE /* Superseeded by separate word model. */
            /* IE, see do_fixed_discrete_multi_modal_clustering(). */
            if (topology_ptr->hack_nando_gaussian)
            {
                for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                {
                    category = dis_item_mp->elements[ point ][ dis_item ];
                    ASSERT_IS_POSITIVE_INT(category);

                    new_P_i_n_mvp->elements[ category ]->elements[ 0 ][ category ] = 1.0;
                }
            }
            else
#endif
            if (max_num_dis_items > 0)
            {
                for (level = first_level; level < last_level_num; level++)
                {
                    level_indicators_pp = level_indicators_ppp[ level ];

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    P_sum_vp = dis_P_sum_vvp->elements[ level ];
                    P_i_n_mp = new_P_i_n_mvp->elements[ level ];

                    for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                    {
                        cluster = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                        if (    (fs_use_specified_clusters)
                             && (data_ptr->group_vp != NULL)
                           )
                        {
                            if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                        }

                        node = node_mp->elements[ cluster ][ level ];
                        q_ptr     = level_indicators_pp[ cluster_index ];
                        P_sum_ptr = &(P_sum_vp->elements[ node ]);
                        P_i_n_ptr = P_i_n_mp->elements[ node ];
                        dis_item_ptr = dis_item_mp->elements[ point ];
                        dis_item_multiplicity_ptr = dis_item_multiplicity_mp->elements[ point ];
#ifdef SUPPORT_IGNORE_DIS_PROB
                        p_use_dis_item_ptr = p_use_dis_item_mp->elements[ cluster_index ];
#endif
                        for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                        {
#ifdef DEBUG_TRIMMING
                            total_dis_items++;
#endif
#ifdef LOOKS_BUGGY
                            /*
                            // Looks a bit BUGGY? Why no state change before
                            // continue? How it was before mods for NULL.
                            */
                            if (*q_ptr < DBL_NOT_SET / 2.0)
                            {
                                ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);
                                continue;
                            }
#else
                            if (*q_ptr < 0.0)
                            {
                                ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);

                                if (options_ptr->model_correspondence < 2)
                                {
                                    SET_CANT_HAPPEN_BUG();
                                    result = ERROR;
                                    break;
                                }

                                q_ptr++;
                                dis_item_ptr++;
                                dis_item_multiplicity_ptr++;
#ifdef SUPPORT_IGNORE_DIS_PROB
                                p_use_dis_item_ptr++;
#endif
                                continue;
                            }
#endif
#ifdef XXX_SUPPORT_TRIMMING
                            else if (    (apply_score_cutoff)
                                      && (dis_score_vp->elements[ dis_item_count + dis_item ] <= dis_score_cutoff)
                                    )
                            {
                                q_ptr++;
                                dis_item_ptr++;
                                dis_item_multiplicity_ptr++;
#ifdef SUPPORT_IGNORE_DIS_PROB
                                p_use_dis_item_ptr++;
#endif
                                continue;
                            }
#endif

#ifdef DEBUG_TRIMMING
                            dis_items_used++;
#endif

                            category = *dis_item_ptr;

                            ASSERT_IS_EQUAL_INT(category, dis_item_mp->elements[ point ][ dis_item ]);
                            ASSERT_IS_EQUAL_INT(*dis_item_multiplicity_ptr, dis_item_multiplicity_mp->elements[ point ][ dis_item ]);

                            ASSERT_IS_NON_NEGATIVE_INT(category);
                            ASSERT_IS_LESS_INT(category , num_categories);

                            ASSERT_IS_NUMBER_DBL(*q_ptr);
                            ASSERT_IS_FINITE_DBL(*q_ptr);
                            ASSERT_IS_NON_NEGATIVE_DBL(*q_ptr);
                            ASSERT_IS_PROB_DBL(*q_ptr);

                            q = (*q_ptr) * dis_factor * (*dis_item_multiplicity_ptr);
#ifdef SUPPORT_IGNORE_DIS_PROB
                            q *= (*p_use_dis_item_ptr);
#endif
                            q *= point_weight;

                            if (dis_item_empirical_vp != NULL)
                            {
                                ASSERT(dis_item_empirical_vp->elements[ category ] > 1e-10);
                                q /= dis_item_empirical_vp->elements[ category ];
                            }

                            ASSERT_IS_GREATER_DBL(dis_factor, 1e-10);

                            ASSERT_IS_NUMBER_DBL(q);
                            ASSERT_IS_FINITE_DBL(q);
                            ASSERT_IS_NON_NEGATIVE_DBL(q);

                            ASSERT_IS_NUMBER_DBL(*dis_item_multiplicity_ptr);
                            ASSERT_IS_NON_NEGATIVE_DBL(*dis_item_multiplicity_ptr);
                            *P_sum_ptr += q;
                            P_i_n_ptr[ category ] += q;

                            q_ptr++;
                            dis_item_ptr++;
                            dis_item_multiplicity_ptr++;
#ifdef SUPPORT_IGNORE_DIS_PROB
                            p_use_dis_item_ptr++;
#endif
                        }
                    }
                }
            }

            dis_item_count += num_dis_items;

#ifdef REPORT_CPU_TIME
/* CPU  */  dis_item_parameter_update_cpu += (get_cpu_time() - cpu_time_offset);
/* CPU  */  cpu_time_offset = get_cpu_time();
#endif

            if (max_num_con_items > 0)
            {
                for (level = first_level; level < last_level_num; level++)
                {
                    P_sum_vp = con_P_sum_vvp->elements[ level ];
                    mean_mp = new_mean_mvp->elements[ level ];
                    var_mp = new_var_mvp->elements[ level ];

                    level_indicators_pp = level_indicators_ppp[ level ];
                    num_nodes_this_level = level_counts_vp->elements[ level ];

                    for (cluster_index = 0; cluster_index < num_clusters_used; cluster_index++)
                    {
                        const Vector*  missing_con_P_sum_vp;

                        cluster  = (cluster_index_ptr == NULL) ? cluster_index : cluster_index_ptr[ cluster_index ];

                        if (    (fs_use_specified_clusters)
                             && (data_ptr->group_vp != NULL)
                           )
                        {
                            if (data_ptr->group_vp->elements[ point ] != cluster) continue;
                        }

                        q_ptr = level_indicators_pp[ cluster_index ] + max_num_dis_items;
                        node = node_mp->elements[ cluster ][ level ];

                        ASSERT_IS_NON_NEGATIVE_INT(node);
                        ASSERT_IS_LESS_INT(node , num_nodes_this_level);

                        missing_con_P_sum_vp = missing_con_P_sum_vvvp->elements[ level ]->elements[ node ];

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            double con_item_weight = 1.0;
                            double weight;
                            int missing_data = missing_index_vvp->elements[ point ]->elements[ con_item ];


                            if ((model_ptr->weight_con_items) && (con_item_weight_vvp != NULL))
                            {
                                static int first_time = TRUE;

                                if (first_time)
                                {
                                    first_time = FALSE;
                                    dbp("Weighting continous features.\n");
                                }

                                con_item_weight = con_item_weight_vvp->elements[point]->elements[con_item];
                            }

                            weight = point_weight * con_item_weight;

#ifdef LOOKS_BUGGY
                            /*
                            // Looks a bit BUGGY? Why no state change before
                            // continue? How it was before mods for NULL.
                            */
                            if (*q_ptr < DBL_NOT_SET / 2.0)
                            {
                                ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);
                                continue;
                            }
#else
                            if (*q_ptr < 0.0)
                            {
                                ASSERT_IS_LESS_DBL(*q_ptr, DBL_NOT_SET + 000.1);
                                ASSERT_IS_GREATER_DBL(*q_ptr, DBL_NOT_SET - 000.1);

                                if (options_ptr->model_correspondence < 2)
                                {
                                    SET_CANT_HAPPEN_BUG();
                                    result = ERROR;
                                    break;
                                }

                                q_ptr++;
                                continue;
                            }
#endif
#ifdef XXX_SUPPORT_TRIMMING
                            else if (    (apply_score_cutoff)
                                      && (con_score_vp->elements[ con_item_count + con_item ] <= con_score_cutoff)
                                    )
                            {
#ifdef DEBUG_TRIMMING
                                total_con_items++;
#endif

                                q_ptr++;
                                continue;
                            }

#ifdef DEBUG_TRIMMING
                            total_con_items++;
                            con_items_used++;
#endif
#endif

                            ASSERT_IS_NUMBER_DBL(con_factor);
                            ASSERT_IS_FINITE_DBL(con_factor);
                            ASSERT_IS_NON_NEGATIVE_DBL(con_factor);
                            ASSERT_IS_NOT_LESS_DBL(con_factor, 1e-10);

                            ASSERT_IS_NUMBER_DBL(*q_ptr);
                            ASSERT_IS_FINITE_DBL(*q_ptr);
                            ASSERT_IS_NON_NEGATIVE_DBL(*q_ptr);
#ifdef MIN_LEVEL_INDICATOR
                            ASSERT_IS_NOT_LESS_DBL(*q_ptr,
                                                  MIN_HORIZONTAL_INDICATOR * MIN_LEVEL_INDICATOR / 4.0);
#endif

                            q     = (*q_ptr) * con_factor * weight;
                            x_ptr = con_item_mvp->elements[ point ]->elements[ con_item ];
                            u_ptr = mean_mp->elements[ node ];
                            v_ptr = var_mp->elements[ node ];

                            ASSERT_IS_NUMBER_DBL(q);
                            ASSERT_IS_FINITE_DBL(q);
                            ASSERT_IS_NON_NEGATIVE_DBL(q);
#ifdef MIN_LEVEL_INDICATOR
                            ASSERT_IS_NOT_LESS_DBL(q, 1e-10 * MIN_HORIZONTAL_INDICATOR * MIN_LEVEL_INDICATOR / 4.0);
#endif

                            /*
                             * Comment added Aug 30, 2004.
                             *
                             * Currently we do the M step over all features, not
                             * just the enabled ones. This is likely just extra
                             * work, but will have no effect. Conceivably, it
                             * will be of use to have the values of the means
                             * and the variances for the disabled features,
                             * computed using the enabled ones (a little how we
                             * were doing black and white). Since this whole
                             * feature enable/disable thing is a bit of a hack
                             * anyway, and is not a performance win, getting it
                             * correct for the few times it is used seems like a
                             * better idea than mucking with the code below for
                             * now.
                            */
                            if (missing_data)
                            {
                                for (con_feature = 0; con_feature < num_con_features; con_feature++)
                                {
                                    ASSERT_IS_NUMBER_DBL(*x_ptr);
                                    ASSERT_IS_FINITE_DBL(*x_ptr);

                                    ASSERT_IS_NUMBER_DBL(*u_ptr);
                                    ASSERT_IS_FINITE_DBL(*u_ptr);

                                    ASSERT_IS_NUMBER_DBL(*v_ptr);
                                    ASSERT_IS_FINITE_DBL(*v_ptr);
                                    ASSERT_IS_NON_NEGATIVE_DBL(*v_ptr);

                                    if (IS_NOT_MISSING_DBL(*x_ptr))
                                    {
                                        temp = q * (*x_ptr);

                                        *u_ptr += temp;
                                        *v_ptr += temp * (*x_ptr);

                                        missing_con_P_sum_vp->elements[ con_feature ] += q;
                                    }

                                    ASSERT_IS_NUMBER_DBL(*u_ptr);
                                    ASSERT_IS_FINITE_DBL(*u_ptr);

                                    ASSERT_IS_NUMBER_DBL(*v_ptr);
                                    ASSERT_IS_FINITE_DBL(*v_ptr);
                                    ASSERT_IS_NON_NEGATIVE_DBL(*v_ptr);

                                    v_ptr++;
                                    u_ptr++;
                                    x_ptr++;
                                }
                            }
                            else
                            {
                                for (con_feature = 0; con_feature < num_con_features; con_feature++)
                                {
                                    ASSERT_IS_NUMBER_DBL(*x_ptr);
                                    ASSERT_IS_FINITE_DBL(*x_ptr);

                                    ASSERT_IS_NUMBER_DBL(*u_ptr);
                                    ASSERT_IS_FINITE_DBL(*u_ptr);

                                    ASSERT_IS_NUMBER_DBL(*v_ptr);
                                    ASSERT_IS_FINITE_DBL(*v_ptr);
                                    ASSERT_IS_NON_NEGATIVE_DBL(*v_ptr);

                                    temp = q * (*x_ptr);

                                    *u_ptr += temp;
                                    *v_ptr += temp * (*x_ptr);

                                    ASSERT_IS_NUMBER_DBL(*u_ptr);
                                    ASSERT_IS_FINITE_DBL(*u_ptr);

                                    ASSERT_IS_NUMBER_DBL(*v_ptr);
                                    ASSERT_IS_FINITE_DBL(*v_ptr);
                                    ASSERT_IS_NON_NEGATIVE_DBL(*v_ptr);

                                    v_ptr++;
                                    u_ptr++;
                                    x_ptr++;
                                }

                                P_sum_vp->elements[ node ] += q;


                                ASSERT_IS_NUMBER_DBL(P_sum_vp->elements[ node ]);
                                ASSERT_IS_FINITE_DBL(P_sum_vp->elements[ node ]);

#ifdef MIN_LEVEL_INDICATOR
                                /* Assumes con_factor exceeds 1e-10 */
                                ASSERT_IS_NOT_LESS_DBL(P_sum_vp->elements[ node ],
                                                      1e-10 * MIN_HORIZONTAL_INDICATOR * MIN_LEVEL_INDICATOR / 4.0);
                                /*
                                // Assumes con_factor exceeds 1e-10 and at least one
                                // con item per point
                                */
                                ASSERT_IS_NOT_LESS_DBL(P_sum_vp->elements[ node ],
                                                      (point + 1) * 1e-10 * MIN_HORIZONTAL_INDICATOR * MIN_LEVEL_INDICATOR / 4.0);
#endif
                            }

                            q_ptr++;
                        }
                    }
                }

#ifdef TEST
                for (level = first_level; level < last_level_num; level++)
                {
                    P_sum_vp = con_P_sum_vvp->elements[ level ];

                    num_nodes_this_level = level_counts_vp->elements[ level ];

                    for (node = 0; node < num_nodes_this_level; node++)
                    {
                        ASSERT_IS_NUMBER_DBL(P_sum_vp->elements[ node ]);
                        ASSERT_IS_FINITE_DBL(P_sum_vp->elements[ node ]);
                    }
                }
#endif

            } /* End case of (max_num_con_items > 0) */

            con_item_count += num_con_items;

#ifdef REPORT_CPU_TIME
/* CPU  */  con_item_parameter_update_cpu += (get_cpu_time() - cpu_time_offset);
#endif
            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
            cpu_time_offset = get_cpu_time();
#endif

            /*
            // Once we have limited the cluster membership, points cannot
            // regain cluster membership (unless we are doing uniform levels,
            // but in this case we would only be limiting clusters for speed),
            // so don't bother making the index.
            */
            if (    ( ! valid_cluster_index )
                 && (num_clusters_used != num_limited_clusters)
               )
            {
                result = get_limited_cluster_membership(cluster_index_mp, P_c_p_vp,
                                                        point, num_limited_clusters,
                                                        &limited_cluster_error);
                if (result == ERROR) { NOTE_ERROR(); break; }

                if (ave_limited_cluster_error < 0.0)
                {
                    ave_limited_cluster_error = limited_cluster_error;
                }
                else
                {
                    ave_limited_cluster_error += limited_cluster_error;
                }

                max_limited_cluster_error = MAX_OF(max_limited_cluster_error,
                                                   limited_cluster_error);
            }

#ifdef REPORT_CPU_TIME
            limited_cluster_cpu += (get_cpu_time() - cpu_time_offset);
#endif

        } /* Loop over points. */

        CHECK_FOR_HEAP_OVERRUNS();

#ifdef REPORT_MEMORY_USE
        if (    (result != ERROR)
             && (get_allocated_memory(&total_bytes_used) != ERROR)
           )
        {
            dbu(total_bytes_used);
        }
#endif

#ifdef DEBUG_TRIMMING
        dbi(con_item_count);
        dbi(dis_item_count);
        dbi(temp_good_dis_item_count);
        dbi(temp_total_dis_item_count);
        dbi(temp_good_con_item_count);
        dbi(temp_total_con_item_count);
#endif

#ifdef XXX_SUPPORT_TRIMMING
        }    /* End of loop over dry and wet runs. */
#endif
        verify_matrix_vector(new_var_mvp, NULL);
        verify_matrix_vector(new_mean_mvp, NULL);
        verify_matrix_vector(new_P_i_n_mvp, NULL);

        if (result == ERROR) { NOTE_ERROR(); break; }

        if ( ! valid_cluster_index )
        {
            ave_limited_cluster_error /= num_points;
        }

        if ( ! current_model_is_valid)
        {
            goto loop_cleanup;
        }


        /*
        //                  Log Likelihood for this iteration.
        */

        if (log_likelihood < DBL_HALF_MOST_NEGATIVE/2.0) break;

        if (fs_norm_con_prob)
        {
            log_likelihood += con_log_norm * ll_con_correction_norm_sum;
            model_ptr->con_log_norm = con_log_norm;
            model_ptr->min_con_log_prob = min_con_log_prob;
        }

        model_ptr->con_score_cutoff = con_score_cutoff;

        if (log_likelihood < DBL_HALF_MOST_NEGATIVE/2.0) break;

        /*
        // Calculate log prior to make log_likelihood into log posterior.
        */
        if (fs_do_map && (max_num_dis_items > 0))
        {
            for (level = first_level; level < last_level_num; level++)
            {
                map_beta_minus_one_vvp = map_beta_minus_one_vvvp->elements[ level ];
                P_i_n_mp = P_i_n_mvp->elements[ level ];

                num_nodes_this_level = level_counts_vp->elements[ level ];

                for (node = 0; node < num_nodes_this_level; node++)
                {
                    map_beta_minus_one_vp = map_beta_minus_one_vvp->elements[ node ];

                    for (category = 0; category < num_categories; category++)
                    {
                        log_likelihood +=
                            map_beta_minus_one_vp->elements[ category ] *
                                       log(P_i_n_mp->elements[ node ][ category ]);
                    }
                }
            }
        }

        if (log_likelihood < DBL_HALF_MOST_NEGATIVE/2.0) break;

        if (fs_do_map && (max_num_con_items > 0))
        {
            if (result == ERROR) { NOTE_ERROR(); break; }

            for (level = first_level; level < last_level_num; level++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }

                map_r_minus_n_g = map_r_minus_n_g_vp->elements[ level ];

                mean_mp  = mean_mvp->elements[  level ];
                var_mp   = var_mvp->elements[   level ];
                omega_mp = omega_mvp->elements[ level ];
                delta_inverse_mp = delta_inverse_mvp->elements[ level ];

                num_nodes_this_level = level_counts_vp->elements[ level ];

                for (node = 0; node < num_nodes_this_level; node++)
                {
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&u_vp, mean_mp, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&w_vp, omega_mp, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&var_vp, var_mp, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_matrix_row(&d_vp, delta_inverse_mp, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_malhalanobis_distance_sqrd(u_vp, w_vp, var_vp, &d);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    log_likelihood -= (fs_map_kappa * d / 2.0);

                    log_likelihood -= (map_r_minus_n_g * con_log_sqrt_det_vvp->elements[ level ]->elements[ node ]);

                    result = ow_invert_vector(var_vp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = get_dot_product(d_vp, var_vp, &d);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    log_likelihood -= (d / 2.0);
                }
            }
        }

        if (log_likelihood < DBL_HALF_MOST_NEGATIVE/2.0) break;

        model_ptr->raw_log_likelihood = log_likelihood;

        if (changed_number_of_parameters)
        {
            warn_pso("The model has changed.\n");
        }

#ifdef DEBUG_TRIMMING
        dbi(num_points);
        dbi(num_good_points);
#endif
        log_likelihood /= num_good_points;

        if (    (IS_LESSER_DBL(log_likelihood, prev_log_likelihood))
             && ( ! changed_number_of_parameters )
           )
        {
            warn_pso("Log likelihood is less than previous.\n");
        }

        ll_rel_diff = log_likelihood - prev_log_likelihood;

        ll_rel_diff /= (ABS_OF(log_likelihood) +  ABS_OF(prev_log_likelihood));
        ll_rel_diff *= 2.0;

        prev_log_likelihood = log_likelihood;


#ifdef REPORT_CPU_TIME
        cpu_time_offset = get_cpu_time();
#endif
        if (need_held_out_log_likelihood)
        {
#ifdef TRY_IT_WITH_AVE_ONLY
            result = get_cluster_membership_2((Matrix**)NULL,
                                              valid_cluster_dependent_aspects ?
                                                            &held_out_P_l_p_c_mvp
                                                            : (Matrix_vector**) NULL,
                                              (Int_matrix_vector**)NULL,
                                              valid_aspects ?
                                                            &held_out_P_l_p_mp
                                                            : (Matrix**) NULL,
                                              (Int_matrix**)NULL,
                                              (Vector**)NULL,
                                              topology_ptr,
                                              a_vp, V_mp,
                                              (const Matrix*)NULL,
                                              (const Matrix_vector*) NULL,
                                              (const Matrix*) NULL,
                                              P_i_n_mvp,
                                              mean_mvp, var_mvp,
                                              con_log_sqrt_det_vvp,
                                              con_log_norm,
                                              con_feature_enable_vp,
                                              con_feature_group_vp,
                                              min_con_log_prob,
                                              fs_max_con_norm_var,
                                              con_score_cutoff,
                                              model_ptr->weight_con_items,
                                              model_ptr->norm_depend,
                                              options_ptr->model_correspondence,
                                              held_out_data_ptr,
                                              (const Int_vector*)NULL,
                                              held_out_dis_factor_vp,
                                              held_out_con_factor_vp,
                                              fs_norm_continuous_features,
                                              MAX_OF(ll_rel_diff,
                                                     fs_iteration_tolerance),
                                              held_out_prev_log_likelihood,
                                              &held_out_log_likelihood,
                                              (const Matrix*)NULL,
                                              FALSE, NULL);
#else
            result = get_cluster_membership_2((Matrix**)NULL,
                                              (Matrix_vector**) NULL,
                                              (Int_matrix_vector**)NULL,
                                              (Matrix**) NULL,
                                              (Int_matrix**)NULL,
                                              (Vector**)NULL,
                                              topology_ptr,
                                              a_vp, V_mp,
                                              (const Matrix*)NULL,
                                              (const Matrix_vector*) NULL,
                                              (const Matrix*) NULL,
                                              P_i_n_mvp,
                                              mean_mvp, var_mvp,
                                              con_log_sqrt_det_vvp,
                                              con_log_norm,
                                              con_feature_enable_vp,
                                              con_feature_group_vp,
                                              min_con_log_prob,
                                              fs_max_con_norm_var,
                                              con_score_cutoff,
                                              model_ptr->weight_con_items,
                                              model_ptr->norm_depend,
                                              options_ptr->model_correspondence,
                                              held_out_data_ptr,
                                              (const Int_vector*)NULL,
                                              held_out_dis_factor_vp,
                                              held_out_con_factor_vp,
                                              fs_norm_continuous_features,
                                              MAX_OF(ll_rel_diff,
                                                     fs_iteration_tolerance),
                                              held_out_prev_log_likelihood,
                                              &held_out_log_likelihood,
                                              (const Matrix*)NULL,
                                              FALSE, NULL);
#endif

            if (result == ERROR) { NOTE_ERROR(); break; }

            held_out_ll_diff = held_out_log_likelihood - held_out_prev_log_likelihood;

            held_out_ll_diff *= 2.0;
            held_out_ll_diff /= (ABS_OF(held_out_log_likelihood) +  ABS_OF(held_out_prev_log_likelihood));

            if (    (held_out_ll_diff < 0.0)
                 && (P_c_p_mp != NULL)  /* Must have limited membership sorted out. */
                 && (it > 1)
               )
            {
                first_negative_held_out_ll_diff = TRUE;
            }

            held_out_prev_log_likelihood = held_out_log_likelihood;

            if  (    (options_ptr->convergence_criterion == MAX_HELD_OUT_LL)
                  && (P_c_p_mp != NULL)  /* Must have limited membership sorted out. */
                  && (held_out_log_likelihood > max_held_out_log_likelihood)
                )
            {
                max_held_out_log_likelihood = held_out_log_likelihood;

                result = copy_multi_modal_model(&save_model_ptr, model_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }

            if  (    (options_ptr->convergence_criterion == MAX_COMBINED_LL)
                  && (P_c_p_mp != NULL)  /* Must have limited membership sorted out. */
                  && (log_likelihood + held_out_log_likelihood > max_combined_log_likelihood)
                )
            {
                max_combined_log_likelihood = log_likelihood + held_out_log_likelihood;

                result = copy_multi_modal_model(&save_model_ptr, model_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
        }

        if ((fs_norm_con_prob) && (ABS_OF(max_con_log_prob) > 1.0))
        {
#ifdef XXX_SUPPORT_TRIMMING
#ifdef DEBUG_TRIMMING
            dbp("^^^^^^^^^^^^^^^^^^");
            dbi(apply_score_cutoff);
#endif
#endif
            con_log_norm += max_con_log_prob;
        }


#ifdef REPORT_CPU_TIME
        held_out_cpu += (get_cpu_time() - cpu_time_offset);
#endif

loop_cleanup:

        CHECK_FOR_HEAP_OVERRUNS();

        verify_matrix_vector(new_var_mvp, NULL);
        verify_matrix_vector(new_mean_mvp, NULL);
        verify_matrix_vector(new_P_i_n_mvp, NULL);

        /*
        //                           V Step Cleanup
        */
#ifdef REPORT_CPU_TIME
        cpu_time_offset = get_cpu_time();
#endif

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = ow_scale_matrix_cols_by_sums(new_V_mp);
        if (result == ERROR) { NOTE_ERROR(); break; }

        if (new_con_V_mp != NULL)
        {
            result = ow_scale_matrix_cols_by_sums(new_con_V_mp);

            if (result == ERROR) { NOTE_ERROR(); break; }
        }

        if (new_dis_V_mp != NULL)
        {
            result = ow_scale_matrix_cols_by_sums(new_dis_V_mp);

            if (result == ERROR) { NOTE_ERROR(); break; }
        }

        /*
        //                           M Step Cleanup
        */

        if (max_num_dis_items > 0)
        {
            for (level = first_level; level < last_level_num; level++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }

                P_i_n_mp = new_P_i_n_mvp->elements[ level ];
                P_sum_vp = dis_P_sum_vvp->elements[ level ];

                verify_vector(P_sum_vp, NULL);

                for (node = 0; node < P_sum_vp->length; node++)
                {
                    s = P_sum_vp->elements[ node ];

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    verify_matrix(P_i_n_mp, NULL);

                    if (s < 1000.0 * DBL_MIN)
                    {
                        warn_pso("Hack to deal with zero probs being used.\n");
                        dbi(level);
                        dbi(node);

                        for (i = 0; i < P_i_n_mp->num_cols; i++)
                        {
                            P_i_n_mp->elements[ node ][ i ] = 1.0 / P_i_n_mp->num_cols;
                        }

                        P_sum_vp->elements[ node ] = 1.0;
                        s = 1.0;
                    }

                    ASSERT_IS_NUMBER_DBL(s);
                    ASSERT_IS_FINITE_DBL(s);
                    ASSERT_IS_NON_NEGATIVE_DBL(s);
                    ASSERT_IS_NOT_LESS_DBL(s, 10.0 * DBL_MIN);

                    result = ow_divide_matrix_row_by_scalar(P_i_n_mp, s, node);
                }
            }

            if (result == ERROR) { NOTE_ERROR(); break; }
            verify_matrix_vector(new_P_i_n_mvp, NULL);

            /*
            // Add a small offset to the word probabilities to stablize
            // training. Problems with training are now rare, so it is hard to
            // verify that this actually does something, but it makes sense--at
            // least as much sense as offsetting the variance!
            */
            for (level = first_level; level < last_level_num; level++)
            {
                P_i_n_mp = new_P_i_n_mvp->elements[ level ];

                result = ow_add_scalar_to_matrix(P_i_n_mp,
                                MIN_DIS_PROB_FOR_OFFSET / num_categories);
                if (result == ERROR) { NOTE_ERROR(); break; }

                verify_matrix(P_i_n_mp, NULL);

                result = ow_scale_matrix_rows_by_sums(P_i_n_mp);

                if (result == ERROR) { NOTE_ERROR(); break; }

                verify_matrix(P_i_n_mp, NULL);
            }

            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef TEST
            /* Verify probs sum to one. */

/* TEST */  for (level = first_level; level < last_level_num; level++)
/* TEST */  {
/* TEST */      num_nodes_this_level = level_counts_vp->elements[ level ];
/* TEST */
/* TEST */      P_i_n_mp = new_P_i_n_mvp->elements[ level ];
/* TEST */
/* TEST */      for (node = 0; node < num_nodes_this_level; node++)
/* TEST */      {
/* TEST */          s = 0.0;
/* TEST */
/* TEST */          for (category = 0; category < num_categories; category++)
/* TEST */          {
/* TEST */              s += P_i_n_mp->elements[ node ][ category ];
/* TEST */          }
/* TEST */
/* TEST */          if (ABS_OF(s - 1.0) > 0.00001)
/* TEST */          {
/* TEST */              warn_pso("Prob of node (%d, %d) emitting ",
/* TEST */                       level, node);
/* TEST */              warn_pso("anything is %f.\n", s);
/* TEST */          }
/* TEST */      }
/* TEST */  }
#endif

            if ((fs_discrete_tie_count > 1) && (num_clusters == 1))
            {
                Vector* ave_P_i_n_vp = NULL;
                int block;
                int num_blocks = (last_level_num - first_level) / fs_discrete_tie_count;

                for (block = 0; block < num_blocks; block++)
                {
                    result = get_zero_vector(&ave_P_i_n_vp, num_categories);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (i = 0; i < fs_discrete_tie_count; i++)
                    {
                        level = first_level + block * fs_discrete_tie_count + i;

                        result = ow_add_matrix_row_to_vector(ave_P_i_n_vp,
                                                             new_P_i_n_mvp->elements[ level ],
                                                             0);
                    }

                    result = ow_divide_vector_by_scalar(ave_P_i_n_vp, fs_discrete_tie_count);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    for (i = 0; i < fs_discrete_tie_count; i++)
                    {
                        level = first_level + block * fs_discrete_tie_count + i;

                        result = put_matrix_row(new_P_i_n_mvp->elements[ level ],
                                                ave_P_i_n_vp,
                                                0);
                    }
                }
                free_vector(ave_P_i_n_vp);
            }
        } /* End of case (max_num_dis_items > 0). */

        if (result == ERROR) { NOTE_ERROR(); break; }

        verify_matrix_vector(new_var_mvp, NULL);
        verify_matrix_vector(new_mean_mvp, NULL);
        verify_matrix_vector(new_P_i_n_mvp, NULL);

        if (max_num_con_items > 0)
        {
            for (level = first_level; level < last_level_num; level++)
            {
                mean_mp  = new_mean_mvp->elements[ level ];
                P_sum_vp = con_P_sum_vvp->elements[ level ];
                var_mp   = new_var_mvp->elements[ level ];
                con_log_sqrt_det_vp = new_con_log_sqrt_det_vvp->elements[ level ];

                num_nodes_this_level = level_counts_vp->elements[ level ];

                /*
                // The first time through we compute the ML estimate which is
                // used as the prior on subsequent iterations.
                */
                if ((fs_do_map) && (omega_mvp->elements[ level ] == NULL))
                {
#ifdef OMEGA_INITIALIZATION_IS_FIRST_ML
                    ASSERT(delta_inverse_mvp->elements[ level ] == NULL);

                    result = copy_matrix(&(omega_mvp->elements[ level ]),
                                         mean_mp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    omega_mp = omega_mvp->elements[ level ];

                    for (node = 0; node < num_nodes_this_level; node++)
                    {
                        if (result == ERROR) { NOTE_ERROR(); break; }

                        s = P_sum_vp->elements[ node ];

                        result = ow_divide_matrix_row_by_scalar(omega_mp, s,
                                                                node);
                    }
#else
                    result = get_zero_matrix(&(omega_mvp->elements[ level ]),
                                             mean_mp->num_rows, mean_mp->num_cols);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                    omega_mp = omega_mvp->elements[ level ];

#endif

#ifdef DELTA_INITIALIZATION_IS_FIRST_ML
                    result = copy_matrix(&(delta_inverse_mvp->elements[ level ]),
                                         var_mp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    delta_inverse_mp = delta_inverse_mvp->elements[ level ];

                    for (node = 0; node < num_nodes_this_level; node++)
                    {
                        s = P_sum_vp->elements[ node ];

                        result = ow_divide_matrix_row_by_scalar(delta_inverse_mp, s,
                                                                node);

                        /*
                         * Comment added Aug 30, 2004.
                         *
                         * Currently we do the M step over all features, not
                         * just the enabled ones. This is likely just extra
                         * work, but will have no effect. Conceivably, it
                         * will be of use to have the values of the means
                         * and the variances for the disabled features,
                         * computed using the enabled ones (a little how we
                         * were doing black and white). Since this whole
                         * feature enable/disable thing is a bit of a hack
                         * anyway, and is not a performance win, getting it
                         * correct for the few times it is used seems like a
                         * better idea than mucking with the code below for
                         * now.
                        */
                        for (con_feature = 0; con_feature < num_con_features; con_feature++)
                        {
                            u = omega_mp->elements[ node ][ con_feature ];
                            u2 = u * u;

                            var = delta_inverse_mp->elements[ node ][ con_feature ] - u2;

                            if (var < 0.0)
                            {
                                /*
                                // This does happen! The calculation of
                                // variance this way is not numerically stable.
                                */
                                var = 0.0;
                            }

                            var += var_offset;
                            delta_inverse_mp->elements[ node ][ con_feature ] = var;
                        }

                        if (result == ERROR) { NOTE_ERROR(); break; }
                    }
#else
                    result = get_zero_matrix(&(delta_inverse_mvp->elements[ level ]),
                                             var_mp->num_rows, var_mp->num_cols);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                    delta_inverse_mp = delta_inverse_mvp->elements[ level ];
#endif
                }

                if (fs_do_map)
                {
                    omega_mp = omega_mvp->elements[ level ];

                    result = ow_add_matrix_times_scalar(mean_mp, omega_mp,
                                                        fs_map_kappa);
                }

                for (node = 0; node < num_nodes_this_level; node++)
                {
                    Vector* missing_con_P_sum_vp = missing_con_P_sum_vvvp->elements[ level ]->elements[ node ];

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    s = P_sum_vp->elements[ node ];

                    if (fs_do_map)
                    {
                        s += fs_map_kappa;
                    }

                    ASSERT_IS_NUMBER_DBL(s);
                    ASSERT_IS_FINITE_DBL(s);
                    ASSERT_IS_NON_NEGATIVE_DBL(s);
                    /*
                    // Assumes con_factor exceeds 1e-10 and at least one
                    // con item per point.
                    */
                    verify_matrix(mean_mp, NULL);
#ifdef HOW_IT_WAS_BEFORE_MISSING
#ifdef MIN_LEVEL_INDICATOR
                    ASSERT_IS_NOT_LESS_DBL(s, num_points * 1e-10 * MIN_LEVEL_INDICATOR * MIN_HORIZONTAL_INDICATOR / 4.0);
#endif

                    ASSERT_IS_NOT_LESS_DBL(s, 10.0 * DBL_MIN);
                    result = ow_divide_matrix_row_by_scalar(mean_mp, s, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }
#else
                    for (con_feature = 0; con_feature < num_con_features; con_feature++)
                    {
                        ASSERT_IS_NOT_LESS_DBL(s + missing_con_P_sum_vp->elements[ con_feature ], 10.0 * DBL_MIN);
#ifdef MIN_LEVEL_INDICATOR
                        ASSERT_IS_NOT_LESS_DBL(s + missing_con_P_sum_vp->elements[ con_feature ], num_points * 1e-10 * MIN_LEVEL_INDICATOR * MIN_HORIZONTAL_INDICATOR / 4.0);
#endif
                        mean_mp->elements[ node ][ con_feature ] /=
                            (s + missing_con_P_sum_vp->elements[ con_feature ]);
                    }
#endif
                    verify_matrix(mean_mp, NULL);
                }

                /* Compute variance. */
                for (node = 0; node < num_nodes_this_level; node++)
                {
                    Vector* missing_con_P_sum_vp = missing_con_P_sum_vvvp->elements[ level ]->elements[ node ];

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    s = P_sum_vp->elements[ node ];

                    verify_matrix(var_mp, NULL);

                    /*
                     * Comment added Aug 30, 2004.
                     *
                     * Currently we do the M step over all features, not just
                     * the enabled ones. This is likely just extra work, but
                     * will have no effect. Conceivably, it will be of use to
                     * have the values of the means and the variances for the
                     * disabled features, computed using the enabled ones (a
                     * little how we were doing black and white). Since this
                     * whole feature enable/disable thing is a bit of a hack
                     * anyway, and is not a performance win, getting it correct
                     * for the few times it is used seems like a better idea
                     * than mucking with the code below for now.
                    */
                    for (con_feature = 0; con_feature < num_con_features; con_feature++)
                    {
                        u = mean_mp->elements[ node ][ con_feature ];
#ifdef HOW_IT_WAS_BEFORE_MISSING
                        u2 = s * u * u;
#else
                        u2 = (s + missing_con_P_sum_vp->elements[ con_feature ]) * u * u;
#endif

                        var = var_mp->elements[ node ][ con_feature ] - u2;

                        if (var < 0.0)
                        {
                            /*
                            // This does happen! The calculation of variance
                            // this way is not numerically stable.
                            */
                            var = 0.0;
                        }

                        if (fs_do_map)
                        {
                            var += delta_inverse_mp->elements[ node ][ con_feature ];

                            temp = omega_mp->elements[ node ][ con_feature ] -
                                                mean_mp->elements[ node ][ con_feature ];

                            var += fs_map_kappa * temp * temp;
                        }

                        var_mp->elements[ node ][ con_feature ] = var;

                        ASSERT_IS_NUMBER_DBL(var);
                        ASSERT_IS_FINITE_DBL(var);
                        ASSERT_IS_NON_NEGATIVE_DBL(var);
                    }

                    if (fs_do_map)
                    {
                        s += (map_r_minus_n_g_vp->elements[ level ]);
                    }

#ifdef HOW_IT_WAS_BEFORE_MISSING
                    result = ow_divide_matrix_row_by_scalar(var_mp, s, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }
#else
                    for (con_feature = 0; con_feature < num_con_features; con_feature++)
                    {
                        var_mp->elements[ node ][ con_feature ] /=
                            (s + missing_con_P_sum_vp->elements[ con_feature ]);
                    }
#endif

                    verify_matrix(var_mp, NULL);

                    result = ow_add_scalar_to_matrix_row(var_mp, var_offset, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    if (fs_tie_var)
                    {
                        double ave_var = 0.0;

                        /*
                         * Comment added Aug 30, 2004.
                         *
                         * Currently we do the M step over all features, not
                         * just the enabled ones. This is likely just extra
                         * work, but will have no effect. Conceivably, it will
                         * be of use to have the values of the means and the
                         * variances for the disabled features, computed using
                         * the enabled ones (a little how we were doing black
                         * and white). Since this whole feature enable/disable
                         * thing is a bit of a hack anyway, and is not a
                         * performance win, getting it correct for the few times
                         * it is used seems like a better idea than mucking with
                         * the code below for now.
                        */
                        for (con_feature = 0; con_feature < num_con_features; con_feature++)
                        {
                            ave_var += var_mp->elements[ node ][ con_feature ];
                        }

                        ave_var /= num_con_features;

                        for (con_feature = 0; con_feature < num_con_features; con_feature++)
                        {
                            var_mp->elements[ node ][ con_feature ] = ave_var;
                        }
                    }
                    else
                    {
                        /* CHECK -- added block for nulls, april 05, 2004 */

                        /* CHECK -- On 05-01-30, this looks a bit puzzling. We
                         *          are overwriting the data!
                        */
                        if (    ( ! fs_vector_feature_counts_set)
                             && (con_feature_enable_vp != NULL)
                             && (con_feature_group_vp != NULL)
                           )
                        {
                            UNTESTED_CODE();

                            ERE(initialize_vector_feature_counts(con_feature_enable_vp,
                                                                 con_feature_group_vp));
                        }

                        if (fs_vector_feature_counts_ptr != NULL)
                        {
                            double ave_var = 0.0;

                            con_feature = 0;

                            while (con_feature < num_con_features)
                            {
                                int count = 0;

                                if (con_feature_group_vp->elements[ con_feature ] >= 0)
                                {
                                    con_feature++;
                                    continue;
                                }

                                for (i = 0; i < num_con_features - con_feature; i++)
                                {
                                    if (con_feature_group_vp->elements[ con_feature ] != con_feature_group_vp->elements[ con_feature + i])
                                    {
                                        break;
                                    }
                                    count++;
                                }

                                for (i = 0; i < count; i++)
                                {
                                    ave_var += var_mp->elements[ node ][ con_feature + i ];
                                }

                                ave_var /= count;

                                for (i = 0; i < count; i++)
                                {
                                    var_mp->elements[ node ][ con_feature + i ] = ave_var;
                                }
                            }
                        }
                    }

                    verify_matrix(var_mp, NULL);

#ifdef ECHO_VAR_AND_MEAN
                    result = get_matrix_row(&u_vp, mean_mp, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    if ((node == 0) && (level < 50))
                    {
                        db_rv(u_vp);
                    }

                    result = get_matrix_row(&var_vp, var_mp, node);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    if ((node == 0) && (level < 50))
                    {
                        db_rv(var_vp);
                    }
#endif
                }

#ifdef TEST
                if (min_matrix_element(var_mp) < var_offset)
                {
                    dbe(var_offset);
                    dbe(fs_var_offset);
                    dbe(fs_var_offset_2);
                    dbe(norm_stdev);
                    dbe(min_matrix_element(var_mp));
                }
#endif

                /* Update con_log_sqrt_det_vp. */
                for (node = 0; node < num_nodes_this_level; node++)
                {
                    con_log_sqrt_det_vp->elements[ node ] = log_scale_factor_sqrd;

                    for (con_feature = 0; con_feature < num_con_features; con_feature++)
                    {
                        if ((con_feature_enable_vp != NULL) && ( ! con_feature_enable_vp->elements[ con_feature ]))
                        {
                            continue;
                        }

                        var = var_mp->elements[ node ][ con_feature ];

                        ASSERT_IS_NOT_LESS_DBL(var, var_offset);

                        if (var < 3.0 * DBL_MIN)
                        {
                            /*
                            // Assuming var_offset is reasonable, this can't
                            // happen!
                            */
                            dbe(var);
                            con_log_sqrt_det_vp->elements[ node ] += LOG_ZERO;
                        }
                        else
                        {
                            con_log_sqrt_det_vp->elements[ node ] += log(var);
                        }
                    }

                    /* Now make it the log of the square root. */
                    con_log_sqrt_det_vp->elements[ node ] /= 2.0;

                    /* And normalize. */
                    con_log_sqrt_det_vp->elements[ node ] += con_log_norm;

                    con_log_sqrt_det_vp->elements[ node ] -= extra_log_con_prob_for_original_data_space;


                    ASSERT_IS_NUMBER_DBL(con_log_sqrt_det_vp->elements[ node ]);
                    ASSERT_IS_FINITE_DBL(con_log_sqrt_det_vp->elements[ node ]);
                }
            }

        } /* End of case (max_num_con_items > 0) */



        /* ------------------ End of M Step cleanup ------------------------ */


        /*
        // Final updates.
        */

        if (max_num_dis_items > 0)
        {
            result = copy_matrix_vector(&(model_ptr->P_i_n_mvp), new_P_i_n_mvp);
            if (result == ERROR) { NOTE_ERROR(); break; }
            P_i_n_mvp = model_ptr->P_i_n_mvp;
        }

        if (max_num_con_items > 0)
        {
            result = copy_matrix_vector(&(model_ptr->mean_mvp), new_mean_mvp);
            if (result == ERROR) { NOTE_ERROR(); break; }
            mean_mvp = model_ptr->mean_mvp;

            result = copy_matrix_vector(&(model_ptr->var_mvp), new_var_mvp);
            if (result == ERROR) { NOTE_ERROR(); break; }
            var_mvp = model_ptr->var_mvp;

            result = copy_vector_vector(&(model_ptr->con_log_sqrt_det_vvp),
                                        new_con_log_sqrt_det_vvp);
            if (result == ERROR) { NOTE_ERROR(); break; }
            con_log_sqrt_det_vvp = model_ptr->con_log_sqrt_det_vvp;

        }

        result = copy_matrix(&(model_ptr->V_mp), new_V_mp);
        if (result == ERROR) { NOTE_ERROR(); break; }
        V_mp = model_ptr->V_mp;

        result = copy_matrix(&(model_ptr->con_V_mp), new_con_V_mp);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = copy_matrix(&(model_ptr->dis_V_mp),
                             new_dis_V_mp);
        if (result == ERROR) { NOTE_ERROR(); break; }

        if ((options_ptr->model_correspondence > 0) && (max_num_dis_items > 0))
        {
            result = copy_matrix_vector(&log_P_i_n_mvp, new_P_i_n_mvp);

            for (level = first_level; level < last_level_num; level++)
            {
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = ow_log_matrix_elements(log_P_i_n_mvp->elements[ level ]);
            }

            verify_matrix_vector(log_P_i_n_mvp, NULL);
        }

        result = copy_vector(&(model_ptr->a_vp), new_a_vp);
        if (result == ERROR) { NOTE_ERROR(); break; }
        a_vp = model_ptr->a_vp;

#ifdef TEST
        ASSERT_IS_EQUAL_INT(a_vp->length, num_clusters);
#endif

        a_vp_normalization = num_points;

        if (    ( ! IS_EQUAL_DBL(fs_map_alpha, 1.0))
             && (map_alpha_vp != NULL)
           )
        {
            dbw();
            dbp("========================================");
            dbw();

            result = ow_add_vectors(a_vp, map_alpha_vp);
            if (result == ERROR) { NOTE_ERROR(); break; }

            result = ow_subtract_scalar_from_vector(a_vp, 1.0);
            if (result == ERROR) { NOTE_ERROR(); break; }

            dbf(a_vp_normalization);
            a_vp_normalization -= num_clusters;
            a_vp_normalization += sum_vector_elements(map_alpha_vp);
            dbf(a_vp_normalization);
        }

#ifdef SUPPORT_IGNORE_DIS_PROB
        model_ptr->extra_dis_prob = extra_dis_prob;
#endif

        verify_vector(a_vp, NULL);
        ASSERT_IS_NOT_LESS_DBL(a_vp_normalization, 10.0 * DBL_MIN);
        result = ow_divide_vector_by_scalar(a_vp, a_vp_normalization);
        if (result == ERROR) { NOTE_ERROR(); break; }
        verify_vector(a_vp, NULL);

        /*
        // Print stats and compute values needed to determine convergence before
        // final update.
        */

        if (current_model_is_valid)
        {
            if (fs_watch)
            {
                pso(" %-3d | %11.4e | %9.2e", 1 + it,
                    log_likelihood,
                    ll_rel_diff);

                if (need_held_out_log_likelihood)
                {
                    pso(" | %11.4e | %9.2e",
                        held_out_log_likelihood / held_out_data_ptr->num_points,
                        held_out_ll_diff);
                }
            }

            if (fs_watch)
            {
#ifdef SUPPORT_ANNEALING
                if (fs_watch_temperature)  pso(" | %10.3f", temperature);
#endif
                if (fs_watch_max_con_prob) pso(" | %9.2e", max_con_log_prob);

#ifdef CANT_SUPPORT_IT_FOR_A_BIT
                /* Need previous key as well as prev_P_c_p_mp. */

                if (fs_watch_rms_cluster_given_point_diff)
                {
                    pso(" | %9.2e",
                        rms_matrix_difference(prev_P_c_p_mp, P_c_p_mp));
                }

                if (fs_watch_max_cluster_given_point_diff)
                {
                    pso(" | %9.2e",
                        max_abs_matrix_difference(prev_P_c_p_mp, P_c_p_mp));
                }

                if (    (fs_watch_max_cluster_given_point_diff)
                     || (fs_watch_rms_cluster_given_point_diff)
                   )
                {
                    result = copy_matrix(&prev_P_c_p_mp, P_c_p_mp);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }
#endif

                if (fs_watch_max_parm_diff)
                {
                    max_parm_diff = DBL_MOST_NEGATIVE;

                    if (max_num_con_items > 0)
                    {
                        for (level = first_level; level < last_level_num; level++)
                        {
                            Matrix* t1_mp = NULL;
                            Matrix* t2_mp = NULL;
                            double  max_diff;

                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = sqrt_matrix_elements(&t1_mp,
                                                          new_var_mvp->elements[ level ]);

                            if (result == ERROR) { NOTE_ERROR(); break; }

                            result = sqrt_matrix_elements(&t2_mp,
                                                          var_mvp->elements[ level ]);

                            max_diff = max_abs_matrix_difference(t1_mp, t2_mp);

                            if (norm_var_vp != NULL)
                            {
                                max_diff /= norm_stdev;
                            }

                            max_parm_diff = MAX_OF(max_parm_diff, max_diff);

                            max_diff = max_abs_matrix_difference(new_mean_mvp->elements[ level ],
                                                                 mean_mvp->elements[ level ]);

                            if (norm_var_vp != NULL)
                            {
                                max_diff /= norm_stdev;
                            }

                            max_parm_diff = MAX_OF(max_parm_diff, max_diff);

                            free_matrix(t1_mp);
                            free_matrix(t2_mp);
                        }

                        if (result == ERROR) { NOTE_ERROR(); break; }
                    }

                    if (max_num_dis_items > 0)
                    {
                        for (level = first_level; level < last_level_num; level++)
                        {
                            max_parm_diff =
                                MAX_OF(max_parm_diff,
                                       max_abs_matrix_difference(
                                               new_P_i_n_mvp->elements[ level ],
                                               P_i_n_mvp->elements[ level ]));
                        }
                    }

                    pso(" | %9.2e", max_parm_diff);
                }

                if (fs_watch_ave_limited_error)
                {
                    pso(" | %9.2e", ave_limited_cluster_error);
                }

                if (fs_watch_max_limited_error)
                {
                    pso(" | %9.2e", max_limited_cluster_error);
                }

                pso("\n");
                kjb_flush();
            }
        }

        if (    (it > 0)
             && (output_model_fn != NULL)
             && (output_dir != NULL)
           )
        {
            Multi_modal_model* intermediate_model_ptr = NULL;

#ifdef DEF_OUT
            if (ll_rel_diff > 0.0)
            {
                ll_diff_exp = (int)(log(ll_rel_diff) / M_LN10) - 1;

                if (ll_diff_exp < min_ll_diff_exp)
                {
                    char output_sub_dir[ MAX_FILE_NAME_SIZE ];

                    if (fs_watch)
                    {
                        pso("Saving intermediate model because relative diff has droped to %.3e.\n",
                            ll_rel_diff);
                    }
                    min_ll_diff_exp = ll_diff_exp;

                    result = kjb_sprintf(output_sub_dir, sizeof(output_sub_dir),
                                         "%s%s%02d", output_dir, DIR_STR,
                                         -ll_diff_exp);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = kjb_mkdir(output_sub_dir);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = copy_multi_modal_model(&intermediate_model_ptr,
                                                     model_ptr);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    result = compute_mdl_log_likelihood(num_points,
                                                        intermediate_model_ptr);
                    if (result == ERROR) { NOTE_ERROR(); break; }

                    if ((norm_mean_vp != NULL) && (norm_var_vp != NULL))
                    {
                        result = ow_un_normalize_multi_modal_model(
                                                        intermediate_model_ptr,
                                                        norm_mean_vp,
                                                        norm_var_vp,
                                                        norm_stdev);
                        if (result == ERROR) { NOTE_ERROR(); break; }
                    }

                    result = (*output_model_fn)(intermediate_model_ptr,
                                                num_points,
                                                output_sub_dir);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }
            }
#endif

            if (    (it == 1) || (it == 2) || (it == 3)
                 || ((it < 30) && (((it + 1) % 5) == 0))
                 || (it == 6)
                 || ((it < 50) && (((it + 1) % 10) == 0))
                 || ((it < 200) && (((it + 1 ) % 20) == 0))
                 || (((it + 1 ) % 40) == 0)
               )
            {
                char output_sub_dir[ MAX_FILE_NAME_SIZE ];

                if (fs_watch)
                {
                    pso("Saving intermediate model because iteration has reached %d.\n",
                        it + 1);
                }

                result = kjb_sprintf(output_sub_dir, sizeof(output_sub_dir),
                                     "%s%s%d", output_dir, DIR_STR,
                                     1 + it);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = kjb_mkdir(output_sub_dir);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = copy_multi_modal_model(&intermediate_model_ptr,
                                                 model_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = compute_mdl_log_likelihood(num_points,
                                                    intermediate_model_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }

                if ((norm_mean_vp != NULL) && (norm_var_vp != NULL))
                {
                    result = ow_un_normalize_multi_modal_model(
                                                    intermediate_model_ptr,
                                                    norm_mean_vp,
                                                    norm_var_vp,
                                                    norm_stdev);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }

                result = (*output_model_fn)(intermediate_model_ptr,
                                            num_points,
                                            output_sub_dir);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }

            free_multi_modal_model(intermediate_model_ptr);
        }

        /*
        // Check for termination.
        */


        if ( ! current_model_is_valid)
        {
            /*EMPTY*/
            ; /* Do nothing. */
        }
        else if (iteration_atn_flag)
        {
            pso("Stopping iterating because of halt request via signal.\n");
            kjb_flush();
            break;
        }
        else if (is_file(halt_file_name))
        {
            pso("Stopping iterating because of halt request via file.\n");
            kjb_flush();
            EPE(kjb_unlink(halt_file_name));
            break;
        }
        else if (
                     (ABS_OF(ll_rel_diff) < fs_iteration_tolerance)
                  && (ll_rel_diff >= 0.0)
#ifdef SUPPORT_ANNEALING
                  && (it > fs_num_cooling_iterations)
#endif
                  && (it > fs_min_num_em_iterations)
                  && ( ! changed_number_of_parameters )
                )
        {
            break;
        }
        else if (    (options_ptr->convergence_criterion == FIRST_HELD_OUT_LL_MAX)
                  && (first_negative_held_out_ll_diff)
                )
        {
            break;
        }
        else if (options_ptr->convergence_criterion == SINGLE_ITERATION)
        {
            break;
        }

#ifdef TEST
        if (compute_cluster_dependent_aspects)
        {
            ASSERT( ! compute_aspects );
        }
        if (compute_aspects)
        {
            ASSERT( ! compute_cluster_dependent_aspects );
        }
        if (valid_cluster_dependent_aspects)
        {
            ASSERT(compute_cluster_dependent_aspects);
            ASSERT( ! valid_aspects );
        }
        if (valid_aspects)
        {
            ASSERT(compute_aspects);
            ASSERT( ! valid_cluster_dependent_aspects );
        }
#endif

        if (    (compute_cluster_dependent_aspects)
             && (!valid_cluster_dependent_aspects)
           )
        {
            valid_cluster_dependent_aspects = TRUE;
            verbose_pso(2, "Using cluster dependent aspects.\n");
        }

        if ((compute_aspects) && (!valid_aspects))
        {
            valid_aspects = TRUE;
            verbose_pso(2, "Using standard aspects.\n");
        }


#ifdef SUPPORT_ANNEALING
        if (it < fs_num_cooling_iterations)
        {
            temperature *= temperature_factor;
        }
#endif

#ifdef REPORT_CPU_TIME
        loop_cleanup_cpu += (get_cpu_time() - cpu_time_offset);
#endif

        verify_matrix(P_c_p_mp, NULL);
        verify_matrix_vector(mean_mvp, NULL);
        verify_matrix_vector(var_mvp, NULL);
        verify_matrix(V_mp, NULL);
        verify_matrix_vector(P_l_p_c_mvp, NULL);
        verify_matrix(P_l_p_mp, NULL);

        kjb_fflush((FILE*)NULL);

#ifdef XXX_LIMIT_MIN_CON_PROB
        if (    (fs_min_con_log_prob_cutoff > 0.0)
             && (fs_min_con_log_prob_cutoff < 1.0)
           )
        {
            if (num_g_probs > 0)
            {
                int index = num_g_probs * fs_min_con_log_prob_cutoff;

                dbi(num_g_probs);
                dbi(num_g_prob_resets);

                g_prob_vp->length = num_g_probs;

                ERE(vp_get_indexed_vector(&sorted_g_prob_vp, g_prob_vp));
                dbm("Begin sort");
                ERE(descend_sort_indexed_vector(sorted_g_prob_vp));
                dbm("End sort");

                min_con_log_prob = sorted_g_prob_vp->elements[ index ].element;

                dbe(min_con_log_prob);

#ifdef PLOT_MIN_CON_PROB
                ERE(plot_id = plot_open());
                ERE(plot_histogram(plot_id, g_prob_vp, 100, NULL));
#endif
            }
            else
            {
                warn_pso("No G-probs. \n");
            }
        }
#endif

#ifdef DEBUG_TRIMMING
        dbi(it);
        dbi(dis_items_used);
        dbi(total_dis_items);
        dbi(con_items_used);
        dbi(total_con_items);
#endif

    }  /* End of main EM loop. */

    CHECK_FOR_HEAP_OVERRUNS();

            dbw(); 
            dbi(result); 
    unset_iteration_atn_trap();

#ifdef REPORT_CPU_TIME
    cpu_time_offset = get_cpu_time();
#endif

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (    (result != ERROR)
         && (save_model_ptr != NULL)
       )
    {
        result = copy_multi_modal_model(&model_ptr, save_model_ptr);
    }

    dbe(model_ptr->raw_log_likelihood);

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if ((result != ERROR) && (cluster_index_mp != NULL))
    {
        UNTESTED_CODE();

        result = copy_matrix(&limited_P_c_p_mp, P_c_p_mp);

        if (result != ERROR)
        {
            P_c_p_mp->num_cols = num_clusters;

            result = ow_zero_matrix(P_c_p_mp);
        }

        if (result != ERROR)
        {
            for (point = 0; point < num_points; point++)
            {
                for (cluster_index = 0; cluster_index < cluster_index_mp->num_cols; cluster_index++)
                {
                    int index = cluster_index_mp->elements[ point ][ cluster_index ];

                    P_c_p_mp->elements[ point ][ index ] =
                                       limited_P_c_p_mp->elements[ point ][ cluster_index ];
                }
            }
        }
    }

    if (result != ERROR)
    {
        model_ptr->max_con_norm_var = fs_max_con_norm_var;

        result = compute_mdl_log_likelihood(num_points, model_ptr);
    }

    if ((result != ERROR) && (fs_save_model_data))
    {
        result = copy_cluster_data(&(model_ptr->data_ptr), data_ptr);
    }

#ifdef TRACK_MEMORY_ALLOCATION
    (void)optimize_free();
#endif

    free_matrix(limited_P_c_p_mp);
    free_vector(map_alpha_vp);
    free_v3(map_beta_minus_one_vvvp);
    free_matrix_vector(delta_inverse_mvp);
    free_matrix_vector(omega_mvp);
    free_vector(new_a_vp);
    free_vector_vector(dis_P_sum_vvp);
    free_vector_vector(con_P_sum_vvp);
    free_matrix_vector(held_out_P_l_p_c_mvp);
    free_matrix(held_out_P_l_p_mp);
    free_matrix_vector(new_P_i_n_mvp);
    free_matrix_vector(new_mean_mvp);
    free_matrix_vector(new_var_mvp);
    free_vector_vector(his_sqrt_det_vvp);
    free_3D_double_array(level_indicators_ppp);
    free_3D_double_array(pair_probs_1_ppp);
    free_3D_double_array(pair_probs_2_ppp);
    free_3D_double_array(log_pair_probs_1_ppp);
    free_3D_double_array(log_pair_probs_2_ppp);
    free_vector(temp_vp);
    free_vector(var_vp);
    free_vector(x_vp);
    free_vector(u_vp);
    free_vector(w_vp);
    free_vector(d_vp);
    free_matrix(new_V_mp);
    free_matrix(new_con_V_mp);
    free_matrix(new_dis_V_mp);
    free_vector(ll_P_c_p_vp);
    free_vector(log_prob_vp);
    free_vector(prob_vp);
#ifdef SUPPORT_ANNEALING
    free_vector(annealed_prob_vp);
    free_vector(annealed_log_prob_vp);
    free_v3(annealed_con_prob_cache_vvvp);
#endif
    free_matrix(prev_P_c_p_mp);
    free_vector(dis_factor_vp);
    free_vector(con_factor_vp);
    free_vector(vec_factor_vp);
    free_vector(his_factor_vp);
    free_vector(held_out_dis_factor_vp);
    free_vector(held_out_con_factor_vp);
    free_int_matrix(cluster_index_mp);
    free_v3(con_prob_cache_vvvp);
    free_vector(P_c_p_vp);
    free_multi_modal_model(save_model_ptr);
    free_vector(map_r_minus_n_g_vp);

    free_matrix_vector(log_pair_level_probs_mvp);
    free_int_matrix(match_mp);
    free_matrix(log_pair_probs_mp);
    free_int_vector(dis_item_match_count_vp);
    free_int_vector(con_item_match_count_vp);
    free_vector(mf_dis_item_max_log_vp);
    free_vector(mf_con_item_max_log_vp);

    free_matrix_vector(log_P_i_n_mvp);
    free_vector(con_item_max_log_vp);
    free_vector(dis_item_max_log_vp);

    free_vector_vector(new_con_log_sqrt_det_vvp);

    free_vector(nu_for_dependent_model_vp);
    free_vector(nu_based_on_dis_items_vp);
    free_vector(nu_based_on_pairs_vp);
    free_vector(log_nu_vp);
    free_vector(nu_vp);
    free_matrix(nu_with_prior_mp);
    free_vector(nu_with_prior_vp);
    free_vector(word_prior_vp);

    free_vector(log_pair_level_priors_vp);

    free_vector(dis_item_empirical_vp);

#ifdef SUPPORT_IGNORE_DIS_PROB
    free_matrix(p_use_dis_item_mp);
#endif

#ifdef XXX_LIMIT_MIN_CON_PROB
    free_vector(g_prob_vp);
    free_indexed_vector(sorted_g_prob_vp);
#endif

#ifdef TEST_WEIGHT_DATA
    free_vector(test_weight_vp);
#endif

#ifdef XXX_SUPPORT_TRIMMING
    free_matrix(con_score_mp);
    free_matrix(dis_score_mp);
    free_vector(dis_score_vp);
    free_vector(con_score_vp);
    free_vector(new_dis_score_vp);
    free_vector(new_con_score_vp);
    free_int_vector(con_score_counts_vp);
    free_int_vector(dis_score_counts_vp);
    free_indexed_vector(sorted_con_score_vp);
    free_indexed_vector(sorted_dis_score_vp);
#endif

    free_int_vector_vector(missing_index_vvp);
    free_v3(missing_con_P_sum_vvvp);

#ifdef REPORT_CPU_TIME
    cleanup_cpu += (get_cpu_time() - cpu_time_offset);

    pso("\nCPU time report");
#ifdef TEST
    pso(" (Misleading for development code)");
#endif
    pso("\n\n");

    pso("Initialization:                %6ld.%-3ld seconds.\n",
        initialization_cpu / MILLI_SECONDS_PER_SECOND,
        initialization_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += initialization_cpu;

    pso("EM loop initialization:        %6ld.%-3ld seconds.\n",
        loop_initialization_cpu / MILLI_SECONDS_PER_SECOND,
        loop_initialization_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += loop_initialization_cpu;

    pso("Vertical weight caching:       %6ld.%-3ld seconds.\n",
        vertical_weight_caching_cpu / MILLI_SECONDS_PER_SECOND,
        vertical_weight_caching_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += vertical_weight_caching_cpu;

    pso("Raw pair prob table:           %6ld.%-3ld seconds.\n",
        raw_pair_prob_table_cpu / MILLI_SECONDS_PER_SECOND,
        raw_pair_prob_table_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += raw_pair_prob_table_cpu;

    pso("Pair prob table:               %6ld.%-3ld seconds.\n",
        pair_prob_table_cpu / MILLI_SECONDS_PER_SECOND,
        pair_prob_table_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += pair_prob_table_cpu;

    pso("Pair matching:                 %6ld.%-3ld seconds.\n",
        pair_match_cpu / MILLI_SECONDS_PER_SECOND,
        pair_match_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += pair_match_cpu;

    pso("Pair item prob updating:       %6ld.%-3ld seconds.\n",
        pair_item_prob_update_cpu / MILLI_SECONDS_PER_SECOND,
        pair_item_prob_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += pair_item_prob_update_cpu;

    pso("Continuous probability cache:  %6ld.%-3ld seconds.\n",
        con_prob_cache_cpu / MILLI_SECONDS_PER_SECOND,
        con_prob_cache_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += con_prob_cache_cpu;

    pso("Discrete probability update:   %6ld.%-3ld seconds.\n",
        dis_item_prob_update_cpu / MILLI_SECONDS_PER_SECOND,
        dis_item_prob_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += dis_item_prob_update_cpu;

    pso("Continuous probability update: %6ld.%-3ld seconds.\n",
        con_item_prob_update_cpu / MILLI_SECONDS_PER_SECOND,
        con_item_prob_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += con_item_prob_update_cpu;

    pso("Log likelihood update:         %6ld.%-3ld seconds.\n",
        log_likelihood_cpu / MILLI_SECONDS_PER_SECOND,
        log_likelihood_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += log_likelihood_cpu;

    pso("Cluster membership update:     %6ld.%-3ld seconds.\n",
        cluster_membership_cpu / MILLI_SECONDS_PER_SECOND,
        cluster_membership_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += cluster_membership_cpu;

    pso("Discrete parameter update:     %6ld.%-3ld seconds.\n",
        dis_item_parameter_update_cpu / MILLI_SECONDS_PER_SECOND,
        dis_item_parameter_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += dis_item_parameter_update_cpu;

    pso("Continuous parameter update:   %6ld.%-3ld seconds.\n",
        con_item_parameter_update_cpu / MILLI_SECONDS_PER_SECOND,
        con_item_parameter_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += con_item_parameter_update_cpu;

    pso("Vertical weight update:        %6ld.%-3ld seconds.\n",
        vertical_weight_update_cpu / MILLI_SECONDS_PER_SECOND,
        vertical_weight_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += vertical_weight_update_cpu;

    pso("Held out update:               %6ld.%-3ld seconds.\n",
        held_out_cpu / MILLI_SECONDS_PER_SECOND,
        held_out_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += held_out_cpu;

    pso("Limited cluster membership:    %6ld.%-3ld seconds.\n",
        limited_cluster_cpu / MILLI_SECONDS_PER_SECOND,
        limited_cluster_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += limited_cluster_cpu;

    pso("EM loop cleanup                %6ld.%-3ld seconds.\n",
        loop_cleanup_cpu / MILLI_SECONDS_PER_SECOND,
        loop_cleanup_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += loop_cleanup_cpu;

    pso("Cleanup:                       %6ld.%-3ld seconds.\n",
        cleanup_cpu / MILLI_SECONDS_PER_SECOND,
        cleanup_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += cleanup_cpu;

#ifdef TEST
    pso("Extra test code:               %6ld.%-3ld seconds.\n",
        test_extra_cpu / MILLI_SECONDS_PER_SECOND,
        test_extra_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += test_extra_cpu;
#endif

    pso("                               -----------------\n");
    pso("                               %6ld.%-3ld seconds.\n",
        total_recorded_cpu / MILLI_SECONDS_PER_SECOND,
        total_recorded_cpu % MILLI_SECONDS_PER_SECOND);

    pso("\n");

    pso("                              (%6ld.%-3ld seconds per iteration).\n",
        (total_recorded_cpu / (1+it)) / MILLI_SECONDS_PER_SECOND,
        (total_recorded_cpu / (1+it)) % MILLI_SECONDS_PER_SECOND);

    pso("\n");

    all_cpu = get_cpu_time() - all_cpu_offset;

    pso("Total including unaccounted:   %6ld.%-3ld seconds.\n",
        all_cpu / MILLI_SECONDS_PER_SECOND,
        all_cpu % MILLI_SECONDS_PER_SECOND);

    pso("\n");

#endif

            dbw(); 
            dbi(result); 
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * This means that we want a model based on a fixed set of nodes for each word.
*/
static int do_fixed_discrete_multi_modal_clustering
(
    const Topology*         topology_ptr,
    const Cluster_data*     data_ptr,
    const Cluster_data*     held_out_data_ptr,
    Multi_modal_model*     model_ptr,
    const Training_options* options_ptr,
    int                     (*output_model_fn) (const Multi_modal_model *, int, const char *),
    const char*             output_dir,
    const Vector*           norm_mean_vp,
    const Vector*           norm_var_vp,
    double                  norm_stdev,
    int                     max_num_em_iterations
)
{
    IMPORT volatile Bool    iteration_atn_flag;
    const int               num_levels_per_category = topology_ptr->num_levels_per_category;
    const int               num_categories    = data_ptr->num_categories;
    const int               num_levels = num_levels_per_category * num_categories;
    const int               num_points        = data_ptr->num_points;
    const int               max_num_dis_items = data_ptr->max_num_dis_items;
    const int               max_num_con_items = data_ptr->max_num_con_items;
    const Int_matrix*       dis_item_mp = (max_num_dis_items == 0) ? NULL : data_ptr->dis_item_mp;
    const Matrix_vector*    con_item_mvp         = (max_num_con_items == 0) ? NULL : data_ptr->con_item_mvp;
    const int               num_con_features  = data_ptr->num_con_features;
    const Int_vector*       con_feature_enable_vp = model_ptr->con_feature_enable_vp;
    const int               num_used_con_features = (con_feature_enable_vp == NULL) ? num_con_features : sum_int_vector_elements(con_feature_enable_vp);
    const Int_vector* con_feature_group_vp = model_ptr->con_feature_group_vp;
    const Vector_vector* con_item_weight_vvp = data_ptr->con_item_weight_vvp;
    const Vector*    con_log_sqrt_det_vp = model_ptr->con_log_sqrt_det_vp;
    const Matrix*    mean_mp = model_ptr->mean_mp = NULL;
    const Matrix*    var_mp = model_ptr->var_mp = NULL;
    const int compute_aspects = ! (options_ptr->uniform_vertical_dist);
    const Matrix*    P_l_p_mp = NULL;
    const Vector* weight_vp = data_ptr->weight_vp;
    const double con_score_cutoff = DBL_HALF_MOST_NEGATIVE;
    const Vector*           V_vp = model_ptr->V_vp;
    int                     point;
    int                     dis_item;
    int                     con_item;
    int                     con_feature;
    int                     it;
    Vector*          con_P_sum_vp         = NULL;
    Vector_vector*                  missing_con_P_sum_vvp         = NULL;
    double                  log_likelihood = 0.0;
    double                  prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;
    double                  ll_rel_diff = DBL_NOT_SET;
    double                  held_out_ll_diff = DBL_NOT_SET;
    int                     result        = NO_ERROR;
    Vector*          new_con_log_sqrt_det_vp = NULL;
    Matrix*          new_mean_mp       = NULL;
    Matrix*          new_var_mp        = NULL;
    Vector*                 new_V_vp = NULL;
    double**                level_indicators_pp = NULL;
    double*                 level_indicators_p;
    int                     level;
    Vector*                 var_vp      = NULL;
    double                  held_out_log_likelihood      = DBL_HALF_MOST_NEGATIVE;
    /*
    // Use DBL_HALF_MOST_NEGATIVE divided by two for
    // held_out_prev_log_likelihood so that a return of DBL_HALF_MOST_NEGATIVE
    // looks like a decrease.
    */
    int                     need_held_out_log_likelihood = FALSE;
    int                     first_negative_held_out_ll_diff = FALSE;
    double                  con_log_norm       = 0.0;
    double                  max_con_log_prob   = 0.0;
    double                  ll_con_correction_norm_sum = 0.0;
    Vector*                 log_prob_vp          = NULL;
    Vector*                 prob_vp          = NULL;
    Vector*                 log_nu_vp            = NULL;
    Vector*                 nu_vp            = NULL;
    double                  max_parm_diff;
    Multi_modal_model* save_model_ptr = NULL;
    Vector*            fit_vp = model_ptr->fit_vp;
    double         var_offset;
    double         log_prob;
    double         log_product;
    double min_con_log_prob = DBL_HALF_MOST_NEGATIVE;
    double extra_log_con_prob_for_original_data_space = 0;
    int current_model_is_valid = FALSE;
    int i;
    Int_vector_vector* missing_index_vvp = NULL;
    int* level_index_array = NULL;
    int level_index;
#ifdef USE_STANDARD_GUASS_NORM_CONSTANT
    double                  log_scale_factor_sqrd = (double)num_used_con_features * log(2.0 * M_PI);
#else
    double                  log_scale_factor_sqrd = 0.0;
#endif
#ifdef MIN_LEVEL_INDICATOR
    /* When adding to quantities that sum to one */
    const double delta_to_ensure_min_level_indicator = MIN_LEVEL_INDICATOR / (1.0 - MIN_LEVEL_INDICATOR * num_levels);
    const double min_level_indicator_norm =
                      (1.0 + delta_to_ensure_min_level_indicator * num_levels);
#endif
#ifdef COMPUTE_HELD_OUT_LOG_LIKELIHOOD
    const int               max_num_held_out_dis_items = (held_out_data_ptr == NULL) ? 0 : held_out_data_ptr->max_num_dis_items;
    const int               max_num_held_out_con_items = (held_out_data_ptr == NULL) ? 0 : held_out_data_ptr->max_num_con_items;
    const Int_matrix*       held_out_dis_item_mp = (max_num_held_out_dis_items == 0) ? NULL : held_out_data_ptr->dis_item_mp;
    const Matrix_vector*    held_out_con_item_mvp   = (max_num_held_out_con_items == 0) ? NULL : held_out_data_ptr->con_item_mvp;
    double                  held_out_prev_log_likelihood = DBL_HALF_MOST_NEGATIVE / 2.0;
    double                  max_combined_log_likelihood  = DBL_HALF_MOST_NEGATIVE;
    double                  max_held_out_log_likelihood  = DBL_HALF_MOST_NEGATIVE;
#endif
#ifdef REPORT_CPU_TIME
    long                    cpu_time_offset;
    long                    initialization_cpu;
    long                    loop_initialization_cpu     = 0;
    long                    con_item_prob_update_cpu      = 0;
    long                    con_item_parameter_update_cpu = 0;
    long                    vertical_weight_update_cpu    = 0;
    long                    held_out_cpu        = 0;
    long                    loop_cleanup_cpu     = 0;
    long                    cleanup_cpu         = 0;
    long                    total_recorded_cpu  = 0;
    long                    all_cpu_offset = 0;
    long                    all_cpu = 0;
#endif
#ifdef REPORT_MEMORY_USE
    unsigned long total_bytes_used;
#endif


    CHECK_FOR_HEAP_OVERRUNS();

#ifdef REPORT_MEMORY_USE
    if (get_allocated_memory(&total_bytes_used) != ERROR)
    {
        dbu(total_bytes_used);
    }
#endif


#ifdef REPORT_CPU_TIME
    cpu_time_offset = get_cpu_time();
    all_cpu_offset  = cpu_time_offset;
#endif

    dbp("+++++++++++++++++++++++++++++++++++++++++++++++++++");
    dbi(num_levels);
    dbi(num_categories);
    dbp("+++++++++++++++++++++++++++++++++++++++++++++++++++");

    if (    (options_ptr->initial_model_is_subset)
         && (options_ptr->initial_model_is_valid)
       )
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }


    if (    (max_num_dis_items <= 0)
         || (max_num_con_items <= 0)
       )
    {
        set_error("Fixed discrete model requires both dis and con items.");
        return ERROR;
    }

    free_matrix(model_ptr->V_mp);
    model_ptr->V_mp = NULL;

    free_matrix_vector(model_ptr->P_i_n_mvp);
    model_ptr->P_i_n_mvp = NULL;

    if (compute_aspects)
    {
        verbose_pso(2, "Using standard aspects.\n");
    }

    if (    (options_ptr != NULL)
         && (    (options_ptr->convergence_criterion == MAX_HELD_OUT_LL)
              || (options_ptr->convergence_criterion == MAX_COMBINED_LL)
              || (options_ptr->convergence_criterion == FIRST_HELD_OUT_LL_MAX)
            )
       )
    {
        if (    (held_out_data_ptr == NULL)
             || (held_out_data_ptr->id_vp == NULL)
             || (held_out_data_ptr->id_vp->length <= 0)
           )
        {
            set_error("Phase two needs held out data but there is none.");
            return ERROR;
        }

        need_held_out_log_likelihood = TRUE;
    }

    if (options_ptr->initial_model_is_valid)
    {
        con_log_norm = model_ptr->con_log_norm;
        min_con_log_prob = model_ptr->min_con_log_prob;
    }
    else
    {
        /* One iteration is not stable unless the model is already valid. */
        max_num_em_iterations = MAX_OF(2, max_num_em_iterations);
    }


    if (norm_var_vp != NULL)
    {
        /* Aug 30, 2004; switched from num_con_features to
         * num_used_con_features.
        */
        extra_log_con_prob_for_original_data_space += num_used_con_features * log(norm_stdev);

        for (con_feature = 0; con_feature < num_con_features; con_feature++)
        {
            /* Aug 30, 2004; only use enabled features. */
            if ((con_feature_enable_vp == NULL) || (con_feature_enable_vp->elements[ con_feature ]))
            {
                extra_log_con_prob_for_original_data_space -=
                    0.5 * log(norm_var_vp->elements[ con_feature ]);
            }
        }
    }

    if (result != ERROR)
    {
        result = initialize_vector_feature_counts(con_feature_enable_vp,
                                                  con_feature_group_vp);
    }

    model_ptr->num_clusters   = 1;
    model_ptr->num_categories = num_categories;



    /* This gets updated if we are using con prob norm. */
    model_ptr->con_log_norm = con_log_norm;
    model_ptr->min_con_log_prob = min_con_log_prob;


    /*
    // In the case of "correspondence", it would be nice to write out a value
    // where word prediction goes down, if there is such a thing (it should not
    // have much of an effect on the other models, except in the case where we
    // want to say that we cannot predict anything at all). However, for calls
    // to get_cluster_membership_2 from *this* routine, con_score_cutoff should
    // be set so that it does NOT get used.
    */
    model_ptr->con_score_cutoff = con_score_cutoff;

    if (result != ERROR)
    {
        result = get_unity_vector(&(model_ptr->a_vp), 1);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        level_index_array = INT_MALLOC(num_levels);
        if (level_index_array == NULL) { result = ERROR; NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = index_matrix_vector_rows_with_missing(&missing_index_vvp,
                                                       con_item_mvp);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = copy_topology(&(model_ptr->topology_ptr), topology_ptr);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&nu_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&log_nu_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&log_prob_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_zero_vector(&prob_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

    if (result != ERROR)
    {
        result = get_target_vector(&(model_ptr->fit_vp), num_points);
        if (result == ERROR)
        {
            NOTE_ERROR();
        }
        else
        {
            fit_vp = model_ptr->fit_vp;
        }
    }

    /*
     * Memory intensive so allocate only if needed.
     *
     * Note that even if the model is valid, we wont have a
     * valid P_l_p_mp, because that depend on the data.
    */
    if (compute_aspects)
    {
        /* Must be zero'd because we only set the values for the levels
         * corresponding to observed words.
        */
        result = get_zero_matrix(&(model_ptr->P_l_p_mp),
                                 num_levels, num_points);
        if (result == ERROR)
        {
            NOTE_ERROR();
        }
        else
        {
            P_l_p_mp = model_ptr->P_l_p_mp;
        }
    }

    if (result != ERROR)
    {
        result = get_target_vector(&new_con_log_sqrt_det_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); }
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (result != ERROR)
    {
        level_indicators_pp = allocate_2D_double_array(num_levels,
                                                       max_num_con_items);
        if (level_indicators_pp == NULL)
        {
            result = ERROR;
            NOTE_ERROR();
        }
#ifdef REPORT_MEMORY_USE
        else if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            dbu(total_bytes_used);
        }
#endif
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

#ifdef TEST
    if (max_num_dis_items > 0)
    {
        ASSERT_IS_POSITIVE_INT(num_categories);
    }
#endif

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (    (result != ERROR)
         && (max_num_con_items > 0)
       )
    {
        result = get_target_vector(&new_con_log_sqrt_det_vp, num_levels);
    }

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if ((result != ERROR) && fs_watch)
    {
        int head_len = 30;

        if (need_held_out_log_likelihood) head_len += 15;
        if (need_held_out_log_likelihood) head_len += 13;
        if (fs_watch_max_con_prob) head_len += 12;
        if (fs_watch_max_cluster_given_point_diff) head_len += 12;
        if (fs_watch_rms_cluster_given_point_diff) head_len += 12;
        if (fs_watch_max_parm_diff) head_len += 12;
        if (fs_watch_ave_limited_error) head_len += 12;
        if (fs_watch_max_limited_error) head_len += 12;

        rep_print(stdout, '=', head_len);
        pso("\n");

        pso(" it  |    %s     |   diff   ", "LL/N");

        if (need_held_out_log_likelihood) pso(" | Held out LL");
        if (need_held_out_log_likelihood) pso(" |   diff   ");
        if (fs_watch_max_con_prob) pso(" | max con P");
        if (fs_watch_rms_cluster_given_point_diff) pso(" | rms P c  ");
        if (fs_watch_max_cluster_given_point_diff) pso(" | max P c  ");
        if (fs_watch_max_parm_diff) pso(" | max parm ");
        if (fs_watch_ave_limited_error) pso(" | ave lim  ");
        if (fs_watch_max_limited_error) pso(" | max lim  ");
        pso("\n");
        rep_print(stdout, '-', head_len);
        pso("\n");
    }

#ifdef REPORT_CPU_TIME
    initialization_cpu = get_cpu_time() - cpu_time_offset;
#endif

    set_iteration_atn_trap();

    /*
    //  -------------------------- Main EM Loop -------------------------------
    */

    CHECK_FOR_HEAP_OVERRUNS();

    for (it = 0; it < max_num_em_iterations; it++)
    {
        CHECK_FOR_HEAP_OVERRUNS();

#ifdef REPORT_MEMORY_USE
        if (    (result != ERROR)
             && (get_allocated_memory(&total_bytes_used) != ERROR)
           )
        {
            dbu(total_bytes_used);
        }
#endif

        var_offset = fs_var_offset + fs_var_offset_2 / (1.0 + it/5);
        var_offset *= (norm_stdev * norm_stdev);

#ifdef REPORT_CPU_TIME
        cpu_time_offset = get_cpu_time();
#endif

        /*
        // ------------------  E Step Initialization --------------------------
        */


        log_likelihood = 0.0;

        ll_con_correction_norm_sum = 0.0;
        max_con_log_prob = DBL_MOST_NEGATIVE;


        CHECK_FOR_HEAP_OVERRUNS();

        /*
        //                   M Step initializations
        */

        result = get_zero_vector(&new_V_vp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_vector(&(con_P_sum_vp), num_levels);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_target_vector_vector(&missing_con_P_sum_vvp, num_levels);
        if (result == ERROR) { NOTE_ERROR(); break; }

        for (level = 0; level < num_levels; level++)
        {
            result = get_zero_vector(&(missing_con_P_sum_vvp->elements[ level ]),
                                     num_con_features);
            if (result == ERROR) { NOTE_ERROR(); break; }
        }

        result = get_zero_matrix(&new_mean_mp, num_levels,
                                 num_con_features);
        if (result == ERROR) { NOTE_ERROR(); break; }

        result = get_zero_matrix(&new_var_mp, num_levels,
                                 num_con_features);
        if (result == ERROR) { NOTE_ERROR(); break; }


#ifdef REPORT_CPU_TIME
        loop_initialization_cpu += (get_cpu_time() - cpu_time_offset);
#endif

#ifdef REPORT_MEMORY_USE
        if (    (result != ERROR)
             && (get_allocated_memory(&total_bytes_used) != ERROR)
           )
        {
            dbu(total_bytes_used);
        }
#endif
        CHECK_FOR_HEAP_OVERRUNS();


        for (point = 0; point < num_points; point++)
        {
            double point_weight = 1.0;
            int num_dis_items = 0;
            int num_con_items = (con_item_mvp->elements[ point ] == NULL) ? 0 : con_item_mvp->elements[ point ]->num_rows;
            int num_levels_used;
            int* level_index_ptr = level_index_array;
            double level_sum = 0.0;

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif
            if (result == ERROR) { NOTE_ERROR(); break; }

            if ((fs_weight_data) && (weight_vp != NULL))
            {
                point_weight = weight_vp->elements[ point ];
            }

            for (dis_item = 0; dis_item < max_num_dis_items; dis_item++)
            {
                int category = dis_item_mp->elements[ point ][ dis_item ];

                if (category < 0) break;

                for (level = 0; level < num_levels_per_category; level++)
                {
                    *level_index_ptr = (category * num_levels_per_category) + level;
                    level_index_ptr++;
                }

                num_dis_items++;
            }

            if (options_ptr->model_correspondence > 0)
            {
                if ((num_dis_items <= 0) || (num_con_items <= 0))
                {
                    warn_pso("Point without either a dis or con item found. Skipping it");
                    continue;
                }
            }

            num_levels_used = num_dis_items * num_levels_per_category;

            /*
            // Initialize if we have not been given an initialized model. Note
            // that we essentially do the M step first, via the goto below.
             */
            if (    (it == 0)
                    && ( ! options_ptr->initial_model_is_valid )
               )
            {
                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    for (dis_item = 0; dis_item < num_dis_items; dis_item++)
                    {
                        double sum = 0.0;

                        for (i = 0; i < num_levels_per_category; i++)
                        {
                            double p = kjb_rand();

                            /* Uniform: For debugging. */
                            if (fs_initialize_method == 0)
                            {
                                p = 1.0;
                            }
                            else if (fs_initialize_method > 1)
                            {
                                p = pow(p, (double)fs_initialize_method);
                            }

                            p += 0.01 / num_levels_per_category;

                            level_indicators_pp[ dis_item * num_levels_per_category + i ][ con_item ] = p;
                            sum += p;
                        }

                        for (i = 0; i < num_levels_per_category; i++)
                        {
                            level_indicators_pp[ dis_item * num_levels_per_category + i ][ con_item ] /= sum;
                        }
                    }
                }

                /*
                 * We want to start with an M-Step, so we jump to it, skiping
                 * the first E-Step iteration.
                 */

                goto v_step;
            }

            current_model_is_valid = TRUE;  /* Log likelihood makes sense now.  */

            /*
            //                          E Step
             */

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif


            /*
            // Cache the level weights, taking logs if we are using logs for
            // probs.
             */

            for (level = 0; level < num_levels; level++)
            {
                double nu;

                if (compute_aspects)
                {
                    ASSERT_IS_LESS_INT(level , P_l_p_mp->num_rows);
                    ASSERT_IS_LESS_INT(point , P_l_p_mp->num_cols);

                    nu = P_l_p_mp->elements[ level ][ point ];

                    ASSERT_IS_NUMBER_DBL(nu);
                    ASSERT_IS_FINITE_DBL(nu);
                    ASSERT_IS_NON_NEGATIVE_DBL(nu);
                }
                else
                {
                    nu = V_vp->elements[ level ];

                    ASSERT_IS_NUMBER_DBL(nu);
                    ASSERT_IS_FINITE_DBL(nu);
                    ASSERT_IS_NON_NEGATIVE_DBL(nu);
                }

                ASSERT_IS_NUMBER_DBL(nu);
                ASSERT_IS_FINITE_DBL(nu);
                ASSERT_IS_NON_NEGATIVE_DBL(nu);

                nu_vp->elements[ level ] = nu;

                if (nu < 5.0 * DBL_MIN)
                {
                    log_nu_vp->elements[ level ] = LOG_ZERO;
                }
                else
                {
                    log_nu_vp->elements[ level ] = log(nu);
                }
            }


            log_product = 0.0;

            for (con_item = 0; con_item < num_con_items; con_item++)
            {
                double con_item_weight = 1.0;
                int missing_data = missing_index_vvp->elements[ point ]->elements[ con_item ];
                double* save_x_ptr = con_item_mvp->elements[ point ]->elements[ con_item ];
                double prob_sum = 0.0;
                double max_log_prob = DBL_MOST_NEGATIVE;
                double log_prob_sum;

                if ((model_ptr->weight_con_items) && (con_item_weight_vvp != NULL))
                {
                    static int first_time = TRUE;

                    if (first_time)
                    {
                        first_time = FALSE;
                        dbp("Weighting continous features by area.\n");
                    }

                    con_item_weight = con_item_weight_vvp->elements[point]->elements[con_item];
                }


                if (result == ERROR) { NOTE_ERROR(); break; }

                for (level = 0; level < num_levels_used; level++)
                {
                    double log_nu;
                    double log_nu_times_prob;

                    level_index = level_index_array[ level ];

                    log_nu = log_nu_vp->elements[ level_index ];

                    if (missing_data)
                    {
                        result = compute_con_prob(con_feature_enable_vp,
                                                  con_feature_group_vp,
                                                  save_x_ptr,
                                                  mean_mp->elements[ level_index ],
                                                  var_mp->elements[ level_index ],
                                                  con_log_sqrt_det_vp->elements[ level_index ],
                                                  fs_norm_continuous_features,
                                                  fs_limit_gaussian_deviation,
                                                  fs_max_con_norm_var,
                                                  fs_vector_feature_counts_ptr,
                                                  &log_prob,
                                                  &max_con_log_prob, TRUE);
                    }
                    else
                    {
                        result = compute_con_prob(con_feature_enable_vp,
                                                  con_feature_group_vp,
                                                  save_x_ptr,
                                                  mean_mp->elements[ level_index ],
                                                  var_mp->elements[ level_index ],
                                                  con_log_sqrt_det_vp->elements[ level_index ],
                                                  fs_norm_continuous_features,
                                                  fs_limit_gaussian_deviation,
                                                  fs_max_con_norm_var,
                                                  fs_vector_feature_counts_ptr,
                                                  &log_prob,
                                                  &max_con_log_prob, FALSE);
                    }

                    log_nu_times_prob = log_prob + log_nu;

                    ASSERT_IS_NUMBER_DBL(log_nu_times_prob);
                    ASSERT_IS_FINITE_DBL(log_nu_times_prob);

                    log_prob_vp->elements[ level ] = log_nu_times_prob;

                    max_log_prob = MAX_OF(max_log_prob, log_nu_times_prob);
                }


                ASSERT_IS_NUMBER_DBL(max_log_prob);
                ASSERT_IS_FINITE_DBL(max_log_prob);
                ASSERT_IS_NOT_LESS_DBL(max_log_prob, DBL_HALF_MOST_NEGATIVE);

                for (level = 0; level < num_levels_used; level++)
                {
                    double prob = exp(log_prob_vp->elements[ level ] - max_log_prob);

                    ASSERT_IS_NUMBER_DBL(log_prob_vp->elements[ level ]);
                    ASSERT_IS_FINITE_DBL(log_prob_vp->elements[ level ]);
                    ASSERT_IS_NUMBER_DBL(log_prob);
                    ASSERT_IS_FINITE_DBL(log_prob);
                    ASSERT_IS_NUMBER_DBL(prob);
                    ASSERT_IS_FINITE_DBL(prob);

                    prob_vp->elements[ level ] = prob;

                    prob_sum += prob;

                    ASSERT_IS_NUMBER_DBL(prob_sum);
                    ASSERT_IS_FINITE_DBL(prob_sum);
                }

                for (level = 0; level < num_levels_used; level++)
                {
                    level_indicators_p = level_indicators_pp[ level ];

                    ASSERT_IS_NUMBER_DBL(prob_vp->elements[ level ]);
                    ASSERT_IS_FINITE_DBL(prob_vp->elements[ level ]);
                    ASSERT_IS_NON_NEGATIVE_DBL(prob_vp->elements[ level ]);

                    ASSERT_IS_NUMBER_DBL(prob_sum);
                    ASSERT_IS_FINITE_DBL(prob_sum);
                    ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);
                    ASSERT_IS_NOT_LESS_DBL(prob_sum, 10.0 * DBL_MIN);

                    level_indicators_p[ con_item ] = prob_vp->elements[ level ] / prob_sum;

                    ASSERT_IS_NUMBER_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_FINITE_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_PROB_DBL(level_indicators_p[ con_item ]);
                }

#ifdef MIN_LEVEL_INDICATOR
                for (level = 0; level < num_levels_used; level++)
                {
                    level_indicators_p = level_indicators_pp[ level ];

                    ASSERT_IS_NUMBER_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_FINITE_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_PROB_DBL(level_indicators_p[ con_item ]);

                    level_indicators_p[ con_item ] += delta_to_ensure_min_level_indicator;
                    level_indicators_p[ con_item ] /= min_level_indicator_norm;

                    ASSERT_IS_NUMBER_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_FINITE_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_NON_NEGATIVE_DBL(level_indicators_p[ con_item ]);
                    ASSERT_IS_PROB_DBL(level_indicators_p[ con_item ]);
                }
#endif


                ASSERT_IS_NUMBER_DBL(prob_sum);
                ASSERT_IS_FINITE_DBL(prob_sum);

                if (prob_sum < 5.0 * DBL_MIN)
                {
                    warn_pso("Zero prob spotted. Making amends.\n");
                    dbe(prob_sum);
                    dbe(max_log_prob);

                    log_prob_sum = LOG_ZERO + max_log_prob;
                }
                else
                {
                    log_prob_sum = log(prob_sum) + max_log_prob;
                }

                ASSERT_IS_NUMBER_DBL(log_prob_sum);
                ASSERT_IS_FINITE_DBL(log_prob_sum);

                ASSERT_IS_NUMBER_DBL(log_prob_sum);
                ASSERT_IS_FINITE_DBL(log_prob_sum);

                log_product += log_prob_sum * con_item_weight;

                ASSERT_IS_NUMBER_DBL(log_product);
                ASSERT_IS_FINITE_DBL(log_product);

            }

#ifdef REPORT_CPU_TIME
            con_item_prob_update_cpu += get_cpu_time() - cpu_time_offset;
            cpu_time_offset = get_cpu_time();
#endif


            /*
            // I'm not sure if this is where we want the correspondence and
            // non-correspondence cases to merge.
             */

            /* --------------------------------------------------------- */

            ll_con_correction_norm_sum += (double)num_con_items;

            log_likelihood += log_product * point_weight;

            ASSERT_IS_NUMBER_DBL(log_likelihood);
            ASSERT_IS_FINITE_DBL(log_likelihood);

            /*
            // CHECK
            //
            // Perhaps we want the point_weight in the fit vector?
             */
            fit_vp->elements[ point ] = log_product;
v_step:


            /*
            //
            //                           V Step
             */

            /*
            // Compute the normalizations of the vertical weights.
             */


            for (level = 0; level < num_levels_used; level++)
            {
                double sum = 0.0;

                level_index = level_index_array[ level ];

                level_indicators_p = level_indicators_pp[ level ];

                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                    ASSERT_IS_PROB_DBL(*level_indicators_p);

                    sum += (*level_indicators_p);
                    level_indicators_p++;

                    ASSERT_IS_NUMBER_DBL(sum);
                    ASSERT_IS_FINITE_DBL(sum);
                    ASSERT_IS_NON_NEGATIVE_DBL(sum);

                }

                new_V_vp->elements[ level_index ] += (sum * point_weight);

                if (compute_aspects)
                {
                    P_l_p_mp->elements[ level_index ][ point ] = sum;
                }

                level_sum += sum;

                ASSERT_IS_NUMBER_DBL(level_sum);
                ASSERT_IS_FINITE_DBL(level_sum);
                ASSERT_IS_NON_NEGATIVE_DBL(level_sum);

            }

            if (compute_aspects)
            {
                if (level_sum > 1000000.0 * DBL_MIN)
                {
                    for (level = 0; level < num_levels_used; level++)
                    {
                        level_index = level_index_array[ level ];

                        P_l_p_mp->elements[ level_index ][ point ] /= level_sum;

                        ASSERT_IS_NUMBER_DBL(P_l_p_mp->elements[ level_index ][ point ]);
                        ASSERT_IS_FINITE_DBL(P_l_p_mp->elements[ level_index ][ point ]);
                        ASSERT_IS_NON_NEGATIVE_DBL(P_l_p_mp->elements[ level_index ][ point ]);

                    }
                }
                else
                {
                    /* This does happen. */

                    for (level = 0; level < num_levels_used; level++)
                    {
                        level_index = level_index_array[ level ];

                        P_l_p_mp->elements[ level_index ][ point ] = 1.0 / (num_levels - 0);

                        ASSERT_IS_NUMBER_DBL(P_l_p_mp->elements[ level_index ][ point ]);
                        ASSERT_IS_FINITE_DBL(P_l_p_mp->elements[ level_index ][ point ]);
                        ASSERT_IS_NON_NEGATIVE_DBL(P_l_p_mp->elements[ level_index ][ point ]);
                    }
                }
            }

            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
            /* CPU  */  vertical_weight_update_cpu += (get_cpu_time() - cpu_time_offset);
#endif


            /*
            Not valid for testing switch of num clusters.

            ASSERT_IS_EQUAL_INT(num_clusters, num_real_clusters);
             */

#ifdef TRACK_OVERRUN_BUG
            CHECK_FOR_HEAP_OVERRUNS();
#endif

            /*
            //                           M Step
             */
#ifdef REPORT_CPU_TIME
            /* CPU  */  cpu_time_offset = get_cpu_time();
#endif


            for (level = 0; level < num_levels_used; level++)
            {
                const Vector*  missing_con_P_sum_vp = missing_con_P_sum_vvp->elements[ level ];

                level_index = level_index_array[ level ];

                level_indicators_p = level_indicators_pp[ level ];

                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    double con_item_weight = 1.0;
                    double weight;
                    int missing_data = missing_index_vvp->elements[ point ]->elements[ con_item ];
                    double* x_ptr = con_item_mvp->elements[ point ]->elements[ con_item ];
                    double* u_ptr = new_mean_mp->elements[ level_index ];
                    double* v_ptr = new_var_mp->elements[ level_index ];
                    double q;

                    if ((model_ptr->weight_con_items) && (con_item_weight_vvp != NULL))
                    {
                        static int first_time = TRUE;

                        if (first_time)
                        {
                            first_time = FALSE;
                            dbp("Weighting continous features.\n");
                        }

                        con_item_weight = con_item_weight_vvp->elements[point]->elements[con_item];
                    }

                    weight = point_weight * con_item_weight;

                    ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                    ASSERT_IS_FINITE_DBL(*level_indicators_p);
                    ASSERT_IS_PROB_DBL(*level_indicators_p);

#ifdef MIN_LEVEL_INDICATOR
                    ASSERT_IS_NOT_LESS_DBL(*level_indicators_p,
                                           MIN_LEVEL_INDICATOR / 4.0);
#endif


                    q = (*level_indicators_p) * weight;

                    ASSERT_IS_NUMBER_DBL(q);
                    ASSERT_IS_FINITE_DBL(q);
                    ASSERT_IS_NON_NEGATIVE_DBL(q);
#ifdef MIN_LEVEL_INDICATOR
                    ASSERT_IS_NOT_LESS_DBL(q, 1e-10 * MIN_LEVEL_INDICATOR / 4.0);
#endif

                    /*
                     * Comment added Aug 30, 2004.
                     *
                     * Currently we do the M step over all features, not
                     * just the enabled ones. This is likely just extra
                     * work, but will have no effect. Conceivably, it
                     * will be of use to have the values of the means
                     * and the variances for the disabled features,
                     * computed using the enabled ones (a little how we
                     * were doing black and white). Since this whole
                     * feature enable/disable thing is a bit of a hack
                     * anyway, and is not a performance win, getting it
                     * correct for the few times it is used seems like a
                     * better idea than mucking with the code below for
                     * now.
                     */
                    if (missing_data)
                    {
                        for (con_feature = 0; con_feature < num_con_features; con_feature++)
                        {
                            double temp;

                            ASSERT_IS_NUMBER_DBL(*x_ptr);
                            ASSERT_IS_FINITE_DBL(*x_ptr);

                            ASSERT_IS_NUMBER_DBL(*u_ptr);
                            ASSERT_IS_FINITE_DBL(*u_ptr);

                            ASSERT_IS_NUMBER_DBL(*v_ptr);
                            ASSERT_IS_FINITE_DBL(*v_ptr);
                            ASSERT_IS_NON_NEGATIVE_DBL(*v_ptr);

                            if (IS_NOT_MISSING_DBL(*x_ptr))
                            {
                                temp = q * (*x_ptr);

                                *u_ptr += temp;
                                *v_ptr += temp * (*x_ptr);

                                missing_con_P_sum_vp->elements[ con_feature ] += q;
                            }

                            ASSERT_IS_NUMBER_DBL(*u_ptr);
                            ASSERT_IS_FINITE_DBL(*u_ptr);

                            ASSERT_IS_NUMBER_DBL(*v_ptr);
                            ASSERT_IS_FINITE_DBL(*v_ptr);
                            ASSERT_IS_NON_NEGATIVE_DBL(*v_ptr);

                            v_ptr++;
                            u_ptr++;
                            x_ptr++;
                        }
                    }
                    else
                    {
                        for (con_feature = 0; con_feature < num_con_features; con_feature++)
                        {
                            double temp;

                            ASSERT_IS_NUMBER_DBL(*x_ptr);
                            ASSERT_IS_FINITE_DBL(*x_ptr);

                            ASSERT_IS_NUMBER_DBL(*u_ptr);
                            ASSERT_IS_FINITE_DBL(*u_ptr);

                            ASSERT_IS_NUMBER_DBL(*v_ptr);
                            ASSERT_IS_FINITE_DBL(*v_ptr);
                            ASSERT_IS_NON_NEGATIVE_DBL(*v_ptr);

                            temp = q * (*x_ptr);

                            *u_ptr += temp;
                            *v_ptr += temp * (*x_ptr);

                            ASSERT_IS_NUMBER_DBL(*u_ptr);
                            ASSERT_IS_FINITE_DBL(*u_ptr);

                            ASSERT_IS_NUMBER_DBL(*v_ptr);
                            ASSERT_IS_FINITE_DBL(*v_ptr);
                            ASSERT_IS_NON_NEGATIVE_DBL(*v_ptr);

                            v_ptr++;
                            u_ptr++;
                            x_ptr++;
                        }

                        con_P_sum_vp->elements[ level_index ] += q;

                        ASSERT_IS_NUMBER_DBL(con_P_sum_vp->elements[ level_index ]);
                        ASSERT_IS_FINITE_DBL(con_P_sum_vp->elements[ level_index ]);

#ifdef MIN_LEVEL_INDICATOR
                        /* Assumes con_factor exceeds 1e-10 */
                        ASSERT_IS_NOT_LESS_DBL(con_P_sum_vp->elements[ level_index ],
                                               1e-10 * MIN_LEVEL_INDICATOR / 4.0);
                        /*
                        // Assumes con_factor exceeds 1e-10 and at least one
                        // con item per point
                         */
                        ASSERT_IS_NOT_LESS_DBL(con_P_sum_vp->elements[ level_index ],
                                               (point + 1) * 1e-10 * MIN_LEVEL_INDICATOR / 4.0);
#endif
                    }

                    level_indicators_p++;
                }
            }

#ifdef REPORT_CPU_TIME
            /* CPU  */  con_item_parameter_update_cpu += (get_cpu_time() - cpu_time_offset);
#endif
            if (result == ERROR) { NOTE_ERROR(); break; }

#ifdef REPORT_CPU_TIME
            cpu_time_offset = get_cpu_time();
#endif
        } /* Loop over points. */

        CHECK_FOR_HEAP_OVERRUNS();

#ifdef REPORT_MEMORY_USE
        if (    (result != ERROR)
             && (get_allocated_memory(&total_bytes_used) != ERROR)
           )
        {
            dbu(total_bytes_used);
        }
#endif

        verify_matrix(new_var_mp, NULL);
        verify_matrix(new_mean_mp, NULL);

        if (result == ERROR) { NOTE_ERROR(); break; }

        if ( ! current_model_is_valid)
        {
            goto loop_cleanup;
        }


        /*
        //                  Log Likelihood for this iteration.
        */

        if (log_likelihood < DBL_HALF_MOST_NEGATIVE/2.0) break;

        if (fs_norm_con_prob)
        {
            log_likelihood += con_log_norm * ll_con_correction_norm_sum;
            model_ptr->con_log_norm = con_log_norm;
            model_ptr->min_con_log_prob = min_con_log_prob;
        }

        model_ptr->con_score_cutoff = con_score_cutoff;

        if (log_likelihood < DBL_HALF_MOST_NEGATIVE/2.0) break;

        model_ptr->raw_log_likelihood = log_likelihood;

        log_likelihood /= num_points;

        if (IS_LESSER_DBL(log_likelihood, prev_log_likelihood))
        {
            warn_pso("Log likelihood is less than previous.\n");
        }

        ll_rel_diff = log_likelihood - prev_log_likelihood;

        ll_rel_diff /= (ABS_OF(log_likelihood) +  ABS_OF(prev_log_likelihood));
        ll_rel_diff *= 2.0;

        prev_log_likelihood = log_likelihood;


#ifdef REPORT_CPU_TIME
        cpu_time_offset = get_cpu_time();
#endif

#ifdef COMPUTE_HELD_OUT_LOG_LIKELIHOOD
        if (need_held_out_log_likelihood)
        {
            SET_CANT_HAPPEN_BUG();

            /*
             * Do something here to compute
            *
            *     held_out_log_likelihood
            *
            *  by a recursive call.
            */

            if (result == ERROR) { NOTE_ERROR(); break; }

            held_out_ll_diff = held_out_log_likelihood - held_out_prev_log_likelihood;

            held_out_ll_diff *= 2.0;
            held_out_ll_diff /= (ABS_OF(held_out_log_likelihood) +  ABS_OF(held_out_prev_log_likelihood));

            if (    (held_out_ll_diff < 0.0)
                 && (P_c_p_mp != NULL)  /* Must have limited membership sorted out. */
                 && (it > 1)
               )
            {
                first_negative_held_out_ll_diff = TRUE;
            }

            held_out_prev_log_likelihood = held_out_log_likelihood;

            if  (    (options_ptr->convergence_criterion == MAX_HELD_OUT_LL)
                  && (P_c_p_mp != NULL)  /* Must have limited membership sorted out. */
                  && (held_out_log_likelihood > max_held_out_log_likelihood)
                )
            {
                max_held_out_log_likelihood = held_out_log_likelihood;

                result = copy_multi_modal_model(&save_model_ptr, model_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }

            if  (    (options_ptr->convergence_criterion == MAX_COMBINED_LL)
                  && (P_c_p_mp != NULL)  /* Must have limited membership sorted out. */
                  && (log_likelihood + held_out_log_likelihood > max_combined_log_likelihood)
                )
            {
                max_combined_log_likelihood = log_likelihood + held_out_log_likelihood;

                result = copy_multi_modal_model(&save_model_ptr, model_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }
        }
#endif

        if ((fs_norm_con_prob) && (ABS_OF(max_con_log_prob) > 1.0))
        {
            con_log_norm += max_con_log_prob;
        }


#ifdef REPORT_CPU_TIME
        held_out_cpu += (get_cpu_time() - cpu_time_offset);
#endif

loop_cleanup:

        CHECK_FOR_HEAP_OVERRUNS();

        verify_matrix(new_var_mp, NULL);
        verify_matrix(new_mean_mp, NULL);

        /*
        //                           V Step Cleanup
        */
#ifdef REPORT_CPU_TIME
        cpu_time_offset = get_cpu_time();
#endif

        if (result == ERROR) { NOTE_ERROR(); break; }

        result = ow_scale_vector_by_sum(new_V_vp);
        if (result == ERROR) { NOTE_ERROR(); break; }

        /*
        //                           M Step Cleanup
        */


        if (result == ERROR) { NOTE_ERROR(); break; }

        for (level_index = 0; level_index < num_levels; level_index++)
        {
            const Vector* missing_con_P_sum_vp = missing_con_P_sum_vvp->elements[ level_index ];
            double s = con_P_sum_vp->elements[ level_index ];

            if (result == ERROR) { NOTE_ERROR(); break; }


            ASSERT_IS_NUMBER_DBL(s);
            ASSERT_IS_FINITE_DBL(s);
            ASSERT_IS_NON_NEGATIVE_DBL(s);
            /*
            // Assumes con_factor exceeds 1e-10 and at least one
            // con item per point.
             */

            for (con_feature = 0; con_feature < num_con_features; con_feature++)
            {
                ASSERT_IS_NOT_LESS_DBL(s + missing_con_P_sum_vp->elements[ con_feature ], 10.0 * DBL_MIN);
#ifdef MIN_LEVEL_INDICATOR
                ASSERT_IS_NOT_LESS_DBL(s + missing_con_P_sum_vp->elements[ con_feature ], num_points * 1e-10 * MIN_LEVEL_INDICATOR * MIN_HORIZONTAL_INDICATOR / 4.0);
#endif
                new_mean_mp->elements[ level_index ][ con_feature ] /=
                    (s + missing_con_P_sum_vp->elements[ con_feature ]);
            }

            /*
             * Comment added Aug 30, 2004.
             *
             * Currently we do the M step over all features, not just
             * the enabled ones. This is likely just extra work, but
             * will have no effect. Conceivably, it will be of use to
             * have the values of the means and the variances for the
             * disabled features, computed using the enabled ones (a
             * little how we were doing black and white). Since this
             * whole feature enable/disable thing is a bit of a hack
             * anyway, and is not a performance win, getting it correct
             * for the few times it is used seems like a better idea
             * than mucking with the code below for now.
             */
            for (con_feature = 0; con_feature < num_con_features; con_feature++)
            {
                double u = new_mean_mp->elements[ level_index ][ con_feature ];
                double u2 = (s + missing_con_P_sum_vp->elements[ con_feature ]) * u * u;
                double var = new_var_mp->elements[ level_index ][ con_feature ] - u2;

                if (var < 0.0)
                {
                    /*
                    // This does happen! The calculation of variance
                    // this way is not numerically stable.
                     */
                    var = 0.0;
                }

                new_var_mp->elements[ level_index ][ con_feature ] = var;

                ASSERT_IS_NUMBER_DBL(var);
                ASSERT_IS_FINITE_DBL(var);
                ASSERT_IS_NON_NEGATIVE_DBL(var);
            }

            for (con_feature = 0; con_feature < num_con_features; con_feature++)
            {
                new_var_mp->elements[ level_index ][ con_feature ] /=
                    (s + missing_con_P_sum_vp->elements[ con_feature ]);
            }

            result = ow_add_scalar_to_matrix_row(new_var_mp, var_offset, level_index);
            if (result == ERROR) { NOTE_ERROR(); break; }

            if (fs_tie_var)
            {
                double ave_var = 0.0;

                /*
                 * Comment added Aug 30, 2004.
                 *
                 * Currently we do the M step over all features, not
                 * just the enabled ones. This is likely just extra
                 * work, but will have no effect. Conceivably, it will
                 * be of use to have the values of the means and the
                 * variances for the disabled features, computed using
                 * the enabled ones (a little how we were doing black
                 * and white). Since this whole feature enable/disable
                 * thing is a bit of a hack anyway, and is not a
                 * performance win, getting it correct for the few times
                 * it is used seems like a better idea than mucking with
                 * the code below for now.
                 */
                for (con_feature = 0; con_feature < num_con_features; con_feature++)
                {
                    ave_var += new_var_mp->elements[ level_index ][ con_feature ];
                }

                ave_var /= num_con_features;

                for (con_feature = 0; con_feature < num_con_features; con_feature++)
                {
                    new_var_mp->elements[ level_index ][ con_feature ] = ave_var;
                }
            }
            else
            {
                /* CHECK -- added block for nulls, april 05, 2004 */

                /* CHECK -- On 05-01-30, this looks a bit puzzling. We
                 *          are overwriting the data!
                 */
                if (    ( ! fs_vector_feature_counts_set)
                        && (con_feature_enable_vp != NULL)
                        && (con_feature_group_vp != NULL)
                   )
                {
                    UNTESTED_CODE();

                    ERE(initialize_vector_feature_counts(con_feature_enable_vp,
                                                         con_feature_group_vp));
                }

                if (fs_vector_feature_counts_ptr != NULL)
                {
                    double ave_var = 0.0;

                    con_feature = 0;

                    while (con_feature < num_con_features)
                    {
                        int count = 0;

                        if (con_feature_group_vp->elements[ con_feature ] >= 0)
                        {
                            con_feature++;
                            continue;
                        }

                        for (i = 0; i < num_con_features - con_feature; i++)
                        {
                            if (con_feature_group_vp->elements[ con_feature ] != con_feature_group_vp->elements[ con_feature + i])
                            {
                                break;
                            }
                            count++;
                        }

                        for (i = 0; i < count; i++)
                        {
                            ave_var += new_var_mp->elements[ level_index ][ con_feature + i ];
                        }

                        ave_var /= count;

                        for (i = 0; i < count; i++)
                        {
                            new_var_mp->elements[ level_index ][ con_feature + i ] = ave_var;
                        }
                    }
                }
            }

            new_con_log_sqrt_det_vp->elements[ level_index ] = log_scale_factor_sqrd;

            for (con_feature = 0; con_feature < num_con_features; con_feature++)
            {
                double var;

                if ((con_feature_enable_vp != NULL) && ( ! con_feature_enable_vp->elements[ con_feature ]))
                {
                    continue;
                }

                var = new_var_mp->elements[ level_index ][ con_feature ];

                ASSERT_IS_NOT_LESS_DBL(var, var_offset);

                if (var < 3.0 * DBL_MIN)
                {
                    /*
                    // Assuming var_offset is reasonable, this can't
                    // happen!
                     */
                    dbe(var);
                    new_con_log_sqrt_det_vp->elements[ level_index ] += LOG_ZERO;
                }
                else
                {
                    new_con_log_sqrt_det_vp->elements[ level_index ] += log(var);
                }
            }

            /* Now make it the log of the square root. */
            new_con_log_sqrt_det_vp->elements[ level_index ] /= 2.0;

            /* And normalize. */
            new_con_log_sqrt_det_vp->elements[ level_index ] += con_log_norm;

            new_con_log_sqrt_det_vp->elements[ level_index ] -= extra_log_con_prob_for_original_data_space;

            ASSERT_IS_NUMBER_DBL(new_con_log_sqrt_det_vp->elements[ level_index ]);
            ASSERT_IS_FINITE_DBL(new_con_log_sqrt_det_vp->elements[ level_index ]);
        }
        verify_matrix(new_mean_mp, NULL);
        verify_matrix(new_var_mp, NULL);


        /*
        // Final updates.
        */

        result = copy_matrix(&(model_ptr->mean_mp), new_mean_mp);
        if (result == ERROR) { NOTE_ERROR(); break; }
        mean_mp = model_ptr->mean_mp;

        result = copy_matrix(&(model_ptr->var_mp), new_var_mp);
        if (result == ERROR) { NOTE_ERROR(); break; }
        var_mp = model_ptr->var_mp;

        result = copy_vector(&(model_ptr->con_log_sqrt_det_vp),
                               new_con_log_sqrt_det_vp);
        if (result == ERROR) { NOTE_ERROR(); break; }
        con_log_sqrt_det_vp = model_ptr->con_log_sqrt_det_vp;

        result = copy_vector(&(model_ptr->V_vp), new_V_vp);
        if (result == ERROR) { NOTE_ERROR(); break; }
        V_vp = model_ptr->V_vp;

        /* ------------------ End of M Step cleanup ------------------------ */


        /*
        // Print stats and compute values needed to determine convergence before
        // final update.
        */

        if (current_model_is_valid)
        {
            if (fs_watch)
            {
                pso(" %-3d | %11.4e | %9.2e", 1 + it,
                    log_likelihood,
                    ll_rel_diff);

                if (need_held_out_log_likelihood)
                {
                    pso(" | %11.4e | %9.2e",
                        held_out_log_likelihood / held_out_data_ptr->num_points,
                        held_out_ll_diff);
                }
            }

            if (fs_watch)
            {
                if (fs_watch_max_con_prob) pso(" | %9.2e", max_con_log_prob);

                if (fs_watch_max_parm_diff)
                {
                    max_parm_diff = DBL_MOST_NEGATIVE;

                    if (max_num_con_items > 0)
                    {
                        Matrix* t1_mp = NULL;
                        Matrix* t2_mp = NULL;
                        double  max_diff;

                        if (result == ERROR) { NOTE_ERROR(); break; }

                        result = sqrt_matrix_elements(&t1_mp,
                                                      new_var_mp);

                        if (result == ERROR) { NOTE_ERROR(); break; }

                        result = sqrt_matrix_elements(&t2_mp,
                                                      var_mp);

                        max_diff = max_abs_matrix_difference(t1_mp, t2_mp);

                        if (norm_var_vp != NULL)
                        {
                            max_diff /= norm_stdev;
                        }

                        max_parm_diff = MAX_OF(max_parm_diff, max_diff);

                        max_diff = max_abs_matrix_difference(new_mean_mp,
                                                             mean_mp);

                        if (norm_var_vp != NULL)
                        {
                            max_diff /= norm_stdev;
                        }

                        max_parm_diff = MAX_OF(max_parm_diff, max_diff);

                        free_matrix(t1_mp);
                        free_matrix(t2_mp);
                    }

                    if (result == ERROR) { NOTE_ERROR(); break; }

                    pso(" | %9.2e", max_parm_diff);
                }

                pso("\n");
                kjb_flush();
            }
        }

        if (    (it > 0)
             && (output_model_fn != NULL)
             && (output_dir != NULL)
           )
        {
            Multi_modal_model* intermediate_model_ptr = NULL;

            if (    (it == 1) || (it == 2) || (it == 3)
                 || ((it < 30) && (((it + 1) % 5) == 0))
                 || (it == 6)
                 || ((it < 50) && (((it + 1) % 10) == 0))
                 || ((it < 200) && (((it + 1 ) % 20) == 0))
                 || (((it + 1 ) % 40) == 0)
               )
            {
                char output_sub_dir[ MAX_FILE_NAME_SIZE ];

                if (fs_watch)
                {
                    pso("Saving intermediate model because iteration has reached %d.\n",
                        it + 1);
                }

                result = kjb_sprintf(output_sub_dir, sizeof(output_sub_dir),
                                     "%s%s%d", output_dir, DIR_STR,
                                     1 + it);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = kjb_mkdir(output_sub_dir);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = copy_multi_modal_model(&intermediate_model_ptr,
                                                 model_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }

                result = compute_mdl_log_likelihood(num_points,
                                                    intermediate_model_ptr);
                if (result == ERROR) { NOTE_ERROR(); break; }

                if ((norm_mean_vp != NULL) && (norm_var_vp != NULL))
                {
                    result = ow_un_normalize_multi_modal_model(
                                                    intermediate_model_ptr,
                                                    norm_mean_vp,
                                                    norm_var_vp,
                                                    norm_stdev);
                    if (result == ERROR) { NOTE_ERROR(); break; }
                }

                result = (*output_model_fn)(intermediate_model_ptr,
                                            num_points,
                                            output_sub_dir);
                if (result == ERROR) { NOTE_ERROR(); break; }
            }

            free_multi_modal_model(intermediate_model_ptr);
        }

        /*
        // Check for termination.
        */

        if ( ! current_model_is_valid)
        {
            /*EMPTY*/
            ; /* Do nothing. */
        }
        else if (iteration_atn_flag)
        {
            pso("Stopping iterating because of halt request via signal.\n");
            break;
        }
        else if (
                     (ABS_OF(ll_rel_diff) < fs_iteration_tolerance)
                  && (ll_rel_diff >= 0.0)
                  && (it > fs_min_num_em_iterations)
                )
        {
            break;
        }
        else if (    (options_ptr->convergence_criterion == FIRST_HELD_OUT_LL_MAX)
                  && (first_negative_held_out_ll_diff)
                )
        {
            break;
        }
        else if (options_ptr->convergence_criterion == SINGLE_ITERATION)
        {
            break;
        }

#ifdef REPORT_CPU_TIME
        loop_cleanup_cpu += (get_cpu_time() - cpu_time_offset);
#endif

        verify_matrix(mean_mp, NULL);
        verify_matrix(var_mp, NULL);
        verify_vector(V_vp, NULL);
        verify_matrix(P_l_p_mp, NULL);

        kjb_fflush((FILE*)NULL);

    }  /* End of main EM loop. */

    CHECK_FOR_HEAP_OVERRUNS();

    unset_iteration_atn_trap();

#ifdef REPORT_CPU_TIME
    cpu_time_offset = get_cpu_time();
#endif

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (    (result != ERROR)
         && (save_model_ptr != NULL)
       )
    {
        result = copy_multi_modal_model(&model_ptr, save_model_ptr);
    }

    dbe(model_ptr->raw_log_likelihood);

#ifdef REPORT_MEMORY_USE
    if (    (result != ERROR)
         && (get_allocated_memory(&total_bytes_used) != ERROR)
       )
    {
        dbu(total_bytes_used);
    }
#endif

    if (result != ERROR)
    {
        model_ptr->max_con_norm_var = fs_max_con_norm_var;

        result = compute_mdl_log_likelihood(num_points, model_ptr);
    }

    if ((result != ERROR) && (fs_save_model_data))
    {
        result = copy_cluster_data(&(model_ptr->data_ptr), data_ptr);
    }

#ifdef TRACK_MEMORY_ALLOCATION
    (void)optimize_free();
#endif

    free_vector(new_con_log_sqrt_det_vp);
    free_matrix(new_mean_mp);
    free_matrix(new_var_mp);
    free_vector(new_V_vp);

    kjb_free(level_index_array);

    free_2D_double_array(level_indicators_pp);

    free_vector(con_P_sum_vp);
    free_vector(var_vp);
    free_vector(log_prob_vp);
    free_vector(prob_vp);
    free_vector(log_nu_vp);
    free_vector(nu_vp);
    free_multi_modal_model(save_model_ptr);

    free_int_vector_vector(missing_index_vvp);
    free_vector_vector(missing_con_P_sum_vvp);

#ifdef REPORT_CPU_TIME
    cleanup_cpu += (get_cpu_time() - cpu_time_offset);

    pso("\nCPU time report");
#ifdef TEST
    pso(" (Misleading for development code)");
#endif
    pso("\n\n");

    pso("Initialization:                %6ld.%-3ld seconds.\n",
        initialization_cpu / MILLI_SECONDS_PER_SECOND,
        initialization_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += initialization_cpu;

    pso("EM loop initialization:        %6ld.%-3ld seconds.\n",
        loop_initialization_cpu / MILLI_SECONDS_PER_SECOND,
        loop_initialization_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += loop_initialization_cpu;

    pso("Continuous probability update: %6ld.%-3ld seconds.\n",
        con_item_prob_update_cpu / MILLI_SECONDS_PER_SECOND,
        con_item_prob_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += con_item_prob_update_cpu;

    pso("Continuous parameter update:   %6ld.%-3ld seconds.\n",
        con_item_parameter_update_cpu / MILLI_SECONDS_PER_SECOND,
        con_item_parameter_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += con_item_parameter_update_cpu;

    pso("Vertical weight update:        %6ld.%-3ld seconds.\n",
        vertical_weight_update_cpu / MILLI_SECONDS_PER_SECOND,
        vertical_weight_update_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += vertical_weight_update_cpu;

    pso("Held out update:               %6ld.%-3ld seconds.\n",
        held_out_cpu / MILLI_SECONDS_PER_SECOND,
        held_out_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += held_out_cpu;

    pso("EM loop cleanup                %6ld.%-3ld seconds.\n",
        loop_cleanup_cpu / MILLI_SECONDS_PER_SECOND,
        loop_cleanup_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += loop_cleanup_cpu;

    pso("Cleanup:                       %6ld.%-3ld seconds.\n",
        cleanup_cpu / MILLI_SECONDS_PER_SECOND,
        cleanup_cpu % MILLI_SECONDS_PER_SECOND);
    total_recorded_cpu += cleanup_cpu;

    pso("                               -----------------\n");
    pso("                               %6ld.%-3ld seconds.\n",
        total_recorded_cpu / MILLI_SECONDS_PER_SECOND,
        total_recorded_cpu % MILLI_SECONDS_PER_SECOND);

    pso("\n");

    pso("                              (%6ld.%-3ld seconds per iteration).\n",
        (total_recorded_cpu / (1+it)) / MILLI_SECONDS_PER_SECOND,
        (total_recorded_cpu / (1+it)) % MILLI_SECONDS_PER_SECOND);

    pso("\n");

    all_cpu = get_cpu_time() - all_cpu_offset;

    pso("Total including unaccounted:   %6ld.%-3ld seconds.\n",
        all_cpu / MILLI_SECONDS_PER_SECOND,
        all_cpu % MILLI_SECONDS_PER_SECOND);

    pso("\n");

#endif

    kjb_fflush((FILE*)NULL);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Likely defects:
 *    (1) The fit value does not take into account con_item_weights
 */

int get_cluster_membership
(
    Matrix**                  P_c_p_mpp,
    Matrix_vector**           P_l_p_c_mvpp,
    Int_matrix_vector**       cluster_level_index_mvpp,
    Matrix**                  P_l_p_mpp,
    Int_matrix**              level_index_mpp,
    Vector**                  fit_vp_ptr,
    const Multi_modal_model*  model_ptr,
    const Cluster_data*       data_ptr,
    const Int_vector*         points_to_use_vp,
    int                       use_factors_flag,
    int                       test_data_flag,
    Vector_vector*            con_score_vpp
)
{
    const Int_vector* con_feature_enable_vp    = model_ptr->con_feature_enable_vp;
    const Int_vector* con_feature_group_vp     = model_ptr->con_feature_group_vp;
    const int         weight_con_items         = model_ptr->weight_con_items;
    const int         norm_depend              = model_ptr->norm_depend;
    const int         model_correspondence     = model_ptr->phase_two_model_correspondence;
    const double      con_prob_boost_factor    = model_ptr->con_prob_boost_factor;
    const double      dis_prob_boost_factor    = model_ptr->dis_prob_boost_factor;
    const int         norm_items               = model_ptr->norm_items;
    const int         norm_continuous_features = model_ptr->norm_continuous_features;
    const Topology*   topology_ptr             = model_ptr->topology_ptr;
    const Matrix*     V_mp = model_ptr->V_mp;
    const Matrix_vector* input_P_l_p_c_mvp = model_ptr->P_l_p_c_mvp;
    const Matrix*        input_P_c_p_mp    = model_ptr->P_c_p_mp;
    const Matrix*        input_P_l_p_mp    = model_ptr->P_l_p_mp;
    const Vector*        a_vp      = model_ptr->a_vp;
    const Matrix_vector* P_i_n_mvp = model_ptr->P_i_n_mvp;
    const Matrix_vector* mean_mvp  = model_ptr->mean_mvp;
    const Matrix_vector* var_mvp   = model_ptr->var_mvp;
    const Vector_vector* con_log_sqrt_det_vvp = model_ptr->con_log_sqrt_det_vvp;
    const double         con_log_norm         = model_ptr->con_log_norm;
    const double         con_score_cutoff     = model_ptr->con_score_cutoff;
    const double         min_con_log_prob     = model_ptr->min_con_log_prob;
    const double         max_con_norm_var     = model_ptr->max_con_norm_var;
    const int            max_num_dis_items    = data_ptr->max_num_dis_items;
    const int            max_num_con_items    = data_ptr->max_num_con_items;
    const Int_matrix*    dis_item_mp          = (max_num_dis_items == 0) ? NULL : data_ptr->dis_item_mp;
    const Matrix*        dis_item_multiplicity_mp = (max_num_dis_items == 0) ? NULL : data_ptr->dis_item_multiplicity_mp;
    const double         max_num_dis_items_with_multiplicity = data_ptr->max_num_dis_items_with_multiplicity;
    const Matrix_vector* con_item_mvp        = (max_num_con_items == 0) ? NULL : data_ptr->con_item_mvp;
    const int            num_categories      = data_ptr->num_categories;
    Vector*              dis_factor_vp       = NULL;
    Vector*              con_factor_vp       = NULL;
    double               prev_log_likelihood = DBL_HALF_MOST_NEGATIVE;


    TEST_PSO(("Level prior not implemented. \n"));

    ASSERT(KJB_IS_SET(norm_continuous_features));
    ASSERT(KJB_IS_SET(norm_items));


    /* -------------------   Verify input --------------------------------  */

    /*
     * We need (more) extensive testing here because the data and the model can be
     * inconsistent due to inconsistent files (i.e. user error). Thus simply
     * checking with ASSERTS is not sufficient.
    */

    if (dis_item_mp != NULL)
    {
        int max;
        int max_i;
        int max_j;

        ERE(get_max_int_matrix_element(dis_item_mp, &max, &max_i, &max_j));

        if (max >= num_categories)
        {
            set_error("Discrete item label (%d) not less then number of categories (%d) ",
                      max, num_categories);
            add_error("found on row %d and col %d of sparce discrete item matrix.",
                      max_i + 1, max_j + 1);
            return ERROR; 
        }
    }

    /* --------------------------------------------------------------------- */

    if ((max_num_dis_items > 0) && (use_factors_flag))
    {
        ERE(get_dis_factors(&dis_factor_vp,
                            dis_item_mp,
                            dis_item_multiplicity_mp,
                            max_num_dis_items_with_multiplicity,
                            norm_items,
                            dis_prob_boost_factor));
    }

    if ((max_num_con_items > 0) && (use_factors_flag))
    {
        ERE(get_con_factors(&con_factor_vp,
                            con_item_mvp,
                            max_num_con_items,
                            norm_items,
                            con_prob_boost_factor));
    }

    set_iteration_atn_trap();

    ERE(get_cluster_membership_2(P_c_p_mpp,
                                 P_l_p_c_mvpp,
                                 cluster_level_index_mvpp,
                                 P_l_p_mpp,
                                 level_index_mpp,
                                 fit_vp_ptr,
                                 topology_ptr,
                                 a_vp,
                                 V_mp,
                                 input_P_c_p_mp,
                                 input_P_l_p_c_mvp,
                                 input_P_l_p_mp,
                                 P_i_n_mvp,
                                 mean_mvp, var_mvp,
                                 con_log_sqrt_det_vvp,
                                 con_log_norm,
                                 con_feature_enable_vp,
                                 con_feature_group_vp,
                                 min_con_log_prob,
                                 max_con_norm_var,
                                 con_score_cutoff,
                                 weight_con_items,
                                 norm_depend,
                                 model_correspondence,
                                 data_ptr,
                                 points_to_use_vp,
                                 dis_factor_vp, con_factor_vp,
                                 norm_continuous_features,
                                 fs_iteration_tolerance,
                                 prev_log_likelihood,
                                 (double*)NULL,
                                 model_ptr->P_c_p_mp,
                                 test_data_flag,
                                 con_score_vpp));

    unset_iteration_atn_trap();

    free_vector(dis_factor_vp);
    free_vector(con_factor_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_cluster_membership_2
(
    Matrix**             P_c_p_mpp,
    Matrix_vector**      P_l_p_c_mvpp,
    Int_matrix_vector**  cluster_level_index_mvpp,
    Matrix**             P_l_p_mpp,
    Int_matrix**         level_index_mpp,
    Vector**             fit_vp_ptr,
    const Topology*      topology_ptr,
    const Vector*        a_vp,
    const Matrix*        V_mp,
    const Matrix*        input_P_c_p_mp,
    const Matrix_vector* input_P_l_p_c_mvp,
    const Matrix*        input_P_l_p_mp,
    const Matrix_vector* P_i_n_mvp,
    const Matrix_vector* mean_mvp,
    const Matrix_vector* var_mvp,
    const Vector_vector* con_log_sqrt_det_vvp,
    double               con_log_norm,
    const Int_vector*    con_feature_enable_vp, 
    const Int_vector*    con_feature_group_vp, 
    double               __attribute__((unused)) min_con_log_prob,
    double               __attribute__((unused)) max_con_norm_var,
    double               con_score_cutoff,
    int                  weight_con_items,
    int                  norm_depend,
    int                  model_correspondence,
    const Cluster_data*  data_ptr,
    const Int_vector*    points_to_use_vp,
    const Vector*        dis_factor_vp,
    const Vector*        con_factor_vp,
    int                  norm_continuous_features,
    double               iteration_tolerance,
    double               prev_log_likelihood,
    double*              log_likelihood_ptr,
    const Matrix*        test_P_c_p_mp,
    int                  test_data_flag,
    Vector_vector*       __attribute__((unused)) dummy_con_score_vpp
)
{
    IMPORT volatile Bool    iteration_atn_flag;
    const int         num_levels        = topology_ptr->num_levels;
    const int         first_level       = topology_ptr->first_level;
    const int         last_level_num       = topology_ptr->last_level_num;
    const Int_matrix* node_mp           = topology_ptr->node_mp;
    const Int_vector* level_counts_vp   = topology_ptr->level_counts_vp;
    const int         num_clusters      = topology_ptr->num_clusters;
    const int         max_num_dis_items = (P_i_n_mvp == NULL) ? 0 : data_ptr->max_num_dis_items;
    const int         max_num_con_items = (mean_mvp == NULL) ? 0 : data_ptr->max_num_con_items;
    const int         max_num_items         = max_num_dis_items + max_num_con_items;
    const Int_matrix* dis_item_mp       = (max_num_dis_items == 0) ? NULL : data_ptr->dis_item_mp;
    const Matrix*     dis_item_multiplicity_mp    = (max_num_dis_items == 0) ? NULL : data_ptr->dis_item_multiplicity_mp;
#ifdef XXX_NEVER_USED
    const Matrix*     dis_category_level_prior_mp = (max_num_dis_items == 0) ? NULL : data_ptr->dis_category_level_prior_mp;
#endif
    const Matrix_vector* con_item_mvp     = (max_num_con_items == 0) ? NULL : data_ptr->con_item_mvp;
#ifdef HOW_IT_WAS_05_01_29
    const int            num_con_features = data_ptr->num_con_features;
#endif
#ifdef TEST
    const int num_categories = data_ptr->num_categories;
#endif
    const Vector_vector* con_item_weight_vvp = data_ptr->con_item_weight_vvp;
    /*
    // FIX this HACK.
    //
    // We need a concept of which model of corresponcence was last used. I
    // don't know if it is quite dealt with, as I am not sure if we are forced
    // to do any phase two interations.
    */
    int                  num_points       = data_ptr->num_points;
    int                  level;
    int                  cluster;
    int                  point;
    int                  dis_item;
    int                  con_item;
    Vector*              fit_vp           = NULL;
    Vector*              col_sum_vp       = NULL;
    Matrix*              P_c_p_mp         = NULL;
#ifdef TEST
    int                  item;
#endif
    const Matrix_vector* P_l_p_c_mvp;
    Matrix* P_l_p_mp;
    int                  it;
    Vector*              nu_vp            = NULL;
    Vector*              log_nu_vp        = NULL;
    double               log_likelihood   = DBL_HALF_MOST_NEGATIVE;
    Vector*              log_prob_vp      = NULL;
    double               ll_rel_diff;
    double               dis_item_multiplicity;
    double***            level_indicators_ppp;
    double**             level_indicators_pp;
    double*              level_indicators_p;
    double               ll_con_correction_norm_sum = 0.0;
#ifdef TEST
    int                  max_index = NOT_SET;
#endif

    double         dis_factor;
    double         con_factor;
    double         max_log_prob;
    double*        P_c_p_ptr;
    double         fit_temp;
    double         prob_sum;
    int            num_con_items;
    double*        save_x_ptr;
    Vector_vector* con_prob_cache_vvp;
    int            num_nodes_this_level;
    double*        con_prob_cache_ptr;
    int            node;
#ifdef HOW_IT_WAS_05_01_29
    double         d;
    double*        u_ptr;
    double*        v_ptr;
    double*        x_ptr;
    double         norm;
    double         dev;
    double         v;
    int            con_feature;
#endif
    double         temp;
    double         log_prob;
    int            dis_item_found;
    double         log_product;
    double         product;
    double         log_dis_prod;
    double         log_con_prod;
#ifdef TEST
    double         sum_nu;
#endif
    int            category;
    double         level_sum;
    double         log_prob_sum;
    double         nu;
    double         prob;
    double log_nu;
    double exp_temp;
    double exp_sum;
    double p;
    double rms_diff;
    double max_diff;
    char user_id[ 1000 ];
    char halt_file_name[ MAX_FILE_NAME_SIZE ];
    const Matrix* lock_P_c_p_mp = NULL;
    Vector* nu_for_dependent_model_vp = NULL;
    Int_vector* con_score_counts_vp = NULL;
#ifdef XXX_NOT_SURE_WHAT_IT_IS_DOING
    Matrix* con_score_mp = NULL;
#endif
    Vector* con_score_vp = NULL;  /* How well is a con_item (blob) explained. */
    int num_sparse_levels = num_levels;
    int first_sparse_level = first_level;
    int  sparse_level;
    const Int_matrix* level_index_mp = NULL;
    Indexed_vector* sorted_l_p_vp = NULL;
    Vector* level_weight_vp = NULL;


    BUFF_GET_USER_ID(user_id);
    BUFF_CPY(halt_file_name, TEMP_DIR);
    BUFF_CAT(halt_file_name, user_id);
    BUFF_CAT(halt_file_name, DIR_STR);
    BUFF_CAT(halt_file_name, "halt-hc");


    if (    (    (level_index_mpp != NULL)
              || (cluster_level_index_mvpp != NULL)
            )
         &&
            (fs_max_num_level_weights > 0)
       )
    {
        if (cluster_level_index_mvpp != NULL)
        {
            UNTESTED_CODE();
        }

        dbp("-----------------------------------------");
        dbw();
        dbp("Using sparse levels");

        num_sparse_levels = MIN_OF(num_levels,
                                   fs_max_num_level_weights);

        dbi(num_levels);
        dbi(num_sparse_levels);

        first_sparse_level = 0;

        if (level_index_mpp != NULL)
        {
#ifdef TEST
            ERE(get_initialized_int_matrix(level_index_mpp,
                                           num_sparse_levels,
                                           num_points,
                                           NOT_SET));
#else
            ERE(get_target_int_matrix(level_index_mpp,
                                      num_sparse_levels,
                                      num_points));
#endif
            level_index_mp = *level_index_mpp;
        }
        else
        {
            dbp("");
            dbp("UNTESTED");
            dbp("Since level_index_mpp is NULL, we have no idea what is going to happen.");
            dbp("UNTESTED");
            dbp("");
        }

        dbp("-----------------------------------------");
    }

    ERE(get_zero_vector(&level_weight_vp, num_levels));

#ifdef NU_SHOULD_NOT_BE_INDEXED_BY_SPARSE_LEVEL
    ERE(get_zero_vector(&nu_vp, num_sparse_levels));
    ERE(get_zero_vector(&log_nu_vp, num_sparse_levels));
    ERE(get_zero_vector(&log_prob_vp, num_sparse_levels));
#else
    ERE(get_zero_vector(&nu_vp, num_levels));
    ERE(get_zero_vector(&log_nu_vp, num_levels));
    ERE(get_zero_vector(&log_prob_vp, num_levels));
#endif

    ERE(get_zero_vector(&fit_vp, num_points));

    ERE(get_target_int_vector(&con_score_counts_vp,
                              max_num_con_items));

    ERE(get_zero_vector(&con_score_vp, max_num_con_items));

#ifdef XXX_NOT_SURE_WHAT_IT_IS_DOING
    ERE(get_target_matrix(&con_score_mp, max_num_con_items,
                          num_levels * num_clusters));
#endif


    verify_probability_matrix_vector(P_i_n_mvp, NULL);
    verify_matrix_vector(mean_mvp, NULL);
    verify_matrix_vector(var_mvp, NULL);

#ifdef TEST
    ERE(get_initialized_matrix(&P_c_p_mp, num_points, num_clusters,
                               MIN_LOG_PROB_CLUSTER_GIVEN_POINT));
#else
    ERE(get_target_matrix(&P_c_p_mp, num_points, num_clusters));
#endif

    if ((P_c_p_mpp == NULL) && (input_P_c_p_mp != NULL))
    {
        lock_P_c_p_mp = input_P_c_p_mp;

        warn_pso("Locking cluster membership.\n");
        warn_pso("This feature is not known to be used at this time.\n");
    }
    else
    {
        lock_P_c_p_mp = P_c_p_mp;
    }

    ERE(initialize_level_indicators_cache(num_levels, num_clusters, max_num_items));
    level_indicators_ppp = fs_cached_level_indicators_ppp;

    if (P_l_p_c_mvpp == NULL)
    {
        P_l_p_c_mvp = NULL;
    }
    else
    {
        int wrong_size = FALSE;

        if (*P_l_p_c_mvpp != NULL)
        {
            if ((*P_l_p_c_mvpp)->length != num_levels)
            {
                wrong_size = TRUE;
            }
            else
            {
                for (level = first_level; level < last_level_num; level++)
                {
                     if (    ((*P_l_p_c_mvpp)->elements[ level ] == NULL)
                          || ((*P_l_p_c_mvpp)->elements[ level ]->num_rows != num_points)
                          || ((*P_l_p_c_mvpp)->elements[ level ]->num_cols != num_clusters)
                        )
                     {
                         wrong_size = TRUE;
                         break;
                     }
                }
            }
        }

        if (wrong_size)
        {
            warn_pso("Unexpected size of level matrix in get_cluster_membership_2.\n");
            warn_pso("Recycling the storage and re-initializing it.\n");
        }

        if ((wrong_size) || (*P_l_p_c_mvpp == NULL))
        {
            ERE(get_target_matrix_vector(P_l_p_c_mvpp, num_levels));
            P_l_p_c_mvp = *P_l_p_c_mvpp;

            for (level = first_level; level < last_level_num; level++)
            {
                ERE(get_zero_matrix(&(P_l_p_c_mvp->elements[ level ]),
                                    num_points, num_clusters));

                for (point = 0; point < num_points; point++)
                {
                    for (cluster = 0; cluster < num_clusters; cluster++)
                    {
                        P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ] =
                            V_mp->elements[ level ][ cluster ];
                    }
                }
            }
        }

        P_l_p_c_mvp = *P_l_p_c_mvpp;
    }

    if (P_l_p_mpp == NULL)
    {
        P_l_p_mp = NULL;
    }
    else if (P_l_p_c_mvpp != NULL)
    {
        warn_pso("Request for aspects in get_cluster_membership_2 ignored\n");
        warn_pso("because cluster specific aspects requested.\n");
        P_l_p_mp = NULL;
    }
    else
    {
        int wrong_size = FALSE;

        if (*P_l_p_mpp != NULL)
        {
            if (    ((*P_l_p_mpp)->num_rows != num_sparse_levels)
                 || ((*P_l_p_mpp)->num_cols != num_points)
               )
            {
                wrong_size = TRUE;
            }
        }

        if (wrong_size)
        {
            warn_pso("Unexpected size of level matrix in get_cluster_membership_2.\n");
            warn_pso("Recycling the storage and re-initializing it.\n");
        }

        if ((wrong_size) || (*P_l_p_mpp == NULL))
        {
            ERE(get_zero_matrix(P_l_p_mpp, num_sparse_levels, num_points));

            for (level = first_level; level < last_level_num; level++)
            {
                double cluster_sum = 0.0;

                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    cluster_sum += V_mp->elements[ level ][ cluster ] * a_vp->elements[ cluster ];
                }

                level_weight_vp->elements[ level ] = cluster_sum;
            }

            if (level_index_mp != NULL)
            {
                /*
                 * Comment after the fact: It looks like we are setting up a
                 * default level_index_mp and P_l_p_mpp because we have no
                 * P_l_p_mpp---but this does not really make sense unless it
                 * gets over-written?
                 *
                 * It seems that our purpose is to compute P_l_p_mpp, but it was
                 * not been done correctly in the case of sparse levels? Now
                 * that we have changed this, presumably this initialization is
                 * not needed?
                */

                /*
                 * We count on the fact that if first_level is not zero, that
                 * level_weight_vp is initialized to zero.
                */
                ERE(vp_get_indexed_vector(&sorted_l_p_vp, level_weight_vp));
                ERE(descend_sort_indexed_vector(sorted_l_p_vp));

                for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
                {
                    for (point = 0; point < num_points; point++)
                    {
                        (*P_l_p_mpp)->elements[ sparse_level ][ point ] = sorted_l_p_vp->elements[ sparse_level ].element;
                        level_index_mp->elements[ sparse_level ][ point ] = sorted_l_p_vp->elements[ sparse_level ].index;
                    }
                }
            }
            else
            {
                for (level = first_level; level < last_level_num; level++)
                {
                    for (point = 0; point < num_points; point++)
                    {
                        (*P_l_p_mpp)->elements[ level ][ point ] = level_weight_vp->elements[ level ];
                    }
                }
            }

            ERE(ow_scale_matrix_cols_by_sums(*P_l_p_mpp));
        }

        P_l_p_mp = *P_l_p_mpp;
    }

#ifdef TEST
    if (level_index_mp != NULL)
    {
        verify_non_negative_int_matrix(level_index_mp, NULL);
    }
#endif

    if (fs_watch_2)
    {
        pso("\n");
        pso("Get cluster membership:\n");

        if (P_l_p_c_mvp != NULL)
        {
            pso("    Fitting document and cluster specific vertical weights.\n");
        }
        else if (P_l_p_mp != NULL)
        {
            pso("    Fitting document specific vertical weights.\n");
        }
        else if (V_mp != NULL)
        {
            pso("    Using average vertical weights.\n");
        }
        else if (input_P_l_p_c_mvp != NULL)
        {
            pso("    Using document and cluster specific vertical weights.\n");
        }
        else if (input_P_l_p_mp != NULL)
        {
            pso("    Using document and cluster specific vertical weights.\n");
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        if ((P_c_p_mpp == NULL) && (input_P_c_p_mp != NULL))
        {
            pso("    Using supplied cluster membership.\n");
        }
        else
        {
            pso("    Learning cluster membership.\n");
        }

        pso("\n");

    }

    ERE(initialize_con_prob_cache(max_num_con_items,
                                  first_level,
                                  last_level_num,
                                  num_levels,
                                  level_counts_vp));

    ERE(initialize_vector_feature_counts(con_feature_enable_vp,
                                         con_feature_group_vp));

    for (it = 0; it < fs_max_num_em_iterations; it++)
    {
        int count = 0;
#ifdef XXX_LIMIT_MIN_CON_PROB
        int num_g_prob_resets = 0;
#endif

        for (point = 0; point < num_points; point++)
        {
            if (    (points_to_use_vp != NULL)
                 && (points_to_use_vp->elements[ point ] == 0)
               )
            {
                continue;
            }

            dis_factor = (dis_factor_vp == NULL) ? 1.0 : dis_factor_vp->elements[ point ];
            con_factor = (con_factor_vp == NULL) ? 1.0 : con_factor_vp->elements[ point ];
            num_con_items = (max_num_con_items == 0) ? 0 : con_item_mvp->elements[ point ]->num_rows;

            if (max_num_con_items > 0)
            {
                ll_con_correction_norm_sum = 0.0;
            }

            /*
            // Cache the probability calculations so that they are done only
            // once per node as opposed to num_levels*num_cluster times.
            */
            for (con_item = 0; con_item < num_con_items; con_item++)
            {
                save_x_ptr = con_item_mvp->elements[ point ]->elements[ con_item ];
                con_prob_cache_vvp = fs_con_prob_cache_vvvp->elements[ con_item ];

                for (level = first_level; level < last_level_num; level++)
                {
                    num_nodes_this_level = level_counts_vp->elements[ level ];
                    con_prob_cache_ptr   = con_prob_cache_vvp->elements[ level ]->elements;

                    for (node = 0; node < num_nodes_this_level; node++)
                    {
#ifdef HOW_IT_WAS_05_01_29
                        d        = 0.0;
                        u_ptr    = mean_mvp->elements[ level ]->elements[ node ];
                        v_ptr    = var_mvp->elements[ level ]->elements[ node ];
                        x_ptr    = save_x_ptr;
                        norm     = con_log_sqrt_det_vvp->elements[ level ]->elements[ node ];

                        ASSERT_IS_POSITIVE_INT(num_con_features);

#ifdef HOW_IT_WAS
                        if (*v_ptr > 0.0)
                        {
#endif
                            ASSERT_IS_NUMBER_DBL(norm);
                            ASSERT_IS_FINITE_DBL(norm);

                            for (con_feature = 0; con_feature < num_con_features; con_feature++)
                            {
                                v = *v_ptr;

                                if (v > 0.0)
                                {
                                    dev = *u_ptr - *x_ptr;
                                    temp = (dev / v) * dev;

                                    ASSERT_IS_NUMBER_DBL(dev);
                                    ASSERT_IS_FINITE_DBL(dev);

                                    ASSERT_IS_NUMBER_DBL(v);
                                    ASSERT_IS_FINITE_DBL(v);

                                    ASSERT_IS_NUMBER_DBL(temp);
                                    ASSERT_IS_FINITE_DBL(temp);

                                    if (max_con_norm_var > 0.0)
                                    {
                                        temp = MIN_OF(temp, max_con_norm_var);
                                    }

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

                            log_prob = -0.5 * d;

                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

                            if (norm_continuous_features)
                            {
                                /*
                                // FIX
                                //
                                // Not valid if features are limited, or if we
                                // are selectively disabling features.
                                */
                                kjb_abort();

                                log_prob /= num_con_features;
                            }

                            log_prob -= norm;

                            ASSERT_IS_NUMBER_DBL(log_prob);
                            ASSERT_IS_FINITE_DBL(log_prob);

#ifdef XXX_LIMIT_MIN_CON_PROB
                            if (log_prob + con_log_norm < min_con_log_prob)
                            {
                                num_g_prob_resets++;
                                log_prob = min_con_log_prob - con_log_norm;
                            }
#endif

                            *con_prob_cache_ptr = log_prob;
#ifdef HOW_IT_WAS
                        }
                        else
                        {
                            /*
                            // This is not handled well below. We are moving
                            // away from node culling, so this should really not
                            // happen. Eventually, it should either never happen,
                            // or be handled better.
                            */
                            *con_prob_cache_ptr = LOG_ZERO;
                        }
#endif

#else
                        ASSERT_IS_LESS_INT(level, last_level_num);
                        ASSERT_IS_NOT_LESS_INT(level, first_level);
                        ASSERT_IS_NON_NEGATIVE_INT(first_level);
                        ASSERT_IS_NON_NEGATIVE_INT(node);
                        ASSERT_IS_LESS_INT(node, mean_mvp->elements[ level ]->num_rows);
                        ASSERT_IS_LESS_INT(node, var_mvp->elements[ level ]->num_rows);
                        ASSERT_IS_LESS_INT(node, con_log_sqrt_det_vvp->elements[ level ]->length);

                        /*
                         * This is a stop gap until we make this routine call
                         * do_multi_modal_clustering_4().
                        */
                        ERE(compute_con_prob(con_feature_enable_vp,
                                             con_feature_group_vp,
                                             save_x_ptr,
                                             mean_mvp->elements[ level ]->elements[ node ],
                                             var_mvp->elements[ level ]->elements[ node ],
                                             con_log_sqrt_det_vvp->elements[ level ]->elements[ node ],
                                             norm_continuous_features,
                                             fs_limit_gaussian_deviation,
                                             fs_max_con_norm_var,
                                             fs_vector_feature_counts_ptr,
                                             con_prob_cache_ptr,
                                             NULL, FALSE));

#endif
                        con_prob_cache_ptr++;
                    }
                }
            }

#ifdef TEST
/* TEST */  for (cluster = 0; cluster < num_clusters; cluster++)
/* TEST */  {
/* TEST */      for (item = 0; item < max_num_items; item++)
/* TEST */      {
/* TEST */          for (level = 0; level < num_levels; level++)
/* TEST */          {
/* TEST */              level_indicators_pp = level_indicators_ppp[ level ];
/* TEST */              level_indicators_p = level_indicators_pp[ cluster ];
/* TEST */
/* TEST */              level_indicators_p += item;
/* TEST */
/* TEST */              ASSERT(level_indicators_p == &(level_indicators_ppp[ level ][ cluster ][ item ]));
/* TEST */
/* TEST */              *level_indicators_p = DBL_NOT_SET;
/* TEST */          }
/* TEST */      }
/* TEST */  }
#endif

            /*
             * I am not sure what this was for, or if it is ever used anymore.
             * It looks like pre-processing for normalizing to prevent over-flow
            */
            if (con_score_cutoff > DBL_HALF_MOST_NEGATIVE / 2.0)
            {
                dbp("--------------------------------------");
                dbp("I am not sure when this is used!");
                dbp("--------------------------------------");
                dbe(con_score_cutoff);
                dbp("--------------------------------------");

                kjb_abort();

#ifdef XXX_NOT_SURE_WHAT_IT_IS_DOING
XXX                  ERE(ow_zero_int_vector(con_score_counts_vp));
XXX
XXX                  for (cluster = 0; cluster < num_clusters; cluster++)
XXX                  {
XXX                      double log_a = DBL_MOST_NEGATIVE;
XXX
XXX                      p = a_vp->elements[ cluster ];
XXX
XXX                      if (p < 5.0 * DBL_MIN)
XXX                      {
XXX                          TEST_PSO(("Cluster (%d) with no meat spotted in E step.\n",
XXX                                    cluster + 1));
XXX                          dbe(p);
XXX                          log_a = LOG_ZERO;
XXX                      }
XXX                      else
XXX                      {
XXX                          log_a = log(p);
XXX                      }
XXX  #ifdef TEST
XXX                      sum_nu = 0.0;
XXX  #endif
XXX                      /*
XXX                      // Cache the level weights, taking logs if we are using logs for
XXX                      // probs.
XXX                      */
XXX                      for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
XXX                      {
XXX                          if (level_index_mp != NULL)
XXX                          {
XXX                              level = level_index_mp->elements[ sparse_level ][ point ];
XXX                          }
XXX                          else
XXX                          {
XXX                              level = sparse_level;
XXX                          }
XXX
XXX                          if (P_l_p_c_mvp != NULL)
XXX                          {
XXX                              nu = P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ];
XXX                          }
XXX                          else if (P_l_p_mp != NULL)
XXX                          {
XXX                              nu = P_l_p_mp->elements[ level ][ point ];
XXX                          }
XXX                          else if (V_mp != NULL)
XXX                          {
XXX                              nu = V_mp->elements[ level ][ cluster ];
XXX                          }
XXX                          else if (input_P_l_p_c_mvp != NULL)
XXX                          {
XXX                              nu = input_P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ];
XXX                          }
XXX                          else if (input_P_l_p_mp != NULL)
XXX                          {
XXX                              nu = input_P_l_p_mp->elements[ level ][ point ];
XXX                          }
XXX                          else
XXX                          {
XXX                              SET_CANT_HAPPEN_BUG();
XXX                              return ERROR;
XXX                          }
XXX
XXX  #ifdef TEST
XXX                          sum_nu += nu;
XXX  #endif
XXX
XXX                          nu_vp->elements[ sparse_level ] = nu;
XXX                          log_nu_vp->elements[ sparse_level ] = SAFE_LOG(nu);
XXX                      }
XXX
XXX  #ifdef TEST
XXX                      if (level_index_mp == NULL)
XXX                      {
XXX  /* TEST */              ASSERT_IS_LESS_DBL(sum_nu, 1.01);
XXX  /* TEST */              ASSERT_IS_NOT_LESS_DBL(sum_nu, 0.99);
XXX                      }
XXX                      else
XXX                      {
XXX                          if (sum_nu < 0.25)
XXX                          {
XXX                              warn_pso(" | Sparse level weights have captured less then 1/4 of full weight.\n");
XXX                              warn_pso(" |     Sum nu is %.3e.\n", sum_nu);
XXX                              warn_pso(" |     Point %d with respect to cluster %d.\n", point, cluster);
XXX                          }
XXX                      }
XXX  #endif
XXX
XXX                      for (con_item = 0; con_item < num_con_items; con_item++)
XXX                      {
XXX                          con_prob_cache_vvp = fs_con_prob_cache_vvvp->elements[ con_item ];
XXX                          prob_sum = 0.0;
XXX                          max_log_prob = DBL_MOST_NEGATIVE;
XXX
XXX                          for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
XXX                          {
XXX                              if (level_index_mp != NULL)
XXX                              {
XXX                                  level = level_index_mp->elements[ sparse_level ][ point ];
XXX                              }
XXX                              else
XXX                              {
XXX                                  level = sparse_level;
XXX                              }
XXX
XXX                              log_nu       = log_nu_vp->elements[ sparse_level ];
XXX                              node         = node_mp->elements[ cluster ][ level ];
XXX                              log_prob = con_prob_cache_vvp->elements[ level ]->elements[ node ];
XXX
XXX                              log_prob += log_nu;
XXX
XXX                              i = con_score_counts_vp->elements[ con_item ];
XXX                              con_score_mp->elements[ con_item ][ i ] = log_prob;
XXX
XXX  #ifdef XXX_SUPPORT_TRIMMING
XXX                              if (fs_trim_hack == 1)
XXX                              {
XXX                                  (con_score_mp->elements[ con_item ][ i ]) += log_a;
XXX                              }
XXX                              else if (fs_trim_hack == 2)
XXX                              {
XXX                                  (con_score_mp->elements[ con_item ][ i ]) += log(P_c_p_mp->elements[ point ][ cluster ]);
XXX                              }
XXX  #endif
XXX
XXX
XXX                              (con_score_counts_vp->elements[ con_item ])++;
XXX                          }
XXX                      }
XXX                  }
XXX
XXX                  sum_collected_scores(num_con_items, 0,
XXX                                       con_score_counts_vp,
XXX                                       con_score_mp,
XXX                                       con_score_vp);
XXX                  if (con_score_vpp != NULL)
XXX                  {
XXX                      ERE(copy_vector(&(con_score_vpp->elements[ point ]),
XXX                                      con_score_vp));
XXX                  }
#endif
            }

            /* XXXXXXXXXXX   Above code not likely to be relavent.   XXXX  */

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                int num_good_con_items = 0;

                dis_item_found  = FALSE;
                product         = a_vp->elements[ cluster ];
                log_dis_prod    = 0.0;
                log_con_prod    = 0.0;
#ifdef TEST
                sum_nu         = 0.0;
#endif

                log_product = SAFE_LOG(product);

#ifdef NU_SHOULD_NOT_BE_INDEXED_BY_SPARSE_LEVEL
XXX                  /*
XXX                  // Cache the level weights, taking logs if we are using logs for
XXX                  // probs.
XXX                  */
XXX                  for (sparse_level = first_level; sparse_level < num_sparse_levels; sparse_level++)
XXX                  {
XXX                      if (level_index_mp != NULL)
XXX                      {
XXX                          level = level_index_mp->elements[ sparse_level ][ point ];
XXX                      }
XXX                      else
XXX                      {
XXX                          level = sparse_level;
XXX                      }
XXX
XXX                      /*
XXX                       * The "new" inference assumes that the prior is V_mp which
XXX                       * is what it should be---the prior is not the value for the
XXX                       * particular document!
XXX                      */
XXX
XXX  #ifdef OLD_INFERENCE
XXX  XXX                      if (P_l_p_c_mvp != NULL)
XXX  XXX                      {
XXX  XXX                          nu = P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ];
XXX  XXX                      }
XXX  XXX                      else if (P_l_p_mp != NULL)
XXX  XXX                      {
XXX  XXX                          nu = P_l_p_mp->elements[ level ][ point ];
XXX  XXX                      }
XXX  XXX                      else if (V_mp != NULL)
XXX  XXX                      {
XXX  XXX                          nu = V_mp->elements[ level ][ cluster ];
XXX  XXX                      }
XXX  XXX                      else if (input_P_l_p_c_mvp != NULL)
XXX  XXX                      {
XXX  XXX                          nu = input_P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ];
XXX  XXX                      }
XXX  XXX                      else if (input_P_l_p_mp != NULL)
XXX  XXX                      {
XXX  XXX                          nu = input_P_l_p_mp->elements[ level ][ point ];
XXX  XXX                      }
XXX  XXX                      else
XXX  XXX                      {
XXX  XXX                          SET_CANT_HAPPEN_BUG();
XXX  XXX                          return ERROR;
XXX  XXX                      }
XXX  #else
XXX                      if (V_mp != NULL)
XXX                      {
XXX                          nu = V_mp->elements[ level ][ cluster ];
XXX                      }
XXX                      else if (input_P_l_p_c_mvp != NULL)
XXX                      {
XXX                          nu = input_P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ];
XXX                      }
XXX                      else if (input_P_l_p_mp != NULL)
XXX                      {
XXX                          UNTESTED_CODE();
XXX                          dbx(input_P_l_p_mp);
XXX                          nu = input_P_l_p_mp->elements[level ][ point ];
XXX                      }
XXX                      else
XXX                      {
XXX                          SET_CANT_HAPPEN_BUG();
XXX                          return ERROR;
XXX                      }
XXX  #endif
XXX
XXX  #ifdef TEST
XXX                      sum_nu += nu;
XXX  #endif
XXX
XXX                      nu_vp->elements[ sparse_level ] = nu;
XXX
XXX                      log_nu_vp->elements[ sparse_level ] = SAFE_LOG(nu);
XXX                  }
XXX
XXX  #ifdef TEST
XXX                  if (level_index_mp == NULL)
XXX                  {
XXX                      ASSERT_IS_LESS_DBL(sum_nu, 1.01);
XXX                      ASSERT_IS_NOT_LESS_DBL(sum_nu, 0.99);
XXX                  }
XXX  #endif
XXX
#else

                /*
                // Cache the level weights, taking logs if we are using logs for
                // probs.
                */
                for (level = first_level; level < last_level_num; level++)
                {
                    if (V_mp != NULL)
                    {
                        nu = V_mp->elements[ level ][ cluster ];
                    }
                    else if (input_P_l_p_c_mvp != NULL)
                    {
                        nu = input_P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ];
                    }
                    else if (input_P_l_p_mp != NULL)
                    {
                        UNTESTED_CODE();
                        dbx(input_P_l_p_mp);
                        nu = input_P_l_p_mp->elements[level ][ point ];
                    }
                    else
                    {
                        SET_CANT_HAPPEN_BUG();
                        return ERROR;
                    }
#ifdef TEST
                    sum_nu += nu;
#endif

                    nu_vp->elements[ level ] = nu;

                    log_nu_vp->elements[ level ] = SAFE_LOG(nu);
                }

#ifdef TEST
                ASSERT_IS_LESS_DBL(sum_nu, 1.01);
                ASSERT_IS_NOT_LESS_DBL(sum_nu, 0.99);
#endif

#endif

                if ((model_correspondence == 1) && (num_con_items > 0))
                {
                    ERE(get_zero_vector(&nu_for_dependent_model_vp,
                                        num_levels));
                }

                for (con_item = 0; con_item < num_con_items; con_item++)
                {
                    exp_sum       = 0.0;
                    max_log_prob  = DBL_MOST_NEGATIVE;
                    con_prob_cache_vvp = fs_con_prob_cache_vvvp->elements[ con_item ];

#ifdef TEST
                    ERE(ow_set_vector(log_prob_vp, DBL_NOT_SET));
#endif



                    if (    (con_score_cutoff > DBL_HALF_MOST_NEGATIVE / 2.0)
                         && (con_score_vp->elements[ con_item ] < con_score_cutoff)
                       )
                    {
                        dbs("Executing rare code?");

                        if (level_index_mp != NULL)
                        {
                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_pp = level_indicators_ppp[ level ];
                                level_indicators_p = level_indicators_pp[ cluster ];
                                level_indicators_p += (max_num_dis_items + con_item);
                                *level_indicators_p = 0.0;
                            }
                        }

#ifdef NU_SHOULD_NOT_BE_INDEXED_BY_SPARSE_LEVEL
XXX                          for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
XXX                          {
XXX                              if (level_index_mp != NULL)
XXX                              {
XXX                                  level = level_index_mp->elements[ sparse_level ][ point ];
XXX                              }
XXX                              else
XXX                              {
XXX                                  level = sparse_level;
XXX                              }
XXX
XXX                              level_indicators_pp = level_indicators_ppp[ level ];
XXX                              level_indicators_p = level_indicators_pp[ cluster ];
XXX
XXX                              /* CHECK */
XXX                              /* Why no deference to con_item? */
XXX                              /* Added the following line on June 21, 2004 */
XXX
XXX                              level_indicators_p += (max_num_dis_items + con_item);
XXX
XXX                              *level_indicators_p = nu_vp->elements[ sparse_level ];
XXX                          }
#else
                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];
                            level_indicators_p = level_indicators_pp[ cluster ];

                            /* CHECK */
                            /* Why no deference to con_item? */
                            /* Added the following line on June 21, 2004 */

                            level_indicators_p += (max_num_dis_items + con_item);

                            *level_indicators_p = nu_vp->elements[ level ];
                        }
#endif

                        continue;
                    }

                    num_good_con_items++;

#ifdef NU_SHOULD_NOT_BE_INDEXED_BY_SPARSE_LEVEL
XXX                      for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
XXX                      {
XXX                          if (level_index_mp != NULL)
XXX                          {
XXX                              level = level_index_mp->elements[ sparse_level ][ point ];
XXX                          }
XXX                          else
XXX                          {
XXX                              level = sparse_level;
XXX                          }
XXX
XXX                          log_nu = log_nu_vp->elements[ sparse_level ];
XXX                          node     = node_mp->elements[ cluster ][ level ];
XXX                          log_prob = con_prob_cache_vvp->elements[ level ]->elements[ node ];
XXX
XXX                          ASSERT_IS_NUMBER_DBL(log_prob);
XXX                          ASSERT_IS_FINITE_DBL(log_prob);
XXX
XXX                          log_prob += log_nu;
XXX
XXX                          ASSERT_IS_NUMBER_DBL(log_prob);
XXX                          ASSERT_IS_FINITE_DBL(log_prob);
XXX
XXX                          max_log_prob = MAX_OF(max_log_prob, log_prob);
XXX                          log_prob_vp->elements[ sparse_level ] = log_prob;
XXX                      }
XXX
XXX                      for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
XXX                      {
XXX                          temp = log_prob_vp->elements[ sparse_level ] - max_log_prob;
XXX                          exp_temp = exp(temp);
XXX
XXX                          exp_sum += exp_temp;
XXX
XXX                          log_prob_vp->elements[ sparse_level ] = exp_temp;
XXX                      }
#else
                    for (level = first_level; level < last_level_num; level++)
                    {
                        log_nu = log_nu_vp->elements[ level ];
                        node     = node_mp->elements[ cluster ][ level ];
                        log_prob = con_prob_cache_vvp->elements[ level ]->elements[ node ];

                        ASSERT_IS_NUMBER_DBL(log_prob);
                        ASSERT_IS_FINITE_DBL(log_prob);

                        log_prob += log_nu;

                        ASSERT_IS_NUMBER_DBL(log_prob);
                        ASSERT_IS_FINITE_DBL(log_prob);

                        max_log_prob = MAX_OF(max_log_prob, log_prob);
                        log_prob_vp->elements[ level ] = log_prob;
                    }

                    for (level = first_level; level < last_level_num; level++)
                    {
                        temp = log_prob_vp->elements[ level ] - max_log_prob;
                        exp_temp = exp(temp);

                        exp_sum += exp_temp;

                        log_prob_vp->elements[ level ] = exp_temp;
                    }
#endif

                    log_prob_sum = SAFE_LOG(exp_sum) + max_log_prob;

                    ASSERT_IS_NUMBER_DBL(log_prob_sum);
                    ASSERT_IS_FINITE_DBL(log_prob_sum);

#ifdef NU_SHOULD_NOT_BE_INDEXED_BY_SPARSE_LEVEL
                    if (level_index_mp != NULL)
                    {
                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];
                            level_indicators_p = level_indicators_pp[ cluster ];
                            level_indicators_p += (max_num_dis_items + con_item);
                            *level_indicators_p = 0.0;
                        }
                    }

                    for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
                    {
                        if (level_index_mp != NULL)
                        {
                            level = level_index_mp->elements[ sparse_level ][ point ];
                        }
                        else
                        {
                            level = sparse_level;
                        }
                        /*
                        // Force indirection in case compiler can't figure out
                        // how to do this efficiently.
                        */
                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ cluster ];
                        level_indicators_p += (max_num_dis_items + con_item);
                        *level_indicators_p = log_prob_vp->elements[ sparse_level ] / exp_sum;
                    }
#else
                    for (level = first_level; level < last_level_num; level++)
                    {
                        /*
                        // Force indirection in case compiler can't figure out
                        // how to do this efficiently.
                        */
                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ cluster ];
                        level_indicators_p += (max_num_dis_items + con_item);
                        *level_indicators_p = log_prob_vp->elements[ level ] / exp_sum;
                    }
#endif

                    if (model_correspondence == 1)
                    {
                        double con_item_weight = 1.0;

                        if ((weight_con_items) && (con_item_weight_vvp != NULL))
                        {
                            static int first_time = TRUE;

                            if (first_time)
                            {
                                first_time = FALSE;
                                dbp("Weighting continous items.\n");
                            }

                            con_item_weight = con_item_weight_vvp->elements[point]->elements[con_item];
                        }

                        if (norm_depend)
                        {
                            level_sum = 0.0;

                            for (level = first_level; level < last_level_num; level++)
                            {
                                level_indicators_pp = level_indicators_ppp[ level ];
                                level_indicators_p = level_indicators_pp[ cluster ] + max_num_dis_items + con_item;
                                level_sum += (*level_indicators_p);
                            }

                            con_item_weight /= level_sum;
                        }

                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];
                            level_indicators_p = level_indicators_pp[ cluster ] + max_num_dis_items + con_item;
                            nu_for_dependent_model_vp->elements[ level ] += con_item_weight * (*level_indicators_p);
                        }
                    }

                    log_con_prod += log_prob_sum;

                    ASSERT_IS_NUMBER_DBL(log_con_prod);
                    ASSERT_IS_FINITE_DBL(log_con_prod);
                }

#ifdef TEST
                if (con_score_cutoff < DBL_HALF_MOST_NEGATIVE / 2.0)
                {
                    ASSERT_IS_EQUAL_INT(num_con_items, num_good_con_items);
                }
#endif
                if (num_good_con_items > 0)
                {
                    ASSERT_IS_NUMBER_DBL(con_factor);
                    ASSERT_IS_FINITE_DBL(con_factor);
                    ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

                    log_product += (log_con_prod * con_factor);
                }

                ASSERT_IS_NUMBER_DBL(log_product);
                ASSERT_IS_FINITE_DBL(log_product);

                ASSERT_IS_NOT_LESS_DBL(log_product,
                                      MIN_LOG_PROB_CLUSTER_GIVEN_POINT / 2.0);

#ifdef NORMALIZE_NU_FOR_DEPENDENT_MODEL
                /*
                // This has little effect -- mostly in the computation of
                // the likelihood.
                */
                if (model_correspondence == 1)
                {
                    if (nu_for_dependent_model_vp != NULL)
                    {
                        ERE(ow_scale_vector_by_sum(nu_for_dependent_model_vp));
                    }
#ifdef TEST
                    else
                    {
                        ASSERT(nu_with_prior_mp != NULL);
                    }
#endif
                }
#endif

                for (dis_item = 0; dis_item < max_num_dis_items; dis_item++)
                {
                    category = dis_item_mp->elements[ point ][ dis_item ];
#ifdef TEST
                    sum_nu = 0.0;
#endif
                    prob_sum = 0.0;

                    if (category < 0) break;

                    ASSERT_IS_LESS_INT(category , num_categories);

                    dis_item_found = TRUE;
                    dis_item_multiplicity = dis_item_multiplicity_mp->elements[ point ][ dis_item ];

                    /* Safest to force non-sparse elements to zero. */
                    if (level_index_mp != NULL)
                    {
                        for (level = first_level; level < last_level_num; level++)
                        {
                            level_indicators_pp = level_indicators_ppp[ level ];
                            level_indicators_p = level_indicators_pp[ cluster ];
                            level_indicators_p += dis_item;
                            *level_indicators_p = 0.0;
                        }
                    }

#ifdef NU_SHOULD_NOT_BE_INDEXED_BY_SPARSE_LEVEL
XXX                      for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
#else
                    for (level = first_level; level < last_level_num; level++)
#endif
                    {
#ifdef NU_SHOULD_NOT_BE_INDEXED_BY_SPARSE_LEVEL
XXX                          if (level_index_mp != NULL)
XXX                          {
XXX                              level = level_index_mp->elements[ sparse_level ][ point ];
XXX                          }
XXX                          else
XXX                          {
XXX                              level = sparse_level;
XXX                          }
#endif

                        verify_probability_matrix(P_i_n_mvp->elements[ level ], NULL);

                        node = node_mp->elements[ cluster ][ level ];

                        ASSERT_IS_NON_NEGATIVE_INT(node);
                        ASSERT_IS_LESS_INT(node, P_i_n_mvp->elements[ level ]->num_rows);
                        ASSERT_IS_NON_NEGATIVE_INT(category);
                        ASSERT_IS_LESS_INT(category, P_i_n_mvp->elements[ level ]->num_cols);

                        prob = P_i_n_mvp->elements[ level ]->elements[ node ][ category ];

                        if ((num_con_items > 0) && (model_correspondence == 1))
                        {
                            nu = nu_for_dependent_model_vp->elements[ level ];
                        }
                        else
                        {
#ifdef NU_SHOULD_NOT_BE_INDEXED_BY_SPARSE_LEVEL
                            nu = nu_vp->elements[ sparse_level ];
#else
                            nu = nu_vp->elements[ level ];
#endif

#ifdef TEST
/* TEST */                  sum_nu += nu;
#endif
                        }

                        level_indicators_pp = level_indicators_ppp[ level ];

                        ASSERT_IS_NUMBER_DBL(prob);
                        ASSERT_IS_FINITE_DBL(prob);
                        ASSERT_IS_NON_NEGATIVE_DBL(prob);

                        ASSERT_IS_NUMBER_DBL(nu);
                        ASSERT_IS_FINITE_DBL(nu);
                        ASSERT_IS_NON_NEGATIVE_DBL(nu);

                        prob *= nu;

                        ASSERT_IS_NUMBER_DBL(prob);
                        ASSERT_IS_FINITE_DBL(prob);
                        ASSERT_IS_NON_NEGATIVE_DBL(prob);

                        level_indicators_pp[ cluster ][ dis_item ] = prob;
                        prob_sum += prob;

                        ASSERT_IS_NUMBER_DBL(prob_sum);
                        ASSERT_IS_FINITE_DBL(prob_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);
                    }

#ifdef TEST
                    if ( ! ((num_con_items > 0) && (model_correspondence == 1)))
                    {
                        if (level_index_mp == NULL)
                        {
                            ASSERT_IS_LESS_DBL(sum_nu, 1.01);
                            ASSERT_IS_NOT_LESS_DBL(sum_nu, 0.99);
                        }
                        else
                        {
                            if (sum_nu < 0.25)
                            {
                                warn_pso(" : Sparse level weights have captured less then 1/4 of full weight.\n");
                                warn_pso(" :     Sum nu is %.3e.\n", sum_nu);
                                warn_pso(" :     Point %d with respect to cluster %d.\n", point, cluster);
                            }
                        }
                    }
#endif

                    ASSERT_IS_NUMBER_DBL(prob_sum);
                    ASSERT_IS_FINITE_DBL(prob_sum);
                    ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);

                    log_prob_sum = SAFE_LOG(prob_sum);

                    ASSERT_IS_NUMBER_DBL(log_prob_sum);
                    ASSERT_IS_FINITE_DBL(log_prob_sum);

#ifdef DONT_USE_SPARSE_HERE
                    for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
#else
                    for (level = first_level; level < last_level_num; level++)
#endif
                    {
#ifdef DONT_USE_SPARSE_HERE
XXX                          if (level_index_mp != NULL)
XXX                          {
XXX                              level = level_index_mp->elements[ sparse_level ][ point ];
XXX                          }
XXX                          else
XXX                          {
XXX                              level = sparse_level;
XXX                          }
#endif

                        /*
                        // Force indirection in case compiler can't figure out
                        // how to do this efficiently.
                        */
                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ cluster ];

                        level_indicators_p += dis_item;

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);

                        *level_indicators_p /= prob_sum;

                        ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                        ASSERT_IS_FINITE_DBL(*level_indicators_p);
                        ASSERT_IS_PROB_DBL(*level_indicators_p);
                    }

                    log_dis_prod += (log_prob_sum * dis_item_multiplicity);
                }

                if (dis_item_found)
                {
                    ASSERT_IS_NUMBER_DBL(dis_factor);
                    ASSERT_IS_FINITE_DBL(dis_factor);
                    ASSERT_IS_NON_NEGATIVE_DBL(dis_factor);
                    ASSERT_IS_GREATER_DBL(dis_factor, 1e-10);

                    log_product += (log_dis_prod * dis_factor);
                }

                ASSERT_IS_NUMBER_DBL(log_product);
                ASSERT_IS_FINITE_DBL(log_product);

                ASSERT_IS_NOT_LESS_DBL(log_product,
                                       MIN_LOG_PROB_CLUSTER_GIVEN_POINT / 2.0);

                /*
                // If there were no good con items, then this should be
                // log(a).
                */
                P_c_p_mp->elements[ point ][ cluster ] = log_product;
            }

            if (num_con_items > 0)
            {
                ASSERT_IS_NUMBER_DBL(con_factor);
                ASSERT_IS_FINITE_DBL(con_factor);
                ASSERT_IS_NON_NEGATIVE_DBL(con_factor);

                ll_con_correction_norm_sum += (double)num_con_items * con_factor;
            }

#ifdef TEST
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                for (item = 0; item < max_num_items; item++)
                {
                    for (level = 0; level < num_levels; level++)
                    {
                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ cluster ];

                        level_indicators_p += item;

                        if (    (item < max_num_dis_items)
                             && (dis_item_mp->elements[ point ][ item ] < 0)
                           )
                        {
                            ASSERT_IS_EQUAL_DBL(*level_indicators_p,
                                                DBL_NOT_SET);
                        }
                        else if (item >= max_num_dis_items + num_con_items)
                        {
                            ASSERT_IS_EQUAL_DBL(*level_indicators_p,
                                                DBL_NOT_SET);
                        }
                        else if (level < first_level)
                        {
                            ASSERT_IS_EQUAL_DBL(*level_indicators_p,
                                                DBL_NOT_SET);
                        }
                        else if (level >= last_level_num)
                        {
                            ASSERT_IS_EQUAL_DBL(*level_indicators_p,
                                                DBL_NOT_SET);
                        }
                        else
                        {
                            ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                            ASSERT_IS_FINITE_DBL(*level_indicators_p);
                            ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);
                            ASSERT_IS_PROB_DBL(*level_indicators_p);
                        }
                    }
                }
            }
#endif

            max_log_prob = DBL_MOST_NEGATIVE;

            P_c_p_ptr = P_c_p_mp->elements[ point ];

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
#ifdef TEST
                ASSERT_IS_NOT_LESS_DBL(*P_c_p_ptr,
                                      MIN_LOG_PROB_CLUSTER_GIVEN_POINT);

                if (*P_c_p_ptr > max_log_prob)
                {
                    max_index = cluster;
                }
#endif
                max_log_prob = MAX_OF(max_log_prob, *P_c_p_ptr);

                P_c_p_ptr++;
            }

            P_c_p_ptr = P_c_p_mp->elements[ point ];

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                 *P_c_p_ptr -= max_log_prob;
                 P_c_p_ptr++;
            }

#ifdef TEST
            ASSERT(KJB_IS_SET(max_index));
            ASSERT_IS_EQUAL_DBL(P_c_p_mp->elements[ point ][ max_index ], 0.0);
#endif

            P_c_p_ptr = P_c_p_mp->elements[ point ];

            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                 *P_c_p_ptr = exp(*P_c_p_ptr);
                 P_c_p_ptr++;
            }

#ifdef TEST
            ASSERT_IS_EQUAL_DBL(P_c_p_mp->elements[ point ][ max_index ], 1.0);
#endif

            prob_sum = sum_matrix_row_elements(P_c_p_mp, point);

            fit_temp = SAFE_LOG(prob_sum);

            fit_temp += max_log_prob;
            fit_temp += (con_log_norm * ll_con_correction_norm_sum);

            fit_vp->elements[ point ] = fit_temp;

            ASSERT_IS_NOT_LESS_DBL(prob_sum, 1.0);

            ERE(ow_divide_matrix_row_by_scalar(P_c_p_mp, prob_sum, point));

#ifdef TEST
            ASSERT_IS_EQUAL_INT(P_c_p_mp->num_cols, num_clusters);

            if (num_clusters == 1)
            {
                ASSERT_IS_EQUAL_DBL(P_c_p_mp->elements[ point ][ 0 ], 1.0);
            }
#endif

            /*
             * Weight each level weight by cluster membership.
            */
            for (cluster = 0; cluster < num_clusters; cluster++)
            {
                /* Generally lock_P_c_p_mp is just P_c_p_mp. */
                p = lock_P_c_p_mp->elements[ point ][ cluster ];

                ASSERT_IS_NUMBER_DBL(p);
                ASSERT_IS_FINITE_DBL(p);
                ASSERT_IS_NON_NEGATIVE_DBL(p);
                ASSERT_IS_PROB_DBL(p);

#ifdef DONT_USE_SPARSE_HERE
                for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
#else
                for (level = first_level; level < last_level_num; level++)
#endif
                {
#ifdef DONT_USE_SPARSE_HERE
                    if (level_index_mp != NULL)
                    {
                        level = level_index_mp->elements[ sparse_level ][ point ];
                    }
                    else
                    {
                        level = sparse_level;
                    }
#endif

                    level_indicators_pp = level_indicators_ppp[ level ];
                    level_indicators_p = level_indicators_pp[ cluster ];

                    for (dis_item = 0; dis_item < max_num_dis_items; dis_item++)
                    {
                        category = dis_item_mp->elements[ point ][ dis_item ];

                        if (category < 0) break;

                        ASSERT_IS_LESS_INT(category, num_categories);

                        *level_indicators_p *= p;
                        level_indicators_p++;
                    }

                    level_indicators_p = level_indicators_pp[ cluster ];
                    level_indicators_p += max_num_dis_items;

                    for (con_item = 0; con_item < num_con_items; con_item++)
                    {
                        *level_indicators_p *= p;
                        level_indicators_p++;
                    }
                }
            }

            if (P_l_p_c_mvp != NULL)
            {
                for (cluster = 0; cluster < num_clusters; cluster++)
                {
                    level_sum = 0.0;

                    for (level = first_level; level < last_level_num; level++)
                    {
                        prob_sum = 0.0;

                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ cluster ];

                        for (dis_item = 0; dis_item < max_num_dis_items; dis_item++)
                        {
                            category = dis_item_mp->elements[ point ][ dis_item ];

                            /*
                            // Can't use break on negative unless use of item
                            // is adjusted.
                            */
                            if (category < 0) break;

                            ASSERT_IS_LESS_INT(category, num_categories);

                            ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                            ASSERT_IS_FINITE_DBL(*level_indicators_p);
                            ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);

                            prob_sum += *level_indicators_p;

                            level_indicators_p++;
                        }

                        level_indicators_p = level_indicators_pp[ cluster ] + max_num_dis_items;

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                            ASSERT_IS_FINITE_DBL(*level_indicators_p);
                            ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);

                            prob_sum += *level_indicators_p;

                            level_indicators_p++;
                        }

                        ASSERT_IS_NUMBER_DBL(prob_sum);
                        ASSERT_IS_FINITE_DBL(prob_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);

                        P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ] = prob_sum;
                        level_sum += prob_sum;
                    }

                    if (level_sum > 10.0 * DBL_MIN)
                    {
                        for (level = first_level; level < last_level_num; level++)
                        {
                            P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ] /= level_sum;
                        }
                    }
                    else
                    {
                        for (level = first_level; level < last_level_num; level++)
                        {
                            P_l_p_c_mvp->elements[ level ]->elements[ point ][ cluster ] = 1.0 / (last_level_num - first_level);
                        }
                    }
                }
            }
            else if (P_l_p_mp != NULL)
            {
                /*
                 * Compute the level weights by marginalizing over items and
                 * clusters. The cluster weight is already in the level weights
                */

#ifdef DONT_USE_SPARSE_HERE
                level_sum = 0.0;

                for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
#else
                for (level = first_level; level < last_level_num; level++)
#endif
                {
                    double cluster_sum = 0.0;

#ifdef DONT_USE_SPARSE_HERE
                    if (level_index_mp != NULL)
                    {
                        level = level_index_mp->elements[ sparse_level ][ point ];
                    }
                    else
                    {
                        level = sparse_level;
                    }
#endif

                    for (cluster = 0; cluster < num_clusters; cluster++)
                    {
                        prob_sum = 0.0;

                        level_indicators_pp = level_indicators_ppp[ level ];
                        level_indicators_p = level_indicators_pp[ cluster ];

                        for (dis_item = 0; dis_item < max_num_dis_items; dis_item++)
                        {
                            category = dis_item_mp->elements[ point ][ dis_item ];

                            /*
                            // Can't use break on negative unless use of item
                            // is adjusted.
                            */
                            if (category < 0) break;

                            ASSERT_IS_LESS_INT(category, num_categories);

                            ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                            ASSERT_IS_FINITE_DBL(*level_indicators_p);
                            ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);

                            prob_sum += *level_indicators_p;

                            level_indicators_p++;
                        }

                        level_indicators_p = level_indicators_pp[ cluster ] + max_num_dis_items;

                        for (con_item = 0; con_item < num_con_items; con_item++)
                        {
                            ASSERT_IS_NUMBER_DBL(*level_indicators_p);
                            ASSERT_IS_FINITE_DBL(*level_indicators_p);
                            ASSERT_IS_NON_NEGATIVE_DBL(*level_indicators_p);

                            /*
                             * Likely only correct for depend=-1.
                            */
                            prob_sum += (*level_indicators_p);

                            level_indicators_p++;
                        }

                        ASSERT_IS_NUMBER_DBL(prob_sum);
                        ASSERT_IS_FINITE_DBL(prob_sum);
                        ASSERT_IS_NON_NEGATIVE_DBL(prob_sum);


                        cluster_sum += prob_sum;
                    }

#ifdef DONT_USE_SPARSE_HERE
                    P_l_p_mp->elements[ sparse_level ][ point ] = cluster_sum;
                    level_sum += cluster_sum;
#else
                    level_weight_vp->elements[ level ] = cluster_sum;
#endif
                }

                if (level_index_mp != NULL)
                {
                    ERE(vp_get_indexed_vector(&sorted_l_p_vp, level_weight_vp));
                    ERE(descend_sort_indexed_vector(sorted_l_p_vp));

                    for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
                    {
                         P_l_p_mp->elements[ sparse_level ][ point ] = sorted_l_p_vp->elements[ sparse_level ].element;
                         level_index_mp->elements[ sparse_level ][ point ] = sorted_l_p_vp->elements[ sparse_level ].index;
                    }
                }
                else
                {
                    ERE(put_matrix_col(P_l_p_mp, level_weight_vp, point));
                }

                level_sum = sum_matrix_col_elements(P_l_p_mp, point);

                if (level_sum > 10.0 * DBL_MIN)
                {
                    for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
                    {
                        P_l_p_mp->elements[ sparse_level ][ point ] /= level_sum;
                    }
                }
                else
                {
                    for (sparse_level = first_sparse_level; sparse_level < num_sparse_levels; sparse_level++)
                    {
                        P_l_p_mp->elements[ sparse_level ][ point ] = 1.0 / (num_sparse_levels - first_sparse_level);
                    }
                }
            }

            count++;

            if (count % 1000 == 0)
            {
                verbose_pso(5, "%d points processed for get_cluster_membership_2() iteration %d.\n",
                            count, it + 1);
            }
        }

        log_likelihood = sum_vector_elements(fit_vp);

        ll_rel_diff = (log_likelihood - prev_log_likelihood);
        ll_rel_diff /= (ABS_OF(log_likelihood) + ABS_OF(prev_log_likelihood));
        ll_rel_diff *= 2.0;
        prev_log_likelihood = log_likelihood;

        if (fs_watch_2)
        {
            rms_diff = 0.0;
            max_diff = 0.0;

            pso(" %-3d | %.6e | %.3e |", 1 + it, log_likelihood, ll_rel_diff);

            prev_log_likelihood = log_likelihood;

            if (test_data_flag)
            {
                rms_diff = rms_matrix_difference(P_c_p_mp,
                                                 test_P_c_p_mp);
                max_diff = max_abs_matrix_difference(P_c_p_mp,
                                                     test_P_c_p_mp);
            }

            pso(" %.3e | %.3e\n", rms_diff, max_diff);
        }

        kjb_fflush((FILE*)NULL);

        if (iteration_atn_flag)
        {
            pso("Stopping because of halt request via signal.\n");
            break;
        }
        else if (is_file(halt_file_name))
        {
            pso("Stopping because of halt request via file.\n");
            EPE(kjb_unlink(halt_file_name));
            break;
        }
        /*
        // If we are not fitting both levels and clusters, then more iterations
        // won't do much.
        */
        else if (    ((P_l_p_c_mvp == NULL) && (P_l_p_mp == NULL))
                  || ((P_c_p_mpp == NULL) && (input_P_c_p_mp != NULL))
                )
        {
            if (fs_watch_2)
            {
                pso("\nStopping because we are not fitting both clusters and aspects.\n");
            }
            break;
        }

        else if (ll_rel_diff < iteration_tolerance)
        {
            break;
        }
    }

    if (fs_watch_2)
    {
        pso("\n---------\n");
    }

    if (fit_vp_ptr != NULL)
    {
        ERE(copy_vector(fit_vp_ptr, fit_vp));
    }

    free_vector(fit_vp);
    free_vector(col_sum_vp);

    if (P_c_p_mpp != NULL)
    {
        /* Generally lock_P_c_p_mp is just P_c_p_mp. */
        ERE(copy_matrix(P_c_p_mpp, lock_P_c_p_mp));
    }

    free_matrix(P_c_p_mp);
    free_vector(log_nu_vp);
    free_vector(nu_vp);
    free_vector(level_weight_vp);
    free_vector(log_prob_vp);
    free_vector(nu_for_dependent_model_vp);

#ifdef XXX_NOT_SURE_WHAT_IT_IS_DOING
    free_matrix(con_score_mp);
#endif
    free_vector(con_score_vp);
    free_int_vector(con_score_counts_vp);

    free_indexed_vector(sorted_l_p_vp);

    if (log_likelihood_ptr != NULL)
    {
        *log_likelihood_ptr = log_likelihood;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_con_prob_cache
(
    int               max_num_con_items,
    int               first_level,
    int               last_level_num,
    int               num_levels,
    const Int_vector* level_counts_vp
)
{
    static int         prev_max_num_con_items = NOT_SET;
    static int         prev_first_level       = NOT_SET;
    static int         prev_last_level_num       = NOT_SET;
    static int         prev_num_levels        = NOT_SET;
    int                con_item;
    int                level;
    int                num_nodes_this_level;


    if (max_num_con_items <= 0) return NO_ERROR;

    if (    (max_num_con_items == prev_max_num_con_items)
         && (first_level == prev_first_level)
         && (last_level_num == prev_last_level_num)
         && (num_levels == prev_num_levels)
       )
    {
        int wrong_size = FALSE;

        /*
        // If prev_num_levels is set, then fs_level_counts_vp is not NULL, and
        // has length prev_num_levels.
        */

        for (level = first_level; level < last_level_num; level++)
        {
            if (fs_level_counts_vp->elements[ level ] != level_counts_vp->elements[ level ])
            {
                wrong_size = TRUE;
                break;
            }
        }

        if ( ! wrong_size) return NO_ERROR;
    }

    SKIP_HEAP_CHECK_2();
    ERE(get_target_v3(&fs_con_prob_cache_vvvp, max_num_con_items));
    CONTINUE_HEAP_CHECK_2();

    for (con_item = 0; con_item < max_num_con_items; con_item++)
    {
        SKIP_HEAP_CHECK_2();
        ERE(get_target_vector_vector(&(fs_con_prob_cache_vvvp->elements[ con_item ]),
                                     num_levels));
        CONTINUE_HEAP_CHECK_2();

        for (level = first_level; level < last_level_num; level++)
        {
            num_nodes_this_level = level_counts_vp->elements[ level ];

            SKIP_HEAP_CHECK_2();
#ifdef TEST
            ERE(get_initialized_vector(&(fs_con_prob_cache_vvvp->elements[ con_item ]->elements[ level ]),
                                       num_nodes_this_level, DBL_NOT_SET));
#else
            ERE(get_target_vector(&(fs_con_prob_cache_vvvp->elements[ con_item ]->elements[ level ]),
                                  num_nodes_this_level));
#endif
            CONTINUE_HEAP_CHECK_2();
        }
    }

    SKIP_HEAP_CHECK_2();
    ERE(copy_int_vector(&fs_level_counts_vp, level_counts_vp));
    CONTINUE_HEAP_CHECK_2();

    prev_num_levels = num_levels;
    prev_first_level = first_level;
    prev_last_level_num = last_level_num;
    prev_max_num_con_items = max_num_con_items;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_level_indicators_cache
(
    int num_levels,
    int num_clusters,
    int num_items
)
{
    static int prev_num_clusters = NOT_SET;
    static int prev_num_items    = NOT_SET;
    static int prev_num_levels   = NOT_SET;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (    (num_clusters != prev_num_clusters)
         || (num_items != prev_num_items)
         || (num_levels != prev_num_levels)
       )
    {
        verbose_pso(5, "Resetting level indicator caches.\n");

        SKIP_HEAP_CHECK_2();

        free_3D_double_array(fs_cached_level_indicators_ppp);

        NRE(fs_cached_level_indicators_ppp = allocate_3D_double_array(num_levels,
                                                                   num_clusters,
                                                                   num_items));
        CONTINUE_HEAP_CHECK_2();

        prev_num_levels   = num_levels;
        prev_num_clusters = num_clusters;
        prev_num_items    = num_items;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_dis_factors
(
    Vector**          factor_vp_ptr,
    const Int_matrix* item_mp,
    const Matrix*     dis_item_multiplicity_mp,
    double            max_num_dis_items_with_multiplicity,
    int               norm_items,
    double            prob_boost_factor
)
{
    const int num_points    = item_mp->num_rows;
    const int max_num_items = item_mp->num_cols;
    Vector*   factor_vp;
    int       point;
    int       item;


    ERE(get_target_vector(factor_vp_ptr, num_points));
    factor_vp = *factor_vp_ptr;

    for (point = 0; point < num_points; point++)
    {
        double factor   = (prob_boost_factor < 0.0) ? 1.0 : prob_boost_factor;
        int*   item_ptr = item_mp->elements[ point ];
        double* dis_item_multiplicity_ptr = dis_item_multiplicity_mp->elements[ point ];
        double  dis_item_count                 = 0.0;
        int*    temp_item_ptr                = item_ptr;
        double* temp_dis_item_multiplicity_ptr = dis_item_multiplicity_ptr;

        /*
        // Technically we only need this if norm_items is set, but we do
        // the double duty of checking that each document has at least one
        // item.
        */
        for (item = 0; item < max_num_items; item++)
        {
            int category = *temp_item_ptr;

            ASSERT_IS_EQUAL_INT(*temp_dis_item_multiplicity_ptr, dis_item_multiplicity_mp->elements[ point ][ item ]);
            ASSERT_IS_EQUAL_INT(category, item_mp->elements[ point ][ item ]);

            if (category >= 0)
            {
                dis_item_count += (*temp_dis_item_multiplicity_ptr);
            }

            temp_item_ptr++;
            temp_dis_item_multiplicity_ptr++;
        }

        ASSERT_IS_NON_NEGATIVE_DBL(dis_item_count);

        if (dis_item_count < 1e-10)
        {
#ifdef HOW_IT_WAS_JUNE_29_03
            /*
            // Generally dis_item_count will be a small integer but we want to
            // allow for the possibility that it is fractional due to using
            // multiplicities less than one. If there are no dis times, then we
            // do not normalize to avoid dividing by zero. In this case the
            // factor should never be used (checked in the case of TEST).
            */
            SET_ARGUMENT_BUG();
            return ERROR;
#else
            factor = DBL_NOT_SET;
#endif
        }
        else if (norm_items)
        {
            factor *= max_num_dis_items_with_multiplicity;
            factor /= dis_item_count;
        }

        factor_vp->elements[ point ]= factor;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_con_factors
(
    Vector**             factor_vp_ptr,
    const Matrix_vector* item_mvp,
    int                  max_num_items,
    int                  norm_items,
    double               prob_boost_factor
)
{
    const int num_points        = item_mvp->length;
    Vector*   factor_vp;
    int       point;


    ERE(get_target_vector(factor_vp_ptr, num_points));
    factor_vp = *factor_vp_ptr;

    for (point = 0; point < num_points; point++)
    {
        double factor    = (prob_boost_factor < 0.0) ? 1.0 : prob_boost_factor;
        int    num_items = item_mvp->elements[ point ]->num_rows;

        if (num_items <= 0)
        {
#ifdef HOW_IT_WAS_JUNE_29_03
            /*
            // If we are clustering on an item, then every document must
            // have at least one.
            */
            SET_ARGUMENT_BUG();
            return ERROR;
#else
            factor = DBL_NOT_SET;
#endif
        }
        else if (norm_items)
        {
            factor *= max_num_items;
            factor /= num_items;
        }

        factor_vp->elements[ point ] = factor;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef VEC_AND_HIS
static int get_vec_factors
(
    Vector**       factor_vp_ptr,
    const V_v_v_v* item_vvvvp,
    int            max_num_items,
    int            norm_items,
    double         prob_boost_factor
)
{
    const int num_points    = item_vvvvp->length;
    Vector*   factor_vp;
    int       point;


    ERE(get_target_vector(factor_vp_ptr, num_points));
    factor_vp = *factor_vp_ptr;

    for (point = 0; point < num_points; point++)
    {
        double factor = (prob_boost_factor < 0.0) ? 1.0 : prob_boost_factor;
        int num_items = item_vvvvp->elements[ point ]->length;

        if (num_items <= 0)
        {
#ifdef HOW_IT_WAS_JUNE_29_03
            /*
            // If we are clustering on an item, then every document must
            // have at least one.
            */
            SET_ARGUMENT_BUG();
            return ERROR;
#else
            factor = DBL_NOT_SET;
#endif
        }
        else if (norm_items)
        {
            factor *= max_num_items;
            factor /= num_items;
        }

        factor_vp->elements[ point ] = factor;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_his_factors
(
    Vector**       factor_vp_ptr,
    const V_v_v_v* item_vvvvp,
    int            max_num_items,
    int            norm_items,
    double         prob_boost_factor
)
{
    const int num_points    = item_vvvvp->length;
    Vector*   factor_vp;
    int       point;


    ERE(get_target_vector(factor_vp_ptr, num_points));
    factor_vp = *factor_vp_ptr;

    for (point = 0; point < num_points; point++)
    {
        double factor = (prob_boost_factor < 0.0) ? 1.0 : prob_boost_factor;
        int num_items = item_vvvvp->elements[ point ]->length;

        if (num_items <= 0)
        {
#ifdef HOW_IT_WAS_JUNE_29_03
            /*
            // If we are clustering on an item, then every document must
            // have at least one.
            */
            SET_ARGUMENT_BUG();
            return ERROR;
#else
            factor = DBL_NOT_SET;
#endif
        }
        else if (norm_items)
        {

            factor *= max_num_items;
            factor /= num_items;
        }

        factor_vp->elements[ point ] = factor;
    }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_limited_cluster_membership
(
    Int_matrix*   cluster_index_mp,
    const Vector* P_c_p_vp,
    int           point,
    int           num_limited_clusters,
    double*       error_ptr
)
{
    int                    cluster;
#ifdef TEST
    const int num_clusters = P_c_p_vp->length;
#endif
    double limited_sum = 0.0;
#ifdef TEST
    double total_sum;
#endif

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif


    SKIP_HEAP_CHECK_2();
    ERE(vp_get_indexed_vector(&fs_cached_cluster_index_vp, P_c_p_vp));
    CONTINUE_HEAP_CHECK_2();

    ERE(descend_sort_indexed_vector(fs_cached_cluster_index_vp));

    for (cluster = 0; cluster < num_limited_clusters; cluster++)
    {
        int index = fs_cached_cluster_index_vp->elements[ cluster ].index;

        cluster_index_mp->elements[ point ][ cluster ] = index;

        limited_sum += fs_cached_cluster_index_vp->elements[ cluster ].element;
    }


#ifdef TEST
    total_sum = limited_sum;

    for (cluster = num_limited_clusters; cluster < num_clusters; cluster++)
    {
        total_sum += fs_cached_cluster_index_vp->elements[ cluster ].element;
    }

    ASSERT_IS_GREATER_DBL(total_sum, 0.999);
    ASSERT_IS_LESS_DBL(total_sum, 1.001);
#endif

    *error_ptr = 1.0 - limited_sum;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef XXX_SUPPORT_TRIMMING
static void sum_collected_scores
(
    int               num_items,
    int               base_count,
    const Int_vector* score_counts_vp,
    const Matrix*     score_mp,
    Vector*           score_vp
)
{
    int    item;
    int    i;
    double max_log_prob;
    double log_prob;

    for (item = 0; item < num_items; item++)
    {
        int count = score_counts_vp->elements[ item ];

        if (count == 0)
        {
            score_vp->elements[ base_count + item ] = DBL_HALF_MOST_NEGATIVE;
        }
        else
        {
            /* Use max trick to stabalize sum. */

            max_log_prob = DBL_MOST_NEGATIVE;

            for (i = 0; i < count; i++)
            {
                log_prob = score_mp->elements[ item ][ i ];
                max_log_prob = MAX_OF(max_log_prob, log_prob);
            }

            score_vp->elements[ base_count + item ] = 0.0;

            for (i = 0; i < count; i++)
            {
                log_prob = score_mp->elements[ item ][ i ];

                score_vp->elements[ base_count + item ] +=
                    exp(log_prob - max_log_prob);
            }

            score_vp->elements[ base_count + item ] =
                log(score_vp->elements[ base_count + item ]) + max_log_prob;

        }
    }

    verify_vector(score_vp, NULL);
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// Routine to set up a vector of the number of features in each vector of
// features so that we can de-weight each of them by their length.
//
// To save time, we only do it once, BUT WE HAVE NO INVALIDATION STRATEGY!!!
*/

static int initialize_vector_feature_counts(const Int_vector* con_feature_enable_vp,
                                            const Int_vector* con_feature_group_vp)
{
    int num_con_features;
    int con_feature      = 0;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    fs_vector_feature_counts_set = TRUE;

    if (con_feature_group_vp == NULL)
    {
        return NO_ERROR;
    }

    num_con_features = con_feature_group_vp->length;

    while (con_feature < num_con_features)
    {
        int i;
        int count = 0;

        if (    (    (con_feature_enable_vp != NULL)
                  && ( ! (con_feature_enable_vp->elements[ con_feature ]))
                )
             || (con_feature_group_vp->elements[ con_feature ] >= 0)
           )
        {
            con_feature++;
            continue;
        }

        ASSERT_IS_NON_NEGATIVE_INT(num_con_features - con_feature);

        for (i = 0; i < num_con_features - con_feature; i++)
        {
            if (con_feature_group_vp->elements[ con_feature ] != con_feature_group_vp->elements[ con_feature + i])
            {
                break;
            }
            count++;
        }

        ASSERT_IS_POSITIVE_INT(count);

        if (fs_vector_feature_counts_ptr == NULL)
        {
            NRE(fs_vector_feature_counts_ptr = INT_MALLOC(num_con_features));

            for (i = 0; i < num_con_features; i++)
            {
                fs_vector_feature_counts_ptr[ i ] = 0;
            }
        }

        for (i = 0; i <count ; i++)
        {
            fs_vector_feature_counts_ptr[ con_feature + i ] = count;
        }

        con_feature += count;

        p_stderr("\nVector feature found with %d components.\n\n",
                count );

    }

    return NO_ERROR;
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
    free_int_vector(fs_sub_group_vp);

    free_indexed_vector(fs_cached_cluster_index_vp);
    fs_cached_cluster_index_vp      = NULL;

    free_3D_double_array(fs_cached_level_indicators_ppp);
    fs_cached_level_indicators_ppp  = NULL;

    free_v3(fs_con_prob_cache_vvvp);
    fs_con_prob_cache_vvvp          = NULL;

    free_int_vector(fs_level_counts_vp);
    fs_level_counts_vp              = NULL;

    kjb_free(fs_vector_feature_counts_ptr);
    fs_vector_feature_counts_ptr    = NULL;
    fs_vector_feature_counts_set    = FALSE;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

