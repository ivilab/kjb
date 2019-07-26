
/* $Id: m_mat_vector.c 21712 2017-08-20 18:21:41Z kobus $ */

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

#include "l/l_incl.h"
#include "m/m_mat_vector.h"
#include "m/m_mat_basic.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                       get_target_matrix_vector
 *
 * Gets a target matrix vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of matrix vectors. If *target_mvpp is
 * NULL, then this routine creates the matrix vector. If it is not null, and it
 * is the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_matrix_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_target_matrix_vector(Matrix_vector** mvpp, int length)
{

    if (    (*mvpp != NULL)
         && ((*mvpp)->length == length)
       )
    {
        return NO_ERROR;
    }

    free_matrix_vector(*mvpp);
    NRE(*mvpp = create_matrix_vector(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should be either static or deleted`. */

/* =============================================================================
 *                       create_matrix_vector
 *
 * Creates a matrix vector
 *
 * This routine creates a matrix array of size "length" which is a type for a
 * list of matrices.  All matrix pointers are set to NULL.
 *
 * This routine sets all the pointers to NULL.
 *
 * The routine free_matrix_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

#endif

Matrix_vector* create_matrix_vector(int length)
{
    Matrix_vector* mvp;
    int           i;


    NRN(mvp = TYPE_MALLOC(Matrix_vector));
    mvp->length = length;
    NRN(mvp->elements = N_TYPE_MALLOC(Matrix*, length));

    for (i=0; i<length; i++)
    {
        mvp->elements[ i ] = NULL;
    }

    return mvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_matrix_vector
 *
 * Frees the storage in a matrix vector
 *
 * This routine frees the storage in a matrix array.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_matrix_vector(Matrix_vector* mvp)
{
    int count, i;
    Matrix** mp_array_pos;


    if (mvp == NULL) return;

    mp_array_pos = mvp->elements;
    count = mvp->length;

    for (i=0; i<count; i++)
    {
        free_matrix(*mp_array_pos);
        mp_array_pos++;
    }

    kjb_free(mvp->elements);
    kjb_free(mvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        count_non_null_matrix_vector_matrices
 *
 * Returns the number of non-null matrix vector matrices
 *
 * This routine returns the number of non-null matrices in a matrix vector. If
 * the matrix vector itself is null, this routine return zero.
 *
 * Returns:
 *    The number of non-null matrix vector matrices.
 *
 * Index:
 *    matrix vectors
 *
 * -----------------------------------------------------------------------------
*/

int count_non_null_matrix_vector_matrices
(
    const Matrix_vector* in_mvp
)
{
    int count = 0;

    if (in_mvp != NULL)
    {
        int length = in_mvp->length;
        int i;

        for (i = 0; i < length ; i++)
        {
            if (in_mvp->elements[ i ] != NULL)
            {
                count++;
            }
        }
    }

    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         matrix_vectors_are_comparable
 *
 * Returns whether two matrix vectors have the same dimensions
 *
 * This routine returns whether two matrix vectors have the same dimensions. It
 * returns TRUE if both arguments are NULL, or if they are matrix vectors of the
 * same length with each paor of corresponding component matrices are of the
 * same dimension or simultaneously NULL.
 *
 * Returns:
 *    TRUE if the matrix vectors are of the same length and all the sub matrices
 *    are the same dimensions (or are similtaneously NULL).
 *
 * Index:
 *    matrix vectors, error handling, debugging
 *
 * -----------------------------------------------------------------------------
*/

int matrix_vectors_are_comparable
(
    const Matrix_vector* in_mvp,
    const Matrix_vector* out_mvp
)
{
    int i, length;


    if ((in_mvp == NULL) && (out_mvp == NULL))
    {
        return TRUE;
    }

    if (    ((in_mvp == NULL) && (out_mvp != NULL))
         || ((out_mvp == NULL) && (in_mvp != NULL))
         || (in_mvp->length != out_mvp->length)
       )
    {
        return FALSE;
    }

    length = in_mvp->length;

    for (i = 0; i < length; i++)
    {
        if ((in_mvp->elements[ i ] == NULL) && (out_mvp->elements[ i ] == NULL))
        {
            continue;
        }
        else if ((in_mvp->elements[ i ] == NULL) && (out_mvp->elements[ i ] != NULL))
        {
            return FALSE;
        }
        else if ((in_mvp->elements[ i ] != NULL) && (out_mvp->elements[ i ] == NULL))
        {
            return FALSE;
        }
        else if (    (in_mvp->elements[ i ]->num_rows != out_mvp->elements[ i ]->num_rows)
                  || (in_mvp->elements[ i ]->num_cols != out_mvp->elements[ i ]->num_cols)
                )
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       is_matrix_vector_consistent
 *
 * Checks if matrix vector is 'consistent'
 *
 * This routine checks whether all matrices in a matrix vector are the same size.
 *
 * Index: arrays, matrices
 *
 * Returns:
 *    TRUE if the all the matrices in mvp are the same size, FALSE otherwise.
 *
 * -----------------------------------------------------------------------------
*/

int is_matrix_vector_consistent(const Matrix_vector* mvp)
{
    int i;

    if(mvp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 1; i < mvp->length; i++)
    {
        if(mvp->elements[i]->num_rows != mvp->elements[0]->num_rows || mvp->elements[i]->num_cols != mvp->elements[0]->num_cols)
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  average_matrices
 *
 * Averages (element-wise) a set of matrices
 *
 * This routine averages all the matrices in 'matrices' and puts the result in
 * avg_mat.
 *
 * Returns:
 *     On error, this routine returns ERROR, with an error message being set.
 *     On success it returns NO_ERROR.
 *
 * Index: matrices
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int average_matrices
(
    Matrix**             avg_mat,
    const Matrix_vector* matrices
)
{
    int i;

    if(matrices == NULL || avg_mat == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(!is_matrix_vector_consistent(matrices))
    {
        set_error("average_matrices: matrices must be of the same size.");
        return ERROR;
    }

    copy_matrix(avg_mat, matrices->elements[0]);

    for(i = 1; i < matrices->length; i++)
    {
        ow_add_matrices(*avg_mat, matrices->elements[i]);
    }

    ow_divide_matrix_by_scalar(*avg_mat, matrices->length);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  std_dev_matrices
 *
 * Computes the element-wise standard deviation of a set of matrices
 *
 * This routine finds the standard deviation all the matrices in 'matrices' and puts
 * the result in std_dev_mat, in matrix form. If the matrix representing the
 * average of the images is available, it can be passed in avg_mat to save
 * some time; otherwise, pass NULL.
 *
 * Returns:
 *     On error, this routine returns ERROR, with an error message being set.
 *     On success it returns NO_ERROR.
 *
 * Index: matrices
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int std_dev_matrices
(
    Matrix**             std_dev_mat,
    const Matrix_vector* matrices,
    const Matrix*        avg_mat
)
{
    Matrix* avg_mat_loc = NULL;
    Matrix* temp_matrix = NULL;
    int i;

    if(matrices == NULL || std_dev_mat == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(!is_matrix_vector_consistent(matrices))
    {
        set_error("std_dev_matrices: matrices must be of the same size.");
        return ERROR;
    }

    if(avg_mat != NULL)
    {
        if(avg_mat->num_rows != matrices->elements[0]->num_rows || avg_mat->num_cols != matrices->elements[0]->num_cols)
        {
            set_error("std_dev_matrices: average matrix must be of the same dimensions as matrices.");
            return ERROR;
        }

        copy_matrix(&avg_mat_loc, avg_mat);
    }
    else
    {
        ERE(average_matrices(&avg_mat_loc, matrices));
    }

    get_initialized_matrix(std_dev_mat, avg_mat_loc->num_rows, avg_mat_loc->num_cols, 0.0);

    for(i = 0; i < matrices->length; i++)
    {
        copy_matrix(&temp_matrix, matrices->elements[i]);
        ow_subtract_matrices(temp_matrix, avg_mat_loc);
        ow_square_matrix_elements(temp_matrix);
        ow_add_matrices(*std_dev_mat, temp_matrix);
    }

    ow_divide_matrix_by_scalar(*std_dev_mat, matrices->length - 1);
    ow_sqrt_matrix_elements(*std_dev_mat);

    free_matrix(avg_mat_loc);
    free_matrix(temp_matrix);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       get_target_matrix_vector_vector
 *
 * Gets a target matrix vector vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of matrix vector vectors. If *target_mvvpp is
 * NULL, then this routine creates the object. If it is not null, and it
 * is the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_matrix_vector_vector should be used to dispose of the
 * storage once it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_target_matrix_vector_vector(Matrix_vector_vector** mvvpp, int length)
{

    if (    (*mvvpp != NULL)
         && ((*mvvpp)->length == length)
       )
    {
        return NO_ERROR;
    }

    free_matrix_vector_vector(*mvvpp);
    NRE(*mvvpp = create_matrix_vector_vector(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/* =============================================================================
 *                       create_matrix_vector_vector
 *
 * Creates a matrix vector vector
 *
 * This routine creates a vector of vectors of matrices array of size "length.
 * All matrix vector pointers are set to NULL.
 *
 * The routine free_matrix_vector_vector should be used to dispose of the
 * storage once it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

#endif

Matrix_vector_vector* create_matrix_vector_vector(int length)
{
    Matrix_vector_vector* mvvp;
    int           i;


    NRN(mvvp = TYPE_MALLOC(Matrix_vector_vector));
    mvvp->length = length;
    NRN(mvvp->elements = N_TYPE_MALLOC(Matrix_vector*, length));

    for (i=0; i<length; i++)
    {
        mvvp->elements[ i ] = NULL;
    }

    return mvvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_matrix_vector_vector
 *
 * Frees the storage in a matrix vector vector
 *
 * This routine frees the storage in a matrix vector vector object.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_matrix_vector_vector(Matrix_vector_vector* mvvp)
{
    int count, i;
    Matrix_vector** mvpp;


    if (mvvp == NULL) return;

    mvpp = mvvp->elements;
    count = mvvp->length;

    for (i=0; i<count; i++)
    {
        free_matrix_vector(*mvpp);
        mvpp++;
    }

    kjb_free(mvvp->elements);
    kjb_free(mvvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Matrix_vector** create_matrix_vector_list(int count)
{
    Matrix_vector** matrix_vector_list;
    int            i;


    NRN(matrix_vector_list = N_TYPE_MALLOC(Matrix_vector*, count));

    for (i=0; i<count; i++)
    {
        matrix_vector_list[ i ] = NULL;
    }

    return matrix_vector_list;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_matrix_vector_list
(
    int             count,
    Matrix_vector** matrix_vector_list
)
{
    int i;


    if (matrix_vector_list == NULL) return;

    for (i=0; i<count; i++)
    {
        free_matrix_vector(matrix_vector_list[ i ]);
    }

    kjb_free(matrix_vector_list);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      interleave_matrix_rows
 *
 * Interleaves the rows of several matrices
 *
 * This routine takes a vector of matrices and interleaves the rows. The
 * matrices must be the same size or NULL. The matrix vector itself can be NULL,
 * and the result is then a NULL matrix.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int interleave_matrix_rows
(
    Matrix**             target_mpp,
    const Matrix_vector* source_mvp
)
{
    int i, j, m, count;
    int num_rows = NOT_SET;
    int num_cols = NOT_SET;
    int num_output_rows = 0;
    Matrix* target_mp;
    const Matrix* source_mp;
    int num_matrices = source_mvp->length;


    if (source_mvp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    for (i=0; i<num_matrices; i++)
    {
        source_mp = source_mvp->elements[ i ];

        if (source_mp != NULL)
        {
            if (num_rows == NOT_SET)
            {
                num_rows = source_mp->num_rows;
            }
            else if (source_mp->num_rows != num_rows)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }

            if (num_cols == NOT_SET)
            {
                num_cols = source_mp->num_cols;
            }
            else if (source_mp->num_cols != num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }

            num_output_rows += num_rows;
        }
    }

    if (num_output_rows == 0)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_target_matrix(target_mpp, num_output_rows, num_cols));
    target_mp = *target_mpp;

    count = 0;

    for (i = 0; i < num_rows; i++)
    {
        for (m=0; m<num_matrices; m++)
        {
            source_mp = source_mvp->elements[ m ];

            if (source_mp != NULL)
            {
                for (j = 0; j < num_cols; j++)
                {
                    target_mp->elements[ count ][ j ] = source_mp->elements[ i ][ j ];
                }
                count++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      interleave_matrix_cols
 *
 * Interleaves the rows of several matrices
 *
 * This routine takes a vector of matrices and interleaves the columns. The
 * matrices must be the same size or NULL. The matrix vector itself can be NULL,
 * and the result is then a NULL matrix.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int interleave_matrix_cols
(
    Matrix**             target_mpp,
    const Matrix_vector* source_mvp
)
{
    int i, j, m, count;
    int num_rows = NOT_SET;
    int num_cols = NOT_SET;
    int num_output_cols = 0;
    Matrix* target_mp;
    const Matrix* source_mp;
    int num_matrices = source_mvp->length;


    if (source_mvp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    for (i=0; i<num_matrices; i++)
    {
        source_mp = source_mvp->elements[ i ];

        if (source_mp != NULL)
        {
            if (num_rows == NOT_SET)
            {
                num_rows = source_mp->num_rows;
            }
            else if (source_mp->num_rows != num_rows)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }

            if (num_cols == NOT_SET)
            {
                num_cols = source_mp->num_cols;
            }
            else if (source_mp->num_cols != num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }

            num_output_cols += num_cols;
        }
    }

    if (num_output_cols == 0)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_target_matrix(target_mpp, num_rows, num_output_cols));
    target_mp = *target_mpp;

    count = 0;

    for (j = 0; j < num_cols; j++)
    {
        for (m=0; m<num_matrices; m++)
        {
            source_mp = source_mvp->elements[ m ];

            if (source_mp != NULL)
            {
                for (i = 0; i < num_rows; i++)
                {
                    target_mp->elements[ i ][ count ] = source_mp->elements[ i ][ j ];
                }
                count++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      concat_matrices_vertically
 *
 * Concatenates matrices vertically
 *
 * This routine takes an array of matrices and forms one matrix consisting of
 * those matrices stacked on top of each other.
 *
 * The first argument is the address of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * The matrix array may contain any number of NULL matrices. If there are only
 * NULL matrices, or if num_matrices is zero, then the target matrix is freed
 * and set to NULL.  All non-null matrices must have the same number of columns.
 *
 * Note:
 *     More often then not, one wants to concatentate the matrices in a matrix
 *     vector. The routine get_matrix_from_matrix_vector() is much more
 *     convenient for that. 
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Related:
 *     get_matrix_from_matrix_vector
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int concat_matrices_vertically
(
    Matrix**      mpp,
    int           num_matrices,
    const Matrix* matrix_list[] 
)
{
    int i, j, k, count;
    int num_rows = 0;
    int num_cols = NOT_SET;
    Matrix* mp;
    Matrix* temp_mp = NULL;


    if (num_matrices <= 0)
    {
        free_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }

    for (i=0; i<num_matrices; i++)
    {
        if (matrix_list[ i ] != NULL)
        {
            if(matrix_list[ i ] == *mpp && NULL == temp_mp)
            {
                copy_matrix(&temp_mp, *mpp);
                matrix_list[ i ] = temp_mp;
            }

            if (num_cols == NOT_SET)
            {
                num_cols = (matrix_list[ i ])->num_cols;
            }
            else if ((matrix_list[ i ])->num_cols != num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
            num_rows += ((matrix_list[ i ])->num_rows);
        }
    }

    if (num_rows == 0)
    {
        free_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }


    ERE(get_target_matrix(mpp, num_rows, num_cols));
    mp = *mpp;

    count = 0;

    while (count < num_rows)
    {
        for (k=0; k<num_matrices; k++)
        {
            if (matrix_list[ k ] != NULL)
            {
                for (i=0; i<(matrix_list[ k ])->num_rows; i++)
                {
                    for (j=0; j<num_cols; j++)
                    {
                        mp->elements[ count ][ j ] =
                                       (matrix_list[ k ])->elements[ i ][ j ];
                    }
                    count++;
                }
            }
        }
    }

    free_matrix(temp_mp);
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      concat_matrices_horizontally
 *
 * Concatenates matrices horizontally
 *
 * This routine takes an array of matrices and forms one matrix consisting of
 * those matrices arranged one after another.
 *
 * The first argument is the address of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * The matrix array may contain any number of NULL matrices. If there are only
 * NULL matrices, or if num_matrices is zero, then the target matrix is freed
 * and set to NULL.  All non-null matrices must have the same number of rows.
 *
 * Note:
 *     More often then not, one wants to concatentate the matrices in a matrix
 *     vector. The routine get_matrix_from_matrix_vector() is much more
 *     convenient for that. 
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Related:
 *     get_matrix_from_matrix_vector
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int concat_matrices_horizontally
(
    Matrix**      mpp,
    int           num_matrices,
    const Matrix* matrix_list[] 
)
{
    int i, j, k, count;
    int num_cols = 0;
    int num_rows = NOT_SET;
    Matrix* mp;
    Matrix* temp_mp = NULL;


    if (num_matrices <= 0)
    {
        free_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }

    for (i=0; i<num_matrices; ++i)
    {
        if (matrix_list[ i ] != NULL)
        {
            if (matrix_list[ i ] == *mpp && NULL == temp_mp)
            {
                copy_matrix(&temp_mp, *mpp);
                matrix_list[ i ] = temp_mp;
            }

            if (num_rows == NOT_SET)
            {
                num_rows = (matrix_list[ i ])->num_rows;
            }
            else if ((matrix_list[ i ])->num_rows != num_rows)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
            num_cols += ((matrix_list[ i ])->num_cols);
        }
    }

    if (num_cols == 0)
    {
        free_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }


    ERE(get_target_matrix(mpp, num_rows, num_cols));
    mp = *mpp;

    count = 0;

    while (count < num_cols)
    {
        for (k=0; k<num_matrices; ++k)
        {
            if (matrix_list[ k ] != NULL)
            {
                for (j=0; j < matrix_list[ k ] -> num_cols; ++j)
                {
                    for (i=0; i<num_rows; ++i)
                    {
                        mp->elements[ i ][ count ] =
                                       (matrix_list[ k ])->elements[ i ][ j ];
                    }
                    ++count;
                }
            }
        }
    }

    free_matrix(temp_mp);
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_matrix_from_matrix_vector
 *
 * Concatenates the matrices in a matrix vector
 *
 * This routine takes the matrices in a matrix vector and forms one matrix
 * consisting of those matrices stacked on top of each other.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * The matrix array may contain any number of NULL matrices. If there are only
 * NULL matrices, or if num_matrices is zero, then the target matrix is freed
 * and set to NULL.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_from_matrix_vector
(
    Matrix**       mpp,
    const Matrix_vector* mvp
)
{
#if 0 /* HOW_IT_WAS_APRIL_18_2007 */
    int i;
    int num_rows = 0;
    int num_cols = NOT_SET;
    Matrix* mp;


    if (mvp == NULL)
    {
        free_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }

    for (i=0; i<mvp->length; i++)
    {
        if (mvp->elements[ i ] != NULL)
        {
            if (num_cols == NOT_SET)
            {
                num_cols = (mvp->elements[ i ])->num_cols;
            }
            else if ((mvp->elements[ i ])->num_cols != num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
            num_rows += ((mvp->elements[ i ])->num_rows);
        }
    }

    if (num_rows == 0)
    {
        free_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_target_matrix(mpp, num_rows, num_cols));
    mp = *mpp;

    num_rows = 0;

    for (i=0; i<mvp->length; i++)
    {
        if (mvp->elements[ i ] != NULL)
        {
            ERE(ow_copy_matrix(mp, num_rows, 0, mvp->elements[ i ]));
            num_rows += ((mvp->elements[ i ])->num_rows);
        }
    }

    return NO_ERROR;
#else
    return get_matrix_from_matrix_vector_with_col_selection(mpp, mvp, NULL);
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                 get_matrix_from_matrix_vector_with_col_selection
 *
 * Concatenates selected columns of matrices from a matrix vector
 *
 * This routine takes the matrices in a matrix vector and forms one matrix
 * consisting of those matrices stacked on top of each other.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * The matrix array may contain any number of NULL matrices. If there are only
 * NULL matrices, or if num_matrices is zero, then the target matrix is freed
 * and set to NULL.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_from_matrix_vector_with_col_selection
(
    Matrix**             mpp,
    const Matrix_vector* mvp,
    const Int_vector*    selected_cols_vp 
)
{
    int i;
    int num_rows = 0;
    int num_cols = NOT_SET;
    Matrix* mp;
    int num_needed_cols = 0;


    if (mvp == NULL) 
    {
        free_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }

    if (selected_cols_vp != NULL)
    {
        num_cols = selected_cols_vp->length; 

        if (num_cols <= 0)
        {
            set_bug("Non-positive number of columns in get_matrix_from_matrix_vector.");
            return ERROR;
        }
    }

    for (i=0; i<mvp->length; i++)
    {
        if (mvp->elements[ i ] != NULL)
        {
            if (num_cols == NOT_SET)
            {
                num_cols = (mvp->elements[ i ])->num_cols;
            }
            else if ((mvp->elements[ i ])->num_cols != num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
            num_rows += ((mvp->elements[ i ])->num_rows);
        }
    }

    if (selected_cols_vp != NULL)
    {
        for (i = 0; i < num_cols; i++)
        {
            if (selected_cols_vp->elements[ i ] != 0)
            {
                num_needed_cols++; 
            }
        }
    }
    else 
    {
        num_needed_cols = num_cols;
    }

    if ((num_rows == 0) || (num_needed_cols == 0))
    {
        free_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_target_matrix(mpp, num_rows, num_needed_cols));
    mp = *mpp;

    num_rows = 0;

    for (i=0; i<mvp->length; i++)
    {
        if (mvp->elements[ i ] != NULL)
        {
            ERE(ow_copy_matrix_with_col_selection(mp, num_rows, 0, mvp->elements[ i ],
                                                  selected_cols_vp));
            num_rows += ((mvp->elements[ i ])->num_rows);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_matrix_vector_from_matrix
 *
 * Constructs a matrix from a matrix vector
 *
 * This routine takes a matrix and puts each row as a 1 by N matrix in a matrix.
 *
 * The first argument is the adress of the target matrix vector. If the target
 * matrix vector is null, then a matrix vector of the appropriate size is
 * created. If the target matrix vector is the wrong size, it is resized.
 * Finally, if it is the right size, then the storage is recycled, as is.
 *
 * If the matrix mp is NULL, then the target matrix beomes NULL as well.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_vector_from_matrix
(
    Matrix_vector**  mvpp,
    const Matrix* mp
)
{
    int i;
    int num_cols = NOT_SET;
    int length;
    Matrix_vector* mvp;


    if (mp == NULL)
    {
        free_matrix_vector(*mvpp);
        *mvpp = NULL;
        return NO_ERROR;
    }

    length = mp->num_rows;
    num_cols = mp->num_cols;

    ERE(get_target_matrix_vector(mvpp, length));
    mvp = *mvpp;

    dbi(length);
    for (i=0; i < length; i++)
    {
        ERE(get_target_matrix(&(mvp->elements[ i ]), 1, num_cols));
        ERE(copy_matrix_row(mvp->elements[ i ], 0, mp, i));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_matrix_vector_from_matrix_2
 *
 * Constructs a matrix from a matrix vector
 *
 * This routine takes a matrix of N rows and puts each block_size rows into a
 * matrix_vector of N/block_size matrices, each one having block_size rows.
 * If there are rows left over (i.e., the number of rows in the matrix is not
 * divisible by block_size, then the extra rows are ignored.
 *
 * The first argument is the adress of the target matrix vector. If the target
 * matrix vector is null, then a matrix vector of the appropriate size is
 * created. If the target matrix vector is the wrong size, it is resized.
 * Finally, if it is the right size, then the storage is recycled, as is.
 *
 * If the matrix mp is NULL, then the target matrix beomes NULL as well.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_vector_from_matrix_2
(
    Matrix_vector**  mvpp,
    const Matrix* mp,
    int block_size
)
{
    int i;
    int num_cols = NOT_SET;
    int length;
    Matrix_vector* mvp;


    if (mp == NULL)
    {
        free_matrix_vector(*mvpp);
        *mvpp = NULL;
        return NO_ERROR;
    }

    length = mp->num_rows / block_size;
    num_cols = mp->num_cols;

    ERE(get_target_matrix_vector(mvpp, length));
    mvp = *mvpp;

    for (i=0; i < length; i++)
    {
        ERE(copy_matrix_block(&(mvp->elements[ i ]), mp, i*block_size, 0,
                              block_size, num_cols));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       allocate_2D_mp_array
 *
 * Allocates a 2D array of matrix pointers
 *
 * This routine returns a pointer P to a two-dimensional array of matrix
 * pointers. P can be de-referenced by P[row][col] to obtain the storage of a
 * Matrix*. P[0] points to a contiguous area of memory which contains the
 * entire array. P[1] is a short-cut pointer to the first element of the second
 * row, and so on. Thus the array can be accessed sequentually starting at
 * P[0], or explicitly by row and column. (Note that this is not the common way
 * of setting up a two-dimensional array--see below).
 *
 * This routine sets all the pointers to NULL.
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_mp_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_mp_array should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Note:
 *    Naming convention--this routine is called "allocate_2D_mp_array" rather
 *    than "create_2D_mp_array" or "create_matrix_matrix" because we are not
 *    creating an abstract data type, but rather just allocating raw storage.
 *    For many puposes this routine is a hack waiting for a
 *    create_matrix_matrix routine to be written, but it has uses outside of
 *    this as well.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a two-dimensional array in "C" where each row is allocated
 *    separately. Here the storage area is contiguous. This allows for certain
 *    operations to be done quickly, but note the following IMPORTANT point:
 *    num_rows cannot be swapped by simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

Matrix*** debug_allocate_2D_mp_array(int num_rows, int num_cols,
                                     const char* file_name, int line_number)
{
    Matrix***   array;
    Matrix***   array_pos;
    Matrix**    col_ptr;
    int         i, j;
    Malloc_size num_bytes;


    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(Matrix**);
    NRN(array = (Matrix ***)debug_kjb_malloc(num_bytes, file_name, line_number));

    num_bytes = num_rows * num_cols * sizeof(Matrix*);
    col_ptr = (Matrix**)debug_kjb_malloc(num_bytes, file_name, line_number);

    if (col_ptr == NULL)
    {
        kjb_free(array);
        return NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    for(i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            array[ i ][ j ] = NULL;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Matrix*** allocate_2D_mp_array(int num_rows, int num_cols)
{
    Matrix*** array;
    Matrix*** array_pos;
    Matrix**  col_ptr;
    int       i, j;


    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(Matrix**, num_rows));

    col_ptr = N_TYPE_MALLOC(Matrix*, num_rows * num_cols);

    if (col_ptr == NULL)
    {
        kjb_free(array);
        return NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    for(i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            array[ i ][ j ] = NULL;
        }
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_2D_mp_array
 *
 * Frees an array from allocate_2D_mp_array
 *
 * This routine frees the storage obtained from allocate_2D_mp_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_mp_array(Matrix*** array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_2D_mp_array_and_matrices
 *
 * Frees a 2D array of matrices
 *
 * This routine frees the storage obtained from allocate_2D_mp_array and the
 * matrices pointed to by the pointers. If the argument is NULL, then this
 * routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_mp_array_and_matrices
(
    Matrix*** array,
    int       num_rows,
    int       num_cols
)
{
    int i, j;


    if (array == NULL) return;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            free_matrix(array[ i ][ j ]);
        }
    }

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif
