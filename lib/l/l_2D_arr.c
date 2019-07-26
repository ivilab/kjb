
/* $Id: l_2D_arr.c 22188 2018-08-19 17:40:36Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2014 by Kobus Barnard (author).
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
#include "l/l_2D_arr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             allocate_2D_byte_array
 *
 * Allocates a 2D array of bytes
 *
 * This routine returns a pointer P to a two-dimensional array of bytes. P can
 * be de-referenced by P[row][col] to obtain the storage of a unsigned char.
 * P[0] points to a contiguous area of memory which contains the entire array.
 * P[1] is a short-cut pointer to the first element of the second row, and so
 * on. Thus the array can be accessed sequentually starting at P[0], or
 * explicitly by row and column. (Note that this is not the common way of
 * setting up a two-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_byte_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_byte_array should be used to dispose of the storage once
 * it is no longer needed.
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
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

unsigned char** debug_allocate_2D_byte_array(int num_rows, int num_cols,
                                     const char* file_name, int line_number)
{
    unsigned char**     array;
    unsigned char**     array_pos;
    unsigned char*      col_ptr = NULL;
    int         i;
    Malloc_size num_bytes;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(unsigned char*);
    NRN(array = (unsigned char **)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_cols > 0)
    {
        num_bytes = num_rows * num_cols;
        col_ptr = (unsigned char*)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

unsigned char** allocate_2D_byte_array(int num_rows, int num_cols)
{
    unsigned char** array;
    unsigned char** array_pos;
    unsigned char*  col_ptr = NULL;
    int     i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(unsigned char*, num_rows));

    if (num_cols > 0)
    {
        col_ptr = BYTE_MALLOC(num_rows * num_cols);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_byte_array
 *
 * Frees an array from allocate_2D_byte_array
 *
 * This routine frees the storage obtained from allocate_2D_byte_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_byte_array(unsigned char** array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_2D_short_array
 *
 * Allocates a 2D array of shorts
 *
 * This routine returns a pointer P to a two-dimensional array of shorts. P can
 * be de-referenced by P[row][col] to obtain the storage of a short. P[0]
 * points to a contiguous area of memory which contains the entire array. P[1]
 * is a short-cut pointer to the first element of the second row, and so on.
 * Thus the array can be accessed sequentually starting at P[0], or explicitly
 * by row and column. (Note that this is not the common way of setting up a
 * two-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_short_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_short_array should be used to dispose of the storage once
 * it is no longer needed.
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
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

short **debug_allocate_2D_short_array(int num_rows, int num_cols,
                                      const char* file_name, int line_number)
{
    short **array, **array_pos;
    short *col_ptr = NULL;
    int i;
    Malloc_size num_bytes;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }


    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(short*);
    NRN(array = (short **)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_cols > 0)
    {
        num_bytes = num_rows * num_cols * sizeof(short);
        col_ptr = (short*)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

short **allocate_2D_short_array(int num_rows, int num_cols)
{
    short **array, **array_pos;
    short *col_ptr = NULL;
    int i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(short*, num_rows));

    if (num_cols > 0)
    {
        col_ptr = SHORT_MALLOC(num_rows * num_cols);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_short_array
 *
 * Frees an array from allocate_2D_short_array
 *
 * This routine frees the storage obtained from allocate_2D_short_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_short_array(short int** array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_2D_int_array
 *
 * Allocates a 2D array of ints
 *
 * This routine returns a pointer P to a two-dimensional array of ints. P can be
 * de-referenced by P[row][col] to obtain the storage of an int. P[0] points to
 * a contiguous area of memory which contains the entire array. P[1] is a
 * short-cut pointer to the first element of the second row, and so on. Thus
 * the array can be accessed sequentually starting at P[0], or explicitly by
 * row and column. (Note that this is not the common way of setting up a
 * two-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_int_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_int_array should be used to dispose of the storage once
 * it is no longer needed.
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
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

int** debug_allocate_2D_int_array(int num_rows, int num_cols,
                                  const char* file_name, int line_number)
{
    int** array, **array_pos;
    int* col_ptr = NULL;
    int i;
    Malloc_size num_bytes;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(int*);
    NRN(array = (int **)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_cols > 0)
    {
        num_bytes = num_rows * num_cols * sizeof(int);
        col_ptr = (int*)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int** allocate_2D_int_array(int num_rows, int num_cols)
{
    int** array, **array_pos;
    int* col_ptr = NULL;
    int i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(int*, num_rows));

    if (num_cols > 0)
    {
        col_ptr = INT_MALLOC(num_rows * num_cols);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_int_array
 *
 * Frees an array from allocate_2D_int_array
 *
 * This routine frees the storage obtained from allocate_2D_int_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_int_array(int** array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_2D_long_array
 *
 * Allocates a 2D array of longs
 *
 * This routine returns a pointer P to a two-dimensional array of longs. P can
 * be de-referenced by P[row][col] to obtain the storage of a long. P[0] points
 * to a contiguous area of memory which contains the entire array. P[1] is a
 * short-cut pointer to the first element of the second row, and so on. Thus
 * the array can be accessed sequentually starting at P[0], or explicitly by
 * row and column. (Note that this is not the common way of setting up a
 * two-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_long_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_long_array should be used to dispose of the storage once
 * it is no longer needed.
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
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

long **debug_allocate_2D_long_array(int num_rows, int num_cols,
                                    const char* file_name, int line_number)
{
    long **array, **array_pos;
    long *col_ptr = NULL;
    int i;
    Malloc_size num_bytes;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(long*);
    NRN(array = (long **)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_cols > 0)
    {
        num_bytes = num_rows * num_cols * sizeof(long);
        col_ptr = (long*)debug_kjb_malloc(num_bytes, file_name, line_number);
        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

long **allocate_2D_long_array(int num_rows, int num_cols)
{
    long **array, **array_pos;
    long *col_ptr = NULL;
    int i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(long*, num_rows));

    if (num_cols > 0)
    {
        col_ptr = LONG_MALLOC(num_rows * num_cols);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_long_array
 *
 * Frees an array from allocate_2D_long_array
 *
 * This routine frees the storage obtained from allocate_2D_long_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_long_array(long **array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_2D_float_array
 *
 * Allocates a 2D array of floats
 *
 * This routine returns a pointer P to a two-dimensional array of floats. P can
 * be de-referenced by P[row][col] to obtain the storage of a float. P[0]
 * points to a contiguous area of memory which contains the entire array. P[1]
 * is a short-cut pointer to the first element of the second row, and so on.
 * Thus the array can be accessed sequentually starting at P[0], or explicitly
 * by row and column. (Note that this is not the common way of setting up a
 * two-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_float_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_float_array should be used to dispose of the storage once
 * it is no longer needed.
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
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

float** debug_allocate_2D_float_array(int num_rows, int num_cols,
                                      const char* file_name, int line_number)
{
    float** array, **array_pos;
    float* col_ptr = NULL;
    int i;
    Malloc_size num_bytes;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(float*);
    NRN(array = (float **)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_cols > 0)
    {
        num_bytes = num_rows * num_cols * sizeof(float);
        col_ptr = (float*)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

float** allocate_2D_float_array(int num_rows, int num_cols)
{
    float** array;
    float** array_pos;
    float*  col_ptr = NULL;
    int     i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(float*, num_rows));

    if (num_cols > 0)
    {
        col_ptr = FLT_MALLOC(num_rows * num_cols);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_float_array
 *
 * Frees an array from allocate_2D_float_array
 *
 * This routine frees the storage obtained from allocate_2D_float_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_float_array(float** array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_2D_double_array
 *
 * Allocates a 2D array of doubles
 *
 * This routine returns a pointer P to a two-dimensional array of doubles. P
 * can be de-referenced by P[row][col] to obtain the storage of a double. P[0]
 * points to a contiguous area of memory which contains the entire array. P[1]
 * is a short-cut pointer to the first element of the second row, and so on.
 * Thus the array can be accessed sequentually starting at P[0], or explicitly
 * by row and column. (Note that this is not the common way of setting up a
 * two-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_double_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_double_array should be used to dispose of the storage
 * once it is no longer needed.
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
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

double** debug_allocate_2D_double_array(int num_rows, int num_cols,
                                        const char* file_name, int line_number)
{
    double**    array;
    double**    array_pos;
    double*     col_ptr = NULL;
    int         i;
    Malloc_size num_bytes;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(double*);
    NRN(array = (double **)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_cols > 0)
    {
        num_bytes = num_rows * num_cols * sizeof(double);
        col_ptr = (double*)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

double** allocate_2D_double_array(int num_rows, int num_cols)
{
    double** array, **array_pos;
    double* col_ptr = NULL;
    int i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(double*, num_rows));

    if (num_cols > 0)
    {
        col_ptr = DBL_MALLOC(num_rows * num_cols);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }
    
    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_double_array
 *
 * Frees an array from allocate_2D_double_array
 *
 * This routine frees the storage obtained from allocate_2D_double_array. If
 * the argument is NULL, then this routine returns safely without doing
 * anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_double_array(double** array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             allocate_2D_ptr_array
 *
 * Allocates a 2D array of pointers to void
 *
 * This routine returns a pointer, P, to a two-dimensional array of pointers to
 * void. P can be de-referenced by P[row][col] to obtain the storage of a ptr.
 * P[0] points to a contiguous area of memory which contains the entire array.
 * P[1] is a short-cut pointer to the first element of the second row, and so
 * on.  Thus the array can be accessed sequentually starting at P[0], or
 * explicitly by row and column. (Note that this is not the common way of
 * setting up a two-dimensional array--see below).
 *
 * If TEST is #defined (unix only) then this routine is #define'd to be
 * debug_allocate_2D_ptr_array, which is the version available in the
 * development library. In development code, memory is tracked so that memory
 * leaks can be found more easily. Furthermore, all memory free'd is checked
 * that it was allocated by a KJB library routine. Finally, memory is checked
 * for overuns.
 *
 * The routine free_2D_ptr_array should be used to dispose of the storage
 * once it is no longer needed.
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
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

void*** debug_allocate_2D_ptr_array(int num_rows, int num_cols,
                                    const char* file_name, int line_number)
{
    void***    array;
    void***    array_pos;
    void**     col_ptr = NULL;
    int         i;
    Malloc_size num_bytes;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    /*
    //  Use debug_kjb_malloc as opposed to macros to pass down file_name
    //  and line_number.
    */
    num_bytes = num_rows * sizeof(void**);
    NRN(array = (void* **)debug_kjb_malloc(num_bytes, file_name, line_number));

    if (num_cols > 0)
    {
        num_bytes = num_rows * num_cols * sizeof(void*);
        col_ptr = (void**)debug_kjb_malloc(num_bytes, file_name, line_number);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}
        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

void*** allocate_2D_ptr_array(int num_rows, int num_cols)
{
    void*** array;
    void*** array_pos;
    void**  col_ptr = NULL;
    int     i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    NRN(array = N_TYPE_MALLOC(void**, num_rows));

    if (num_cols > 0)
    {
        col_ptr = N_TYPE_MALLOC(void*, num_rows * num_cols);

        if (col_ptr == NULL)
        {
            kjb_free(array);
            return NULL;
        }
    }
    else 
    {
        col_ptr = NULL;
    }

    array_pos = array;

    for(i=0; i<num_rows; i++)
    {
        *array_pos = col_ptr;
        array_pos++;
        col_ptr += num_cols;
    }

    return array;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_ptr_array
 *
 * Frees an array from allocate_2D_ptr_array
 *
 * This routine frees the storage obtained from allocate_2D_ptr_array. If the
 * argument is NULL, then this routine returns safely without doing anything.
 *
 * Index: memory allocation, arrays
 *
 * ==> Test program: 2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

void free_2D_ptr_array(void*** array)
{


    if (array == NULL) return;

    kjb_free(*array);
    kjb_free(array);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


