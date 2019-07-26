
/* $Id: m_mat_arith.h 6610 2010-08-27 22:28:15Z delpero $ */

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

#ifndef M_MAT_ARITH_INCLUDED
#define M_MAT_ARITH_INCLUDED


#include "m/m_matrix.h"
#include "l/l_int_matrix.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int get_vector_outer_product
(
    Matrix**      mpp,
    const Vector* vp1,
    const Vector* vp2
);

int add_matrices
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Matrix* second_mp
);

int ow_add_matrices(Matrix* first_mp, const Matrix* second_mp);

int ow_add_matrices_2
(
    Matrix*       first_mp,
    int           row_offset,
    int           col_offset,
    const Matrix* second_mp 
);

int subtract_matrices
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Matrix* second_mp
);

int ow_subtract_matrices(Matrix* first_mp, const Matrix* second_mp);

int multiply_matrices_ew
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Matrix* second_mp
);

int ow_multiply_matrices_ew(Matrix* first_mp, const Matrix* second_mp);

int ow_multiply_matrices_ew_2
(
    Matrix*       first_mp,
    int           row_offset,
    int           col_offset,
    const Matrix* second_mp 
);

int divide_matrices_ew
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Matrix* second_mp
);

int ow_divide_matrices_ew(Matrix* first_mp, const Matrix* second_mp);

int ow_add_matrix_times_scalar
(
    Matrix*       first_mp,
    const Matrix* second_mp,
    double        scalar
);

int ow_add_matrix_times_scalar_2
(
    Matrix*       first_mp,
    int           row_offset,
    int           col_offset,
    const Matrix* second_mp,
    double        scalar 
);

void ow_add_int_matrix_to_matrix
(
    Matrix*           target_mp,
    const Int_matrix* source_mp
);

int multiply_matrices
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Matrix* second_mp
);

int multiply_by_transpose
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Matrix* second_mp
);

int multiply_with_transpose
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Matrix* second_mp
);

int invert_matrix_elements(Matrix** target_mpp, const Matrix* source_mp);

int square_matrix_elements(Matrix** target_mpp, const Matrix* source_mp);

int sqrt_matrix_elements(Matrix** target_mpp, const Matrix* source_mp);

int exp_matrix_elements(Matrix** target_mpp, const Matrix* source_mp);

int log_matrix_elements
(
    Matrix**      target_mpp,
    const Matrix* source_mp
);

int log_matrix_elements_2
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        log_zero
);

int add_scalar_to_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        scalar
);

int subtract_scalar_from_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        scalar
);

int multiply_matrix_by_scalar
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        scalar
);

int divide_matrix_by_scalar
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        scalar
);

int ow_invert_matrix_elements(Matrix* source_mp);
int ow_square_matrix_elements(Matrix* source_mp);
int ow_sqrt_matrix_elements(Matrix* source_mp);
int ow_exp_matrix_elements(Matrix* source_mp);

int ow_log_matrix_elements(Matrix* source_mp);
int ow_log_matrix_elements_2(Matrix* source_mp, double log_zero);

int ow_add_scalar_to_matrix(Matrix* source_mp, double scalar);

int ow_subtract_scalar_from_matrix(Matrix* source_mp, double scalar);

int ow_multiply_matrix_by_scalar(Matrix* source_mp, double scalar);

int ow_divide_matrix_by_scalar(Matrix* source_mp, double scalar);

int multiply_vector_and_matrix
(
    Vector**      output_vpp,
    const Vector* input_vp,
    const Matrix* input_mp
);

int multiply_matrix_and_vector
(
    Vector**      output_vpp,
    const Matrix* input_mp,
    const Vector* input_vp
);

int multiply_matrix_rows
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Matrix* second_mp
);

int add_row_vector_to_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Vector* vp
);

int subtract_row_vector_from_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Vector* vp
);

int multiply_matrix_by_row_vector_ew
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Vector* vp
);

int divide_matrix_by_row_vector
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Vector* vp
);

int ow_add_row_vector_to_matrix(Matrix* source_mp, const Vector* vp);

int ow_subtract_row_vector_from_matrix
(
    Matrix*       source_mp,
    const Vector* vp
);

int ow_multiply_matrix_by_row_vector_ew
(
    Matrix*       source_mp,
    const Vector* vp
);

int ow_divide_matrix_by_row_vector(Matrix* source_mp, const Vector* vp);

int add_col_vector_to_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Vector* vp
);

int ow_add_col_vector_to_matrix(Matrix* source_mp, const Vector* vp);

int subtract_col_vector_from_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Vector* vp
);

int ow_subtract_col_vector_from_matrix
(
    Matrix*       source_mp,
    const Vector* vp
);

int multiply_matrix_by_col_vector_ew
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Vector* vp
);

int ow_multiply_matrix_by_col_vector_ew
(
    Matrix*       source_mp,
    const Vector* vp
);

int divide_matrix_by_col_vector
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Vector* vp
);

int ow_divide_matrix_by_col_vector(Matrix* source_mp, const Vector* vp);

int ow_add_vector_to_matrix_row(Matrix* mp, const Vector* vp, int row);
int ow_add_vector_to_matrix_col(Matrix* mp, const Vector* vp, int col);

int ow_add_scalar_times_vector_to_matrix_row(Matrix* mp, const Vector* vp,
                                             double scalar, int row);

int ow_subtract_vector_from_matrix_row
(
    Matrix*       mp,
    const Vector* vp,
    int           row
);

int ow_multiply_matrix_row_by_vector
(
    Matrix*       mp,
    const Vector* vp,
    int           row
);

int ow_divide_matrix_row_by_vector(Matrix* mp, const Vector* vp, int row);

int ow_multiply_matrix_col_by_vector(Matrix* mp, const Vector* vp, int col);

int ow_add_scalar_to_matrix_row(Matrix* mp, double scalar, int row);

int ow_subtract_scalar_from_matrix_row
(
    Matrix* mp,
    double  scalar,
    int     row
);

int ow_multiply_matrix_row_by_scalar(Matrix* mp, double scalar, int row);

int ow_divide_matrix_row_by_scalar(Matrix* mp, double scalar, int row);


int ow_add_matrix_row_times_scalar
(
    Matrix*       target_mp,
    int           target_row,
    const Matrix* source_mp,
    int           source_row,
    double        scalar 
);
    
int ow_add_matrix_rows_ew(Matrix* mp, int row, const Matrix* mp2, int row2);
int ow_multiply_matrix_rows_ew(Matrix* mp, int row, const Matrix* mp2, int row2);

double sum_matrix_elements(const Matrix* mp);

double sum_matrix_row_elements(const Matrix* mp, int row);

double sum_matrix_col_elements(const Matrix* mp, int col);

double average_matrix_elements(const Matrix* mp);

int ow_subtract_identity_matrix(Matrix* mp);

int do_matrix_recomposition
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Vector* vp,
    const Matrix* second_mp
);

int do_matrix_recomposition_2
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Vector* vp,
    const Matrix* second_mp
);

double log_sum_log_matrix_elements(Matrix* log_mp);

double ow_exp_scale_by_sum_log_matrix_row(Matrix* log_mp, int row);

int multiply_by_own_transpose
(
    Matrix**      target_mpp,
    const Matrix* source_mp
);

int get_dot_product_of_matrix_rows
(
    const Matrix* mp,
    int           r1,
    int           r2,
    double*       dot_prod_ptr
);

int get_dot_product_of_matrix_rows_2
(
    const Matrix* first_mp,
    int           r1,
    const Matrix* second_mp,
    int           r2,
    double*       dot_prod_ptr
);

int ow_add_matrix_row_to_vector(Vector* vp, const Matrix* mp, int row);

int ow_get_abs_of_matrix(Matrix* mp);

int get_abs_of_matrix(Matrix** target_mpp, const Matrix* source_mp);

void get_euler_rotation_matrix
(
    Matrix  ** target_mpp,
    float      phi,
    float      theta,
    float      psi
);

void get_euler_homo_rotation_matrix
(
    Matrix  ** target_mpp,
    float      phi,
    float      theta,
    float      psi
);

int get_3d_rotation_matrix_1
(
    Matrix**      target_mpp,
    double          phi,
    const Vector* v
);

int get_2d_rotation_matrix
(
    Matrix  ** target_mpp,
    double     phi
);

int get_3d_rotation_matrix_2
(
    Matrix  ** target_mpp,
    double     phi,
    double     x,
    double     y,
    double     z
);

int get_3d_homo_rotation_matrix_1
(
    Matrix**      target_mpp,
    double          phi,
    const Vector* v
);

int get_2d_homo_rotation_matrix
(
    Matrix**   target_mpp,
    double     phi
);

int get_3d_homo_rotation_matrix_2
(
    Matrix**   target_mpp,
    double     phi,
    double     x,
    double     y,
    double     z
);

int get_3d_scaling_matrix_1
(
    Matrix**      target_mpp,
    const Vector* v
);

int get_3d_scaling_matrix_2
(
    Matrix** target_mpp,
    double     x,
    double     y,
    double     z
);

int get_3d_homo_scaling_matrix_1
(
    Matrix**      target_mpp,
    const Vector* v
);

int get_3d_homo_scaling_matrix_2
(
    Matrix**   target_mpp,
    double     x,
    double     y,
    double     z
);

int get_3d_homo_translation_matrix_1
(
    Matrix**      target_mpp,
    const Vector* v
);

int get_3d_homo_translation_matrix_2
(
    Matrix**   target_mpp,
    double     x,
    double     y,
    double     z
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


