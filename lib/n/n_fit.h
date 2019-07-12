
/* $Id: n_fit.h 12108 2012-04-18 07:39:01Z ksimek $ */

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

#ifndef N_FIT_INCLUDED
#define N_FIT_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_least_squares_options(const char* option, const char* value);

int get_best_diagonal_post_map
(
    Vector**      result_vpp,
    const Matrix* in_mp,
    const Matrix* out_mp
);

int get_diagonal_post_map_error
(
    const Vector* diag_trans_vp,
    const Matrix* in_mp,
    const Matrix* out_mp,
    double*       error_ptr
);

int get_diagonal_post_map_relative_row_error
(
    const Vector* diag_trans_vp,
    const Matrix* in_mp,
    const Matrix* out_mp,
    double*       error_ptr
);

int get_best_post_map
(
    Matrix**      best_map_mpp,
    const Matrix* in_mp,
    const Matrix* out_mp
);

int get_best_map
(
    Matrix**      best_map_mpp,
    const Matrix* in_mp,
    const Matrix* out_mp
);

int get_post_map_error
(
    const Matrix* map_mp,
    const Matrix* in_mp,
    const Matrix* out_mp,
    double*       error_ptr
);

int get_post_map_relative_row_error
(
    const Matrix* map_mp,
    const Matrix* in_mp,
    const Matrix* out_mp,
    double*       error_ptr
);

/*
 * For legacy code.
*/
#define least_squares_solve least_squares

int least_squares
(
    Vector**      result_vpp,
    const Matrix* mp,
    const Vector* vp
);

int least_squares_2
(
    Vector**      result_vpp,
    const Matrix* A_mp,
    const Vector* b_vp,
    double*       error_ptr
);

int get_linear_equation_rms_error
(
    const Matrix* a_mp,
    const Vector* x_vp,
    const Vector* b_vp,
    double*       error_ptr 
);

int get_svd_basis_for_rows
(
    const Matrix* mp,
    Matrix**      basis_mpp,
    Vector**      singular_vpp
);

int get_row_fits
(
    Matrix**      estimate_mpp,
    const Matrix* observed_mp,
    int           num_PC,
    const Matrix* PC_mp,
    double*       error_ptr
);

int project_rows_onto_basis
(
    Matrix**      estimate_mpp,
    const Matrix* observed_mp,
    const Matrix* basis_mp,
    double*       error_ptr
);

int get_best_linear_fit
(
    Vector**      result_vpp,
    const Vector* x_vp,
    const Vector* y_vp
);

int get_best_linear_fit_2
(
    Vector**      result_vpp,
    const Vector* x_vp,
    const Vector* y_vp,
    double* error_ptr
);

int get_two_mode_basis_for_rows
(
    const Matrix* P_mp,
    int           num_A_basis_vectors,
    const Matrix* A_mp,
    Matrix**      A_basis_mpp,
    int           num_B_basis_vectors,
    const Matrix* B_mp,
    Matrix**      B_basis_mpp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

