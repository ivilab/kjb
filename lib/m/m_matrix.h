
/* $Id: m_matrix.h 6352 2010-07-11 20:13:21Z kobus $ */

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

#ifndef M_MATRIX_INCLUDED
#define M_MATRIX_INCLUDED


#include "l/l_gen.h"
#include "m/m_vector.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
#    ifdef PROGRAMMER_IS_kobus
         /* This a bit dangerous as often freeing non-initialized memory is not
          * really a bug. */
/*
*/
#        define CHECK_MATRIX_INITIALZATION_ON_FREE
#    endif
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                   Matrix
 *
 * KJB library matrix type
 *
 * This type is the matrix type for the KJB library and is used by many
 * routines. It stores a matrix with num_rows rows and num_cols cols. If "mp"
 * is a pointer to the type Matrix, then mp->elements[ row ][ col ] accesses
 * the element (row, col), and mp->elements[ row ] accesses the row'th row.
 * Note that counting starts at 0. The elements of the matrix are doubles.
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
 *    get_target_matrix, free_matrix
 *
 * Index: 
 *    matrices, data types
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Matrix
{
    int    num_rows;        /* Current number of rows from the callers point of view. */
    int    num_cols;        /* Current number of cols from the callers point of view. */
    double **elements;      /* A pointer to row pointers. */
    int    max_num_elements; /* Private: The number of elements that can be stored. */
    int    max_num_rows;     /* Private: The number of rows that we have pointers for. */
    int    max_num_cols;     /* Private: The current wrapping count. */
}
Matrix;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

#    define SET_MAT_SIZE(mp, new_num_rows, new_num_cols) \
        UNTESTED_CODE(); \
        \
        if (    ((new_num_rows < 0) || (new_num_rows > mp->max_num_rows)) \
             || ((new_num_cols < 0) || (new_num_cols > mp->max_num_cols)) \
           ) \
        { \
            SET_BOUNDS_BUG(); \
        } \
        else \
        { \
            mp->num_rows = new_num_rows; \
            mp->num_cols = new_num_cols; \
        }


#    define SET_MAT_ROWS(mp, new_num_rows) \
        \
        if ((new_num_rows < 0) || (new_num_rows > mp->max_num_rows)) \
        { \
            SET_BOUNDS_BUG(); \
        } \
        else \
        { \
            mp->num_rows = new_num_rows; \
        }


#    define SET_MAT_COLS(mp, new_num_cols) \
        UNTESTED_CODE(); \
        \
        if ((new_num_cols < 0) || (new_num_cols > mp->max_num_cols)) \
        { \
            SET_BOUNDS_BUG(); \
        } \
        else \
        { \
            mp->num_cols = new_num_cols; \
        }

#    define MAT_VAL(mp,i,j)     debug_mat_val(mp, i, j, __FILE__, __LINE__)

     double debug_mat_val
     (
         const Matrix* mp,
         int           i,
         int           j,
         const char*   file_name,
         int           line_number
     );

#else
        /*  Production versions  */

#    define SET_MAT_SIZE(mp, new_num_rows, new_num_cols) \
            mp->num_rows = new_num_rows; \
            mp->num_cols = new_num_cols;


#    define SET_MAT_ROWS(mp, new_num_rows) \
            mp->num_rows = new_num_rows;


#    define SET_MAT_COLS(mp, new_num_cols) \
            mp->num_cols = new_num_cols;

#    define MAT_VAL(mp,i,j)  (mp->elements[ i ][ j ])

#endif


/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION

#   define get_zero_matrix(x, y, z)   \
                            debug_get_zero_matrix(x, y, z, __FILE__, __LINE__)

#   define get_unity_matrix(x, y, z)   \
                            debug_get_unity_matrix(x, y, z, __FILE__, __LINE__)

#   define get_initialized_matrix(w, x, y, z)   \
                  debug_get_initialized_matrix(w, x, y, z, __FILE__, __LINE__)

#   define get_target_matrix(x, y, z)   \
                            debug_get_target_matrix(x, y, z, __FILE__, __LINE__)

#   define create_matrix(x,y)  DEBUG_create_matrix(x,y, __FILE__, __LINE__)


    int debug_get_zero_matrix
    (
        Matrix**    target_mpp,
        int         num_rows,
        int         num_cols,
        const char* file_name,
        int         line_number
    );

    int debug_get_unity_matrix
    (
        Matrix**    target_mpp,
        int         num_rows,
        int         num_cols,
        const char* file_name,
        int         line_number
    );

    int debug_get_initialized_matrix
    (
        Matrix**    target_mpp,
        int         num_rows,
        int         num_cols,
        double      initial_value,
        const char* file_name,
        int         line_number
    );

    int debug_get_target_matrix
    (
        Matrix**    target_mpp,
        int         num_rows,
        int         num_cols,
        const char* file_name,
        int         line_number
    );

    Matrix* DEBUG_create_matrix
    (
        int         rows,
        int         cols,
        const char* file_name,
        int         line_number
    );

#else
    int get_zero_matrix(Matrix** target_mpp, int num_rows, int num_cols);

    int get_unity_matrix(Matrix** target_mpp, int num_rows, int num_cols);

    int get_initialized_matrix
    (
        Matrix** target_mpp,
        int      num_rows,
        int      num_cols,
        double   initial_value
    );

    Matrix* create_matrix(int rows, int cols);

    int get_target_matrix
    (
        Matrix** out_arg_mp,
        int      num_rows,
        int      num_cols
    );
#endif

void free_matrix        (Matrix* mp);
int  get_identity_matrix(Matrix** output_mpp, int size);
int  ow_zero_matrix     (Matrix*);
int  ow_set_matrix      (Matrix*, double);

int  get_diagonal_matrix(Matrix** mp, const Vector* vp);

int  get_random_matrix
(
    Matrix** target_mpp,
    int      num_rows,
    int      num_cols
);

int  get_random_matrix_2
(
    Matrix** target_mpp,
    int      num_rows,
    int      num_cols
);

int  ow_perturb_matrix   (Matrix* mp, double fraction);
int  is_symmetric_matrix (const Matrix* mp);

/*
 * Will be static soon.
*/
Matrix* create_identity_matrix (int);
Matrix* create_zero_matrix     (int num_rows, int num_cols);
Matrix* create_diagonal_matrix (const Vector*);
Matrix* create_random_matrix   (int, int);
Matrix* create_random_matrix_2 (int num_rows, int num_cols);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

