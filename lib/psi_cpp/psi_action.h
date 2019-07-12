/* $Id: psi_action.h 12743 2012-07-25 23:52:02Z jguan1 $ */
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

#ifndef PSI_VERSION_1_ACTION_H
#define PSI_VERSION_1_ACTION_H

#include <l_cpp/l_exception.h>
#include <string>
#include <vector>
#include <psi_cpp/psi_units.h>
#include <people_tracking_cpp/pt_entity.h>

#include <boost/function.hpp>
namespace kjb
{
namespace psi
{

/**
 * Action representation.
 *
 * The design implemented here is somewhat of an experiment.  It's 
 * basically object oriented code written in the style of C.  The
 * motivation for this is as follows:
 *
 * (a) All actions need to be handled by a uniform way through a single type
 * (b) We'll never need interact with a single specific type of action, so having sub-classes for each action will largely be unused.
 * (c) We'd like to add new actions without needing to create new classes for them
 * (d) We need to know the type of action at run-time, and prefer to avoid the inelegance of using dynamic_cast and typeid().
 *
 *
 * State is represented by structs, polymorphism is implemented using a type index,
 * and objects are constructed by calling non-member functions.  However,
 * we still leverage useful features of C++, like function overloading, pass-by-const-reference,
 * return-by-value, etc.  
 *
 * The overall design could be pushed more toward an object-oriented interface in the future, but I'm curious to see what pitfalls (if any) strike us by using a more C-style interface.   Object oriented coding can add unwanted complexity, and I've heard anecdotes extoling the virtues of a more C-style approach inside C++ code.  I have a theory that the overall programmer experience will be improved by using a less sophisticated interface... we'll see.  
 */

enum Action_type {NULL_ACTION, WALK_ACTION, WALK_IN_ARC_ACTION, WALK_THROUGH_POINTS_ACTION, 
                    FOLLOW_ACTION};

// get information about an action
const std::string& get_name(Action_type t);
Unit_type get_units(Action_type a, size_t i);

Action_type action_name_to_type(const std::string& name);

/**
 * Representation of an action in psi.
 *
 * DESIGN RATIONALE:
 * This is intentionally simple (e.g. NOT object-oriented) to 
 * permit extensibility without changing any code.  For example
 * new actions types could be read from a file and immediately
 * available without creating a new type for each one.
 */
struct Action
{
    Action()  :
        parent_type(pt::NUM_ENTITY_TYPES),
        parent_index(-1)
    {
    }

    Action_type type;
    std::vector<double> parameters;

    // these two fields are for actions that depend on another entity's state
    int parent_type;
    int parent_index;
};

/**
 * Meta-structure that describes various actions, including its
 * name, and the names and units of all parameters.
 */
struct Action_descriptor
{
    Action_descriptor() {}

    Action_descriptor(
            Action_type type_,
            std::string name_,
            size_t num_params_,
            std::vector<std::string> param_names_,
            std::vector<Unit_type> param_units_, 
            boost::function1<double, const Action&> get_duration_,
            bool fixed_length_,
            bool is_relative_,
            boost::function1<bool, const Action&> validator_) :
        type(type_),
        name(name_),
        num_params(num_params_),
        param_names(param_names_),
        param_units(param_units_),
        get_duration(get_duration_),
        fixed_length(fixed_length_),
        is_relative(is_relative_),
        validator(validator_)
    { }


    Action_type type;
    std::string name;
    size_t num_params;
    std::vector<std::string> param_names;
    std::vector<Unit_type> param_units;

    boost::function1<double, const Action&> get_duration;

    /** 
     * fixed_length
     *
     * If false, the total number of parameters is unknown, but must be (num_params * n + 1), where n is a non-negative integer.  The first parameter of the action is the value of n, and the remaining parameters are repeated groups of values of size num_params..
     *
     * For example, a set of parameters {(1,2,3), (4,5,6), (7,8,9)} would have num_params == 3, and the parameters themselves would be represented as:
     * {3, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0}.
     */
    bool fixed_length;

    /**
     * is_relative
     *
     * if true, the action depends on other entities. For example, FOLLOW action depends on the entity that the current entity is following. In this case, the parent_type and parent_index of the Action needs to be set 
     */
    bool is_relative;

    boost::function1<bool, const Action&> validator;
};

double get_action_duration(const Action& action);

/** @brief Check that an action is consistent 
 * (correct number of parameters, etc)
 */
void validate(const Action& a);

/** @brief Construct a walk along the trajectory specified by the points */
Action make_walk_through_points_action
(
    const std::vector<std::vector<double> >& points
);

/** Construct a follow action 
 *  @param walker the actor to be followed 
 *  @param time_behind the distance behind the actor 
 **/
inline Action make_follow_action
(
    double duration,
    size_t parent_type,
    size_t parent_index,
    double time_behind 
)
{
    Action result;
    result.type = FOLLOW_ACTION;
    result.parent_type = parent_type;
    result.parent_index = parent_index;

    result.parameters.resize(2);
    result.parameters[0] = duration;
    result.parameters[1] = time_behind;
    return result;
}


/** @brief Construct a walk action */
inline Action make_walk_action(double duration, double speed)
{
    Action result;
    result.type = WALK_ACTION;
    result.parameters.resize(2);

    result.parameters[0] = duration;
    result.parameters[1] = speed;

    return result;
}

/** @brief Construct a walk-in-arc action */
inline Action make_walk_in_arc_action
(
    double duration, 
    double speed,
    double angular_speed
)
{
    Action result;
    result.type = WALK_IN_ARC_ACTION;
    result.parameters.resize(3);

    result.parameters[0] = duration;
    result.parameters[1] = speed;
    result.parameters[2] = angular_speed;

    return result;
}

/** @brief Construct a null action */
inline Action make_null_action(double duration)
{
    Action result;
    result.type = NULL_ACTION;
    result.parameters.resize(1);
    result.parameters[0] = duration;

    return result;
}

 /*----------------------------------------------------------------------*/
 /*                             Utility Functions                        */
 /*----------------------------------------------------------------------*/

/** @brief Get the number of waypoints from the size of the parameters 
 *  parameters is in size of 3*num_waypoints+1
 **/
inline size_t get_num_waypoints(size_t param_size)
{
    return (param_size - 1) / 3;
}

/** @brief Get the x index of the i'th point  */
inline size_t to_x_index(size_t i)
{
    return (1 + (3 * i + 0));
}

/** @brief Get the z index of the i'th point  */
inline size_t to_z_index(size_t i)
{
    return (1 + (3 * i + 1));
}

/** @brief Get the time point index of the i'th point */
inline size_t to_time_index(size_t i)
{
    return (1 + (3 * i + 2));
}

/** @brief serialize an action */
std::ostream& operator<<(std::ostream& ost, const Action& action);

/** @brief unserialize an action */
std::istream& operator>>(std::istream& ist, Action& action);

/** @brief read an action that is specified using command-line-interface format */
Action parse_cli_action(const std::string& str);


} // namespace psi
} // namespace kjb
#endif

