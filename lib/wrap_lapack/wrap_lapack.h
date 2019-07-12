
/* $Id: wrap_lapack.h 17997 2014-10-30 06:36:07Z ksimek $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.      |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */

#ifndef WRAP_LAPACK_INCLUDED
#define WRAP_LAPACK_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int have_lapack(void);

int lapack_solve_triangular
(
    const Matrix* A_mp,
    const Matrix* B_mp,
    Matrix**      X_mpp
);

int lapack_solve_upper_triangular(
        const Matrix* A_mp,
        const Matrix* B_mp,
        Matrix** X_mpp);

int lapack_solve_symmetric
(
    const Matrix* A_mp,
    const Matrix* B_mp,
    Matrix**      X_mpp
);

int lapack_solve_symmetric_pd
(
    const Matrix* A_mp,
    const Matrix* B_mp,
    Matrix**      X_mpp
);

int lapack_solve
(
    const Matrix* A_mp,
    const Matrix* B_mp,
    Matrix**      X_mpp
);


int lapack_diagonalize(const Matrix* mp, Matrix** E_mpp, Vector** D_vpp);

int lapack_diagonalize_2
(
    const Matrix* mp,
    Matrix**      E_re_mpp,
    Matrix**      E_im_mpp,
    Vector**      D_re_vpp,
    Vector**      D_im_vpp
);

int lapack_qr_decompose
(
    const Matrix* mp,
    Matrix** Q_mpp, 
    Matrix** R_mpp
);

int lapack_diagonalize_symmetric
(
    const Matrix* mp,
    Matrix**      E_mpp,
    Vector**      D_vpp
);

int do_lapack_svd
(
    const Matrix* a_mp,
    Matrix**      u_mpp,
    Vector**      d_vpp,
    Matrix**      vt_mpp
);

int do_lapack_matrix_inversion
(
    Matrix**      target_mpp,
    const Matrix* input_mp
);

int do_lapack_matrix_inversion_2
(
    Matrix**      target_mpp,
    const Matrix* input_mp,
    double* log_det,
    int* det_sign
);

int do_lapack_cholesky_decomposition
(
    Matrix* input_mp
);

int do_lapack_cholesky_decomposition_2
(
    Matrix** target_mpp,
    const Matrix* input_mp
);

#ifdef CURRENTLY_DOES_NOT_ALWAYS_WORK
int do_lapack_dot_product
(
    const Vector* vp1,
    const Vector* vp2,
    double *dp_ptr
);
#endif


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

