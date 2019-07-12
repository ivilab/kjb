
/* $Id: m_mat_matrix.h 22184 2018-07-16 00:08:22Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2012 by Kobus Barnard.
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

#ifndef M_MAT_MATRIX_INCLUDED
#define M_MAT_MATRIX_INCLUDED


#include "l/l_gen.h"
#include "l/l_int_matrix.h"
#include "m/m_matrix.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* =============================================================================
 *                                   Matrix_matrix
 *
 * Type for a matrix of matrices
 *
 * This type is used in the KJB library for matrices of matrices.
 *
 * It stores a main matrix with num_rows rows and num_cols cols, whose elements 
 * are also matrices. If "mp" is a pointer to the type Matrix, then 
 * mp->elements[ row ][ col ] accesses the matrix stored at (row, col), and 
 * mp->elements[ row ] accesses the row'th row.
 * Note that counting starts at 0. 
 *
 * The matrix rows may or may not be stored consecutively in memory. Because of
 * this, and because some routines may (most don't) take advantage of knowlege
 * of the internal structure for performance, it is important NOT to swap rows
 * by swapping pointers -- the elements should be copied.
 *
 * The sizes (num_rows and num_cols) must both be nonnegative.
 * Conventionally, num_rows and num_cols are both positive;
 * however, it is also acceptable to have num_rows and num_cols both zero.
 * Anything else (such as a mix of positive and zero sizes) is regarded as a
 * bug in the calling program -- set_bug(3) is called and (if it returns) 
 * ERROR is returned.
 *
 * Warning:
 *    As described above, the user of this library must take care with
 *    assumptions about the structure of the array embedded in the matrix type.
 *
 * Related:
 *    create_matrix_matrix, free_matrix_matrix
 *
 * Index: 
 *    matrices, data types
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Matrix_matrix
{
    int    num_rows;        /* Current number of rows from the callers point of view. */
    int    num_cols;        /* Current number of cols from the callers point of view. */
    Matrix*** elements;      /* A pointer to row pointers. */
    int    max_num_elements; /* Private: The number of elements that can be stored. */
    int    max_num_rows;     /* Private: The number of rows that we have pointers for. */
    int    max_num_cols;     /* Private: The current wrapping count. */
}
Matrix_matrix;

/* OBSOLETE, delete soon (or change to "target"). FIXME */ 
Matrix_matrix* create_matrix_matrix
(
    int num_rows, 
    int num_cols, 
    int elt_rows,
    int elt_cols
);
 
/* OBSOLETE, delete soon. (There are no homography semantics here!) FIXME */
Matrix_matrix* create_homography_matrix_matrix(int num_rows, int num_cols);

void free_matrix_matrix (Matrix_matrix* mmp);


/* -------------------------------------------------------------------------- */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* FIXME. If we want this, it should be in the l library. */

/* =============================================================================
 *                                Int_matrix_matrix
 *
 * Type for a matrix of integer matrices
 *
 * This type is used in the KJB library for matrices of integer matrices.
 *
 * It stores a main matrix with num_rows rows and num_cols cols, whose elements 
 * are also matrices. If "mp" is a pointer to the type Matrix, then 
 * mp->elements[ row ][ col ] accesses the matrix stored at (row, col), and 
 * mp->elements[ row ] accesses the row'th row.
 * Note that counting starts at 0. 
 *
 * The matrix rows may or may not be stored consecutively in memory. Because of
 * this, and because some routines may (most don't) take advantage of knowlege
 * of the internal structure for performance, it is important NOT to swap rows
 * by swapping pointers -- the elements should be copied.
 *
 * The sizes (num_rows and num_cols) must both be nonnegative.
 * Conventionally, num_rows and num_cols are both positive;
 * however, it is also acceptable to have num_rows and num_cols both zero.
 * Anything else (such as a mix of positive and zero sizes) is regarded as a
 * bug in the calling program -- set_bug(3) is called and (if it returns) 
 * ERROR is returned.
 *
 * Warning:
 *    As described above, the user of this library must take care with
 *    assumptions about the structure of the array embedded in the matrix type.
 *
 * Related:
 *    create_int_matrix_matrix, free_int_matrix_matrix
 *
 * Index: 
 *    matrices, data types
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Int_matrix_matrix
{
    int    num_rows;        /* Current number of rows from the callers point of view. */
    int    num_cols;        /* Current number of cols from the callers point of view. */
    Int_matrix*** elements;      /* A pointer to row pointers. */
    int    max_num_elements; /* Private: The number of elements that can be stored. */
    int    max_num_rows;     /* Private: The number of rows that we have pointers for. */
    int    max_num_cols;     /* Private: The current wrapping count. */
}
Int_matrix_matrix;

/* FIXME. Create routines should not be exposed. But SLIC probably uses this
 * one. */
Int_matrix_matrix* create_int_matrix_matrix
(
    int num_rows, 
    int num_cols
);

/* FIXME. Create routines should not be exposed. Do we need this at all? */
Int_matrix_matrix* create_int_matrix_matrix_with_submatrices
(
    int num_rows, 
    int num_cols, 
    int elt_rows,
    int elt_cols
);

/*
#ifdef TRACK_MEMORY_ALLOCATION

Int_matrix*** debug_allocate_2D_int_mp_array
(
    int num_rows, 
    int num_cols, 
    const char* file_name, 
    int line_number
);
#else
*/
/* FIXME. The debug allocation versions seem unfinished.  */
Int_matrix*** allocate_2D_int_mp_array(int num_rows, int num_cols);
/*#endif*/
/* #ifdef TRACK_MEMORY_ALLOCATION ... #else ... */

void free_2D_int_mp_array(Int_matrix*** array);

void free_2D_int_mp_array_and_matrices
(
    Int_matrix*** array,
    int       num_rows,
    int       num_cols
);

void free_int_matrix_matrix (Int_matrix_matrix* mmp);


/* -------------------------------------------------------------------------- */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */




#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

