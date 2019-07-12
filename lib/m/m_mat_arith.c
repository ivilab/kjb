
/* $Id: m_mat_arith.c 21522 2017-07-22 15:14:27Z kobus $ */

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

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "m/m_mat_arith.h"
#include "m/m_missing.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TEST
#ifdef PROGRAMMER_IS_kobus
#define REPORT_MISSING_VALUE_ARITHMETIC
#endif
#endif

/*
#define SLOW_MATRIX_DIVIDE
*/

/* -------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                              get_vector_outer_product
 *
 * Computes outer product of two vectors.
 *
 * This routine computes the outer product of two vectors. The two vectors must
 * have the same dimensions.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, vector arithmetic, matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_vector_outer_product(Matrix** mpp, const Vector* vp1, const Vector* vp2)
{
    int i, j, len;


    ERE(check_same_vector_lengths(vp1, vp2, "get_vector_outer_product"));
    len = vp1->length;
    ERE(get_target_matrix(mpp, len, len));

    for (i=0; i<len; i++)
    {
        for (j=0; j<len; j++)
        {
            (*mpp)->elements[ i ][ j ]= vp1->elements[ i ] * vp2->elements[ j ];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              add_matrices
 *
 * Performs element-wise addition of two matrices.
 *
 * This routine performs element-wise addition of two matrices. The second
 * matrix must fit into the first an integral number of times. For example, if
 * the first matrix has dimenions 6x3, then the second can have dimensions 6x3
 * (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT 2x2, 5x1, etc. The second
 * matrix is added to each block in the first matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int add_matrices(Matrix** target_mpp, const Matrix* first_mp,
                 const Matrix* second_mp)
{
    ERE(copy_matrix(target_mpp, first_mp));

    if ((first_mp == NULL) || (second_mp == NULL)) 
    {
        if ((first_mp == NULL) && (second_mp == NULL))
        {
            return NO_ERROR;
        }
        else
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (    (first_mp->num_rows <= 0) || (second_mp->num_rows <= 0)
         || (first_mp->num_cols <= 0) || (second_mp->num_cols <= 0)
       )
    {
        if (    (first_mp->num_rows != second_mp->num_rows)
             || (first_mp->num_cols != second_mp->num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
        else
        {
            return NO_ERROR;
        }
    }


    return ow_add_matrices(*target_mpp, second_mp); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_add_matrices
 *
 * Performs element-wise addition of two matrices.
 *
 * This routine performs element-wise addition of two matrices. The second
 * matrix must fit into the first an integral number of times. For example, if
 * the first matrix has dimenions 6x3, then the second can have dimensions 6x3
 * (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT 2x2, 5x1, etc. The second
 * matrix is added to each block in the first matrix. The first matrix is thus
 * overwritten.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_matrices(Matrix* first_mp, const Matrix* second_mp)
{
    int i, j;
    int block_i;
    int block_j;
    int target_i;
    int target_j;
    int num_rows, num_cols;
    int num_second_rows;
    int num_second_cols;
    int row_factor;
    int col_factor;


    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    num_second_rows = second_mp->num_rows;
    num_second_cols = second_mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0) || (num_second_rows <= 0) || (num_second_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_factor = num_rows / num_second_rows;
    col_factor = num_cols / num_second_cols;

    if (    (num_rows != row_factor * num_second_rows)
         || (num_cols != col_factor * num_second_cols)
       )
    {
        set_error("%d by %d can't be added to %d by %d.",
                  num_second_rows, num_second_cols,
                  num_rows, num_cols);
        return ERROR;
    }

    for (block_i=0; block_i<row_factor; block_i++)
    {
        for (block_j=0; block_j<col_factor; block_j++)
        {
            for (i=0; i<num_second_rows; i++)
            {
                for (j=0; j<num_second_cols; j++)
                {
                    target_i = block_i * num_second_rows + i;
                    target_j = block_j * num_second_cols + j;

                    first_mp->elements[ target_i ][ target_j ] +=
                                                 second_mp->elements[ i ][ j ];
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         ow_add_matrices_2
 *
 * Adds a matrix to another at an offiset
 *
 * This addes a matrix to another at the offset given by row_offset and column
 * offset.  
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_matrices_2
(
    Matrix*       first_mp,
    int           row_offset,
    int           col_offset,
    const Matrix* second_mp 
)
{
    const int num_rows = first_mp->num_rows;
    const int num_cols = first_mp->num_cols;
    const int num_second_rows = second_mp->num_rows;
    const int num_second_cols = second_mp->num_cols;
    int i, j;


    /* Was written to be used, but we solved the problem a different way. */
    UNTESTED_CODE(); 

    if (    (row_offset + num_second_rows > num_rows) 
         || (col_offset + num_second_cols > num_cols)
       )
    {
        set_error("%d by %d can't be added to %d by %d at offset %d, %d.",
                  num_second_rows, num_second_cols,
                  num_rows, num_cols,
                  row_offset, col_offset);
        return ERROR;
    }

    for (i=0; i<num_second_rows; i++)
    {
        for (j=0; j<num_second_cols; j++)
        {
            first_mp->elements[ row_offset + i ][ col_offset + j ] += second_mp->elements[ i ][ j ];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              subtract_matrices
 *
 * Performs element-wise subtraction of two matrices.
 *
 * This routine performs element-wise subtraction of two matrices. The second
 * matrix must fit into the first an integral number of times. For example, if
 * the first matrix has dimenions 6x3, then the second can have dimensions 6x3
 * (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT 2x2, 5x1, etc. The second
 * matrix is subtracted from each block in the first matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int subtract_matrices(Matrix** target_mpp, const Matrix* first_mp,
                      const Matrix* second_mp)
{
    ERE(copy_matrix(target_mpp, first_mp));

    if ((first_mp == NULL) || (second_mp == NULL)) 
    {
        if ((first_mp == NULL) && (second_mp == NULL))
        {
            return NO_ERROR;
        }
        else
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (    (first_mp->num_rows <= 0) || (second_mp->num_rows <= 0)
         || (first_mp->num_cols <= 0) || (second_mp->num_cols <= 0)
       )
    {
        if (    (first_mp->num_rows != second_mp->num_rows)
             || (first_mp->num_cols != second_mp->num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
        else
        {
            return NO_ERROR;
        }
    }

    return ow_subtract_matrices(*target_mpp, second_mp); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_subtract_matrices
 *
 * Performs element-wise subtraction of two matrices.
 *
 * This routine performs element-wise subtraction of two matrices. The second
 * matrix must fit into the first an integral number of times. For example, if
 * the first matrix has dimenions 6x3, then the second can have dimensions 6x3
 * (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT 2x2, 5x1, etc. The second
 * matrix is subtracted from each block in the first matrix. The first matrix
 * is thus over-written.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_matrices(Matrix* first_mp, const Matrix* second_mp)
{
    int i, j;
    int block_i;
    int block_j;
    int target_i;
    int target_j;
    int num_rows, num_cols;
    int num_second_rows;
    int num_second_cols;
    int row_factor;
    int col_factor;


    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    num_second_rows = second_mp->num_rows;
    num_second_cols = second_mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0) || (num_second_rows <= 0) || (num_second_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_factor = num_rows / num_second_rows;
    col_factor = num_cols / num_second_cols;

    if (    (num_rows != row_factor * num_second_rows)
         || (num_cols != col_factor * num_second_cols)
       )
    {
        set_error("%d by %d can't be subtracted from %d by %d.",
                  num_second_rows, num_second_cols,
                  num_rows, num_cols);
        return ERROR;
    }

    for (block_i=0; block_i<row_factor; block_i++)
    {
        for (block_j=0; block_j<col_factor; block_j++)
        {
            for (i=0; i<num_second_rows; i++)
            {
                for (j=0; j<num_second_cols; j++)
                {
                    target_i = block_i * num_second_rows + i;
                    target_j = block_j * num_second_cols + j;

                    first_mp->elements[ target_i ][ target_j ] -=
                                                 second_mp->elements[ i ][ j ];
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          multiply_matrices_ew
 *
 * Performs element-wise multiplication of two matrices.
 *
 * This routine performs element-wise multiplication of two matrices. The second
 * matrix must fit into the first an integral number of times. For example, if
 * the first matrix has dimenions 6x3, then the second can have dimensions 6x3
 * (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT 2x2, 5x1, etc. Each block
 * in the first matrix with dimensions of the second is multipled elementwise
 * by the second.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_matrices_ew(Matrix** target_mpp, const Matrix* first_mp,
                         const Matrix* second_mp)
{
    ERE(copy_matrix(target_mpp, first_mp));

    if ((first_mp == NULL) || (second_mp == NULL)) 
    {
        if ((first_mp == NULL) && (second_mp == NULL))
        {
            return NO_ERROR;
        }
        else
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (    (first_mp->num_rows <= 0) || (second_mp->num_rows <= 0)
         || (first_mp->num_cols <= 0) || (second_mp->num_cols <= 0)
       )
    {
        if (    (first_mp->num_rows != second_mp->num_rows)
             || (first_mp->num_cols != second_mp->num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
        else
        {
            return NO_ERROR;
        }
    }


    return ow_multiply_matrices_ew(*target_mpp, second_mp); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      ow_multiply_matrices_ew
 *
 * Performs element-wise multiplication of two matrices.
 *
 * This routine performs element-wise multiplication of two matrices. The second
 * matrix must fit into the first an integral number of times. For example, if
 * the first matrix has dimenions 6x3, then the second can have dimensions 6x3
 * (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT 2x2, 5x1, etc. Each block
 * in the first matrix with dimensions of the second is multipled elementwise
 * by the second. Thus the first matrix is over-written.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_matrices_ew(Matrix* first_mp, const Matrix* second_mp)
{
    int i, j;
    int block_i;
    int block_j;
    int target_i;
    int target_j;
    int num_rows, num_cols;
    int num_second_rows;
    int num_second_cols;
    int row_factor;
    int col_factor;


    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    num_second_rows = second_mp->num_rows;
    num_second_cols = second_mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0) || (num_second_rows <= 0) || (num_second_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_factor = num_rows / num_second_rows;
    col_factor = num_cols / num_second_cols;

    if (    (num_rows != row_factor * num_second_rows)
         || (num_cols != col_factor * num_second_cols)
       )
    {
        set_error("%d by %d can't multiply %d by %d.",
                  num_rows, num_cols, num_second_rows,
                  num_second_cols);
        return ERROR;
    }

    for (block_i=0; block_i<row_factor; block_i++)
    {
        for (block_j=0; block_j<col_factor; block_j++)
        {
            for (i=0; i<num_second_rows; i++)
            {
                for (j=0; j<num_second_cols; j++)
                {
                    target_i = block_i * num_second_rows + i;
                    target_j = block_j * num_second_cols + j;

                    first_mp->elements[ target_i ][ target_j ] *=
                                                 second_mp->elements[ i ][ j ];
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      ow_multiply_matrices_ew_2
 *
 * Performs element-wise multiplication of two matrices.
 *
 * This routine performs element-wise multiplication of two matrices, with the
 * second matrix being shifted to (row_pos,col_pos) of the first. The second
 * matrix, together with any shif, must thus fit inside the first matrix . 
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_matrices_ew_2
(
    Matrix*       first_mp,
    int           row_offset,
    int           col_offset,
    const Matrix* second_mp 
)
{
    int i, j;
    int target_i;
    int target_j;
    int num_rows = first_mp->num_rows;
    int num_cols = first_mp->num_cols;
    int num_second_rows = second_mp->num_rows;
    int num_second_cols = second_mp->num_cols;

    if (    (row_offset + num_second_rows > num_rows)
         || (col_offset + num_second_cols > num_cols)
       )
    {
        set_error("%d by %d can't multiply %d by %d shifted to (%d, %d).",
                  num_rows, num_cols, num_second_rows, num_second_cols,
                  row_offset, col_offset);
        return ERROR;
    }

    for (i=0; i<num_second_rows; i++)
    {
        for (j=0; j<num_second_cols; j++)
        {
            target_i = row_offset + i;
            target_j = col_offset + j;

            first_mp->elements[ target_i ][ target_j ] *= 
                                         second_mp->elements[ i ][ j ];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          divide_matrices_ew
 *
 * Performs element-wise division of two matrices.
 *
 * This routine performs element-wise division of two matrices. The second
 * matrix must fit into the first an integral number of times. For example, if
 * the first matrix has dimenions 6x3, then the second can have dimensions 6x3
 * (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT 2x2, 5x1, etc. Each block
 * of the first matrix is divided by the second matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails. The routine will also fail due to
 *     dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int divide_matrices_ew(Matrix** target_mpp, const Matrix* first_mp,
                       const Matrix* second_mp)
{
    ERE(copy_matrix(target_mpp, first_mp));

    if ((first_mp == NULL) || (second_mp == NULL)) 
    {
        if ((first_mp == NULL) && (second_mp == NULL))
        {
            return NO_ERROR;
        }
        else
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (    (first_mp->num_rows <= 0) || (second_mp->num_rows <= 0)
         || (first_mp->num_cols <= 0) || (second_mp->num_cols <= 0)
       )
    {
        if (    (first_mp->num_rows != second_mp->num_rows)
             || (first_mp->num_cols != second_mp->num_cols)
           )
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
        else
        {
            return NO_ERROR;
        }
    }


    return ow_divide_matrices_ew(*target_mpp, second_mp); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_divide_matrices_ew
 *
 * Performs element-wise division of two matrices.
 *
 * This routine performs element-wise division of two matrices. The second
 * matrix must fit into the first an integral number of times. For example, if
 * the first matrix has dimenions 6x3, then the second can have dimensions 6x3
 * (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT 2x2, 5x1, etc. Each block
 * of the first matrix is divided by the second matrix. The first matrix is
 * thus over-written.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails. The routine will also fail due to
 *     dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_divide_matrices_ew(Matrix* first_mp, const Matrix* second_mp)
{
    int i, j;
    int block_i;
    int block_j;
    int target_i;
    int target_j;
    int num_rows, num_cols;
    int num_second_rows;
    int num_second_cols;
    int row_factor;
    int col_factor;


    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    num_second_rows = second_mp->num_rows;
    num_second_cols = second_mp->num_cols;

    if ((num_rows <= 0) || (num_cols <= 0) || (num_second_rows <= 0) || (num_second_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_factor = num_rows / num_second_rows;
    col_factor = num_cols / num_second_cols;

    if (    (num_rows != row_factor * num_second_rows)
         || (num_cols != col_factor * num_second_cols)
       )
    {
        set_error("%d by %d can't divide %d by %d.",
                  num_second_rows, num_second_cols,
                  num_rows, num_cols);
        return ERROR;
    }

    for (block_i=0; block_i<row_factor; block_i++)
    {
        for (block_j=0; block_j<col_factor; block_j++)
        {
            for (i=0; i<num_second_rows; i++)
            {
                for (j=0; j<num_second_cols; j++)
                {
                    double divisor;


                    target_i = block_i * num_second_rows + i;
                    target_j = block_j * num_second_cols + j;

                    divisor = second_mp->elements[ i ][ j ];

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
                    if (IS_ZERO_DBL(divisor))
                    {
                        SET_DIVIDE_BY_ZERO_ERROR();
                        return ERROR;
                    }
#else
                    /*
                    // Exact test is OK, because case of small divisor is
                    // handled below.
                    */
                    if (divisor == 0.0)
                    {
                        SET_DIVIDE_BY_ZERO_ERROR();
                        return ERROR;
                    }
#endif

                    first_mp->elements[ target_i ][ target_j ] =
                          first_mp->elements[ target_i ][ target_j ] / divisor;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
                    if (    (first_mp->elements[ target_i ][ target_j ] >
                                                      DBL_MOST_POSITIVE)
                         || (first_mp->elements[ target_i ][ target_j ] <
                                                      DBL_MOST_NEGATIVE)
                       )
                    {
                        SET_OVERFLOW_ERROR();
                        return ERROR;
                    }
#endif

                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         ow_add_matrix_times_scalar
 *
 * Performs element-wise addition of a matrix and a matrix times scalar.
 *
 * This routine performs element-wise addition of a matrix and a matrix times a
 * scalar. The second matrix must fit into the first an integral number of
 * times. For example, if the first matrix has dimenions 6x3, then the second
 * can have dimensions 6x3 (the common case), 6x1, 3x3, 2x3, 1x6, etc, but NOT
 * 2x2, 5x1, etc. The second matrix is added to each block in the first matrix.
 * The first matrix is thus overwritten.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_matrix_times_scalar(Matrix* first_mp, const Matrix* second_mp,
                               double scalar)
{
    int i, j;
    int block_i;
    int block_j;
    int target_i;
    int target_j;
    int num_rows, num_cols;
    int num_second_rows;
    int num_second_cols;
    int row_factor;
    int col_factor;


    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    num_second_rows = second_mp->num_rows;
    num_second_cols = second_mp->num_cols;

    row_factor = num_rows / num_second_rows;
    col_factor = num_cols / num_second_cols;

    if (    (num_rows != row_factor * num_second_rows)
         || (num_cols != col_factor * num_second_cols)
       )
    {
        set_error("%d by %d can't be added to %d by %d.",
                  num_second_rows, num_second_cols,
                  num_rows, num_cols);
        return ERROR;
    }

    for (block_i=0; block_i<row_factor; block_i++)
    {
        for (block_j=0; block_j<col_factor; block_j++)
        {
            for (i=0; i<num_second_rows; i++)
            {
                for (j=0; j<num_second_cols; j++)
                {
                    target_i = block_i * num_second_rows + i;
                    target_j = block_j * num_second_cols + j;

                    first_mp->elements[ target_i ][ target_j ] =
                        first_mp->elements[ target_i ][ target_j ] +
                                         scalar * second_mp->elements[ i ][ j ];
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         ow_add_matrix_times_scalar_2
 *
 * Performs element-wise addition of a matrix and a matrix times scalar.
 *
 * This routine performs element-wise addition of a matrix and a matrix times a
 * scalar with offset row_offset and column offset.  
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_matrix_times_scalar_2(Matrix* first_mp, int row_offset, int col_offset, 
                                 const Matrix* second_mp, double scalar)
{
    const int num_rows = first_mp->num_rows;
    const int num_cols = first_mp->num_cols;
    const int num_second_rows = second_mp->num_rows;
    const int num_second_cols = second_mp->num_cols;
    int i, j;


    /* Was written to be used, but we solved the problem a different way. */
    UNTESTED_CODE(); 

    if (    (row_offset + num_second_rows > num_rows) 
         || (col_offset + num_second_cols > num_cols)
       )
    {
        set_error("%d by %d can't be added to %d by %d at offset %d, %d.",
                  num_second_rows, num_second_cols,
                  num_rows, num_cols,
                  row_offset, col_offset);
        return ERROR;
    }

    for (i=0; i<num_second_rows; i++)
    {
        for (j=0; j<num_second_cols; j++)
        {
            first_mp->elements[ row_offset + i ][ col_offset + j ] +=
                                 scalar * second_mp->elements[ i ][ j ];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ow_add_int_matrix_to_matrix
 *
 * Adds an integer matrix to a matrix
 *
 * This routine addes the integer matrix source_mp to the matrix
 * target_mp.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

void ow_add_int_matrix_to_matrix(Matrix* target_mp, const Int_matrix* source_mp)
{
    int     i, j, num_rows, num_cols;
    double* target_pos;
    int* source_pos;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        source_pos = (source_mp->elements)[ i ];
        target_pos = (target_mp->elements)[ i ];

        for (j=0; j<num_cols; j++)
        {
            *target_pos += (double)(*source_pos);
            target_pos++;
            source_pos++;
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          multiply_matrices
 *
 * Multiplies two matrices.
 *
 * This routine multiplies two matrices. The matrices must be compatable for
 * muliplication, or ERROR is returned.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_matrices(Matrix** target_mpp, const Matrix* first_mp,
                      const Matrix* second_mp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     k;
    int     num_rows;
    int     num_cols;
    int     length;
    double*   first_pos;
    double**  second_elements;
    int result = NO_ERROR;


    if ((first_mp == NULL) || (second_mp == NULL))
    {
        UNTESTED_CODE(); 
        free_matrix(*target_mpp);
        *target_mpp = NULL;
    }
    else if (first_mp->num_cols != second_mp->num_rows)
    {
        set_error("Incompatable dimensions for matrix multiplication.");
        add_error("Attempt to multiply a %d by %d with a %d by %d.",
                  first_mp->num_rows, first_mp->num_cols,
                  second_mp->num_rows, second_mp->num_cols);
        result = ERROR;
    }
    /*
     * If it so happens that the user has made the target matrix either of the
     * two factors, then we need to compute the result into a temporary
     * location and copy the result, otherwise we will be changing one of the
     * factors as we compute the answer. 
    */
    else if (    (*target_mpp == first_mp)
              || (*target_mpp == second_mp)
            )
    {
        Matrix* temp_mp = NULL;

        result = multiply_matrices(&temp_mp, first_mp, second_mp);

        if (result != ERROR)
        {
            result = copy_matrix(target_mpp, temp_mp); 
        }

        free_matrix(temp_mp); 
    }
    else 
    {
        ERE(get_target_matrix(target_mpp, first_mp->num_rows,
                              second_mp->num_cols));

        target_mp = *target_mpp;

        second_elements = second_mp->elements;

        num_rows = target_mp->num_rows;
        num_cols = target_mp->num_cols;
        length   = first_mp->num_cols;

        for (i=0; i<num_rows; i++)
        {
            for (j=0; j<num_cols; j++)
            {
                long_double sum = 0.0; /* long_double only has extra precision
                                          where it is supported in hardware. */

                first_pos  = (first_mp->elements)[ i ];

                for (k=0; k<length; k++)
                {
                    sum += ((*first_pos) * (second_elements)[k][j]);
                    first_pos++;
                }

                target_mp->elements[ i ][ j ] = sum;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        multiply_by_transpose
 *
 * Multiplies one matrix by the transpose of the second.
 *
 * This routine multiplies one matrix by the transpose of the second. The
 * matrices must be compatable for muliplication, or ERROR is returned.
 *
 * It is slightly faster to multiply by the transpose, if it is available.
 * Hence, if the operation required is multiplication by the transpose, then it
 * is much better to use this routine, as opposed to getting the transpose, and
 * then multiplying.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_by_transpose(Matrix** target_mpp, const Matrix* first_mp,
                          const Matrix* second_mp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     k;
    int     num_rows;
    int     num_cols;
    int     length;
    int result = NO_ERROR;


    if ((first_mp == NULL) || (second_mp == NULL))
    {
        UNTESTED_CODE(); 
        free_matrix(*target_mpp);
        *target_mpp = NULL;
    }
    else if (first_mp->num_cols != second_mp->num_cols)
    {
        set_error("Incompatable dimensions for matrix multiplication.");
        add_error("Attempt to multiply %d by %d with the transpose of %d by %d.",
                  first_mp->num_rows, first_mp->num_cols,
                  second_mp->num_rows, second_mp->num_cols);
        result = ERROR;
    }
    /*
     * If it so happens that the user has made the target matrix either of the
     * two factors, then we need to compute the result into a temporary
     * location and copy the result, otherwise we will be changing one of the
     * factors as we compute the answer. 
    */
    else if (    (*target_mpp == first_mp)
              || (*target_mpp == second_mp)
            )
    {
        Matrix* temp_mp = NULL;

        result = multiply_by_transpose(&temp_mp, first_mp, second_mp);

        if (result != ERROR)
        {
            result = copy_matrix(target_mpp, temp_mp); 
        }

        free_matrix(temp_mp); 
    }
    else 
    {
        ERE(get_target_matrix(target_mpp, first_mp->num_rows,
                              second_mp->num_rows));

        target_mp = *target_mpp;

        num_rows = target_mp->num_rows;
        num_cols = target_mp->num_cols;
        length   = first_mp->num_cols;

        for (i=0; i<num_rows; i++)
        {
            for (j=0; j<num_cols; j++)
            {
                long_double sum = 0.0; /* long_double only has extra precision
                                          where it is supported in hardware. */
                double* first_pos;
                double* second_pos;


                first_pos  = (first_mp->elements)[ i ];
                second_pos = (second_mp->elements)[ j ];

                for (k=0; k<length; k++)
                {
                    sum += (*first_pos) * (*second_pos);
                    first_pos++;
                    second_pos++;
                }

                target_mp->elements[ i ][ j ] = sum;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       multiply_with_transpose
 *
 * Multiplies the transpose of a matrix by the second.
 *
 * This routine multiplies the transpose of the first matrix argument by the
 * second matrix. The matrices must be compatable for muliplication, or ERROR
 * is returned.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_with_transpose(Matrix** target_mpp, const Matrix* first_mp,
                            const Matrix* second_mp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     k;
    int     num_rows;
    int     num_cols;
    int     length;
    int result = NO_ERROR;


    if ((first_mp == NULL) || (second_mp == NULL))
    {
        UNTESTED_CODE(); 
        free_matrix(*target_mpp);
        *target_mpp = NULL;
    }
    else if (first_mp->num_rows != second_mp->num_rows)
    {
        set_error("Incompatable dimensions for matrix multiplication.");
        add_error("Attempt to multiply the transpose of %d by %d with %d by %d.",
                  first_mp->num_rows, first_mp->num_cols,
                  second_mp->num_rows, second_mp->num_cols);
        result = ERROR;
    }
    /*
     * If it so happens that the user has made the target matrix either of the
     * two factors, then we need to compute the result into a temporary
     * location and copy the result, otherwise we will be changing one of the
     * factors as we compute the answer. 
    */
    else if (    (*target_mpp == first_mp)
              || (*target_mpp == second_mp)
            )
    {
        Matrix* temp_mp = NULL;

        result = multiply_with_transpose(&temp_mp, first_mp, second_mp);

        if (result != ERROR)
        {
            result = copy_matrix(target_mpp, temp_mp); 
        }

        free_matrix(temp_mp); 
    }
    else 
    {
        ERE(get_target_matrix(target_mpp, first_mp->num_cols,
                              second_mp->num_cols));

        target_mp = *target_mpp;

        num_rows = target_mp->num_rows;
        num_cols = target_mp->num_cols;
        length   = first_mp->num_rows;

        for (i=0; i<num_rows; i++)
        {
            for (j=0; j<num_cols; j++)
            {
                long_double sum = 0.0; /* long_double only has extra precision
                                          where it is supported in hardware. */

                for (k=0; k<length; k++)
                {
                    double first_elem;
                    double second_elem;


                    first_elem = first_mp->elements[ k ][ i ];
                    second_elem = second_mp->elements[ k ][ j ];

                    sum += first_elem * second_elem;
                }

                target_mp->elements[ i ][ j ] = sum;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                     multiply_by_own_transpose
 *
 * Multiplies a matrix by its transpose
 *
 * This routine multiplies a matrix by its transpose. Thus it computes  B = AA'
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_by_own_transpose(Matrix** target_mpp, const Matrix* source_mp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     k;
    int     out_dim = source_mp->num_rows;
    int     length = source_mp->num_cols;
    int result = NO_ERROR;


    if (source_mp == NULL)
    {
        UNTESTED_CODE(); 
        free_matrix(*target_mpp);
        *target_mpp = NULL;
    }
    /*
     * If it so happens that the user has made the target matrix the input
     * matrix, then we need to compute the result into a temporary location and
     * copy the result, otherwise we will be changing the input while we compute
     * the answer. 
    */
    else if (*target_mpp == source_mp)
    {
        Matrix* temp_mp = NULL;

        UNTESTED_CODE(); 

        result = multiply_by_own_transpose(&temp_mp, source_mp);

        if (result != ERROR)
        {
            result = copy_matrix(target_mpp, temp_mp); 
        }

        free_matrix(temp_mp); 
    }
    else 
    {
        ERE(get_target_matrix(target_mpp, out_dim, out_dim));
        target_mp = *target_mpp;

        for (i=0; i<out_dim; i++)
        {
            for (j=0; j <= i; j++)
            {
                double *r1_ptr = source_mp->elements[ i ];
                double *r2_ptr = source_mp->elements[ j ];

                long_double sum = 0.0; /* long_double only has extra precision
                                          where it is supported in hardware. */
                for (k=0; k<length; k++)
                {
                    sum += (*r1_ptr)*(*r2_ptr);
                    r1_ptr++;
                    r2_ptr++;
                }

                target_mp->elements[ i ][ j ] = sum;
                target_mp->elements[ j ][ i ] = sum;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        get_dot_product_of_matrix_rows
 *
 * Computes the dot product of two matrix rows.
 *
 * This routine computes the dot product of two matrix rows of a single matrix.
 * The routine get_dot_product_of_matrix_rows_2() can be used if the rows come
 * from different matrices.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_dot_product_of_matrix_rows(const Matrix* mp, int r1, int r2,
                                   double* dot_prod_ptr)
{
#ifdef HOW_IT_WAS
    int num_cols = mp->num_cols;
    int num_rows = mp->num_rows;
    int i;
    double* r1_ptr;
    double* r2_ptr;
    double sum = 0.0;

    if ((r1 < 0) || (r1 >= num_rows) || (r2 < 0) || (r2 >= num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    r1_ptr = mp->elements[ r1 ];
    r2_ptr = mp->elements[ r2 ];

    for (i = 0; i < num_cols; i++)
    {
        sum += (*r1_ptr)*(*r2_ptr);
        r1_ptr++;
        r2_ptr++;
    }

    *dot_prod_ptr = sum;

    return NO_ERROR;
#else
    return get_dot_product_of_matrix_rows_2(mp, r1, mp, r2, dot_prod_ptr);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        get_dot_product_of_matrix_rows_2
 *
 * Computes the dot product of two matrix rows.
 *
 * This routine computes the dot product of two matrix rows. It is similar to
 * get_dot_product_of_matrix_rows(), except that the matrix rows come from
 * (presumably) different matrices.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int get_dot_product_of_matrix_rows_2(const Matrix* first_mp, int r1,
                                     const Matrix* second_mp, int r2,
                                     double* dot_prod_ptr)
{
    int num_cols = first_mp->num_cols;
    int num_cols_2 = second_mp->num_cols;
    int num_rows = first_mp->num_rows;
    int num_rows_2 = second_mp->num_rows;
    int i;
    double* r1_ptr;
    double* r2_ptr;
    double sum = 0.0;



    if (    (r1 < 0) || (r1 >= num_rows) || (r2 < 0) || (r2 >= num_rows_2)
         || (num_cols != num_cols_2)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    r1_ptr = first_mp->elements[ r1 ];
    r2_ptr = second_mp->elements[ r2 ];

    for (i = 0; i < num_cols; i++)
    {
        sum += (*r1_ptr)*(*r2_ptr);
        r1_ptr++;
        r2_ptr++;
    }

    *dot_prod_ptr = sum;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         invert_matrix_elements
 *
 * Computes the element-wise reciprocal of a matrix
 *
 * This routine computes the element-wise reciprocal of a matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Thos routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails.
 *
 * Related:
 *     ow_invert_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int invert_matrix_elements(Matrix** target_mpp, const Matrix* source_mp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

#ifdef CHECK_MATRIX_INITIALZATION_ON_FREE
    /* Prepare for divide by zero. */
    ERE(get_zero_matrix(target_mpp, num_rows, num_cols));
#else
    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
#endif

    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);
        target_row_pos_ptr = (*target_row_ptr);

        for (j=0; j<num_cols; j++)
        {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
            if (IS_ZERO_DBL(*source_row_pos_ptr))
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#else
            /*
            // Exact test is OK, because case of small divisor is
            // handled below.
            */
            if (*source_row_pos_ptr == 0.0)
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#endif

            (*target_row_pos_ptr) = 1.0 / (*source_row_pos_ptr);


#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*target_row_pos_ptr > DBL_MOST_POSITIVE)
                 || (*target_row_pos_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif

            source_row_pos_ptr++;
            target_row_pos_ptr++;
        }
        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         square_matrix_elements
 *
 * Computes the element-wise square of a matrix
 *
 * This routine computes the element-wise square of a matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Thos routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails.
 *
 * Related:
 *     ow_square_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int square_matrix_elements(Matrix** target_mpp, const Matrix* source_mp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;


    UNTESTED_CODE();

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);
        target_row_pos_ptr = (*target_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            (*target_row_pos_ptr) = (*source_row_pos_ptr)
                                                    * (*source_row_pos_ptr);

            source_row_pos_ptr++;
            target_row_pos_ptr++;
        }
        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         exp_matrix_elements
 *
 * Computes exp(x) for matrix elements
 *
 * This routine computes exp(x) for matrix elements.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Thos routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails.
 *
 * Related:
 *     ow_exp_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int exp_matrix_elements(Matrix** target_mpp, const Matrix* source_mp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;


    UNTESTED_CODE();

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);
        target_row_pos_ptr = (*target_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            (*target_row_pos_ptr) = exp(*source_row_pos_ptr);

            source_row_pos_ptr++;
            target_row_pos_ptr++;
        }
        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         log_matrix_elements
 *
 * Computes log(x) for matrix elements
 *
 * This routine computes log(x) for matrix elements. If an element is zero,
 * LOG_ZERO is used instead. If an element is negative, then this routine fails
 * with an appropriate error message being set.  LOG_ZERO is suffiently negative
 * number that exp(LOG_ZERO) is zero.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is. If the matrix element is 0.0, then
 * the value of the argument log_zero is used.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Thos routine will fail if any of the matrix elements are negative,
 *     or if memory allocation fails.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int log_matrix_elements(Matrix** target_mpp, const Matrix* source_mp)
{
    return log_matrix_elements_2(target_mpp, source_mp, LOG_ZERO);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         log_matrix_elements_2
 *
 * Computes log(x) for matrix elements
 *
 * This routine computes log(x) for matrix elements.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is. If the matrix element is 0.0, then
 * the value of the argument log_zero is used.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Thos routine will fail if any of the matrix elements are negative,
 *     or if memory allocation fails.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int log_matrix_elements_2(Matrix** target_mpp, const Matrix* source_mp,
                          double log_zero)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);
        target_row_pos_ptr = (*target_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            double x = *source_row_pos_ptr;

            if (x < 0.0)
            {
                set_error("Attempt to take the log of a negative number.");
                add_error("(Element (%d, %d) of a %d x %d matrix.",
                          i + 1, j + 1, num_rows, num_cols);
                return NO_ERROR;
            }
            else if (x == 0.0)
            {
                (*target_row_pos_ptr) = log_zero;
            }
            else
            {
                (*target_row_pos_ptr) = log(x);
            }

            source_row_pos_ptr++;
            target_row_pos_ptr++;
        }
        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         sqrt_matrix_elements
 *
 * Computes sqrt(x) for matrix elements
 *
 * This routine computes sqrt(x) for matrix elements.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is. If the matrix element is 0.0, then
 * the value of the argument log_zero is used.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Thos routine will fail if any of the matrix elements are negative,
 *     or if memory allocation fails.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int sqrt_matrix_elements(Matrix** target_mpp, const Matrix* source_mp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);
        target_row_pos_ptr = (*target_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            double x = *source_row_pos_ptr;

            if (x < 0.0)
            {
                set_error("Attempt to take the squar root of a negative number.");
                add_error("(Element (%d, %d) of a %d x %d matrix.",
                          i + 1, j + 1, num_rows, num_cols);
                return NO_ERROR;
            }
            else
            {
                (*target_row_pos_ptr) = sqrt(x);
            }

            source_row_pos_ptr++;
            target_row_pos_ptr++;
        }
        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          add_scalar_to_matrix
 *
 * Adds a scalar to each element of a matrix
 *
 * This routine adds a scalar to each element of a matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure.
 *
 * Related:
 *     ow_add_scalar_to_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int add_scalar_to_matrix(Matrix** target_mpp, const Matrix* source_mp,
                         double scalar)
{
    Matrix* target_mp;
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** source_row_ptr;
    double** target_row_ptr;
    double*  source_row_pos_ptr;
    double*  target_row_pos_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return get_initialized_matrix(target_mpp, num_rows, num_cols,
                                          DBL_MISSING);
        }

        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = (*source_row_ptr);
            target_row_pos_ptr = (*target_row_ptr);

            for (j=0; j<num_cols; j++)
            {
                if (IS_MISSING_DBL(*source_row_pos_ptr))
                {
                    *target_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    (*target_row_pos_ptr) = (*source_row_pos_ptr) + scalar;
                }
                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }
            source_row_ptr++;
            target_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = (*source_row_ptr);
            target_row_pos_ptr = (*target_row_ptr);

            for (j=0; j<num_cols; j++)
            {
                (*target_row_pos_ptr) = (*source_row_pos_ptr) + scalar;
                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }
            source_row_ptr++;
            target_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        subtract_scalar_from_matrix
 *
 * Subtracts a scalar to each element of a matrix
 *
 * This routine subtracts a scalar to each element of a matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure.
 *
 * Related:
 *     ow_subtract_scalar_from_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int subtract_scalar_from_matrix(Matrix** target_mpp, const Matrix* source_mp,
                                double scalar)
{
#ifdef REGRESS_TO_05_02_11
    Matrix* target_mp;
    int     i, j, num_rows, num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);
        target_row_pos_ptr = (*target_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            (*target_row_pos_ptr) = (*source_row_pos_ptr) - scalar;
            source_row_pos_ptr++;
            target_row_pos_ptr++;
        }
        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;
#else
    if (respect_missing_values() && (IS_MISSING_DBL(scalar)))
    {
        int num_rows = source_mp->num_rows;
        int num_cols = source_mp->num_cols;

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
        dbm("Missing value arithmetic.");
#endif
        return get_initialized_matrix(target_mpp, num_rows, num_cols,
                                      DBL_MISSING);
    }
    else
    {
        return add_scalar_to_matrix(target_mpp, source_mp, - scalar);
    }

#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         multiply_matrix_by_scalar
 *
 * Multiplies each element of a matrix by a scalar
 *
 * This routine multiplies each element of a matrix by a scalar.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure.
 *
 * Related:
 *     ow_multiply_matrix_by_scalar(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_matrix_by_scalar(Matrix** target_mpp, const Matrix* source_mp,
                              double scalar)
{
    Matrix* target_mp;
    int    i, j, num_rows, num_cols;
    double** source_row_ptr;
    double** target_row_ptr;
    double*  source_row_pos_ptr;
    double*  target_row_pos_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return get_initialized_matrix(target_mpp, num_rows, num_cols,
                                          DBL_MISSING);
        }

        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = (*source_row_ptr);
            target_row_pos_ptr = (*target_row_ptr);

            for (j=0; j<num_cols; j++)
            {
                if (IS_MISSING_DBL(*source_row_pos_ptr))
                {
                    *target_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    (*target_row_pos_ptr) = (*source_row_pos_ptr) * scalar;
                }
                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }
            source_row_ptr++;
            target_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = (*source_row_ptr);
            target_row_pos_ptr = (*target_row_ptr);

            for (j=0; j<num_cols; j++)
            {
                (*target_row_pos_ptr) = (*source_row_pos_ptr) * scalar;
                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }
            source_row_ptr++;
            target_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         divide_matrix_by_scalar
 *
 * Divides each element of a matrix by a scalar
 *
 * This routine divides each element of a matrix by a scalar.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. ERROR will be returned if the scalar is too close to zero.   set.
 *     This routine will fail if the scalar is too close to zero or if memory
 *     allocation failure.
 *
 * Related:
 *     ow_divide_matrix_by_scalar(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int divide_matrix_by_scalar(Matrix** target_mpp, const Matrix* source_mp,
                            double scalar)
{
#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    Matrix* target_mp;
    int     i, j, num_rows, num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
    if (IS_ZERO_DBL(scalar))
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        return ERROR;
    }
#else
    if (scalar == 0.0)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        return ERROR;
    }
#endif

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

#ifdef CHECK_MATRIX_INITIALZATION_ON_FREE
    /* Prepare for overflow. */
    ERE(get_zero_matrix(target_mpp, num_rows, num_cols));
#else
    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
#endif

    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);
        target_row_pos_ptr = (*target_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            *target_row_pos_ptr = (*source_row_pos_ptr) / scalar;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*target_row_pos_ptr > DBL_MOST_POSITIVE)
                 || (*target_row_pos_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif

            source_row_pos_ptr++;
            target_row_pos_ptr++;
        }
        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;

#else

    if (ABS_OF(scalar) < 10.0 * DBL_MIN)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        return ERROR;
    }
    else if (respect_missing_values() && (IS_MISSING_DBL(scalar)))
    {
        int num_rows = source_mp->num_rows;
        int num_cols = source_mp->num_cols;

#ifdef REPORT_MISSING_VALUE_ARITHMETIC
        dbm("Missing value arithmetic.");
#endif
        return get_initialized_matrix(target_mpp, num_rows, num_cols,
                                      DBL_MISSING);
    }
    else
    {
        return multiply_matrix_by_scalar(target_mpp, source_mp, 1.0 / scalar);
    }

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_invert_matrix_elements
 *
 * Computes the element-wise reciprocal of a matrix
 *
 * This routine replaces a matrix by its element-wise reciprocal.
 *
 * Note:
 *     This routine overwrites its input. See invert_matrix_elements(3) for an
 *     alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails if any of the matrix elements are too close to
 *     zero.
 *
 * Related:
 *     invert_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_invert_matrix_elements(Matrix* source_mp)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);

        for (j=0; j<num_cols; j++)
        {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
            if (IS_ZERO_DBL(*source_row_pos_ptr))
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#else
            /*
            // Exact compare is OK  because overflow is handled separately.
            */
            if (*source_row_pos_ptr == 0.0)
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#endif

            (*source_row_pos_ptr) = 1.0 / (*source_row_pos_ptr);

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*source_row_pos_ptr > DBL_MOST_POSITIVE)
                 || (*source_row_pos_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif

            source_row_pos_ptr++;
        }
        source_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_square_matrix_elements
 *
 * Computes the element-wise square of a matrix
 *
 * This routine replaces a matrix by its element-wise square.
 *
 * Note:
 *     This routine overwrites its input. See square_matrix_elements(3) for an
 *     alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails if any of the matrix elements are too close to
 *     zero.
 *
 * Related:
 *     square_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_square_matrix_elements(Matrix* source_mp)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            (*source_row_pos_ptr) = (*source_row_pos_ptr)*(*source_row_pos_ptr);

            source_row_pos_ptr++;
        }
        source_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_sqrt_matrix_elements(Matrix* source_mp)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;


    UNTESTED_CODE();

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            if (*source_row_pos_ptr < 0.0)
            {
                set_error("Attempt to take the square root of negative number.");
                add_error("(Element (%d, %d) of a %d x %d matrix.",
                          i + 1, j + 1, num_rows, num_cols);
                return ERROR;
            }
            else
            {
                *source_row_pos_ptr = sqrt(*source_row_pos_ptr);
            }

            source_row_pos_ptr++;
        }
        source_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_exp_matrix_elements
 *
 * Computes exp(x) of matrix elements
 *
 * This routine computes exp(x) of matrix elements
 *
 * Note:
 *     This routine overwrites its input. See exp_matrix_elements(3) for an
 *     alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails if any of the matrix elements are too close to
 *     zero.
 *
 * Related:
 *     exp_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_exp_matrix_elements(Matrix* source_mp)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            (*source_row_pos_ptr) = exp(*source_row_pos_ptr);

            source_row_pos_ptr++;
        }
        source_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_log_matrix_elements
 *
 * Computes log(x) of matrix elements
 *
 * This routine computes log(x) of matrix elements. If the element is zero, then
 * value LOG_ZERO is used instead. If an element is negative, this routine
 * fails.  LOG_ZERO is suffiently negative number that exp(LOG_ZERO) is zero.
 *
 * Note:
 *     This routine overwrites its input. See log_matrix_elements(3) for an
 *     alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails if any of the matrix elements are negative.
 *
 * Related:
 *     log_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_log_matrix_elements(Matrix* source_mp)
{

    return ow_log_matrix_elements_2(source_mp, LOG_ZERO);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_log_matrix_elements_2
 *
 * Computes log(x) of matrix elements
 *
 * This routine computes log(x) of matrix elements. If the element is zero, then
 * the argument log_zero is used instead. If the element is negative, routine
 * fails.
 *
 * Note:
 *     This routine overwrites its input. See log_matrix_elements(3) for an
 *     alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails if any of the matrix elements are negative.
 *
 * Related:
 *     log_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_log_matrix_elements_2(Matrix* source_mp, double log_zero)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            double x = *source_row_pos_ptr;

            if (x < 0.0)
            {
                set_error("Attempt to take the log of negative number.");
                add_error("(Element (%d, %d) of a %d x %d matrix.",
                          i + 1, j + 1, num_rows, num_cols);
                return ERROR;
            }
            else if (x == 0.0)
            {
                (*source_row_pos_ptr) = log_zero;
            }
            else
            {
                (*source_row_pos_ptr) = log(x);
            }

            source_row_pos_ptr++;
        }
        source_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         ow_add_scalar_to_matrix
 *
 * Adds a scalar to each element of a matrix
 *
 * This routine adds a scalar to each element of a matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Note:
 *     This routine overwrites its input. See add_scalar_to_matrix(3) for an
 *     alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine cannot fail (gracefully!).
 *
 * Related:
 *     add_scalar_to_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_scalar_to_matrix(Matrix* source_mp, double scalar)
{
    int    i, j, num_rows, num_cols;
    double** source_row_ptr;
    double*  source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return ow_set_matrix(source_mp, DBL_MISSING);
        }

        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = (*source_row_ptr);

            for (j=0; j<num_cols; j++)
            {
                if (IS_MISSING_DBL(*source_row_pos_ptr))
                {
                    *source_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    (*source_row_pos_ptr) = (*source_row_pos_ptr) + scalar;
                }
                source_row_pos_ptr++;
            }
            source_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = (*source_row_ptr);

            for (j=0; j<num_cols; j++)
            {
                (*source_row_pos_ptr) = (*source_row_pos_ptr) + scalar;
                source_row_pos_ptr++;
            }
            source_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_subtract_scalar_from_matrix
 *
 * Subtracts a scalar to each element of a matrix
 *
 * This routine subtracts a scalar to each element of a matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Note:
 *     This routine overwrites its input. See subtract_scalar_from_matrix(3) for
 *     an alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine cannot fail (gracefully!).
 *
 * Related:
 *     subtract_scalar_from_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_scalar_from_matrix(Matrix* source_mp, double scalar)
{
#ifdef REGRESS_TO_05_02_11
    int i, j, num_rows, num_cols;
    double** source_row_ptr;
    double*  source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = (*source_row_ptr);

        for (j=0; j<num_cols; j++)
        {
            (*source_row_pos_ptr) = (*source_row_pos_ptr) - scalar;
            source_row_pos_ptr++;
        }
        source_row_ptr++;
    }

    return NO_ERROR;
#else
    if (respect_missing_values() && (IS_MISSING_DBL(scalar)))
    {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
        dbm("Missing value arithmetic.");
#endif
        return ow_set_matrix(source_mp, DBL_MISSING);
    }
    else
    {
        return ow_add_scalar_to_matrix(source_mp,  - scalar);
    }
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_multiply_matrix_by_scalar
 *
 * Multiplies each element of a matrix by a scalar
 *
 * This routine multiplies each element of a matrix by a scalar.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Note:
 *     This routine overwrites its input. See multiply_matrix_by_scalar(3) for
 *     an alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine cannot fail (gracefully!).
 *
 * Related:
 *     multiply_matrix_by_scalar(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_matrix_by_scalar(Matrix* source_mp, double scalar)
{
    int    i, j, num_rows, num_cols;
    double** source_row_ptr;
    double*  source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        if (IS_MISSING_DBL(scalar))
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            return ow_set_matrix(source_mp, DBL_MISSING);
        }

        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = (*source_row_ptr);

            for (j=0; j<num_cols; j++)
            {
                if (IS_MISSING_DBL(*source_row_pos_ptr))
                {
                    *source_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    (*source_row_pos_ptr) = (*source_row_pos_ptr) * scalar;
                }
                source_row_pos_ptr++;
            }
            source_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = (*source_row_ptr);

            for (j=0; j<num_cols; j++)
            {
                (*source_row_pos_ptr) = (*source_row_pos_ptr) * scalar;
                source_row_pos_ptr++;
            }
            source_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_divide_matrix_by_scalar
 *
 * Divides each element of a matrix by a scalar
 *
 * This routine divides each element of a matrix by a scalar.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Note:
 *     This routine overwrites its input. See divide_matrix_by_scalar(3) for an
 *     alternate routine which does not.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine cannot fail (gracefully!).
 *
 * Related:
 *     divide_matrix_by_scalar(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_divide_matrix_by_scalar(Matrix* source_mp, double scalar)
{

    if (ABS_OF(scalar) < 10.0 * DBL_MIN)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        return ERROR;
    }
    else if (respect_missing_values() && (IS_MISSING_DBL(scalar)))
    {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
        dbm("Missing value arithmetic.");
#endif
        return ow_set_matrix(source_mp, DBL_MISSING);
    }
    else
    {
        return ow_multiply_matrix_by_scalar(source_mp, 1.0 / scalar);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       multiply_vector_and_matrix
 *
 * Multiplies a vector by a matrix
 *
 * This routine multiplies a vector by a matrix. The vector is used as a row
 * vector. The dimensions must be appropriate for muliplication, or ERROR is
 * returned.
 *
 * The first argument is a pointer to the target vector. If the target vector
 * itself is null, then a vector of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_vector_and_matrix(Vector** output_vpp, const Vector* input_vp,
                               const Matrix* input_mp)
{
    int    i, j;
    double sum;
    int    result = NO_ERROR;


    if ((input_vp == NULL) || (input_mp == NULL))
    {
        UNTESTED_CODE(); 
        free_vector(*output_vpp);
        *output_vpp = NULL;
    }
    else if (input_vp->length != input_mp->num_rows)
    {
        SET_ARGUMENT_BUG();
        result = ERROR;
    }
    /*
     * If it so happens that the user has made the target vector the input
     * vector, then we need to compute the result into a temporary location and
     * copy the result, otherwise we will be changing the input as we
     * compute the answer. 
    */
    else if (*output_vpp == input_vp)
    {
        Vector* temp_vp = NULL;

        result = multiply_vector_and_matrix(&temp_vp, input_vp, input_mp);

        if (result != ERROR)
        {
            result = copy_vector(output_vpp, temp_vp); 
        }

        free_vector(temp_vp); 
    }
    else 
    {
        const Vector* output_vp;

        ERE(get_target_vector(output_vpp, input_mp->num_cols));
        output_vp = *output_vpp;

        for (j=0; j<output_vp->length; j++)
        {
            sum = 0.0;

            for (i=0; i<input_mp->num_rows; i++)
            {
                sum += ((input_mp->elements)[i][j]) *
                                          ((input_vp->elements)[i]);
            }

            (output_vp->elements)[j] = sum;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        multiply_matrix_and_vector
 *
 * Multiplies a matrix by a vector
 *
 * This routine multiplies a matrix by a vector. The vector is used as a column
 * vector. The dimensions must be appropriate for muliplication, or ERROR is
 * returned.
 *
 * The first argument is a pointer to the target vector. If the target vector
 * itself is null, then a vector of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: vectors, matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_matrix_and_vector(Vector** output_vpp, const Matrix* input_mp,
                               const Vector* input_vp)
{
    int     i, j;
    double  sum;
    double* vec_pos;
    double* mat_row_pos;
    int     result      = NO_ERROR;


    if ((input_vp == NULL) || (input_mp == NULL))
    {
        UNTESTED_CODE(); 
        free_vector(*output_vpp);
        *output_vpp = NULL;
    }
    else if  (input_vp->length != input_mp->num_cols)
    {
        SET_ARGUMENT_BUG();
        result = ERROR;
    }
    /*
     * If it so happens that the user has made the target vector the input
     * vector, then we need to compute the result into a temporary location and
     * copy the result, otherwise we will be changing the input as we
     * compute the answer. 
    */
    else if (*output_vpp == input_vp)
    {
        Vector* temp_vp = NULL;

        result = multiply_matrix_and_vector(&temp_vp, input_mp, input_vp);

        if (result != ERROR)
        {
            result = copy_vector(output_vpp, temp_vp); 
        }

        free_vector(temp_vp); 
    }
    else 
    {
        const Vector* output_vp;
        
        ERE(get_target_vector(output_vpp, input_mp->num_rows));
        output_vp = *output_vpp;

        for (i = output_vp->length; --i>=0; )
        {
            sum = 0.0;
            vec_pos = input_vp->elements;
            mat_row_pos = input_mp->elements[i];

            for (j = input_mp->num_cols; --j>=0; )
            {
                sum += (*mat_row_pos) * (*vec_pos);
                mat_row_pos++;
                vec_pos++;
            }

            (output_vp->elements)[i] = sum;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         multiply_matrix_rows
 *
 * Multiplies each row of a matrix by each row of another
 *
 * This routine multiplies each row of a matrix by each row of another. The
 * number of columns in the two matrices must be the same. The resulting matrix
 * is a stack of elementwise mulitiplied rows, starting with the first row of
 * the first matrix multiplied by each row in the second matrix, and so on.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Warning:
 *     This routine was created to simplify multipliying spectra, and is used in
 *     several other ways as well. However, it seems not to be a natural
 *     interface. Thus either the routine or the name may change in the future.
 *     It should be considered somewhat OBSOLETE. 
 *
 * Note: 
 *     The target matrix (first argument) must be different from the other two
 *     matrices.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure. The routine will also fail due to dimension mismatch,
 *     but this is currently treated as a bug (see set_bug()).
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_matrix_rows(Matrix** target_mpp, const Matrix* first_mp,
                         const Matrix* second_mp)
{
    Matrix* target_mp;
    int     i, j, k;
    int     first_num_rows;
    int     second_num_rows;
    int     num_rows, num_cols;
    double**  first_row_ptr;
    double*   first_row_pos_ptr;
    double**  second_row_ptr;
    double*   second_row_pos_ptr;
    double**  target_row_ptr;
    double*   target_row_pos_ptr;


    if (first_mp->num_cols != second_mp->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    first_num_rows = first_mp->num_rows;
    second_num_rows = second_mp->num_rows;

    num_rows = first_num_rows * second_num_rows;
    num_cols = first_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    first_row_ptr = first_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<first_num_rows; i++)
    {
        second_row_ptr = second_mp->elements;

        for (j=0; j<second_num_rows; j++)
        {
            first_row_pos_ptr = *first_row_ptr;
            second_row_pos_ptr = *second_row_ptr;

            target_row_pos_ptr = *target_row_ptr;

            for (k=0; k<num_cols; k++)
            {
                *target_row_pos_ptr = (*first_row_pos_ptr) *
                                                         (*second_row_pos_ptr);
                first_row_pos_ptr++;
                second_row_pos_ptr++;
                target_row_pos_ptr++;
            }

           target_row_ptr++;
           second_row_ptr++;
        }

        first_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        add_row_vector_to_matrix
 *
 * Adds a vector to each row of a matrix
 *
 * This routine adds a vector to each row of a matrix. The length of the
 * vector must match the number of columns in the matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure. The routine will also fail due to dimension mismatch,
 *     but this is currently treated as a bug (see set_bug()).
 *
 * Related:
 *     ow_add_row_vector_to_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int add_row_vector_to_matrix(Matrix** target_mpp, const Matrix* source_mp,
                             const Vector* vp)
{
    Matrix* target_mp;
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** source_row_ptr;
    double** target_row_ptr;
    double*  source_row_pos_ptr;
    double*  target_row_pos_ptr;
    double*  vp_pos;


    if (source_mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *target_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *target_row_pos_ptr = (*source_row_pos_ptr) + (*vp_pos);
                }

                source_row_pos_ptr++;
                target_row_pos_ptr++;
                vp_pos++;
            }

            source_row_ptr++;
            target_row_ptr++;
        }

    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                *target_row_pos_ptr = (*source_row_pos_ptr) + (*vp_pos);
                source_row_pos_ptr++;
                target_row_pos_ptr++;
                vp_pos++;
            }
            source_row_ptr++;
            target_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      subtract_row_vector_from_matrix
 *
 * Subtracts a vector from each row of a matrix
 *
 * This routine subtracts a vector to each row of a matrix. The length of the
 * vector must match the number of columns in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure. The routine will also fail due to dimension mismatch,
 *     but this is currently treated as a bug (see set_bug()).
 *
 * Related:
 *     ow_subtract_row_vector_from_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int subtract_row_vector_from_matrix(Matrix** target_mpp,
                                    const Matrix* source_mp,
                                    const Vector* vp)
{
    Matrix* target_mp;
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** source_row_ptr;
    double** target_row_ptr;
    double*  source_row_pos_ptr;
    double*  target_row_pos_ptr;
    double*  vp_pos;


    if (source_mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *target_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *target_row_pos_ptr = (*source_row_pos_ptr) - (*vp_pos);
                }

                source_row_pos_ptr++;
                target_row_pos_ptr++;
                vp_pos++;
            }

            source_row_ptr++;
            target_row_ptr++;
        }

    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                *target_row_pos_ptr = (*source_row_pos_ptr) - (*vp_pos);
                source_row_pos_ptr++;
                target_row_pos_ptr++;
                vp_pos++;
            }
            source_row_ptr++;
            target_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           multiply_matrix_by_row_vector_ew
 *
 * Multiplies each row of a matrix by a vector
 *
 * This routine multiplies each row of a matrix by a vector. The length of the
 * vector must match the number of columns in the matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure. The routine will also fail due to dimension mismatch,
 *     but this is currently treated as a bug (see set_bug()).
 *
 * Related:
 *     ow_multiply_matrix_by_row_vector_ew(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_matrix_by_row_vector_ew(Matrix** target_mpp,
                                     const Matrix* source_mp,
                                     const Vector* vp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;
    double*   vp_pos;


    if (source_mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *target_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *target_row_pos_ptr = (*source_row_pos_ptr) * (*vp_pos);
                }

                source_row_pos_ptr++;
                target_row_pos_ptr++;
                vp_pos++;
            }

            source_row_ptr++;
            target_row_ptr++;
        }

    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                *target_row_pos_ptr = (*source_row_pos_ptr) * (*vp_pos);
                source_row_pos_ptr++;
                target_row_pos_ptr++;
                vp_pos++;
            }

            source_row_ptr++;
            target_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         divide_matrix_by_row_vector
 *
 * Divides each row of a matrix by a vector
 *
 * This routine divides each row of a matrix by a vector. The length of the
 * vector must match the number of columns in the matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero.  The routine will also fail due to dimension mismatch, but this
 *     is currently treated as a bug (see set_bug()).
 *
 * Related:
 *     ow_divide_matrix_by_row_vector(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int divide_matrix_by_row_vector(Matrix** target_mpp,
                                const Matrix* source_mp,
                                const Vector* vp)
{
#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;
    double*   vp_pos;
#else
    Vector* inv_vp = NULL;
    int result;
#endif


#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    if (source_mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef CHECK_MATRIX_INITIALZATION_ON_FREE
    /* Prepare for divide by zero. */
    ERE(get_zero_matrix(target_mpp, source_mp->num_rows, source_mp->num_cols));
#else
    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
#endif

    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = *source_row_ptr;
        target_row_pos_ptr = *target_row_ptr;
        vp_pos = vp->elements;

        for (j=0; j<num_cols; j++)
        {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
            if (IS_ZERO_DBL(*vp_pos))
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#else
            /*
            // Exact compare is OK because overflow is handled below.
            */
            if (*vp_pos == 0.0)
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#endif

            *target_row_pos_ptr = (*source_row_pos_ptr) / (*vp_pos);

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*target_row_pos_ptr > DBL_MOST_POSITIVE)
                 || (*target_row_pos_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif

            source_row_pos_ptr++;
            target_row_pos_ptr++;
            vp_pos++;
        }

        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;

#else
    result = invert_vector(&inv_vp, vp);

    if (result != ERROR)
    {
        result = multiply_matrix_by_row_vector_ew(target_mpp, source_mp, inv_vp);
    }

    free_vector(inv_vp);

    return result;

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          ow_add_row_vector_to_matrix
 *
 * Adds a vector to each row of a matrix.
 *
 * This routine adds a vector to each row of the input matrix. The length of the
 * vector must match the number of columns in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can't fail gracefully.  The routine will fail
 *     due to dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Related:
 *     add_row_vector_to_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_row_vector_to_matrix(Matrix* source_mp, const Vector* vp)
{
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** source_row_ptr;
    double*  source_row_pos_ptr;
    double*  vp_pos;


    if (source_mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *source_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *source_row_pos_ptr += (*vp_pos);
                }
                source_row_pos_ptr++;
                vp_pos++;
            }
            source_row_ptr++;
        }

    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                *source_row_pos_ptr += (*vp_pos);
                source_row_pos_ptr++;
                vp_pos++;
            }
            source_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         ow_subtract_row_vector_from_matrix
 *
 * Subtracts a vector from each row of a matrix.
 *
 * This routine subtracts a vector to each row of the input matrix. The length
 * of the vector must match the number of columns in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can't fail gracefully.  The routine will fail
 *     due to dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Related:
 *     subtract_vector_to_matrix_rows(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_row_vector_from_matrix(Matrix* source_mp, const Vector* vp)
{
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** source_row_ptr;
    double*  source_row_pos_ptr;
    double*  vp_pos;


    if (source_mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *source_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *source_row_pos_ptr -= (*vp_pos);
                }
                source_row_pos_ptr++;
                vp_pos++;
            }
            source_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                *source_row_pos_ptr -= (*vp_pos);
                source_row_pos_ptr++;
                vp_pos++;
            }
            source_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_multiply_matrix_by_row_vector_ew
 *
 * Multiplies each row of a matrix by a vector
 *
 * This routine multiplies each row of the input matrix by a vector. The length
 * of the vector must match the number of columns in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Note:
 *     We use the "_ew" suffix (element wise) to be clear that this is not
 *     matrix multiplication. Analogous routines for add/subtract/divide do not
 *     have this suffix because it is not ambiguous in that case.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can't fail gracefully.  The routine will fail
 *     due to dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Related:
 *     multiply_matrix_by_row_vector_ew(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_matrix_by_row_vector_ew(Matrix* source_mp, const Vector* vp)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;
    double*   vp_pos;


    if (source_mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *source_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *source_row_pos_ptr *= (*vp_pos);
                }
                source_row_pos_ptr++;
                vp_pos++;
            }

            source_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            vp_pos = vp->elements;

            for (j=0; j<num_cols; j++)
            {
                *source_row_pos_ptr *= (*vp_pos);
                source_row_pos_ptr++;
                vp_pos++;
            }

            source_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_divide_matrix_by_row_vector
 *
 * Divides each row of a matrix by a vector
 *
 * This routine divides each row of the input matrix by a vector. The length
 * of the vector must match the number of columns in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails If any of the matrix elements are too close to
 *     zero.
 *
 * Related:
 *     divide_matrix_by_row_vector(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_divide_matrix_by_row_vector(Matrix* mp, const Vector* vp)
{
#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  row_ptr;
    double*   row_pos_ptr;
    double*   vp_pos;
#else
    Vector* inv_vp = NULL;
    int result;
#endif


#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    if (respect_missing_values())
    {
        set_bug("The regression version does not support respecting missing values.");
        return ERROR;
    }

    if (mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_ptr = mp->elements;

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        row_pos_ptr = *row_ptr;
        vp_pos = vp->elements;

        for (j=0; j<num_cols; j++)
        {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
            if (IS_ZERO_DBL(*vp_pos))
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#else
            /*
            // Exact compare is OK because overflow is handled below.
            */
            if (*vp_pos == 0.0)
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#endif

            *row_pos_ptr /= *vp_pos;

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*row_pos_ptr > DBL_MOST_POSITIVE)
                 || (*row_pos_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif

            row_pos_ptr++;
            vp_pos++;
        }

        row_ptr++;
    }

    return NO_ERROR;

#else

    result = invert_vector(&inv_vp, vp);

    if (result != ERROR)
    {
        result = ow_multiply_matrix_by_row_vector_ew(mp, inv_vp);
    }

    free_vector(inv_vp);

    return result;

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_add_vector_to_matrix_row
 *
 * Adds a vector to a specified row of a matrix
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * This routine adds a vector to a specified row of the input matrix. The length
 * of the vector must match the number of columns in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_vector_to_matrix_row(Matrix* mp, const Vector* vp, int row)
{
    int     j;
    int     num_cols;
    double*   row_pos;
    double*   vp_pos;


    if (    (mp->num_cols != vp->length)
         || (row < 0)
         || (row >= mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        UNTESTED_CODE();

        for (j=0; j<num_cols; j++)
        {
            if ((IS_MISSING_DBL(*row_pos)) || (IS_MISSING_DBL(*vp_pos)))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *row_pos = DBL_MISSING;
            }
            else
            {
                *row_pos += *vp_pos;
            }

            row_pos++;
            vp_pos++;
        }
    }
    else
    {
        for (j=0; j<num_cols; j++)
        {
            *row_pos += *vp_pos;

            row_pos++;
            vp_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_add_vector_to_matrix_col
 *
 * Adds a vector to a specified col of a matrix
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * This routine adds a vector to a specified col of the input matrix. The length
 * of the vector must match the number of rows in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_vector_to_matrix_col(Matrix* mp, const Vector* vp, int col)
{
    int     i;
    int     num_rows;
    double* vp_pos;


    if (    (mp->num_rows != vp->length)
         || (col < 0)
         || (col >= mp->num_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = mp->num_rows;
    vp_pos = vp->elements;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        UNTESTED_CODE();

        for (i=0; i<num_rows; i++)
        {
            if ((IS_MISSING_DBL(mp->elements[ i ][ col ])) || (IS_MISSING_DBL(*vp_pos)))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                mp->elements[ i ][ col ] = DBL_MISSING;
            }
            else
            {
                mp->elements[ i ][ col ] += *vp_pos;
            }

            vp_pos++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            mp->elements[ i ][ col ] += *vp_pos;

            vp_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                     ow_add_scalar_times_vector_to_matrix_row
 *
 * Adds a scalar times a vector to a specified row of a matrix
 *
 * This routine adds a vector to a specified row of the input matrix. The length
 * of the vector must match the number of columns in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_scalar_times_vector_to_matrix_row(Matrix* mp, const Vector* vp,
                                             double scalar, int row)
{
    int     j;
    int     num_cols;
    double*   row_pos;
    double*   vp_pos;


    if (    (mp->num_cols != vp->length)
         || (row < 0)
         || (row >= mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;
    vp_pos = vp->elements;

    for (j=0; j<num_cols; j++)
    {
        *row_pos += (scalar * (*vp_pos));

        row_pos++;
        vp_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_subtract_vector_from_matrix_row
 *
 * Subtracs a vector from the specified row of a matrix
 *
 * This routine adds a vector to a specified row of the input matrix. The length
 * of the vector must match the number of columns in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_vector_from_matrix_row(Matrix* mp, const Vector* vp, int row)
{
    int     j;
    int     num_cols;
    double*   row_pos;
    double*   vp_pos;


    if (    (mp->num_cols != vp->length)
         || (row < 0)
         || (row >= mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        UNTESTED_CODE();

        for (j=0; j<num_cols; j++)
        {
            if ((IS_MISSING_DBL(*row_pos)) || (IS_MISSING_DBL(*vp_pos)))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *row_pos = DBL_MISSING;
            }
            else
            {
                *row_pos -= *vp_pos;
            }

            row_pos++;
            vp_pos++;
        }
    }
    else
    {
        for (j=0; j<num_cols; j++)
        {
            *row_pos -= *vp_pos;

            row_pos++;
            vp_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_multiply_matrix_row_by_vector
 *
 * Multiplies a specified row of a matrix by a vector
 *
 * This routine multiplies a specified row of a matrix by a vector. The length
 * of the vector must match the number of columns in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_matrix_row_by_vector(Matrix* mp, const Vector* vp, int row)
{
    int     j;
    int     num_cols;
    double*   row_pos;
    double*   vp_pos;


    if (    (mp->num_cols != vp->length)
         || (row < 0)
         || (row >= mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        UNTESTED_CODE();

        for (j=0; j<num_cols; j++)
        {
            if ((IS_MISSING_DBL(*row_pos)) || (IS_MISSING_DBL(*vp_pos)))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *row_pos = DBL_MISSING;
            }
            else
            {
                *row_pos *= *vp_pos;
            }

            row_pos++;
            vp_pos++;
        }
    }
    else
    {
        for (j=0; j<num_cols; j++)
        {
            *row_pos *= *vp_pos;

            row_pos++;
            vp_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_divide_matrix_row_by_vector
 *
 * Divides a specified row of a matrix by a vector
 *
 * This routine divides a specified row of the input matrix by a vector. The
 * length of the vector must match the number of columns in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails If any of the matrix elements are too close to
 *     zero.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_divide_matrix_row_by_vector(Matrix* mp, const Vector* vp, int row)
{
#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    int     j;
    int     num_cols;
    double*   row_pos;
    double*   vp_pos;
#else
    Vector* inv_vp = NULL;
    int result;
#endif

#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    if (    (mp->num_cols != vp->length)
         || (row < 0)
         || (row >= mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;
    vp_pos = vp->elements;

    for (j=0; j<num_cols; j++)
    {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
        if (IS_ZERO_DBL(*vp_pos))
        {
            SET_DIVIDE_BY_ZERO_ERROR();
            return ERROR;
        }
#else
        /*
        // Exact compare is OK because overflow is handled below.
        */
        if (*vp_pos == 0.0)
        {
            SET_DIVIDE_BY_ZERO_ERROR();
            return ERROR;
        }
#endif

        if (    (respect_missing_values())
             && ((IS_MISSING_DBL(*row_pos)) || (IS_MISSING_DBL(*vp_pos)))
           )
        {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
            dbm("Missing value arithmetic.");
#endif
            *row_pos = DBL_MISSING;
        }
        else
        {
            *row_pos /= *vp_pos;
        }

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
        if (    (*row_pos > DBL_MOST_POSITIVE)
             || (*row_pos < DBL_MOST_NEGATIVE)
           )
        {
            SET_OVERFLOW_ERROR();
            return ERROR;
        }
#endif

        row_pos++;
        vp_pos++;
    }

    return NO_ERROR;

#else

    result = invert_vector(&inv_vp, vp);

    if (result != ERROR)
    {
        result = ow_multiply_matrix_row_by_vector(mp, inv_vp, row);
    }

    free_vector(inv_vp);

    return result;

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      ow_multiply_matrix_col_by_vector
 *
 * Multiplies a specified column of a matrix by a vector
 *
 * This routine multiplies a specified column of a matrix by a vector. The
 * length of the vector must match the number of rows in the matrix.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine returns error if the length of the vector is not the
 *     number of rows in the matrix. Other dimension mismatches are treated as
 *     bugs (see set_bug()), also returning ERROR.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

/*
 * TODO: Need to do add, subtract, divide, still.
*/

int ow_multiply_matrix_col_by_vector(Matrix* mp, const Vector* vp, int col)
{
    int     i;
    int     num_rows = mp->num_rows;
    int     num_cols = mp->num_cols;
    double* vp_pos = vp->elements;


    UNTESTED_CODE();

    if ((col < 0) || (col >= mp->num_cols))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (num_rows != vp->length)
    {
        dbw();
        set_error("Attempt to multiply a column of a %d by %d matrix with a vector of length %d.",
                  num_rows, num_cols, vp->length);
        return ERROR;
    }

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        UNTESTED_CODE();

        for (i=0; i<num_rows; i++)
        {
            if ((IS_MISSING_DBL(mp->elements[ i ][ col ])) || (IS_MISSING_DBL(*vp_pos)))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                mp->elements[ i ][ col ] = DBL_MISSING;
            }
            else
            {
                mp->elements[ i ][ col ] *= *vp_pos;
            }

            vp_pos++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            mp->elements[ i ][ col ] *= *vp_pos;
            vp_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_add_scalar_to_matrix_row
 *
 * Adds a scalar to a specified row of a matrix
 *
 * This routine adds a scalar to a specified row of the input matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails If any of the matrix elements are too close to
 *     zero.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_scalar_to_matrix_row(Matrix* mp, double scalar, int row)
{
    int   j;
    int   num_cols;
    double* row_pos;


    if ((row < 0) || (row >= mp->num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;

    for (j=0; j<num_cols; j++)
    {
        *row_pos += scalar;
        row_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_subtract_scalar_from_matrix_row
 *
 * Subtracts a scalar from a specified row of a matrix
 *
 * This routine subtracts a scalar from a specified row of the input matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails If any of the matrix elements are too close to
 *     zero.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_scalar_from_matrix_row(Matrix* mp, double scalar, int row)
{
    int   j;
    int   num_cols;
    double* row_pos;


    UNTESTED_CODE();

    if ((row < 0) || (row >= mp->num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;

    for (j=0; j<num_cols; j++)
    {
        *row_pos -= scalar;
        row_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_multiply_matrix_row_by_scalar
 *
 * Multiplies a specified row of a matrix by a scalar
 *
 * This routine multiplies the specified row of a matrix by a scalar.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails If any of the matrix elements are too close to
 *     zero.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_matrix_row_by_scalar(Matrix* mp, double scalar, int row)
{
    int   j;
    int   num_cols;
    double* row_pos;


    if ((row < 0) || (row >= mp->num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;

    for (j=0; j<num_cols; j++)
    {
        *row_pos *= scalar;
        row_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_divide_matrix_row_by_scalar
 *
 * Divides a specified row of a matrix by a scalar
 *
 * This routine divides a specified row of the input matrix by a scalar.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails If any of the matrix elements are too close to
 *     zero.
 *
 * Related:
 *     divide_matrix_by_row_vector(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_divide_matrix_row_by_scalar(Matrix* mp, double scalar, int row)
{
#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    int   j;
    int   num_cols;
    double* row_pos;


    if ((row < 0) || (row >= mp->num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
    if (IS_ZERO_DBL(scalar)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        return ERROR;
    }
#else
    /*
    // Exact compare is OK because overflow is handled below.
    */
    if (scalar == 0.0)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        return ERROR;
    }
#endif

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;

    for (j=0; j<num_cols; j++)
    {

        ASSERT_IS_NUMBER_DBL(*row_pos);
        ASSERT_IS_FINITE_DBL(*row_pos);

        ASSERT_IS_NUMBER_DBL(scalar);
        ASSERT_IS_FINITE_DBL(scalar);
        ASSERT_IS_GREATER_DBL(scalar, 10.9 * DBL_MIN);

        *row_pos /= scalar;

        ASSERT_IS_NUMBER_DBL(*row_pos);
        ASSERT_IS_FINITE_DBL(*row_pos);

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
        if (    (*row_pos > DBL_MOST_POSITIVE)
             || (*row_pos < DBL_MOST_NEGATIVE)
           )
        {
            SET_OVERFLOW_ERROR();
            return ERROR;
        }
#endif

        row_pos++;
    }

    return NO_ERROR;

#else

    if (ABS_OF(scalar) < 10.0 * DBL_MIN)
    {
        SET_DIVIDE_BY_ZERO_ERROR();
        return ERROR;
    }
    else
    {
        return ow_multiply_matrix_row_by_scalar(mp, 1.0 / scalar, row);
    }

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         add_col_vector_to_matrix
 *
 * Adds a vector to each column of a matrix
 *
 * This routine adds a vector to each column of a matrix. The length of the
 * vector must match the number of rows in the matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure. The routine will also fail due to dimension mismatch,
 *     but this is currently treated as a bug (see set_bug()).
 *
 * Related:
 *     ow_add_col_vector_to_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int add_col_vector_to_matrix(Matrix** target_mpp,
                             const Matrix* source_mp,
                             const Vector* vp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;
    double*   vp_pos;


    if (source_mp->num_rows != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *target_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    (*target_row_pos_ptr) = (*source_row_pos_ptr) + (*vp_pos);
                }

                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
            target_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                (*target_row_pos_ptr) = (*source_row_pos_ptr) + (*vp_pos);
                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
            target_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             ow_add_col_vector_to_matrix
 *
 * Adds a vector to each column of a matrix.
 *
 * This routine adds a vector to each column of the input matrix. The length of
 * the vector must match the number of rows in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can't fail gracefully.  The routine will fail
 *     due to dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Related:
 *     add_col_vector_to_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_col_vector_to_matrix(Matrix* source_mp, const Vector* vp)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;
    double*   vp_pos;


    if (source_mp->num_rows != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *source_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *source_row_pos_ptr += (*vp_pos);
                }
                source_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                *source_row_pos_ptr += (*vp_pos);
                source_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       subtract_col_vector_from_matrix
 *
 * Subtracts a vector to each column of a matrix
 *
 * This routine subtracts a vector to each column of a matrix. The length of the
 * vector must match the number of rows in the matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure. The routine will also fail due to dimension mismatch,
 *     but this is currently treated as a bug (see set_bug()).
 *
 * Related:
 *     ow_subtract_col_vector_from_matrix(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int subtract_col_vector_from_matrix(Matrix** target_mpp,
                                    const Matrix* source_mp,
                                    const Vector* vp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;
    double*   vp_pos;


    if (source_mp->num_rows != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *target_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    (*target_row_pos_ptr) = (*source_row_pos_ptr) - (*vp_pos);
                }

                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
            target_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                (*target_row_pos_ptr) = (*source_row_pos_ptr) - (*vp_pos);
                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
            target_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          ow_subtract_col_vector_from_matrix
 *
 * Subtracts a vector to each column of a matrix.
 *
 * This routine subtracts a vector to each column of the input matrix. The
 * length of the vector must match the number of rows in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can't fail gracefully.  The routine will fail
 *     due to dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Related:
 *     subtract_vector_to_matrix_cols(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_col_vector_from_matrix(Matrix* source_mp, const Vector* vp)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;
    double*   vp_pos;


    if (source_mp->num_rows != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *source_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *source_row_pos_ptr -= (*vp_pos);
                }
                source_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                *source_row_pos_ptr -= (*vp_pos);
                source_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                         multiply_matrix_by_col_vector_ew
 *
 * Multiplies each column of a matrix by a vector
 *
 * This routine multiplies each column of a matrix by a vector. The length of
 * the vector must match the number of rows in the matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can only fail (gracefully!) due to memory
 *     allocation failure. The routine will also fail due to dimension mismatch,
 *     but this is currently treated as a bug (see set_bug()).
 *
 * Related:
 *     ow_multiply_matrix_by_col_vector_ew(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_matrix_by_col_vector_ew(Matrix** target_mpp,
                                     const Matrix* source_mp,
                                     const Vector* vp)
{
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;
    double*   vp_pos;


    if (source_mp->num_rows != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *target_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    (*target_row_pos_ptr) = (*source_row_pos_ptr) * (*vp_pos);
                }

                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
            target_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;
            target_row_pos_ptr = *target_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                (*target_row_pos_ptr) = (*source_row_pos_ptr) * (*vp_pos);
                source_row_pos_ptr++;
                target_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
            target_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      ow_multiply_matrix_by_col_vector_ew
 *
 * Multiplies each column of a matrix by a vector
 *
 * This routine multiplies each column of the input matrix by a vector. The
 * length of the vector must match the number of rows in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can't fail gracefully.  The routine will fail
 *     due to dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Related:
 *     multiply_matrix_by_col_vector_ew(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_matrix_by_col_vector_ew(Matrix* source_mp, const Vector* vp)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;
    double*   vp_pos;


    if (source_mp->num_rows != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate some code for speed.
    */
    if (respect_missing_values())
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                if (    (IS_MISSING_DBL(*source_row_pos_ptr))
                     || (IS_MISSING_DBL(*vp_pos))
                   )
                {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                    dbm("Missing value arithmetic.");
#endif
                    *source_row_pos_ptr = DBL_MISSING;
                }
                else
                {
                    *source_row_pos_ptr *= (*vp_pos);
                }
                source_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_row_pos_ptr = *source_row_ptr;

            for (j=0; j<num_cols; j++)
            {
                *source_row_pos_ptr *= (*vp_pos);
                source_row_pos_ptr++;
            }

            vp_pos++;
            source_row_ptr++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          divide_matrix_by_col_vector
 *
 * Divides each column of a matrix by a vector
 *
 * This routine divides each column of a matrix by a vector. The length of the
 * vector must match the number of rows in the matrix.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails if any of the matrix elements are too close to
 *     zero, or if memory allocation fails. The routine will also fail due to
 *     dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Related:
 *     ow_divide_matrix_by_col_vector(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int divide_matrix_by_col_vector(Matrix** target_mpp,
                                const Matrix* source_mp,
                                const Vector* vp)
{
#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    Matrix* target_mp;
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double**  target_row_ptr;
    double*   source_row_pos_ptr;
    double*   target_row_pos_ptr;
    double*   vp_pos;
#else
    Vector* inv_vp = NULL;
    int result;
#endif


#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    if (source_mp->num_rows != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef CHECK_MATRIX_INITIALZATION_ON_FREE
    /* Prepare for divide by zero. */
    ERE(get_zero_matrix(target_mpp, num_rows, num_cols));
#else
    ERE(get_target_matrix(target_mpp, source_mp->num_rows,
                          source_mp->num_cols));
#endif

    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    vp_pos = vp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = *source_row_ptr;
        target_row_pos_ptr = *target_row_ptr;

        for (j=0; j<num_cols; j++)
        {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
            if (IS_ZERO_DBL(*vp_pos))
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#else
            /*
            // Exact compare is OK because overflow is handled below.
            */
            if (*vp_pos == 0.0)
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#endif

            (*target_row_pos_ptr) = (*source_row_pos_ptr) / (*vp_pos);

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*target_row_pos_ptr > DBL_MOST_POSITIVE)
                 || (*target_row_pos_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif

            source_row_pos_ptr++;
            target_row_pos_ptr++;
        }

        vp_pos++;
        source_row_ptr++;
        target_row_ptr++;
    }

    return NO_ERROR;

#else

    result = invert_vector(&inv_vp, vp);

    if (result == ERROR)
    {
        NOTE_ERROR(); 
    }
    else 
    {
        result = multiply_matrix_by_col_vector_ew(target_mpp, source_mp, inv_vp);
    }

    free_vector(inv_vp);

    return result;

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_divide_matrix_by_col_vector
 *
 * Divides each column of a matrix by a vector
 *
 * This routine divides each column of the input matrix by a vector. The length
 * of the vector must match the number of rows in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero, This routine will also fail if there is a dimension mismatch,
 *     but this is currently treated as a bug (see set_bug()).
 *
 * Related:
 *     divide_matrix_by_col_vector(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_divide_matrix_by_col_vector(Matrix* source_mp, const Vector* vp)
{
#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    double**  source_row_ptr;
    double*   source_row_pos_ptr;
    double*   vp_pos;
#else
    Vector* inv_vp = NULL;
    int result;
#endif


#ifdef SLOW_MATRIX_DIVIDE  /* For regession testing. */
    if (source_mp->num_rows != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    vp_pos = vp->elements;

    for (i=0; i<num_rows; i++)
    {
        source_row_pos_ptr = *source_row_ptr;

        for (j=0; j<num_cols; j++)
        {
#ifdef USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO
            if (IS_ZERO_DBL(*vp_pos))
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#else
            /*
            // Exact compare is OK because overflow is handled below.
            */
            if (*vp_pos == 0.0)
            {
                SET_DIVIDE_BY_ZERO_ERROR();
                return ERROR;
            }
#endif

            *source_row_pos_ptr /= (*vp_pos);

#ifdef USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
            if (    (*source_row_pos_ptr > DBL_MOST_POSITIVE)
                 || (*source_row_pos_ptr < DBL_MOST_NEGATIVE)
               )
            {
                SET_OVERFLOW_ERROR();
                return ERROR;
            }
#endif

            source_row_pos_ptr++;
        }

        vp_pos++;
        source_row_ptr++;
    }

    return NO_ERROR;

#else

    result = invert_vector(&inv_vp, vp);

    if (result != ERROR)
    {
        result = ow_multiply_matrix_by_col_vector_ew(source_mp, inv_vp);
    }

    free_vector(inv_vp);

    return result;

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_add_matrix_row_times_scalar
 *
 * Adds matrix row times a scalar to a second matrix row
 *
 * This routine adds a matrix row times a scalar to a matrix row specified by
 * the first two arguments.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_matrix_row_times_scalar
(
    Matrix*       target_mp,
    int           target_row,
    const Matrix* source_mp,
    int           source_row,
    double        scalar 
)
{
    int     j;
    int     num_cols;
    double*   target_row_pos;
    double*   source_row_pos;


    if (    (target_mp->num_cols != source_mp->num_cols)
         || (target_row < 0)
         || (target_row >= target_mp->num_rows)
         || (source_row < 0)
         || (source_row >= source_mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    target_row_pos = *(target_mp->elements + target_row);
    source_row_pos = *(source_mp->elements + source_row);

    num_cols = target_mp->num_cols;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        UNTESTED_CODE();

        for (j=0; j<num_cols; j++)
        {
            if (    (IS_MISSING_DBL(*target_row_pos)) 
                 || (IS_MISSING_DBL(*source_row_pos))
                 || (IS_MISSING_DBL(scalar))
               )
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *target_row_pos = DBL_MISSING;
            }
            else
            {
                *target_row_pos += (*source_row_pos)*scalar;
            }

            target_row_pos++;
            source_row_pos++;
        }
    }
    else
    {
        for (j=0; j<num_cols; j++)
        {
            *target_row_pos += (*source_row_pos)*scalar;
            target_row_pos++;
            source_row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_add_matrix_rows_ew
 *
 * Adds a specified row of a matrix by a row of another
 *
 * This routine multiplies a specified row of a matrix by a row of another. The
 * number of columns of the two matrices must match.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_matrix_rows_ew(Matrix* mp, int row, const Matrix* mp2, int row2)
{
    int     j;
    int     num_cols = mp->num_cols;
    double*   row_pos = *(mp->elements + row);
    double*   row2_pos = *(mp2->elements + row2);


    UNTESTED_CODE();

    if (    (num_cols != mp2->num_cols)
         || (row < 0)
         || (row >= mp->num_rows)
         || (row2 < 0)
         || (row2 >= mp2->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        UNTESTED_CODE();

        for (j=0; j<num_cols; j++)
        {
            if ((IS_MISSING_DBL(*row_pos)) || (IS_MISSING_DBL(*row2_pos)))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *row_pos = DBL_MISSING;
            }
            else
            {
                *row_pos += *row2_pos;
            }

            row_pos++;
            row2_pos++;
        }
    }
    else
    {
        for (j=0; j<num_cols; j++)
        {
            *row_pos += *row2_pos;

            row_pos++;
            row2_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_multiply_matrix_rows_ew
 *
 * Multiplies a specified row of a matrix by a row of another
 *
 * This routine multiplies a specified row of a matrix by a row of another. The
 * number of columns of the two matrices must match.
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_matrix_rows_ew(Matrix* mp, int row, const Matrix* mp2, int row2)
{
    int     j;
    int     num_cols = mp->num_cols;
    double*   row_pos = *(mp->elements + row);
    double*   row2_pos = *(mp2->elements + row2);


    UNTESTED_CODE();

    if (    (num_cols != mp2->num_cols)
         || (row < 0)
         || (row >= mp->num_rows)
         || (row2 < 0)
         || (row2 >= mp2->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        UNTESTED_CODE();

        for (j=0; j<num_cols; j++)
        {
            if ((IS_MISSING_DBL(*row_pos)) || (IS_MISSING_DBL(*row2_pos)))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *row_pos = DBL_MISSING;
            }
            else
            {
                *row_pos *= *row2_pos;
            }

            row_pos++;
            row2_pos++;
        }
    }
    else
    {
        for (j=0; j<num_cols; j++)
        {
            *row_pos *= *row2_pos;

            row_pos++;
            row2_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              sum_matrix_elements
 *
 * Adds the elements of a matrix
 *
 * This routine returns the sum of the elements of the matrix pointed to by mp
 *
 * Returns:
 *    The sum of the elements of the matrix pointed to by mp.
 *
 * Related:
 *     average_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

double sum_matrix_elements(const Matrix* mp)
{
    int    i;
    int    j;
    double   sum;
    double** row_ptr;
    double*  element_ptr;


    if (mp->num_rows == 0) return 0.0;
    if (mp->num_cols == 0) return 0.0;

    sum = 0.0;

    row_ptr =  mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            sum += *element_ptr;
            element_ptr++;
        }
        row_ptr++;
    }

     return sum;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          sum_matrix_row_elements
 *
 * Returns the sum of a specified matrix row
 *
 * This routine returns the sum of the elements of a specified row of the
 * matrix pointed to by mp
 *
 * Returns:
 *    The sum of the elements of the specified row of the matrix pointed to by
 *    mp.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

double sum_matrix_row_elements(const Matrix* mp, int row)
{
    int   j;
    double  sum         = 0.0;
    double* element_ptr = *(mp->elements + row);


    for (j = 0; j<mp->num_cols; j++)
    {
        sum += *element_ptr;
        element_ptr++;
    }

     return sum;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              sum_matrix_col_elements
 *
 * Returns the sum of a specified matrix column
 *
 * This routine returns the sum of the elements of a specified column of the
 * matrix pointed to by mp
 *
 * Returns:
 *    The sum of the elements of the specified column of the matrix pointed to
 *    by mp.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

double sum_matrix_col_elements(const Matrix* mp, int col)
{
    int  i;
    double sum = 0.0;


    for (i = 0; i<mp->num_rows; i++)
    {
        sum += mp->elements[ i ][ col ];
    }

     return sum;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              average_matrix_elements
 *
 * Averages the elements of a matrix
 *
 * This routine returns the average of the elements of the matrix pointed to by
 * mp
 *
 * Returns:
 *    The average of the elements of the matrix pointed to by mp.
 *
 * Related:
 *     sum_matrix_elements(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

double average_matrix_elements(const Matrix* mp)
{


    if (mp->num_rows == 0) return 0.0;
    if (mp->num_cols == 0) return 0.0;

    return sum_matrix_elements(mp) / ((mp->num_rows) * (mp->num_cols));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       ow_subtract_identity_matrix
 *
 * Subtracts the identity matrix from a square matrix.
 *
 * This routine subtracts the identity matrix from a square matrix. The matrix
 * is over-written. If the matrix is not square, this is treated as a bug.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_subtract_identity_matrix(Matrix* mp)
{
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    int i;

    if (num_cols != num_rows)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        mp->elements[ i ][ i ] -= 1.0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            do_matrix_recomposition
 *
 * Computes A*diag*B
 *
 * This routine multiplies a matrix by a a diagonal matrix stored as a vector,
 * and then multiplies the result be a third matrix. The matrices must be
 * compatable for muliplication, or ERROR is returned.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int do_matrix_recomposition(Matrix** target_mpp,
                            const Matrix* first_mp,
                            const Vector* vp,
                            const Matrix* second_mp)
{
    Matrix* temp_target_mp = NULL;
    int     result = NO_ERROR;



    ERE(multiply_matrix_by_col_vector_ew(&temp_target_mp, first_mp, vp));

    if (multiply_matrices(target_mpp, temp_target_mp, second_mp) == ERROR)
    {
        result = ERROR;
    }

    free_matrix(temp_target_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            do_matrix_recomposition_2
 *
 * Computes A*diag*B'
 *
 * This routine multiplies a matrix by a a diagonal matrix stored as a vector
 * by a matrix transpose. The matrices must be compatable for muliplication, or
 * ERROR is returned.
 *
 * The first argument is a pointer to the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int do_matrix_recomposition_2
(
    Matrix**      target_mpp,
    const Matrix* first_mp,
    const Vector* vp,
    const Matrix* second_mp
)
{
    Matrix* temp_target_mp = NULL;
    int     result = NO_ERROR;



    ERE(multiply_matrix_by_col_vector_ew(&temp_target_mp, first_mp, vp));

    if (multiply_by_transpose(target_mpp, temp_target_mp, second_mp) == ERROR)
    {
        result = ERROR;
    }

    free_matrix(temp_target_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            log_sum_log_matrix_elements
 *
 * Adds the elements of a matrix in log space
 *
 * This routine returns the log of the sum of the anti-log (exp()) of the
 * elements of the matrix pointed to by mp. This is done is a way which reduces
 * problems due the possibility that the numbers are not well represented in
 * non-log space. Think of the matrix being a log representation of some
 * quantities. We want the sum of the non-log quantities, but we want the log
 * representation of it.
 *
 * If needed, LOG_ZERO is used to represent log(0). LOG_ZERO is suffiently
 * negative number that exp(LOG_ZERO) is zero.
 *
 * Returns:
 *    log(sum(ew_exp(mp))   [ "ew" means element-wise ].
 *
 * Related:
 *     sum_matrix_elements, average_matrix_elements)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

double log_sum_log_matrix_elements(Matrix* log_mp)
{
    int num_rows = log_mp->num_rows;
    int num_cols = log_mp->num_cols;
    int i, j;
    double    max = DBL_MOST_NEGATIVE;
    double    sum = 0.0;

    verify_matrix(log_mp, NULL);

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            max = MAX_OF(max, log_mp->elements[ i ][ j ]);
        }
    }

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            double temp = exp(log_mp->elements[ i ][ j ] - max);
            sum += temp;
        }
    }

    return (SAFE_LOG(sum) + max);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                      ow_exp_scale_by_sum_log_matrix_row
 *
 * Scales the elements of a matrix row in log space
 *
 * This routine scales a row of a matrix of log quantities by the sum of the non
 * log quantities, replacing the matrix row with non log quantities. This is a a
 * standard normalization in probablistic calculations. The log of the sum is
 * returned (which typically maps onto a log likelihood of a mixture
 * marginalization).
 *
 * If needed, LOG_ZERO is used to represent log(0). LOG_ZERO is suffiently
 * negative number that exp(LOG_ZERO) is zero.
 *
 * Returns:
 *    log(sum(ew_exp(row))   [ "ew" means element-wise ].
 *
 * Related:
 *     sum_matrix_elements, average_matrix_elements,
 *     ow_scale_matrix_rows_by_sums, ow_scale_matrix_row_by_sum,
 *     log_sum_log_matrix_elements
 *
 * Index: matrices, matrix arithmetic, probability
 *
 * -----------------------------------------------------------------------------
 */

double ow_exp_scale_by_sum_log_matrix_row(Matrix* log_mp, int row)
{
    int num_cols = log_mp->num_cols;
    int j;
    double    max = DBL_MOST_NEGATIVE;
    double    sum = 0.0;
    double log_sum;


    verify_matrix(log_mp, NULL);

    for (j = 0; j < num_cols; j++)
    {
        max = MAX_OF(max, log_mp->elements[ row ][ j ]);
    }

    for (j = 0; j < num_cols; j++)
    {
        double temp = exp(log_mp->elements[ row ][ j ] - max);
        sum += temp;
    }

    log_sum = SAFE_LOG(sum);

    for (j = 0; j < num_cols; j++)
    {
        log_mp->elements[ row ][ j ] = exp(log_mp->elements[ row ][ j ]  - max - log_sum);
    }

#ifdef TEST
    sum = 0.0;

    for (j = 0; j < num_cols; j++)
    {
        sum += log_mp->elements[ row ][ j ];
    }

    ASSERT(ABS_OF(sum - 1.0) < 0.00001);
#endif

    return log_sum + max;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           ow_add_matrix_row_to_vector
 *
 * Adds a matrix row to a vector
 *
 * If we are respecting missing values (disabled by default), then, if either
 * operands are DBL_MISSING, then the result is DBL_MISSING as well.
 *
 * This routine adds a specified matrix row to a vector. The length of the
 * vector must match the number of columns in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine fails If any of the matrix elements are too close to
 *     zero.
 *
 * Index: vectors, matrices, vector arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_matrix_row_to_vector(Vector* vp, const Matrix* mp, int row)
{
    int     j;
    int     num_cols;
    double*   row_pos;
    double*   vp_pos;


    if (    (mp->num_cols != vp->length)
         || (row < 0)
         || (row >= mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos = *(mp->elements + row);
    num_cols = mp->num_cols;
    vp_pos = vp->elements;

    /*
     * Duplicate code for speed.
    */
    if (respect_missing_values())
    {
        for (j=0; j<num_cols; j++)
        {
            if ((IS_MISSING_DBL(*row_pos)) || (IS_MISSING_DBL(*vp_pos)))
            {
#ifdef REPORT_MISSING_VALUE_ARITHMETIC
                dbm("Missing value arithmetic.");
#endif
                *vp_pos = DBL_MISSING;
            }
            else
            {
                *vp_pos += *row_pos;
            }

            row_pos++;
            vp_pos++;
        }
    }
    else
    {
        for (j=0; j<num_cols; j++)
        {
            *vp_pos += *row_pos;

            row_pos++;
            vp_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_get_abs_of_matrix
 *
 * Calculates the element-wise absolute value of a matrix.
 *
 * This routine calculates the element-wise absolute value of a matrix,
 * overwiting the input matrix with the result.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. THis routine will fail if any of the element of the input vector is
 *     too close to zero.
 *
 * Index: matrices, matrix arithmetic
 *
 * Author: Ernesto Brau, Kobus Barnard
 *
 * Documentor: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int ow_get_abs_of_matrix(Matrix* mp)
{
    int i, j;


    for (i=0; i<mp->num_rows; i++)
    {
        for (j=0; j<mp->num_cols; j++)
        {
            mp->elements[ i ][ j ] = ABS_OF(mp->elements[ i ][ j ]);
        }
    }

   return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_abs_of_matrix
 *
 * Calculates the absolute values of matrix components.
 *
 * This routine calculates the absolute values of matrix components.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. This routine will fail if any of the matrix elements are too close
 *     to zero, or if memory allocation fails.
 *
 * Index: matrices, matrix arithmetic
 *
 * Author: Ernesto Brau, Kobus Barnard
 *
 * Documentor: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int get_abs_of_matrix(Matrix** target_mpp, const Matrix* source_mp)
{
    Matrix* target_mp;
    int i, j;

    ERE(get_target_matrix(target_mpp, source_mp->num_rows, source_mp->num_cols));
    target_mp = *target_mpp;

    for (i=0; i<source_mp->num_rows; i++)
    {
        for (j=0; j<source_mp->num_cols; j++)
        {
            target_mp->elements[i][j] = ABS_OF(source_mp->elements[i][j]);
        }
    }

   return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_euler_rotation_matrix
 *
 * Creates a 3D euler rotation matrix given three input angles (in radian)
 *
 * Phi, theta and psi are, respectively, the rotations around
 * the z, x and y axes
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
void get_euler_rotation_matrix
(
    Matrix  ** target_mpp,
    float      phi,
    float      theta,
    float      psi
)
{
    double cos_phi, sin_phi;
    double cos_theta, sin_theta;
    double cos_psi, sin_psi;

    cos_phi   = cos(phi);
    sin_phi   = sin(phi);
    cos_theta = cos(theta);
    sin_theta = sin(theta);
    cos_psi   = cos(psi);
    sin_psi   = sin(psi);

    get_zero_matrix(target_mpp, 3, 3);
    (*target_mpp)->elements[0][0] = cos_psi*cos_phi - cos_theta*sin_phi*sin_psi;
    (*target_mpp)->elements[0][1] = cos_psi*sin_phi + cos_theta*cos_phi*sin_psi;
    (*target_mpp)->elements[0][2] = sin_psi*sin_theta;
    (*target_mpp)->elements[1][0] = -sin_psi*cos_phi - cos_theta*sin_phi*cos_psi;
    (*target_mpp)->elements[1][1] = -sin_psi*sin_phi + cos_theta*cos_phi*cos_psi;
    (*target_mpp)->elements[1][2] = cos_psi*sin_theta;
    (*target_mpp)->elements[2][0] = sin_theta*sin_phi;
    (*target_mpp)->elements[2][1] = -sin_theta*cos_phi;
    (*target_mpp)->elements[2][2] = cos_theta;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_euler_homo_rotation_matrix
 *
 * Creates a 3D euler homogeneous rotation matrix given three input angles (in radian)
 *
 * Phi, theta and psi are, respectively, the rotations around
 * the z, x and y axes
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
void get_euler_homo_rotation_matrix
(
    Matrix  ** target_mpp,
    float      phi,
    float      theta,
    float      psi
)
{
    double cos_phi, sin_phi;
    double cos_theta, sin_theta;
    double cos_psi, sin_psi;

    cos_phi   = cos(phi);
    sin_phi   = sin(phi);
    cos_theta = cos(theta);
    sin_theta = sin(theta);
    cos_psi   = cos(psi);
    sin_psi   = sin(psi);

    get_zero_matrix(target_mpp, 4, 4);
    (*target_mpp)->elements[0][0] = cos_psi*cos_phi - cos_theta*sin_phi*sin_psi;
    (*target_mpp)->elements[0][1] = cos_psi*sin_phi + cos_theta*cos_phi*sin_psi;
    (*target_mpp)->elements[0][2] = sin_psi*sin_theta;
    (*target_mpp)->elements[0][3] = 0.0;
    (*target_mpp)->elements[1][0] = -sin_psi*cos_phi - cos_theta*sin_phi*cos_psi;
    (*target_mpp)->elements[1][1] = -sin_psi*sin_phi + cos_theta*cos_phi*cos_psi;
    (*target_mpp)->elements[1][2] = cos_psi*sin_theta;
    (*target_mpp)->elements[1][3] = 0.0;
    (*target_mpp)->elements[2][0] = sin_theta*sin_phi;
    (*target_mpp)->elements[2][1] = -sin_theta*cos_phi;
    (*target_mpp)->elements[2][2] = cos_theta;
    (*target_mpp)->elements[2][3] = 0.0;
    (*target_mpp)->elements[3][1] = 0.0;
    (*target_mpp)->elements[3][2] = 0.0;
    (*target_mpp)->elements[3][3] = 0.0;
    (*target_mpp)->elements[3][3] = 1.0;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_3d_rotation_matrix_1
 *
 * Creates a 3D rotation matrix,
 * from three angles stored in a vector
 *
 * The input vector is the vector to rotate around. Phi
 * is the rotation angle, use a negative angle for clockwise
 * rotation.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_rotation_matrix_1
(
    Matrix**      target_mpp,
    double          phi,
    const Vector* v
)
{
    if (v->length < 3)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return ERROR;
    }

    return get_3d_rotation_matrix_2(target_mpp, phi,
            v->elements[0], v->elements[1], v->elements[2]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_3d_rotation_matrix_2
 *
 * Creates a 3D rotation matrix from three angles stored in a vector
 *
 * X, Y, and Z are the coordinates of the vector to rotate around. Phi
 * is the rotation angle, use a negative angle for clockwise
 * rotation.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_rotation_matrix_2
(
    Matrix  ** target_mpp,
    double     phi,
    double     x,
    double     y,
    double     z
)
{
    double cos_phi, cos_theta, cos_psi;
    double sin_phi, sin_theta, sin_psi;
    double mag_xyz, mag_xy;

    Matrix* R_phi   = NULL;
    Matrix* R_theta = NULL;
    Matrix* R_psi   = NULL;

    /*
     * We are trying to be consistent with most geometry books where
     * negative angles imply clockwise rotation.
     */
    phi = -phi;

    mag_xyz = sqrt(x*x + y*y + z*z);

    if (mag_xyz < 1.0e-8)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return ERROR;
    }

    /* psi used to rotate around z-axis to put vector on the yz-plane. */
    mag_xy = sqrt(x*x + y*y);
    if (mag_xy > 1.0e-8)
    {
        cos_psi = y / mag_xy;
        sin_psi = x / mag_xy;
    }
    else
    {
        cos_psi = 0.0;
        sin_psi = 1.0;
    }

    /* theta used to rotate around x-axis to put vector onto the z-axis. */
    cos_theta = z / mag_xyz;
    sin_theta = mag_xy / mag_xyz;

    /* phi used to rotate around z-axis. This is actually the angle to rotate
     * around the vector. */
    cos_phi = cos(phi);
    sin_phi = sin(phi);

    get_identity_matrix(&R_phi, 3);
    R_phi->elements[ 0 ][ 0 ] = cos_phi;
    R_phi->elements[ 0 ][ 1 ] = sin_phi;
    R_phi->elements[ 1 ][ 0 ] = -sin_phi;
    R_phi->elements[ 1 ][ 1 ] = cos_phi;

    get_identity_matrix(&R_theta, 3);
    R_theta->elements[ 1 ][ 1 ] = cos_theta;
    R_theta->elements[ 1 ][ 2 ] = sin_theta;
    R_theta->elements[ 2 ][ 1 ] = -sin_theta;
    R_theta->elements[ 2 ][ 2 ] = cos_theta;

    get_identity_matrix(&R_psi, 3);
    R_psi->elements[ 0 ][ 0 ] = cos_psi;
    R_psi->elements[ 0 ][ 1 ] = sin_psi;
    R_psi->elements[ 1 ][ 0 ] = -sin_psi;
    R_psi->elements[ 1 ][ 1 ] = cos_psi;

    /* *m_out = R_psi * R_theta * R_phi * R_theta^T * R_psi^T */
    multiply_matrices(target_mpp, R_psi, R_theta);
    multiply_matrices(target_mpp, *target_mpp, R_phi);
    get_matrix_transpose(&R_theta, R_theta);
    get_matrix_transpose(&R_psi, R_psi);
    multiply_matrices(target_mpp, *target_mpp, R_theta);
    multiply_matrices(target_mpp, *target_mpp, R_psi);

    free_matrix(R_phi);
    free_matrix(R_theta);
    free_matrix(R_psi);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_2d_rotation_matrix
 *
 * Creates a 2D rotation matrix to rotate in 2D around the origin
 *
 * Phi is the rotation angle, use a negative angle for clockwise
 * rotation.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_2d_rotation_matrix
(
    Matrix  ** target_mpp,
    double     phi
)
{
    double cos_phi = cos(phi);
    double sin_phi = sin(phi);

    get_identity_matrix(target_mpp, 2);
    (*target_mpp)->elements[ 0 ][ 0 ] = cos_phi;
    (*target_mpp)->elements[ 0 ][ 1 ] = sin_phi;
    (*target_mpp)->elements[ 1 ][ 0 ] = -sin_phi;
    (*target_mpp)->elements[ 1 ][ 1 ] = cos_phi;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_3d_homo_rotation_matrix_1
 *
 * Creates a 3D rotation matrix in homogeneous coordinates,
 * from three angles stored in a vector
 *
 * The input vector is the vector to rotate around. Phi
 * is the rotation angle, use a negative angle for clockwise
 * rotation.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_homo_rotation_matrix_1
(
    Matrix**      target_mpp,
    double          phi,
    const Vector* v
)
{
    if (v->length < 3)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return ERROR;
    }

    return get_3d_homo_rotation_matrix_2(target_mpp, phi,
            v->elements[0], v->elements[1], v->elements[2]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_3d_rotation_matrix_2
 *
 * Creates a 3D rotation matrix from three angles stored in a vector
 *
 * X, Y, and Z are the coordinates of the vector to rotate around. Phi
 * is the rotation angle, use a negative angle for clockwise
 * rotation.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_homo_rotation_matrix_2
(
    Matrix**   target_mpp,
    double     phi,
    double     x,
    double     y,
    double     z
)
{
    double cos_phi, cos_theta, cos_psi;
    double sin_phi, sin_theta, sin_psi;
    double mag_xyz, mag_xy;

    Matrix* R_phi   = NULL;
    Matrix* R_theta = NULL;
    Matrix* R_psi   = NULL;
    /*
     * We are trying to be consistent with most geometry books where
     * negative angles imply clockwise rotation.
     */
    phi = -phi;

    mag_xyz = sqrt(x*x + y*y + z*z);

    if (mag_xyz < 1.0e-8)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return ERROR;
    }

    /* psi used to rotate around z-axis to put vector on the yz-plane. */
    mag_xy = sqrt(x*x + y*y);
    if (mag_xy > 1.0e-8)
    {
        cos_psi = y / mag_xy;
        sin_psi = x / mag_xy;
    }
    else
    {
        cos_psi = 0.0;
        sin_psi = 1.0;
    }

    /* theta used to rotate around x-axis to put vector onto the z-axis. */
    cos_theta = z / mag_xyz;
    sin_theta = mag_xy / mag_xyz;

    /* phi used to rotate around z-axis. This is actually the angle to rotate
     * around the vector. */
    cos_phi = cos(phi);
    sin_phi = sin(phi);

    get_identity_matrix(&R_phi, 4);
    R_phi->elements[ 0 ][ 0 ] = cos_phi;
    R_phi->elements[ 0 ][ 1 ] = sin_phi;
    R_phi->elements[ 1 ][ 0 ] = -sin_phi;
    R_phi->elements[ 1 ][ 1 ] = cos_phi;

    get_identity_matrix(&R_theta, 4);
    R_theta->elements[ 1 ][ 1 ] = cos_theta;
    R_theta->elements[ 1 ][ 2 ] = sin_theta;
    R_theta->elements[ 2 ][ 1 ] = -sin_theta;
    R_theta->elements[ 2 ][ 2 ] = cos_theta;

    get_identity_matrix(&R_psi, 4);
    R_psi->elements[ 0 ][ 0 ] = cos_psi;
    R_psi->elements[ 0 ][ 1 ] = sin_psi;
    R_psi->elements[ 1 ][ 0 ] = -sin_psi;
    R_psi->elements[ 1 ][ 1 ] = cos_psi;

    /* *m_out = R_psi * R_theta * R_phi * R_theta^T * R_psi^T */
    multiply_matrices(target_mpp, R_psi, R_theta);
    multiply_matrices(target_mpp, *target_mpp, R_phi);
    get_matrix_transpose(&R_theta, R_theta);
    get_matrix_transpose(&R_psi, R_psi);
    multiply_matrices(target_mpp, *target_mpp, R_theta);
    multiply_matrices(target_mpp, *target_mpp, R_psi);

    free_matrix(R_phi);
    free_matrix(R_theta);
    free_matrix(R_psi);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_2d_homo_rotation_matrix
 *
 * Creates a 2D rotation matrix to rotate in 2D around the origin. In homogeneous
 * coordinates
 *
 * Phi is the rotation angle, use a negative angle for clockwise
 * rotation.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_2d_homo_rotation_matrix
(
    Matrix  ** target_mpp,
    double     phi
)
{
    double cos_phi = cos(phi);
    double sin_phi = sin(phi);

    get_identity_matrix(target_mpp, 3);
    (*target_mpp)->elements[ 0 ][ 0 ] = cos_phi;
    (*target_mpp)->elements[ 0 ][ 1 ] = sin_phi;
    (*target_mpp)->elements[ 1 ][ 0 ] = -sin_phi;
    (*target_mpp)->elements[ 1 ][ 1 ] = cos_phi;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                        get_3d_scaling_matrix_1
 *
 * Creates a 3D scaling matrix
 *
 * The input vector is the vector to rotate around. Phi
 * is the rotation angle, use a negative angle for clockwise
 * rotation.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_scaling_matrix_1
(
    Matrix**      target_mpp,
    const Vector* v
)
{
    if (v->length != 3)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return ERROR;
    }

    return get_3d_scaling_matrix_2(target_mpp,
            v->elements[0], v->elements[1], v->elements[2]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_3d_scaling_matrix_2
 *
 * Creates a 3D scaling matrix.
 *
 * X, Y, and Z are the amount of scaling in each dimension.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_scaling_matrix_2
(
    Matrix** target_mpp,
    double     x,
    double     y,
    double     z
)
{
    get_zero_matrix(target_mpp, 3, 3);
    (*target_mpp)->elements[ 0 ][ 0 ] = x;
    (*target_mpp)->elements[ 1 ][ 1 ] = y;
    (*target_mpp)->elements[ 2 ][ 2 ] = z;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_3d_homo_scaling_matrix_1
 *
 * Creates a 3D scaling matrix in homogeneous coordinates
 *
 * The first, second and third element of the input vector, are, respectively,
 * the amount of scaling along the x, y and z axis
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_homo_scaling_matrix_1
(
    Matrix**      target_mpp,
    const Vector* v
)
{
    if (v->length < 3)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return ERROR;
    }

    return get_3d_homo_scaling_matrix_2(target_mpp,
            v->elements[0], v->elements[1], v->elements[2]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_3d_homo_scaling_matrix_2
 *
 * Creates a 3D scaling matrix in homogeneous coordinates
 *
 * X, Y, and Z are the amount of scaling along each dimension.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_homo_scaling_matrix_2
(
    Matrix**   target_mpp,
    double     x,
    double     y,
    double     z
)
{
    get_identity_matrix(target_mpp, 4);
    (*target_mpp)->elements[ 0 ][ 0 ] = x;
    (*target_mpp)->elements[ 1 ][ 1 ] = y;
    (*target_mpp)->elements[ 2 ][ 2 ] = z;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                        get_3d_homo_translation_matrix_1
 *
 * Creates a 3D translation matrix in homogeneous coordinates
 *
 * The first, second and third element of the input vector, are, respectively,
 * the amount of translation along the x, y and z axis
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_homo_translation_matrix_1
(
    Matrix**      target_mpp,
    const Vector* v
)
{
    if (v->length < 3)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return ERROR;
    }

    return get_3d_homo_translation_matrix_2(target_mpp,
            v->elements[0], v->elements[1], v->elements[2]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                        get_3d_homo_scaling_matrix_2
 *
 * Creates a 3D translation matrix in homogeneous coordinates
 *
 * X, Y, and Z are the amount of translation along each dimension.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
int get_3d_homo_translation_matrix_2
(
    Matrix**   target_mpp,
    double     x,
    double     y,
    double     z
)
{
    get_identity_matrix(target_mpp, 4);
    (*target_mpp)->elements[ 0 ][ 3 ] = x;
    (*target_mpp)->elements[ 1 ][ 3 ] = y;
    (*target_mpp)->elements[ 2 ][ 3 ] = z;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

