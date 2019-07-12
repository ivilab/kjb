
/* $Id: l_3D_arr.h 4899 2009-11-28 20:50:49Z kobus $ */

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

#ifndef L_3D_ARRAY_INCLUDED
#define L_3D_ARRAY_INCLUDED


#include "l/l_def.h"
#include "l/l_sys_mal.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* =============================================================================
 *                             allocate_3D_int16_array
 *
 * (MACRO) Allocates a 3D array of 16 bit integers
 *
 * This macro expands to a 3D array allocation routine appropriate for 16 bit
 * integers.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a two-dimensional array in "C", where each row is allocated
 *    separately. Here the storage area is contiguous. This allows for certain
 *    operations to be done quickly, but note the following IMPORTANT point:
 *    num_rows cannot be swapped by simply swapping row pointers!
 *
 * Index: memory allocation, memory tracking, Macros
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * ==> Test program: l_3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    kjb_int16 **allocate_3D_int16_array(int num_rows, int num_cols);
#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line (only).
                      I have no idea what the problem is. */
#ifdef INT16_IS_SHORT
#    define allocate_3D_int16_array allocate_3D_short_array
#else
#ifdef INT16_IS_INT
#    define allocate_3D_int16_array allocate_3D_int_array
#endif
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_int16_array
 *
 * (MACRO) Frees an array from allocate_3D_int16_array
 *
 * This macro expands to the appropriate routine to free the storage obtained
 * from allocate_3D_int16_array. If the argument is NULL, then it returns
 * safely without doing anything.
 *
 * Index: memory allocation, memory tracking, Macros
 *
 * ==> Test program: l_3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
     void free_3D_int16_array(kjb_int16 **array);
#else
#ifdef INT16_IS_SHORT
#    define free_3D_int16_array     free_3D_short_array
#else
#ifdef INT16_IS_INT
#    define free_3D_int16_array     free_3D_int_array
#endif
#endif
#endif

/* =============================================================================
 *                             allocate_3D_int32_array
 *
 * (MACRO) Allocates a 3D array of 32 bit integers
 *
 * This macro expands to a 3D array allocation routine appropriate for 32 bit
 * integers.
 *
 * Warning:
 *    The structure of the returned array is somewhat different than a more
 *    common form of a three-dimensional array in "C" where each block, and
 *    each row within each block, is allocated separately. Here the storage area
 *    is contiguous. This allows for certain operations to be done quickly, but
 *    note the following IMPORTANT point: Blocks and rows cannot be swapped by
 *    simply swapping row pointers!
 *
 * Index: memory allocation, memory tracking, Macros
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * ==> Test program: l_3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
    kjb_int32 **allocate_3D_int32_array(int num_rows, int num_cols);
#else
#ifdef INT32_IS_LONG
#    define allocate_3D_int32_array allocate_3D_long_array
#else
#ifdef INT32_IS_INT
#    define allocate_3D_int32_array allocate_3D_int_array
#endif
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_3D_int32_array
 *
 * (MACRO) Frees an array from allocate_3D_int32_array
 *
 * This macro expands to the appropriate routine to free the storage obtained
 * from allocate_3D_int32_array. If the argument is NULL, then it returns
 * safely without doing anything.
 *
 * Index: memory allocation, memory tracking, Macros
 *
 * ==> Test program: l_3D_arr.c
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__
     void free_3D_int32_array(kjb_int32 **array);
#else
#ifdef INT32_IS_LONG
#    define free_3D_int32_array     free_3D_long_array
#else
#ifdef INT32_IS_INT
#    define free_3D_int32_array     free_3D_int_array
#endif
#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION
#   define allocate_3D_byte_array(x, y, z) \
               debug_allocate_3D_byte_array((x), (y), (z),  __FILE__, __LINE__)

#   define allocate_3D_short_array(x, y, z) \
               debug_allocate_3D_short_array((x), (y), (z),  __FILE__, __LINE__)

#   define allocate_3D_int_array(x, y, z) \
               debug_allocate_3D_int_array((x), (y), (z),  __FILE__, __LINE__)

#   define allocate_3D_long_array(x, y, z) \
               debug_allocate_3D_long_array((x), (y), (z),  __FILE__, __LINE__)

#   define allocate_3D_float_array(x, y, z) \
               debug_allocate_3D_float_array((x), (y), (z),  __FILE__, __LINE__)

#   define allocate_3D_double_array(x, y, z) \
               debug_allocate_3D_double_array((x), (y), (z), __FILE__, __LINE__)

#   define allocate_3D_ptr_array(x, y, z) \
               debug_allocate_3D_ptr_array((x), (y), (z), __FILE__, __LINE__)

    unsigned char***  debug_allocate_3D_byte_array
    (
        int         ,
        int         ,
        int         ,
        const char* ,
        int
    );

    short***  debug_allocate_3D_short_array
    (
        int         ,
        int         ,
        int         ,
        const char* ,
        int
    );

    int***    debug_allocate_3D_int_array
    (
        int         ,
        int         ,
        int         ,
        const char* ,
        int
    );

    long***   debug_allocate_3D_long_array
    (
        int         ,
        int         ,
        int         ,
        const char* ,
        int
    );

    float***  debug_allocate_3D_float_array
    (
        int         ,
        int         ,
        int         ,
        const char* ,
        int
    );

    double*** debug_allocate_3D_double_array
    (
        int         ,
        int         ,
        int         ,
        const char* ,
        int
    );

    void****  debug_allocate_3D_ptr_array
    (
        int         ,
        int         ,
        int         ,
        const char* ,
        int
    );

#else
    unsigned char***  allocate_3D_byte_array  (int, int, int);
    short***  allocate_3D_short_array (int, int, int);
    int***    allocate_3D_int_array   (int, int, int);
    long***   allocate_3D_long_array  (int, int, int);
    float***  allocate_3D_float_array (int, int, int);
    double*** allocate_3D_double_array(int, int, int);
    void****  allocate_3D_ptr_array   (int, int, int);
#endif

void free_3D_byte_array(unsigned char***);
void free_3D_short_array (short***);
void free_3D_int_array   (int***);
void free_3D_long_array  (long***);
void free_3D_float_array (float***);
void free_3D_double_array(double***);
void free_3D_ptr_array   (void****);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

