/* $Id: psi_start_state.h 10707 2011-09-29 20:05:56Z predoehl $ */
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
#ifndef PSI_V1_START_STATE_H
#define PSI_V1_START_STATE_H

#include <psi_cpp/psi_units.h>
#include <string>
#include <sstream>

namespace kjb
{
namespace psi
{

struct Start_state
{
    Start_state() :
        x(),
        y(),
        theta()
    {
    }

    Start_state(double in_x, double in_y, double in_theta) :
        x(in_x),
        y(in_y),
        theta(in_theta)
    {}

    double x;
    double y;
    double theta;

    double operator[](size_t i) const 
    {
        switch(i)
        {
            case 0: return x;
            case 1: return y;
            case 2: return theta;
            default: KJB_THROW(kjb::Index_out_of_bounds);
        }
    }

    double& operator[](size_t i) 
    {
        switch(i)
        {
            case 0: return x;
            case 1: return y;
            case 2: return theta;
            default: KJB_THROW(kjb::Index_out_of_bounds);
        }
    }

    static size_t size() {return 3;}

    static Unit_type get_units(size_t i)
    {
        switch(i)
        {
            case 0: return SPACIAL_UNIT;
            case 1: return SPACIAL_UNIT;
            case 2: return ANGLE_UNIT;
            default : KJB_THROW_2(kjb::Illegal_argument, "Unknown unit type.");
        }
    }
};

inline std::ostream& operator<<(std::ostream& ost, const Start_state& start_state)
{
        ost << start_state.x << " ";
        ost << start_state.y << " ";
        ost << start_state.theta;

        return ost;
}

inline std::istream& operator>>(std::istream& ist, Start_state& start_state)
{
    using namespace std;

    ist >> start_state.x;
    ist >> start_state.y;
    ist >> start_state.theta;

    return ist;
}

inline Start_state parse_cli_start_state(const std::string& str)
{
    std::istringstream ist(str);
    Start_state start;
    ist >> start;
    return start;
}

} // namespace psi
} // namespace kjb
#endif
