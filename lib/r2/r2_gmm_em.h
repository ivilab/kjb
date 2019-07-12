/* $Id: r2_gmm_em.h 16540 2014-03-13 18:04:57Z ksimek $ */
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
#ifndef R_GMM_EM_H
#define R_GMM_EM_H

#include <l_mt/l_mt_pthread.h>
#include "m/m_gen.h"


#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

#ifdef KJB_HAVE_PTHREAD
struct thread_GMM_data
{
    int           num_clusters;
    int           num_iteration;
    int           start_index;
    int           end_index;
    const Matrix* feature_mp;
    const Int_vector* held_out_indicator_vp;
    Vector*       a_vp; 
    Vector**      x_vpp;
    Vector**      x2_vpp;
    Vector**      I_vpp;
    Vector**      p_sum_vpp;
    Vector**      p_square_sum_vpp;
    Vector**      log_sqrt_det_vpp;
    Matrix*       u_mp;
    Matrix*       var_mp;
    Matrix*       new_u_mp;
    Matrix*       new_var_mp;
    Matrix*       I_mp;
    Matrix**      P_mpp;
    double*       log_likelihood_ptr;
    double*       held_out_log_likelihood_ptr;
    kjb_pthread_mutex_t* mutexsum_p;
};
#endif /* KJB_HAVE_PTHREAD*/

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
);

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

int get_independent_GMM_2_with_missing_data
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


#ifdef KJB_HAVE_PTHREAD
int get_independent_GMM_3_mt
(
    int               num_threads,
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

void *create_independent_GMM_thread (void *arg);
#endif /* KJB_HAVE_PTHREAD*/


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

