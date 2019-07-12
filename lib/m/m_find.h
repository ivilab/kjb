
/* $Id: m_find.h 4884 2009-11-24 22:43:59Z ernesto $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2007 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowleged in publications, and relevent papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarentee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#ifndef M_FIND_INCLUDED
#define M_FIND_INCLUDED

#include "l/l_incl.h"
#include "m/m_matrix.h"
#include "m/m_vector.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* Finding procedures */

int find_in_vector(Int_vector** indices, const Vector* vp, int (*predicate)(const Vector*, int, void*), void* params);

int find_in_matrix_by_rows(Int_vector** indices, const Matrix* mp, int (*predicate)(const Matrix*, int, void*), void* params);

int find_in_matrix_by_cols(Int_vector** indices, const Matrix* mp, int (*predicate)(const Matrix*, int, void*), void* params);

int find_in_matrix(Int_matrix** elems, const Matrix* mp, int (*predicate)(const Matrix*, int, int, void*), void* params);

int find_in_matrix_as_vector(Int_vector** indices, const Matrix* mp, int (*predicate)(const Matrix*, int, int, void*), void* params);

/* Copying procedures */

int copy_vector_with_selection(Vector** target_vpp, const Vector* source_vp, const Int_vector* elems);

int copy_matrix_with_selection(Matrix** target_mpp, const Matrix* source_mp, const Int_vector* rows, const Int_vector* cols);

int copy_matrix_with_selection_2(Matrix** target_mpp, const Matrix* source_mp, const Int_vector* rows, const Int_vector* cols);

int get_matrix_as_vector_with_selection(Vector** target_vpp, const Matrix* mp, const Int_vector* elems);

/* Predicates for finding procedures */

/* For finding in vectors */

int is_element_zero(const Vector* vp, int elem, void* params);

int is_element_nonzero(const Vector* vp, int elem, void* params);

int is_element_nan(const Vector* vp, int elem, void* params);

int is_element_nonnan(const Vector* vp, int elem, void* params);

int is_element_equal_to(const Vector* vp, int elem, void* params);

int is_element_different_from(const Vector* vp, int elem, void* params);

int is_element_greater_than(const Vector* vp, int elem, void* params);

int is_element_less_than(const Vector* vp, int elem, void* params);

/* For finding in matrices by rows or columns */

int is_row_sum_equal_to(const Matrix* mp, int row, void* params);

int is_row_sum_different_from(const Matrix* mp, int row, void* params);

int is_column_sum_equal_to(const Matrix* mp, int col, void* params);

int is_column_sum_different_from(const Matrix* mp, int col, void* params);

int is_row_sum_less_than(const Matrix* mp, int row, void* params);

int is_row_sum_greater_than(const Matrix* mp, int row, void* params);

int is_column_sum_less_than(const Matrix* mp, int col, void* params);

int is_column_sum_greater_than(const Matrix* mp, int col, void* params);

/* For finding in matrices by elements */

int is_matrix_element_zero(const Matrix* mp, int row, int col, void* params);

int is_matrix_element_nonzero(const Matrix* mp, int row, int col, void* params);

int is_matrix_element_nan(const Matrix* mp, int row, int col, void* params);

int is_matrix_element_nonnan(const Matrix* mp, int row, int col, void* params);

int is_matrix_element_equal_to(const Matrix* mp, int row, int col, void* params);

int is_matrix_element_different_from(const Matrix* mp, int row, int col, void* params);

int is_matrix_element_greater_than(const Matrix* mp, int row, int col, void* params);

int is_matrix_element_less_than(const Matrix* mp, int row, int col, void* params);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif /* M_FIND_INCLUDED */

