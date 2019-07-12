/* $Id: psi_skeleton.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "psi_cpp/psi_skeleton.h"

#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <string>

namespace kjb {
namespace psi {

typedef boost::bimap<Body_part_entity, std::string> Body_part_map;
typedef Body_part_map::value_type Body_part_map_entry;
const Body_part_map body_part_map = boost::assign::list_of
    (Body_part_map_entry(BODY, "Person"))
    (Body_part_map_entry(HEAD, "Head"))
    (Body_part_map_entry(CHEST, "Chest"))
    (Body_part_map_entry(LEFT_ELBOW, "ElbowLeft"))
    (Body_part_map_entry(RIGHT_ELBOW, "ElbowRight"))
    (Body_part_map_entry(LEFT_HAND, "HandLeft"))
    (Body_part_map_entry(RIGHT_HAND, "HandRight"))
    (Body_part_map_entry(LEFT_KNEE, "KneeLeft"))
    (Body_part_map_entry(RIGHT_KNEE, "KneeRight"))
    (Body_part_map_entry(LEFT_FOOT, "FootLeft"))
    (Body_part_map_entry(RIGHT_FOOT, "FootRight"));
    
Body_part_entity Psi_skeleton::get_body_part_entity(const std::string& label) const
{
    return body_part_map.right.at(label);
}

void Psi_skeleton::set_body_part(const std::string& label, const Psi_body_part& part)
{
    Body_part_entity part_type = get_body_part_entity(label);
    set_body_part(part_type, part);
}

std::ostream& operator<<(std::ostream& ost, const Psi_skeleton& skeleton)
{
    std::streamsize w = ost.width();
    std::streamsize p = ost.precision();
    std::ios::fmtflags f = ost.flags();

    Psi_skeleton::const_iterator it;
    for(it = skeleton.begin(); it < skeleton.end(); it++)
    {
        ost << std::setw(10) << *it;
    }

    ost.width( w );
    ost.precision( p );
    ost.flags( f );
    
    return ost;
}


/*std::istream& operator>>(const std::istream& ist, Psi_skeleton& skeleton)
{
    Psi_skeleton::iterator it;
    for(it = skeleton.begin(); it < skeleton.end(); it++)
    {
        Psi_body_part part;
        ist >> part;
        it->set_body_part(part.label, part);
    }
    return ist; 
}
*/


std::vector<Psi_skeleton> parse_skeleton(std::istream& ist)
{
    using namespace boost;
    using namespace std;

    vector<Psi_skeleton> skeletons;
    string line;

    while(getline(ist, line))
    {
        Psi_skeleton skeleton;
        vector<string> tokens;
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of("\t ,\n\r"), boost::token_compress_on);
        const size_t num_fields = 7;
        ASSERT(tokens.size() == skeleton.size() * num_fields + 1 ||
                tokens.size() == skeleton.size() * num_fields);

        size_t i = 0; 
        for(size_t part_i = 0; part_i < skeleton.size(); part_i++)
        {
            Psi_body_part part; 
            part.label = boost::lexical_cast<string>(tokens[i++]);
            part.lost = boost::lexical_cast<int>(tokens[i++]);
            part.occluded = boost::lexical_cast<int>(tokens[i++]);
            Vector p1(2), p2(2);
            p1[0] = boost::lexical_cast<double>(tokens[i++]);
            p1[1] = boost::lexical_cast<double>(tokens[i++]);
            p2[0] = boost::lexical_cast<double>(tokens[i++]);
            p2[1] = boost::lexical_cast<double>(tokens[i++]);
            part.box = Bbox(p1, p2); 
            skeleton.set_body_part(part.label, part);
        }
        skeletons.push_back(skeleton);
    }
    return skeletons;
}

} // namespace psi
} // namespace kjb
