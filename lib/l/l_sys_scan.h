
/* $Id: l_sys_scan.h 21341 2017-03-26 03:45:12Z kobus $ */

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

#ifndef L_S_SCAN_INCLUDED
#define L_S_SCAN_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* If we do not have 64 bit integers, then these won't get defined, and we
 * should not use them! The macro HAVE_64_BIT_INT can be used to protect. 
*/

#ifdef INT64_IS_LONG
#    define ss1u64     ss1ul
#    define ss1i64     ss1l
#    define ss1pi64    ss1pl
#    define ss1spi64   ss1spl
#    define ss1pi64_2  ss1pl_2
#else
#ifdef INT64_IS_INT
#    define ss1u64     ss1ui
#    define ss1i64     ss1i
#    define ss1pi64    ss1pi
#    define ss1spi64   ss1spi
#    define ss1pi64_2  ss1pi_2
#endif
#endif


#ifdef INT32_IS_LONG
#    define ss1u32     ss1ul
#    define ss1i32     ss1l
#    define ss1pi32    ss1pl
#    define ss1spi32   ss1spl
#    define ss1pi32_2  ss1pl_2
#else
#ifdef INT32_IS_INT
#    define ss1u32     ss1ui
#    define ss1i32     ss1i
#    define ss1pi32    ss1pi
#    define ss1spi32   ss1spi
#    define ss1pi32_2  ss1pi_2
#endif
#endif


#ifdef INT16_IS_INT
#    define ss1u16     ss1us
#    define ss1i16     ss1i
#    define ss1pi16    ss1pi
#    define ss1spi16   ss1spi
#    define ss1pi16_2  ss1pi_2
#else
#ifdef INT16_IS_SHORT
#    define ss1u16     ss1us
#    define ss1i16     ss1s
#    define ss1pi16    ss1ps
#    define ss1spi16   ss1sps
#    define ss1pi16_2  ss1ps_2
#endif
#endif


/* ------------------------------------------------------------------------- */

int set_low_level_scan_options(const char* option, const char* value);

int ss1ul(const char* input_str, unsigned long* long_ptr);
int ss1l   (const char* input_str, long* long_ptr);
int ss1spl (const char* input_str, long* long_ptr);
int ss1pl  (const char* input_str, long* long_ptr);
int ss1pl_2(const char* input_str, long* long_ptr);

int ss1ui  (const char* input_str, unsigned int* uint_ptr);
int ss1i   (const char* input_str, int* int_ptr);
int ss1spi (const char* input_str, int* int_ptr);
int ss1pi  (const char* input_str, int* int_ptr);
int ss1pi_2(const char* input_str, int* int_ptr);

int ss1us  (const char* input_str, unsigned short* ushort_ptr);
int ss1s   (const char* input_str, short* short_ptr);
int ss1sps (const char* input_str, short* short_ptr);
int ss1ps  (const char* input_str, short* short_ptr);
int ss1ps_2(const char* input_str, short* short_ptr);

int ss1f  (const char* input_str, float* float_ptr);
int ss1snf(const char* input_str, float* float_ptr);

int ss1d  (const char* input_str, double* double_ptr);
int ss1snd(const char* input_str, double* double_ptr);

int is_scientific_notation_number(const char*);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


