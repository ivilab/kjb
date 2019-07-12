/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

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
| Authors:
|     Joseph Schlecht, Luca Del Pero
|
* =========================================================================== */

/**
 * @file
 *
 * @author Joseph Schlecht, Luca Del Pero
 *
 * @brief Manipulate data types on the bit (or byte) level.
 */


#include "l/l_sys_debug.h"
#include "l/l_sys_lib.h"
#include "l/l_sys_io.h"
#include "l/l_global.h"
#include "l/l_bits.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @name bswap
 *
 * Swaps the byte ordering between big and little endian.
 * @{
 */


/** Endian test looks at the first byte of ((int)1).
 *  We would make global variable kjb_endian_test a const int if we could, but
 *  doing so confuses g++.  So, as a consolation, we use ASSERT.
 */
int kjb_is_bigendian()
{
    IMPORT int kjb_endian_test;
    ASSERT(1 == kjb_endian_test);
    return 0 == (*(char*)&kjb_endian_test);
}

/**
 * Byte swaps a big-endian value to little-endian and vice versa.
 * The parameter is passed by reference.
 *
 * @param  x  Value to swap the bytes of.
 */
void bswap_u16(kjb_uint16* x)
{
    *x = (*x>>8) |
         (*x<<8);
}


/**
 * Byte swaps a big-endian value to little-endian and vice versa.
 * The parameter is passed by reference.
 *
 * @param  x  Value to swap the bytes of.
 */
void bswap_u32(kjb_uint32* x)
{
    *x = (*x>>24) |
         ((*x<<8) & 0x00FF0000) |
         ((*x>>8) & 0x0000FF00) |
         (*x<<24);
}


/**
 * Byte swaps a big-endian value to little-endian and vice versa.
 * The parameter is passed by reference.
 *
 * @param  x  Value to swap the bytes of.
 */
void bswap_u64(kjb_uint64* x)
{
    kjb_uint64 a = 0xFFu;
    ASSERT(8 == sizeof(kjb_uint64));

    *x = (*x>>56) |
         ((*x<<40) & (a<<48)) |
         ((*x<<24) & (a<<40)) |
         ((*x<<8)  & (a<<32)) |
         ((*x>>8)  & (a<<24)) |
         ((*x>>24) & (a<<16)) |
         ((*x>>40) & (a<<8))  |
         (*x<<56);
}

/*@}*/

#ifdef __cplusplus
}
#endif

