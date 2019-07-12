
/* $Id: l_int_matrix.c 21520 2017-07-22 15:09:04Z kobus $ */

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

#include "l/l_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "l/l_2D_arr.h"
#include "l/l_int_matrix.h"
#include "l/l_int_vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
static Int_matrix* debug_create_int_matrix(int num_rows, int num_cols,
                                           const char* file_name, int line_number);
#else 
static Int_matrix* create_int_matrix(int num_rows, int num_cols);
#endif 

static Int_matrix_vector* create_int_matrix_vector(int length);
static Int_vector_matrix* create_int_vector_matrix(int num_rows, int num_cols);
static Int_matrix_vector_vector* create_int_matrix_vector_vector(int length);

/* -------------------------------------------------------------------------- */

#ifdef TEST
int debug_int_mat_val
(
    const Int_matrix* mp,
    int               i,
    int               j,
    const char*       file_name,
    int               line_number
)
{
    if ((i < 0) || (i >= mp->num_rows) || (j < 0) || (j >= mp->num_cols))
    {

        set_bug("Bounds check error on line %d of file %s.",
                line_number, file_name);
        return 0;
    }
    else
    {
        return mp->elements[i][j];
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           same_int_matrix_dimensions
 *
 * Returns TRUE if two matrices have the same dimensions
 *
 * This routine returns TRUE if two matrices have the same dimensions and FALSE
 * otherwise. If either matrix is NULL, then TRUE is returned if both are NULL,
 * and FALSE otherwise.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int same_int_matrix_dimensions(const Int_matrix* first_mp,
                               const Int_matrix* second_mp)
{
    if ((first_mp == NULL) || (second_mp == NULL))
    {
        return (first_mp == second_mp);
    }
    else if ((first_mp->num_rows != second_mp->num_rows))
    {
        return FALSE;
    }
    else if ((first_mp->num_cols != second_mp->num_cols))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_zero_int_matrix
 *
 * Gets a zero integer matrix
 *
 * This routine is identical to get_target_int_matrix(), except that the
 * int_matrix is set to zero (regardless of whether it is created or reused).
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_zero_int_matrix(Int_matrix** target_mpp, int num_rows, int num_cols)
{


    ERE(get_target_int_matrix(target_mpp, num_rows, num_cols));

    return ow_zero_int_matrix(*target_mpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_initialized_int_matrix
 *
 * Gets an initialized target int_matrix
 *
 * This routine is identical to get_target_int_matrix(), except that the
 * int_matrix elements are set to a specified initial value (regardless of
 * whether it is created or reused).
 *
 * Index : integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_initialized_int_matrix(Int_matrix** target_mpp,
                               int num_rows, int num_cols, int initial_value)
{


    ERE(get_target_int_matrix(target_mpp, num_rows, num_cols));

    return ow_set_int_matrix(*target_mpp, initial_value);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_target_int_matrix
 *
 * Gets target Int_matrix
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of integer matrices. If *target_mpp is NULL, then this
 * routine creates the int_matrix. If it is not null, and it is the right size,
 * then this routine does nothing. If it is the wrong size, then it is resized.
 *
 * In order to be memory efficient, we free before resizing. If the resizing
 * fails, then the original contents of the *target_mpp will be lost.
 * However, target_mpp->elements will be set to NULL, so *target_mpp can
 * be safely sent to free_matrix().
 *
 * The sizes (num_rows and num_cols) must both be nonnegative.
 * Conventionally, num_rows and num_cols are both positive;
 * however, it is also acceptable to have num_rows and num_cols both zero.
 * Anything else (such as a mix of positive and zero sizes) is regarded as a
 * bug in the calling program -- set_bug(3) is called and (if it returns) 
 * ERROR is returned.
 *
 * Related:
 *    Int_matrix
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_get_target_int_matrix(Int_matrix** target_mpp,
                                int num_rows, int num_cols,
                                const char* file_name, int line_number)
{
    Int_matrix* out_mp = *target_mpp;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* Prevent nx0 or 0xn matrix */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (out_mp == NULL)
    {
        NRE(out_mp = debug_create_int_matrix(num_rows, num_cols, file_name,
                                             line_number));
        *target_mpp = out_mp;
    }
    else if ((num_cols == out_mp->num_cols) && (num_rows <= out_mp->num_rows))
    {
        out_mp->num_rows = num_rows;
    }
    else if (num_rows * num_cols <= out_mp->max_num_elements)
    {
        int** row_ptr          = out_mp->elements;
        int*  elem_ptr         = *row_ptr;
        int   i;
        int   max_num_elements = out_mp->max_num_elements;

        /*
         * Either the number of rows is to be increased, or the column size is
         * to change. In both cases the pointers need to be fixed. 
        */

        if (num_rows > out_mp->max_num_rows)
        {
            kjb_free(out_mp->elements);
            NRE(out_mp->elements = DEBUG_N_TYPE_MALLOC(int*, num_rows,
                                                       file_name, line_number));
            row_ptr = out_mp->elements;

            out_mp->max_num_rows = num_rows;
        }

        out_mp->max_num_cols = num_cols;

        for (i=0; i<num_rows; i++)
        {
            (*row_ptr) = elem_ptr;
            row_ptr++;
            elem_ptr += num_cols;
        }

        out_mp->num_rows = num_rows;
        out_mp->num_cols = num_cols;

        ASSERT_IS_NOT_GREATER_INT(num_cols, out_mp->max_num_cols); 
        ASSERT_IS_NOT_GREATER_INT(num_rows, out_mp->max_num_rows); 

        /*
         * Note that it is not requried that the following holds with
         * out_mp->max_num_rows instead of out_mp->num_rows. The former only
         * tells us the capacity of the pointer array. 
        */
        ASSERT_IS_NOT_GREATER_INT(out_mp->num_rows * out_mp->max_num_cols, max_num_elements); 
    }
    else
    {
        free_2D_int_array(out_mp->elements);

        out_mp->elements = debug_allocate_2D_int_array(num_rows,
                                                       num_cols,
                                                       file_name,
                                                       line_number);

        if ((num_rows != 0) && (out_mp->elements == NULL))
        {
            return ERROR;
        }
                                                       
        out_mp->num_rows = num_rows;
        out_mp->num_cols = num_cols;
        out_mp->max_num_elements = num_rows * num_cols;
        out_mp->max_num_rows = num_rows;
        out_mp->max_num_cols = num_cols;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_int_matrix(Int_matrix** target_mpp, int num_rows, int num_cols)
{
    Int_matrix* out_mp = *target_mpp;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* Prevent nx0 or 0xn matrix */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (out_mp == NULL)
    {
        NRE(out_mp = create_int_matrix(num_rows, num_cols));
        *target_mpp = out_mp;
    }
    else if ((num_cols == out_mp->num_cols) && (num_rows <= out_mp->num_rows))
    {
        out_mp->num_rows = num_rows;
    }
    else if (num_rows * num_cols <= out_mp->max_num_elements)
    {
        int** row_ptr = out_mp->elements;
        int* elem_ptr = *row_ptr;
        int     i;

        /*
         * Either the number of rows is to be increased, or the column size is
         * to change. In both cases the pointers need to be fixed. 
        */

        if (num_rows > out_mp->max_num_rows)
        {
            kjb_free(out_mp->elements);
            NRE(out_mp->elements = N_TYPE_MALLOC(int*, num_rows));
            row_ptr = out_mp->elements;

            out_mp->max_num_rows = num_rows;
        }

        out_mp->max_num_cols = num_cols;

        for (i=0; i<num_rows; i++)
        {
            (*row_ptr) = elem_ptr;
            row_ptr++;
            elem_ptr += num_cols;
        }

        out_mp->num_cols = num_cols;
        out_mp->num_rows = num_rows;
    }
    else
    {
        free_2D_int_array(out_mp->elements);

        out_mp->elements = allocate_2D_int_array(num_rows, num_cols);

        if ((num_rows != 0) && (out_mp->elements == NULL))
        {
            return ERROR; 
        }

        out_mp->num_rows = num_rows;
        out_mp->num_cols = num_cols;
        out_mp->max_num_elements = num_rows * num_cols;
        out_mp->max_num_rows = num_rows;
        out_mp->max_num_cols = num_cols;
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                ra_get_target_int_matrix
 *
 * Gets target Int_matrix
 *
 * This routine is similar to get_target_int_matrix(), except that existing
 * storage is preserved. Further, if the number of rows is decreased, the amount
 * of storage claimed by the matrix is reduced. This is in contrast to
 * get_target_int_matrix() where simply the number of effective rows is simply
 * changed, and the storage remains allocated for possible future use if a
 * subsequent call wants to bump up the storage use. 
 *
 * Related:
 *    Int_matrix
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_ra_get_target_int_matrix
(
    Int_matrix** target_mpp,
    int          num_rows,
    int          num_cols,
    const char*  file_name,
    int          line_number 
)
{
    Int_matrix* out_mp = *target_mpp;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* Prevent nx0 or 0xn matrix */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (    (out_mp == NULL) 
         || (out_mp->num_rows == 0) || (out_mp->num_cols == 0)
         || (num_rows == 0) || (num_cols == 0)
       )
    {
        free_int_matrix(out_mp); 
        NRE(out_mp = debug_create_int_matrix(num_rows, num_cols, file_name,
                                             line_number));
        *target_mpp = out_mp;
    }
    else if (num_cols == out_mp->num_cols) 
    {
        if (num_rows == out_mp->num_rows)
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else 
        {
            int*  elem_ptr;
            int   i;

            elem_ptr = out_mp->elements[ 0 ];

            NRE(elem_ptr = DEBUG_INT_REALLOC(elem_ptr,
                                             num_rows * num_cols,
                                             file_name, line_number));
            kjb_free(out_mp->elements);

            NRE(out_mp->elements = DEBUG_N_TYPE_MALLOC(int*, num_rows,
                                                       file_name, line_number));

            for (i = 0; i < num_rows; i++)
            {
                out_mp->elements[ i ] = elem_ptr;
                elem_ptr += num_cols;
            }

            out_mp->num_rows = num_rows;
            out_mp->max_num_elements = num_rows * num_cols;
            out_mp->max_num_rows = num_rows;
        }
    }
    else
    {
        /*
         * The number of columns has changed. Hence we need to reorganize the
         * exisiting elements.
        */
        Int_matrix* temp_mp = NULL;
        int preserved_rows = MIN_OF(num_rows, out_mp->num_rows);
        int preserved_cols = MIN_OF(num_cols, out_mp->num_cols);

        ERE(debug_get_target_int_matrix(&temp_mp, 
                                        preserved_rows, 
                                        preserved_cols,
                                        file_name, line_number)); 

        ERE(ow_copy_int_matrix_block(temp_mp, 0, 0, out_mp, 0, 0, 
                                     preserved_rows, preserved_cols)); 

        ERE(debug_get_target_int_matrix(target_mpp, num_rows, num_cols, 
                                        file_name, line_number));
        ERE(ow_set_int_matrix(*target_mpp, NOT_SET));

        ERE(ow_copy_int_matrix(*target_mpp, 0, 0, temp_mp));  

        free_int_matrix(temp_mp); 
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int ra_get_target_int_matrix(Int_matrix** target_mpp, int num_rows, int num_cols)
{
    Int_matrix* out_mp = *target_mpp;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* Prevent nx0 or 0xn matrix */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (    (out_mp == NULL) 
         || (out_mp->num_rows == 0) || (out_mp->num_cols == 0)
         || (num_rows == 0) || (num_cols == 0)
       )
    {
        free_int_matrix(out_mp); 
        NRE(out_mp = create_int_matrix(num_rows, num_cols));
        *target_mpp = out_mp;
    }
    else if (num_cols == out_mp->num_cols) 
    {
        if (num_rows == out_mp->num_rows)
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else 
        {
            int*  elem_ptr;
            int   i;

            UNTESTED_CODE();

            elem_ptr = out_mp->elements[ 0 ];

            NRE(elem_ptr = INT_REALLOC(elem_ptr, num_rows * num_cols));

            kjb_free(out_mp->elements);

            NRE(out_mp->elements = N_TYPE_MALLOC(int*, num_rows));

            for (i = 0; i < num_rows; i++)
            {
                out_mp->elements[ i ] = elem_ptr;
                elem_ptr += num_cols;
            }

            out_mp->num_rows = num_rows;
            out_mp->max_num_elements = num_rows * num_cols;
            out_mp->max_num_rows = num_rows;
        }
    }
    else
    {
        /*
         * The number of columns has changed. Hence we need to reorganize the
         * exisiting elements.
        */
        Int_matrix* temp_mp = NULL;
        int preserved_rows = MIN_OF(num_rows, out_mp->num_rows);
        int preserved_cols = MIN_OF(num_cols, out_mp->num_cols);

        UNTESTED_CODE(); 

        ERE(get_target_int_matrix(&temp_mp, 
                                  preserved_rows, 
                                  preserved_cols)); 

        ERE(ow_copy_int_matrix_block(temp_mp, 0, 0, out_mp, 0, 0, 
                                     preserved_rows, preserved_cols)); 

        ERE(get_target_int_matrix(target_mpp, num_rows, num_cols));

        ERE(ow_copy_int_matrix(*target_mpp, 0, 0, temp_mp));  

        free_int_matrix(temp_mp); 
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static Int_matrix* debug_create_int_matrix(int num_rows, int num_cols,
                                           const char* file_name, int line_number)
{
    Int_matrix* mp;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(mp = DEBUG_TYPE_MALLOC(Int_matrix, file_name, line_number));

    mp->elements = debug_allocate_2D_int_array(num_rows, num_cols,
                                               file_name, line_number);
    
    if ((num_rows != 0) && (mp->elements == NULL))
    {
        kjb_free(mp);
        return NULL; 
    }
                                                       
    mp->num_rows = num_rows;
    mp->num_cols = num_cols;
    mp->max_num_elements = num_rows * num_cols;
    mp->max_num_rows = num_rows;
    mp->max_num_cols = num_cols;

    return mp;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

static Int_matrix* create_int_matrix(int num_rows, int num_cols)
{
    Int_matrix* mp;


    NRN(mp = TYPE_MALLOC(Int_matrix));

    mp->elements = allocate_2D_int_array(num_rows, num_cols);

    if ((num_rows != 0) && (mp->elements == NULL))
    {
        kjb_free(mp);
        return NULL; 
    }
                                                       
    mp->num_rows = num_rows;
    mp->num_cols = num_cols;
    mp->max_num_elements = num_rows * num_cols;
    mp->max_num_rows = num_rows;
    mp->max_num_cols = num_cols;

    return mp;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        get_diagonal_int_matrix
 *
 * Gets a diagonal int matrix from an int vector
 *
 * This routine creates a diagonal int matrix from an int vector provided as the
 * argument.
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

int get_diagonal_int_matrix(Int_matrix** mpp, const Int_vector* vp)
{
    Int_matrix* d_mp;
    int     i;


    ERE(get_zero_int_matrix(mpp, vp->length, vp->length));
    d_mp = *mpp;

    for (i = 0; i < vp->length; i++)
    {
        (d_mp->elements)[i][i] = (vp->elements)[i];
     }

     return NO_ERROR;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_int_matrix
 *
 * Frees a the space associated with a int_matrix.
 *
 * This routine frees the storage associated with an Int_matrix obtained from
 * get_target_int_matrix(),  perhaps indirectrly. If the argument is NULL, then
 * this routine returns safely without doing anything.
 *
 * Related:
 *    Int_matrix
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_int_matrix(Int_matrix* mp)
{


    if (mp != NULL)
    {
        if (mp->elements != NULL)
        {
#if 0 /* was ifdef DISABLE */
#ifdef TRACK_MEMORY_ALLOCATION
            /*
             * We must have a valid pointer to check the initialization,
             * otherwise problems such as double free can look like unitialized
             * memory.
            */
            kjb_check_free(mp);

            check_initialization(mp->elements[ 0 ], mp->num_rows * mp->num_cols,
                                 sizeof(int));
#endif
#endif

            free_2D_int_array(mp->elements);
        }

        kjb_free(mp);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#if 0 /* was ifdef OBSOLETE */

static Int_matrix* create_zero_int_matrix(int num_rows, int num_cols)
{
    Int_matrix* output_mp;


    NRN(output_mp = create_int_matrix(num_rows, num_cols));

    if (ow_zero_int_matrix(output_mp) == ERROR)
    {
        free_int_matrix(output_mp);
        return NULL;
    }

    return output_mp;
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ow_zero_int_matrix
 *
 * Sets all elements of a int_matrix to zero
 *
 * This routine sets all elements of a int_matrix to zero. The int_matrix must
 * have already been created.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure. Failure is only possible if the
 *     int_matrix is NULL or malformed, and this is treated as a bug.
 *
 * Index : integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_zero_int_matrix(Int_matrix* mp)
{

    return ow_set_int_matrix(mp, 0);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  ow_set_int_matrix
 *
 * Sets all elements of a int_matrix to the specified value
 *
 * This routine sets all elements of a int_matrix to the specified value. The
 * int_matrix must have already been created.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure. Failure is only possible if the
 *     int_matrix is NULL or malformed, and this is treated as a bug.
 *
 * Index : integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_set_int_matrix(Int_matrix* mp, int set_val)
{
    int   i;
    int   j;
    int* row_pos;


    if (mp->num_cols == 0) return NO_ERROR;

    for (i=0; i<mp->num_rows; i++)
    {
        row_pos = (mp->elements)[ i ];

        for (j=0; j<mp->num_cols; j++)
        {
            *row_pos++ = set_val;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     ra_get_target_int_matrix_vector
 *
 * Gets a target integer matrix vector
 *
 * This routine is similar to get_target_int_matrix_vector() except that
 * existing data that fits into the new size is preserved. 
 *
 * The routine free_int_matrix_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Index: memory allocation, integer matrices, integer matrix vectors
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure.
 *
 * -----------------------------------------------------------------------------
*/

int ra_get_target_int_matrix_vector(Int_matrix_vector **mvpp, int length)
{
    Int_matrix_vector* mvp = *mvpp;

    
    UNTESTED_CODE();
    
    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ((mvp == NULL) || (mvp->length == 0) || (length == 0))
    {
        return get_target_int_matrix_vector(mvpp, length);
    }
    else
    {
        int prev_len = mvp->length;
        int i;

        for (i = length; i < prev_len; i++)
        {
            free_int_matrix(mvp->elements[ i ]); 
        }

        NRE(mvp->elements = N_TYPE_REALLOC(mvp->elements, Int_matrix*, length));

        for (i = prev_len; i < length; i++)
        {
            mvp->elements[ i ] = NULL;
        }

        mvp->length = length; 
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     get_target_int_matrix_vector
 *
 * Gets a target integer matrix vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of matrix vectors. If *target_mvpp is
 * NULL, then this routine creates the matrix matrix. If it is not null, and it
 * is the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_int_matrix_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Index: memory allocation, integer matrices, integer matrix vectors
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure.
 *
 * -----------------------------------------------------------------------------
*/

int get_target_int_matrix_vector(Int_matrix_vector **mvpp, int length)
{


    if (length < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (    (*mvpp != NULL)
         && ((*mvpp)->length == length)
       )
    {
        return NO_ERROR;
    }

    free_int_matrix_vector(*mvpp);
    NRE(*mvpp = create_int_matrix_vector(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Int_matrix_vector* create_int_matrix_vector(int length)
{
    Int_matrix_vector* int_mvp;
    int           i;


    NRN(int_mvp = TYPE_MALLOC(Int_matrix_vector));
    int_mvp->length = length;
    NRN(int_mvp->elements = N_TYPE_MALLOC(Int_matrix*, length));

    for (i=0; i<length; i++)
    {
        int_mvp->elements[ i ] = NULL;
    }

    return int_mvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_int_matrix_vector
 *
 * Frees a the space associated with a Int_matrix_vector.
 *
 * This routine frees the storage associated with a Int_matrix_matrix obtained
 * from get_target_int_matrix_vector(), perhaps indirectrly. If the argument is
 * NULL, then this routine returns safely without doing anything.
 *
 * Related:
 *    Int_matrix
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_int_matrix_vector(Int_matrix_vector *int_mvp)
{
    int         count;
    int         i;
    Int_matrix** mp_array_pos;


    if (int_mvp == NULL) return;

    mp_array_pos = int_mvp->elements;
    count = int_mvp->length;

    for (i=0; i<count; i++)
    {
        free_int_matrix(*mp_array_pos);
        mp_array_pos++;
    }

    kjb_free(int_mvp->elements);
    kjb_free(int_mvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#if 0 /* was ifdef OBSOLETE */
/* Should be revived as get_target_int_matrix_vector_vector() */

Int_matrix_vector** create_int_matrix_vector_list(int count)
{
    Int_matrix_vector** int_matrix_vector_list;
    int            i;


    UNTESTED_CODE();

    NRN(int_matrix_vector_list = N_TYPE_MALLOC(Int_matrix_vector*, count));

    for (i=0; i<count; i++)
    {
        int_matrix_vector_list[ i ] = NULL;
    }

    return int_matrix_vector_list;
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#if 0 /* was ifdef OBSOLETE */
/* Should be revived as free_int_matrix_vector_vector() */

void free_int_matrix_vector_list(int count,
                                 Int_matrix_vector **int_matrix_vector_list)
{
    int i;


    if (int_matrix_vector_list == NULL) return;

    for (i=0; i<count; i++)
    {
        free_int_matrix_vector(int_matrix_vector_list[ i ]);
    }

    kjb_free(int_matrix_vector_list);
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     get_target_int_vector_matrix
 *
 * Gets a target integer vector matrix
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of integer vector matrices. If *target_vmpp is NULL, then
 * this routine creates the vector matrix. If it is not null, and it is the
 * right size, then this routine does nothing. If it is the wrong size, then it
 * is resized.
 *
 * The routine free_vector_matrix should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Index: memory allocation, integer matrices, integer vector matrices
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * -----------------------------------------------------------------------------
*/

int get_target_int_vector_matrix(Int_vector_matrix **vmpp, int num_rows, int num_cols)
{

    if (    (*vmpp != NULL)
         && ((*vmpp)->num_rows == num_rows)
         && ((*vmpp)->num_cols == num_cols)
       )
    {
        return NO_ERROR;
    }

    free_int_vector_matrix(*vmpp);
    NRE(*vmpp = create_int_vector_matrix(num_rows, num_cols));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Int_vector_matrix* create_int_vector_matrix(int num_rows, int num_cols)
{
    Int_vector_matrix* int_vmp;
    int           i, j;


    NRN(int_vmp = TYPE_MALLOC(Int_vector_matrix));
    int_vmp->num_rows = num_rows;
    int_vmp->num_cols = num_cols;

    int_vmp->elements = (Int_vector***)allocate_2D_ptr_array(num_rows, num_cols);

    if ((num_rows != 0) && (int_vmp->elements == NULL))
    {
        kjb_free(int_vmp);
        return NULL; 
    }
                                                       
    for (i=0; i<num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            int_vmp->elements[ i ][ j ] = NULL;
        }
    }

    return int_vmp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_int_vector_matrix
 *
 * Frees a the space associated with a Int_vector_matrix.
 *
 * This routine frees the storage associated with a Int_vector_matrix obtained
 * from get_target_int_vector_matrix(), perhaps indirectrly. If the argument is
 * NULL, then this routine returns safely without doing anything.
 *
 * Related:
 *    Int_matrix
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_int_vector_matrix(Int_vector_matrix *int_vmp)
{
    int num_rows, num_cols, i, j;


    if (int_vmp == NULL) return;

    num_rows = int_vmp->num_rows;
    num_cols = int_vmp->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            free_int_vector(int_vmp->elements[ i ][ j ]);
        }
    }

    free_2D_ptr_array((void***)(int_vmp->elements)); 

    kjb_free(int_vmp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        concat_int_matrices_vertically
 *
 * Concatenates integer matrices verically
 *
 * This routine takes an array of integer matrices and forms one int_matrix
 * consisting of those integer matrices stacked on top of each other.
 *
 * The first argument is the adress of the target int_matrix. If the target
 * int_matrix itself is null, then a int_matrix of the appropriate size is
 * created. If the target int_matrix is the wrong size, it is resized. Finally,
 * if it is the right size, then the storage is recycled, as is.
 *
 * The int_matrix array may contain any number of NULL integer matrices. If there
 * are only NULL integer matrices, or if num_integer_matrices is zero, then the
 * target int_matrix is freed and set to NULL.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int concat_int_matrices_vertically(Int_matrix** mpp, int num_int_matrices,
                                   const Int_matrix** int_matrix_list)
{
    int i, j, k, count;
    int num_rows = 0;
    int num_cols = NOT_SET;
    Int_matrix* mp;


    if (num_int_matrices < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (num_int_matrices == 0)
    {
        free_int_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }

    for (i=0; i<num_int_matrices; i++)
    {
        if (int_matrix_list[ i ] != NULL)
        {
            if (num_cols == NOT_SET)
            {
                num_cols = (int_matrix_list[ i ])->num_cols;
            }
            else if ((int_matrix_list[ i ])->num_cols != num_cols)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
            num_rows += ((int_matrix_list[ i ])->num_rows);
        }
    }

    if (num_rows == 0)
    {
        free_int_matrix(*mpp);
        *mpp = NULL;
        return NO_ERROR;
    }

    ERE(get_target_int_matrix(mpp, num_rows, num_cols));
    mp = *mpp;

    count = 0;

    while (count < num_rows)
    {
        for (k=0; k<num_int_matrices; k++)
        {
            if (int_matrix_list[ k ] != NULL)
            {
                for (i=0; i<(int_matrix_list[ k ])->num_rows; i++)
                {
                    for (j=0; j<num_cols; j++)
                    {
                        mp->elements[ count ][ j ] =
                                     (int_matrix_list[ k ])->elements[ i ][ j ];
                    }
                    count++;
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#if 0 /* was ifdef OBSOLETE */
/*
 * =============================================================================
 *                               create_int_matrix_copy
 *
 * Creates a copy of a integer array
 *
 * This routine creates a copy of a integer array and returns a pointer to it.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then NULL is returned
 *    with and error message being set. Otherwise a pointer to a newly created
 *    copy of the input integer array is returned.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

Int_matrix* create_int_matrix_copy(const Int_matrix* source_mp)
{
    Int_matrix* target_mp = NULL;


    ERN(copy_int_matrix(&target_mp, source_mp));

    return target_mp;
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int split_int_matrix_by_rows
(
     Int_matrix** target_1_mpp,
     Int_matrix** target_2_mpp,
     const Int_matrix* source_mp,
     const Int_vector* index_list_vp
)
{
    int   i;
    int   num_rows;
    int   num_cols;
    int   num_index_rows;
    int target_1_index = 0;
    int target_2_index = 0;


    UNTESTED_CODE();

    if (source_mp == NULL)
    {
        free_int_matrix(*target_1_mpp);
        free_int_matrix(*target_2_mpp);
        *target_1_mpp = NULL;
        *target_2_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;
    num_index_rows = index_list_vp->length;

    ERE(get_target_int_matrix(target_1_mpp, num_index_rows, num_cols));
    ERE(get_target_int_matrix(target_2_mpp, num_rows - num_index_rows, num_cols));

    for (i=0; i<num_rows; i++)
    {
        if (i == index_list_vp->elements[ target_1_index ])
        {
            copy_int_matrix_row(*target_1_mpp,target_1_index,source_mp,i);
            target_1_index++;
        }
        else
        {
            copy_int_matrix_row(*target_2_mpp,target_2_index,source_mp,i);
            target_2_index++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                copy_int_matrix
 *
 * Copies a integer array
 *
 * This routine copies the integer array into the one pointed to by
 * target_mpp. If the integer array pointed to by target_mpp is NULL,
 * then a integer array of the appropriate size is created. If it exists, but
 * is the wrong size, then it is recycled. Otherwise, the storage is recycled.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_copy_int_matrix(Int_matrix** target_mpp, const Int_matrix* source_mp,
                          const char* file_name, int line_number)
{
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    int* target_pos;
    int* source_pos;


    if (source_mp == NULL)
    {
        free_int_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(debug_get_target_int_matrix(target_mpp, num_rows, num_cols, file_name,
                                line_number));

    if(num_rows == 0 || num_cols == 0) 
    {
        return NO_ERROR;
    }

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

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int copy_int_matrix(Int_matrix** target_mpp, const Int_matrix* source_mp)
{
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    int* target_pos;
    int* source_pos;


    UNTESTED_CODE();

    if (source_mp == NULL)
    {
        free_int_matrix(*target_mpp);
        *target_mpp = NULL;
        return NO_ERROR;
    }

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_int_matrix(target_mpp, num_rows, num_cols));

    if(num_rows == 0 || num_cols == 0) 
    {
        return NO_ERROR;
    }

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

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_copy_int_matrix
 *
 * Copies an integer matrix to an offset position of a target integer matrix
 *
 * This routine copies source_mp into target_mp, offset by (target_row_offset,
 * target_col_offset).
 *
 * Returns :
 *    If the routine fails (can really happen unless there is an argument
 *    error), then ERROR is returned with and error message being set.
 *    Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int ow_copy_int_matrix(Int_matrix* target_mp,
                       int target_row_offset, int target_col_offset,
                       const Int_matrix* source_mp)
{
    int  i, j, num_rows, num_cols;
    int* target_pos;
    int* source_pos;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    if (target_row_offset + num_rows > target_mp->num_rows)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (target_col_offset + num_cols > target_mp->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i=0; i<num_rows; i++)
    {
        source_pos = source_mp->elements[ i ];
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
 *                            copy_int_matrix_block
 *
 * Copies a matrix block to target matrix
 *
 * This routine copies a block of source_mp into *target_mpp which is created or
 * resized if necessary.
 *
 * Returns :
 *    If the routine fails then ERROR is returned with and error message being
 *    set.  Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int copy_int_matrix_block
(
    Int_matrix**      target_mpp,
    const Int_matrix* source_mp,
    int               source_row_offset,
    int               source_col_offset,
    int               num_rows,
    int               num_cols
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

    ERE(get_target_int_matrix(target_mpp, num_rows, num_cols));

    ERE(ow_copy_int_matrix_block(*target_mpp, 0, 0,
                                 source_mp,
                                 source_row_offset, source_col_offset,
                                 num_rows, num_cols));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_copy_int_matrix_block
 *
 * Copies a integer matrix block to an offset position of a target integer matrix
 *
 * This routine copies a block of source_mp into target_mp, offset by
 * (target_row_offset, target_col_offset).
 *
 * Returns :
 *    If the routine fails (can really happen unless there is an argument
 *    error), then ERROR is returned with and error message being set.
 *    Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int ow_copy_int_matrix_block(Int_matrix* target_mp,
                             int target_row_offset,
                             int target_col_offset,
                             const Int_matrix* source_mp,
                             int source_row_offset,
                             int source_col_offset,
                             int num_rows,
                             int num_cols)
{
    int i, j;
    int* target_pos;
    int* source_pos;


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

int copy_int_matrix_vector(Int_matrix_vector **out_int_mvpp,
                           const Int_matrix_vector *in_int_mvp)
{
    Int_matrix** out_mvp;
    Int_matrix** in_mvp;
    int         num_int_matrices;
    int         i;


    UNTESTED_CODE();

    if (in_int_mvp == NULL)
    {
        free_int_matrix_vector(*out_int_mvpp);
        *out_int_mvpp = NULL;
        return NO_ERROR;
    }

    num_int_matrices = in_int_mvp->length;

    if (    (*out_int_mvpp != NULL)
         && ((*out_int_mvpp)->length != num_int_matrices)
       )
    {
        free_int_matrix_vector(*out_int_mvpp);
        *out_int_mvpp = NULL;
    }

    if (*out_int_mvpp == NULL)
    {
        NRE(*out_int_mvpp = create_int_matrix_vector(num_int_matrices));
    }

    out_mvp = (*out_int_mvpp)->elements;
    in_mvp  = in_int_mvp->elements;

    for (i=0; i<num_int_matrices; i++)
    {
        ERE(copy_int_matrix(&(out_mvp[ i ]), in_mvp[ i ]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                copy_int_matrix_row
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
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices, integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int copy_int_matrix_row(Int_matrix* target_mp, int target_row_num,
                        const Int_matrix* source_mp, int source_row_num)
{

    int len, i;
    int* target_row_pos;
    int* source_row_pos;


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
 *                             copy_int_matrix_col
 *
 * Replaces an integer matrix column with a column from another
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
 * Index: integer matrices, integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int copy_int_matrix_col
(
    Int_matrix*       target_mp,
    int               target_col_num,
    const Int_matrix* source_mp,
    int               source_col_num
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
 *                                get_int_matrix_row
 *
 * Returns an integer matrix row
 *
 * This routine gets row "row_num" from the integer matrix "mp" and puts it
 * into the integer vector pointed to by vpp. If *vpp is NULL, then the vector
 * is created.  If it is the wrong length, then it is resized. Otherwise, the
 * storage is reused.
 *
 * Note :
 *     Int_matrix row counting begins with 0.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices, integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_int_matrix_row(Int_vector** vpp, const Int_matrix* mp, int row_num)
{
    int  len, i;
    int* row_pos;
    int* vect_pos;


    if ((row_num < 0) || (row_num >= mp->num_rows))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = mp->num_cols;

    ERE(get_target_int_vector(vpp, len));

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
 *                                get_int_matrix_col
 *
 * Returns an integer matrix column
 *
 * This routine gets column "col_num" from the integer matrix "mp" and puts it
 * into the integer vector pointed to by vpp. If *vpp is NULL, then the vector
 * is created.  If it is the wrong length, then it is resized. Otherwise, the
 * storage is reused.
 *
 * Note :
 *     Int_matrix column counting begins with 0.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices, integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_int_matrix_col(Int_vector** vpp, const Int_matrix* mp, int col_num)
{
    int  len, i;
    int* vect_pos;


    if ((col_num < 0) || (col_num >= mp->num_cols))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = mp->num_rows;

    ERE(get_target_int_vector(vpp, len));

    vect_pos = (*vpp)->elements;

    for (i=0; i<len; i++)
    {
        *vect_pos++ = mp->elements[ i ][ col_num ];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                put_int_matrix_row
 *
 * Replaces an integer matrix row with the contents of an integer vector
 *
 * This routine replaces row "row_num" of the integer matrix "mp" with the
 * contents of the integer vector vp.
 *
 * Note :
 *     Int_matrix row counting begins with 0.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices, integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int put_int_matrix_row(Int_matrix* mp, const Int_vector* vp, int row_num)
{
    int  len, i;
    int* row_pos;
    int* vect_pos;


    if (    (row_num < 0) || (row_num >= mp->num_rows)
         || (mp->num_cols != vp->length))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    len = vp->length;

    row_pos = mp->elements[ row_num ];
    vect_pos = vp->elements;

    for (i=0; i<len; i++)
    {
        *row_pos = *vect_pos;
        row_pos++;
        vect_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                put_int_matrix_col
 *
 * Replaces an integer matrix column with the contents of an integer vector
 *
 * This routine replaces column "col_num" of the integer matrix "mp" with the
 * contents of integer vector vp.
 *
 * Note :
 *     Int_matrix column counting begins with 0.
 *
 * Returns :
 *    If the routine fails (due to storage allocation), then ERROR is returned
 *    with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Index: integer matrices, integer vectors
 *
 * -----------------------------------------------------------------------------
*/

int put_int_matrix_col(Int_matrix* mp, const Int_vector* vp, int col_num)
{
    int   len;
    int   i;
    int* vect_pos;


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
        mp->elements[ i ][ col_num ] = *vect_pos;
        vect_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              get_int_transpose
 *
 * Gets the transposes an integer matrix
 *
 * This routine produces an integer matrix which is the transpose of the
 * argument.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then an integer matrix of the appropriate size is created.
 * If the target matrix is the wrong size, it is resized. Finally, if it is the
 * right size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int get_int_transpose(Int_matrix** target_mpp, const Int_matrix* mp)
{
    Int_matrix* target_mp;
    int     i;
    int     j;
    int     result = NO_ERROR;


    if (mp == NULL)
    {
        free_int_matrix(*target_mpp);
        *target_mpp = NULL;
    }
    /*
     * If it so happens that the user has made the target matrix the same as the
     * input matrix, then we need to compute the result into a temporary
     * location and copy the result, otherwise we will be changing the input as
     * we compute the answer. 
    */
    else if (*target_mpp == mp)
    {
        Int_matrix* temp_mp = NULL;

        result = get_int_transpose(&temp_mp, mp);

        if (result != ERROR)
        {
            result = copy_int_matrix(target_mpp, temp_mp);
        }

        free_int_matrix(temp_mp);
    }
    else 
    {
        ERE(get_target_int_matrix(target_mpp, mp->num_cols, mp->num_rows));
        target_mp = *target_mpp;

        for (i = 0; i < mp->num_rows; i++)
        {
            for (j = 0; j < mp->num_cols; j++)
            {
                (target_mp->elements)[j][i] = (mp->elements)[i][j];
             }
         }
    }

     return result;
 }

/*
 * =============================================================================
 *                            get_int_identity_matrix
 *
 * Gets an identity matrix of the specified size
 *
 * This routine puts an identity matrix of the specified size into
 * *output_mpp. If *output_mpp is NULL, then the matrix is created. If it
 * is the wrong sized, then it is resized. Otherwise, the storage is recycled.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
 */

int get_int_identity_matrix(Int_matrix** output_mpp, int size)
{
    Int_matrix* output_mp;
    int i;


    ERE(get_zero_int_matrix(output_mpp, size, size));
    output_mp = *output_mpp;

    for (i=0; i<size; i++)
    {
        (output_mp->elements)[i][i] = 1;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         swap_int_matrix_rows
 *
 * Replaces a matrix row with a row from another
 *
 * This routine swaps row "target_row_num" of the matrix "target_mp" with the
 * contents of "source_row_num" of "source_mp" and vice-versa. The same matrix
 * can be used as source and target.
 *
 * Note :
 *     Matrix row counting begins with 0.
 *
 * Returns :
 *    If the routine fails then ERROR is returned with and error message being
 *    set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *     Abin Shahab
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int swap_int_matrix_rows
(
    Int_matrix* target_mp,
    int         target_row_num,
    Int_matrix* source_mp,
    int         source_row_num
)
{
    int len, i, temp;
    int* target_row_pos;
    int* source_row_pos;

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
        temp = target_row_pos[i];
        target_row_pos[i] = source_row_pos[i];
        source_row_pos[i] = temp;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               min_int_matrix_element
 *
 * Returns the smallest element of an int matrix
 *
 * This routine returns the smallest element in the matrix specified by "mp".
 *
 * Returns:
 *     smallest matrix element.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int min_int_matrix_element(const Int_matrix* mp)
{
    int min;

    EPETE(get_min_int_matrix_element(mp, &min, NULL, NULL));

    return min;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_min_int_matrix_element(const Int_matrix* mp,
                               int* min_ptr, int* min_i_ptr, int* min_j_ptr)
{
    int    i;
    int    j;
    int   min;
    int** row_ptr;
    int*  element_ptr;
    int    min_i, min_j;

    if (    (mp == NULL)
         || (mp->num_rows == 0)
         || (mp->num_cols == 0)
       )
     {
         /*
         // This could be made to simply set an error message and return error,
         // but for now, treat it as a bug.
         */
         SET_ARGUMENT_BUG();
         return ERROR;
     }

    min = mp->elements[0][0];
    min_i = 0;
    min_j = 0;

    row_ptr =  mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            if (*element_ptr < min)
            {
                min = *element_ptr;
                min_i = i;
                min_j = j;
            }
            element_ptr++;
        }
        row_ptr++;
    }

    if (min_ptr != NULL) *min_ptr = min;
    if (min_i_ptr != NULL) *min_i_ptr = min_i;
    if (min_j_ptr != NULL) *min_j_ptr = min_j;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               max_int_matrix_element
 *
 * Returns the largest element of a matrix
 *
 * This routine returns the largest element in the matrix specified by "mp".
 *
 * Returns:
 *     largest matrix element.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int max_int_matrix_element(const Int_matrix* mp)
{
    int max;

    EPETE(get_max_int_matrix_element(mp, &max, NULL, NULL));

    return max;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_max_int_matrix_element(const Int_matrix* mp,
                               int* max_ptr, int* max_i_ptr, int* max_j_ptr)
{
    int    i;
    int    j;
    int   max;
    int** row_ptr;
    int*  element_ptr;
    int    max_i, max_j;


    if (    (mp == NULL)
         || (mp->num_rows == 0)
         || (mp->num_cols == 0)
       )
    {
         /*
         // This could be made to simply set an error message and return error,
         // but for now, treat it as a bug.
         */
         SET_ARGUMENT_BUG();
         return ERROR;
    }

    max = mp->elements[0][0];
    max_i = 0;
    max_j = 0;

    row_ptr =  mp->elements;

    for (i = 0; i<mp->num_rows; i++)
    {
        element_ptr = *row_ptr;

        for (j = 0; j<mp->num_cols; j++)
        {
            if (*element_ptr > max)
            {
                max = *element_ptr;
                max_i = i;
                max_j = j;
            }
            element_ptr++;
        }
        row_ptr++;
    }

    if (max_ptr != NULL) *max_ptr = max;
    if (max_i_ptr != NULL) *max_i_ptr = max_i;
    if (max_j_ptr != NULL) *max_j_ptr = max_j;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void debug_verify_non_negative_int_matrix
(
    const Int_matrix* mp,
    Bool*          failure_ptr, 
    const char*   file_name,
    int           line_number
)
{
    int i,j, num_rows, num_cols;

    if (failure_ptr != NULL) { *failure_ptr = TRUE; }

    if (mp == NULL) return;

    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (mp->elements[ i ][ j ] < 0)
            {
                warn_pso("Non-negative matrix verifcation failed.\n");
                warn_pso("Element (%d, %d) is %d\n", i, j,
                         mp->elements[ i ][ j ]);
                warn_pso("Setting it to zero\n");
                mp->elements[ i ][ j ] = 0;
                warn_pso("Verification called from line %d of %s.\n",
                         line_number, file_name);

                if (failure_ptr != NULL) { *failure_ptr = TRUE; }
            }
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          max_abs_int_matrix_difference
 *
 * Returns the max absolute difference between two matrices
 *
 * This routine calculates the maximum of the absolute value of the difference
 * of corrresponding elements of two dimension compatable matrices.
 *
 * Note that the abs in the name refers to the "not relative" sense of absolute,
 * but the value computed is also the maximum absolute value.
 *
 * Returns:
 *    The distance between the two matrices, or a negative number if there is an
 *    error (not very likely--currently impossible). Note that sending in
 *    matrices with incompatable dimensions is regarded as a bug. (See
 *    set_bug(3)).
 *
 * Index: matrices, matrix measures
 *
 * -----------------------------------------------------------------------------
*/

int max_abs_int_matrix_difference
(
    const Int_matrix* first_mp,
    const Int_matrix* second_mp
)
{
    int   i;
    int   j;
    int   num_rows;
    int   num_cols;
    int* first_pos;
    int* second_pos;
    int  max_diff   = 0;


    /*
     * Return is a bit awkward due to function semantics, so we opt for calling
     * dimension problems a bug. The ERROR return gets cast into a double which
     * is fine as this routine should return a non-negative value.
    */
    ESBRE(check_same_int_matrix_dimensions(first_mp, second_mp,
                                           "max_abs_int_matrix_difference"));

    num_rows = first_mp->num_rows;
    num_cols = first_mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        first_pos = first_mp->elements[ i ];
        second_pos = second_mp->elements[ i ];

        for (j=0; j<num_cols; j++)
        {
            int diff;


            diff = ABS_OF(*first_pos - *second_pos);

            if (diff > max_diff) max_diff = diff;

            first_pos++;
            second_pos++;
        }
    }

    return max_diff;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       check_same_int_matrix_dimensions
 *
 * Checks that two matrices have the same dimensions
 *
 * This routine checks that two matrices have the same dimensions. It the
 * matrices do not have the same dimension, then the an error message is set and
 * ERROR is returned. The argument context_str can be used to add more
 * information; typically, it is the name of the calling routine, but it could
 * be some other string which works well as the sequal to the message "failed
 * in".  If context_str is NULL, then it is not used.
 *
 * Macros:
 *     If different matrix dimensions are likely due to a bug, then you may want
 *     to wrap this routine in the macro ESBRE(), which prints the error, calls
 *     set_bug(), (the "SB"), and then returns ERROR. If different matrix
 *     dimensions are simply an error that the calling routine should deal with,
 *     then wrapping the call to this routine in ERE() saves typing.
 *
 * Returns:
 *    NO_ERROR if the matrices have the same dimensions, and ERROR otherwise.
 *    Depending on the bug handler in place, this routine may not return at all.
 *
 * Index: error handling, debugging
 *
 * -----------------------------------------------------------------------------
*/

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
)
{
    int first_matrix_num_rows, second_matrix_num_rows;
    int first_matrix_num_cols, second_matrix_num_cols;
    int result = NO_ERROR;


    if ((first_mp == NULL) && (second_mp == NULL))
    {
        return NO_ERROR;
    }

    if (first_mp == NULL)
    {
        set_error("Null first matrix but not second in dimension equality check.");
        return ERROR;
    }

    if ((result != ERROR) && (second_mp == NULL))
    {
        set_error("Null second matrix but not first in dimension equality check.");
        return ERROR;
    }

    if (result != ERROR)
    {
        first_matrix_num_rows = first_mp->num_rows;
        second_matrix_num_rows = second_mp->num_rows;

        if (first_matrix_num_rows != second_matrix_num_rows)
        {
            set_error("Mismatch in matrix row counts (%d versus %d).",
                      first_matrix_num_rows, second_matrix_num_rows);
            result = ERROR;
        }
    }

    if (result != ERROR)
    {
        first_matrix_num_cols = first_mp->num_cols;
        second_matrix_num_cols = second_mp->num_cols;

        if (first_matrix_num_cols != second_matrix_num_cols)
        {
            set_error("Mismatch in matrix columm counts (%d versus %d).",
                     first_matrix_num_cols, second_matrix_num_cols);
            result = ERROR;
        }
    }

    if (result == ERROR)
    {
#ifdef TEST
        add_error("Failed dimension check called from line %d of file %s.",
                  line_number, file_name);
#endif

        if (context_str != NULL)
        {
            add_error("Matrix dimension equality check failed in %s.", context_str);
        }
        else
        {
            add_error("Matrix dimension equality check failed.");
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              sum_int_matrix_elements
 *
 * Adds the elements of a integer matrix
 *
 * This routine returns the sum of the elements of the matrix pointed to by mp
 *
 * Returns:
 *    The sum of the elements of the matrix pointed to by mp.
 *
 * Related:
 *     average_int_matrix_elements(3)
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int sum_int_matrix_elements(const Int_matrix* mp)
{
    int    i;
    int    j;
    int   sum;
    int** row_ptr;
    int*  element_ptr;


    if (mp->num_rows == 0) return 0;
    if (mp->num_cols == 0) return 0;

    sum = 0;

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
 *                               sum_int_matrix_rows
 *
 * Sums integer matrix rows
 *
 * This routine sums the rows of a matrix putting the result into the vector
 * pointed to by *output_vpp which becomes a vector of size num_cols.
 *
 * If we are respecting missing values, then they are excluded from the average.
 * If all values for a colum are missing then the average is set to DBL_MISSING.
 *
 * If *output_vpp is NULL, then a vector of the appropriate size is created. If
 * it is the wrong size, it is resized. Finally, if it is the right size, then
 * the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int sum_int_matrix_rows
(
    Int_vector**      output_vpp,
    const Int_matrix* input_mp 
)
{

    /*if (respect_missing_values())
      {
      return sum_int_matrix_rows_without_missing(output_vpp, input_mp);
      }*/

    ERE(get_target_int_vector(output_vpp, input_mp->num_cols));

    return ow_sum_int_matrix_rows(*output_vpp, input_mp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * An ow_ version of sum_int_matrix_rows() does not really make sense, but it is the
 * easiest solution to forcing the answer to go into a specified matrix which is
 * done in annotate.c to put the results into a subset of a vector.
*/

int ow_sum_int_matrix_rows
(
    Int_vector*       output_vp,
    const Int_matrix* input_mp 
)
{
    int     i;
    int     j;
    int*   row_pos;
    int*   out_pos;


    ERE(ow_zero_int_vector(output_vp));

    for (i=0; i<input_mp->num_rows; i++)
    {
        row_pos = input_mp->elements[ i ];
        out_pos = output_vp->elements;

        for (j=0; j<input_mp->num_cols; j++)
        {
            *out_pos += *row_pos;
            out_pos++;
            row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             ow_add_col_int_vector_to_int_matrix
 *
 * Adds an integer vector to each column of an integer matrix.
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
 *     add_col_int_vector_to_int_matrix(3)
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_col_int_vector_to_int_matrix
(
    Int_matrix*       source_mp,
    const Int_vector* vp 
)
{
    int     i;
    int     j;
    int     num_rows;
    int     num_cols;
    int**  source_row_ptr;
    int*   source_row_pos_ptr;
    int*   vp_pos;


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
            *source_row_pos_ptr += (*vp_pos);
            source_row_pos_ptr++;
        }

        vp_pos++;
        source_row_ptr++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                          ow_add_row_vector_to_matrix
 *
 * Adds an integer vector to each row of an integer matrix.
 *
 * This routine adds a vector to each row of the input matrix. The length of the
 * vector must match the number of columns in the matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. Currenly this routine can't fail gracefully.  The routine will fail
 *     due to dimension mismatch, but this is currently treated as a bug (see
 *     set_bug()).
 *
 * Related:
 *     add_row_int_vector_to_int_matrix(3)
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_row_int_vector_to_int_matrix
(
    Int_matrix*       source_mp,
    const Int_vector* vp 
)
{
    int    i;
    int    j;
    int    num_rows;
    int    num_cols;
    int** source_row_ptr;
    int*  source_row_pos_ptr;
    int*  vp_pos;


    if (source_mp->num_cols != vp->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

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

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                           ow_add_int_scalar_to_int_matrix_row
 *
 * Adds an integer scalar to a specified row of an integer matrix
 *
 * This routine adds a scalar to a specified row of the input matrix.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_int_scalar_to_int_matrix(Int_matrix* source_mp, int scalar)
{
    int    i, j, num_rows, num_cols;
    int** source_row_ptr;
    int*  source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

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

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*
 * =============================================================================
 *                              subtract_int_matrices
 *
 * Performs element-wise subtraction of two int matrices.
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

int subtract_int_matrices(Int_matrix** target_mpp, const Int_matrix* first_mp,
                      const Int_matrix* second_mp)
{
    ERE(copy_int_matrix(target_mpp, first_mp));

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

    return ow_subtract_int_matrices(*target_mpp, second_mp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_subtract_int_matrices
 *
 * Performs element-wise subtraction of two int matrices.
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

int ow_subtract_int_matrices(Int_matrix* first_mp, const Int_matrix* second_mp)
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
 *                          multiply_int_matrices
 *
 * Multiplies two int matrices.
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

int multiply_int_matrices(Int_matrix** target_mpp, const Int_matrix* first_mp,
                      const Int_matrix* second_mp)
{
    Int_matrix* target_mp;
    int     i;
    int     j;
    int     k;
    int     num_rows;
    int     num_cols;
    int     length;
    int*   first_pos;
    int**  second_elements;
    int result = NO_ERROR;


    if ((first_mp == NULL) || (second_mp == NULL))
    {
        UNTESTED_CODE();
        free_int_matrix(*target_mpp);
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
        Int_matrix* temp_mp = NULL;

        result = multiply_int_matrices(&temp_mp, first_mp, second_mp);

        if (result != ERROR)
        {
            result = copy_int_matrix(target_mpp, temp_mp);
        }

        free_int_matrix(temp_mp);
    }
    else
    {
        ERE(get_target_int_matrix(target_mpp, first_mp->num_rows,
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
                long int sum = 0; /* long_double only has extra precision
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
 *                              ow_get_abs_of_int_matrix
 *
 * Calculates the element-wise absolute value of an int matrix.
 *
 * This routine calculates the element-wise absolute value of a matrix,
 * overwiting the input matrix with the result.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set. THis routine will fail if any of the element of the input vector is
 *     too close to zero.
 *
 * Index: integer matrices
 *
 * Author: Ernesto Brau, Kobus Barnard
 *
 * Documentor: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
 */

int ow_get_abs_of_int_matrix(Int_matrix* mp)
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
 *                              get_abs_of_int_matrix
 *
 * Calculates the absolute values of an int matrix components.
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

int get_abs_of_int_matrix(Int_matrix** target_mpp, const Int_matrix* source_mp)
{
    Int_matrix* target_mp;
    int i, j;

    ERE(get_target_int_matrix(target_mpp, source_mp->num_rows, source_mp->num_cols));
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

/*
 * =============================================================================
 *                         multiply_int_matrix_by_int_scalar
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
 *     ow_multiply_int_matrix_by_int_scalar(3)
 *
 * Index: matrices, matrix arithmetic
 *
 * -----------------------------------------------------------------------------
 */

int multiply_int_matrix_by_int_scalar(Int_matrix** target_mpp, const Int_matrix* source_mp,
                              int scalar)
{
    Int_matrix* target_mp;
    int    i, j, num_rows, num_cols;
    int** source_row_ptr;
    int** target_row_ptr;
    int*  source_row_pos_ptr;
    int*  target_row_pos_ptr;


    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

    ERE(get_target_int_matrix(target_mpp, num_rows, num_cols));
    target_mp = *target_mpp;

    source_row_ptr = source_mp->elements;
    target_row_ptr = target_mp->elements;

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

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        ow_multiply_int_matrix_by_int_scalar
 *
 * Multiplies each element of an integer matrix by an integer scalar
 *
 * This routine multiplies each element of a matrix by a scalar.
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
 *     multiply_int_matrix_by_int_scalar(3)
 *
 * Author:
 *     Ernesto Brau
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_multiply_int_matrix_by_int_scalar
(
    Int_matrix* source_mp,
    int         scalar 
)
{
    int    i, j, num_rows, num_cols;
    int** source_row_ptr;
    int*  source_row_pos_ptr;


    source_row_ptr = source_mp->elements;

    num_rows = source_mp->num_rows;
    num_cols = source_mp->num_cols;

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

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        multiply_int_matrix_and_int_vector
 *
 * Multiplies an int matrix by an int vector
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

int multiply_int_matrix_and_int_vector(Int_vector** output_vpp, const Int_matrix* input_mp,
                               const Int_vector* input_vp)
{
    int     i, j;
    int  sum;
    int* vec_pos;
    int* mat_row_pos;
    int     result      = NO_ERROR;


    if ((input_vp == NULL) || (input_mp == NULL))
    {
        UNTESTED_CODE();
        free_int_vector(*output_vpp);
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
        Int_vector* temp_vp = NULL;

        result = multiply_int_matrix_and_int_vector(&temp_vp, input_mp, input_vp);

        if (result != ERROR)
        {
            result = copy_int_vector(output_vpp, temp_vp);
        }

        free_int_vector(temp_vp);
    }
    else
    {
        const Int_vector* output_vp;

        ERE(get_target_int_vector(output_vpp, input_mp->num_rows));
        output_vp = *output_vpp;

        for (i = output_vp->length; --i>=0; )
        {
            sum = 0;
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
 *                      multiply_int_vector_and_int_matrix
 *
 * Multiplies an int vector by an int matrix
 *
 * This routine multiplies an int vector by an int matrix. The vector is used as a row
 * vector. The dimensions must be appropriate for muliplication, or ERROR is
 * returned.
 *
 * The first argument is a pointer to the target int vector. If the target int vector
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

int multiply_int_vector_and_int_matrix
(
    Int_vector**      output_vpp,
    const Int_vector* input_vp,
    const Int_matrix* input_mp
)
{
    int    i, j;
    int    sum;
    int    result = NO_ERROR;


    if ((input_vp == NULL) || (input_mp == NULL))
    {
        UNTESTED_CODE(); 
        free_int_vector(*output_vpp);
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
        Int_vector* temp_vp = NULL;

        result = multiply_int_vector_and_int_matrix(&temp_vp, input_vp, input_mp);

        if (result != ERROR)
        {
            result = copy_int_vector(output_vpp, temp_vp); 
        }

        free_int_vector(temp_vp); 
    }
    else 
    {
        const Int_vector* output_vp;

        ERE(get_target_int_vector(output_vpp, input_mp->num_cols));
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
 *                              add_int_matrices
 *
 * Performs element-wise addition of two integer matrices.
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
 * Author:
 *     Ernesto Brau
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int add_int_matrices
(
    Int_matrix**      target_mpp,
    const Int_matrix* first_mp,
    const Int_matrix* second_mp 
)
{
    ERE(copy_int_matrix(target_mpp, first_mp));

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


    return ow_add_int_matrices(*target_mpp, second_mp); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              ow_add_int_matrices
 *
 * Performs element-wise addition of two integer matrices.
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
 * Author:
 *     Ernesto Brau
 *
 * Index: integer matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_add_int_matrices
(
    Int_matrix*        first_mp,
    const Int_matrix*  second_mp
)
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


/* =============================================================================
 *                       get_target_int_matrix_vector_vector
 *
 * Gets a target integer matrix vector vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of int matrix vector vectors. If *target_mvvpp is
 * NULL, then this routine creates the object. If it is not null, and it
 * is the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_int_matrix_vector_vector should be used to dispose of the
 * storage once it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Author: Kobus Barnard, Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * Index: memory allocation, arrays, integer matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_target_int_matrix_vector_vector(Int_matrix_vector_vector** mvvpp, int length)
{

    if (    (*mvvpp != NULL)
         && ((*mvvpp)->length == length)
       )
    {
        return NO_ERROR;
    }

    free_int_matrix_vector_vector(*mvvpp);
    NRE(*mvvpp = create_int_matrix_vector_vector(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Int_matrix_vector_vector* create_int_matrix_vector_vector(int length)
{
    Int_matrix_vector_vector* mvvp;
    int           i;


    NRN(mvvp = TYPE_MALLOC(Int_matrix_vector_vector));
    mvvp->length = length;
    NRN(mvvp->elements = N_TYPE_MALLOC(Int_matrix_vector*, length));

    for (i=0; i<length; i++)
    {
        mvvp->elements[ i ] = NULL;
    }

    return mvvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_int_matrix_vector_vector
 *
 * Frees the storage in a integer matrix vector vector
 *
 * This routine frees the storage in a integer matrix vector vector object.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Author: Kobus Barnard, Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * Index: memory allocation, arrays, integer matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_int_matrix_vector_vector(Int_matrix_vector_vector* mvvp)
{
    int count, i;
    Int_matrix_vector** mvpp;


    if (mvvp == NULL) return;

    mvpp = mvvp->elements;
    count = mvvp->length;

    for (i=0; i<count; i++)
    {
        free_int_matrix_vector(*mvpp);
        mvpp++;
    }

    kjb_free(mvvp->elements);
    kjb_free(mvvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

