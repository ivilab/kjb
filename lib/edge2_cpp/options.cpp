/* $Id: options.cpp 17393 2014-08-23 20:19:14Z predoehl $ */
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

#include <edge2_cpp/options.h>

#include <boost/assign/list_of.hpp>

namespace kjb
{

#ifdef KJB_HAVE_BST_POPTIONS
namespace po = boost::program_options;

boost::program_options::options_description
get_canny_edge_detector_options()
{
    po::options_description edge_options("Edge Detector Configuration");
    edge_options.add_options()
        ("canny-begin-threshold", po::value<float>(), "Edge detection threshold.")
        ("canny-end-threshold", po::value<float>(), "Edge detector threshold for ending hysteresis.")
        ("canny-sigma", po::value<float>(), "Edge detector blurring sigma.")
        ;

    return edge_options;
}


Canny_edge_detector make_canny_edge_detector(const boost::program_options::variables_map& options)
{
    using namespace std;
    using boost::assign::list_of;

    const vector<string> required = list_of
        (std::string("begin-threshold"))
        ("end-threshold")
        ("sigma");

    for(size_t i = 0; i < required.size(); i++)
    {
        if(options.count(required[i]) == 0)
            KJB_THROW_2(Missing_option, required[i]);
    }

    const float sigma = options["sigma"].as<float>();
    const float begin_threshold = options["begin-threshold"].as<float>();
    const float end_threshold = options["end-threshold"].as<float>();

    return Canny_edge_detector(sigma, begin_threshold, end_threshold);

}
#endif /* KJB_HAVE_BST_POPTIONS */

}
