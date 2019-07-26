
/* $Id: m_mat_norm.h 11913 2012-03-16 18:01:03Z ykk $ */

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

#ifndef M_MAT_NORM_INCLUDED
#define M_MAT_NORM_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


double frobenius_matrix_norm(const Matrix* mp);

int get_matrix_col_norms(Vector** result_vpp, const Matrix* mp);
int get_matrix_row_norms(Vector** result_vpp, const Matrix* mp);

int ow_scale_matrix_by_max_abs(Matrix*);
int ow_scale_matrix_rows_by_sums(Matrix*);
int ow_scale_matrix_rows_by_max_abs(Matrix*);
int ow_scale_matrix_rows_by_max(Matrix*);
int ow_scale_matrix_by_max(Matrix*);
int ow_scale_matrix_by_max_abs(Matrix*);

#ifdef TEST
#   define safe_ow_scale_matrix_row_by_sum(x,y) debug_safe_ow_scale_matrix_row_by_sum(x, y, __FILE__, __LINE__)
#   define safe_ow_scale_matrix_rows_by_sums(x) debug_safe_ow_scale_matrix_rows_by_sums(x, __FILE__, __LINE__)

    int debug_safe_ow_scale_matrix_row_by_sum
    (
        Matrix*     mp,
        int         row, 
        const char* file,
        int         line
    );

    int debug_safe_ow_scale_matrix_rows_by_sums
    (
        Matrix*     mp,
        const char* file,
        int         line
    );
#else
    int safe_ow_scale_matrix_row_by_sum(Matrix* mp, int row);
    int safe_ow_scale_matrix_rows_by_sums(Matrix* mp);
#endif

int ow_scale_matrix_by_sum(Matrix* mp);

int safe_ow_scale_matrix_cols_by_sums(Matrix* mp);
int ow_scale_matrix_cols_by_sums(Matrix* mp);

double max_matrix_element(const Matrix*);
double min_matrix_element(const Matrix*);
double max_abs_matrix_element(const Matrix*);
double min_abs_matrix_element(const Matrix* mp);

int get_min_matrix_element
(
    const Matrix* mp,
    double*       max_ptr,
    int*          max_i_ptr,
    int*          max_j_ptr
);

int get_max_matrix_element
(
    const Matrix* mp,
    double*       max_ptr,
    int*          max_i_ptr,
    int*          max_j_ptr
);

int get_min_matrix_col_elements(Vector** result_vpp, const Matrix* mp);

int get_max_matrix_col_elements(Vector** result_vpp, const Matrix* mp);

int get_max_matrix_row_elements(Vector** result_vpp, const Matrix* mp);

int get_max_int_matrix_row_elements(Int_vector** result_ivpp, const Int_matrix* imp);

int get_min_matrix_row_elements(Vector** result_vpp, const Matrix* mp);

int get_max_matrix_row_elements_2
(
    Vector**      result_vpp,
    Int_vector**  index_vpp,
    const Matrix* mp
);

int get_max_int_matrix_row_elements_2
(
    Int_vector**      result_ivpp,
    Int_vector**      index_vpp,
    const Int_matrix* imp
);

int get_min_matrix_row_elements_2
(
    Vector**      result_vpp,
    Int_vector**  index_vpp,
    const Matrix* mp
);

int get_max_matrix_row_sum(const Matrix* mp, double* max_sum_ptr);

int get_max_matrix_row_product(const Matrix* mp, double* max_product_ptr);

int min_thresh_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        min_val
);

int ow_min_abs_thresh_matrix(Matrix* source_mp, double min_val);
int ow_min_thresh_matrix(Matrix* source_mp, double min_val);

int ow_min_thresh_matrix_col(Matrix* source_mp, int col, double min_val);

int max_thresh_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    double        max_val
);

int ow_max_thresh_matrix(Matrix* source_mp, double max_val);

int ow_max_thresh_matrix_col(Matrix* source_mp, int col, double max_val);

int normalize_matrix_rows(Matrix** target_mpp, const Matrix* input_mp);

int ow_normalize_matrix_rows(Matrix* mp);

int normalize_matrix_cols(Matrix** target_mpp, const Matrix* input_mp);

int ow_normalize_matrix_cols(Matrix* mp);

int ow_scale_matrix_row_by_sum(Matrix* mp, int row);
int ow_scale_matrix_col_by_sum(Matrix* mp, int col);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


