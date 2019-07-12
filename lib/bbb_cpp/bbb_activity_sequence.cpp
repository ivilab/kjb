/* =========================================================================== *
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#include "bbb_cpp/bbb_activity_sequence.h"

#include <iostream>
#include <boost/variant.hpp>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::bbb;
using namespace std;

ostream& kjb::bbb::operator<<(ostream& ost, const Activity_sequence& aseq)
{
    // output name and times
    ost << "ROLE: " << aseq.role() << endl;

    // output member activities
    Output_activity visitor(ost);
    BOOST_FOREACH(const Activity_sequence::Activity& act, aseq)
    {
        boost::apply_visitor(visitor, act);
        ost << std::endl;
    }

    return ost;
}

