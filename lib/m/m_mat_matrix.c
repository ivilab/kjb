
/* $Id: m_mat_matrix.c 22184 2018-07-16 00:08:22Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2012 by Kobus Barnard (author).
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
#include "m/m_mat_matrix.h"
#include "m/m_mat_basic.h"

#ifdef __cplusplus
extern "C" {
#endif
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should be either static or deleted. */
/* =============================================================================
 *                          create_matrix_matrix
 *
 * Creates a Matrix of matrices
 *
 * This routine creates a matrix of the specified dimensions and returns a
 * pointer
 * P to it. The number of rows is P->num_rows, the number of columns is
 * P->num_cols, 
 * and the elements (of type "Matrix*") may be referenced by
 * (P->elements)[ row ][ col ].
 *
 * The routine free_matrix_matrix should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Warning:
 *    The user of this library must take care with assumptions about the
 *    structure of the array embedded in the matrix type. See the documentation
 *    for "Matrix" for more details and CRITICAL information on the internal
 *    structure of the returned matrix.
 *
 *
 * Returns:
 *     A  pointer to the Matrix_matrix on success and NULL on failure
 *     (with an error message being set.) This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices, matrix matrix
 *
 * -----------------------------------------------------------------------------
*/
#endif 

/* OBSOLETE, delete soon (or change to "target" and move to l lib). FIXME */ 

Matrix_matrix* create_matrix_matrix
(
    int num_rows, 
    int num_cols, 
    int elt_rows,
    int elt_cols
)
{
    int             i;
    int             j;
    Matrix_matrix*  mmp;

    NRN(mmp = TYPE_MALLOC(Matrix_matrix));

    NRN(mmp->elements = allocate_2D_mp_array(num_rows, num_cols));

    if (   (num_rows != 0) && (num_cols >= 0) 
        && (elt_rows != 0) && (elt_cols >= 0) 
        && (mmp->elements == NULL)
       )
    {
        free_matrix_matrix(mmp);
        return NULL;
    }

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            get_target_matrix(&(mmp->elements[ i ][ j ]), elt_rows, elt_cols);
        }
    }

    mmp->num_rows = num_rows;
    mmp->num_cols = num_cols;
    mmp->max_num_rows = num_rows;
    mmp->max_num_cols = num_cols;
    mmp->max_num_elements = num_rows * num_cols;

    return mmp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DOCUMENT_CREATE_VERSIONS  /* Create confuses. Should be either static or deleted`. */

/*
 * =============================================================================
 *                        create_homography_matrix_matrix
 *
 * Creates a matrix with all elements set to be a 3x3 matrix.
 *
 * This routine returns a pointer to a Matrix_matrix where each element is 
 * a 3x3 matrix.
 *
 * The routine free_matrix_matrix should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *     A  pointer to the Matrix_matrix on success and NULL on failure (with an 
       error message being set.) This routine will only fail if storage allocation
 *     fails.
 *
 * Index: matrices, matrix matrix
 *
 * -----------------------------------------------------------------------------
 */
#endif 

/* OBSOLETE, delete soon. (There are no homography semantics here!) FIXME */
/* Note that this routine might be used by SLIC. */

Matrix_matrix* create_homography_matrix_matrix(int num_rows, int num_cols)
{
    Matrix_matrix*  mmp;
    int             i;
    int             j;

    NRN(mmp = TYPE_MALLOC(Matrix_matrix));
    NRN(mmp->elements = allocate_2D_mp_array(num_rows, num_cols));

    if ((num_rows != 0) && (num_cols >= 0) && (mmp->elements == NULL))
    {
        free_matrix_matrix(mmp);
        return NULL;
    }


    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            get_target_matrix(&(mmp->elements[ i ][ j ]), 3, 3);
        }
    }

    mmp->num_rows = num_rows;
    mmp->num_cols = num_cols;
    mmp->max_num_rows = num_rows;
    mmp->max_num_cols = num_cols;
    mmp->max_num_elements = num_rows * num_cols;

    return mmp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_matrix_matrix
 *
 * Frees the storage in a Matrix_matrix
 *
 * This routine frees the storage in a matrix of matrices. If the argument is
 * NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, matrix matrix, matrices
 *
 * -----------------------------------------------------------------------------
*/  
void free_matrix_matrix (Matrix_matrix* mmp)
{

    if (mmp != NULL) 
    {
        if(mmp->elements != NULL)
        {
            int       rows;
            int       cols;

            rows = mmp->num_rows;
            cols = mmp->num_cols;

        free_2D_mp_array_and_matrices(mmp->elements, rows, cols);
        }

        kjb_free(mmp);
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* OBSOLETE, delete soon. FIXME */
/* Note that this routine is used by SLIC. */

Int_matrix_matrix* create_int_matrix_matrix
(
    int num_rows, 
    int num_cols 
)
{
    int i , j;
    Int_matrix_matrix* immp = NULL;

    NRN(immp = TYPE_MALLOC( Int_matrix_matrix ));

    NRN(immp->elements = allocate_2D_int_mp_array(num_rows, num_cols));

    if (   (num_rows != 0) && (num_cols >= 0) 
        && (immp->elements == NULL)
       )
    {
        free_int_matrix_matrix(immp);
        return NULL;
    }

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            immp->elements[ i ][ j ] = NULL;
        }
    }

    immp->num_rows = num_rows;
    immp->num_cols = num_cols;
    immp->max_num_rows = num_rows;
    immp->max_num_cols = num_cols;
    immp->max_num_elements = num_rows * num_cols;

    return immp;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* OBSOLETE, delete soon, or convert to target and move to l lib. FIXME */

Int_matrix_matrix* create_int_matrix_matrix_with_submatrices
(
    int num_rows, 
    int num_cols, 
    int elt_rows,
    int elt_cols
)
{
    int i , j;
    Int_matrix_matrix* immp = NULL;

    NRN(immp = TYPE_MALLOC( Int_matrix_matrix ));

    NRN(immp->elements = allocate_2D_int_mp_array(num_rows, num_cols));

    if (   (num_rows != 0) && (num_cols >= 0) 
        && (elt_rows != 0) && (elt_cols >= 0) 
        && (immp->elements == NULL)
       )
    {
        free_int_matrix_matrix(immp);
        return NULL;
    }

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            get_target_int_matrix(&(immp->elements[ i ][ j ]), elt_rows, elt_cols);
        }
    }

    immp->num_rows = num_rows;
    immp->num_cols = num_cols;
    immp->max_num_rows = num_rows;
    immp->max_num_cols = num_cols;
    immp->max_num_elements = num_rows * num_cols;



    return immp;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                       allocate_2D_int_mp_array
 *
 * Allocates a 2D array of Int_matrix pointers
 *
 * This routine returns a pointer P to a two-dimensional array of Int_matrix
 * pointers. P can be de-referenced by P[row][col] to obtain the storage of a
 * Int_matrix*. P[0] points to a contiguous area of memory which contains the
 * entire array. P[1] is a short-cut pointer to the first element of the second
 * row, and so on. Thus the array can be accessed sequentually starting at
 * P[0], or explicitly by row and column. (Note that this is not the common way
 * of setting up a two-dimensional array--see below).
 *
 * This routine sets all the pointers to NULL.
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_int_mp_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_int_mp_array should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Note:
 *    Naming convention--this routine is called "allocate_2D_int_mp_array" rather
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
 *    rows cannot be swapped by simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

/* FIXME CHECK does this routine even belong here? Why not in "l"?  */

/* FIXME CHECK I have no idea why this is commented out two ways. Is the
 * need for tracking memory subsumed in the routines that are called. 
*/
/*#ifdef TRACK_MEMORY_ALLOCATION*/
#if 0
/*
Int_matrix*** debug_allocate_2D_int_mp_array(int num_rows, int num_cols,
                                     const char* file_name, int line_number)
{
    Int_matrix***   array;
    Int_matrix***   array_pos;
    Int_matrix**    col_ptr;
    int         i, j;
    Malloc_size num_bytes;


    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    num_bytes = num_rows * sizeof(Int_matrix**);
    NRN(array = (Int_matrix ***)debug_kjb_malloc(num_bytes, file_name, line_number));

    num_bytes = num_rows * num_cols * sizeof(Int_matrix*);
    col_ptr = (Int_matrix**)debug_kjb_malloc(num_bytes, file_name, line_number);

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
*/
#endif
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
/*#else  */
/* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Int_matrix*** allocate_2D_int_mp_array(int num_rows, int num_cols)
{
    Int_matrix*** array;
    Int_matrix*** array_pos;
    Int_matrix**  col_ptr;
    int       i, j;


    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(Int_matrix**, num_rows));

    col_ptr = N_TYPE_MALLOC(Int_matrix*, num_rows * num_cols);

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

/*#endif*/    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_2D_int_mp_array
 *
 * Frees an array from allocate_2D_int_mp_array
 *
 * This routine frees the storage obtained from allocate_2D_int_mp_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays, matrices
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_int_mp_array(Int_matrix*** array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_2D_int_mp_array_and_matrices
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

void free_2D_int_mp_array_and_matrices
(
    Int_matrix*** array,
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
            free_int_matrix(array[ i ][ j ]);
        }
    }

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                             free_int_matrix_matrix
 *
 * Frees the storage in a Int_matrix_matrix
 *
 * This routine frees the storage in a matrix of integer matrices. If the argument 
 * is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, integer matrix matrix
 *
 * -----------------------------------------------------------------------------
*/ 
void free_int_matrix_matrix (Int_matrix_matrix* immp)
{
    if (immp != NULL)
    {
        if (immp->elements != NULL)
        {
            int       rows;
            int       cols;
            /*int       i, j;*/

            rows = immp->num_rows;
            cols = immp->num_cols;

            /*
            for (i = 0; i < rows; i++)
            {
                for (j = 0; j < cols; j++)
                {
                    free_int_matrix( immp->elements[ i ][ j ]);
                }
            }
            */
            free_2D_int_mp_array_and_matrices( immp->elements, rows, cols );
        }
        kjb_free( immp );
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

