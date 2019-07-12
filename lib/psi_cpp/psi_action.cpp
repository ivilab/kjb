/* $Id: psi_action.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "l/l_sys_debug.h"  /* For ASSERT */
#include "psi_cpp/psi_action.h"
#include "people_tracking_cpp/pt_entity.h"

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

namespace kjb
{
namespace psi
{

using boost::assign::list_of;

void validate(Action_type a);
std::istream& read_action_v0(std::istream& ist, Action& action);


static boost::function1<bool, const Action&> null_validator;

static double get_default_duration(const Action& action)
{
    return action.parameters[0];
}

/// @pre parameter vector has valid structure; i.e. validate(action) has been called
static bool walk_through_points_validator(const Action& action)
{
    // no waypoints
    if(action.parameters[0] == 0) return true;

    // Make sure waypoint times are monotonically increasing
    double cur_t = 0;
    for(size_t i = 3; i < action.parameters.size(); i += 3)
    {
        double new_t = action.parameters.at(i);
        if(new_t < cur_t) return false;

        cur_t = new_t;
    }
    return true;
}

static double get_walk_through_points_duration(const Action& action)
{

    size_t n = action.parameters.size();

    ASSERT(n > 0);

    if(action.parameters[0] == 0)
    {
        // zero waypoints
        
        ASSERT(n == 1);
        return 0.0;
    }

    // last element of the parameters array is the final time, which is also the duration of the entire set of waypoints.
    return action.parameters[n-1];
}

static double get_follow_duration(const Action& action)
{
    size_t n = action.parameters.size();

    // the second to the last element of the parameters array is the time of the last 
    // points along the trajectory 
    // the last parameter is how much in time the follower is behind the followee
    return action.parameters[n-2] + action.parameters[n-1];
}

/**
 * The following array contains metadata for all action types.
 * Creating a new action consists of (a) adding a new name to 
 * the Action_type enum, and (b) creating a new entry to this array.
 *
 * Most functions that follow rely on this structure to get
 * the necessary information.
 */
const std::vector<Action_descriptor> action_descriptors = list_of
    // NULL ACTION
    (Action_descriptor(
        NULL_ACTION,  // type
        "NULL_ACTION", // name
        1,  // num_params
        list_of<std::string> // param names
            ("duration")
        , 
        list_of<Unit_type> // param units
            (TIME_UNIT),
        get_default_duration,
        true, // fixed_length
        false, // relative 
        null_validator
        ))
    // WALK ACTION
    (Action_descriptor(
        WALK_ACTION,  // type
        "WALK_ACTION", // name
        2,  // num_params
        list_of<std::string> // param names
            ("duration")
            ("speed")
        , 
        list_of<Unit_type> // param units
            (TIME_UNIT)
            (VSPACIAL_UNIT),
        get_default_duration,
        true, // fixed length
        false, // relative 
        null_validator
        ))
    // WALK-IN-ARC ACTION
    (Action_descriptor(
        WALK_IN_ARC_ACTION, // type
        "WALK_IN_ARC_ACTION", // name
        3,  // num_params
        list_of<std::string> // param names
            ("duration")
            ("speed")
            ("angular_speed")
        , 
        list_of<Unit_type> // param units
            (TIME_UNIT)
            (VSPACIAL_UNIT)
            (ANGLE_UNIT),
        get_default_duration,
        true, // fixed length
        false, // relative 
        null_validator
        ))
    // WALK_THROUGH_POINTS ACTION
    (Action_descriptor(
        WALK_THROUGH_POINTS_ACTION, // type
        "WALK_THROUGH_POINTS_ACTION", // name 
        3,  // num_params
        list_of<std::string> // param names
            ("x_position")
            ("y_position")
            ("time")
        , 
        list_of<Unit_type> // param units
            (SPACIAL_UNIT)
            (SPACIAL_UNIT)
            (TIME_UNIT)
        ,
        get_walk_through_points_duration,
        false,  // !fixed_length
        false, // relative 
        walk_through_points_validator
        ))
    // FOLLOW ACTION
    (Action_descriptor(
        FOLLOW_ACTION, // type
        "FOLLOW_ACTION", // name 
        2,  // num_params
        list_of<std::string> // param names
            ("duration")
            ("behind_time")
        , 
        list_of<Unit_type> // param units
            (TIME_UNIT)
            (TIME_UNIT)
        ,
        get_default_duration,
        true,  // !fixed_length
        true, // relative 
        null_validator
        ))
    ;


/// Convert Action_type to string
const std::string& get_name(Action_type t)
{
    static const std::string unknown_str("UNKNOWN_ACTION");


    const size_t i = static_cast<size_t>(t);

    if(i >= action_descriptors.size())
        return unknown_str;

    return action_descriptors[i].name;
}

/// Convert string to Action_type.
Action_type action_name_to_type(const std::string& name)
{
    static std::map<std::string, Action_type> type_map;

    if(type_map.size() == 0)
    {
        for(size_t i = 0; i < action_descriptors.size(); i++)
        {
            type_map[action_descriptors[i].name] = static_cast<Action_type>(i);
        }
    }

    if(type_map.count(name) == 0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Unknown action name.");
    }

    return type_map[name];
}

double get_action_duration(const Action& action)
{
    return action_descriptors[action.type].get_duration(action);
}

/// check that action_type represents an actual descriptor.
void validate(Action_type a)
{
    const size_t a_i = static_cast<size_t>(a);


    if(a_i >= action_descriptors.size())
    {
        KJB_THROW_2(kjb::Illegal_argument, "Unknown action type.");
    }
}

/** @brief Construct a walk along the trajectory specified by the points */
Action make_walk_through_points_action
(
    const std::vector<std::vector<double> >& points
)
{
    Action result;
    result.type = WALK_THROUGH_POINTS_ACTION;

    size_t NUM_PARAMS = action_descriptors[result.type].num_params;

    result.parameters.resize(points.size() * NUM_PARAMS + 1);
    size_t j = 0;

    // first parameter is the number of "blocks"
    result.parameters[j++] = points.size();

    for(size_t p = 0; p < points.size(); p++)
    {
        ASSERT(points[p].size() == NUM_PARAMS);
        for(size_t i = 0; i < points[p].size(); i++)
        {
            result.parameters.at(j++) = points[p][i];
        }
    }

#ifdef TEST
    validate(result);
#endif

    return result;
}


void validate(const Action& a)
{
    validate(a.type);

    const Action_descriptor& meta = action_descriptors[a.type];

    if(meta.fixed_length)
    {
        if(meta.num_params != a.parameters.size())
        {
            KJB_THROW_2(kjb::Runtime_error, "Actions parameter count is invalid.");
        }
    }
    else
    {
        size_t group_size = a.parameters[0];
        size_t total_num_params = meta.num_params * group_size + 1;
        if(total_num_params != a.parameters.size())
        {
            KJB_THROW_2(kjb::Runtime_error, "Actions parameter count is invalid.");
        }
    }

    if(meta.is_relative)
    {
        if(a.parent_type == pt::NUM_ENTITY_TYPES || a.parent_index < 0)
        {
            KJB_THROW_2(kjb::Runtime_error, "Parent is not initialized.");
        }
    }


    if(meta.validator)
    {
        if(!meta.validator(a))
            KJB_THROW_3(kjb::Runtime_error, "%s action validation failed.", (meta.name.c_str()));
    }
}

Unit_type get_units(Action_type a, size_t i)
{

    // action index
    validate(a);
    
    const size_t a_i = static_cast<size_t>(a);

    return action_descriptors[a_i].param_units.at(i);
}

std::ostream& operator<<(std::ostream& ost, const Action& action)
{
    ost << get_name(action.type);

    ost << " " << action.parameters.size();
    for(size_t param = 0; param < action.parameters.size(); param++)
    {
        ost << " " << action.parameters[param];
    }

    return ost;
}

std::istream& operator>>(std::istream& ist, Action& action)
{
    // if output format changes, we'll need a switch statement
    // here to call the correct version of the read function.
    // For now, version 0 is all we have.
    return read_action_v0(ist, action);
}

std::istream& read_action_v0(std::istream& ist, Action& action)
{
    std::string action_name;
    ist >> action_name;

    action.type = action_name_to_type(action_name);

    size_t param_size;
    ist >> param_size;

    action.parameters.resize(param_size);

    for(size_t i = 0; i < param_size; i++)
    {
        ist >> action.parameters[i];
    }

    return ist;
}

/**
 * similar format to reading from istream, except num-parameters
 * is not explicitly specified, but is implied from token count,
 * making hand-writing easy, but parsing slightly harder.
 *
 * e.g.: "WALK_ACTION 5 0.5" would construct a walk action with
 * duration 5 and speed 0.5
 */
Action parse_cli_action(const std::string& str)
{
    using namespace boost;
    using namespace std;

    // split into tokens
    vector<string> tokens;
    split(tokens, str, is_any_of(" \t\r\n"), token_compress_on);

    size_t i = 0;


    // parse
    Action action;
    action.type = action_name_to_type(tokens[i++]);

    const Action_descriptor& meta = action_descriptors[action.type];

    if(meta.is_relative)
    {
        action.parent_type = boost::lexical_cast<pt::Entity_type>(tokens.at(i++));
        action.parent_index = boost::lexical_cast<size_t>(tokens.at(i++));
    }

    action.parameters.resize(tokens.size() - i);
    size_t param_i = 0;
    for(; i < tokens.size(); i++)
    {
        action.parameters[param_i++] = lexical_cast<double>(tokens[i]);
    }

    validate(action);

    return action;
}

} // namespace psi
} // namespace kjb
