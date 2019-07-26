
/* $Id: mm_cluster_lib.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|
| Copyright (c) 2000-2008 by Kobus Barnard (author), UCB, and UA. Currently,
| the use of this code is retricted to the UA and UCB image and text projects.
|
* =========================================================================== */

#ifndef MM_CLUSTER_LIB_INCLUDED
#define MM_CLUSTER_LIB_INCLUDED


#include "mm/mm_type.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_multi_modal_cluster_lib_options(const char* option, const char* value);

int get_initial_vertical_indicators
(
    Matrix** V_l_c_mpp,
    int      first_level,    /* Counting starts at zero. */
    int      last_level_num, /* Counting starts at one --> "_num" */
    int      num_levels,
    int      num_clusters
);

int get_initial_horizontal_indicators
(
    Matrix** P_c_p_mpp,
    int      num_points,
    int      num_clusters
);

int copy_multi_modal_model
(
    Multi_modal_model**      target_model_ptr_ptr,
    const Multi_modal_model* source_model_ptr
);

int compute_mdl_log_likelihood
(
    int                 num_points,
    Multi_modal_model* model_ptr
);

int compute_mdl_log_likelihood_2
(
    int                 num_points,
    Multi_modal_model* model_ptr,
    double              raw_log_likelihood,
    double*             mdl_ll_ptr,
    double*             aic_ll_ptr
);

int get_target_topology_vector
(
    Topology_vector** topology_vpp,
    int               num_topologies
);

void free_topology_vector(Topology_vector* topology_vp);

int get_topology
(
    Topology** topology_ptr_ptr,
    int        num_levels,
    int        first_level,
    int        last_level_num,
    int        fan_out,
    int        first_fan_out,
    int        last_fan_out
);


int get_topology_2
(
    Topology** topology_ptr_ptr,
    const Int_vector* fan_out_vp,
    int        first_level,
    int        last_level_num
);


Topology* allocate_topology(void);

void free_topology(Topology* topology_ptr);

int copy_topology
(
    Topology**      target_topology_ptr_ptr,
    const Topology* source_topology_ptr
);

Cluster_data* allocate_cluster_data(void);

int copy_cluster_data
(
    Cluster_data**      target_data_ptr_ptr,
    const Cluster_data* source_data_ptr
);

int split_cluster_data
(
    Cluster_data**      train_data_ptr_ptr,
    Cluster_data**      held_out_data_ptr_ptr,
    const Cluster_data* source_data_ptr,
    const Int_vector*   training_image_nums_vp,
    const Int_vector*   held_out_image_nums_vp
);

int get_target_cluster_data_like_source
(
    Cluster_data**      target_data_ptr_ptr,
    const Cluster_data* source_data_ptr,
    int                 num_points
);

int set_max_num_cluster_data_items(Cluster_data* data_ptr);

void free_cluster_data(Cluster_data* data_ptr);

Multi_modal_model* allocate_multi_modal_model(void);

void free_multi_modal_model(Multi_modal_model* model_ptr);

Multi_modal_model* allocate_multi_modal_model(void);

int ow_un_normalize_multi_modal_model
(
    Multi_modal_model* model_ptr,
    const Vector*       mean_vp,
    const Vector*       var_vp,
    double              norm_stdev
);

void free_multi_modal_model(Multi_modal_model* model_ptr);

void verify_cluster_data(const Cluster_data* data_ptr);

int get_matches
(
    Int_matrix**  match_mpp,
    int*          num_matches_ptr,
    const Matrix* log_pair_probs_mp,
    int           use_ml_sampling,
    int           num_nulls,
    int           duplicate_words_for_matching
);

int add_discrete_nulls_to_data
(
    Cluster_data**      data_with_nulls_pp,
    const Cluster_data* data_ptr
);

int subtract_discrete_nulls_from_model(Multi_modal_model* model_ptr);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

