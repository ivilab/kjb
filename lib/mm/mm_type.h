
/* $Id: mm_type.h 22174 2018-07-01 21:49:18Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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

#ifndef MM_TYPE_INCLUDED
#define MM_TYPE_INCLUDED


#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define MM_CONTIGUOUS_PRIOR  (5.0)


typedef enum Convergence_criterion
{
    MAX_DATA_LL,
    MAX_HELD_OUT_LL,
    MAX_COMBINED_LL,
    FIRST_HELD_OUT_LL_MAX,
    SINGLE_ITERATION,
    THREE_ITERATIONS,
    NUM_CONVERGENCE_CRITERIA
}
Convergence_criterion;


/*
 * This will likely become something like
 *
 * typedef struct Cluster_data
 * {
 *    Vector*        weight_vp;
 *    int num_points;
 *    Int_vector*    id_vp;
 *    Int_vector*    label_vp;
 *    int num_con_groups
 *    Cluster_con_data** con_data_ptr[]
 *    int num_dis_groups
 *    Cluster_dis_data** dis_data_ptr[]
 * }
 * Cluster_data;
 *
 * (Perhaps some of the meta_data_ptr bit in "it" should move here: e.g.,
 * "vocabulary_ptr --> dis_item_labels_ptr)?)
 *
 */

typedef struct Cluster_data
{
    /*
     * The following group of quantities are duplicated in the "it" program in
     * Meta_data. Typically they should be consistent across data sets and hence
     * apply to more than than one data set. However, at this level, we don't
     * make any such assumption. These variables are generally "convenience"
     * variables as they are implicit in the structures that follow, except that
     * num_categories and num_tokens allow for entities that occur in some data
     * set (e.g. training), but not necessarily in another one (e.g. testing).
     * The maximum integers in dis_item_mp and con_item_token_mp should be less
     * than num_categories and num_tokens respectively.
    */
    int num_categories;
    int num_con_features;
    int num_vec_features;
    int num_his_features;
    int num_tokens;

    /*
     * Quantities which apply to only to a specific data set.
    */
    int            num_points;
    int            max_num_dis_items;
    double         max_num_dis_items_with_multiplicity;
    int            max_num_con_items;
    int            max_num_vec_items;
    int            max_num_his_items;
    Vector*        weight_vp;
    Int_vector*    group_vp;
    Vector_vector* con_item_weight_vvp;
    Int_vector*    dis_predict_flag_vp;  /* Not used for anything yet. */
    Int_matrix*    dis_item_mp;
    Matrix*        dis_item_multiplicity_mp;
    Matrix*        dis_item_aux_mp;
    Vector_vector* dis_item_aux_value_vvp;
    Int_vector_vector* dis_item_aux_index_vvp;
    Int_matrix*    dis_item_observed_counts_mp;
    Int_matrix*    dis_item_derived_counts_mp;
    Matrix*        dis_category_level_prior_mp;  /* Not used for anything yet. */
    Int_vector*    con_predict_flag_vp;  /* Not used for anything yet. */
    Int_matrix*    con_item_token_mp;
    Matrix_vector* con_item_mvp;
    Int_vector_vector* con_item_index_vvp;
    Matrix_vector* con_item_info_mvp;
    Matrix_vector* con_item_merge_prior_mvp;
    V_v_v_v*       vec_item_vvvvp;
    Int_vector*    vec_num_dim_vp;
    V_v_v_v*       his_item_vvvvp;
    Int_vector*    his_num_dim_vp;
    Int_vector*    id_vp;
    Int_vector*    label_vp;
    Matrix_vector* blob_word_prior_mvp;
    Matrix_vector* blob_word_posterior_mvp;
}
Cluster_data;


typedef struct Topology
{
    int            num_levels_per_category;
    int            num_levels;
    int            num_clusters;
    int            first_level;
    int            last_level_num;
    Int_vector*    fan_out_vp;
    Int_vector*    level_counts_vp;
    Int_matrix*    node_mp;
}
Topology;


typedef struct Topology_vector
{
    int            num_topologies;
    Topology**     topologies;
}
Topology_vector;


typedef struct Multi_modal_model
{
    Topology*      topology_ptr;
    int            num_clusters;
    int            num_categories;
    int            num_con_features; 

    Int_vector*    con_feature_enable_vp;
    Int_vector*    con_feature_group_vp;
    Vector*        con_feature_weight_vp;

    int            discrete_tie_count;
    double         dis_item_prob_threshold;
    int            norm_items;
    int            norm_continuous_features;
    double         ignore_dis_prob_fraction;
    double         ignore_con_prob_fraction;
    double         dis_prob_boost_factor;
    double         con_prob_boost_factor;
    double         vec_prob_boost_factor;
    double         his_prob_boost_factor;
    double         var_offset;

    int            phase_one_num_iterations;
    int            phase_two_num_iterations;
    int            phase_one_uniform_vertical_dist;
    int            phase_two_uniform_vertical_dist;
    int            phase_one_cluster_dependent_vertical_dist;
    int            phase_two_cluster_dependent_vertical_dist;
    int            phase_one_model_correspondence;
    int            phase_two_model_correspondence;
    int            phase_one_share_matches;
    int            phase_two_share_matches;
    int            phase_one_sample_matches;
    int            phase_two_sample_matches;
    int            phase_one_last_sample;
    int            phase_two_last_sample;

    int            weight_con_items;
    int            norm_depend;

    Vector*        a_vp;
    Matrix*        V_mp;
    Vector*        V_vp;
    Matrix*        con_V_mp;
    Matrix*        dis_V_mp;
    Matrix*        P_c_p_mp;
    Matrix_vector* P_l_p_c_mvp;
    Matrix*        P_l_p_mp;
    Vector*        fit_vp;
    Matrix_vector* P_i_n_mvp;
    Matrix_vector* mean_mvp;
    Matrix_vector* var_mvp;
    Matrix*        mean_mp;
    Matrix*        var_mp;
    Vector_vector* con_log_sqrt_det_vvp;
    Vector*        con_log_sqrt_det_vp;
    double         con_log_norm;
    double         con_score_cutoff;
    double         min_con_log_prob;
    double         max_con_norm_var;

    double         extra_dis_prob;
    double         extra_con_prob;

    V_v_v_v*       vec_mean_vvvvp;
    V_v_v*         vec_var_vvvp;
    V_v_v_v*       his_mean_vvvvp;
    V_v_v*         his_var_vvvp;

    int            num_parameters;

    double         raw_log_likelihood;
    double         mdl_log_likelihood;
    double         aic;

    Cluster_data*  data_ptr; 
}
Multi_modal_model;


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

