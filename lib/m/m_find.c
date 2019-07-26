
/* $Id: m_find.c 21524 2017-07-22 15:52:39Z kobus $ */

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

#include "m/m_find.h"
#include "m/m_vec_basic.h"
#include "m/m_mat_basic.h"

/* Some C++ compilers need this. (not g++). */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             find_in_vector
 *
 * Finds elements in a vector.
 *
 * This routine searches the vector vp for elements which return 1 on the given
 * predicate. That is, index i is inserted in *indices if and only if 
 * predicate(vp, i, ...) returns 1. *indices is an integer vector that holds 
 * all indices which fulfill the mentioned condition. *indices can then be
 * used as input to any of the copy functions.
 *
 * If *indices is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int find_in_vector(Int_vector** indices, const Vector* vp, int (*predicate)(const Vector*, int, void*), void* params)
{
    int count = 0;
    int i;

    if(vp == NULL)
    {
        free_int_vector(*indices);
        *indices = NULL;
        return NO_ERROR;
    }

    if(predicate == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 0; i < vp->length; i++)
    {
        if((*predicate)(vp, i, params))
        {
            count++;
        }
    }

    if(count == 0)
    {
        free_int_vector(*indices);
        *indices = NULL;
        return NO_ERROR;
    }

    get_target_int_vector(indices, count);

    count = 0;
    for(i = 0; i < vp->length; i++)
    {
        if((*predicate)(vp, i, params))
        {
            (*indices)->elements[count] = i;
            count++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             find_in_matrix_by_rows
 *
 * Finds rows in a matrix.
 *
 * This routine searches the matrix mp for rows which return 1 on the given
 * predicate. That is, row index i is inserted in *indices if and only if 
 * predicate(mp, i, ...) returns 1. *indices is an integer vector that holds 
 * all row indices which fulfill the mentioned condition. *indices can then be
 * used as input to any of the copy functions.
 *
 * If *indices is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int find_in_matrix_by_rows(Int_vector** indices, const Matrix* mp, int (*predicate)(const Matrix*, int, void*), void* params)
{
    int count = 0;
    int i;

    if(mp == NULL)
    {
        free_int_vector(*indices);
        *indices = NULL;
        return NO_ERROR;
    }

    if(predicate == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 0; i < mp->num_rows; i++)
    {
        if((*predicate)(mp, i, params))
        {
            count++;
        }
    }

    if(count == 0)
    {
        free_int_vector(*indices);
        *indices = NULL;
        return NO_ERROR;
    }

    get_target_int_vector(indices, count);

    count = 0;
    for(i = 0; i < mp->num_rows; i++)
    {
        if((*predicate)(mp, i, params))
        {
            (*indices)->elements[count] = i;
            count++;
        }
    }
    return NO_ERROR;
}
 
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             find_in_matrix_by_cols
 *
 * Finds columns in a matrix.
 *
 * This routine searches the matrix mp for columns which return 1 on the given
 * predicate. That is, colunm index i is inserted in *indices if and only if 
 * predicate(mp, i, ...) returns 1. *indices is an integer vector that holds 
 * all column indices which fulfill the mentioned condition. *indices can then
 * be used as input to any of the copy functions.
 *
 * If *indices is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int find_in_matrix_by_cols(Int_vector** indices, const Matrix* mp, int (*predicate)(const Matrix*, int, void*), void* params)
{
    int count = 0;
    int i;

    if(mp == NULL)
    {
        free_int_vector(*indices);
        *indices = NULL;
        return NO_ERROR;
    }

    if(predicate == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 0; i < mp->num_cols; i++)
    {
        if((*predicate)(mp, i, params))
        {
            count++;
        }
    }

    if(count == 0)
    {
        free_int_vector(*indices);
        *indices = NULL;
        return NO_ERROR;
    }

    get_target_int_vector(indices, count);

    count = 0;
    for(i = 0; i < mp->num_cols; i++)
    {
        if((*predicate)(mp, i, params))
        {
            (*indices)->elements[count] = i;
            count++;
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  find_in_matrix
 *
 * Finds elements in a matrix.
 *
 * This routine searches the matrix mp for elements which return 1 on the given
 * predicate. That is, *elems at i,j is 1 if and only if predicate(mp, i, j, ...) 
 * returns 1. *elems is an binary matrix whose elements are 1 whenever the said
 * condition is met. *elems can be used with some of the copy with selection
 * routines.
 *
 * If *indices is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int find_in_matrix(Int_matrix** elems, const Matrix* mp, int (*predicate)(const Matrix*, int, int, void*), void* params)
{
    int i, j;

    if(mp == NULL)
    {
        free_int_matrix(*elems);
        *elems = NULL;
        return NO_ERROR;
    }

    if(predicate == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    get_initialized_int_matrix(elems, mp->num_rows, mp->num_cols, 0);

    for(i = 0; i < mp->num_rows; i++)
    {
        for(j = 0; j < mp->num_cols; j++)
        {
            if((*predicate)(mp, i, j, params))
            {
                (*elems)->elements[i][j] = 1;
            }
        }
    }
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             find_in_matrix_as_vector
 *
 * Finds elements in matrix (treated as vector)
 *
 * This routine searches the matrix mp for elements which return 1 on the given
 * predicate. That is, index k is inserted in *indices if and only if 
 * predicate(mp, i, j ...) returns 1, where k = j*mp->num_rows + i. In other
 * words, matrix mp is treated as the vector formed by stacking its columns.
 * *indices is an integer vector that holds element indices which fulfill
 * the mentioned condition. *indices can then be used as input to the
 * appropriate copy function.
 *
 * If *indices is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int find_in_matrix_as_vector(Int_vector** indices, const Matrix* mp, int(*predicate)(const Matrix*, int, int, void*), void* params)
{
    int count = 0;
    int i, j;

    if(mp == NULL)
    {
        free_int_vector(*indices);
        *indices = NULL;
        return NO_ERROR;
    }

    if(predicate == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 0; i < mp->num_cols; i++)
    {
        for(j = 0; j < mp->num_rows; j++)
        {
            if(predicate(mp, j, i, params))
            {
                count++;
            }
        }
    }

    if(count == 0)
    {
        free_int_vector(*indices);
        *indices = NULL;
        return NO_ERROR;
    }

    get_target_int_vector(indices, count);

    count = 0;
    for(i = 0; i < mp->num_cols; i++)
    {
        for(j = 0; j < mp->num_rows; j++)
        {
            if(predicate(mp, j, i, params))
            {
                (*indices)->elements[count] = i * mp->num_rows + j;
                count++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             copy_vector_with_selection
 *
 * Copies selected elements of a vector.
 *
 * This routine copies the elements (specified in the integer vector elems) of
 * vector source_vp into vector *target_vpp. Naturally, every element of elems
 * must be between 0 and source_vp->length - 1, inclusive. If elems is NULL,
 * the whole vector is copied (via copy_vector).
 *
 * If *target_vpp is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int copy_vector_with_selection(Vector** target_vpp, const Vector* source_vp, const Int_vector* elems)
{
    int i;

    if(elems == NULL)
    {
        return copy_vector(target_vpp, source_vp);
    }

    if(source_vp == NULL)
    {
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    for(i = 0; i < elems->length; i++)
    {
        if(elems->elements[i] >= source_vp->length)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    get_target_vector(target_vpp, elems->length);

    for(i = 0; i < elems->length; i++)
    {
        (*target_vpp)->elements[i] = source_vp->elements[elems->elements[i]];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             copy_matrix_with_selection
 *
 * Copies selected elements of a matrix.
 *
 * This routine copies the elements (specified in the integer vectors rows and
 * cols) of matrix source_mp into vector *matrix_mpp. Naturally, every element
 * of rows and cols must be (inclusively) between 0 and source_mp->num_rows - 1
 * and source_mp->num_cols - 1, respectively. If rows (cols) is NULL, then all
 * the rows (columns) are copied. As a special case of this, if both rows and
 * cols are NULL, then copy_matrix is invoked.
 *
 * If *target_vpp is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int copy_matrix_with_selection(Matrix** target_mpp, const Matrix* source_mp, const Int_vector* rows, const Int_vector* cols)
{
    int i, j;

    if(source_mp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    if(rows != NULL)
    {
        for(i = 0; i < rows->length; i++)
        {
            if(rows->elements[i] >= source_mp->num_rows)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
        }
    }

    if(cols != NULL)
    {
        for(i = 0; i < cols->length; i++)
        {
            if(cols->elements[i] >= source_mp->num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
        }
    }

    if(rows != NULL && cols != NULL)
    {
        get_target_matrix(target_mpp, rows->length, cols->length);
        for(i = 0; i < rows->length; i++)
        {
            for(j = 0; j < cols->length; j++)
            {
                (*target_mpp)->elements[i][j] = source_mp->elements[rows->elements[i]][cols->elements[j]];
            }
        }
    }
    else if(rows == NULL && cols == NULL)
    {
        copy_matrix(target_mpp, source_mp);
    }
    else if(rows == NULL)
    {
        get_target_matrix(target_mpp, source_mp->num_rows, cols->length);
        for(i = 0; i < (*target_mpp)->num_rows; i++)
        {
            for(j = 0; j < cols->length; j++)
            {
                (*target_mpp)->elements[i][j] = source_mp->elements[i][cols->elements[j]];
            }
        }
    }
    else if(cols == NULL)
    {
        get_target_matrix(target_mpp, rows->length, source_mp->num_cols);
        for(i = 0; i < rows->length; i++)
        {
            for(j = 0; j < (*target_mpp)->num_cols; j++)
            {
                (*target_mpp)->elements[i][j] = source_mp->elements[rows->elements[i]][j];
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             copy_matrix_with_selection_2
 *
 * Copies selected elements of a matrix.
 *
 * This routine copies the elements (specified in the binary vectors rows and
 * cols) of matrix source_mp into vector *matrix_mpp. A row (column) is copied
 * if and only if the corresponding element of rows (cols) is non-zero. This
 * means that rows->length must be equal to source_mp->num_rows. The analogous 
 * statement must hold for cols->length. If rows (cols) is NULL, then all
 * the rows (columns) are copied. As a special case of this, if both rows and
 * cols are NULL, then copy_matrix is invoked.
 *
 * If *target_vpp is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int copy_matrix_with_selection_2(Matrix** target_mpp, const Matrix* source_mp, const Int_vector* rows, const Int_vector* cols)
{
    int num_rows;
    int num_cols;
    int i, j;
    int row, col;

    if(source_mp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    if(rows != NULL)
    {
        if(rows->length != source_mp->num_rows)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if(cols != NULL)
    {
        if(cols->length != source_mp->num_cols)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if(rows != NULL && cols != NULL)
    {

        num_rows = sum_int_vector_elements(rows);
        num_cols = sum_int_vector_elements(cols);

        get_target_matrix(target_mpp, num_rows, num_cols);
        row = 0;
        for(i = 0; i < rows->length; i++)
        {
            if(rows->elements[i] != 0)
            {
                col = 0;
                for(j = 0; j < cols->length; j++)
                {
                    if(cols->elements[j] != 0)
                    {
                        (*target_mpp)->elements[row][col] = source_mp->elements[i][j];
                        col++;
                    }
                }
                row++;
            }
        }
    }
    else if(rows == NULL && cols == NULL)
    {
        copy_matrix(target_mpp, source_mp);
    }
    else if(rows == NULL)
    {
        num_cols = sum_int_vector_elements(cols);

        get_target_matrix(target_mpp, source_mp->num_rows, num_cols);
        for(i = 0; i < (*target_mpp)->num_rows; i++)
        {
            col = 0;
            for(j = 0; j < cols->length; j++)
            {
                if(cols->elements[j] != 0)
                {
                    (*target_mpp)->elements[i][col] = source_mp->elements[i][j];
                    col++;
                }
            }
        }
    }
    else if(cols == NULL)
    {
        num_rows = sum_int_vector_elements(rows);

        get_target_matrix(target_mpp, num_rows, source_mp->num_cols);
        row = 0;
        for(i = 0; i < rows->length; i++)
        {
            if(rows->elements[i] != 0)
            {
                for(j = 0; j < (*target_mpp)->num_cols; j++)
                {
                    (*target_mpp)->elements[row][j] = source_mp->elements[i][j];
                }
                row++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      get_matrix_as_vector_with_selection
 *
 * Copies selected elements of a matrix (treated as a vector).
 *
 * This routine copies the elements (specified in the integer vector elems) of
 * matrix source_mp into vector *target_vpp. The Matrix is treated as a vector,
 * where the columns are concatenated. Naturally, every element of elems
 * must be between 0 and (source_mp->num_rows - 1)*(source_mp->num_cols - 1),
 * inclusive. If elems is NULL, the whole matrix is copied into *target_vpp.
 *
 * If *target_vpp is NULL, it is created; if it is the wrong size, it is resized;
 * finally, if it is the right size, the storage is recycled, as is.
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int get_matrix_as_vector_with_selection(Vector** target_vpp, const Matrix* source_mp, const Int_vector* elems)
{
    int i, j;
    int count = 0;
    int r, c;

    if (source_mp == NULL)
    {
        free_vector(*target_vpp);
        *target_vpp = NULL;
        return NO_ERROR;
    }

    for(i = 0; i < elems->length; i++)
    {
        if(elems->elements[i] >= source_mp->num_rows * source_mp->num_cols)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (elems == NULL)
    {
        get_target_vector(target_vpp, source_mp->num_rows * source_mp->num_cols);

        for(i = 0; i < source_mp->num_cols; i++)
        {
            for(j = 0; j < source_mp->num_rows; j++)
            {
                (*target_vpp)->elements[count] = source_mp->elements[j][i];
                count++;
            }
        }
    }
    else
    {
        get_target_vector(target_vpp, elems->length);
        for(i = 0; i < elems->length; i++)
        {
            c = elems->elements[i]/ source_mp->num_rows;
            r = elems->elements[i] - (c * source_mp->num_rows);
            (*target_vpp)->elements[i] = source_mp->elements[r][c];
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              is_element_zero
 *
 * Tests whether an element of a vector is zero.
 *
 * This routine is a predicate for the vector find functions. It returns 1 if 
 * vp->elements[elem] is zero.
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_zero(const Vector* vp, int elem, __attribute__((unused)) void*  dummy_params)
{
    if(elem >= vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return vp->elements[elem] == 0;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              is_element_nonzero
 *
 * Tests whether an element of a vector is non-zero.
 *
 * This routine is a predicate for the vector find functions. It returns 1 if 
 * vp->elements[elem] is non-zero.
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_nonzero(const Vector* vp, int elem,  __attribute__ ((unused)) void* dummy_params)
{
    if(elem >= vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return vp->elements[elem] != 0;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              is_element_nan
 *
 * Tests whether an element of a vector is NaN
 *
 * This routine is a predicate for the vector find functions. It returns 1 if 
 * vp->elements[elem] is NaN.
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_nan(const Vector* vp, int elem, __attribute__ ((unused))  void* dummy_params)
{
    if(elem >= vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return isnand(vp->elements[elem]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              is_element_nonnan
 *
 * Tests whether an element of a vector is not NaN.
 *
 * This routine is a predicate for the vector find functions. It returns 1 if 
 * vp->elements[elem] is not NaN.
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_nonnan(const Vector* vp, int elem,  __attribute__ ((unused))  void*  dummy_params)
{
    if(elem >= vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return !isnand(vp->elements[elem]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_element_equal_to
 *
 * Tests whether an element of a vector is equal to a given value.
 *
 * This routine is a predicate for the vector find functions. It returns 1 if 
 * vp->elements[elem] is equal to *params (cast to a double).
 *
 * Kobus adds: The test is for exact equality. 
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_equal_to(const Vector* vp, int elem, void* params)
{
    if(elem >= vp->length || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return vp->elements[elem] == *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_element_different_from
 *
 * Tests whether an element of a vector is different from a given value.
 *
 * This routine is a predicate for the vector find functions. It returns 1 if 
 * vp->elements[elem] is different from *params (cast to a double).
 *
 * Kobus adds: The test is for exact equality. 
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_different_from(const Vector* vp, int elem, void* params)
{
    if(elem >= vp->length || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return vp->elements[elem] != *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_element_greater_than
 *
 * Tests whether an element of a vector is greater than a given value.
 *
 * This routine is a predicate for the vector find functions. It returns 1 if 
 * vp->elements[elem] is greater than *params (cast to a double).
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_greater_than(const Vector* vp, int elem, void* params)
{
    if(elem >= vp->length || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return vp->elements[elem] > *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_element_less_than
 *
 * Tests whether an element of a vector is less than a given value.
 *
 * This routine is a predicate for the vector find functions. It returns 1 if 
 * vp->elements[elem] is less than *params (cast to a double).
 *
 * Index: vectors
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_element_less_than(const Vector* vp, int elem, void* params)
{
    if(elem >= vp->length || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return vp->elements[elem] < *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_row_sum_equal_to
 *
 * Tests whether the elements of a row of a matrix sum to a certain value.
 *
 * This routine is a predicate for the find_in_matrix_by_rows routine. It
 * returns 1 if the elements of row 'row' sum to *params (when cast to a double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_row_sum_equal_to(const Matrix* mp, int row, void* params)
{
    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return sum_matrix_row_elements(mp, row) == *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_row_sum_different_from
 *
 * Tests whether the elements of a row of a matrix do not sum to a certain
 * value.
 *
 * This routine is a predicate for the find_in_matrix_by_rows routine. It
 * returns 1 if the elements of row 'row' do not sum to *params (when cast to
 * a double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_row_sum_different_from(const Matrix* mp, int row, void* params)
{
    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return sum_matrix_row_elements(mp, row) != *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_column_sum_equal_to
 *
 * Tests whether the elements of a column of a matrix sum to a certain
 * value.
 *
 * This routine is a predicate for the find_in_matrix_by_cols routine. It
 * returns 1 if * the elements of column 'col' sums to *params (when cast to a
 * double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_column_sum_equal_to(const Matrix* mp, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return sum_matrix_col_elements(mp, col) == *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      is_column_sum_different_from
 *
 * Tests whether the elements of a column of a matrix do not sum to a certain
 * value.
 *
 * This routine is a predicate for the find_in_matrix_by_cols routine. It
 * returns 1 if the elements of column 'col' do not sum to *params (when cast
 * to a double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_column_sum_different_from(const Matrix* mp, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return sum_matrix_col_elements(mp, col) != *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_row_sum_less_than
 *
 * Tests whether the sum of the elements of a row of a matrix is less than a
 * certain value.
 *
 * This routine is a predicate for find_in_matrix_by_rows routine. It returns
 * 1 if the the sum of the elements of row 'row' is less than *params (when cast
 * to a double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_row_sum_less_than(const Matrix* mp, int row, void* params)
{
    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return sum_matrix_row_elements(mp, row) < *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_row_sum_greater_than
 *
 * Tests whether the sum of the elements of a row of a matrix is greater than a
 * certain value.
 *
 * This routine is a predicate for the find_in_matrix_by_rows routine. It
 * returns 1 if the the sum of the elements of row 'row' is greater than *params
 * (when cast to a double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_row_sum_greater_than(const Matrix* mp, int row, void* params)
{
    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return sum_matrix_row_elements(mp, row) > *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_column_sum_less_than
 *
 * Tests whether the sum of the elements of a column of a matrix is less than a
 * certain value.
 *
 * This routine is a predicate for the find_in_matrix_by_cols routine. It
 * returns 1 if the the sum of the elements of column 'col' is less than *params
 * (when cast to a double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_column_sum_less_than(const Matrix* mp, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return sum_matrix_col_elements(mp, col) < *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_column_sum_greater_than
 *
 * Tests whether the sum of the elements of a column of a matrix is greater
 * than a certain value.
 *
 * This routine is a predicate for the find_in_matrix_by_cols routine. It
 * returns 1 if the the sum of the elements of column 'col' is greater than
 * *params (when cast to a double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_column_sum_greater_than(const Matrix* mp, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return sum_matrix_col_elements(mp, col) > *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_matrix_element_zero
 *
 * Tests whether a given element of a matrix is zero.
 *
 * This routine is a predicate for the find_in_matrix_as_vector routine. It
 * returns 1 if mp->elements[row][col] is zero.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_element_zero(const Matrix* mp, int row, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return mp->elements[row][col] == 0;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_matrix_element_nonzero
 *
 * Tests whether a given element of a matrix is non-zero.
 *
 * This routine is a predicate for the find_in_matrix_as_vector routine. It
 * returns 1 if mp->elements[row][col] is non-zero.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_element_nonzero(const Matrix* mp, int row, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return mp->elements[row][col] != 0;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_matrix_element_non
 *
 * Tests whether a given element of a matrix is NaN.
 *
 * This routine is a predicate for the find_in_matrix_as_vector routine. It
 * returns 1 if mp->elements[row][col] is NaN.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_element_nan(const Matrix* mp, int row, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return isnand(mp->elements[row][col]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_matrix_element_nonnan
 *
 * Tests whether a given element of a matrix is not NaN.
 *
 * This routine is a predicate for the find_in_matrix_as_vector routine. It
 * returns 1 if mp->elements[row][col] is not NaN.
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_element_nonnan(const Matrix* mp, int row, int col,  __attribute__ ((unused)) void*  dummy_params)
{
    if(col >= mp->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(row >= mp->num_rows)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return !isnand(mp->elements[row][col]);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          is_matrix_element_equal_to
 *
 * Tests whether a given element of a matrix is equal to a certain value.
 *
 * This routine is a predicate for the find_in_matrix_as_vector routine. It
 * returns 1 if mp->elements[row][col] is equal to *params (cast to a double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_element_equal_to(const Matrix* mp, int row, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return mp->elements[row][col] == *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      is_matrix_element_different_from
 *
 * Tests whether a given element of a matrix is different from a certain value.
 *
 * This routine is a predicate for the find_in_matrix_as_vector routine. It
 * returns 1 if mp->elements[row][col] is different from *params (cast to a
 * double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_element_different_from(const Matrix* mp, int row, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return mp->elements[row][col] != *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      is_matrix_element_greater_than
 *
 * Tests whether a given element of a matrix is greater than a certain value.
 *
 * This routine is a predicate for the find_in_matrix_as_vector routine. It
 * returns 1 if mp->elements[row][col] is greater than *params (cast to a
 * double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_element_greater_than(const Matrix* mp, int row, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return mp->elements[row][col] > *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      is_matrix_element_less_than
 *
 * Tests whether a given element of a matrix is less than a certain value.
 *
 * This routine is a predicate for the find_in_matrix_as_vector routine. It
 * returns 1 if mp->elements[row][col] is less than *params (cast to a
 * double).
 *
 * Index: matrices
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an error being set.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_element_less_than(const Matrix* mp, int row, int col, void* params)
{
    if(col >= mp->num_cols || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(row >= mp->num_rows || params == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    return mp->elements[row][col] < *(double*)params;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

