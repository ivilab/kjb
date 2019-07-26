
/* $Id: r_cluster.h 9312 2011-04-13 06:49:57Z kobus $ */

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

#ifndef R_CLUSTER_INCLUDED
#define R_CLUSTER_INCLUDED

#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_em_cluster_options(const char* option, const char* value);


int select_independent_GMM
(
    int           max_num_clusters,
    const Matrix* feature_mp,
    Vector**      a_vpp,
    Matrix**      u_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp
);

int select_full_GMM
(
    int             max_num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp
);

int get_full_GMM
(
    int             num_clusters,
    const Matrix*   feature_mp,
    const Matrix*   covariance_mask_mp,
    Vector**        a_vpp,
    Matrix**        u_mpp,
    Matrix_vector** S_mvpp,
    Matrix**        P_mpp
);

int get_independent_GMM
(
    int           num_clusters,
    const Matrix* feature_mp,
    Vector**      a_vpp,
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp
);

int get_independent_GMM_2
(
    int           num_clusters,
    const Matrix* feature_mp,
    Vector**      a_vpp,
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_mpp,
    double*       log_likelihood_ptr 
);

int get_independent_GMM_3
(
    int               num_clusters,
    const Matrix*     feature_mp,
    const Int_vector* held_out_indicator_vp,
    const Vector*     initial_a_vp,
    const Matrix*     initial_means_mp,
    const Matrix*     initial_var_mp,
    Vector**          a_vpp,
    Matrix**          means_mpp,
    Matrix**          var_mpp,
    Matrix**          P_mpp,
    double*           log_likelihood_ptr,
    double*           held_out_log_likelihood_ptr,
    int*              num_iterations_ptr 
);

int select_multinomial_mixture_model
(
    int               max_num_clusters,
    int               num_categories,
    const Int_matrix* item_mp,
    const Matrix*     multiplicity_mp,
    Vector**          a_vpp,
    Matrix**          P_i_c_mpp,
    Matrix**          P_c_p_mpp
);

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
);

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
);

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
);

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
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_cluster_mpp
);

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
    Matrix**      means_mpp,
    Matrix**      var_mpp,
    Matrix**      P_cluster_mpp
);

int shift_point_cyclic
(
    Vector**      shifted_point_vp,
    const Vector* point_vp,
    const int     shift
);

int shift_point_with_extension
(
    Vector**      shifted_point_vp,
    const Vector* point_vp,
    const int     shift
);

int shift_point_with_zero_padding
(
    Vector**      shifted_point_vp,
    const Vector* point_vp,
    const int     shift
);

int get_GMM_blk_compound_sym_cov
(
    const Int_vector_vector* block_diag_sizes_vvp,
    const Matrix*            feature_mp,
    const Int_vector*        held_out_indicator_vp,
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
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

