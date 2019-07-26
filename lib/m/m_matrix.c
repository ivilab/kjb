
/* $Id: m_matrix.c 20654 2016-05-05 23:13:43Z kobus $ */

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
#include "m/m_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TEST
double debug_mat_val
(
 const Matrix* mp,
 int           i,
 int           j,
 const char*   file_name,
 int           line_number
)
{
    if ((i < 0) || (i >= mp->num_rows) || (j < 0) || (j >= mp->num_cols))
    {

        set_bug("Bounds check error on line %d of file %s.",
                line_number, file_name);
        return 0.0;
    }
    else
    {
        return mp->elements[i][j];
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_zero_matrix
 *
 * Gets a zero target matrix
 *
 * This routine is identical to get_target_matrix(), except that the matrix is
 * set to zero (regardless of whether it is created or reused).
 *
 * Index : matrices
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_get_zero_matrix
(
    Matrix**    target_mpp,
    int         num_rows,
    int         num_cols,
    const char* file_name,
    int         line_number
)
{


    ERE(debug_get_target_matrix(target_mpp, num_rows, num_cols,
                                file_name, line_number));

    return ow_zero_matrix(*target_mpp);
}

#else

int get_zero_matrix(Matrix** target_mpp, int num_rows, int num_cols)
{


    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    return ow_zero_matrix(*target_mpp);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     get_unity_matrix
 *
 * Gets a target matrix initialized to all ones.
 *
 * This routine is identical to get_target_matrix(), except that the matrix is
 * set to ones (regardless of whether it is created or reused).
 *
 * Index : matrices
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_get_unity_matrix
(
    Matrix**    target_mpp,
    int         num_rows,
    int         num_cols,
    const char* file_name,
    int         line_number
)
{


    ERE(debug_get_target_matrix(target_mpp, num_rows, num_cols,
                                file_name, line_number));

    return ow_set_matrix(*target_mpp, 1.0);
}

#else

int get_unity_matrix(Matrix** target_mpp, int num_rows, int num_cols)
{


    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    return ow_set_matrix(*target_mpp, 1.0);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         get_initialized_matrix
 *
 * Gets an initialized target matrix
 *
 * This routine is identical to get_target_matrix(), except that the matrix
 * elements are set to a specified initial value (regardless of whether it is
 * created or reused).
 *
 * Index : matrices
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_get_initialized_matrix
(
    Matrix**    target_mpp,
    int         num_rows,
    int         num_cols,
    double      initial_value,
    const char* file_name,
    int         line_number
)
{


    ERE(debug_get_target_matrix(target_mpp, num_rows, num_cols,
                                file_name, line_number));

    return ow_set_matrix(*target_mpp, initial_value);
}

#else

int get_initialized_matrix
(
    Matrix** target_mpp,
    int      num_rows,
    int      num_cols,
    double   initial_value
)
{


    ERE(get_target_matrix(target_mpp, num_rows, num_cols));

    return ow_set_matrix(*target_mpp, initial_value);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_target_matrix
 *
 * Gets target matrix
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of matrices. If *target_mpp is NULL, then this routine
 * creates the matrix. If it is not null, and it is the right size, then this
 * routine does nothing. If it is the wrong size, then it is resized.
 *
 * In order to be memory efficient, we free before resizing. If the resizing
 * fails, then the original contents of the *target_mpp will be lost.  However,
 * target_mpp->elements will be set to NULL, so *target_mpp can be safely sent
 * to free_matrix().  Note that this is in fact the convention throughout the
 * KJB library--if destruction on failure is a problem (usually when *target_mpp
 * is global)--then work on a copy!
 *
 * The sizes (num_rows and num_cols) must both be nonnegative.
 * Conventionally, num_rows and num_cols are both positive;
 * however, it is also acceptable to have num_rows and num_cols both zero.
 * Anything else (such as a mix of positive and zero sizes) is regarded as a
 * bug in the calling program -- set_bug(3) is called and (if it returns) 
 * ERROR is returned.
 *
 * Related:
 *    Matrix
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

/*
// FIX
//
// We need to upgrade this routine so that the storage can be recycled, but this
// is tricky because we have to fix both the array of row pointers.
//
// Actually, this only makes sense if we store the amount of storage as an extra
// variable. Then the storage gets bumped up to a max and stays there. If we
// simply downsize, then on a upsize we have to free anyway. Hence this is not
// as useful.
*/

#ifdef TRACK_MEMORY_ALLOCATION

int debug_get_target_matrix
(
    Matrix**    target_mpp,
    int         num_rows,
    int         num_cols,
    const char* file_name,
    int         line_number
)
{
    Matrix* out_mp = *target_mpp;


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
        NRE(out_mp = DEBUG_create_matrix(num_rows, num_cols, file_name,
                                         line_number));
        *target_mpp = out_mp;
    }
    else if ((num_cols == out_mp->num_cols) && (num_rows <= out_mp->num_rows))
    {
        out_mp->num_rows = num_rows;
    }
    else if (num_rows * num_cols <= out_mp->max_num_elements)
    {
        double** row_ptr = out_mp->elements;
        double* elem_ptr = *row_ptr;
        int     i;
        int max_num_elements = out_mp->max_num_elements;

        /*
         * Either the number of rows is to be increased, or the column size is
         * to change. In both cases the pointers need to be fixed. 
        */

        if (num_rows > out_mp->max_num_rows)
        {
            kjb_free(out_mp->elements);
            NRE(out_mp->elements = (double **)debug_kjb_malloc(num_rows * sizeof(double*),
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
        free_2D_double_array(out_mp->elements);

        out_mp->elements = debug_allocate_2D_double_array(num_rows,
                                                          num_cols,
                                                          file_name,
                                                          line_number);

        if ((num_rows != 0) && (out_mp->elements == NULL))
        {
            return ERROR;
        }

        out_mp->max_num_elements = num_rows * num_cols;
        out_mp->max_num_rows = num_rows;
        out_mp->max_num_cols = num_cols;
        out_mp->num_rows = num_rows;
        out_mp->num_cols = num_cols;
    }

    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_matrix(Matrix** target_mpp, int num_rows, int num_cols)
{
    Matrix* out_mp = *target_mpp;


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
        NRE(out_mp = create_matrix(num_rows, num_cols));
        *target_mpp = out_mp;
    }
    else if ((num_cols == out_mp->num_cols) && (num_rows <= out_mp->num_rows))
    {
        out_mp->num_rows = num_rows;
    }
    else if (num_rows * num_cols <= out_mp->max_num_elements)
    {
        double** row_ptr = out_mp->elements;
        double* elem_ptr = *row_ptr;
        int     i;

        /*
         * Either the number of rows is to be increased, or the column size is
         * to change. In both cases the pointers need to be fixed. 
        */

        if (num_rows > out_mp->max_num_rows)
        {
            kjb_free(out_mp->elements);
            NRE(out_mp->elements = (double **)kjb_malloc(num_rows * sizeof(double*)));
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
        free_2D_double_array(out_mp->elements);

        out_mp->elements = allocate_2D_double_array(num_rows,
                                                    num_cols);

        if ((num_rows != 0) && (out_mp->elements == NULL))
        {
            return ERROR;
        }

        out_mp->max_num_elements = num_rows * num_cols;
        out_mp->max_num_rows = num_rows;
        out_mp->max_num_cols = num_cols;
        out_mp->num_rows = num_rows;
        out_mp->num_cols = num_cols;
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/* =============================================================================
 *                       create_matrix
 *
 * Creates a matrix
 *
 * This routine creates a matrix and returns a pointer P to it. The number
 * of rows is P->num_rows, the number of columns is P->num_cols, and the
 * elements (of type "double") may be referenced by (P->elements)[ row ][ col ].
 * Type "double" is either float or double, depending on whether the single or
 * double precision library is being used. (Usually double precision).
 *
 * The documenation for the Matrix type (see "Matrix") should be consulted for
 * CRITICAL information on the internal structure of the returned matrix.
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * DEBUG_create_matrix, which is the version available in the developement
 * library. In developement code memory is tracked so that memory leaks can be
 * found more easily. Furthermore, all memory freed is checked that it was
 * allocated by an L library routine.
 *
 * The routine free_matrix should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Warning:
 *    The user of this library must take care with assumptions about the
 *    structure of the array embedded in the matrix type. See the documentation
 *    for "Matrix" for more details.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    One success it returns a pointer to the matrix.
 *
 * Related:
 *    Matrix
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

#endif

#ifdef TRACK_MEMORY_ALLOCATION

Matrix* DEBUG_create_matrix
(
    int         num_rows,
    int         num_cols,
    const char* file_name,
    int         line_number
)
{
    Matrix* mp;


    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */

    NRN(mp = DEBUG_TYPE_MALLOC(Matrix, file_name, line_number));

    mp->elements = debug_allocate_2D_double_array(num_rows, num_cols, file_name,
                                                line_number);

    if ((num_rows != 0) && (num_cols >= 0) && (mp->elements == NULL))
    {
        free_matrix(mp);
        return NULL;
    }

    mp->num_rows = num_rows;
    mp->num_cols = num_cols;
    mp->max_num_rows = num_rows;
    mp->max_num_cols = num_cols;
    mp->max_num_elements = num_rows * num_cols;

    return mp;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Matrix* create_matrix(int num_rows, int num_cols)
{
    Matrix* mp;


    NRN(mp = TYPE_MALLOC(Matrix));

    mp->elements = allocate_2D_double_array(num_rows, num_cols);

    if ((num_rows != 0) && (num_cols >= 0) && (mp->elements == NULL))
    {
        free_matrix(mp);
        return NULL;
    }

    mp->num_rows = num_rows;
    mp->num_cols = num_cols;
    mp->max_num_rows = num_rows;
    mp->max_num_cols = num_cols;
    mp->max_num_elements = num_rows * num_cols;

    return mp;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_matrix
 *
 * Frees a the space associated with a matrix.
 *
 * This routine frees the storage associated with a matrix obtained from
 * create_vector (perhaps indirectrly). If the argument is NULL, then this
 * routine returns safely without doing anything.
 *
 * Related:
 *    Matrix
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_matrix(Matrix* mp)
{

    if (mp != NULL)
    {
        if (mp->elements != NULL)
        {
#ifdef CHECK_MATRIX_INITIALZATION_ON_FREE
            /*
             * We must have a valid pointer to check the initialization,
             * otherwise problems such as double free can look like unitialized
             * memory.
            */
            kjb_check_free(mp);

            if (kjb_debug_level >= 10)
            {
                check_initialization(mp->elements[ 0 ],
                                     mp->num_rows * mp->num_cols,
                                     sizeof(double));
            }
#endif

            free_2D_double_array(mp->elements);
        }

        kjb_free(mp);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/*
 * =============================================================================
 *                        create_zero_matrix
 *
 * Creates a matrix with all elements zero
 *
 * This routine returns a pointer to a zero matrix of the requested size.
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

Matrix* create_zero_matrix(int num_rows, int num_cols)
{
    Matrix* output_mp;


    NRN(output_mp = create_matrix(num_rows, num_cols));

    if (ow_zero_matrix(output_mp) == ERROR)
    {
        free_matrix(output_mp);
        return NULL;
    }

    return output_mp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/*
 * =============================================================================
 *                            create_identity_matrix
 *
 * Creates an identity matrix of the specified size
 *
 * This routine returns a pointer to an identity matrix of the specified size.
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

Matrix* create_identity_matrix(int size)
{
    Matrix* output_mp;
    int i;


    NRN(output_mp = create_zero_matrix(size, size));

    for (i=0; i<size; i++)
    {
        (output_mp->elements)[i][i] = 1.0;
    }

    return output_mp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            get_identity_matrix
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

int get_identity_matrix(Matrix** output_mpp, int size)
{
    Matrix* output_mp;
    int i;


    ERE(get_zero_matrix(output_mpp, size, size));
    output_mp = *output_mpp;

    for (i=0; i<size; i++)
    {
        (output_mp->elements)[i][i] = 1.0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            ow_zero_matrix
 *
 * Sets all elements of a matrix to zero
 *
 * This routine sets all elements of a matrix to zero. The matrix must have
 * already been created.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure. Failure is only possible if the
 *     matrix is NULL or malformed, and this is treated as a bug.
 *
 * Index : matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_zero_matrix(Matrix* mp)
{

    /*
    // Previously I used memset on the pointer to the first element, but this
    // is a bug because we can temporarily reset the rows and cols of the
    // matrix to work on a subset. Until we implement the concept of a max num
    // rows and cols, we cannot use this kind of trick to speed things up!
    //
    // If we explictly implement subsetting, then we would do a memset on the
    // num_rows * max_num_cols.
    */

    return ow_set_matrix(mp, 0.0);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            ow_set_matrix
 *
 * Sets all elements of a matrix to the specified value
 *
 * This routine sets all elements of a matrix to the specified value. The matrix
 * must have already been created.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure. Failure is only possible if the
 *     matrix is NULL or malformed, and this is treated as a bug.
 *
 * Index : matrices
 *
 * -----------------------------------------------------------------------------
 */

int ow_set_matrix(Matrix* mp, double set_val)
{
    int i, j;
    double* row_pos;
    double  real_set_val = set_val;

    if (mp != NULL)
    {
        for (i=0; i<mp->num_rows; i++)
        {
            row_pos = (mp->elements)[ i ];

            for (j=0; j<mp->num_cols; j++)
            {
                *row_pos = real_set_val;
                row_pos++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                        get_diagonal_matrix
 *
 * Gets a diagonal matrix from a vector
 *
 * This routine creates a diagonal matrix from a vector provided as the
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

int get_diagonal_matrix(Matrix** mpp, const Vector* vp)
{
    Matrix* d_mp;
    int     i;


    ERE(get_zero_matrix(mpp, vp->length, vp->length));
    d_mp = *mpp;

    for (i = 0; i < vp->length; i++)
    {
        (d_mp->elements)[i][i] = (vp->elements)[i];
     }

     return NO_ERROR;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */


/*
 * =============================================================================
 *                        create_diagonal_matrix
 *
 * Creates a diagonal matrix from a vector
 *
 * This routine creates a diagonal matrix from a vector provided as the
 * argument.
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

Matrix* create_diagonal_matrix(const Vector* vp)
{
    Matrix* d_mp;
    int i;


    NRN(d_mp = create_zero_matrix(vp->length, vp->length));

    for (i = 0; i < vp->length; i++)
    {
        (d_mp->elements)[i][i] = (vp->elements)[i];
     }

     return d_mp;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/* =============================================================================
 *                          create_random_matrix
 *
 * Creates a random matrix
 *
 * This routine creates a matrix of the specified dimensions, and fills it with
 * random values between 0.0 and 1.0. The routine kjb_rand is used for the
 * random numbers.
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

Matrix* create_random_matrix(int num_rows, int num_cols)
{
    Matrix* mp;
    double*   elem_ptr;
    int    i, j;


    NRN(mp = create_matrix(num_rows, num_cols));

    for (i=0; i<num_rows; i++)
    {
        elem_ptr = (mp->elements)[ i ];

        for (j=0; j<num_cols; j++)
        {
            *elem_ptr++ = kjb_rand();
        }
    }

    return mp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should likely be static. */

/* =============================================================================
 *                          create_random_matrix_2
 *
 * Creates a random matrix
 *
 * This routine creates a matrix of the specified dimensions, and fills it with
 * random values between 0.0 and 1.0. The routine kjb_rand_2() is used for the
 * random numbers.
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

Matrix* create_random_matrix_2(int num_rows, int num_cols)
{
    Matrix* mp;
    double*   elem_ptr;
    int    i, j;


    NRN(mp = create_matrix(num_rows, num_cols));

    for (i=0; i<num_rows; i++)
    {
        elem_ptr = (mp->elements)[ i ];

        for (j=0; j<num_cols; j++)
        {
            *elem_ptr++ = kjb_rand_2();
        }
    }

    return mp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_random_matrix
 *
 * Gets a random matrix
 *
 * This routine gets a matrix of the specified dimensions, and fills it with
 * random values between 0.0 and 1.0. The routine kjb_rand() is used.
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

int get_random_matrix(Matrix** target_mpp, int num_rows, int num_cols)
{
    Matrix* mp;
    double*   elem_ptr;
    int    i, j;


    ERE(get_target_matrix(target_mpp, num_rows, num_cols));
    mp = *target_mpp;

    for (i=0; i<num_rows; i++)
    {
        elem_ptr = (mp->elements)[ i ];

        for (j=0; j<num_cols; j++)
        {
            *elem_ptr++ = kjb_rand();
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_random_matrix_2
 *
 * Gets a random matrix
 *
 * This routine gets a matrix of the specified dimensions, and fills it with
 * random values between 0.0 and 1.0. The routine kjb_rand_2 is used.
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Note:
 *     This routine is exactly the same as get_random_matrix(), except that the
 *     alternative random stream kjb_rand_2() is used.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_random_matrix_2(Matrix** target_mpp, int num_rows, int num_cols)
{
    Matrix* mp;
    double*   elem_ptr;
    int    i, j;


    ERE(get_target_matrix(target_mpp,num_rows, num_cols));
    mp = *target_mpp;

    for (i=0; i<num_rows; i++)
    {
        elem_ptr = (mp->elements)[ i ];

        for (j=0; j<num_cols; j++)
        {
            *elem_ptr++ = kjb_rand_2();
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_perturb_matrix(Matrix* mp, double fraction)
{
    int i, j, num_rows, num_cols;


    num_rows = mp->num_rows;
    num_cols = mp->num_cols;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            mp->elements[ i ][ j ] *= (1.0 + fraction * (kjb_rand_2() - 0.5));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_symmetric_matrix(const Matrix* mp)
{

    if (mp == NULL)
    {
        /* Somewhat arbitrary distinction that a NULL matrix is symmetric. */
        return TRUE;
    }
    else
    {
        int num_rows = mp->num_rows;
        int num_cols = mp->num_cols;

        if (num_rows != num_cols)
        {
            return FALSE;
        }
        else
        {
            int i,j;
            double abs_val, abs_diff;
            double max_abs = 0.0;
            double max_abs_diff = 0.0;

            for (i = 0; i < num_rows; i++)
            {
                for (j = 0; j < num_cols; j++)
                {
                    abs_val = ABS_OF(mp->elements[ i ][ j ]);
                    max_abs = MAX_OF(max_abs, abs_val);
                    abs_diff = ABS_OF(mp->elements[ i ][ j ] - mp->elements[ j ][ i ]);
                    max_abs_diff = MAX_OF(max_abs_diff, abs_diff);
                }
            }

            if (max_abs_diff > 100.0 * DBL_EPSILON * max_abs)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

