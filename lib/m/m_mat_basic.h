
/* $Id: m_mat_basic.h 21522 2017-07-22 15:14:27Z kobus $ */

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

#ifndef M_MAT_BASIC_INCLUDED
#define M_MAT_BASIC_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
 * Will be static soon.
*/
Matrix* create_matrix_transpose(const Matrix*);
Matrix* create_matrix_copy(const Matrix* source_mp);


int random_split_matrix_by_rows
(
    Matrix**          target_1_mpp,
    Matrix**          target_2_mpp,
    const Matrix*     source_mp,
    double fraction 
);

int split_matrix_by_rows
(
    Matrix**          target_1_mpp,
    Matrix**          target_2_mpp,
    const Matrix*     source_mp,
    const Int_vector* index_list_vp
);

int get_matrix_transpose(Matrix** target_mpp, const Matrix* mp);

#ifdef TRACK_MEMORY_ALLOCATION

#   define copy_matrix(x,y)  debug_copy_matrix(x,y, __FILE__, __LINE__)

    int debug_copy_matrix
    (
        Matrix**      target_mpp,
        const Matrix* source_mp,
        const char*   file_name,
        int           line_number
    );
#else
    int copy_matrix(Matrix** target_mpp, const Matrix* source_mp);
#endif

int select_matrix_cols
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Int_vector* selected_cols_vp
);

int copy_matrix_block
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    int           source_row_offset,
    int           source_col_offset,
    int           num_rows,
    int           num_cols
);

int ow_copy_matrix_block
(
    Matrix*       target_mp,
    int           target_row_offset,
    int           target_col_offset,
    const Matrix* source_mp,
    int           source_row_offset,
    int           source_col_offset,
    int           num_rows,
    int           num_cols
);

int copy_matrix_block_2
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    int           source_row_offset,
    int           source_col_offset,
    int           source_row_step,
    int           source_col_step,
    int           num_rows,
    int           num_cols
);

int ow_copy_matrix_block_2
(
    Matrix*       target_mp,
    int           target_row_offset,
    int           target_col_offset,
    int           target_row_step,
    int           target_col_step,
    const Matrix* source_mp,
    int           source_row_offset,
    int           source_col_offset,
    int           source_row_step,
    int           source_col_step,
    int           num_rows,
    int           num_cols
);

int ow_copy_matrix
(
    Matrix*       target_mp,
    int           target_row_offset,
    int           target_col_offset,
    const Matrix* source_mp
);

int ow_copy_matrix_with_col_selection
(
    Matrix*       target_mp,
    int           target_row_offset,
    int           target_col_offset,
    const Matrix* source_mp,
    const Int_vector* select_cols_vp
);

#ifdef HOW_IT_WAS_JUNE_28_2004

/* This routine looks exactly the same as copy_matrix_row(), so it got purged.
 * Of course, a matrix row copy is overwrite, but do we put it into the naming
 * convection? If we did that, we would want to rename put_matrix_row().
*/
int ow_copy_matrix_row
(
    Matrix*       target_mp,
    int           target_row,
    const Matrix* source_mp,
    int           source_row
);
#endif

int copy_matrix_data
(
    Matrix**      target_mpp,
    const Matrix* mp,
    const int*    invalid_data
);

int copy_int_matrix_to_matrix
(
    Matrix**          target_mpp,
    const Int_matrix* source_mp
);

int copy_matrix_to_int_matrix
(
    Int_matrix**  target_mpp,
    const Matrix* source_mp
);

int split_matrix_vector
(
    Matrix_vector**      out_1_mvpp,
    Matrix_vector**      out_2_mvpp,
    const Matrix_vector* in_mvp,
    const Int_vector*    index_list_vp
);

int copy_matrix_vector(Matrix_vector**, const Matrix_vector*);

int copy_matrix_vector_block
(
    Matrix_vector**      out_mvpp,
    const Matrix_vector* in_mvp,
    int start_index,
    int num_matrices
);

int concat_matrix_vectors
(
    Matrix_vector**      mvpp,
    int                  num_vectors,
    const Matrix_vector* matrix_vectors[]
);

int get_random_matrix_row(Vector** vpp, const Matrix* mp);

int mp_row_get_indexed_vector
(
    Indexed_vector** vpp,
    Matrix*          init_mp,
    int              row
);

int get_matrix_row(Vector** vpp, const Matrix* mp, int row_num);
int ow_get_matrix_row(Vector* vp, const Matrix* mp, int row_num);

int remove_matrix_row(Vector** vpp, Matrix* mp, int row_num);

int put_matrix_row(Matrix* mp, const Vector* vp, int row_num);

int copy_matrix_row
(
    Matrix*       target_mp,
    int           target_row_num,
    const Matrix* source_mp,
    int           source_row_num
);

int get_random_matrix_col(Vector** vpp, const Matrix* mp);

int get_matrix_col(Vector** vpp, const Matrix* mp, int col_num);

int remove_matrix_col(Vector** vpp, Matrix* mp, int col_num);

int copy_matrix_col
(
    Matrix*       target_mp,
    int           target_col_num,
    const Matrix* source_mp,
    int           source_col_num
);

int put_matrix_col(Matrix* mp, const Vector* vp, int col_num);

int insert_zero_row_in_matrix(Matrix* mp, int row_num);    
int insert_zero_col_in_matrix(Matrix* mp, int col_num);    

int swap_matrix_rows(Matrix*, int, int);
int swap_matrix_cols(Matrix*, int, int);

int select_matrix_data
(
    Matrix**      selected_mpp,
    const Matrix* data_mp,
    int           num_data_points,
    const int*    data_point_nums,
    const int*    invalid_data
);

int vector_is_matrix_row(const Matrix* mp, const Vector* vp);


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             verify_matrix_vector
 *
 * (MACRO) Debug verification of a matrix vector
 *
 * When TEST is defined (typically this is "development" mode),
 * verify_matrix_vector checks that all elements of a matrix vector are valid
 * numbers. NULL input is considered valid. If invalid numbers are found, then
 * they are set to zero, and a warning is printed. 
 *
 * In addition, if the second argument is not NULL, then the Boolean variable
 * pointed to it is set to either FALSE or TRUE depending on whether the
 * verification passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_matrix_vector becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void verify_matrix_vector(const Matrix_vector* mvp, Bool* failed_ptr);
#else
#   ifdef TEST
#       define verify_matrix_vector(x,y)              debug_verify_matrix_vector(x,y,__FILE__,__LINE__)

        void debug_verify_matrix_vector
        (
            const Matrix_vector* mvp,
            Bool*                failed_ptr, 
            const char*          file_name,
            int                  line_number
        );

#else
#       define verify_matrix_vector(x,y)
#endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          verify_non_negative_matrix_vector
 *
 * (MACRO) Debug verification of a matrix vector
 *
 * When TEST is defined (typically this is "development" mode),
 * verify_non_negative_matrix_vector checks that all elements of a matrix vector
 * are valid numbers, AND that they are non-negative. NULL input is considered
 * valid. If invalid numbers are found, then they are set to zero, and a warning
 * is printed.
 *
 * In addition, if the second argument is not NULL, then the Boolean variable
 * pointed to it is set to either FALSE or TRUE depending on whether the
 * verification passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_non_negative_matrix_vector becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
     void verify_non_negative_matrix_vector(const Matrix_vector* mvp, Bool* failed_ptr);
#else
#    ifdef TEST
#    define verify_non_negative_matrix_vector(x,y) debug_verify_non_negative_matrix_vector(x,y,__FILE__,__LINE__)

        void debug_verify_non_negative_matrix_vector
        (
            const Matrix_vector* mvp,
            Bool*                failed_ptr, 
            const char*          file_name,
            int                  line_number
        );

#    else
#        define verify_non_negative_matrix_vector(x,y)
#    endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         verify_probability_matrix_vector
 *
 * (MACRO) Debug verification of a matrix vector
 *
 * When TEST is defined (typically this is "development" mode),
 * verify_probability_matrix_vector checks that all elements of a matrix vector
 * are valid numbers, and that they are in the range of [0,1]. NULL input is
 * considered valid. If invalid numbers are found, then they are set to either
 * zero or one (depending on which side of [0,1] they fall), and a warning is
 * printed.
 *
 * In addition, if the second argument is not NULL, then the Boolean variable
 * pointed to it is set to either FALSE or TRUE depending on whether the
 * verification passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_probability_matrix_vector becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
        void verify_probability_matrix_vector(const Matrix_vector* mvp, Bool* failed_ptr);
#else
#    ifdef TEST
#       define verify_probability_matrix_vector(x,y)  debug_verify_probability_matrix_vector(x,y,__FILE__,__LINE__)

        void debug_verify_probability_matrix_vector
        (
            const Matrix_vector* mvp,
            Bool*                failed_ptr, 
            const char*          file_name,
            int                  line_number
        );

#    else
#       define verify_probability_matrix_vector(x,y)
#    endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                   verify_probability_row_matrix_vector
 *
 * (MACRO) Debug verification of a matrix vector
 *
 * When TEST is defined (typically this is "development" mode),
 * verify_probability_row_matrix_vector verifies all matrices in a matrix vector
 * using verify_probability_row_matrix(). NULL input is considered valid.
 *
 * In addition, if the second argument is not NULL, then the Boolean variable
 * pointed to it is set to either FALSE or TRUE depending on whether the
 * verification passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_probability_row_matrix_vector becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
        void verify_probability_row_matrix_vector(const Matrix_vector* mvp, Bool* failed_ptr);
#else
#    ifdef TEST
#       define verify_probability_row_matrix_vector(x,y)  debug_verify_probability_row_matrix_vector(x,y,__FILE__,__LINE__)

        void debug_verify_probability_row_matrix_vector
        (
            const Matrix_vector* mvp,
            Bool*                failed_ptr, 
            const char*          file_name,
            int                  line_number
        );

#    else
#       define verify_probability_row_matrix_vector(x,y)
#    endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             verify_matrix
 *
 * (MACRO) Debug verification of a matrix
 *
 * When TEST is defined (typically this is "development" mode), verify_matrix
 * checks that all elements of a matrix are valid numbers. NULL input is
 * considered valid. If invalid numbers are found, then they are set to zero,
 * and a warning is printed.
 *
 * In addition, if the second argument is not NULL, then the Boolean variable
 * pointed to it is set to either FALSE or TRUE depending on whether the
 * verification passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_matrix becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void verify_matrix(const Matrix* mp, Bool* failed_ptr);
#else
#    ifdef TEST
#       define verify_matrix(x,y)                     debug_verify_matrix(x,y, __FILE__, __LINE__)

        void debug_verify_matrix
        (
            const Matrix* mp,
            Bool*         failed_ptr, 
            const char*   file_name,
            int           line_number
        );

#    else
#        define verify_matrix(x,y)
#    endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          verify_non_negative_matrix
 *
 * (MACRO) Debug verification of a matrix
 *
 * When TEST is defined (typically this is "development" mode),
 * verify_non_negative_matrix checks that all elements of a matrix are valid
 * numbers, AND that they are non-negative. NULL input is considered valid. If
 * invalid numbers are found, then they are set to zero, and a warning is
 * printed.
 *
 * In addition, if the second argument is not NULL, then the Boolean variable
 * pointed to it is set to either FALSE or TRUE depending on whether the
 * verification passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_non_negative_matrix becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void verify_non_negative_matrix(const Matrix* mp, Bool* failed_ptr);
#else
#    ifdef TEST
#       define verify_non_negative_matrix(x,y)        debug_verify_non_negative_matrix(x,y, __FILE__, __LINE__)

        void debug_verify_non_negative_matrix
        (
            const Matrix* mp,
            Bool*         failed_ptr, 
            const char*   file_name,
            int           line_number
        );

#    else
#       define verify_non_negative_matrix(x,y)
#    endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         verify_probability_matrix
 *
 * (MACRO) Debug verification of a matrix
 *
 * When TEST is defined (typically this is "development" mode),
 * verify_probability_matrix checks that all elements of a matrix are valid
 * numbers, and that they are in the range of [0,1]. NULL input is considered
 * valid. If invalid numbers are found, then they are set to either zero or one
 * (depending on which side of [0,1] they fall), and a warning is printed.
 * Finally (new for 2017) we exit with EXIT_BUG. 
 *
 * In addition, if the second argument is not NULL, then the Boolean variable
 * pointed to it is set to either FALSE or TRUE depending on whether the
 * verification passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_probability_matrix becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void verify_probability_matrix(const Matrix* mp, Bool* failed_ptr);
#else
#   ifdef TEST
#       define verify_probability_matrix(x,y)         debug_verify_probability_matrix(x,y, __FILE__, __LINE__)

        void debug_verify_probability_matrix
        (
            const Matrix* mp,
            Bool*         failed_ptr, 
            const char*   file_name,
            int           line_number
        );

#   else
#       define verify_probability_matrix(x,y)
#   endif
#endif


/* =============================================================================
 *                         verify_probability_row_matrix
 *
 * (MACRO) Debug verification of a matrix
 *
 * When TEST is defined (typically this is "development" mode),
 * verify_probability_row_matrix calls verify_probability_matrix(), and does the
 * additional check that all rows sum to 1.0. NULL input is considered valid.
 *
 * In addition, if the second argument is not NULL, then the Boolean variable
 * pointed to it is set to either FALSE or TRUE depending on whether the
 * verification passed or failed.
 *
 * When TEST is not defined (typically "production" mode), then
 * verify_probability_row_matrix becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void verify_probability_row_matrix(const Matrix* mp, Bool* failed_ptr);
#else
#   ifdef TEST
#       define verify_probability_row_matrix(x,y)         debug_verify_probability_row_matrix(x,y, __FILE__, __LINE__)

        void debug_verify_probability_row_matrix
        (
            const Matrix* mp,
            Bool*         failed_ptr, 
            const char*   file_name,
            int           line_number
        );

#   else
#       define verify_probability_row_matrix(x,y)
#   endif
#endif


int stack_matrix_rows(Vector** vpp, const Matrix* mp);
int unstack_matrix_rows(Matrix** mpp, const Vector* vp, int num_rows, int num_cols);


int randomize_matrix_rows(Matrix** target_mpp, const Matrix* source_mp);

int ow_max_thresh_matrix_ew(Matrix* target_mp, const Matrix* max_mp);

int pad_matrix_by_extending(Matrix** target_mp, const Matrix* max_mp, int left, int right, int top, int bottom);

int get_matrix_trace(const Matrix* source_vp, double* trace);

int is_matrix_diagonal(const Matrix* M);

int nonnan_matrix(Int_matrix** nonnans, const Matrix* mp);

/* =============================================================================
 *                             SWAP_MATRICES
 *
 * (MACRO) Swaps the contents of two matrices.
 *
 * This macro swaps the contents of two matrices by swapping their array 
 * pointers and metadata.  This may seem trivial (and it is), but it is a 
 * fast alternative to copying in some circumstances. One example use-case is in
 * functions that implement operations that can't be performed in-place, but 
 * should allow the source and target pointer to be the same.  In this case, 
 * if you detect that the pointers are the same, an elegant solution is:
 *      (1) build the result in a temporary matrix
 *      (2) Swap the temporary matrix with the target matrix
 *      (3) Free the temporary object.
 *
 * The alternative is to deep-copy the temporary object into the target matrix, 
 * but swapping avoids the extra copy operation.
 *
 * Index: matrices
 * -----------------------------------------------------------------------------
 */
#ifdef __C2MAN__
    void SWAP_MATRICES(Matrix* mp_1, Matrix* mp_2);
#else

#define SWAP_MATRICES(mp_1, mp_2) \
{ \
    Matrix tmp = *mp_1; \
    *mp_1 = *mp_2; \
    *mp_2 = tmp; \
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


