/* $Id: psi_model.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "people_tracking_cpp/pt_entity.h"
#include "psi_cpp/psi_model.h"
#include "psi_cpp/psi_util.h"
#include "psi_cpp/psi_action.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/io/ios_state.hpp>

#ifdef KJB_HAVE_UA_CARTWHEEL
#include <MathLib/Vector3d.h>
#endif

namespace kjb
{
namespace psi
{

// prototypes
std::istream& read_model(std::istream& ist, size_t version, Model& m);
std::istream& read_model_v0(std::istream& ist, Model& m);
std::istream& read_model_v1(std::istream& ist, Model& m);
std::istream& read_model_v2(std::istream& ist, Model& m);
std::istream& read_model_v3(std::istream& ist, Model& m);
std::istream& read_model_v4(std::istream& ist, Model& m);

// adapter between cartwheel and kjb units.
// currently the same since I implemented them both ;-D
#ifdef KJB_HAVE_UA_CARTWHEEL
Unit_type get_extended_action_units(const CartWheel::ExtendedAction& action, size_t i)
{
    CartWheel::UnitType unit = action.getParamUnits(i);

    switch(unit)
    {
        case CartWheel::UNKNOWN_UNIT:
            return UNKNOWN_UNIT;
        case CartWheel::SPACIAL_UNIT:
            return SPACIAL_UNIT;
        case CartWheel::VSPACIAL_UNIT:
            return VSPACIAL_UNIT;
        case CartWheel::ASPACIAL_UNIT:
            return ASPACIAL_UNIT;
        case CartWheel::ANGLE_UNIT:
            return ANGLE_UNIT;
        case CartWheel::VANGLE_UNIT:
            return VANGLE_UNIT;
        case CartWheel::AANGLE_UNIT:
            return AANGLE_UNIT;
        case CartWheel::TIME_UNIT:
            return TIME_UNIT;
        case CartWheel::OTHER_UNIT:
            return OTHER_UNIT;
        default:
            KJB_THROW_2(kjb::Runtime_error, "Unknown Cartwheel unit type");
    }
}
#endif

Unit_type Model::get_units(size_t dimension) const
{
    ASSERT(start_state.size() == pt::NUM_ENTITY_TYPES && 
           entities_actions.size() == pt::NUM_ENTITY_TYPES);

    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {

        ASSERT(start_state[type].size() == entities_actions[type].size());

        const size_t num_entities = start_state[type].size();
        const size_t params_per_entity = Start_state::size();
        // start state
        if(dimension < num_entities * params_per_entity)
        {
            size_t param_i = dimension % params_per_entity;
            return Start_state::get_units(param_i);
        }

        dimension -= num_entities * params_per_entity;

        // loop over actions
        for(size_t entity = 0; entity < num_entities; entity++)
        {
            const std::vector<Action>& actions = entities_actions[type][entity];
            for(size_t a = 0; a < actions.size(); a++)
            {
                const Action& action = actions[a];
                if(dimension < action.parameters.size())
                    return ::kjb::psi::get_units(action.type, dimension);
                dimension -= action.parameters.size();
            }
        }

    }

    KJB_THROW(kjb::Index_out_of_bounds);
}


double Model::get(size_t dimension) const
{
    ASSERT(start_state.size() == pt::NUM_ENTITY_TYPES && 
           entities_actions.size() == pt::NUM_ENTITY_TYPES);

    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {
        ASSERT(start_state.size() == entities_actions.size());

        const size_t num_entities = start_state[type].size();
        const size_t params_per_entity = Start_state::size();

        if(dimension < num_entities * params_per_entity)
        {
            size_t entity_i = dimension / params_per_entity;
            size_t param_i = dimension % params_per_entity;
            return start_state[type][entity_i][param_i];
        }

        // dimension doesn't refer to a start state parameter.
        // Moving on to action parameters...
        dimension -= num_entities * params_per_entity;

        for(size_t entity_i = 0; entity_i < num_entities; entity_i++)
        {
            const std::vector<Action>& actions = entities_actions[type][entity_i];
            const size_t num_actions = actions.size();

            for(size_t action_i = 0; action_i < num_actions; action_i++)
            {
                const Action& action = actions[action_i];
                if(dimension < action.parameters.size())
                {
                    return action.parameters[dimension];
                }

                dimension -= action.parameters.size();
            }

        }
    }

    KJB_THROW(kjb::Index_out_of_bounds);
}

void Model::set(size_t dimension, double value) 
{
    ASSERT(start_state.size() == pt::NUM_ENTITY_TYPES && 
           entities_actions.size() == pt::NUM_ENTITY_TYPES);

    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {
        ASSERT(start_state[type].size() == entities_actions[type].size());

        const size_t num_entities = start_state[type].size();
        const size_t params_per_entity = Start_state::size();

        if(dimension < num_entities * params_per_entity)
        {
            size_t entity_i = dimension / params_per_entity;
            size_t param_i = dimension % params_per_entity;
            start_state[type][entity_i][param_i] = value;
            return;
        }

        // dimension doesn't refer to a start state parameter.
        // Moving on to action parameters...
        dimension -= num_entities * params_per_entity;

        for(size_t entity_i = 0; entity_i < num_entities; entity_i++)
        {
            std::vector<Action>& actions = entities_actions[type][entity_i];
            const size_t num_actions = actions.size();

            for(size_t action_i = 0; action_i < num_actions; action_i++)
            {
                Action& action = actions[action_i];
                if(dimension < action.parameters.size())
                {
                    action.parameters[dimension] = value;
                    return;
                }

                dimension -= action.parameters.size();
            }
        }
    }

    KJB_THROW(kjb::Index_out_of_bounds);
}

size_t Model::size() const
{
    ASSERT(start_state.size() == pt::NUM_ENTITY_TYPES && 
           entities_actions.size() == pt::NUM_ENTITY_TYPES);

    size_t size = 0;
    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {
        ASSERT(start_state[type].size() == entities_actions[type].size());

        size += start_state[type].size() * Start_state::size();


        for(size_t entity_i = 0; entity_i < entities_actions[type].size(); entity_i++)
        {
            const std::vector<Action>& actions = entities_actions[type][entity_i];
            for(size_t i = 0; i < actions.size(); i++)
            {
                size += actions[i].parameters.size();
            }
        }
    }

    return size;
}

std::ostream& operator<<(std::ostream& ost, const Model& m)
{
    // IMPORTANT: increment this every time you change the file format.  Also, update the read function to accomodate this new version.
    const size_t FILE_FORMAT_VERSION = 4;
    ost << 'v' << FILE_FORMAT_VERSION << ' ';

    ost << ' ' << static_cast<int>(pt::NUM_ENTITY_TYPES);

    ASSERT(m.entities_actions.size() == pt::NUM_ENTITY_TYPES);
    ASSERT(m.start_state.size() == pt::NUM_ENTITY_TYPES);

    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {
        ost << ' ' << m.start_state[type].size();
        for(size_t i = 0; i < m.start_state[type].size(); i++)
        {
            ost << ' ' << m.start_state[type][i];
        }

        for(size_t entity = 0; entity < m.entities_actions[type].size(); entity++)
        {
            const std::vector<Action>& actions = m.entities_actions[type][entity];
            ost << " " << actions.size();

            for(size_t i = 0; i < actions.size(); i++)
            {
                ost << " " << actions[i];
            }
        }
    }

    return ost;
}

std::istream& operator>>(std::istream& ist, Model& m)
{
    using std::istream;

    // this will restore the "skipws" state after leaving scope
    boost::io::ios_flags_saver  ifs( ist );
    boost::io::ios_exception_saver  ixs( ist );
    ist.exceptions(istream::failbit | istream::badbit );

    try{
        ist >> std::skipws;

        char v = ist.peek();
        size_t version;



        if(v != 'v')
        {
            version = 0;
        }
        else
        {
            ist >> v;
            ist >> version;
        }

        return read_model(ist, version, m);
    }
    catch(std::exception& e)
    {
        KJB_THROW_2(IO_error, "Invalid file format.");
    }
}


std::istream& read_model(std::istream& ist, size_t version, Model& m)
{
    if(version == 0)
    {
        return read_model_v0(ist, m);
    }

    if(version == 1)
    {
        return read_model_v1(ist, m);
    }

    if(version == 2)
    {
        return read_model_v2(ist, m);
    }

    if(version == 3)
    {
        return read_model_v3(ist, m);
    }

    if(version == 4)
    {
        return read_model_v4(ist, m);
    }

    KJB_THROW_2(kjb::Runtime_error, "Unknown file format version");
}


std::istream& read_model_v0(std::istream& ist, Model& m)
{
    m.start_state.resize(pt::NUM_ENTITY_TYPES);
    m.entities_actions.resize(pt::NUM_ENTITY_TYPES);

    m.start_state[pt::PERSON_ENTITY].resize(2);
    ist >> m.start_state[pt::PERSON_ENTITY][0].x;
    ist >> m.start_state[pt::PERSON_ENTITY][0].y;
    ist >> m.start_state[pt::PERSON_ENTITY][0].theta;

    ist >> m.start_state[pt::PERSON_ENTITY][1].x;
    ist >> m.start_state[pt::PERSON_ENTITY][1].y;
    ist >> m.start_state[pt::PERSON_ENTITY][1].theta;

    // early model only allowed a single entity
    m.entities_actions[pt::PERSON_ENTITY].resize(1);

    while(ist.peek() != EOF && ist.peek() != '\n')
    {
        Action a;
        ist >> a;

        m.entities_actions[pt::PERSON_ENTITY][0].push_back(a);
    }

    return ist;
}

std::istream& read_model_v1(std::istream& ist, Model& m)
{
    m.start_state.resize(pt::NUM_ENTITY_TYPES);
    m.entities_actions.resize(pt::NUM_ENTITY_TYPES);

    size_t num_entities;
    ist >> num_entities;

    m.start_state.resize(num_entities);

    for(size_t i = 0; i < num_entities; i++)
    {
        ist >> m.start_state[pt::PERSON_ENTITY][i].x;
        ist >> m.start_state[pt::PERSON_ENTITY][i].y;
        ist >> m.start_state[pt::PERSON_ENTITY][i].theta;
    }

    // early models only allowed one entity, so 
    // vector length is hard-coded to 1.
    m.entities_actions[pt::PERSON_ENTITY].resize(1);
    std::vector<Action>& actions = m.entities_actions[pt::PERSON_ENTITY][0];

    size_t num_actions;
    ist >> num_actions;

    for(size_t i = 0; i < num_actions; i++)
    {
        Action a;
        ist >> a;

        actions.push_back(a);
    }

    return ist;
}

std::istream& read_model_v2(std::istream& ist, Model& m)
{
    m.start_state.resize(pt::NUM_ENTITY_TYPES);
    m.entities_actions.resize(pt::NUM_ENTITY_TYPES);

    size_t num_entities;
    ist >> num_entities;

    m.start_state[pt::PERSON_ENTITY].resize(num_entities);
    m.entities_actions[pt::PERSON_ENTITY].resize(num_entities);

    for(size_t i = 0; i < num_entities; i++)
    {
        ist >> m.start_state[pt::PERSON_ENTITY][i];
    }

    for(size_t entity = 0; entity < num_entities; entity++)
    {
        size_t num_actions;
        ist >> num_actions;

        for(size_t i = 0; i < num_actions; i++)
        {
            Action a;
            ist >> a;

            m.entities_actions[pt::PERSON_ENTITY][entity].push_back(a);
        }

    }

    return ist;
}


std::istream& read_model_v3(std::istream& ist, Model& m)
{
    m.start_state.resize(pt::NUM_ENTITY_TYPES);
    m.entities_actions.resize(pt::NUM_ENTITY_TYPES);

    size_t num_entities;
    ist >> num_entities;

    m.start_state[pt::PERSON_ENTITY].resize(num_entities);
    m.entities_actions[pt::PERSON_ENTITY].resize(num_entities);

    for(size_t i = 0; i < num_entities; i++)
    {
        ist >> m.start_state[pt::PERSON_ENTITY][i];
    }

    for(size_t entity = 0; entity < num_entities; entity++)
    {
        size_t num_actions;
        ist >> num_actions;

        for(size_t i = 0; i < num_actions; i++)
        {
            Action a;
            ist >> a;

            m.entities_actions[pt::PERSON_ENTITY][entity].push_back(a);
        }

    }

    size_t num_boxes;
    ist >> num_boxes;
// Boxes are ignored for now
//
//    for(size_t i = 0; i < num_boxes; i++)
//    {
//        Weighted_box b;
//        ist >> b;
//        m.boxes.push_back(b);
//    }

    return ist;
}

std::istream& read_model_v4(std::istream& ist, Model& m)
{
    size_t num_entity_types;
    ist >> num_entity_types;

    if(num_entity_types > pt::NUM_ENTITY_TYPES)
    {
        KJB_THROW_2(IO_error, "Invalid file format: num_entity_types exceeds number of known entities");
    }

    m.start_state.resize(num_entity_types);
    m.entities_actions.resize(num_entity_types);

    ASSERT(m.entities_actions.size() == pt::NUM_ENTITY_TYPES);
    ASSERT(m.start_state.size() == pt::NUM_ENTITY_TYPES);

    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {

        size_t num_entities;

//        std::string delete_me;
//        ist >> delete_me;
//        abort();
        ist >> num_entities;

        m.start_state[type].resize(num_entities);
        m.entities_actions[type].resize(num_entities);

        for(size_t i = 0; i < num_entities; i++)
        {
            ist >> m.start_state[type][i];
        }

        for(size_t entity = 0; entity < num_entities; entity++)
        {
            size_t num_actions;
            ist >> num_actions;

            for(size_t i = 0; i < num_actions; i++)
            {
                Action a;
                ist >> a;

                m.entities_actions[type][entity].push_back(a);
            }

        }

//        size_t num_boxes;
//        ist >> num_boxes;
//
//        for(size_t i = 0; i < num_boxes; i++)
//        {
//            Weighted_box b;
//            ist >> b;
//            m.boxes.push_back(b);
//        }
    }

    return ist;
}



#ifdef KJB_HAVE_UA_CARTWHEEL
std::vector<CartWheel::StartStatePtr> to_cw_start_state(const Model& m)
{
    int id = 1;
    std::vector<CartWheel::StartStatePtr> start_state;

    // massage the model into something that works with the siulator interface
    BOOST_FOREACH(Start_state s, m.start_state[pt::PERSON_ENTITY])
    {
        std::string name = "Human" + boost::lexical_cast<std::string>((id++));
        CartWheel::StartStatePtr start(
            new CartWheel::StartState(name,
                        s.x,
                        s.y,
                        s.theta));
        start_state.push_back(start);
    }

    return start_state;
}
#endif

#ifdef KJB_HAVE_UA_CARTWHEEL
/**
 * @param box The box to convert to cartwheel format
 * @param i The index of the box (indexes must be unique)
 */
CartWheel::BoxStatePtr to_cw(const Weighted_box& box, size_t i)
{
    using namespace CartWheel;
    std::string name = "Box" + boost::lexical_cast<std::string>(i);
    // TODO: does box need to be slightly above the ground?
    Math::Vector3d position = to_cw_vec_3d(box.get_center());
    Math::Vector3d size = to_cw_vec_3d(box.get_size());
    double mass = box.get_mass();
    double rotation = box.get_orientation().get_angle();
    Vector axis = box.get_orientation().get_axis();
    if(axis[1] < 0)
        rotation = -rotation;

    BoxStatePtr result(new BoxState(name, position, rotation, size, mass));
    return result;
}
#endif

#ifdef KJB_HAVE_UA_CARTWHEEL
#if 0
// no longer support boxes explicityl
std::vector<CartWheel::BoxStatePtr> to_cw_boxes(const Model& m)
{
    std::vector<CartWheel::BoxStatePtr> boxes;

    size_t id=1;
    BOOST_FOREACH(const Weighted_box& box, m.boxes)
    {
        boxes.push_back(to_cw(box, id++));
    }

    return boxes;
}
#endif
#endif

const std::vector<std::vector<Start_state> >& to_start_state(const Model& m)
{
// currently have the same representation
    return m.start_state;
}

#if 0
// not supported
const std::vector<Weighted_box> to_boxes(const Model& m)
{
    return m.boxes;
}
#endif


/// Construct cartwheel actions from
#ifdef KJB_HAVE_UA_CARTWHEEL
std::vector<std::vector<CartWheel::ExtendedActionPtr> > to_cw_actions(const Model& m)
{
    using namespace CartWheel;

    int id = 1;
    std::vector<std::vector<ExtendedActionPtr> > cw_actors_actions;

    BOOST_FOREACH(const std::vector<Action>& actions, m.entities_actions[pt::PERSON_ENTITY])
    {
        std::vector<ExtendedActionPtr> cw_actions;
        std::string name = "Human" + boost::lexical_cast<std::string>((id++));

        BOOST_FOREACH(const Action& a, actions)
        {
            switch(a.type)
            {
                case NULL_ACTION:
                {
                    using CartWheel::WrapperAction;

                    // for now, "do nothing" means "walk at speed zero"
                    ExtendedActionPtr tmp(new WrapperAction(std::string("standStill"), name, a.parameters));
                    cw_actions.push_back(tmp);
                    break;
                }
                case WALK_ACTION:
                {
                    using CartWheel::WrapperAction;

                    ExtendedActionPtr tmp(new WrapperAction(std::string("walk"), name, a.parameters));
                    cw_actions.push_back(tmp);
                    break;
                }
                case WALK_IN_ARC_ACTION:
                {
                    using CartWheel::WrapperAction;
                    
                    ExtendedActionPtr tmp(new WrapperAction(std::string("walkTurn"), name, a.parameters));
                    cw_actions.push_back(tmp);
                    break;
                }
                case WALK_THROUGH_POINTS_ACTION:
                {
                    using CartWheel::WrapperAction;
                    //@TODO: need to be implemented...
                    break;
                }
                case FOLLOW_ACTION:
                {
                    using CartWheel::WrapperAction;
                    //@TODO: need to be implemented...
                    break;
                }
                default:
                KJB_THROW_2(kjb::Illegal_argument, "Unknown action");
            }
        }

        cw_actors_actions.push_back(cw_actions);
    }

    return cw_actors_actions;
}
#endif


const std::vector<std::vector<std::vector<Action> > >& to_actions(const Model& m)
{
    return m.entities_actions;
}


double model_prior (const Model& m)
{
    double prior = 0.0;

    // TODO: update for transition time instead of duration
    for(size_t actor = 0; actor < m.entities_actions[pt::PERSON_ENTITY].size(); actor++)
    {
        const std::vector<Action>& actions = m.entities_actions[pt::PERSON_ENTITY][actor];

        for(size_t i = 0; i < actions.size(); i++)
        {
            const size_t DURATION_INDEX = 0;
            const double duration = actions[i].parameters[DURATION_INDEX];
            ASSERT(duration == duration);  // !NaN
            if(duration < 0)
                prior = -std::numeric_limits<double>::infinity();


            // uniform over [-3, 3], with negative values having half
            // the probability of positive ones
            if(actions[i].type == WALK_ACTION || actions[i].type == WALK_IN_ARC_ACTION)
            {
                const size_t SPEED_INDEX = 1;
                const double speed = actions[i].parameters[SPEED_INDEX];
//                if(speed > 3 || speed < -3)
                if(speed > 3 || speed < 0)
                    prior = -std::numeric_limits<double>::infinity();
                else if(speed < 0)
                    prior += log(0.1);
                else
                {
                    ; // prior += log(1)
                }
            }
            else if(actions[i].type == WALK_IN_ARC_ACTION)
            {
                const size_t ANGULAR_SPEED_INDEX = 2;
                const double angular_speed = actions[i].parameters[ANGULAR_SPEED_INDEX];
                // angular_speed is uniform over [-M_PI_4, M_PI_4]
                if(angular_speed > M_PI_4 || angular_speed < -M_PI_4)
                {
                    prior = -std::numeric_limits<double>::infinity();
                }
            }
            else if(actions[i].type == FOLLOW_ACTION)
            {
                const size_t BEHIND_TIME_INDEX = 1;
                const double behind_time = actions[i].parameters[BEHIND_TIME_INDEX];
                // the follower's behind time is uniform over [0.1, 100] 
                // mabye should be a gaussian distribution ??? 
                if(behind_time > 100 || behind_time < 0)
                {
                    prior = -std::numeric_limits<double>::infinity();
                }
            }
        }
    }


    // log-normal distributions
//    kjb::Normal_distribution p_size(log(0.25), log(3));
//    kjb::Normal_distribution p_mass(log(1), log(20));
//    for(size_t i = 0; i < m.boxes.size(); i++)
//    {
//        const Weighted_box& box = m.boxes[i]; 
//        prior += log(kjb::pdf(p_size, log(box.get_size()[0])));
//        prior += log(kjb::pdf(p_size, log(box.get_size()[1])));
//        prior += log(kjb::pdf(p_size, log(box.get_size()[2])));
//        prior += log(kjb::pdf(p_mass, log(box.get_mass())));
//    }

    return prior;
}


} // namespace psi
} // namespace kjb
