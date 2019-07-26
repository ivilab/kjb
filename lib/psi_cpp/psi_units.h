/* $Id: psi_units.h 10707 2011-09-29 20:05:56Z predoehl $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#ifndef PSI_UNITS_VERSION_1_H
#define PSI_UNITS_VERSION_1_H

namespace kjb
{
namespace psi
{

enum Unit_type { UNKNOWN_UNIT,  // error/bug
                  SPACIAL_UNIT,  // position, height, width, etc.
                  VSPACIAL_UNIT, // velocity
                  ASPACIAL_UNIT, // acceleration
                  ANGLE_UNIT,    // angles and orientations
                  VANGLE_UNIT,   // angular velocity
                  AANGLE_UNIT,   // angular acceleration
                  TIME_UNIT,     // seconds
                  MASS_UNIT,     // kilograms 
                  LENGTH_UNIT,     // meters
                  DISCRETE_UNIT,   // indexes 
                  OTHER_UNIT};    // placeholder (how to handle oddballs?)


} // namespace psi
} // namespace kjb

#endif
