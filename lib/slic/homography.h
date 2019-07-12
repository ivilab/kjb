/* $Id: homography.h 15688 2013-10-14 08:46:32Z predoehl $
 */
#ifndef SLIC_HOMOGRAPHY_DEFINED_H_
#define SLIC_HOMOGRAPHY_DEFINED_H_

#include "slic/basic.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int fit_homography
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    Matrix       **H_mpp,
    double       *fit_err_ptr
);

int normalize_2Dpoints
(
    const Matrix *pts_mp,
    Matrix       **new_pts_mpp,
    Matrix       **trans_mpp
);

int is_homography_degenerate
(
    const Matrix *x_mp
);

int get_homography_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *H_mp, 
    Vector       **dist_vpp
);

int get_dual_homography_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *H_mp, 
    Vector       **dist_vpp
);

int get_homography_fitting_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *H_mp,
    double *fit_err_ptr
);

int homography_inverse
(
    const Matrix *H_mp,
    const Matrix *mp,
    Matrix       **res_mpp
);

int homography_transform
(
    const Matrix *p_mp,
    const Matrix *x_mp,
    Matrix       **y_mpp
);

int constrained_homography_1
(
    const Matrix *H_mp
);

int constrained_homography_2
(
    const Matrix *H_mp
);

int get_unity_homography
(
    Matrix **h_mpp
);

int normalize_homography
(
    Matrix *h_mp
);

int verify_homography
(
    const Matrix *H_mp,
    const Matrix *x_mp,
    const Matrix *y_mp,
    double       *err_ptr
);

int verify_homography_set
(
    const Matrix_vector *H_mvp,
    const Matrix_vector *x_mvp,
    const Matrix_vector *y_mvp,
    Vector              **err_vpp
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
