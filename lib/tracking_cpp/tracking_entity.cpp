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
   |  Author:  Kyle Simek, Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id: tracking_entity.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "tracking_cpp/tracking_entity.h"
#include "l_cpp/l_exception.h"

#include <string>
#include <vector>
#include <ostream>
#include <istream>
#include <ios>
#include <iomanip>
#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/lexical_cast.hpp>

using namespace kjb;
using namespace kjb::tracking;

// Map between names and types
typedef boost::bimap<Entity_type, std::string> Entity_type_map;
typedef Entity_type_map::value_type Entity_map_entry;
const Entity_type_map entity_type_map = boost::assign::list_of
    (Entity_map_entry(PERSON_ENTITY, std::string("person")))
    (Entity_map_entry(BICYCLE_ENTITY, "bicycle"))
    (Entity_map_entry(BOTTLE_ENTITY, "bottle"))
    (Entity_map_entry(CAR_ENTITY, "car"))
    (Entity_map_entry(CHAIR_ENTITY, "chair"))
    (Entity_map_entry(DOG_ENTITY, "dog"))
    (Entity_map_entry(FRAMES_ENTITY, "frames"))
    (Entity_map_entry(MOTORBIKE_ENTITY, "motorbike"))
    (Entity_map_entry(OBJECT_ENTITY, "object"))
    (Entity_map_entry(BOX_ENTITY, "box"))
    (Entity_map_entry(BAG_ENTITY, "bag"));

// entity average heights
const double entity_type_avg_heights[] = {1.7, -1.0, -1.0, -1.0, -1.0,
                                         -1.0, -1.0, -1.0, -1.0};

// entity standard deviation of heights
const double entity_type_sdv_heights[] = {0.1, -1.0, -1.0, -1.0, -1.0,
                                         -1.0, -1.0, -1.0, -1.0};

// entity average widths
const double entity_type_avg_widths[] = {0.4, -1.0, -1.0, -1.0, -1.0,
                                        -1.0, -1.0, -1.0, -1.0};

// entity standard deviation of widths
const double entity_type_sdv_widths[] = {0.04, -1.0, -1.0, -1.0, -1.0,
                                         -1.0, -1.0, -1.0, -1.0};

// entity average girths
const double entity_type_avg_girths[] = {0.3, -1.0, -1.0, -1.0, -1.0,
                                        -1.0, -1.0, -1.0, -1.0};

// entity standard deviation of girths
const double entity_type_sdv_girths[] = {0.04, -1.0, -1.0, -1.0, -1.0,
                                         -1.0, -1.0, -1.0, -1.0};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<std::string> kjb::tracking::get_all_entity_type_names()
{
    std::vector<std::string> result;

    typedef Entity_type_map::left_map::const_iterator Iterator;
    for(Iterator it = entity_type_map.left.begin();
                 it != entity_type_map.left.end(); ++it)
    {
        result.push_back((*it).second);
    }

    return result;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Entity_type kjb::tracking::get_entity_type(const std::string& name)
{
    return entity_type_map.right.at(name);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

const std::string& kjb::tracking::get_entity_type_name(Entity_type type)
{
    return entity_type_map.left.at(type);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::tracking::get_entity_type_average_height(Entity_type type)
{
    return entity_type_avg_heights[type];
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::tracking::get_entity_type_stddev_height(Entity_type type)
{
    return entity_type_sdv_heights[type];
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::tracking::get_entity_type_average_width(Entity_type type)
{
    return entity_type_avg_widths[type];
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::tracking::get_entity_type_stddev_width(Entity_type type)
{
    return entity_type_sdv_widths[type];
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::tracking::get_entity_type_average_girth(Entity_type type)
{
    return entity_type_avg_girths[type];
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::tracking::get_entity_type_stddev_girth(Entity_type type)
{
    return entity_type_sdv_girths[type];
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::ostream& kjb::tracking::operator<<(std::ostream& ost, Entity_type type)
{
    // NUM_ENTITY_TYPES is a sentinal value, and should never be read/written
    ASSERT(type != NUM_ENTITY_TYPES);

    ost << entity_type_map.left.at(type);
    return ost;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::istream& kjb::tracking::operator>>(std::istream& ist, Entity_type& type)
{
    // NUM_ENTITY_TYPES is a sentinal value, and should never be read/written
    ASSERT(type != NUM_ENTITY_TYPES);

    std::string token;
    ist >> token;

    if(entity_type_map.right.count(token) == 0)
    {
        ist.setstate(std::ios::failbit);
    }
    else
    {
        type = entity_type_map.right.at(token);
    }

    return ist;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::istream& kjb::tracking::operator>>(std::istream& ist, Entity_id& id)
{
    // this will restore the "skipws" state after leaving scope.
    boost::io::ios_flags_saver  ifs( ist );
    boost::io::ios_exception_saver  ixs( ist );
    // required for lexical_cast to work
    ist.exceptions(std::istream::failbit | std::istream::badbit );
    ist >> std::skipws;

    std::string stype, sindex;
    ist >> stype;
    ASSERT(!(!ist));
    ist >> sindex;
    ASSERT(!(!ist));

    // '.' means null
    if(stype != ".")
    {
        if(sindex == ".")
        {
            // must specify both stype and sindex
            KJB_THROW_2(IO_error, "Cannot read entity-id; wrong format.");
        }

        id.type = boost::lexical_cast<Entity_type>(stype);
        id.index = boost::lexical_cast<int>(sindex);
    }

    return ist;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::ostream& kjb::tracking::operator<<(std::ostream& ost, const Entity_id& id)
{
    if(!id)
    {
        ost << ". .";
    }
    else
    {
        ost << id.type << ' ';
        ost << id.index;
    }

    return ost;
}

