
/* $Id: l_2D_arr.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef L_2D_ARRAY_INCLUDED
#define L_2D_ARRAY_INCLUDED


#include "l/l_def.h"
#include "l/l_sys_mal.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* =============================================================================
 *                             allocate_2D_int16_array
 *
 * (MACRO) Allocates a 2D array of 16 bit integers
 *
 * This macro expands to a 2D array allocation routine appropriate for 16 bit
 * integers.
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
 * Index: memory allocation, memory tracking, Macros
 *
 * ==> Test program: l_2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    kjb_int16 **allocate_2D_int16_array(int num_rows, int num_cols);
#endif

/*
// Definition of allocate_2D_int16_array in terms of other macros, at least
// in the case of TRACK_MEMORY_ALLOCATION has changed to a more direct
// definition below, because some brain dead pre-processor (next) could not
// handle the previous definition.
*/

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_int16_array
 *
 * (MACRO) Frees array from from allocate_2D_int16_array
 *
 * This macro expands to the appropriate routine to free the storage obtained
 * from allocate_2D_int16_array. If the argument is NULL, then it returns
 * safely without doing anything.
 *
 * Index: memory allocation, memory tracking, Macros
 *
 * ==> Test program: l_2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
     void free_2D_int16_array(kjb_int16 **array);
#else
#ifdef INT16_IS_SHORT
#    define free_2D_int16_array     free_2D_short_array
#else
#ifdef INT16_IS_INT
#    define free_2D_int16_array     free_2D_int_array
#endif
#endif
#endif

/* =============================================================================
 *                             allocate_2D_int32_array
 *
 * (MACRO) Allocates a 2D array of 32 bit integers
 *
 * This macro expands to a 2D array allocation routine appropriate for 32 bit
 * integers.
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
 * Index: memory allocation, memory tracking, Macros
 *
 * ==> Test program: l_2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    kjb_int32 **allocate_2D_int32_array(int num_rows, int num_cols);
#endif

/*
// Definition of allocate_2D_int32_array in terms of other macros, at least
// in the case of TRACK_MEMORY_ALLOCATION has changed to a more direct
// definition below, because some brain dead pre-processor (next) could not
// handle the previous definition.
*/

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_2D_int32_array
 *
 * (MACRO) Frees array from from allocate_2D_int32_array
 *
 * This macro expands to the appropriate routine to free the storage
 * obtained from allocate_2D_int32_array. If the argument is NULL,
 * then it returns safely without doing anything.
 *
 * Index: memory allocation, memory tracking, Macros
 *
 * ==> Test program: l_2D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
     void free_2D_int32_array(int **array);
#else
#ifdef INT32_IS_LONG
#    define free_2D_int32_array     free_2D_long_array
#else
#ifdef INT32_IS_INT
#    define free_2D_int32_array     free_2D_int_array
#endif
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
#   define allocate_2D_byte_array(x, y) \
                 debug_allocate_2D_byte_array((x), (y), __FILE__, __LINE__)

#   define allocate_2D_short_array(x, y) \
                 debug_allocate_2D_short_array((x), (y), __FILE__, __LINE__)

#   define allocate_2D_int_array(x, y) \
                 debug_allocate_2D_int_array((x), (y), __FILE__, __LINE__)

#   define allocate_2D_long_array(x, y) \
                 debug_allocate_2D_long_array((x), (y), __FILE__, __LINE__)

#   define allocate_2D_float_array(x, y) \
                 debug_allocate_2D_float_array((x), (y), __FILE__, __LINE__)

#   define allocate_2D_double_array(x, y) \
                 debug_allocate_2D_double_array((x), (y), __FILE__, __LINE__)

#   define allocate_2D_ptr_array(x, y) \
                 debug_allocate_2D_ptr_array((x), (y), __FILE__, __LINE__)

#ifndef __C2MAN__

#ifdef INT16_IS_SHORT
#    define allocate_2D_int16_array(x, y) \
                 debug_allocate_2D_short_array((x), (y), __FILE__, __LINE__)
#else
#ifdef INT16_IS_INT
#    define allocate_2D_int16_array(x, y) \
                 debug_allocate_2D_int_array((x), (y), __FILE__, __LINE__)
#endif
#endif

#ifdef INT32_IS_LONG
#    define allocate_2D_int32_array(x, y) \
                 debug_allocate_2D_long_array((x), (y), __FILE__, __LINE__)
#else
#ifdef INT32_IS_INT
#    define allocate_2D_int32_array(x, y) \
                 debug_allocate_2D_int_array((x), (y), __FILE__, __LINE__)
#endif
#endif

#endif

    unsigned char**  debug_allocate_2D_byte_array
    (
        int         ,
        int         ,
        const char* ,
        int
    );

    short**  debug_allocate_2D_short_array
    (
        int         ,
        int         ,
        const char* ,
        int
    );

    int**    debug_allocate_2D_int_array
    (
        int         ,
        int         ,
        const char* ,
        int
    );

    long**   debug_allocate_2D_long_array
    (
        int         ,
        int         ,
        const char* ,
        int
    );

    float**  debug_allocate_2D_float_array
    (
        int         ,
        int         ,
        const char* ,
        int
    );

    double** debug_allocate_2D_double_array
    (
        int         ,
        int         ,
        const char* ,
        int
    );


    void*** debug_allocate_2D_ptr_array
    (
        int         ,
        int         ,
        const char* ,
        int
    );


        /*  ==>                                              /\               */
        /*  ==>  TRACK_MEMORY_ALLOCATION (Development code)  ||               */
#else   /* ------------------------------------------------------------------ */
        /*  ==>                           Production code    ||               */
        /*  ==>                                              \/               */

#ifndef __C2MAN__

#ifdef INT16_IS_SHORT
#    define allocate_2D_int16_array    allocate_2D_short_array
#else
#ifdef INT16_IS_INT
#    define allocate_2D_int16_array    allocate_2D_int_array
#endif
#endif

#ifdef INT32_IS_LONG
#    define allocate_2D_int32_array    allocate_2D_long_array
#else
#ifdef INT32_IS_INT
#    define allocate_2D_int32_array    allocate_2D_int_array
#endif
#endif


#endif

    unsigned char** allocate_2D_byte_array  (int, int);
    short**         allocate_2D_short_array (int, int);
    int**           allocate_2D_int_array   (int, int);
    long**          allocate_2D_long_array  (int, int);
    float**         allocate_2D_float_array (int, int);
    double**        allocate_2D_double_array(int, int);
    void***         allocate_2D_ptr_array   (int, int);

#endif  /* End of case NOT TRACK_MEMORY_ALLOCATION */


void free_2D_byte_array  (unsigned char**);
void free_2D_short_array (short**);
void free_2D_int_array   (int**);
void free_2D_long_array  (long**);
void free_2D_float_array (float**);
void free_2D_double_array(double**);
void free_2D_ptr_array(void***);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif


#endif


