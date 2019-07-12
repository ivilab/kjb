
/* $Id: l_def.h 21755 2017-09-07 21:54:51Z kobus $ */

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

#ifndef L_DEF_INCLUDED
#define L_DEF_INCLUDED

#include "l/l_sys_def.h"

#ifdef COMPILING_CPLUSPLUS_SOURCE
#    define USING_KJB_C() using namespace kjb_c;
#else 
#    define USING_KJB_C()   
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifdef __C2MAN__
    typedef int typedef_struct;
    typedef int typedef_union;
    typedef int type_T; 

#   ifdef __STDC_VERSION__
#       undef __STDC_VERSION__
#   endif 
#   define __STDC_VERSION__ 198901
#endif


#ifdef __cplusplus
#    define EXTERN_C  extern "C" 
#else 
#    define EXTERN_C 
#endif 


/*
#ifndef TRUE
#    define TRUE   (1)
#endif

#ifndef FALSE
#    define FALSE  (0)
#endif
*/

#ifdef FALSE
#    undef FALSE
#endif 

#ifdef TRUE
#    undef TRUE
#endif 

#ifdef __cplusplus
    typedef bool Bool; 
#   define TRUE true 
#   define FALSE false 
#else  
#    ifdef USING_C99
#        define HAVE_BUILTIN_BOOL
#    endif 

#    ifdef HAVE_BUILTIN_BOOL
         typedef _Bool Bool; 
#        define TRUE   (1)
#        define FALSE  (0)
#    else
        typedef enum Bool 
        {
            FALSE = 0,
            TRUE  = 1
        }
        Bool;
#    endif 
#endif 
       

/*
 * Exit status for when a bug is encountered. For example, if code that is
 * executed cannot happen. Portable exit status values probably should be
 * between 0 and 255. This one should be different from all other commonly used exit
 * status's. Hence it should not be a small integer, nor should it be the return
 * convention from signals (128+SIGNUM) or timeout (124). So, something larger
 * than 196 seems to make sense. But, it definately should be 255 which is the
 * exit status on many systems if some uses '-1' as the exit value. So, for now,
 * lets go with 199. 
*/
#ifndef EXIT_BUG
#    define EXIT_BUG   (199) 
#endif 

#ifndef EXIT_CANNOT_TEST
#    define EXIT_CANNOT_TEST   (198) 
#endif 

/*
//  CHECK
//
//  Windows (at least under BC45) defines ERROR and NO_ERROR.
//  However, until we use them in conjunction with windows code, there
//  is no need to use windows' definition of them. Furthermore, it may
//  be preferable to use WINDOWS_ERROR and WINDOWS_NO_ERROR in this case.
//  Alternately, we could change ERROR to KJB_ERROR, and NO_ERROR to
//  KJB_NO_ERROR.
//
//  Since this is a little dangerous, try without, and deal with it when
//  necessary.
//
//  #ifdef ERROR
//  #    undef ERROR
//  #endif
//
//  #ifdef NO_ERROR
//  #    undef NO_ERROR
//  #endif
*/


/*
// Make ALL of these enum specific numbers, to make sure that they are
// positve or negative as the case may be. 
//
// IMPORTANT: Additions here should be reflected in get_status_string().
*/
typedef enum Return_status
{
    NO_ERROR        = 0,
    FOUND           = 1,
    SHADOW_EOF      = EOF,    /* Force it so none of these can be EOF */
    TRUNCATED       = -2,
    INTERRUPTED     = -3,
    WOULD_BLOCK     = -4,
    BACKGROUND_READ = -5,
    NOT_FOUND       = -9,
    NOT_AVAILABLE   = -19,
    NO_SOLUTION     = -29,
    PROCESS_IS_DEAD = -39,
    NOT_SET         = -99,
    NEVER_SET       = -109,
    ERROR           = -999
}
Return_status;


/* -------------------------------------------------------------------------- */

#define KJB_IS_SET(x)   ((x) != NOT_SET)
/*
#define IF_SET(x)   if ((x) != NOT_SET)
*/

/*
#define FOR(x, y)     for (x=0; x<y; x++)
*/

#define SET_TO_ZERO(x)          memset((void*)&(x), 0, sizeof(x ))

#define MILLI_SECONDS_PER_SECOND    1000
#define MICRO_SECONDS_PER_SECOND    1000000

#define ROOM_FOR_NULL    1
#define ROOM_FOR_BLANK   1

#define INT_DUMMY_ARG              (-12345)
#define DBL_DUMMY_ARG              (-12345.0)
#define INT_DUMMY_RETURN           (-6789)
#define REAL_DUMMY_RETURN          (-6789.0)

/* The following limit allows array sizes of 10**30 (enough!) */
#define MAX_BIN_SEARCH_LOOP_COUNT 100

/* Move ??? */
#define IS_CHILD(x)               ((x)==0)
#define IS_PARENT(x)              ((x)!=0)

/* Move ??? */
#define READ_END                  0
#define WRITE_END                 1

#define NO_SYSTEM_ERROR          0

#define ABS_OF(A)      (((A) >= 0) ? (A) : (-(A)))
#define MAX_OF(A, B)   ((A) > (B) ? (A) : (B ))
#define MIN_OF(A, B)   ((A) < (B) ? (A) : (B ))
#define CLAMP(X, MIN, MAX) ((X) < (MIN) ? (MIN) : ((X) > (MAX) ? (MAX) : (X)))
#define INT_SIGN_OF(A) ( (A) < 0 ? -1 : ( (A) > 0 ? 1 : 0 ) )
#define SIGN_OF(A) ( (A) < 0.0 ? -1.0 : ( (A) > 0.0 ? 1.0 : 0.0 ) )
/*
#define ROUND_DBL_TO_INT(x)  ((x>0.0) ? ((int)(x + 0.5)) : ((int)(x - 0.5)))
#define ROUND_FLT_TO_INT(x)  ((x>0.0f) ? ((int)(x + 0.5f)) : ((int)(x - 0.5f)))
*/


#ifndef IS_EVEN
#    define IS_EVEN(x)   ((x%2) == 0)
#endif 

#ifndef IS_ODD
#    define IS_ODD(x)    ((x%2) != 0)
#endif 

#define FLT_ALIGNED(x)   (x%4==0)
#define DBL_ALIGNED(x)   (x%8==0)

/* Move ? */
#define HIGH_LIGHT_CHAR                 '+'
#define DONT_FORMAT_CHAR                '|'


#ifdef LONG_DOUBLE_OK
    typedef long double long_double;
#else
    /*
    //
    // Breaks on HPUX where there is a (lousy) long_double.
    //
    // typedef double long_double;
    //
    */
    #define long_double double
#endif


/*
// The use of these should likely be minimized. However, then are ocasionly
// useful.
*/
#ifdef SINGLE_PRECISION_CONSTANT_OK
#    define FLT_ZERO         0.0F
#    define FLT_HALF         0.5F
#    define FLT_ONE          1.0F
#    define FLT_TWO          2.0F
#    define FLT_THREE        3.0F
#    define FLT_TEN         10.0F
#    define FLT_255        255.0F
#    define FLT_NOT_SET   ((float)NOT_SET)
#else
#    define FLT_ZERO          ((float)0.0)
#    define FLT_HALF          ((float)0.5)
#    define FLT_ONE           ((float)1.0)
#    define FLT_TWO           ((float)2.0)
#    define FLT_THREE         ((float)3.0)
#    define FLT_TEN          ((float)10.0)
#    define FLT_255         ((float)255.0)
#    define FLT_NOT_SET    ((float)NOT_SET)
#endif


#define DBL_NOT_SET   ((double)NOT_SET)
#define EXTRA_FOR_ROUNDING (0.5)

#ifndef FLT_DIG
#    define     FLT_DIG         6
#endif

#ifndef DBL_DIG
#    define     DBL_DIG         15
#endif

#ifndef FLT_EPSILON
    /* Danger: We don't really want to set these ourselves. */
    /* This constant is used to provide a warning message that we did so. */
#    ifndef MACHINE_CONSTANT_NOT_AVAILABLE
#        define MACHINE_CONSTANT_NOT_AVAILABLE
#    endif

#    ifdef SINGLE_PRECISION_CONSTANT_OK
#        define FLT_EPSILON        1.192092896E-07F
#    else
#        define FLT_EPSILON        1.192092896E-07
#    endif
#endif

#ifndef FLT_MIN
    /* Danger: We don't really want to set these ourselves. */
    /* This constant is used to provide a warning message that we did so. */
#    ifndef MACHINE_CONSTANT_NOT_AVAILABLE
#        define MACHINE_CONSTANT_NOT_AVAILABLE
#    endif

#    ifdef SINGLE_PRECISION_CONSTANT_OK
#        define FLT_MIN            1.175494351E-38F
#    else
#        define FLT_MIN            1.175494351E-38
#    endif
#endif

#ifndef FLT_MAX
    /* Danger: We don't really want to set these ourselves. */
    /* This constant is used to provide a warning message that we did so. */
#    ifndef MACHINE_CONSTANT_NOT_AVAILABLE
#        define MACHINE_CONSTANT_NOT_AVAILABLE
#    endif

#    ifdef SINGLE_PRECISION_CONSTANT_OK
#        define FLT_MAX            3.402823466E+38F
#    else
#        define FLT_MAX            3.402823466E+38
#    endif
#endif

#define FLT_MOST_POSITIVE     FLT_MAX
#define FLT_MOST_NEGATIVE   (-FLT_MAX)

#define FLT_HALF_MOST_POSITIVE    (FLT_MOST_POSITIVE / FLT_TWO)
#define FLT_HALF_MOST_NEGATIVE    (FLT_MOST_NEGATIVE / FLT_TWO)

#ifndef DBL_EPSILON
    /* Danger: We don't really want to set these ourselves. */
    /* This constant is used to provide a warning message that we did so. */
#    ifndef MACHINE_CONSTANT_NOT_AVAILABLE
#        define MACHINE_CONSTANT_NOT_AVAILABLE
#    endif

#    define DBL_EPSILON        2.2204460492503131E-16
#endif

#ifndef DBL_MIN
    /* Danger: We don't really want to set these ourselves. */
    /* This constant is used to provide a warning message that we did so. */
#    ifndef MACHINE_CONSTANT_NOT_AVAILABLE
#        define MACHINE_CONSTANT_NOT_AVAILABLE
#    endif

#    define DBL_MIN            2.2250738585072014E-308
#endif

/* Old name for DBL_MIN? */
#ifndef MINDOUBLE
#    define MINDOUBLE DBL_MIN 
#endif 

#ifndef DBL_MAX
    /* Danger: We don't really want to set these ourselves. */
    /* This constant is used to provide a warning message that we did so. */
#    ifndef MACHINE_CONSTANT_NOT_AVAILABLE
#        define MACHINE_CONSTANT_NOT_AVAILABLE
#    endif

#    define DBL_MAX            1.7976931348623157E+308
#endif

/* Old name for DBL_MAX? */
#ifndef MAXDOUBLE
#    define MAXDOUBLE DBL_MAX 
#endif 

#define DBL_MOST_POSITIVE     DBL_MAX
#define DBL_MOST_NEGATIVE  (-DBL_MAX)

#define DBL_HALF_MOST_POSITIVE    (DBL_MOST_POSITIVE / 2.0)
#define DBL_HALF_MOST_NEGATIVE    (DBL_MOST_NEGATIVE / 2.0)

#define MIN_LOG_ARG 4.47628e-309

#define IS_FINITE_FLT(x)  \
          (((x) < FLT_HALF_MOST_POSITIVE) && ((x) > FLT_HALF_MOST_NEGATIVE))

#define IS_INFINITE_FLT(x)   ( ! (IS_FINITE_FLT(x)) )


#define IS_INTEGRAL_FLT(x) \
                (    (IS_ZERO_FLT(x)) \
                  || (ABS_OF((x - (float)((int)x))/x) < FLT_EPSILON) \
                )

#define IS_ZERO_FLT(x) \
          (((x) <= (FLT_THREE * FLT_MIN)) && (((x) >= -(FLT_THREE * FLT_MIN))))

#define IS_EQUAL_FLT(x, y)  \
        IS_NEARLY_EQUAL_FLT(x, y, 2.0 * FLT_EPSILON)

#define IS_NEARLY_EQUAL_FLT(x, y, e)    \
           ( ((x) == (y)) ? TRUE : \
             (  ((x) < (y)) ? (x >= SUB_RELATIVE_FLT(y, e)) :  \
                              (y >= SUB_RELATIVE_FLT(x, e)) ) )


#define IS_POSITIVE_FLT(x)     ((x) > (FLT_THREE * FLT_MIN))
#define IS_NEGATIVE_FLT(x)     ((x) < -(FLT_THREE * FLT_MIN))

#define IS_NOT_NEGATIVE_FLT(x)   ( ! (IS_NEGATIVE_FLT(x)) )
#define IS_NOT_POSITIVE_FLT(x)   ( ! (IS_POSITIVE_FLT(x)) )


#define IS_GREATER_FLT(x,y)      ((x) > ADD_FLT_EPSILON(y))

#define IS_LESSER_FLT(x,y)       ((x) < SUB_FLT_EPSILON(y))

#define IS_NOT_GREATER_FLT(x,y)  ((x) <= ADD_FLT_EPSILON(y))

#define IS_NOT_LESSER_FLT(x,y)   ((x) >= SUB_FLT_EPSILON(y))


#define ADD_RELATIVE_FLT(x, y)   ( ((x)>FLT_ZERO) ?                            \
                                         ((x)*(FLT_ONE + y))                   \
                                      :  ((x)*(FLT_ONE - y))                   \
                                   )

#define SUB_RELATIVE_FLT(x, y)  ( ((x)>FLT_ZERO) ?                             \
                                         ((x)*(FLT_ONE - y))                   \
                                      :  ((x)*(FLT_ONE + y))                   \
                                   )

#define ADD_FLT_EPSILON(x)    ADD_RELATIVE_FLT(x, FLT_EPSILON)
#define SUB_FLT_EPSILON(x)    SUB_RELATIVE_FLT(x, FLT_EPSILON)


#define IS_FINITE_DBL(x)  \
         (((x) < DBL_HALF_MOST_POSITIVE) && ((x) > DBL_HALF_MOST_NEGATIVE))

#define IS_INFINITE_DBL(x)   ( ! (IS_FINITE_DBL(x)) )

#define IS_INTEGRAL_DBL(x) \
                (    (IS_ZERO_DBL(x)) \
                  || (ABS_OF((x - (float)((int)x))/x) < DBL_EPSILON) \
                )

#define IS_ZERO_DBL(x) \
           (((x) <= (3.0 * DBL_MIN)) && (((x) >= -(3.0 * DBL_MIN))))

#define IS_EQUAL_DBL(x, y)  \
        IS_NEARLY_EQUAL_DBL(x, y, 2.0 * DBL_EPSILON)


#define IS_NEARLY_EQUAL_DBL(x, y, e)    \
           ( ((x) == (y)) ? TRUE : \
             (  ((x) < (y)) ? (x >= SUB_RELATIVE_DBL(y, e)) :  \
                              (y >= SUB_RELATIVE_DBL(x, e)) ) )


#define IS_POSITIVE_DBL(x)     ((x) > DBL_EPSILON)
#define IS_NEGATIVE_DBL(x)     ((x) < -DBL_EPSILON)

#define IS_NOT_NEGATIVE_DBL(x)   ( ! (IS_NEGATIVE_DBL(x)) )
#define IS_NOT_POSITIVE_DBL(x)   ( ! (IS_POSITIVE_DBL(x)) )


#define IS_GREATER_DBL(x,y)      ((x) > ADD_DBL_EPSILON(y))

#define IS_LESSER_DBL(x,y)       ((x) < SUB_DBL_EPSILON(y))

#define IS_NOT_GREATER_DBL(x,y)  ((x) <= ADD_DBL_EPSILON(y))

#define IS_NOT_LESSER_DBL(x,y)   ((x) >= SUB_DBL_EPSILON(y))


#define ADD_RELATIVE_DBL(x, y)   ( ((x)>0.0) ?                                 \
                                         ((x)*(1.0 + y))                       \
                                      :  ((x)*(1.0 - y))                       \
                                   )

#define SUB_RELATIVE_DBL(x, y)  ( ((x)>0.0) ?                                  \
                                         ((x)*(1.0 - y))                       \
                                      :  ((x)*(1.0 + y))                       \
                                   )

#define ADD_DBL_EPSILON(x)     ADD_RELATIVE_DBL(x, DBL_EPSILON)
#define SUB_DBL_EPSILON(x)     SUB_RELATIVE_DBL(x, DBL_EPSILON)

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


