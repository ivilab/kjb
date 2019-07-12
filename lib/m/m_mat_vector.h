
/* $Id: m_mat_vector.h 15081 2013-08-01 18:00:47Z predoehl $ */

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

#ifndef M_MAT_VECTOR_INCLUDED
#define M_MAT_VECTOR_INCLUDED


#include "l/l_gen.h"
#include "m/m_vector.h"
#include "m/m_matrix.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* =============================================================================
 *                                   Matrix_vector
 *
 * Type for an array of matrices
 *
 * This type is used in the KJB library for arrays of matrices.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Matrix_vector
{
    int      length;
    Matrix** elements;
}
Matrix_vector;


/* -------------------------------------------------------------------------- */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               Matrix_vector_vector
 *
 * Type for an array of arrays of matrices
 *
 * This type is used in the KJB library for arrays of arrays of matrices.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Matrix_vector_vector
{
    int             length;
    Matrix_vector** elements;
}
Matrix_vector_vector;


/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION

#   define allocate_2D_mp_array(x, y) \
                 debug_allocate_2D_mp_array((x), (y), __FILE__, __LINE__)


    Matrix*** debug_allocate_2D_mp_array(int, int, const char*, int);
#else
    Matrix*** allocate_2D_mp_array(int, int);
#endif

int get_target_matrix_vector(Matrix_vector** mvpp, int count);

int count_non_null_matrix_vector_matrices
(
    const Matrix_vector* in_mvp
);

int matrix_vectors_are_comparable
(
    const Matrix_vector* in_mvp,
    const Matrix_vector* out_mvp
);

Matrix_vector* create_matrix_vector(int count);

void free_matrix_vector(Matrix_vector*);

int                   get_target_matrix_vector_vector
(
    Matrix_vector_vector** mvvpp,
    int                    length
);

Matrix_vector_vector* create_matrix_vector_vector(int length);

void                  free_matrix_vector_vector
(
    Matrix_vector_vector* mvvp
);
Matrix_vector** create_matrix_vector_list(int count);

void free_matrix_vector_list
(
    int             count,
    Matrix_vector** matrix_vector_list
);

int interleave_matrix_rows
(
    Matrix**             target_mpp,
    const Matrix_vector* source_mvp
);

int interleave_matrix_cols
(
    Matrix**             target_mpp,
    const Matrix_vector* source_mvp
);

int concat_matrices_vertically
(
    Matrix**      mpp,
    int           num_matrices,
    const Matrix* matrix_list[] 
);

int concat_matrices_horizontally
(
    Matrix**      mpp,
    int           num_matrices,
    const Matrix* matrix_list[] 
);

int get_matrix_from_matrix_vector(Matrix** mpp, const Matrix_vector* mvp);

int get_matrix_vector_from_matrix_2
(
    Matrix_vector**  mvpp,
    const Matrix* mp,
    int block_size
);

int get_matrix_vector_from_matrix
(
    Matrix_vector**  mvpp,
    const Matrix* mp
);

int get_matrix_from_matrix_vector_with_col_selection
(
    Matrix**             mpp,
    const Matrix_vector* mvp,
    const Int_vector*    selected_cols_vp 
);

void free_2D_mp_array(Matrix***);

void free_2D_mp_array_and_matrices
(
    Matrix*** array,
    int       num_rows,
    int       num_cols
);

int is_matrix_vector_consistent(const Matrix_vector* mvp);

int average_matrices(Matrix** avg_mat, const Matrix_vector* matrices);

int std_dev_matrices
(
    Matrix**                std_dev_mat,
    const Matrix_vector*    matrices,
    const Matrix*           avg_mat
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

