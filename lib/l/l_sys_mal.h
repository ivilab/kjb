
/* $Id: l_sys_mal.h 21520 2017-07-22 15:09:04Z kobus $ */

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

#ifndef L_SYS_MAL_INCLUDED
#define L_SYS_MAL_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifdef __C2MAN__
#ifndef TRACK_MEMORY_ALLOCATION_OR_C2MAN
#define TRACK_MEMORY_ALLOCATION_OR_C2MAN
#endif
#endif

#ifdef TRACK_MEMORY_ALLOCATION
#ifndef TRACK_MEMORY_ALLOCATION_OR_C2MAN
#define TRACK_MEMORY_ALLOCATION_OR_C2MAN
#endif
#endif


#ifdef TRACK_MEMORY_ALLOCATION
#    define kjb_malloc(x)   \
                       debug_kjb_malloc((Malloc_size)(x), __FILE__ , __LINE__ )

#    define kjb_calloc(x, y)  \
                debug_kjb_calloc((Malloc_size)(x), (Malloc_size)(y), __FILE__ ,\
                                 __LINE__ )

#    define kjb_realloc(p, x)   \
                       debug_kjb_realloc(p, (Malloc_size)(x), __FILE__ , __LINE__ )


/*
 * The following are used ONLY for writing debug versions of constructor
 * functions. Normally we use the non-debug versions regardless of whether
 * TRACK_MEMORY_ALLOCATION is defined.  The macro variables x and y invariably
 * are file_name and line_number.
*/

#define DEBUG_KJB_MALLOC(x, y, z)       debug_kjb_malloc((Malloc_size)(x), y, z)

#define DEBUG_KJB_CALLOC(u, v, y, z) \
                    debug_kjb_calloc((Malloc_size)(u), (Malloc_size)(v), y, z)

#define DEBUG_KJB_REALLOC(p, x, y, z)       debug_kjb_realloc(p, (Malloc_size)(x), y, z)

#define DEBUG_TYPE_MALLOC(t, y, z)      (t *)debug_kjb_malloc(sizeof(t), y, z)
#define DEBUG_TYPE_REALLOC(p, t, y, z)  (t *)debug_kjb_realloc(p, sizeof(t), y, z)

#define DEBUG_N_TYPE_MALLOC(t, n, y, z) \
              (t *)debug_kjb_malloc((sizeof(t))*((kjb_int32)(n)), y, z)
#define DEBUG_N_TYPE_REALLOC(p, t, n, y, z) \
              (t *)debug_kjb_realloc(p, (sizeof(t))*((kjb_int32)(n)), y, z)

#define DEBUG_UCHAR_MALLOC(x, y, z)  \
            (unsigned char*)debug_kjb_malloc((Malloc_size)(x), y, z)
#define DEBUG_UCHAR_REALLOC(p, x, y, z)  \
            (unsigned char*)debug_kjb_realloc(p, (Malloc_size)(x), y, z)

#define DEBUG_BYTE_MALLOC(x, y, z) \
            (Byte*)debug_kjb_malloc((Malloc_size)(x), y, z)
#define DEBUG_BYTE_REALLOC(p, x, y, z) \
            (Byte*)debug_kjb_realloc(p, (Malloc_size)(x), y, z)

#define DEBUG_STR_MALLOC(x, y, z) \
            (char*)debug_kjb_malloc((Malloc_size)(x), y, z)
#define DEBUG_STR_REALLOC(p, x, y, z) \
            (char*)debug_kjb_realloc(p, (Malloc_size)(x), y, z)

#define DEBUG_SHORT_MALLOC(x, y, z) \
          (short*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(short)), y, z)
#define DEBUG_SHORT_REALLOC(p, x, y, z) \
          (short*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(short)), y, z)

#define DEBUG_INT_MALLOC(x, y, z)    \
          (int*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(int)), y, z)
#define DEBUG_INT_REALLOC(p, x, y, z)    \
          (int*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(int)), y, z)

#define DEBUG_UINT_MALLOC(x, y, z)    \
          (unsigned int*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(unsigned int)), y, z)
#define DEBUG_UINT_REALLOC(p, x, y, z)    \
          (unsigned int*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(unsigned int)), y, z)

#define DEBUG_LONG_MALLOC(x, y, z)   \
          (long*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(Malloc_size)), \
                                  y, z)
#define DEBUG_LONG_REALLOC(p, x, y, z)   \
          (long*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(Malloc_size)), \
                                   y, z)

#define FLT_DEBUG_MALLOC(x, y, z)  \
          (float*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(float)), y, z)
#define FLT_DEBUG_REALLOC(p, x, y, z)  \
          (float*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(float)), y, z)

#define DBL_DEBUG_MALLOC(x, y, z) \
          (double*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(double)), y, z)
#define DBL_DEBUG_REALLOC(p, x, y, z) \
          (double*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(double)), y, z)

#define DEBUG_INT16_MALLOC(x, y, z)  \
          (kjb_int16*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(kjb_int16)), y, z)
#define DEBUG_INT16_REALLOC(p, x, y, z)  \
          (kjb_int16*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(kjb_int16)), y, z)

#define DEBUG_UINT16_MALLOC(x, y, z) \
          (kjb_uint16*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(kjb_uint16)), y, z)
#define DEBUG_UINT16_REALLOC(p, x, y, z) \
          (kjb_uint16*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(kjb_uint16)), y, z)

#define DEBUG_INT32_MALLOC(x, y, z)  \
          (kjb_int32*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(Malloc_size)), y, z)
#define DEBUG_INT32_REALLOC(p, x, y, z)  \
          (kjb_int32*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(Malloc_size)), y, z)
#define DEBUG_UINT32_MALLOC(x, y, z) \
          (kjb_uint32*)debug_kjb_malloc(((Malloc_size)(x))*(sizeof(kjb_uint32)), y, z)
#define DEBUG_UINT32_REALLOC(p, x, y, z) \
          (kjb_uint32*)debug_kjb_realloc(p, ((Malloc_size)(x))*(sizeof(kjb_uint32)), y, z)

#define START_HEAP_CHECK_2()  start_heap_check_2(__FILE__, __LINE__)
#define FINISH_HEAP_CHECK_2() finish_heap_check_2(__FILE__, __LINE__)

#define SKIP_HEAP_CHECK_2     skip_heap_check_2
#define CONTINUE_HEAP_CHECK_2 continue_heap_check_2
#define DISABLE_HEAP_CHECK_2  disable_heap_check_2
#define DISABLE_HEAP_CHECK_FOR_FORK  disable_heap_checking_for_fork

#else

#define START_HEAP_CHECK_2()
#define FINISH_HEAP_CHECK_2()

#define SKIP_HEAP_CHECK_2()
#define CONTINUE_HEAP_CHECK_2()
#define DISABLE_HEAP_CHECK_2()
#define DISABLE_HEAP_CHECK_FOR_FORK()

#endif    /*  TRACK_MEMORY_ALLOCATION  */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             KJB_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc
 *
 * This macro sets up the cast for kjb_malloc.  It is defined by:
 * |    #define KJB_MALLOC(n)       kjb_malloc((Malloc_size)(n))
 *
 *
 * Usually it is best to use a more specific version of wrapping MACRO. IE, one
 * of the ones listed in "see also". 
 * 
 * Related:
 *        TYPE_MALLOC(t), N_TYPE_MALLOC(t,n), VOID_MALLOC(x),
 *        UCHAR_MALLOC(x), BYTE_MALLOC(x), STR_MALLOC(x), SHORT_MALLOC(x),
 *        INT_MALLOC(x), LONG_MALLOC(x), FLT_MALLOC(x), DBL_MALLOC(x),
 *        INT16_MALLOC(x), UINT16_MALLOC(x), INT32_MALLOC(x), UINT32_MALLOC(x),
 *        DBL_MALLOC(x)
 *
 * Index: memory allocation
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     void* KJB_MALLOC(Malloc_size);
#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line.
                      I have no idea what the problem is. */
#    define KJB_MALLOC(n)       kjb_malloc((Malloc_size)(n))
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             KJB_CALLOC
 *
 * (MACRO) Sets up call for kjb_calloc
 *
 * This macro sets up the cast for kjb_calloc.  It is defined by:
 * |    #define KJB_CALLOC(n,s)     kjb_calloc((Malloc_size)(n), (Malloc_size)(s))
 *
 *
 * Usually it is best to use a more specific version of wrapping MACRO. IE, one
 * of the ones listed in "see also". 
 * 
 * Related:
 *        TYPE_CALLOC(t), N_TYPE_CALLOC(t,n), VOID_CALLOC(x),
 *        UCHAR_CALLOC(x), BYTE_CALLOC(x), STR_CALLOC(x), SHORT_CALLOC(x),
 *        INT_CALLOC(x), LONG_CALLOC(x), FLT_CALLOC(x), DBL_CALLOC(x),
 *        INT16_CALLOC(x), UINT16_CALLOC(x), INT32_CALLOC(x), UINT32_CALLOC(x),
 *        DBL_CALLOC(x)
 *
 * Index: memory allocation
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     void* KJB_CALLOC(Malloc_size, Malloc_size);
#else
#    define KJB_CALLOC(n,s)     kjb_calloc((Malloc_size)(n), (Malloc_size)(s))
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             KJB_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc
 *
 * This macro sets up the cast for kjb_realloc.  It is defined by:
 * |    #define KJB_REALLOC(p,n)     kjb_realloc(p, (Malloc_size)(n))
 *
 *
 * Usually it is best to use a more specific version of wrapping MACRO. IE, one
 * of the ones listed in "see also". 
 * 
 * Related:
 *        TYPE_REALLOC(t), N_TYPE_REALLOC(t,n), VOID_REALLOC(x),
 *        UCHAR_REALLOC(x), BYTE_REALLOC(x), STR_REALLOC(x), SHORT_REALLOC(x),
 *        INT_REALLOC(x), LONG_REALLOC(x), FLT_REALLOC(x), DBL_REALLOC(x),
 *        INT16_REALLOC(x), UINT16_REALLOC(x), INT32_REALLOC(x), UINT32_REALLOC(x),
 *        DBL_REALLOC(x)
 *
 * Index: memory allocation
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     void* KJB_REALLOC(void*, Malloc_size);
#else
#    define KJB_REALLOC(p,n)    kjb_realloc(p, (Malloc_size)(n))
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             TYPE_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc
 *
 * This macro takes a type as an argument, and sets up the call to kjb_malloc
 * to return storage for that type.  It is defined by:
 * |    #define TYPE_MALLOC(t)      (t *)kjb_malloc(sizeof(t))
 *
 * Index: memory allocation
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     type_T* TYPE_MALLOC(type_T t); 
#else
#    define TYPE_MALLOC(t)      (t *)kjb_malloc(sizeof(t))
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             N_TYPE_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc
 *
 * This macro takes a type and a count as an argument, and sets up the call to
 * kjb_malloc to return storage for count of that type. It is defined by:
 * | #define N_TYPE_MALLOC(t, n) (t *)kjb_malloc((sizeof(t))*((Malloc_size)(n)))
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     type_T* N_TYPE_MALLOC(type_T t, Malloc_size n); 
#else
#    define N_TYPE_MALLOC(t, n) (t *)kjb_malloc((sizeof(t))*((Malloc_size)(n)))
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             UCHAR_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for unsigned char
 *
 * This macro takes a count for the number of unsigned char needed and returns
 * storage for them. It is defined by:
 * |  #define UCHAR_MALLOC(n)     (unsigned char*)kjb_malloc((Malloc_size)(n))
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     unsigned char* UCHAR_MALLOC(Malloc_size n); 
#else
#    define UCHAR_MALLOC(n)     (unsigned char*)kjb_malloc((Malloc_size)(n))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BYTE_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for bytes 
 *
 * This macro takes a count for the number of bytes needed and returns storage
 * for them. It is defined by:
 * |  #define  BYTE_MALLOC(n) (unsigned char*)kjb_malloc((Malloc_size)(n))
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Note: 
 *     This macro is identical to UCHAR_MALLOC().
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     unsigned char* BYTE_MALLOC(Malloc_size n); 
#else
#    define BYTE_MALLOC(n)      (unsigned char*)kjb_malloc((Malloc_size)(n))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             STR_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for a string
 *
 * This macro takes a length for the number of characters needed and returns
 * storage for them. It is defined by:
 * |  #define  STR_MALLOC(n)  (char*)kjb_malloc((Malloc_size)(n))
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     char* STR_MALLOC(Malloc_size n); 
#else
#    define STR_MALLOC(n)       (char*)kjb_malloc((Malloc_size)(n))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BOOL_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for Bool type.
 *
 * This macro takes a count for the number of Bools needed and returns storage
 * for them. It is defined by:
 * |  #define BOOL_MALLOC(n) (Bool*)kjb_malloc(((Malloc_size)(n))*(sizeof(Bool))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     Bool* BOOL_MALLOC(Malloc_size n); 
#else
#    define BOOL_MALLOC(n) \
              (Bool*)kjb_malloc(((Malloc_size)(n))*(sizeof(Bool)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SHORT_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for short integers
 *
 * This macro takes a count for the number of short integers needed and returns storage
 * for them. It is defined by:
 * |  #define SHORT_MALLOC(n) (short*)kjb_malloc(((Malloc_size)(n))*(sizeof(short))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     short* SHORT_MALLOC(Malloc_size n); 
#else
#    define SHORT_MALLOC(n) \
              (short*)kjb_malloc(((Malloc_size)(n))*(sizeof(short)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             INT_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for integers
 *
 * This macro takes a count for the number of integers needed and returns storage
 * for them. It is defined by:
 * |  #define INT_MALLOC(n) (int*)kjb_malloc(((Malloc_size)(n))*(sizeof(int))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     int* INT_MALLOC(Malloc_size n); 
#else
#    define INT_MALLOC(n)    \
              (int*)kjb_malloc(((Malloc_size)(n))*(sizeof(int)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             UINT_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for unsigned integers
 *
 * This macro takes a count for the number of unsigned integers needed and
 * returns storage for them. It is defined by:
 * |  #define UINT_MALLOC(n) 
 * |      (unsigned int*)kjb_malloc(((Malloc_size)(n))*(sizeof(unsigned int))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     unsigned int* UINT_MALLOC(Malloc_size n); 
#else
#    define UINT_MALLOC(n)    \
              (unsigned int*)kjb_malloc(((Malloc_size)(n))*(sizeof(unsigned int)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             LONG_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for long integers
 *
 * This macro takes a count for the number of long integers needed and returns storage
 * for them. It is defined by:
 * |  #define LONG_MALLOC(n) (long*)kjb_malloc(((Malloc_size)(n))*(sizeof(long))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     long* LONG_MALLOC(Malloc_size n); 
#else
#    define LONG_MALLOC(n)   \
              (long*)kjb_malloc(((Malloc_size)(n))*(sizeof(long)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             FLT_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for floats
 *
 * This macro takes a count for the number of floats needed and returns storage
 * for them. It is defined by:
 * |  #define FLT_MALLOC(n) (float*)kjb_malloc(((Malloc_size)(n))*(sizeof(float))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     float* FLT_MALLOC(Malloc_size n); 
#else
#    define FLT_MALLOC(n)  \
              (float*)kjb_malloc(((Malloc_size)(n))*(sizeof(float)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             DBL_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for doubles
 *
 * This macro takes a count for the number of doubles needed and returns storage
 * for them. It is defined by:
 * |  #define DBL_MALLOC(n) 
 * |     (double*)kjb_malloc(((Malloc_size)(n))*(sizeof(double))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     double* DBL_MALLOC(Malloc_size n); 
#else
#    define DBL_MALLOC(n) \
              (double*)kjb_malloc(((Malloc_size)(n))*(sizeof(double)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             INT16_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for 16 bit integers
 *
 * This macro takes a count for the number of 16 bit integers needed and returns
 * storage for them. It is defined by:
 * |  #define INT16_MALLOC(n) 
 * |      (kjb_int16*)kjb_malloc(((Malloc_size)(n))*(sizeof(kjb_int16))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     kjb_int16* INT16_MALLOC(Malloc_size n); 
#else
#    define INT16_MALLOC(n)  \
              (kjb_int16*)kjb_malloc(((Malloc_size)(n))*(sizeof(kjb_int16)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             UINT16_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for 16 bit unsigned integers
 *
 * This macro takes a count for the number of 16 bit unsigned integers needed
 * and returns storage for them. It is defined by:
 * |  #define UINT16_MALLOC(n) 
 * |       (kjb_uint16*)kjb_malloc(((Malloc_size)(n))*(sizeof(kjb_uint16))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     kjb_uint16* UINT16_MALLOC(Malloc_size n); 
#else
#    define UINT16_MALLOC(n) \
              (kjb_uint16*)kjb_malloc(((Malloc_size)(n))*(sizeof(kjb_uint16)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             INT32_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for 32 bit integers
 *
 * This macro takes a count for the number of 32 bit integers needed and returns
 * storage for them. It is defined by:
 * |  #define INT32_MALLOC(n)  
 * |         (kjb_int32*)kjb_malloc(((Malloc_size)(n))*(sizeof(kjb_int32))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     kjb_int32* INT32_MALLOC(Malloc_size n); 
#else
#    define INT32_MALLOC(n)  \
              (kjb_int32*)kjb_malloc(((Malloc_size)(n))*(sizeof(kjb_int32)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             UINT32_MALLOC
 *
 * (MACRO) Sets up call for kjb_malloc for 32 bit unsigned integers
 *
 * This macro takes a count for the number of 32 bit unsigned integers needed
 * and returns storage for them. It is defined by:
 * |  #define UINT32_MALLOC(n) 
 * |         (kjb_uint32*)kjb_malloc(((Malloc_size)(n))*(sizeof(kjb_uint32))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     kjb_uint32* UINT32_MALLOC(Malloc_size n); 
#else
#    define UINT32_MALLOC(n) \
              (kjb_uint32*)kjb_malloc(((Malloc_size)(n))*(sizeof(kjb_uint32)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             N_TYPE_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc
 *
 * This macro takes a type and a count as an argument, and sets up the call to
 * kjb_realloc to return storage for count of that type. It is defined by:
 * | #define N_TYPE_REALLOC(p, t, n) (t *)kjb_realloc(p, (sizeof(t))*((Malloc_size)(n)))
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     type_T* N_TYPE_REALLOC(void* p, type_T t, Malloc_size n); 
#else
#    define N_TYPE_REALLOC(p, t, n) (t *)kjb_realloc(p, (sizeof(t))*((Malloc_size)(n)))
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             UCHAR_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for unsigned char
 *
 * This macro takes a count for the number of unsigned char needed and returns
 * storage for them. It is defined by:
 * |  #define UCHAR_REALLOC(p, n)     (unsigned char*)kjb_realloc(p, (Malloc_size)(n))
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     unsigned char* UCHAR_REALLOC(void* p, Malloc_size n); 
#else
#    define UCHAR_REALLOC(p, n)     (unsigned char*)kjb_realloc(p, (Malloc_size)(n))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BYTE_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for bytes 
 *
 * This macro takes a count for the number of bytes needed and returns storage
 * for them. It is defined by:
 * |  #define  BYTE_REALLOC(p, n) (unsigned char*)kjb_realloc(p, (Malloc_size)(n))
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Note: 
 *     This macro is identical to UCHAR_REALLOC(p, ).
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     unsigned char* BYTE_REALLOC(void* p, Malloc_size n); 
#else
#    define BYTE_REALLOC(p, n)      (unsigned char*)kjb_realloc(p, (Malloc_size)(n))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             STR_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for a string
 *
 * This macro takes a length for the number of characters needed and returns
 * storage for them. It is defined by:
 * |  #define  STR_REALLOC(p, n)  (short*)kjb_realloc(p, (Malloc_size)(n))
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     unsigned char* STR_REALLOC(void* p, Malloc_size n); 
#else
#    define STR_REALLOC(p, n)       (char*)kjb_realloc(p, (Malloc_size)(n))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             SHORT_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for short integers
 *
 * This macro takes a count for the number of short integers needed and returns storage
 * for them. It is defined by:
 * |  #define SHORT_REALLOC(p, n) (short*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(short))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     short* SHORT_REALLOC(void* p, Malloc_size n); 
#else
#    define SHORT_REALLOC(p, n) \
              (short*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(short)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             INT_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for integers
 *
 * This macro takes a count for the number of integers needed and returns storage
 * for them. It is defined by:
 * |  #define INT_REALLOC(p, n) (int*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(int))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     short* INT_REALLOC(void* p, Malloc_size n); 
#else
#    define INT_REALLOC(p, n)    \
              (int*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(int)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             UINT_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for unsigned integers
 *
 * This macro takes a count for the number of unsigned integers needed and
 * returns storage for them. It is defined by:
 * |  #define UINT_REALLOC(p, n) 
 * |      (unsigned * int*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(unsigned int))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     short* UINT_REALLOC(void* p, Malloc_size n); 
#else
#    define UINT_REALLOC(p, n)    \
              (unsigned int*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(unsigned int)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             LONG_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for long integers
 *
 * This macro takes a count for the number of long integers needed and returns storage
 * for them. It is defined by:
 * |  #define LONG_REALLOC(p, n) (long*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(long))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     short* LONG_REALLOC(void* p, Malloc_size n); 
#else
#    define LONG_REALLOC(p, n)   \
              (long*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(long)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             FLT_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for floats
 *
 * This macro takes a count for the number of floats needed and returns storage
 * for them. It is defined by:
 * |  #define FLT_REALLOC(p, n) (float*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(float))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     float* FLT_REALLOC(void* p, Malloc_size n); 
#else
#    define FLT_REALLOC(p, n)  \
              (float*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(float)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             DBL_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for doubles
 *
 * This macro takes a count for the number of doubles needed and returns storage
 * for them. It is defined by:
 * |  #define DBL_REALLOC(p, n) 
 * |     (double*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(double))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     short* DBL_REALLOC(void* p, Malloc_size n); 
#else
#    define DBL_REALLOC(p, n) \
              (double*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(double)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             INT16_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for 16 bit integers
 *
 * This macro takes a count for the number of 16 bit integers needed and returns
 * storage for them. It is defined by:
 * |  #define INT16_REALLOC(p, n) 
 * |      (kjb_int16*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(kjb_int16))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     kjb_int16* INT16_REALLOC(void* p, Malloc_size n); 
#else
#    define INT16_REALLOC(p, n)  \
              (kjb_int16*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(kjb_int16)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             UINT16_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for 16 bit unsigned integers
 *
 * This macro takes a count for the number of 16 bit unsigned integers needed
 * and returns storage for them. It is defined by:
 * |  #define UINT16_REALLOC(p, n) 
 * |       (kjb_uint16*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(kjb_uint16))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     kjb_uint16* UINT16_REALLOC(void* p, Malloc_size n); 
#else
#    define UINT16_REALLOC(p, n) \
              (kjb_uint16*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(kjb_uint16)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             INT32_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for 32 bit integers
 *
 * This macro takes a count for the number of 32 bit integers needed and returns
 * storage for them. It is defined by:
 * |  #define INT32_REALLOC(p, n)  
 * |         (kjb_int32*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(kjb_int32))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     kjb_int32* INT32_REALLOC(void* p, Malloc_size n); 
#else
#    define INT32_REALLOC(p, n)  \
              (kjb_int32*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(kjb_int32)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             UINT32_REALLOC
 *
 * (MACRO) Sets up call for kjb_realloc for 32 bit unsigned integers
 *
 * This macro takes a count for the number of 32 bit unsigned integers needed
 * and returns storage for them. It is defined by:
 * |  #define UINT32_REALLOC(p, n) 
 * |         (kjb_uint32*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(kjb_uint32))) 
 *
 * Note:
 *     n is cast to Malloc_size is almost always size_t. 
 *
 * Index: memory allocation, macros
 *
 * -----------------------------------------------------------------------------
*/

#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
     kjb_uint32* UINT32_REALLOC(void* p, Malloc_size n); 
#else
#    define UINT32_REALLOC(p, n) \
              (kjb_uint32*)kjb_realloc(p, ((Malloc_size)(n))*(sizeof(kjb_uint32)))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_heap_options(const char* option, const char* value);

int memory_used(unsigned long* total_bytes_used_ptr);

#ifdef TRACK_MEMORY_ALLOCATION
    void* debug_kjb_malloc(Malloc_size, const char*, int);

    void* debug_kjb_calloc
    (
        Malloc_size ,
        Malloc_size ,
        const char* ,
        int
    );

    void* debug_kjb_realloc(void* ptr, Malloc_size, const char*, int);

#else
    void* kjb_calloc(Malloc_size, Malloc_size);
    void* kjb_malloc(Malloc_size);
    void* kjb_realloc(void*, Malloc_size);
#endif

void float_align(char**);
void null_and_float_align(char**);

#ifdef TRACK_MEMORY_ALLOCATION_OR_C2MAN
    void watch_for_free(void* ptr);
    void check_initialization
    (
        const void* ptr,
        size_t      count,
        size_t      block_size
    );

void check_for_heap_overruns(const char* file_name, int line_number);

#else
#   define watch_for_free(x)
#   define check_initialization(x, y, z)
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  CHECK_FOR_HEAP_OVERRUNS
 *
 * (Debugging MACRO) Checks that memory was not overrun
 *
 * In development code, this routine checks for heap memory which has been
 * overrun. In debug mode the kjb library will identify overruns when freeing,
 * but often this is too late to pinpoint the problem. In this case this macro
 * can be interspersed in suspect code to localize where the overrun is
 * occurring.
 *
 * Index: memory allocation, debugging, macros
 *
 * -----------------------------------------------------------------------------
 */

#ifdef __C2MAN__
    void CHECK_FOR_HEAP_OVERRUNS(void);
#else
#ifdef TRACK_MEMORY_ALLOCATION
#   define CHECK_FOR_HEAP_OVERRUNS() \
                check_for_heap_overruns(__FILE__, __LINE__)
    void check_for_heap_overruns(const char* file_name, int line_number);
#else
#   define CHECK_FOR_HEAP_OVERRUNS()
#endif
#endif


void optimize_free(void);
void kjb_free(void*);
int print_allocated_memory(FILE*);
int get_allocated_memory(unsigned long* total_bytes_ptr);

#ifdef TEST
#define print_heap_memory_used()  \
        debug_print_heap_memory_used(__FILE__, __LINE__)

int debug_print_heap_memory_used(char* file, int line);
#else
#define print_heap_memory_used()
#endif

int optimize_free_2(void);

#ifdef TRACK_MEMORY_ALLOCATION

void  start_heap_check_2
(
    const char* file_name,
    int         line_number
);

void  finish_heap_check_2
(
    const char* file_name,
    int         line_number
);
void skip_heap_check_2             (void);
void continue_heap_check_2         (void);
void disable_heap_check_2          (void);

#endif 

#ifdef TRACK_MEMORY_ALLOCATION
void disable_heap_checking(void);
void disable_heap_checking_for_thread_use(void);
#else
#    define disable_heap_checking(x)
#    define disable_heap_checking_for_thread_use(x)
#endif 

#ifdef OBSOLETE
#ifdef TRACK_MEMORY_ALLOCATION
    void reset_heap_checking_for_fork(void);
#else
#    define reset_heap_checking_for_fork(x)
#endif 
#endif 


#ifdef TRACK_MEMORY_ALLOCATION
    void kjb_check_free(const void* ptr);
#else
#    define kjb_check_free(x)
#endif


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

