/* $Id: l_debug.h 14544 2013-05-29 20:34:46Z predoehl $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef KJB_CPP_L_DEBUG_H
#define KJB_CPP_L_DEBUG_H

#include <string> 

namespace kjb
{

namespace debug
{

/**
 * Convert a float to a string of '0' and '1's representing
 * its binary representation.
 *
 * @note This not optimized for production use and is intended for debugging purposes only.  
 *
 *
 * TODO: Make this a generic template function.
 */
template <class T>
std::string to_bitstring(T v)
{
#ifdef PRODUCTION
#if PRODUCTION == 1
    std::cerr << "WARNING: executing debugging code in production mode: to_bitstring().  Client code should disable this for production mode.";
#endif
#endif
    const int LENGTH = (sizeof(T)) * 9 + 1;
    char out[LENGTH];

    unsigned char* bytes = (unsigned char*) &v;

    // by = byte
    // bi = bit
    //
    int i = 0;
    for(int by = sizeof(T) - 1; by >= 0; by--)
    {
        for(int bi = 0; bi < 8; bi++)
        {
            out[i] = (bytes[by] & (1 << bi) ? '1' : '0');
            i++;
        }

        out[i] = ' ';
        i++;

    }

    out[LENGTH - 1] = '\0';

    return std::string(out);
}

} // namespace debug
} // namespace kjb

#endif
