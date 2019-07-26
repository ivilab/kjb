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

/* $Id: tracking_entity.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef TRACKING_ENTITY_H
#define TRACKING_ENTITY_H

#include "l/l_sys_debug.h"  /* For ASSERT. */

#include <string>
#include <vector>
#include <istream>
#include <ostream>
/*
#include <cassert>
*/

namespace kjb {
namespace tracking {

/**
 * @file Structs and functions for representing, reading, and writing Entity
 * types, which are based on the objects we have detectors for.  If you're
 * incorporating data from a new detector, you'll need to edit this file.
 */

// To add a new type, add entries to Entity_type, and Entity_type_map;
enum Entity_type { PERSON_ENTITY, BICYCLE_ENTITY, BOTTLE_ENTITY,
                   CAR_ENTITY, CHAIR_ENTITY, DOG_ENTITY, FRAMES_ENTITY,
                   MOTORBIKE_ENTITY, OBJECT_ENTITY, BOX_ENTITY, 
                   BAG_ENTITY, NUM_ENTITY_TYPES };

/** @brief  Get all types. */
std::vector<std::string> get_all_entity_type_names();

/** @brief  Get the name of a entity type. */
const std::string& get_entity_type_name(Entity_type type);

/** @brief  Get the type of a entity name. */
Entity_type get_entity_type(const std::string& name);

/** @brief  Get the average height of an entity. */
double get_entity_type_average_height(Entity_type type);

/** @brief  Get the standard deviation of the height of an entity. */
double get_entity_type_stddev_height(Entity_type type);

/** @brief  Get the average width of an entity. */
double get_entity_type_average_width(Entity_type type);

/** @brief  Get the standard deviation of the width of an entity. */
double get_entity_type_stddev_width(Entity_type type);

/** @brief  Get the average girth of an entity. */
double get_entity_type_average_girth(Entity_type type);

/** @brief  Get the standard deviation of the girth of an entity. */
double get_entity_type_stddev_girth(Entity_type type);

/** @brief  Stream out an entity. */
std::ostream& operator<<(std::ostream& ost, Entity_type type);

/** @brief  Stream in an entity. */
std::istream& operator>>(std::istream& ist, Entity_type& type);

/**
 * @struct  Entity_id
 * @brief   Entity + index; used for file I/O.
 */
struct Entity_id
{
    Entity_type type;
    int index;

    Entity_id() :
        type(NUM_ENTITY_TYPES),
        index(-1)
    {}

    Entity_id(Entity_type type_, int index_) :
        type(type_),
        index(index_)
    {}
    
    bool operator!=(const Entity_id& other) const
    {
        return !operator==(other);
    }

    bool operator==(const Entity_id& other) const
    {
        if(type != other.type) return false;
        if(index != other.index) return false;
        return true;
    }

    bool operator<(const Entity_id& other) const
    {
        // lexicographical ordering
        return type < other.type || (type == other.type && index < other.index);
    }

    bool operator!() const
    {
        // for consistency, either both values are "null" or neither are.
        ASSERT((type == NUM_ENTITY_TYPES && index == -1) || 
               (type != NUM_ENTITY_TYPES && index != -1));

        return (type == NUM_ENTITY_TYPES);
    }
};

/** @brief  Stream out an entity-id. */
std::istream& operator>>(std::istream& ist, Entity_id& id);

/** @brief  Stream in an entity-id. */
std::ostream& operator<<(std::ostream& ost, const Entity_id& id);

}} //namespace kjb::tracking

#endif /* TRACKING_ENTITY_H */

