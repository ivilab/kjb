
/* $Id: mm_compute_con_prob.h 20918 2016-10-31 22:08:27Z kobus $ */

/* =========================================================================== *
|
| Copyright (c) 2000-2008 by Kobus Barnard (author), UCB, and UA. Currently,
| the use of this code is retricted to the UA and UCB image and text projects.
|
* =========================================================================== */

#ifndef MM_COMPUTE_CON_PROB_INCLUDED
#define MM_COMPUTE_CON_PROB_INCLUDED


#include "l/l_int_vector.h" 

#ifndef __C2MAN__

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int compute_con_prob
(
    const Int_vector* con_feature_enable_vp,
    const Int_vector* con_feature_group_vp,
    const double* x_ptr,
    const double* u_ptr,
    const double* v_ptr,
    double  normalizing_factor,
    int     norm_continuous_features,
    int     limit_gaussian_deviation,
    int     max_con_norm_var,
    const int*  vector_feature_counts_ptr,
    double* log_prob_ptr,
    double* max_con_log_prob_ptr,
    int enable_missing_data
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif    /* #ifndef __C2MAN__   */

#endif 



