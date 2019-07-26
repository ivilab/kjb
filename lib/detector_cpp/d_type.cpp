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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: d_type.cpp 14240 2013-04-11 08:13:35Z jguan1 $ */

#include <detector_cpp/d_type.h>

#include <string>
#include <ostream>
#include <istream>
#include <vector>
#include <ios>

#include <iomanip>
#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>

using namespace kjb;

// Map between names and types
typedef boost::bimap<Detection_type, std::string> Detection_type_map;
typedef Detection_type_map::value_type Detection_map_entry;
const Detection_type_map detection_type_map = boost::assign::list_of
    (Detection_map_entry(DEVA, std::string("deva_box")))
    (Detection_map_entry(CV_FRONTAL_DEFAULT, "cv_frontal_default"))
    (Detection_map_entry(CV_FRONTAL_ALT, "cv_frontal_alt"))
    (Detection_map_entry(CV_FRONTAL_ALT2, "cv_frontal_alt2"))
    (Detection_map_entry(CV_FRONTAL_ALT_TREE, "cv_frontal_alt_tree"))
    (Detection_map_entry(CV_PROFILE, "cv_profile"))
    (Detection_map_entry(FACE_COM, std::string("face_com")));

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::string> kjb::get_all_detection_type_names()
{
    std::vector<std::string> result;

    typedef Detection_type_map::left_map::const_iterator Iterator;
    for(Iterator it = detection_type_map.left.begin();
                 it != detection_type_map.left.end(); ++it)
    {
        result.push_back((*it).second);
    }

    return result;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Detection_type kjb::get_detection_type(const std::string& name)
{
    return detection_type_map.right.at(name);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

const std::string& kjb::get_detection_type_name(Detection_type type)
{
    return detection_type_map.left.at(type);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::ostream& kjb::operator<<(std::ostream& ost, Detection_type type)
{
    // NUM_DETECTION_TYPES is a sentinal value, and should never be read/written
    assert(type != NUM_DETECTION_TYPES);

    ost << detection_type_map.left.at(type);
    return ost;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::istream& kjb::operator>>(std::istream& ist, Detection_type& type)
{
    // NUM_DETECTION_TYPES is a sentinal value, and should never be read/written
    assert(type != NUM_DETECTION_TYPES);

    std::string token;
    ist >> token;

    if(detection_type_map.right.count(token) == 0)
    {
        ist.setstate(std::ios::failbit);
    }
    else
    {
        type = detection_type_map.right.at(token);
    }

    return ist;
}

