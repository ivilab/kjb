/* $Id: psi_skeleton_detection.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Jinyan Guan
 * =========================================================================== }}}*/

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "psi_cpp/psi_skeleton_detection.h"

#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <string>

namespace kjb
{
namespace psi
{

Bbox Skeleton_detection::get_bounding_box() const
{
    double min_x = DBL_MAX; 
    double max_x = 0.0; 
    double min_y = DBL_MAX; 
    double max_y = 0.0;
    BOOST_FOREACH(const Bbox& box, *this)
    {
        Vector tl = box.get_top_left();
        Vector br = box.get_bottom_right();
        if(tl[0] < min_x)
            min_x = tl[0];
        if(tl[1] < min_y)
            min_y = tl[1];
        if(br[0] > max_x)
            max_x = br[0];
        if(br[1] > max_y)
            max_y = br[1];
    }
    return Bbox(Vector(min_x, min_y), Vector(max_x, max_y));
}

std::vector<Skeleton_detection> parse_skeleton_detection(std::istream& ist)
{
    using namespace boost;
    using namespace std;

    vector<Skeleton_detection> skeletons;
    string line;

    while(getline(ist, line))
    {
        Skeleton_detection skeleton;
        vector<string> tokens;
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of("\t ,\n\r"), boost::token_compress_on);
        const size_t num_fields = 4;
        ASSERT(tokens.size() == skeleton.size() * num_fields + 1 ||
                tokens.size() == skeleton.size() * num_fields + 2);

        size_t i = 0; 
        for(size_t part_i = 0; part_i < skeleton.size(); part_i++)
        {
            Vector p1(2), p2(2);
            p1[0] = boost::lexical_cast<double>(tokens[i++]);
            p1[1] = boost::lexical_cast<double>(tokens[i++]);
            p2[0] = boost::lexical_cast<double>(tokens[i++]);
            p2[1] = boost::lexical_cast<double>(tokens[i++]);
            skeleton.set_body_part(part_i, Bbox(p1, p2));
        }
        if (tokens.size() == skeleton.size() * num_fields + 2)
        {
            i++;
        }
        double score = boost::lexical_cast<double>(tokens[i++]);
        skeleton.set_score(score);
        skeletons.push_back(skeleton);
    }
    return skeletons;
}

}
}
