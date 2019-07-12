/* $Id: affine.h 15688 2013-10-14 08:46:32Z predoehl $
 */
#ifndef SLIC_AFFINE_DEFINED_H_
#define SLIC_AFFINE_DEFINED_H_

#include "slic/basic.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int fit_affine
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    Matrix       **a_mpp,
    double       *fit_err_ptr
);

int affine_inverse
(
    const Matrix *a_mp,
    const Matrix *mp,
    Matrix       **res_mpp
);

int affine_transform_single_point
(
    const Matrix *a_mp,
    const Vector *x_vp,
    Vector       **y_vpp
);

int affine_transform
(
    const Matrix *a_mp,
    const Matrix *x_mp,
    Matrix       **y_mpp
);

int get_affine_fitting_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp,
    double *fit_err_ptr
);
        
int get_affine_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp, 
    Vector       **dist_vpp
);

/* 
   Ernesto added a struct in n/n_cholesky_decomposition
   So I renamed this to a more specific struct name.
 */
int cholesky_decomposition_SLIC
(
    const Matrix *mp,
    Matrix       **res_mpp
);

int affine_decomposition
(
    const Matrix *a_mp,
    Matrix       **R_mpp,
    Matrix       **S_mpp,
    Matrix       **Z_mpp
);

int is_affine_degenerate
(
    const Matrix *x_mp
);

int constrained_affine_1
(
    const Matrix *a_mp
);

int constraint_affine_2
(
    const Matrix *a_mp
);


int get_color_constancy_matrix
(
    const KJB_image *src_img,
    const KJB_image *target_img,
    const Int_matrix *mask_imp,
    Matrix **color_constancy_mpp
);

int apply_color_constancy
(
    const KJB_image *src_img,
    const Int_matrix *mask_imp,
    const Matrix *color_constancy_mp,
    KJB_image       **res_img
);

int do_color_constancy
(
    const KJB_image *src_img,
    const KJB_image *target_img,
    const Int_matrix *mask_imp,
    KJB_image       **res_img
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
