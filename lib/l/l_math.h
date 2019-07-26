
/* $Id: l_math.h 21593 2017-07-30 16:48:05Z kobus $ */

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

#ifndef L_MATH_INCLUDED
#define L_MATH_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*
 * Sometimes math symbols are missing. Make sure we get them all.
*/

#ifndef M_E
#    define M_E         2.7182818284590452354   /* e */
#endif

#ifndef M_LOG2E
#    define M_LOG2E     1.4426950408889634074   /* log_2 e */
#endif

#ifndef M_LOG10E
#    define M_LOG10E    0.43429448190325182765  /* log_10 e */
#endif

#ifndef M_LN2
#    define M_LN2       0.69314718055994530942  /* log_e 2 */
#endif

#ifndef M_LN10
#    define M_LN10      2.30258509299404568402  /* log_e 10 */
#endif

#ifndef M_PI
#    define M_PI        3.14159265358979323846  /* pi */
#endif

#ifndef M_PI_2
#    define M_PI_2      1.57079632679489661923  /* pi/2 */
#endif

#ifndef M_PI_4
#    define M_PI_4      0.78539816339744830962  /* pi/4 */
#endif

#ifndef M_1_PI
#    define M_1_PI      0.31830988618379067154  /* 1/pi */
#endif

#ifndef M_2_PI
#    define M_2_PI      0.63661977236758134308  /* 2/pi */
#endif

#ifndef M_2_TIMES_PI
#    define M_2_TIMES_PI     6.28318530717958f  /* 2*pi */
#endif


#ifndef M_2_SQRTPI
#    define M_2_SQRTPI  1.12837916709551257390  /* 2/sqrt(pi) */
#endif

#ifndef M_SQRT2
#    define M_SQRT2     1.41421356237309504880  /* sqrt(2) */
#endif

#ifndef M_SQRT1_2
#    define M_SQRT1_2   0.70710678118654752440  /* 1/sqrt(2) */
#endif


#define LOG_ZERO    ( -710.0 )

#define SAFE_LOG(x)   ((x <= 2.0 * DBL_MIN) ? LOG_ZERO : log(x))


int ipower(int, int);
int int_plus(int, int);
int ptr_int_compare(int*, int*, int);
int max3(int, int, int);
int min3(int, int, int);

#ifdef STANDARD_VAR_ARGS
int pos_min(int, ...);
#else
int pos_min(void);
#endif


int get_random_integer_list
(
    int  count,
    int  min_value,
    int  max_value,
    int* output_array
);

double kjb_log2(double x);

int kjb_floor(double x);
int kjb_rint (double x);
int kjb_rintf(float x);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

