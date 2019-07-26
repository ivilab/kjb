
/* $Id: l_int_matrix.h 21520 2017-07-22 15:09:04Z kobus $ */

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

#ifndef L_INT_ARRAY_INCLUDED
#define L_INT_ARRAY_INCLUDED


#include "l/l_def.h"
#include "l/l_int_vector.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* =============================================================================
 *                                   Int_matrix
 *
 * Int_matrix type
 *
 * This type is the int_matrix type for the KJB library and is used by many
 * routines. It stores a int_matrix with num_rows rows and num_cols cols. If "mp"
 * is a pointer to the type Int_matrix, then mp->elements[ row ][ col ] accesses
 * the element (row, col), and mp->elements[ row ] accesses the row'th row.
 * Note that counting starts at 0.
 *
 * The integer array rows may or may not be stored consecutively in memory.
 * Because of this, and because some routines may (most don't) take advantage
 * of knowlege of the internal structure for performance, it is important NOT
 * to swap rows by swapping pointers -- the elements should be copied. Even if
 * no routines make use of the internal structutre, swapping rows by swapping
 * pointers will make it so that the memory does not get free'd properly.
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
 *    assumptions about the structure of the array embedded in the int_matrix
 *    type.
 *
 * Related:
 *    get_target_int_matrix, free_int_matrix
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Int_matrix
{
    int num_rows;        /* Current number of rows from the callers point of view. */
    int num_cols;        /* Current number of cols from the callers point of view. */
    int **elements;      /* Where the data is. */
    int max_num_elements; /* Private: The number of elements that can be stored. */
    int max_num_rows;     /* Private: The number of rows that we have pointers for. */
    int max_num_cols;     /* Private: The current wrapping count. */
}
Int_matrix;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        Int_matrix_vector
 *
 * Type for an array of integer matrices
 *
 * This type is used in the KJB library for arrays of integer matrices.
 *
 * Index: integer matrices, integer matrix vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Int_matrix_vector
{
    int          length;
    Int_matrix** elements;
}
Int_matrix_vector;


/* -------------------------------------------------------------------------- */


/* =============================================================================
 *                        Int_vector_matrix
 *
 * Type for an 2D array of integer vectors
 *
 * This type is used in the KJB library for 2D arrays of integer vectors.
 *
 * Index: integer vectors, integer vector matrices
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Int_vector_matrix
{
    int           num_rows;
    int           num_cols;
    Int_vector*** elements;
}
Int_vector_matrix;


/* -------------------------------------------------------------------------- */


#ifdef TEST

#    define SET_INT_MAT_SIZE(mp, new_num_rows, new_num_cols) \
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


#    define SET_INT_MAT_ROWS(mp,new_num_rows) \
        UNTESTED_CODE(); \
        \
        if ((new_num_rows < 0) || (new_num_rows > mp->max_num_rows)) \
        { \
            SET_BOUNDS_BUG(); \
        } \
        else \
        { \
            mp->num_rows = new_num_rows; \
        }


#    define SET_INT_MAT_COLS(mp,new_num_cols) \
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

#    define INT_MAT_VAL(mp,i,j)     debug_int_mat_val(mp, i, j, __FILE__, __LINE__)

     int debug_int_mat_val
     (
         const Int_matrix* mp,
         int               i,
         int               j,
         const char*       file_name,
         int               line_number
     );

#else
        /*  Production versions  */

#    define SET_INT_MAT_SIZE(mp, new_num_rows, new_num_cols) \
            mp->num_rows = new_num_rows; \
            mp->num_cols = new_num_cols;


#    define SET_INT_MAT_ROWS(mp, new_num_rows) \
            mp->num_rows = new_num_rows;


#    define SET_INT_MAT_COLS(mp, new_num_cols) \
            mp->num_cols = new_num_cols;

#    define INT_MAT_VAL(mp,i,j)  (mp->elements[ i ][ j ])


#endif


/* -------------------------------------------------------------------------- */

int same_int_matrix_dimensions
(
    const Int_matrix* first_mp,
    const Int_matrix* second_mp
);

int get_zero_int_matrix
(
    Int_matrix** target_mpp,
    int          num_rows,
    int          num_cols
);

int get_initialized_int_matrix
(
    Int_matrix** target_mpp,
    int          num_rows,
    int          num_cols,
    int          initial_value
);

#ifdef TRACK_MEMORY_ALLOCATION

#   define get_target_int_matrix(x, y, z)   \
                        debug_get_target_int_matrix(x, y, z, __FILE__, __LINE__)

    int debug_get_target_int_matrix
    (
        Int_matrix** target_mpp,
        int          num_rows,
        int          num_cols,
        const char*  file_name,
        int          line_number
    );

#   define ra_get_target_int_matrix(x, y, z)   \
                        debug_ra_get_target_int_matrix(x, y, z, __FILE__, __LINE__)

    int debug_ra_get_target_int_matrix
    (
        Int_matrix** target_mpp,
        int          num_rows,
        int          num_cols,
        const char*  file_name,
        int          line_number
    );

#else
    int get_target_int_matrix
    (
        Int_matrix** target_mpp,
        int          num_rows,
        int          num_cols
    );

    int ra_get_target_int_matrix
    (
        Int_matrix** target_mpp,
        int          num_rows,
        int          num_cols
    );
#endif

int  get_diagonal_int_matrix(Int_matrix** mp, const Int_vector* vp);

void free_int_matrix   (Int_matrix* mp);
int  ow_zero_int_matrix(Int_matrix*);
int  ow_set_int_matrix (Int_matrix*, int);

int  get_target_int_matrix_vector(Int_matrix_vector** mvpp, int length);
int  ra_get_target_int_matrix_vector(Int_matrix_vector** mvpp, int length);
void free_int_matrix_vector      (Int_matrix_vector*);

#if 0 /* was ifdef OBSOLETE */
Int_matrix_vector** create_int_matrix_vector_list(int count);
void free_int_matrix_vector_list(int count, Int_matrix_vector** int_mvpp);
#endif 

int get_target_int_vector_matrix(Int_vector_matrix **vmpp, int num_rows, int num_cols);

void free_int_vector_matrix(Int_vector_matrix *int_vmp);

int concat_int_matrices_vertically
(
    Int_matrix**      mpp,
    int               num_matrices,
    const Int_matrix* int_matrix_list[]
);

#if 0 /* was ifdef OBSOLETE */
Int_matrix* create_int_matrix_copy(const Int_matrix* source_mp);
#endif 

int split_int_matrix_by_rows
(
     Int_matrix** target_1_mpp,
     Int_matrix** target_2_mpp,
     const Int_matrix* source_mp,
     const Int_vector* index_list_vp
);

#ifdef TRACK_MEMORY_ALLOCATION
#    define copy_int_matrix(x,y) debug_copy_int_matrix(x,y, __FILE__, __LINE__)

    int debug_copy_int_matrix
    (
        Int_matrix**      target_mpp,
        const Int_matrix* source_mp,
        const char*       file_name,
        int               line_number
    );
#else
    int copy_int_matrix
    (
        Int_matrix**      target_mpp,
        const Int_matrix* source_mp
    );
#endif

int copy_int_matrix_block
(
    Int_matrix**      target_mpp,
    const Int_matrix* source_mp,
    int               source_row_offset,
    int               source_col_offset,
    int               num_rows,
    int               num_cols
);

int ow_copy_int_matrix_block
(
    Int_matrix*       target_mp,
    int               target_row_offset,
    int               target_col_offset,
    const Int_matrix* source_mp,
    int               source_row_offset,
    int               source_col_offset,
    int               num_rows,
    int               num_cols
);

int ow_copy_int_matrix
(
    Int_matrix*       target_mp,
    int               target_row_offset,
    int               target_col_offset,
    const Int_matrix* source_mp
);

int copy_int_matrix_vector
(
    Int_matrix_vector**      out_int_mvpp,
    const Int_matrix_vector* in_int_mvp
);

int copy_int_matrix_row
(
    Int_matrix*       target_mp,
    int               target_row_num,
    const Int_matrix* source_mp,
    int               source_row_num
);

int copy_int_matrix_col
(
    Int_matrix*       target_mp,
    int               target_col_num,
    const Int_matrix* source_mp,
    int               source_col_num
);

int get_int_matrix_row
(
    Int_vector**      vpp,
    const Int_matrix* mp,
    int               row_num
);

int get_int_matrix_col
(
    Int_vector**      vpp,
    const Int_matrix* mp,
    int               col_num
);

int put_int_matrix_row(Int_matrix* mp, const Int_vector* vp, int row_num);

int put_int_matrix_col(Int_matrix* mp, const Int_vector* vp, int col_num);

int get_int_transpose(Int_matrix** target_mpp, const Int_matrix* mp);

int get_int_identity_matrix(Int_matrix** output_mpp, int size);

int swap_int_matrix_rows
(
    Int_matrix* target_mp,
    int         target_row_num,
    Int_matrix* source_mp,
    int         source_row_num
);

int min_int_matrix_element(const Int_matrix* mp);

int get_min_int_matrix_element
(
    const Int_matrix* mp,
    int*              min_ptr,
    int*              min_i_ptr,
    int*              min_j_ptr
);

int max_int_matrix_element(const Int_matrix* mp);

int get_max_int_matrix_element
(
    const Int_matrix* mp,
    int*              max_ptr,
    int*              max_i_ptr,
    int*              max_j_ptr
);

int sum_int_matrix_elements (const Int_matrix* mp);
int sum_int_matrix_rows     (Int_vector** output_vpp, const Int_matrix* input_mp);
int ow_sum_int_matrix_rows  (Int_vector* output_vp, const Int_matrix* input_mp);

int ow_add_col_int_vector_to_int_matrix (Int_matrix* source_mp, const Int_vector* vp);
int ow_add_row_int_vector_to_int_matrix (Int_matrix* source_mp, const Int_vector* vp);
int ow_add_int_scalar_to_int_matrix     (Int_matrix* source_mp, int scalar);

int subtract_int_matrices(Int_matrix** target_mpp, const Int_matrix* first_mp,
                      const Int_matrix* second_mp);
int ow_subtract_int_matrices(Int_matrix* first_mp, const Int_matrix* second_mp);

int multiply_int_matrices(Int_matrix** target_mpp, const Int_matrix* first_mp,
                      const Int_matrix* second_mp);

int ow_get_abs_of_int_matrix(Int_matrix* mp);

int get_abs_of_int_matrix(Int_matrix** target_mpp, const Int_matrix* source_mp);


int multiply_int_matrix_by_int_scalar(Int_matrix** target_mpp, const Int_matrix* source_mp, int scalar);
int ow_multiply_int_matrix_by_int_scalar(Int_matrix* source_mp, int scalar);

int multiply_int_matrix_and_int_vector(Int_vector** output_vpp, const Int_matrix* input_mp,
                               const Int_vector* input_vp);

int multiply_int_vector_and_int_matrix
(
    Int_vector**        output_vpp,
    const Int_vector*   input_vp,
    const Int_matrix*   intput_mp
);

int ow_add_int_matrices(Int_matrix* first_mp, const Int_matrix* second_mp);

int add_int_matrices
(
    Int_matrix** target_mpp,
    const Int_matrix* first_mp,
    const Int_matrix* second_mp
);


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          Int_matrix_vector_vector
 *
 * Type for an array of arrays of integer matrices
 *
 * This type is used in the KJB library for arrays of arrays of integer matrices.
 *
 * Index: integer matrices
 *
 * Author: Kobus Barnard, Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Int_matrix_vector_vector
{
    int                 length;
    Int_matrix_vector** elements;
}
Int_matrix_vector_vector;


/* -------------------------------------------------------------------------- */


int get_target_int_matrix_vector_vector(Int_matrix_vector_vector** mvvpp, int length);

void free_int_matrix_vector_vector(Int_matrix_vector_vector* mvvp);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        verify_non_negative_int_matrix
 *
 * (MACRO) Debug verification that an integer matrix is non-negative
 *
 * When TEST is defined (typically this is "development" mode),
 * verify_non_negative_matrix checks that all elements of a integer matrix are
 * non-negative. If invalid numbers are found, then they are set to zero, and a
 * warning is printed. 
 *
 * In addition, if the second argument is not NULL, then the integer pointed to
 * it is set to either NO_ERROR or ERROR depending on whether the verification
 * passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_non_negative_matrix becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void verify_non_negative_int_matrix(const Int_matrix* mp, int* failure_ptr);
#else
#    ifdef TEST
#       define verify_non_negative_int_matrix(x,y)        debug_verify_non_negative_int_matrix(x,y,__FILE__,__LINE__)

        void debug_verify_non_negative_int_matrix
        (
            const Int_matrix* mp,
            Bool*          failure_ptr,
            const char*   file_name,
            int           line_number
        );

#    else
#       define verify_non_negative_int_matrix(x,y)
#    endif
#endif

int max_abs_int_matrix_difference
(
    const Int_matrix* first_mp,
    const Int_matrix* second_mp
);

#ifdef TEST
#    define check_same_int_matrix_dimensions(x,y,z)  \
                debug_check_same_int_matrix_dimensions(x,y,__LINE__, __FILE__,z)
#endif

#ifdef TEST
int debug_check_same_int_matrix_dimensions
#else
int check_same_int_matrix_dimensions
#endif
(
    const Int_matrix* first_mp,
    const Int_matrix* second_mp,
#ifdef TEST
    int           line_number,
    const char*   file_name,
#endif
    const char*   context_str
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

