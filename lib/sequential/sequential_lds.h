
/* $Id: */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
* =========================================================================== */

#ifndef SEQUENTIAL_LDS_INCLUDED
#define SEQUENTIAL_LDS_INCLUDED

#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int sample_from_LDS
(
    Vector_vector**      x,
    Vector_vector**      y,
    int                  N,
    const Matrix*        A,
    const Matrix*        Q,
    const Matrix*        H,
    const Matrix*        R,
    const Vector*        mu_0,
    const Matrix*        S_0
);

int sample_from_LDS_2
(
    Vector_vector**      x,
    Vector_vector**      y,
    int                  N,
    const Matrix_vector* A,
    const Matrix_vector* Q,
    const Matrix_vector* H,
    const Matrix_vector* R,
    const Vector*        mu_0,
    const Matrix*        S_0
);

int compute_kalman_filter
(
    Vector_vector**      means,
    Matrix_vector**      covariances,
    double*              likelihood,
    const Vector_vector* y,
    const Matrix*        A,
    const Matrix*        Q,
    const Matrix*        H,
    const Matrix*        R,
    const Vector*        mu_0,
    const Matrix*        S_0
);

int compute_kalman_filter_stable
(
    Vector_vector**      means,
    Matrix_vector**      covariances,
    double*              likelihood,
    const Vector_vector* y,
    const Matrix*        A,
    const Matrix*        Q,
    const Matrix*        H,
    const Matrix*        R,
    const Vector*        mu_0,
    const Matrix*        S_0
);

int compute_kalman_filter_2
(
    Vector_vector**      means,
    Matrix_vector**      covariances,
    double*              likelihood,
    const Vector_vector* y,
    const Matrix_vector* A,
    const Matrix_vector* Q,
    const Matrix_vector* H,
    const Matrix_vector* R,
    const Vector*        mu_0,
    const Matrix*        S_0
);

int compute_kalman_filter_2_stable
(
    Vector_vector**      means,
    Matrix_vector**      covariances,
    double*              likelihood,
    const Vector_vector* y,
    const Matrix_vector* A,
    const Matrix_vector* Q,
    const Matrix_vector* H,
    const Matrix_vector* R,
    const Vector*        mu_0,
    const Matrix*        S_0
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

