/* $Id: psi_cylinder_world.h 18331 2014-12-02 04:30:55Z ksimek $ */
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

#ifndef PSI_CYLINDER_WORLD_H
#define PSI_CYLINDER_WORLD_H

/*
 * TODO: rename this file to psi_simulator.h
 * Rationale: this now contains cartwheel stuff too (Simulator_type)
 */

#include <m_cpp/m_vector.h>
#include <g_cpp/g_cylinder.h>
#include <st_cpp/st_parapiped.h>
#include <camera_cpp/perspective_camera.h>
#include <vector>
#include <algorithm>

#include <psi_cpp/psi_action.h>
#include <psi_cpp/psi_start_state.h>
#include <psi_cpp/psi_weighted_box.h>

#include <psi_cpp/psi_bbox.h>
#include <psi_cpp/psi_util.h>

#include <boost/shared_ptr.hpp>

namespace kjb
{
namespace psi
{

/// @file Classes for doing simple action simulations without physics

/**
 * This class manages the state of an "entity" object in the 
 * scene.
 *
 * By convention, +y is up, -z is roughly "into" screen, and +x is right.
 *
 * "Heading" describes facing direction, with zero representing the +z direction, and rotating counter-clockwise as angle increases (right-handed convention)
 */
class Entity_state
{
public:
    Entity_state();

    virtual void render() const;
    kjb::Cylinder get_cylinder() const { 
        return kjb::Cylinder(
                get_position_3d(), // base
                get_position_3d() + kjb::Vector(0.0, 1.0, 0.0) * get_height(), // top
                get_width()/2.0); // radius
    }

    const kjb::Vector& get_position() const {return pos_; }
    kjb::Vector get_position_3d() const {return kjb::Vector(pos_[0], 0.0, pos_[1]); }

    const kjb::Vector& get_direction() const { return dir_; }
    kjb::Vector get_direction_3d() const { return kjb::Vector(dir_[0], 0.0, dir_[1]); }

    double get_heading() const {return heading_; }

    double get_height() const {return height_; }
    double get_width() const {return width_; }

    void set_position(const kjb::Vector& p) {pos_ = p; }

    /// @human direction in radians
    void set_heading(double heading ) 
    {
        heading_ = heading; 

        double rad = heading_;

        // angle = 0 -> pointing in z direction
        dir_[1] = cos(rad);

        // angle = epsilon -> x slightly negative
        dir_[0] = sin(rad);
    }

    void set_height(double h) {height_ = h; }
    void set_width(double w) {width_ = w; }

    /* 
     * @brief Move the entity @distance away in the direction that 
     * sepcified by @dir_
     */

    void move_forward(double distance)
    {
        pos_ += distance * dir_;
    }

private:
    kjb::Vector pos_;

    double heading_;
    kjb::Vector dir_;


    double height_;
    double width_;
};


inline Bbox get_bounding_box(const Entity_state& entity, const kjb::Perspective_camera& cam)
{
    kjb::Cylinder c = entity.get_cylinder();
    return get_bounding_box(c, cam);

}

class Simple_simulator
{

private:
    struct Action_context
    {
        size_t action_index;
        size_t entity_type;
        size_t entity_index;
        const Action* action;
        std::vector<double> memory;
    };
    typedef boost::shared_ptr<Action_context> Action_context_ptr;

public:
    Simple_simulator() :
        sampling_rate_(2),
        sampling_period_(0.5)
    { }

    bool  simulate(
        const std::vector<std::vector<Start_state> >& start_state,
//        const std::vector<Weighted_box>& boxes,
        const std::vector<std::vector<std::vector<Action> > >& actions 
        ) ;

//    bool simulate(
//        const std::vector<Start_state>& start_state,
//        const std::vector<std::vector<Action> >& actions 
//        ) 
//    {
//        return simulate(start_state, std::vector<Weighted_box>(), actions);
//    }


    /// returns 2d array or entity states, indexed by states[actor][frame]
    const std::vector<std::vector<std::vector<Entity_state> > >& get_entity_states() const
    {
        return entity_states_;
    }

    void set_sampling_rate(double r) 
    {
        sampling_rate_ = r;
        sampling_period_ = 1/r;
    }

protected:
    /**
     * If multiple cylinders, and one's actions take longer
     * than another's, this adds a null action to the shorter
     * one so they're all the same length.
     *
     * @pre total duration of all actions is <= duration
     */
//    void pad_actions(std::vector<Action>& actions, double duration) const;

    /// return the length of simulation, as defined by a sequence of extended actions
    double get_actions_length(const std::vector<Action>& actions);

    /**
     * Convert action durations from seconds (float point) to sample counts (integers).  Takes care to not accumulate drift over long trajectories.
     *
     */
    std::vector<size_t> quantize_action_durations(const std::vector<Action>& actions);

    /**
     * Document soon...
     * Quantize the WALK_THROUGH_POINTS_ACTION into 
     */
    std::vector<double> quantize_walk_through_points_action(const std::vector<double>& parameters);

    /**
     * Run an action for N time-steps.  Saves the state at every time-step in the state log, and updates the time counter.
     *
     * @param time The time index to start running the action.  After returning, time will be updated to be the time index for the next action to start running.
     * @pre All states for entity at previous times have already been computed.
     */
    void execute_action(size_t time, Action_context& context)
    {
        switch(context.action->type)
        {
            case FOLLOW_ACTION:
                execute_follow_action(time, context);
                break;
            case WALK_THROUGH_POINTS_ACTION:
                execute_walk_through_points_action(time, context);
                break;
            case WALK_IN_ARC_ACTION:
                execute_walk_in_arc_action(time, context);
                break;
            case WALK_ACTION:
                execute_walk_action(time, context);
                break;
            case NULL_ACTION:
                execute_null_action(time, context);
                break;
            default:
                KJB_THROW_2(kjb::Illegal_argument, "Unknown action type.");
                break;
        }
    }

    /**
     * Handler for "null" activity.
     * @sa execute_action
     */
    void execute_null_action(
        size_t time,
        Action_context& context
    );

    /**
     * Handler for "walk" action.
     *
     * @pre All previous states for all entities have been computed.
     * @pre Current states for all dependency entities have been computed.
     * @sa execute_action
     * */
    void execute_walk_action(
        size_t time, 
        Action_context& context
    );

    /**
     * Handler for "walk-and-turn" action
     *
     * @param time The time index to start running the action.  
     * After returning, time will be updated to be the time index for the next action 
     * to start running.  Must be > 0.
     *
     * */
    void execute_walk_in_arc_action
    (
        size_t time, 
        Action_context& context
    );

    /**
     * Handler for "walk_though_points" action.
     *
     * @param time The time index to start running the action.  After returning, time will be updated to be the time index for the next action to start running.  Must be > 0.
     *
     * @returns Amount to add to next action's duration, to account for time discretization over/underrun error.
     * @pre All states for entity at previous times have already been computed.
     * @sa execute_action
     * */
    void execute_walk_through_points_action
    (
        size_t time, 
        Action_context& context
    );

    /**
     * Handler for "follow" action. 
     *
     * @param time The time index to start running the action.  After returning, time will be updated to be the time index for the next action to start running.  Must be > 0.
     *
     * @param entity_1: the actor that is being followed 
     * @param entity_2: the actor is following entity_1
     * @returns Amount to add to next action's duration, to account for time discretization over/underrun error.
     * @pre All states for entity at previous times have already been computed.
     * @sa execute_action
     * */
    void execute_follow_action
    (
        size_t time, 
        Action_context& context
    );

private:
    double round(double a)
    {
        return std::floor(a+0.5);
    }

    size_t duration_to_samples(double length)
    {
        return round(length * sampling_rate_) + 1; // add one for first state (bookends)
    }

private:
    std::vector<std::vector<std::vector<Entity_state> > > entity_states_;

    double sampling_rate_;
    double sampling_period_;
};


} // namespace psi
} // namespace kjb

#endif
