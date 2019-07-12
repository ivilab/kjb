/* $Id: test_concept.cpp 17431 2014-09-01 18:49:11Z predoehl $ */
/* =========================================================================== *
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
 * =========================================================================== */

#include <m/m_incl.h>
#include <m_cpp/m_cpp_incl.h>
#include <l/l_incl.h>
#include <iostream>
//#include <cstdlibs>
#ifdef KJB_HAVE_BOOST_HEADERS
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
using namespace boost;
#endif


using namespace kjb;
using namespace std;



// This test is not intended to run.  If it compiles successfully, the test has passed.

int main (int /* argc */, char ** /* argv */)
{
    kjb_c::kjb_init();
#ifdef KJB_HAVE_BOOST_HEADERS
    // Vector is a model of RandomAccessContainer
    // http://www.sgi.com/tech/stl/RandomAccessContainer.html
    BOOST_CONCEPT_ASSERT((RandomAccessContainer<Vector>));
    BOOST_CONCEPT_ASSERT((BackInsertionSequence<Vector>));
#endif

    return EXIT_SUCCESS;
}
