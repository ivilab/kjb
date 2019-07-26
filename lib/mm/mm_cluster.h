
/* $Id: mm_cluster.h 20918 2016-10-31 22:08:27Z kobus $ */

/* =========================================================================== *
|
| Copyright (c) 2000-2008 by Kobus Barnard (author), UCB, and UA. Currently,
| the use of this code is retricted to the UA and UCB image and text projects.
|
* =========================================================================== */

#ifndef MM_CLUSTER_INCLUDED
#define MM_CLUSTER_INCLUDED


#include "mm/mm_type.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
#define XXX_HOW_IT_WAS_OCT_5_03
*/


int set_multi_modal_cluster_options(const char* option, const char* value);

int do_multi_modal_clustering
(
    const Topology_vector* topology_vp,
    const Int_vector*      dis_item_level_vp,
    const Cluster_data*    data_ptr,
    const Cluster_data*    held_out_data_ptr,
    Multi_modal_model*    model_ptr,
    int                    (*output_model_fn) (const Multi_modal_model*, int, const char*),
    const char*            output_dir,
    double                 (*dis_item_loss_fn)(int, const double*)
);


int get_cluster_membership
(
    Matrix**                  P_c_p_mp_ptr,
    Matrix_vector**           P_l_p_c_mvp_ptr,
    Int_matrix_vector**       cluster_level_index_mvpp,
    Matrix**                  P_l_p_mp_ptr,
    Int_matrix**              level_index_mp,
    Vector**                  fit_vp_ptr,
    const Multi_modal_model* model_ptr,
    const Cluster_data*       data_ptr,
    const Int_vector*         points_to_use_vp,
    int                       use_factors_flag,
    int                       test_data_flag,
    Vector_vector*            con_score_cutoff_vpp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

