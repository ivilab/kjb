/* $Id: m_sequence.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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


#include "m_cpp/m_sequence.h"
#include "l_cpp/l_exception.h"
#include <vector>

namespace kjb {
Interval_sequence::Interval_sequence(double start, double interval, double end) :
    start_(start),
    end_(end),
    interval_(interval)
{
    if((end - start) / interval < 0)
        KJB_THROW_2(Illegal_argument, "Invalid range.  start + N * interval must be greater or equal than end for some N >= 0.");

}

double Interval_sequence::operator[](size_t index) const
{
    double result = start_ + index * interval_;

    int sign = (interval_ < 0.0 ? -1 : 1);
    if(sign * result > sign * end_)
        KJB_THROW(Index_out_of_bounds);

    return result;
}

std::vector<double> Interval_sequence::to_vector() const
{
    size_t size = (end_ - start_) / interval_;
    std::vector<double> result(size);
    for(size_t i = 0; i < result.size(); i++)
    {
        result[i] = operator[](i);
    }

    return result;
}

} // namespace kjb
