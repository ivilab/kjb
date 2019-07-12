
/* $Id: m_mat_basic.c 21712 2017-08-20 18:21:41Z kobus $ */

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
#include "m/m_mat_basic.h"
#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                            random_split_matrix_by_rows
 *
 * Splits a matrix row-wise
 *
 * This routine splits the source matrix into two matrices pointed to by
 * target_1_mpp and target_2_mpp, putting a randomly selected fraction
 * "fraction" into the first matrix and the rest into the second matrix. If the
 * matrix pointed to by target_1_mpp or target_2_mpp is NULL, then a matrix of
 * the appropriate size is created. If it exists, but is the wrong size, then it
 * is recycled. Otherwise, the storage is recycled. If the source matrix is
 * NULL, then the target matrices becomes NULL also, and any storage associated
 * with it is freed.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Kobus Barnard
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
 */

int random_split_matrix_by_rows
(
    Matrix**          target_1_mpp,
    Matrix**          target_2_mpp,
    const Matrix*     source_mp,
    double fraction
)
{
    int   i;
    int   num_rows;
    int   num_rows_1;
    int   num_rows_2;
    int   num_cols;
    int target_1_index = 0;
    int target_2_index = 0;
    Indexed_vector* index_vp = NULL;
    Int_vector* radix_vp = NULL;

    if ((fraction < 0.0) || (fraction > 1.0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (source_mp == NULL)
    {
        free_matrix(*target_1_mpp);
        *target_1_mpp = NULL;
        free_matrix(*target_2_mpp);
        *target_2_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_zero_int_vector(&radix_vp, num_rows));
    ERE(get_random_indexed_vector(&index_vp, num_rows));
    ERE(ascend_sort_indexed_vector(index_vp));


    num_rows_1 = MAX_OF(0, MIN_OF(num_rows, kjb_rint(fraction * num_rows)));
    num_rows_2 = num_rows - num_rows_1;

    for (i=0; i<num_rows_1; i++)
    {
        radix_vp->elements[ index_vp->elements[ i ].index ] = TRUE;
    }

    if (num_rows_1 == 0)
    {
        free_matrix(*target_1_mpp);
        *target_1_mpp = NULL;
    }
    else
    {
        ERE(get_target_matrix(target_1_mpp, num_rows_1, num_cols));
    }

    if (num_rows_2 == 0)
    {
        free_matrix(*target_2_mpp);
        *target_2_mpp = NULL;
    }
    else
    {
        ERE(get_target_matrix(target_2_mpp, num_rows_2, num_cols));
    }

    for (i=0; i<num_rows; i++)
    {
        if (radix_vp->elements[ i ])
        {
            ERE(copy_matrix_row(*target_1_mpp, target_1_index, source_mp, i));
            target_1_index++;
        }
        else
        {
            ERE(copy_matrix_row(*target_2_mpp, target_2_index, source_mp, i));
            target_2_index++;
        }
    }

    free_indexed_vector(index_vp);
    free_int_vector(radix_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                split_matrix_by_rows
 *
 * Splits a matrix row-wise
 *
 * This routine splits the source matrix into two matrices pointed to by
 * target_1_mpp and target_2_mpp. If the matrix pointed to by target_1_mpp or
 * target_2_mpp is NULL, then a matrix of the appropriate size is created. If it
 * exists, but is the wrong size, then it is recycled. Otherwise, the storage is
 * recycled. If the source matrix is NULL, then the target matrices becomes NULL
 * also, and any storage associated with it is freed.
 *
 * The parameter index_list_vp is a list of row numbers in source_mp that should
 * be copied to target_1_mpp. The remainder go into target_2_mpp.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ranjini Swaminathan
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
 */

int split_matrix_by_rows
(
    Matrix**          target_1_mpp,
    Matrix**          target_2_mpp,
    const Matrix*     source_mp,
    const Int_vector* index_list_vp
)
{
    int   i;
    int   num_rows;
    int   num_cols;
    int   num_index_rows;
    int target_1_index = 0;
    int target_2_index = 0;

    if (source_mp == NULL)
    {
        free_matrix(*target_1_mpp);
        *target_1_mpp = NULL;
        free_matrix(*target_2_mpp);
        *target_2_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    num_index_rows = index_list_vp->length;

    ERE(get_target_matrix(target_1_mpp, num_index_rows, num_cols));
    ERE(get_target_matrix(target_2_mpp, num_rows - num_index_rows, num_cols));

    for (i=0; i<num_rows; i++)
    {
        if(i == index_list_vp->elements[ target_1_index ])
        {
            copy_matrix_row(*target_1_mpp,target_1_index,source_mp,i);
            target_1_index++;
        }
        else
        {
            copy_matrix_row(*target_2_mpp,target_2_index,source_mp,i);
            target_2_index++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             get_matrix_transpose
 *
 * Gets the transpose a matrix
 *
 * This routine produces a matrix which is the transpose of the argument.
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

int get_matrix_transpose(Matrix** target_mpp, const Matrix* mp)
{
    Matrix* target_mp;
    int     i;
    int     j;


    int     result = NO_ERROR;


    if (mp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
    }
    /*
     * If it so happens that the user has made the target matrix the input
     * matrix, then we need to compute the result into a temporary location and
     * copy the result, otherwise we will be changing the input as we compute
     * the answer.
    */
    else if (*target_mpp == mp)
    {
        Matrix* temp_mp = NULL;

        result = get_matrix_transpose(&temp_mp, mp);

        if (result != ERROR)
        {
            result = copy_matrix(target_mpp, temp_mp);
        }

        free_matrix(temp_mp);
    }
    else
    {
        ERE(get_target_matrix(target_mpp, mp->num_cols, mp->num_rows));
        target_mp = *target_mpp;

        for (i = 0; i < mp->num_rows; i++)
        {
            for (j = 0; j < mp->num_cols; j++)
            {
                (target_mp->elements)[j][i] = (mp->elements)[i][j];
             }
         }
    }

     return NO_ERROR;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should be either static or deleted`. */

/*
 * =============================================================================
 *                        create_matrix_transpose
 *
 * Creates a matrix which is the transpose of the argument
 *
 * This routine creates a matrix which is the transpose of the argument.
 *
 * Returns:
 *     A  pointer to the matrix on success and NULL on failure (with an error
 *     message being set.) This routine will only fail if storage allocation
 *     fails.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
 */

#endif

/* OBSOLETE, delete soon. */
Matrix* create_matrix_transpose(const Matrix* mp)
{
    Matrix* t_mp;
    int i, j;


    NRN(t_mp = create_matrix(mp->num_cols, mp->num_rows));

    for (i = 0; i < mp->num_rows; i++)
    {
        for (j = 0; j < mp->num_cols; j++)
        {
            (t_mp->elements)[j][i] = (mp->elements)[i][j];
         }
     }

     return t_mp;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should be either static or deleted`. */
/*
 * =============================================================================
 *                               create_matrix_copy
 *
 * Creates a copy of a matrix
 *
 * This routine creates a copy of a matrix and returns a pointer to it.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then NULL is returned
 *    with and error message being set. Otherwise a pointer to a newly created
 *    copy of the input matrix is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/
#endif

/* OBSOLETE, delete soon. */
Matrix* create_matrix_copy(const Matrix* source_mp)
{
    Matrix* target_mp = NULL;


    ERN(copy_matrix(&target_mp, source_mp));

    return target_mp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                copy_matrix
 *
 * Copies a matrix
 *
 * This routine copies the matrix into the one pointed to by target_mpp. If
 * the matrix pointed to by target_mpp is NULL, then a matrix of the
 * appropriate size is created. If it exists, but is the wrong size, then it is
 * recycled. Otherwise, the storage is recycled. If the source matrix is NULL,
 * then the target matrix becomes NULL also, and any storage associated with it
 * is freed.  If the source matrix and target matrix point to the same address,
 * this function simply returns NO_ERROR.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_copy_matrix
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const char*   file_name,
    int           line_number
)
{
    IMPORT int kjb_use_memcpy;
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    double* target_pos;
    double* source_pos;


    if (source_mp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    if(*target_mpp == source_mp)
        return NO_ERROR;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(debug_get_target_matrix(target_mpp, num_rows, num_cols, file_name,
                                line_number));

    if(num_rows == 0 || num_cols == 0)
    {
        return NO_ERROR;
    }

    if (    (kjb_use_memcpy)
         && (num_cols == source_mp->max_num_cols)
         /* Don't need to check source becase we just did the get_target. */
       )
    {
        (void)memcpy((*target_mpp)->elements[ 0 ],
                     source_mp->elements[ 0 ],
                     (size_t)num_cols * (size_t)num_rows * sizeof(double));
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_pos = (source_mp->elements)[ i ];
            target_pos = ((*target_mpp)->elements)[ i ];

            for (j=0; j<num_cols; j++)
            {
                *target_pos = *source_pos;
                target_pos++;
                source_pos++;
            }
        }
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int copy_matrix(Matrix** target_mpp, const Matrix* source_mp)
{
    IMPORT int kjb_use_memcpy;
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    double* target_pos;
    double* source_pos;


    if (source_mp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    if(*target_mpp == source_mp)
        return NO_ERROR;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    if(num_rows == 0 || num_cols == 0)
    {
        return NO_ERROR;
    }


    if (    (kjb_use_memcpy)
         && (num_cols == source_mp->max_num_cols)
         /* Don't need to check source becase we just did the get_target. */
       )
    {
        (void)memcpy((*target_mpp)->elements[ 0 ],
                     source_mp->elements[ 0 ],
                     ((size_t)num_cols * num_rows) * sizeof(double));
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            source_pos = (source_mp->elements)[ i ];
            target_pos = ((*target_mpp)->elements)[ i ];

            for (j=0; j<num_cols; j++)
            {
                *target_pos = *source_pos;
                target_pos++;
                source_pos++;
            }
        }
    }

    return NO_ERROR;
}

#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            select_matrix_cols
 *
 * Copies a matrix cols to target matrix
 *
 * This routine copies columns source_mp into *target_mpp which is created or
 * resized if necessary.  If num_rows or num_cols is negative, then the number
 * for rows or columns respectively in the source matrix is used.
 *
 * Returns :
 *    If the routine fails then ERROR is returned with and error message being
 *    set.  Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int select_matrix_cols
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    const Int_vector* selected_cols_vp
)
{

    if ((selected_cols_vp == NULL) || (source_mp == NULL))
    {
        return copy_matrix(target_mpp, source_mp);
    }
    else
    {
        int i;
        int num_needed_cols = 0;
        int num_rows = source_mp->num_rows;
        int num_cols = source_mp->num_cols;

        if (selected_cols_vp->length != num_cols)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        for (i = 0; i < num_cols; i++)
        {
            if (selected_cols_vp->elements[ i ] != 0)
            {
                num_needed_cols++;
            }
        }

        if (num_needed_cols == 0)
        {
            free_matrix(*target_mpp);
            *target_mpp = NULL;
            return NO_ERROR;
        }
        else
        {
            ERE(get_target_matrix(target_mpp, num_rows, num_needed_cols));

            return ow_copy_matrix_with_col_selection(*target_mpp, 0, 0,
                                                     source_mp,
                                                     selected_cols_vp);
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            copy_matrix_block
 *
 * Copies a matrix block to target matrix
 *
 * This routine copies a block of source_mp into *target_mpp which is created or
 * resized if necessary.  If num_rows or num_cols is negative, then the number
 * for rows or columns respectively in the source matrix is used.
 *
 * Returns :
 *    If the routine fails then ERROR is returned with and error message being
 *    set.  Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int copy_matrix_block
(
    Matrix**      target_mpp,
    const Matrix* source_mp,
    int           source_row_offset,
    int           source_col_offset,
    int           num_rows,
    int           num_cols
)
{


    if (num_rows < 0)
    {
        num_rows = source_mp->num_rows;
    }

    if (num_cols < 0)
    {
        num_cols = source_mp->num_cols;
    }

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    ERE(ow_copy_matrix_block(*target_mpp, 0, 0,
                             source_mp, source_row_offset, source_col_offset,
                             num_rows, num_cols));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_copy_matrix_block
 *
 * Copies a matrix block to an offset position of a target matrix
 *
 * This routine copies a block of source_mp into target_mp, offset by
 * (target_row_offset, target_col_offset). If num_rows or num_cols is negative,
 * then the number for rows or columns respectively in the source matrix is
 * used.  The target matrix must be large enough to hold the entire selection,
 * otherwise the bug handler is called.
 *
 * Returns :
 *    If the routine fails then ERROR is returned with and error message being
 *    set.  Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

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
)
{
    int   i;
    int   j;
    double* target_pos;
    double* source_pos;


    NRE(source_mp);
    NRE(target_mp);

    if (num_rows < 0)
    {
        num_rows = source_mp->num_rows;
    }

    if (num_cols < 0)
    {
        num_cols = source_mp->num_cols;
    }

    if (    (target_row_offset + num_rows > target_mp->num_rows)
         || (target_col_offset + num_cols > target_mp->num_cols)
         || (source_row_offset + num_rows > source_mp->num_rows)
         || (source_col_offset + num_cols > source_mp->num_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        source_pos = source_mp->elements[ source_row_offset + i ] + source_col_offset;
        target_pos = target_mp->elements[ target_row_offset + i ] + target_col_offset;

        for (j=0; j<num_cols; j++)
        {
            *target_pos = *source_pos;
            target_pos++;
            source_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            copy_matrix_block_2
 *
 * Copies a matrix block to target matrix
 *
 * This routine copies a block of source_mp into *target_mpp which is created or
 * resized if necessary. It has further functionality from copy_matrix_block, in
 * that the copying can jump rows/cols in the source matrix.
 *
 * Returns :
 *    If the routine fails then ERROR is returned with and error message being
 *    set.  Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

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
)
{


    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    ERE(ow_copy_matrix_block_2(*target_mpp, 0, 0, 1, 1,
                               source_mp, source_row_offset, source_col_offset,
                               source_row_step, source_col_step,
                               num_rows, num_cols));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_copy_matrix_block_2
 *
 * Copies a matrix block to an offset position of a target matrix
 *
 * This routine copies a block of source_mp into target_mp, offset by
 * (target_row_offset, target_col_offset). If source_row_step or source_col_step
 * are greater than 1, then the copying jumps rows/columns in the source matrix.
 * If target_row_step or target_col_step are greater than one, then the copying
 * jumps rows/columns in the target matrix.
 *
 * Returns :
 *    If the routine fails (can't really happen unless there is an argument
 *    error), then ERROR is returned with and error message being set.
 *    Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

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
)
{
    int   i;
    int   j;
    double* target_pos;
    double* source_pos;


    /*
    dbi(target_mp->num_rows);
    dbi(target_mp->num_cols);
    dbi(target_row_offset);
    dbi(target_col_offset);
    dbi(target_row_step);
    dbi(target_col_step);

    dbi(source_mp->num_rows);
    dbi(source_mp->num_cols);
    dbi(source_row_offset);
    dbi(source_col_offset);
    dbi(source_row_step);
    dbi(source_col_step);

    dbi(num_rows);
    dbi(num_cols);
    */

    if (source_row_step < 1) source_row_step = 1;
    if (source_col_step < 1) source_col_step = 1;
    if (target_row_step < 1) target_row_step = 1;
    if (target_col_step < 1) target_col_step = 1;

    if (    (target_row_offset > 0)
         || (source_row_offset > 0)
       )
    {
        UNTESTED_CODE();
    }

    if (    (target_row_offset + target_row_step * (num_rows - 1) >= target_mp->num_rows)
         || (target_col_offset + target_col_step * (num_cols - 1) >= target_mp->num_cols)
         || (source_row_offset + source_row_step * (num_rows - 1) >= source_mp->num_rows)
         || (source_col_offset + source_col_step * (num_cols - 1) >= source_mp->num_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        source_pos = source_mp->elements[ source_row_offset + i * source_row_step ] + source_col_offset;
        target_pos = target_mp->elements[ target_row_offset + i * target_row_step ] + target_col_offset;

        for (j=0; j<num_cols; j++)
        {
            *target_pos = *source_pos;
            target_pos += target_col_step;
            source_pos += source_col_step;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_copy_matrix
 *
 * Copies a matrix to an offset position of a target matrix
 *
 * This routine copies source_mp into target_mp, offset by (target_row_offset,
 * target_col_offset).
 *
 * Returns :
 *    If the routine fails (can't really happen unless there is an argument
 *    error), then ERROR is returned with and error message being set.
 *    Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int ow_copy_matrix
(
    Matrix*       target_mp,
    int           target_row_offset,
    int           target_col_offset,
    const Matrix* source_mp
)
{
#ifdef HOW_IT_WAS_APRIL_18_2007
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    int   num_target_rows;
    int   num_target_cols;
    double* target_pos;
    double* source_pos;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    num_target_rows = target_mp->num_rows;
    num_target_cols = target_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        if (target_row_offset + i < 0) continue;
        if (target_row_offset + i >= num_target_rows) continue;

        target_pos = target_mp->elements[ target_row_offset + i ];
        source_pos = source_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            if (target_col_offset + j < 0) continue;
            if (target_col_offset + j >= num_target_cols) continue;

            target_pos[ target_col_offset + j ] = source_pos[ j ];
        }
    }

    return NO_ERROR;
#else
    return ow_copy_matrix_with_col_selection(target_mp,
                                             target_row_offset,
                                             target_col_offset,
                                             source_mp,
                                             NULL);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       ow_copy_matrix_with_col_selection
 *
 * Copies a selected columns of a matrix to an offset position of a target matrix
 *
 * This routine copies selected columns of source_mp into target_mp, offset by
 * (target_row_offset, target_col_offset).
 *
 * Returns :
 *    If the routine fails (can't really happen unless there is an argument
 *    error), then ERROR is returned with and error message being set.
 *    Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int ow_copy_matrix_with_col_selection
(
    Matrix*       target_mp,
    int           target_row_offset,
    int           target_col_offset,
    const Matrix* source_mp,
    const Int_vector* selected_cols_vp
)
{
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    int   num_target_rows;
    int   num_target_cols;
    double* target_pos;
    double* source_pos;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    num_target_rows = target_mp->num_rows;
    num_target_cols = target_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        int col_count = 0;

        if (target_row_offset + i < 0) continue;
        if (target_row_offset + i >= num_target_rows) continue;

        target_pos = target_mp->elements[ target_row_offset + i ];
        source_pos = source_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            if ((selected_cols_vp != NULL) && (selected_cols_vp->elements[ j ] == 0)) continue;

            if (target_col_offset + col_count < 0) continue;
            if (target_col_offset + col_count >= num_target_cols) continue;

            target_pos[ target_col_offset + col_count ] = source_pos[ j ];
            col_count++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef HOW_IT_WAS_JUNE_28_2004

/* This routine looks exactly the same as copy_matrix_row(), so it got purged.
 * Of course, a matrix row copy is overwrite, but do we put it into the naming
 * convection? If we did that, we would want to rename put_matrix_row().
*/

/* =============================================================================
 *                            ow_copy_matrix_row
 *
 * Copies a matrix row to another matrix
 *
 * This routine copies a row of source_mp into a row of target_mp.
 *
 * Note :
 *     This routine is the same as copy_matrix_row()! Eventually, one of them
 *     should be purged!
 *
 * Returns :
 *    If the routine fails (can't really happen unless there is an argument
 *    error), then ERROR is returned with and error message being set.
 *    Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int ow_copy_matrix_row
(
    Matrix*       target_mp,
    int           target_row,
    const Matrix* source_mp,
    int           source_row
)
{
    int j;
    int           num_source_rows;
    int           num_source_cols;
    int           num_target_rows;
    int           num_target_cols;
    double*       target_pos;
    const double* source_pos;


    num_source_rows = source_mp->num_rows;
    num_source_cols = source_mp->num_cols;

    num_target_rows = target_mp->num_rows;
    num_target_cols = target_mp->num_cols;

    if (    (target_row < 0)
         || (target_row >= num_target_rows)
         || (source_row < 0)
         || (source_row >= num_source_cols)
         || (num_target_cols != num_source_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    target_pos = target_mp->elements[ target_row ];
    source_pos = source_mp->elements[ source_row ];

    for (j=0; j<num_target_cols; j++)
    {
        *target_pos = *source_pos;
        target_pos++;
        source_pos++;
    }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_matrix_data
(
    Matrix**      target_mpp,
    const Matrix* mp,
    const int*    invalid_data
)
{

    if ((invalid_data == NULL) || (mp == NULL))
    {
        return copy_matrix(target_mpp, mp);
    }
    else
    {
        int   count      = 0;
        int   i, j;
        int   num_rows   = mp->num_rows;
        int   num_cols   = mp->num_cols;
        double* target_pos;
        double* source_pos;


        ERE(get_target_matrix(target_mpp, num_rows, num_cols));

        for (i=0; i<num_rows; i++)
        {
            if ( ! invalid_data[ i ])
            {
                source_pos = (mp->elements)[ i ];
                target_pos = ((*target_mpp)->elements)[ count ];

                for (j=0; j<num_cols; j++)
                {
                    *target_pos++ = *source_pos++;
                }

                count++;
            }
        }

        if (count == 0)
        {
            set_error("No valid data.");
            free_matrix(*target_mpp);
            *target_mpp = NULL;
            return ERROR;
        }
        else
        {
            (*target_mpp)->num_rows = count;
            return NO_ERROR;
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          copy_int_matrix_to_matrix
 *
 * Copies an integer matrix to a matrix
 *
 * This routine copies the integer matrix source_mp to the one pointed to by
 * target_mpp. It is thus used mostly for caching conversion results for
 * performance. If the matrix pointed to by target_mpp is NULL, then a matrix
 * of the appropriate size is created. If it exists, but is the wrong size,
 * then it is recycled. Otherwise, the storage is recycled. If the source
 * matrix is NULL, then the target matrix becomes NULL also, and any storage
 * associated with it is freed.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int copy_int_matrix_to_matrix
(
    Matrix**          target_mpp,
    const Int_matrix* source_mp
)
{
    int     i, j, num_rows, num_cols;
    double* target_pos;
    int* source_pos;


    if (source_mp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    for (i=0; i<num_rows; i++)
    {
        source_pos = (source_mp->elements)[ i ];
        target_pos = ((*target_mpp)->elements)[ i ];

        for (j=0; j<num_cols; j++)
        {
            *target_pos++ = *source_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          copy_matrix_to_int_matrix
 *
 * Copies a matrix to an integer matrix
 *
 * This routine copies the matrix source_mp to the integer matrix pointed to by
 * target_mpp. It is thus used mostly for caching conversion results for
 * performance. If the matrix pointed to by target_mpp is NULL, then a matrix
 * of the appropriate size is created. If it exists, but is the wrong size,
 * then it is recycled. Otherwise, the storage is recycled. If the source
 * matrix is NULL, then the target matrix becomes NULL also, and any storage
 * associated with it is freed.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int copy_matrix_to_int_matrix
(
    Int_matrix**  target_mpp,
    const Matrix* source_mp
)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    int*    target_pos;
    double* source_pos;


    if (source_mp == NULL)
    {
        free_int_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_int_matrix(target_mpp, num_rows, num_cols));

    for (i=0; i<num_rows; i++)
    {
        source_pos = (source_mp->elements)[ i ];
        target_pos = ((*target_mpp)->elements)[ i ];

        for (j=0; j<num_cols; j++)
        {
            *target_pos = kjb_rint(*source_pos);
            target_pos++;
            source_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int split_matrix_vector
(
    Matrix_vector**      out_1_mvpp,
    Matrix_vector**      out_2_mvpp,
    const Matrix_vector* in_mvp,
    const Int_vector*    index_list_vp
)
{
    Matrix**      out_1_mp2;
    Matrix**      out_2_mp2;
    Matrix**      in_mp2;
    int           num_matrices;
    int           i;
    int           index_list_length;

    int index_1      = 0;
    int index_2      = 0;
    int source_index = 0;
    if (in_mvp == NULL)
    {
        free_matrix_vector(*out_1_mvpp);
        *out_1_mvpp = NULL;
        free_matrix_vector(*out_2_mvpp);
        *out_2_mvpp = NULL;
        return NO_ERROR;
    }

    num_matrices = in_mvp->length;
    index_list_length = index_list_vp->length;

    if (    (*out_1_mvpp != NULL)
            && ((*out_1_mvpp)->length != index_list_length)
       )
    {
        free_matrix_vector(*out_1_mvpp);
        *out_1_mvpp = NULL;
    }

    if (*out_1_mvpp == NULL)
    {
        NRE(*out_1_mvpp = create_matrix_vector(index_list_length));
    }
    if (    (*out_2_mvpp != NULL)
            && ((*out_2_mvpp)->length != num_matrices - index_list_length)
       )
    {
        free_matrix_vector(*out_2_mvpp);
        *out_2_mvpp = NULL;
    }

    if (*out_2_mvpp == NULL)
    {
        NRE(*out_2_mvpp = create_matrix_vector(num_matrices - index_list_length));
    }


    out_1_mp2 = (*out_1_mvpp)->elements;
    out_2_mp2 = (*out_2_mvpp)->elements;
    in_mp2  = in_mvp->elements;

    for (i=0; i<num_matrices; i++)
    {
        if(index_list_vp->elements[ source_index ] == i)
        {
            ERE(copy_matrix(&(out_1_mp2[ index_1 ]), in_mp2[ i ]));
            source_index++;
            index_1++;
        }
        else
        {
            ERE(copy_matrix(&(out_2_mp2[ index_2 ]), in_mp2[ i ]));
            index_2++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_matrix_vector
(
    Matrix_vector**      out_mvpp,
    const Matrix_vector* in_mvp
)
{
    Matrix**      out_mp2;
    Matrix**      in_mp2;
    int           num_matrices;
    int           i;


    if (in_mvp == NULL)
    {
        free_matrix_vector(*out_mvpp);
        *out_mvpp = NULL;
        return NO_ERROR;
    }

    num_matrices = in_mvp->length;

    if (    (*out_mvpp != NULL)
         && ((*out_mvpp)->length != num_matrices)
       )
    {
        free_matrix_vector(*out_mvpp);
        *out_mvpp = NULL;
    }

    ERE(get_target_matrix_vector(out_mvpp, num_matrices));

    out_mp2 = (*out_mvpp)->elements;
    in_mp2  = in_mvp->elements;

    for (i=0; i<num_matrices; i++)
    {
        ERE(copy_matrix(&(out_mp2[ i ]), in_mp2[ i ]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int copy_matrix_vector_block
(
    Matrix_vector**      out_mvpp,
    const Matrix_vector* in_mvp,
    int start_index,
    int num_matrices
)
{
    Matrix**      out_mp2;
    Matrix**      in_mp2;
    int           i;


    if (in_mvp == NULL)
    {
        free_matrix_vector(*out_mvpp);
        *out_mvpp = NULL;
        return NO_ERROR;
    }

    if (start_index + num_matrices > in_mvp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (    (*out_mvpp != NULL)
         && ((*out_mvpp)->length != num_matrices)
       )
    {
        free_matrix_vector(*out_mvpp);
        *out_mvpp = NULL;
    }

    ERE(get_target_matrix_vector(out_mvpp, num_matrices));

    out_mp2 = (*out_mvpp)->elements;
    in_mp2  = in_mvp->elements;

    for (i=0; i<num_matrices; i++)
    {
        ERE(copy_matrix(&(out_mp2[ i ]), in_mp2[ i + start_index ]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int concat_matrix_vectors
(
    Matrix_vector**       mvpp,
    int                   num_vectors,
    const Matrix_vector** matrix_vectors
)
{
    int i, length = 0;
    Matrix** target_pos;

    UNTESTED_CODE();

    for (i = 0; i < num_vectors; i++)
    {
        if (matrix_vectors[ i ] != NULL)
        {
            length += matrix_vectors[ i ]->length;
        }
    }

    ERE(get_target_matrix_vector(mvpp, length));
    target_pos = (*mvpp)->elements;

    for (i = 0; i < num_vectors; i++)
    {
        const Matrix_vector* source_mvp    = matrix_vectors[ i ];

        if (source_mvp != NULL)
        {
            int                  source_length = source_mvp->length;
            Matrix**             source_pos    = source_mvp->elements;
            int                  j;

            for (j = 0; j < source_length; j++)
            {
                ERE(copy_matrix(target_pos, *source_pos));
                target_pos++;
                source_pos++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int mp_row_get_indexed_vector
(
    Indexed_vector** vpp,
    Matrix*          init_mp,
    int              row
)
{
    Indexed_vector_element* vp_pos;
    int length = init_mp->num_cols;
    double* mp_row_pos = init_mp->elements[ row ];
    int             i;


    ERE(get_target_indexed_vector(vpp, length));

    vp_pos = (*vpp)->elements;

    for (i=0; i<length; i++)
    {
        vp_pos->index = i;
        vp_pos->element = *mp_row_pos;
        vp_pos++;
        mp_row_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_random_matrix_row
 *
 * Gets a random matrix row
 *
 * This routine gets a random row from the matrix "mp" and puts it into the
 * vector pointed to by vpp. If *vpp is NULL, then the vector is created.
 * If it is the wrong length, then it is resized. Otherwise, the storage is
 * reused.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise the index of the row selected
 *    is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_random_matrix_row(Vector** vpp, const Matrix* mp)
{
    int index = (int)(kjb_rand_2() * mp->num_rows);


    if (index == mp->num_rows) index--;

    ERE(get_matrix_row(vpp, mp, index));

    return index;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_matrix_row
 *
 * Returns a matrix row
 *
 * This routine gets row "row_num" from the matrix "mp" and puts it into the
 * vector pointed to by vpp. If *vpp is NULL, then the vector is created.
 * If it is the wrong length, then it is resized. Otherwise, the storage is
 * reused.
 *
 * Note :
 *     Matrix row counting begins with 0.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_row(Vector** vpp, const Matrix* mp, int row_num)
{
    int     len;
    int     i;
    double*   row_pos;
    double*   vect_pos;


    if ((row_num < 0) || (row_num >= mp->num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = mp->num_cols;

    ERE(get_target_vector(vpp, len));

    row_pos  = mp->elements[ row_num ];
    vect_pos = (*vpp)->elements;

    for (i=0; i<len; i++)
    {
        *vect_pos++ = *row_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               ow_get_matrix_row
 *
 * Returns a matrix row
 *
 * This routine gets row "row_num" from the matrix "mp" and puts it into the
 * vector pointed to by vp which must be allocated and the correct size.
 *
 * Note :
 *     Matrix row counting begins with 0.
 *
 * Returns :
 *    If the routine fails, then ERROR is returned with and error message being
 *    set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int ow_get_matrix_row(Vector* vp, const Matrix* mp, int row_num)
{
    int     len;
    int     i;
    double*   row_pos;
    double*   vect_pos;


    if ((row_num < 0) || (row_num >= mp->num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = mp->num_cols;

    if (len != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    row_pos  = mp->elements[ row_num ];
    vect_pos = vp->elements;

    for (i=0; i<len; i++)
    {
        *vect_pos = *row_pos;
        vect_pos++;
        row_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                remove_matrix_row
 *
 * Removes a matrix row
 *
 * This routine removes row "row_num" from the matrix "mp" and optionally puts
 * it into the vector pointed to by vpp (if vpp is not NULL). If *vpp
 * is NULL, then the vector is created.  If it is the wrong length, then it is
 * resized. Otherwise, the storage is reused.
 *
 * Note:
 *     Matrix row counting begins with 0.
 *
 * Note
 *    Currently, trying to remove the only row of matrix is treated as a bug.
 *    This may change with future versions.
 *
 * Returns:
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int remove_matrix_row(Vector** vpp, Matrix* mp, int row_num)
{
    int len, i, j;
    int num_rows = mp->num_rows;


    /*
    // FIX
    //
    // Currently, lets not deal with single row matrices. Once we implement
    // resziable matrices, then this restriction should be relaxed.
    */
    if ((row_num < 0) || (row_num >= num_rows) || (num_rows <= 1))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = mp->num_cols;

    if (vpp != NULL)
    {
        ERE(get_target_vector(vpp, len));
    }

    num_rows--;

    for (j=0; j<len; j++)
    {
        if (vpp != NULL)
        {
            (*vpp)->elements[ j ] = mp->elements[ row_num ][ j ];
        }

        for (i=row_num; i<num_rows; i++)
        {
            mp->elements[ i ][ j ] = mp->elements[ i + 1 ][ j ];
        }
    }

    mp->num_rows = num_rows;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                put_matrix_row
 *
 * Replaces a matrix row with the contents of a vector
 *
 * This routine replaces row "row_num" of the matrix "mp" with the contents of
 * vector vp.
 *
 * Note :
 *     Matrix row counting begins with 0.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int put_matrix_row(Matrix* mp, const Vector* vp, int row_num)
{

    int len, i;
    double* row_pos;
    double* vect_pos;


    if (    (row_num < 0)
         || (row_num >= mp->num_rows)
         || (mp->num_cols != vp->length)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = vp->length;

    row_pos = mp->elements[ row_num ];
    vect_pos = vp->elements;

    for (i=0; i<len; i++)
    {
        *row_pos++ = *vect_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                copy_matrix_row
 *
 * Replaces a matrix row with a row from another
 *
 * This routine replaces row "target_row_num" of the matrix "target_mp" with the
 * contents of "source_row_num" of "source_mp".
 *
 * Note :
 *     Matrix row counting begins with 0.
 *
 * Returns :
 *    If the routine fails, then ERROR is returned with and error message being
 *    set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int copy_matrix_row
(
    Matrix*       target_mp,
    int           target_row_num,
    const Matrix* source_mp,
    int           source_row_num
)
{
    int len, i;
    double* target_row_pos;
    double* source_row_pos;


    if (    (source_row_num < 0) || (source_row_num >= source_mp->num_rows)
         || (target_row_num < 0) || (target_row_num >= target_mp->num_rows)
         || (source_mp->num_cols != target_mp->num_cols)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = source_mp->num_cols;

    source_row_pos = source_mp->elements[ source_row_num ];
    target_row_pos = target_mp->elements[ target_row_num ];

    for (i=0; i<len; i++)
    {
        *target_row_pos = *source_row_pos;
        target_row_pos++;
        source_row_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_random_matrix_col
 *
 * Gets a random matrix column
 *
 * This routine gets a random column from the matrix "mp" and puts it into the
 * vector pointed to by vpp. If *vpp is NULL, then the vector is created.
 * If it is the wrong length, then it is resized. Otherwise, the storage is
 * reused.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise the index of the column
 *    selected is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_random_matrix_col(Vector** vpp, const Matrix* mp)
{
    int index = (int)(kjb_rand_2() * mp->num_cols);


    ERE(get_matrix_col(vpp, mp, index));

    return index;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_matrix_col
 *
 * Returns a matrix column
 *
 * This routine gets column "col_num" from the matrix "mp" and puts it into the
 * vector pointed to by vpp. If *vpp is NULL, then the vector is created.
 * If it is the wrong length, then it is resized. Otherwise, the storage is
 * reused.
 *
 * Note :
 *     Matrix column counting begins with 0.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_col(Vector** vpp, const Matrix* mp, int col_num)
{
    int   len;
    int   i;
    double* vect_pos;


    if ((col_num < 0) || (col_num >= mp->num_cols))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = mp->num_rows;

    ERE(get_target_vector(vpp, len));

    vect_pos = (*vpp)->elements;

    for (i=0; i<len; i++)
    {
        *vect_pos++ = mp->elements[ i ][ col_num ];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                remove_matrix_col
 *
 * Removes a matrix column
 *
 * This routine removes column "col_num" from the matrix "mp" and optionally
 * puts it into the vector pointed to by vpp (if vpp is not NULL). If
 * *vpp is NULL, then the vector is created.  If it is the wrong length,
 * then it is resized. Otherwise, the storage is reused.
 *
 * Note:
 *     Matrix column counting begins with 0.
 *
 * Note
 *    Currently, trying to remove the only column of matrix is treated as a
 *    bug.  This may change with future versions.
 *
 * Returns:
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int remove_matrix_col(Vector** vpp, Matrix* mp, int col_num)
{
    int len, i, j = 0;
    int num_cols = mp->num_cols;

    
    /* FIX */
    
    /* Currently, lets not deal with single column matrices. Once we implement */
    /* resziable matrices, then this restriction should be relaxed. */
    if ((col_num < 0) || (col_num >= num_cols) || (num_cols <= 1))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = mp->num_rows;

    if (vpp != 0)
    {
        printf("vpp is not NULL");
        ERE(get_target_vector(vpp, len));
    }

    if(col_num < num_cols--)
    {
        for (i=0; i<len; i++)
        {
            if (vpp != 0)
            {
                (*vpp)->elements[ i ] = mp->elements[ i ][ col_num ];
            }

            /*  update row pointer */
            mp->elements[i] -= i;

            /*  shift entire row left */
            for (j=0; j<col_num; j++)
                mp->elements[ i ][ j ] = mp->elements[ i ][ j + i ];

            /*  ... and shift elements after `col_num` left by one extra */
            for (j=col_num; j<num_cols; j++)
            {
                mp->elements[ i ][ j ] = mp->elements[ i ][ j + i + 1 ];
            }
        }
    }

    mp->num_cols = num_cols;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                put_matrix_col
 *
 * Replaces a matrix column with the contents of a vector
 *
 * This routine replaces column "col_num" of the matrix "mp" with the contents
 * of vector vp.
 *
 * Note :
 *     Matrix column counting begins with 0.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int put_matrix_col(Matrix* mp, const Vector* vp, int col_num)
{
    int   len;
    int   i;
    double* vect_pos;


    if (    (col_num < 0) || (col_num >= mp->num_cols)
         || (mp->num_rows != vp->length))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = vp->length;

    vect_pos = vp->elements;

    for (i=0; i<len; i++)
    {
        mp->elements[ i ][ col_num ] = *vect_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             copy_matrix_col
 *
 * Replaces a matrix column with a column from another
 *
 * This routine replaces column "target_col_num" of the matrix "target_mp" with
 * the contents of "source_col_num" of "source_mp".
 *
 * Note :
 *     Matrix col counting begins with 0.
 *
 * Returns :
 *    If the routine fails, then ERROR is returned with and error message being
 *    set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int copy_matrix_col
(
    Matrix*       target_mp,
    int           target_col_num,
    const Matrix* source_mp,
    int           source_col_num
)
{
    int len, i;


    if (    (source_col_num < 0) || (source_col_num >= source_mp->num_cols)
         || (target_col_num < 0) || (target_col_num >= target_mp->num_cols)
         || (source_mp->num_rows != target_mp->num_rows)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = source_mp->num_rows;

    for (i=0; i<len; i++)
    {
        target_mp->elements[ i ][ target_col_num ] = source_mp->elements[ i ][ source_col_num ];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                insert_zero_row_in_matrix
 *
 * Inserts a row of zeroes into a matrix, shifting existing elements correspondingly
 *
 * This routine inserts zeroes into row "col_num" of the matrix mp,
 * shifting the existing contents from rows "row_num" and above down a row
 * and discarding the last row
 *
 * Note:
 *     Matrix row counting begins with 0.
 *
 * Returns:
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int insert_zero_row_in_matrix(Matrix* mp, int row_num)
{
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    int i,j;

    if ((row_num < 0) || (row_num > num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (j=0; j<num_cols; j++)
    {
        for (i = num_rows - 2; i>=row_num; i--)
        {
            mp->elements[ i + 1 ][ j  ] = mp->elements[ i ][ j ];
        }
        mp->elements[ row_num ][ j ] = 0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                insert_zero_col_in_matrix
 *
 * Inserts a column of zeroes into a matrix, shifting existing elements correspondingly
 *
 * This routine inserts zeroes into column "col_num" of the matrix mp,
 * shifting the existing contents from columns "col_num" and above over a column
 * and discarding the last column
 * 
 *
 * Note:
 *     Matrix column counting begins with 0.
 *
 * Returns:
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int insert_zero_col_in_matrix(Matrix* mp, int col_num)
{
    int num_cols = mp->num_cols;
    int num_rows = mp->num_rows;
    int i,j;

    if ((col_num < 0) || (col_num > num_cols))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        for (j=num_cols-2; j>=col_num; j--)
        {
            mp->elements[ i ][ j + 1 ] = mp->elements[ i ][ j ];
        }
        mp->elements[ i ][ col_num ] = 0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int swap_matrix_rows(Matrix* mp, int row1, int row2)
{
    double  temp;
    double* row1_pos;
    double* row2_pos;
    int   i;
    int   num_cols;


    num_cols = mp->num_cols;
    row1_pos = (mp->elements)[row1];
    row2_pos = (mp->elements)[row2];

    for (i=0; i<num_cols; i++)
    {
        temp = *row1_pos;
        *row1_pos = *row2_pos;
        *row2_pos = temp;

        row1_pos++;
        row2_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int swap_matrix_cols(Matrix* mp, int col1, int col2)
{
    int  num_rows;
    int  i;
    double temp;


    num_rows = mp->num_rows;

    for (i=0; i<num_rows; i++)
    {
        temp = (mp->elements)[i][col1];
        (mp->elements)[i][col1] = (mp->elements)[i][col2];
        (mp->elements)[i][col2] = temp;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int select_matrix_data
(
    Matrix**      selected_mpp,
    const Matrix* data_mp,
    int           num_data_points,
    const int*    data_point_nums,
    const int*    invalid_data
)
{
    Matrix* selected_mp;
    int     num_cols, i, j;
    int     count = 0;


    if ((data_mp == NULL) || (data_mp->num_rows == 0))
    {
        set_error("No valid input data to select data from.");
        return ERROR;
    }
    else if (num_data_points <= 0)
    {
        return copy_matrix_data(selected_mpp, data_mp, invalid_data);
    }

    num_cols = data_mp->num_cols;

    ERE(get_target_matrix(selected_mpp, num_data_points, num_cols));
    selected_mp = *selected_mpp;

    for (i=0; i<num_data_points; i++)
    {
        int index = data_point_nums[ i ] - 1;

        if (index >= data_mp->num_rows)
        {
            set_bug("Index exceeds matrix dimension in select_matrix_data().");
            return ERROR;
        }

        if ((invalid_data == NULL) || (! invalid_data[ i ]))
        {
            for (j=0; j<num_cols; j++)
            {
                (selected_mp->elements)[ count ][ j ] =
                                             (data_mp->elements)[ index ][ j ];
            }
            count++;
        }
    }

    selected_mp->num_rows = count;

    if (count == 0)
    {
        set_error("No valid input data to select data from.\n");
        return ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              vector_is_matrix_row
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int vector_is_matrix_row(const Matrix* mp, const Vector* vp)
{
    int i, j;
    int num_rows = mp->num_rows;
    int num_cols = mp->num_cols;
    int this_row;


    if (vp->length != num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        this_row = TRUE;

        for (j=0; j<num_cols; j++)
        {
            if ( ! IS_EQUAL_DBL(mp->elements[ i ][ j ], vp->elements[ j ]))
            {
                this_row = FALSE;
                break;
            }
        }

        if (this_row) return TRUE;
    }

    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

void debug_verify_matrix_vector
(
    const Matrix_vector* mvp,
    Bool*                failed_ptr, 
    const char*          file_name,
    int                  line_number

)
{
    int i, length;
    Bool failed = FALSE;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }

    if (mvp == NULL) return;

    length = mvp->length;

    for (i = 0; i < length; i++)
    {
        debug_verify_matrix(mvp->elements[ i], &failed, file_name, line_number);

        if ((failed_ptr != NULL) && (failed))
        {
            *failed_ptr = TRUE; 
        }
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

void debug_verify_probability_matrix_vector
(
    const Matrix_vector* mvp,
    Bool*                failed_ptr, 
    const char*          file_name,
    int                  line_number
)
{
    int i, length;
    Bool failed = FALSE;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }

    if (mvp == NULL) return;

    length = mvp->length;

    for (i = 0; i < length; i++)
    {
        debug_verify_probability_matrix(mvp->elements[ i], &failed, file_name, line_number);

        if ((failed_ptr != NULL) && (failed))
        {
            *failed_ptr = TRUE; 
        }
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

void debug_verify_probability_row_matrix_vector
(
    const Matrix_vector* mvp,
    Bool*                failed_ptr, 
    const char*          file_name,
    int                  line_number
)
{
    int i, length;
    Bool failed = FALSE;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }
    if (mvp == NULL) return;

    length = mvp->length;

    for (i = 0; i < length; i++)
    {
        debug_verify_probability_row_matrix(mvp->elements[ i], &failed, file_name, line_number);

        if ((failed_ptr != NULL) && (failed))
        {
            *failed_ptr = TRUE; 
        }
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

void debug_verify_non_negative_matrix_vector
(
    const Matrix_vector* mvp,
    Bool*                failed_ptr, 
    const char*          file_name,
    int                  line_number
)
{
    int i, length;
    Bool failed = FALSE;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }

    if (mvp == NULL) return;

    length = mvp->length;

    for (i = 0; i < length; i++)
    {
        debug_verify_non_negative_matrix(mvp->elements[ i], &failed, file_name, line_number);

        if ((failed_ptr != NULL) && (failed))
        {
            *failed_ptr = TRUE; 
        }
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void debug_verify_matrix
(
    const Matrix* mp,
    Bool*         failed_ptr, 
    const char*   file_name,
    int           line_number
)
{
    int i,j, num_rows, num_cols;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }

    if (mp == NULL) return;

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (    (isnand(mp->elements[ i ][ j ]))
                 || ( ! isfinited(mp->elements[ i ][ j ]))
               )
            {
                warn_pso("Matrix verification failed.\n");
                warn_pso("Element (%d, %d) is not ", i, j);

                if (isnand(mp->elements[ i ][ j ]))
                {
                    warn_pso("a number.\n");
                }
                else
                {
                    warn_pso("finite.\n");
                }

                warn_pso("Bits are %x, %x\n",
                         *((int*)&(mp->elements[ i ][ j ])),
                         *(1 + ((int*)&(mp->elements[ i ][ j ]))));
                warn_pso("Setting it to zero\n");
                mp->elements[ i ][ j ] = 0.0;
                warn_pso("Verification called from line %d of %s.\n",
                         line_number, file_name);

                if (failed_ptr != NULL) { *failed_ptr = TRUE; }
            }
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void debug_verify_non_negative_matrix
(
    const Matrix* mp,
    Bool*         failed_ptr,
    const char*   file_name,
    int           line_number
)
{
    int i,j, num_rows, num_cols;
    Bool failed = FALSE;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }

    if (mp == NULL) return;

    debug_verify_matrix(mp, &failed, file_name, line_number);

    if ((failed_ptr != NULL) && (failed))
    {
        *failed_ptr = TRUE; 
    }

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (mp->elements[ i ][ j ] < 0.0)
            {
                warn_pso("Non-negative matrix verification failed.\n");
                warn_pso("Element (%d, %d) is %.3e\n", i, j,
                         mp->elements[ i ][ j ]);
                warn_pso("Setting it to zero\n");
                mp->elements[ i ][ j ] = 0.0;
                warn_pso("Verification called from line %d of %s.\n",
                         line_number, file_name);

                if (failed_ptr != NULL) { *failed_ptr = TRUE; }
            }
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void debug_verify_probability_matrix
(
    const Matrix* mp,
    Bool*         failed_ptr,
    const char*   file_name,
    int           line_number
)
{
    int i,j, num_rows, num_cols;
    Bool failed = FALSE;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }

    if (mp == NULL) return;

    debug_verify_non_negative_matrix(mp, &failed, file_name, line_number);

    if ((failed_ptr != NULL) && (failed))
    {
        *failed_ptr = TRUE; 
    }

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (mp->elements[ i ][ j ] > 1.0)
            {
                warn_pso("Probability matrix verification failed.\n");
                warn_pso("Element (%d, %d) exceeds one by %.3e\n", i, j,
                         mp->elements[ i ][ j ] - 1.0);
                warn_pso("Setting it to zero\n");
                mp->elements[ i ][ j ] = 0.0;
                warn_pso("Verification called from line %d of %s.\n",
                         line_number, file_name);

                if (failed_ptr != NULL) { *failed_ptr = TRUE; }
            }
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void debug_verify_probability_row_matrix
(
    const Matrix* mp,
    Bool*         failed_ptr,
    const char*   file_name,
    int           line_number
)
{
    int i,j, num_rows, num_cols;
    Bool failed = FALSE;

    if (failed_ptr != NULL) { *failed_ptr = FALSE; }

    if (mp == NULL) return;

    debug_verify_non_negative_matrix(mp, &failed, file_name, line_number);

    if ((failed_ptr != NULL) && (failed))
    {
        *failed_ptr = TRUE; 
    }

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        double sum = 0.0;

        for (j = 0; j < num_cols; j++)
        {
            sum += mp->elements[ i ][ j ];
        }

        if (! IS_NEARLY_EQUAL_DBL(sum, 1.0, 1e-5))
        {
            warn_pso("Probability row matrix verification failed.\n");
            warn_pso("Row %d sum is %.3e\n", i, sum);
            warn_pso("Setting all elements equal\n");

            for (j = 0; j < num_cols; j++)
            {
                mp->elements[ i ][ j ] = 1.0 / (double)num_cols;
            }

            warn_pso("Verification called from line %d of %s.\n",
                     line_number, file_name);

            if (failed_ptr != NULL) { *failed_ptr = TRUE; }
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             stack_matrix_rows
 *
 * Copies matrix rows into a vector
 *
 * This routine copies matrix rows into a vector, left to right, top to bottom.
 *
 * Returns :
 *    If the routine fails, then ERROR is returned with and error message being
 *    set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int stack_matrix_rows(Vector** vpp, const Matrix* mp)
{
    if (mp == NULL)
    {
        free_vector(*vpp);
        *vpp = NULL;
    }
    else
    {
        int num_rows = mp->num_rows;
        int num_cols = mp->num_cols;
        int len = num_rows * num_cols;
        double* vec_pos;

        ERE(get_target_vector(vpp, len));
        vec_pos = (*vpp)->elements;

        if (kjb_use_memcpy)
        {
            UNTESTED_CODE();

            memcpy(vec_pos, mp->elements[ 0 ], len * sizeof(double));
        }
        else
        {
            int i, j;

            UNTESTED_CODE();


            for (i = 0; i < num_rows; i++)
            {
                for (j = 0; j < num_cols; j++)
                {
                    *vec_pos = mp->elements[ i ][ j ];
                    vec_pos++;
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             unstack_matrix_rows
 *
 * Copies vector into matrix rows
 *
 * This routine copies a vector into matrix rows, left to right, top to bottom.
 *
 * Returns :
 *    If the routine fails, then ERROR is returned with and error message being
 *    set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices, vectors
 *
 * -----------------------------------------------------------------------------
*/

int unstack_matrix_rows
(
    Matrix**      mpp,
    const Vector* vp,
    int           num_rows,
    int           num_cols
)
{
    if (vp == NULL)
    {
        free_matrix(*mpp);
        *mpp = NULL;
    }
    else
    {
        int len = vp->length;
        double* vec_pos = vp->elements;

        if (len != num_rows * num_cols)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        ERE(get_target_matrix(mpp, num_rows, num_cols));

        if (kjb_use_memcpy)
        {
            UNTESTED_CODE();

            memcpy((*mpp)->elements[ 0 ], vec_pos, len * sizeof(double));
        }
        else
        {
            int i, j;

            UNTESTED_CODE();


            for (i = 0; i < num_rows; i++)
            {
                for (j = 0; j < num_cols; j++)
                {
                    (*mpp)->elements[ i ][ j ] = *vec_pos;
                    vec_pos++;
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         randomize_matrix_rows
 *
 * Copies a matrix shuffling the rows randomly
 *
 * This routine copies the matrix into the one pointed to by target_mpp, but the
 * rows are randomized along the way.  If the matrix pointed to by target_mpp is
 * NULL, then a matrix of the appropriate size is created. If it exists, but is
 * the wrong size, then it is recycled. Otherwise, the storage is recycled. If
 * the source matrix is NULL, then the target matrix becomes NULL also, and any
 * storage associated with it is freed.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int randomize_matrix_rows(Matrix** target_mpp, const Matrix* source_mp)
{
    int             i, j, num_rows, num_cols;
    double*         target_pos;
    double*         source_pos;
    Indexed_vector* order_vp   = NULL;


    if (source_mp == NULL)
    {
        free_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    ERE(get_random_indexed_vector(&order_vp, num_rows));
    ERE(ascend_sort_indexed_vector(order_vp));

    for (i=0; i<num_rows; i++)
    {
        int source_i = order_vp->elements[ i ].index;

        source_pos = (source_mp->elements)[ source_i ];
        target_pos = ((*target_mpp)->elements)[ i ];

        for (j=0; j<num_cols; j++)
        {
            *target_pos = *source_pos;
            target_pos++;
            source_pos++;
        }
    }

    free_indexed_vector(order_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_max_thresh_matrix_ew(Matrix* target_mp, const Matrix* max_mp)
{
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    double** target_row_ptr;
    double** max_row_ptr;
    double*  target_ptr;
    double*  max_ptr;


    check_same_matrix_dimensions(target_mp, max_mp, "ow_max_thresh_matrix_ew");

    if (target_mp == NULL)
    {
        return NO_ERROR;
    }

    num_rows = target_mp->num_rows;
    num_cols = target_mp->num_cols;

    max_row_ptr = max_mp->elements;
    target_row_ptr = target_mp->elements;

    for (i=0; i<num_rows; i++)
    {
        target_ptr = *target_row_ptr;
        max_ptr = *max_row_ptr;

        for (j=0; j<num_cols; j++)
        {
            *target_ptr = MIN_OF(*target_ptr, *max_ptr);

            target_ptr++;
            max_ptr++;
        }

        target_row_ptr++;
        max_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        pad_matrix_by_extending
 *
 * Pads a matrix by extending the value at the edges of the matrix
 *
 * Instead of padding a matrix with zeros, it's often desirable for the padded
 * region to have the same value as the adjacent matrix elements.  For example,
 * consider the input matrx M:
 *
 *   M = [1 2]
 *       [3 4]
 *
 * Calling pad_matrix_by_extending(&M_padded, M, 2, 2, 2, 2) would result in:
 *
 *              [ 1 1 1 2 2 2 ]
 *              [ 1 1 1 2 2 2 ]
 *   M_padded = [ 1 1 1 2 2 2 ]
 *              [ 3 3 3 4 4 4 ]
 *              [ 3 3 3 4 4 4 ]
 *              [ 3 3 3 4 4 4 ]
 *
 * This is useful in edge-detection, so edges aren't detected at the edges
 * of an image.
 *
 *
 * Returns :
 *    If the routine fails, then ERROR is returned with and error message being
 *    set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
*/
int pad_matrix_by_extending(
        Matrix** target_mpp,   /* Padded output matrix */
        const Matrix* src_mp, /* Input matrix */
        int left, /* Number of columns to add to left of matrix */
        int right, /* Number of columns to add to right of matrix */
        int top, /* Number of columns to add to top of matrix */
        int bottom) /* Number of columns to add to bottom of matrix */
{
    int row, col;
    int num_rows = src_mp->num_rows + top + bottom;
    int num_cols = src_mp->num_cols + left + right;

    if(src_mp == NULL) return NO_ERROR;

    if(*target_mpp == src_mp)
    {
        Matrix* temp_mp = NULL;

        ERE(pad_matrix_by_extending(
            &temp_mp, src_mp,
            left, right,
            top, bottom));

        SWAP_MATRICES(temp_mp, *target_mpp);

        return NO_ERROR;
    }

    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    for(row = 0; row < num_rows; row++)
    {
        int src_row = row - top;

        for(col = 0; col < num_cols; col++)
        {
            int src_col = col - left;

            src_col = CLAMP(src_col, 0, src_mp->num_cols-1);
            src_row = CLAMP(src_row, 0, src_mp->num_rows-1);

            (*target_mpp)->elements[row][col] = src_mp->elements[src_row][src_col];
        }
    }

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_matrix_trace
 *
 * Computes the trace of a matrix
 *
 * This routine computes the trace (the sum of the elements on the main diagonal)
 * of the matrix pointed to by source_vp, which must be square.
 *
 * Returns :
 *    If the routine fails, then ERROR is returned with and error message being
 *    set. Otherwise NO_ERROR is returned.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_matrix_trace(const Matrix* source_vp, double* trace)
{
    int i;

    if(source_vp == NULL || source_vp->num_rows != source_vp->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    *trace = 0.0;

    for(i = 0; i < source_vp->num_rows; i++)
    {
        *trace += source_vp->elements[i][i];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          is_matrix_diagonal
 *
 * Checks if a matrix is diagonal.
 *
 * This routine checks if a matrix is diagonal.
 *
 * Returns:
 *     TRUE if the M is diagonal and FALSE if not. If M is NULL or not square,
 * this routine returns ERROR.
 *
 * Index: matrices
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int is_matrix_diagonal(const Matrix* M)
{
    int i, j;

    if(M == NULL || M->num_rows != M->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 0; i < M->num_rows; i++)
    {
        for(j = 0; j < M->num_cols; j++)
        {
            if(i != j && M->elements[i][j] != 0)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             nonnan_matrix
 *
 * Computes a binary matrix of non-nan elements of a matrix
 *
 * This routine creates a binary matrix of the same size as mp, whose elements
 * are 1 if the corresponding element in mp is not NaN, and 0 otherwise. In other
 * words:
 *
 *      (*nonnans)->elements[i][j] = !isnand(mp->elements[i][j])
 *
 * Index: matrices
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise.
 *
 * Author:
 *    Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int nonnan_matrix(Int_matrix** nonnans, const Matrix* mp)
{
    int i, j;

    if(mp == NULL)
    {
        free_int_matrix(*nonnans);
        *nonnans = NULL;
        return NO_ERROR;
    }

    ERE(get_target_int_matrix(nonnans, mp->num_rows, mp->num_cols));
    for(i = 0; i < mp->num_rows; i++)
    {
        for(j = 0; j < mp->num_cols; j++)
        {
            (*nonnans)->elements[i][j] = !isnand(mp->elements[i][j]);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

