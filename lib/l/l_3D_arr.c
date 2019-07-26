
/* $Id: l_3D_arr.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */
#include "l/l_3D_arr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             allocate_3D_byte_array
 *
 * Allocates a 3D array of bytes
 *
 * This routine returns a pointer P to a three-dimensional array of bytes. P
 * can be de-referenced by P[block][row][col] to obtain the storage of a unsigned char.
 * P[0][0] points to a contiguous area of memory which contains the entire
 * array.  Thus the array can be accessed sequentually starting at P[0][0], or
 * explicitly by block, row and column. (Note that this is not the common way
 * of setting up a three-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_3D_byte_array, which is the version available in the
 * developement library. In developement code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_3D_byte_array should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a three-dimensional array in "C" where each block, and
 *    each row within each block, is allocated separately. Here the storage area
 *    is contiguous. This allows for certain operations to be done quickly, but
 *    note the following IMPORTANT point: Blocks and rows cannot be swapped by
 *    simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

unsigned char*** debug_allocate_3D_byte_array(int num_blocks, int num_rows,int num_cols,
                                              const char* file_name, int line_number)
{
    unsigned char***    array;
    unsigned char***    array_pos;
    unsigned char**     row_ptr = NULL;
    unsigned char*      col_ptr = NULL;
    int         i;
    int         j;
    Malloc_size num_bytes;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_blocks * sizeof(unsigned char**);
    NRN(array = (unsigned char ***)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_rows > 0)
    {
        num_bytes = num_blocks * num_rows * sizeof(unsigned char*);

        row_ptr = (unsigned char **)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            num_bytes = num_blocks * num_rows * num_cols;
            col_ptr = (unsigned char*)debug_kjb_malloc(num_bytes, file_name, line_number);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
    }
    else 
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

unsigned char*** allocate_3D_byte_array(int num_blocks, int num_rows, int num_cols)
{
    unsigned char*** array;
    unsigned char*** array_pos;
    unsigned char**  row_ptr = NULL;
    unsigned char*   col_ptr = NULL;
    int      i;
    int      j;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(unsigned char**, num_blocks));

    if (num_rows > 0)
    {
        row_ptr = N_TYPE_MALLOC(unsigned char*, num_blocks * num_rows);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            col_ptr = BYTE_MALLOC(num_blocks * num_rows * num_cols);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
    }
    else 
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_byte_array
 *
 * Frees an array from allocate_3D_byte_array
 *
 * This routine frees the storage obtained from allocate_3D_byte_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_3D_byte_array(unsigned char*** array)
{


    if (array == NULL) return;

    if (*array != NULL)
    {
        kjb_free(**array);
    }

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_3D_short_array
 *
 * Allocates a 3D array of shorts
 *
 * This routine returns a pointer P to a three-dimensional array of shorts. P
 * can be de-referenced by P[block][row][col] to obtain the storage of a short.
 * P[0][0] points to a contiguous area of memory which contains the entire
 * array.  Thus the array can be accessed sequentually starting at P[0][0], or
 * explicitly by block, row and column. (Note that this is not the common way
 * of setting up a three-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_3D_short_array, which is the version available in the
 * developement library. In developement code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_3D_short_array should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a three-dimensional array in "C" where each block, and
 *    each row within each block, is allocated separately. Here the storage area
 *    is contiguous. This allows for certain operations to be done quickly, but
 *    note the following IMPORTANT point: Blocks and rows cannot be swapped by
 *    simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

short ***debug_allocate_3D_short_array(int num_blocks,int num_rows,int num_cols,
                                       const char* file_name, int line_number)
{
    short***    array;
    short***    array_pos;
    short**     row_ptr = NULL;
    short*      col_ptr = NULL;
    int         i;
    int         j;
    Malloc_size num_bytes;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
     */
    num_bytes = num_blocks * sizeof(short**);
    NRN(array = (short ***)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_rows > 0)
    {
        num_bytes = num_blocks * num_rows *sizeof(short*);
        row_ptr = (short **)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            num_bytes = num_blocks * num_rows * num_cols * sizeof(short);
            col_ptr = (short*)debug_kjb_malloc(num_bytes, file_name, line_number);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }

    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

short ***allocate_3D_short_array(int num_blocks, int num_rows, int num_cols)
{
    short*** array;
    short*** array_pos;
    short**  row_ptr = NULL;
    short*   col_ptr = NULL;
    int      i;
    int      j;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(short**, num_blocks));

    if (num_rows > 0)
    {
        row_ptr = N_TYPE_MALLOC(short*, num_blocks * num_rows);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            col_ptr = SHORT_MALLOC(num_blocks * num_rows * num_cols);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_short_array
 *
 * Frees an array from allocate_3D_short_array
 *
 * This routine frees the storage obtained from allocate_3D_short_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_3D_short_array(short int*** array)
{


    if (array == NULL) return;

    if (*array != NULL)
    {
        kjb_free(**array);
    }

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_3D_int_array
 *
 * Allocates a 3D array of ints
 *
 * This routine returns a pointer P to a three-dimensional array of ints. P can
 * be de-referenced by P[block][row][col] to obtain the storage of an int.
 * P[0][0] points to a contiguous area of memory which contains the entire
 * array.  Thus the array can be accessed sequentually starting at P[0][0], or
 * explicitly by block, row and column. (Note that this is not the common way
 * of setting up a three-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_3D_int_array, which is the version available in the
 * developement library. In developement code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_3D_int_array should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a three-dimensional array in "C" where each block, and
 *    each row within each block, is allocated separately. Here the storage area
 *    is contiguous. This allows for certain operations to be done quickly, but
 *    note the following IMPORTANT point: Blocks and rows cannot be swapped by
 *    simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int*** debug_allocate_3D_int_array(int num_blocks, int num_rows, int num_cols,
                                   const char* file_name, int line_number)
{
    int***      array;
    int***      array_pos;
    int**       row_ptr = NULL;
    int*        col_ptr = NULL;
    int         i;
    int         j;
    Malloc_size num_bytes;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_blocks * sizeof(int**);
    NRN(array = (int ***)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_rows > 0)
    {
        num_bytes = num_blocks * num_rows * sizeof(int*);
        row_ptr = (int **)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            num_bytes = num_blocks * num_rows * num_cols * sizeof(int);
            col_ptr = (int*)debug_kjb_malloc(num_bytes, file_name, line_number);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL; 
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int*** allocate_3D_int_array(int num_blocks, int num_rows, int num_cols)
{
    int*** array;
    int*** array_pos;
    int**  row_ptr = NULL;
    int*   col_ptr = NULL;
    int    i;
    int    j;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(int**, num_blocks));

    if (num_rows > 0) 
    {
        row_ptr = N_TYPE_MALLOC(int*, num_blocks * num_rows);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    
        if (num_cols > 0)
        {
            col_ptr = INT_MALLOC(num_blocks * num_rows * num_cols);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else
        {
            col_ptr = NULL;
        }
    }
    else 
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_int_array
 *
 * Frees an array from allocate_3D_int_array
 *
 * This routine frees the storage obtained from allocate_3D_int_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_3D_int_array(int*** array)
{


    if (array == NULL) return;

    if (*array != NULL)
    {
        kjb_free(**array);
    }

    kjb_free(*array);
    kjb_free(array);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_3D_long_array
 *
 * Allocates a 3D array of longs
 *
 * This routine returns a pointer P to a three-dimensional array of longs. P
 * can be de-referenced by P[block][row][col] to obtain the storage of a long.
 * P[0][0] points to a contiguous area of memory which contains the entire
 * array.  Thus the array can be accessed sequentually starting at P[0][0], or
 * explicitly by block, row and column. (Note that this is not the common way
 * of setting up a three-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_3D_long_array, which is the version available in the
 * developement library. In developement code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_3D_long_array should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a three-dimensional array in "C" where each block, and
 *    each row within each block, is allocated separately. Here the storage area
 *    is contiguous. This allows for certain operations to be done quickly, but
 *    note the following IMPORTANT point: Blocks and rows cannot be swapped by
 *    simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

long ***debug_allocate_3D_long_array(int num_blocks, int num_rows, int num_cols,
                                     const char* file_name, int line_number)
{
    long***     array;
    long***     array_pos;
    long**      row_ptr = NULL;
    long*       col_ptr = NULL;
    int         i;
    int         j;
    Malloc_size num_bytes;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_blocks * sizeof(long**);
    NRN(array = (long ***)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_rows > 0)
    {
        num_bytes = num_blocks * num_rows * sizeof(long*);
        row_ptr = (long **)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            num_bytes = num_blocks * num_rows * num_cols * sizeof(long);
            col_ptr = (long*)debug_kjb_malloc(num_bytes, file_name, line_number);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

long ***allocate_3D_long_array(int num_blocks, int num_rows, int num_cols)
{
    long*** array;
    long*** array_pos;
    long**  row_ptr = NULL;
    long*   col_ptr = NULL;
    int     i;
    int     j;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(long**, num_blocks));

    if (num_rows > 0)
    {
        row_ptr =  N_TYPE_MALLOC(long*, num_blocks * num_rows);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            col_ptr = LONG_MALLOC(num_blocks * num_rows * num_cols);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}
#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_long_array
 *
 * Frees an array from allocate_3D_long_array
 *
 * This routine frees the storage obtained from allocate_3D_long_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_3D_long_array(long int*** array)
{


    if (array == NULL) return;

    if (*array != NULL)
    {
        kjb_free(**array);
    }

    kjb_free(*array);
    kjb_free(array);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_3D_float_array
 *
 * Allocates a 3D array of floats
 *
 * This routine returns a pointer P to a three-dimensional array of floats. P
 * can be de-referenced by P[block][row][col] to obtain the storage of a float.
 * P[0][0] points to a contiguous area of memory which contains the entire
 * array.  Thus the array can be accessed sequentually starting at P[0][0], or
 * explicitly by block, row and column. (Note that this is not the common way
 * of setting up a three-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_3D_float_array, which is the version available in the
 * developement library. In developement code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_3D_float_array should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a three-dimensional array in "C" where each block, and
 *    each row within each block, is allocated separately. Here the storage area
 *    is contiguous. This allows for certain operations to be done quickly, but
 *    note the following IMPORTANT point: Blocks and rows cannot be swapped by
 *    simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

float*** debug_allocate_3D_float_array(int num_blocks,int num_rows,int num_cols,
                                       const char* file_name, int line_number)
{
    float***    array;
    float***    array_pos;
    float**     row_ptr = NULL;
    float*      col_ptr = NULL;
    int         i;
    int         j;
    Malloc_size num_bytes;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_blocks * sizeof(float**);
    NRN(array = (float ***)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_rows > 0)
    {
        num_bytes = num_blocks * num_rows * sizeof(float*);
        row_ptr = (float **)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            num_bytes = num_blocks * num_rows * num_cols * sizeof(float);
            col_ptr = (float*)debug_kjb_malloc(num_bytes, file_name, line_number);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

float*** allocate_3D_float_array(int num_blocks, int num_rows, int num_cols)
{
    float*** array;
    float*** array_pos;
    float**  row_ptr = NULL;
    float*   col_ptr = NULL;
    int      i;
    int      j;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(float**, num_blocks));

    if (num_rows > 0)
    {
        row_ptr = N_TYPE_MALLOC(float*, num_blocks * num_rows);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            col_ptr = FLT_MALLOC(num_blocks * num_rows * num_cols);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_float_array
 *
 * Frees an array from allocate_3D_float_array
 *
 * This routine frees the storage obtained from allocate_3D_float_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_3D_float_array(float*** array)
{


    if (array == NULL) return;

    if (*array != NULL)
    {
        kjb_free(**array);
    }

    kjb_free(*array);
    kjb_free(array);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_3D_double_array
 *
 * Allocates a 3D array of doubles
 *
 * This routine returns a pointer P to a three-dimensional array of doubles. P
 * can be de-referenced by P[block][row][col] to obtain the storage of a
 * double. P[0][0] points to a contiguous area of memory which contains the
 * entire array.  Thus the array can be accessed sequentually starting at
 * P[0][0], or explicitly by block, row and column. (Note that this is not the
 * common way of setting up a three-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_3D_double_array, which is the version available in the
 * developement library. In developement code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_3D_double_array should be used to dispose of the storage
 * once it is no longer needed.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a three-dimensional array in "C" where each block, and
 *    each row within each block, is allocated separately. Here the storage area
 *    is contiguous. This allows for certain operations to be done quickly, but
 *    note the following IMPORTANT point: Blocks and rows cannot be swapped by
 *    simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

double*** debug_allocate_3D_double_array(int num_blocks,
                                         int num_rows,
                                         int num_cols,
                                         const char* file_name,
                                         int line_number)
{
    double***   array;
    double***   array_pos;
    double**    row_ptr = NULL;
    double*     col_ptr = NULL;
    int         i;
    int         j;
    Malloc_size num_bytes;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_blocks * sizeof(double**);
    NRN(array = (double ***)debug_kjb_malloc(num_bytes, file_name,
                                             line_number));

    if (num_rows > 0)
    {
        num_bytes = num_blocks * num_rows * sizeof(double*);
        row_ptr = (double**)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            num_bytes = num_blocks * num_rows * num_cols * sizeof(double);
            col_ptr = (double*)debug_kjb_malloc(num_bytes, file_name, line_number);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

double*** allocate_3D_double_array(int num_blocks, int num_rows, int num_cols)
{
    double*** array;
    double*** array_pos;
    double**  row_ptr = NULL;
    double*   col_ptr = NULL;
    int       i;
    int       j;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(double**, num_blocks));

    if (num_rows > 0)
    {
        row_ptr = N_TYPE_MALLOC(double*, num_blocks * num_rows);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            col_ptr = DBL_MALLOC(num_blocks * num_rows * num_cols);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_double_array
 *
 * Frees an array from allocate_3D_double_array
 *
 * This routine frees the storage obtained from allocate_3D_double_array. If
 * the argument is NULL, then this routine returns safely without doing
 * anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_3D_double_array(double*** array)
{


    if (array == NULL) return;

    if (*array != NULL)
    {
        kjb_free(**array);
    }

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_3D_ptr_array
 *
 * Allocates a 3D array of pointers
 *
 * This routine returns a pointer P to a three-dimensional array of pointers. P
 * can be de-referenced by P[block][row][col] to obtain the storage of a
 * ptr. P[0][0] points to a contiguous area of memory which contains the
 * entire array.  Thus the array can be accessed sequentually starting at
 * P[0][0], or explicitly by block, row and column. (Note that this is not the
 * common way of setting up a three-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_3D_ptr_array, which is the version available in the
 * developement library. In developement code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_3D_ptr_array should be used to dispose of the storage
 * once it is no longer needed.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a three-dimensional array in "C" where each block, and
 *    each row within each block, is allocated separately. Here the storage area
 *    is contiguous. This allows for certain operations to be done quickly, but
 *    note the following IMPORTANT point: Blocks and rows cannot be swapped by
 *    simply swapping row pointers!
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

void**** debug_allocate_3D_ptr_array(int num_blocks, int num_rows, int num_cols,
                                     const char* file_name, int line_number)
{
    void****   array;
    void****   array_pos;
    void***    row_ptr = NULL;
    void**     col_ptr = NULL;
    int         i;
    int         j;
    Malloc_size num_bytes;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_blocks * sizeof(void***);
    NRN(array = (void ****)debug_kjb_malloc(num_bytes, file_name,
                                             line_number));

    if (num_rows > 0)
    {
        num_bytes = num_blocks * num_rows * sizeof(void**);
        row_ptr = (void***)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            num_bytes = num_blocks * num_rows * num_cols * sizeof(void*);
            col_ptr = (void**)debug_kjb_malloc(num_bytes, file_name, line_number);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

void**** allocate_3D_ptr_array(int num_blocks, int num_rows, int num_cols)
{
    void**** array;
    void**** array_pos;
    void***  row_ptr = NULL;
    void**   col_ptr = NULL;
    int       i;
    int       j;


    if ((num_blocks < 0) || (num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_blocks == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(void***, num_blocks));

    if (num_rows > 0)
    {
        row_ptr = N_TYPE_MALLOC(void**, num_blocks * num_rows);

        if (row_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }

        if (num_cols > 0)
        {
            col_ptr = N_TYPE_MALLOC(void*, num_blocks * num_rows * num_cols);

            if (col_ptr == NULL)
            {
                kjb_free(array);
                kjb_free(row_ptr);
                return NULL;
            }
        }
        else 
        {
            col_ptr = NULL;
        }
    }
    else
    {
        row_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_blocks; i++)
    {
        *array_pos = row_ptr;
        array_pos++;

        for(j=0; j<num_rows; j++)
        {
            *row_ptr = col_ptr;
            col_ptr += num_cols;
            row_ptr++;
        }
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_void*_array
 *
 * Frees an array from allocate_3D_void*_array
 *
 * This routine frees the storage obtained from allocate_3D_void*_array. If
 * the argument is NULL, then this routine returns safely without doing
 * anything.
 *
 * Index: memory allocation, memory tracking, Macros
 *
 * ==> Test program: 3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_3D_ptr_array(void * ***array)
{


    if (array == NULL) return;

    if (*array != NULL)
    {
        kjb_free(**array);
    }

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

