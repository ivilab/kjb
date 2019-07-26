/* $Id: psi_cylinder_world.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "psi_cpp/psi_cylinder_world.h"
#include "psi_cpp/psi_model.h"
#include "people_tracking_cpp/pt_entity.h"

#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_primitive.h"

#include <boost/assign/list_of.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/topological_sort.hpp>

#include <istream>
#include <map>

namespace kjb
{
namespace psi
{

/**
 * @param returns false if t lies outside of the domain [t_0, t_1), where t_0 and t_1 are the first and last timestamps of the trajectory, respectively.
 */
static bool lookup_piecewise_trajectory(const std::vector<double>& parameters, double t, double cur_x, double cur_z, double& x, double& z, double& theta)
{
    size_t i = 0;
    double next_time = -DBL_MAX;
    size_t num_waypoints = get_num_waypoints(parameters.size());
    for(i = 0; i < num_waypoints; i++)
    {
        next_time = parameters[to_time_index(i)];
        if(next_time > t) break;
    }

    if(i == num_waypoints) return false;

    double prev_x;
    double prev_z;
    double prev_time;

    if(i == 0)
    {
        prev_x = cur_x;
        prev_z = cur_z;
        prev_time = 0;
    }
    else
    {
        prev_x = parameters[to_x_index(i-1)];
        prev_z = parameters[to_z_index(i-1)];
        prev_time = parameters[to_time_index(i-1)];
    }

    double next_x = parameters[to_x_index(i)];
    double next_z = parameters[to_z_index(i)];

    double dt = next_time - prev_time;
    ASSERT(dt >= 0); // times must be monotonically increasing
    // convert to [0, 1)
    t = (t - prev_time) / dt; 

    x = (1-t) * prev_x + t * next_x;
    z = (1-t) * prev_z + t * next_z;

    // get heading from current segment.  If segment has zero length,
    // look at previous segments.
    double dx = next_x - prev_x;
    double dz = next_z - prev_z;
    while(fabs(dx) < FLT_EPSILON &&
          fabs(dz) < FLT_EPSILON)
    {
        if(i == 1) break;
        i--;
        dx = next_x - parameters[to_x_index(i-1)];
        dz = next_z - parameters[to_z_index(i-1)];
    }

    if(fabs(dx) < FLT_EPSILON &&
       fabs(dz) < FLT_EPSILON)
    {
        theta = 0;
    }
    else
    {
        theta = atan2(dx, dz);
    }

    return true;
}
const double AVG_HUMAN_HEIGHT = 1.778; // 5'10"
const double AVG_HUMAN_WIDTH= 0.61; //  ~ 2 ft

Entity_state::Entity_state() : 
        pos_(0.0, 0.0),
        heading_(0.0),
        dir_(0.0, 1.0),
        height_(AVG_HUMAN_HEIGHT),
        width_(AVG_HUMAN_WIDTH)
{}

void Entity_state::render() const
{
    using namespace kjb::opengl;
    using kjb::Vector;

    static const bool ENABLE_LIGHTING = 0;
#ifdef KJB_HAVE_OPENGL
    glPushMatrix();
    glTranslated(pos_[0], 0.0, pos_[1]);
//    glRotated(heading_, 0.0, 1.0, 0.0);

    if(ENABLE_LIGHTING)
    {
        glPushAttrib(GL_ENABLE_BIT);

        glEnable(GL_NORMALIZE);
    }

    // show heading with an arrow
    Vector arrow_position(0.0, height_, 0.0);

    std::cout << "dir: " << get_direction_3d() << std::endl;
    Vector arrow_direction(get_direction_3d() * 2 * width_);
    double arrow_width = width_/5.0;

    if(ENABLE_LIGHTING)
    {
        glEnable(GL_LIGHTING);
    }

    Arrow3d(arrow_position, arrow_direction, arrow_width).wire_render();

    // stretch cylinder to size

    // align with y-axis
    glRotated(-90, 1.0, 0.0, 0.0);

    // radius 1, height 1
    ::kjb::opengl::Cylinder c(width_/2.0, width_/2.0, height_);
    c.wire_render();

    if(ENABLE_LIGHTING)
    {
        glDisable(GL_LIGHTING);
        glPopAttrib();
    }

    glPopMatrix();

    GL_ETX();
#endif
}

// TODO: split everything into vector<vector> > where everything is indexed first by type`
bool Simple_simulator::simulate(
        const std::vector<std::vector<Start_state> >& start_state,
//        const std::vector<Weighted_box>& boxes,
        const std::vector<std::vector<std::vector<Action> > >& entity_actions
        ) 
{
    const Action NULL_ACTION = make_null_action(0);

    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {
        if(start_state[type].size() != entity_actions[type].size())
        {
            KJB_THROW_2(kjb::Illegal_argument, "start_state and action count do not match.");
        }
    }


    double length = 0;
    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {
        for(size_t i = 0; i < entity_actions[type].size(); i++)
        {
            length = std::max(length, get_actions_length(entity_actions[type][i]));
        }
    }

    size_t num_samples = duration_to_samples(length);

    entity_states_.clear();
    entity_states_.resize(pt::NUM_ENTITY_TYPES);

    // allocated 3d array of action contexts
    std::vector<std::vector<std::vector<Action_context_ptr> > > action_sequence(pt::NUM_ENTITY_TYPES);

    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {
        const size_t num_entites = start_state[type].size();

        entity_states_[type].resize(num_entites, std::vector<Entity_state>(num_samples));
        action_sequence[type].resize(num_entites, std::vector<Action_context_ptr>(num_samples));


        // fill in action sequence
        // TODO: Simulate without constructing an action sequence.  Current approach won't scale
        for(size_t e = 0; e < num_entites; e++)
        {
            Entity_state entity;
            entity.set_position(kjb::Vector(start_state[type][e].x, start_state[type][e].y));
            entity.set_heading(start_state[type][e].theta);

            entity_states_[type][e][0] = entity;

            std::vector<size_t> action_durations = quantize_action_durations(entity_actions[type][e]);

            // for each entity, construct acction array
            size_t i = 0;
            size_t next_switch = 0;// action_durations[0]; // what time do we switch to the next action?
            Action_context_ptr context;

            for(size_t t = 0; t < num_samples; t++)
            {
                if(t >= next_switch)
                {

                    // if action sequence has ended, fill the rest with null actions.
                    if(i >= action_durations.size())
                    {
                        const Action_context_ptr null_action_context(new Action_context());
                        null_action_context->action_index = i;
                        null_action_context->entity_index = e;
                        null_action_context->entity_type  = type;
                        null_action_context->action = &NULL_ACTION;

                        std::fill(action_sequence[type][e].begin() + t, action_sequence[type][e].end(), null_action_context);
                        break;
                    }

                    next_switch += action_durations[i];

                    Action_context_ptr tmp(new Action_context);
                    context.swap(tmp);

                    context->action_index = i;
                    context->entity_index = e;
                    context->entity_type = type;
                    context->action = &entity_actions[type][e][i];

                    i++;
                }

                action_sequence[type][e][t] = context;
            }
        }
    }

    // simulate each time slice
    for(size_t t = 1; t < num_samples; t++)
    {
        typedef std::pair<int, int> Entity_id;

        using namespace boost;
          typedef adjacency_list<vecS, vecS, bidirectionalS, 
              property<vertex_color_t, default_color_type>
            > Graph;
        typedef graph_traits<Graph>::vertex_descriptor Vertex;

        // construct dependency graph
        typedef std::pair<size_t, size_t> Edge;

        std::vector<Edge> depends;

        std::map<Entity_id, size_t> vertex_indices;
        std::vector<Entity_id> vertices;
        int node_index = 0;
        for(size_t type = 0; type < pt::NUM_ENTITY_TYPES; type++)
        {
            size_t num_entites = action_sequence[type].size();

            for(size_t e = 0; e < num_entites; e++)
            {
                Entity_id cur_entity(type, e);
                vertices.push_back(cur_entity);
                vertex_indices[cur_entity] = node_index;
                node_index++;
            }
        }

        for(size_t type = 0; type < pt::NUM_ENTITY_TYPES; type++)
        {
            size_t num_entites = action_sequence[type].size();

            for(size_t e = 0; e < num_entites; e++)
            {
                Entity_id cur_entity(type, e);
                int parent_type  = action_sequence[type][e][t]->action->parent_type;
                int parent_index = action_sequence[type][e][t]->action->parent_index;


                if( parent_index >= 0 )
                {
                    Entity_id parent_entity(parent_type, parent_index);
                    size_t cur_vertex_index = vertex_indices[cur_entity];
                    size_t parent_vertex_index = vertex_indices[parent_entity];
                    depends.push_back(Edge(parent_vertex_index, cur_vertex_index));
                }

            }
        }

        Graph g(depends.begin(), depends.end(), vertices.size());

        // order by dependencies
        std::vector<Vertex> order(vertices.size());
        try 
        {
            boost::topological_sort(g, order.begin());
        }
        catch(boost::not_a_dag& ex)
        {
            // illegal activity sequence
            return false;
        }

        // simulate in dependency order
        for(std::vector<Vertex>::reverse_iterator it = order.rbegin(); it < order.rend(); it++)
        {
            size_t node_i = *it;
            Entity_id entity_id = vertices[node_i];
            size_t entity_type = entity_id.first;
            size_t entity_index = entity_id.second;

            // use action at t-1, because previous action results in current state.
            // Also because t=0 is given by start_state, so no activity is responsible for it
            execute_action(t, *action_sequence.at(entity_type)[entity_index][t-1]);
        }
    }
    return true;
}

//void Simple_simulator::pad_actions(std::vector<Action>& actions, double duration) const
//{
//    double cur_duration = 0;
//    for(size_t a = 0; a < actions.size(); a++)
//    {
//        ASSERT(actions[a].parameters.size() > 0);
//        
//        //cur_duration += actions[a].parameters[0];
//        cur_duration += get_action_duration(actions[a]);
//    }
//
//    Action null;
//    null.type = NULL_ACTION;
//    null.parameters.resize(1, duration - cur_duration);
//    ASSERT(null.parameters[0] >= 0);
//
//    actions.push_back(null);
//}


double Simple_simulator::get_actions_length(const std::vector<Action>& actions)
{
    double duration = 0;

    for(size_t i = 0; i < actions.size(); i++)
    {
        // all actions should have first parameter as duration.
        // This is no longer the case for WALK_THROUGH_POINTS_ACTION
        //size_t DURATION_INDEX = 0;
        //ASSERT(actions[i].parameters.size() > 0);
        //ASSERT(actions[i].parameters[DURATION_INDEX] >= 0);
        //duration += actions[i].parameters[0];
        double action_duration = get_action_duration(actions[i]);
        ASSERT(action_duration > 0.0);
        duration += action_duration;
    }

    return duration;
}

std::vector<size_t> Simple_simulator::quantize_action_durations(const std::vector<Action>& actions)
{
    std::vector<size_t> result;
    result.reserve(25);

    double cur_time = 0;

    size_t prev_end = 0;

    for(size_t a = 0; a < actions.size(); a++)
    {
        // assume first element is always duration
        ASSERT(actions[a].parameters.size() > 0);

        //double cur_duration = actions[a].parameters[0];
        const Action& action = actions[a];
//        const Action_type action_type = action.type;
        //double cur_duration = action_descriptors[action_type].get_duration(action);
        double cur_duration = get_action_duration(action);

        cur_time += cur_duration;

        size_t cur_end = round(cur_time / sampling_period_);

        size_t step_duration = cur_end - prev_end;
        result.push_back(step_duration);

        prev_end = cur_end;
    }

    return result;
}

void Simple_simulator::execute_null_action(
        size_t time,
        Action_context& context)
{
    ASSERT(time > 0);

    size_t entity = context.entity_index;
    size_t type = context.entity_type;
//    const std::vector<double>& parameters = context.action->parameters;

    entity_states_[type][entity][time] = entity_states_[type][entity][time - 1];
}


void Simple_simulator::execute_walk_action(
        size_t time,
        Action_context& context)
{
    ASSERT(time > 0);

    size_t entity = context.entity_index;
    size_t type = context.entity_type;

    const std::vector<double>& parameters = context.action->parameters;

    double speed = parameters[1];

    entity_states_[type][entity][time] = entity_states_[type][entity][time-1];

    entity_states_[type][entity][time].move_forward(speed * sampling_period_);
}

void Simple_simulator::execute_walk_in_arc_action
(
    size_t time, 
    Action_context& context
)
{
    ASSERT(time > 0);

    size_t entity = context.entity_index;
    size_t type = context.entity_type;

    const std::vector<double>& parameters = context.action->parameters;

    double speed = parameters[1];
    double angular_speed = parameters[2];

    Entity_state* prev_entity_state = &entity_states_[type][entity][time-1];
    Entity_state* cur_entity_state = &entity_states_[type][entity][time];
    *cur_entity_state = *prev_entity_state;
    
    double old_heading = prev_entity_state->get_heading();
    double new_heading = old_heading+angular_speed * sampling_period_; 
    // Update the direction
    cur_entity_state->set_heading(new_heading);

    cur_entity_state->move_forward(speed * sampling_period_);
}


void Simple_simulator::execute_walk_through_points_action
(
    size_t time, 
    Action_context& context
)
{
    ASSERT(time > 0);

    size_t entity = context.entity_index;
    size_t type = context.entity_type;

    const std::vector<double>& parameters = context.action->parameters;
    std::vector<double>& memory = context.memory;
    if(memory.size() == 0)
    {
        memory.resize(3);
        memory[0] = time; // set start time of the activity
        memory[1] = entity_states_[type][entity][time-1].get_position()[0];
        memory[2] = entity_states_[type][entity][time-1].get_position()[1];
    }

    size_t start_time = memory[0];
    double cur_x = memory[1];
    double cur_z = memory[2];

    double x, z, theta;

    size_t rel_time = time - start_time;
    
    // TODO: this will fail when the duration is rounded up to the next timestep, because 
    // the requested time is outside the domain of the trajectory.  Need to handle this
    // UPDATE: this doesn't seem to be happening, and I can't figure out why.  kls, July 29, 2011
    ASSERT(lookup_piecewise_trajectory(parameters, rel_time * sampling_period_, cur_x, cur_z, x, z, theta));

    Entity_state* cur_entity_state = &entity_states_[type][entity][time];
    cur_entity_state->set_position(Vector(x, z));
    cur_entity_state->set_heading(theta);
}


void Simple_simulator::execute_follow_action
(
    size_t time, 
    Action_context& context
)
{
    ASSERT(time > 0);

    size_t entity = context.entity_index;
    size_t type = context.entity_type;

    const std::vector<double>& parameters = context.action->parameters;
//    std::vector<double>& memory = context.memory;
    
    size_t parent_type = context.action->parent_type;
    size_t parent_index = context.action->parent_index;

    // check the parent type and parent_index 
    size_t num_types = entity_states_.size();
    if(parent_type > num_types)
    {
        KJB_THROW_2(kjb::Runtime_error, "Parent type is out of bound!");
    }

    size_t num_entities = entity_states_[parent_type].size(); 
    if(parent_index > num_entities)
    {
        KJB_THROW_2(kjb::Runtime_error, "Parent index is out of bound!");
    }

    double time_behind = parameters[1];
    size_t time_behind_q = round(time_behind/sampling_period_);
    
    Entity_state* cur_walker_state = &entity_states_[type][entity][time];

    // Get the state that the walker-to-be-followed at time (time-time_behind)
    // Assume the state has already been updated
    double prev_tbf_time = time - time_behind_q;
    if(prev_tbf_time < 0.0)
    {
        // stand still
    }
    else 
    {
        Entity_state* cur_walker_tbf_state = 
                            &entity_states_[parent_type][parent_index][static_cast<size_t>(prev_tbf_time)];
        const kjb::Vector& tbf_position = cur_walker_tbf_state->get_position();
        double tbf_heading = cur_walker_tbf_state->get_heading();
        cur_walker_state->set_heading(tbf_heading);
        cur_walker_state->set_position(tbf_position);
    }
            
}

} // namespace psi
} // namespace kjb
