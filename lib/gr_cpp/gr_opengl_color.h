/* $Id: gr_opengl_color.h 21599 2017-07-31 00:44:30Z kobus $ */
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
//
#ifndef KJB_CPP_OPENGL_COLOR_H
#define KJB_CPP_OPENGL_COLOR_H

#include "m_cpp/m_vector_d.h"

namespace kjb
{
namespace opengl
{

extern const Vector3 WHITE;
extern const Vector3 BLACK;

extern const Vector3 RED;
extern const Vector3 GREEN;
extern const Vector3 BLUE;

extern const Vector3 YELLOW;
extern const Vector3 CYAN;
extern const Vector3 MAGENTA;

/**
 * pack the lowest 24-bits of i into a 3-element byte array.
 *
 * @param rgb pointer to a 3-element byte array to store result
 */
inline void uint_to_rgb(unsigned int i, unsigned char* rgb)
{
    rgb[0] = i & 0xFF;
    rgb[1] = (i >> 8) & 0xFF;
    rgb[2] = (i >> 16) & 0xFF;
}

/**
 * unpack 3-element byte array into lowest 24 bits of an unsigned integer
 * @param rgb pointer to a 3-element byte array
 */
inline unsigned int rgb_to_uint(unsigned char rgb[3])
{
    return 
        static_cast<unsigned int>(rgb[0]) | 
            (static_cast<unsigned int>(rgb[1]) << 8) |
            (static_cast<unsigned int>(rgb[2]) << 16);
}

}
}

#endif
