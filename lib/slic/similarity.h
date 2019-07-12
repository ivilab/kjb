/* $Id: similarity.h 15688 2013-10-14 08:46:32Z predoehl $
 */
#ifndef SLIC_SIMILARITY_DEFINED_H_
#define SLIC_SIMILARITY_DEFINED_H_

#include "slic/basic.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int fit_similarity
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    Matrix       **a_mpp,
    double       *fit_err_ptr
);

int similarity_inverse
(
    const Matrix *a_mp,
    const Matrix *mp,
    Matrix       **res_mpp
);

int similarity_transform_single_point
(
    const Matrix *a_mp,
    const Vector *x_vp,
    Vector       **y_vpp
);

int similarity_transform
(
    const Matrix *a_mp,
    const Matrix *x_mp,
    Matrix       **y_mpp
);

int get_similarity_fitting_error
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp,
    double *fit_err_ptr
);
        
int get_similarity_distance
(
    const Matrix *x_mp,
    const Matrix *y_mp,
    const Matrix *a_mp, 
    Vector       **dist_vpp
);

int is_similarity_degenerate
(
    const Matrix *x_mp
);

int constrained_similarity
(
    const Matrix *a_mp
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
