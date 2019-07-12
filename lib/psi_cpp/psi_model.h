/* $Id: psi_model.h 12743 2012-07-25 23:52:02Z jguan1 $ */
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

#ifndef PSI_VERSION_1_MODEL_H
#define PSI_VERSION_1_MODEL_H

#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector.h>

#include <boost/shared_ptr.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/assign.hpp>
#include <vector>
#include <typeinfo>

#include <psi_cpp/psi_action.h>
#include <psi_cpp/psi_start_state.h>
#include <psi_cpp/psi_weighted_box.h>
#include <people_tracking_cpp/pt_entity.h>

#ifdef KJB_HAVE_UA_CARTWHEEL
#include <Control/SimulationInterface.h>
#include <Control/ExtendedAction.h>
#include <Control/WrapperAction.h>
#include <Control/StartState.h>
#include <Control/BoxState.h>
#endif


namespace kjb
{
namespace psi
{


#ifdef KJB_HAVE_UA_CARTWHEEL
typedef boost::shared_ptr<CartWheel::ExtendedAction> Ex_action_ptr;

// this probably doesn't belong in model.h...
typedef boost::shared_ptr<CartWheel::SimulationInterface> InterfacePtr;
#endif

struct Model
{
    Model() : 
        start_state(pt::NUM_ENTITY_TYPES),
        entities_actions(pt::NUM_ENTITY_TYPES)
    { }

    std::vector<std::vector<Start_state> > start_state;
    std::vector<std::vector<std::vector<Action> > > entities_actions;

    size_t size() const;

    double get(size_t dimension) const;

    void set(size_t dimension, double value);

    Unit_type get_units(size_t dimension) const;


//    std::vector<std::vector<int> > correspondences;
};


#ifdef KJB_HAVE_UA_CARTWHEEL
// extract start state in a format compatible with 
// the Cartwheel SimulationInterface class.
std::vector<CartWheel::StartStatePtr> to_cw_start_state(const Model& m);

// extract box list in a format compatible with 
// the Cartwheel SimulationInterface class.
//std::vector<CartWheel::BoxStatePtr> to_cw_boxes(const Model& m);

// extract action list in a format compatible with 
// the Cartwheel SimulationInterface class.
std::vector<std::vector<CartWheel::ExtendedActionPtr> > to_cw_actions(const Model& m);
#endif

const std::vector<std::vector<Start_state> >& to_start_state(const Model& m);
const std::vector<Weighted_box> to_boxes(const Model& m);
const std::vector<std::vector<std::vector<Action> > >& to_actions(const Model& m);

std::ostream& operator<<(std::ostream& ost, const Model& m);
std::istream& operator>>(std::istream& ist, Model& m);
/**
 * Currently an improper uniform distribution, with zero probability
 * for durations < 0.
 */
double model_prior (const Model& m);

//class Frustum_prior
//{
//public:
//    Frustum_prior(const kjb::Perspective_camera& cam, double width, double height)
//    {
//        // find out horizon line
//        kjb::Matrix M = build_camera_matrix();
//
//        // get three vanishing points
//        kjb::Vector p1 = M * kjb::Vector(1, 0, 0, 0);
//        kjb::Vector p2 = M * kjb::Vector(0, 0, 1, 0);
//        kjb::Vector p3 = M * kjb::Vector(1, 0, 1, 0);
//
//        // pick the two most numerically stable points
//
//        // order by decreasing 'w' parameter
//        if(p1[1] < p2[1])
//            swap(p1, p2);
//        if(p2[1] < p3[2])
//            swap(p2, p3);
//
//        // only use two points with highest 'w' parameter
//        // (hopefully the most numerically stable)
//        
//        kjb::Vector dir = (p2 - p1);
//        dir.normalize();
//
//        // find intersection with left side of screen
//        double t_left = (-width/2 - p1[0]) / dir[0];
//        kjb::Vector left_horizon = p1 + t_left * dir;
//        // find intersection with right side of screen
//        double t_right = (width/2 - p1[0]) / dir[0];
//        kjb::Vector right_horizon = p1 + t_right * dir;
//
//        // TODO: Plot these points to test it
//
//        // convert 2d points to 3d direction vector
//
//
//        // find bounding area on ground plane
//        // intersect view vector with bottom-left and bottom-right of screen
//        kjb::Vector view_bottom_left
//
//
//
//
//
//    }
//};

} // namespace psi
} // namespace kjb
#endif
