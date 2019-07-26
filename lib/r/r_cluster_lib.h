
/* $Id: r_cluster_lib.h 8504 2011-01-31 18:11:23Z predoehl $ */

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

#ifndef R_CLUSTER_LIB_INCLUDED
#define R_CLUSTER_LIB_INCLUDED

#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_cluster_lib_options(const char* option, const char* value);

int get_cluster_random_matrix_1(Matrix** mpp, int num_rows, int num_cols);

int get_cluster_random_matrix_2(Matrix** mpp, int num_rows, int num_cols);

int get_cluster_random_matrix(Matrix** mpp, int num_rows, int num_cols);

int ow_perturb_unary_cluster_data
(
    Matrix* unary_feature_mp,
    double  perturbation,
    const Int_vector* disable_perturb_vp
);

int perturb_unary_cluster_data
(
    Matrix**      perturbed_unary_feature_mpp,
    const Matrix* unary_feature_mp,
    double        perturbation,
    const Int_vector* disable_perturb_vp
);

int perturb_composite_cluster_data
(
    Matrix_vector**      perturbed_composite_feature_mvpp,
    const Matrix_vector* composite_feature_mvp,
    double               perturbation,
    const Int_vector* disable_perturb_vp
);

int ow_perturb_composite_cluster_data
(
    Matrix_vector* composite_feature_mvp,
    double         perturbation,
    const Int_vector* disable_perturb_vp
);

int ow_normalize_composite_cluster_data
(
    Matrix_vector* composite_feature_mvp,
    Matrix_vector* held_out_composite_feature_mvp,
    Vector**       mean_vpp,
    Vector**       var_vpp,
    double         norm_stdev,
    const Int_vector* disable_normalize_vp

);

int ow_normalize_cluster_data
(
    Matrix*  feature_mvp,
    Matrix*  held_out_feature_mvp,
    Vector** mean_vpp,
    Vector** var_vpp,
    double   norm_stdev,
    const Int_vector* disable_normalize_vp
);

int ow_un_normalize_cluster_data
(
    Matrix*       model_mean_mp,
    Matrix*       model_var_mp,
    const Vector* mean_vp,
    const Vector* var_vp,
    double        norm_stdev
);

int plot_log_likelihood
(
    const Vector* log_likelihood_sum_vp,
    const Vector* counts_vp,
    const Vector* min_log_likelihood_vp,
    const Vector* max_log_likelihood_vp,
    const char*   output_dir,
    const char*   title_str
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


