/* $Id: turntable_camera.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 * =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "camera_cpp/turntable_camera.h"
#include "camera_cpp/calibrated_camera.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "g_cpp/g_quaternion.h"
#include "prob_cpp/prob_stat.h"
#include "l_cpp/l_algorithm.h"
#include "l_cpp/l_index.h"
#include "g_cpp/g_util.h"
#include "g2_cpp/g2_rotation.h"
#include "m_cpp/m_sequence.h"

#include <iterator>
#include <fstream>
#include <numeric>

namespace kjb {

Turntable_camera::Turntable_camera() :
        neutral_camera_(),
        turntable_origin_(),
        rotation_axis_(),
        angles_(),
        current_index_(),
        current_camera_()
{}

Turntable_camera::Turntable_camera(

    const Perspective_camera& cam,
    const Vector& rotation_axis,
    const Vector& turntable_origin,
    int num_positions,
    bool clockwise) :
        neutral_camera_(cam),
        turntable_origin_(turntable_origin),
        rotation_axis_(rotation_axis),
        angles_(Interval_sequence(
            0.0,
            (clockwise ? -1 : 1) * 2 * M_PI / num_positions,
            (clockwise ? -1 : 1) * 2 * M_PI).to_vector()),
        true_angles_(angles_),
        current_index_(0),
        current_camera_(neutral_camera_)

{}
Turntable_camera::Turntable_camera(const Turntable_camera& other) :
        neutral_camera_(other.neutral_camera_),
        turntable_origin_(other.turntable_origin_),
        rotation_axis_(other.rotation_axis_),
        angles_(other.angles_),
        true_angles_(other.true_angles_),
        current_index_(other.current_index_),
        current_camera_(other.current_camera_),
        indices_(other.indices_)
{}

std::ofstream& operator<<(std::ofstream& out, const Quaternion& q)
{
    out << q.get_euler_angles();
    return out;
}

void Turntable_camera::swap(Turntable_camera& other)
{
    using std::swap;
    neutral_camera_.swap(other.neutral_camera_);
    turntable_origin_.swap(other.turntable_origin_);
    rotation_axis_.swap(other.rotation_axis_);
    angles_.swap(other.angles_);
    true_angles_.swap(other.true_angles_);
    swap(current_index_, other.current_index_);
    current_camera_.swap(other.current_camera_);
    indices_.swap(other.indices_);
}

void Turntable_camera::set_active_camera(size_t i) const
{ 
    ASSERT(angles_.size() > 0);
    angles_.at(i); // trigger Index_out_of_bounds exception if necessarey
    current_index_ = i; 
    current_camera_ = neutral_camera_;

    Vector ccenter = neutral_camera_.get_camera_centre();
    ASSERT(ccenter[3] == 1.0);
    ccenter.resize(3);


    // rotate world origin around turntable axis.
    Rotation_axis axis(turntable_origin_, rotation_axis_);
    ccenter = axis.rotate(ccenter, angles_[i]);
    current_camera_.set_camera_centre(ccenter);

    // rotate camera orientation
    const Quaternion& orientation = neutral_camera_.get_orientation();
    Quaternion rotation(rotation_axis_, (angles_[i]));
    current_camera_.set_orientation(orientation * rotation.conj());

    current_camera_.update_rendering_interface();

}

void Turntable_camera::set_next()
{
    ASSERT(angles_.size() > 0);

    current_index_++;
    if(current_index_ == angles_.size())
        current_index_ = 0;

    set_active_camera(current_index_);
}

void Turntable_camera::set_previous()
{
    ASSERT(angles_.size() > 0);

    if(current_index_ == 0)
        current_index_ = angles_.size();

    current_index_--;

    set_active_camera(current_index_);
}

void Turntable_camera::set_index_list(const Index_range& indices)
{
    // special case - use all angles
    if(indices.all())
    {
        angles_ = true_angles_;
        indices_.resize(true_angles_.size());
        std::generate(
            indices_.begin(),
            indices_.end(),
            kjb::Increment<size_t>(0));

        return;
    }

    if(indices.size() == 0)
        KJB_THROW_2(Illegal_argument, "index list must contain at least one index.");

    angles_.clear();
    indices_.clear();
    for(size_t ii = 0; ii < indices.size(); ii++)
    {
        size_t i = indices[ii];

        angles_.push_back(true_angles_.at(i));
        indices_.push_back(i);
    }

    set_active_camera(0);

}

void Turntable_camera::evenly_space_cameras(size_t n)
{
    angles_.resize(n);
    indices_.clear();
    kjb::linspace(0, 2*M_PI, n, angles_.begin(), false);
    set_active_camera(0);
}

const Perspective_camera Turntable_camera::operator[](int i) const
{
    Perspective_camera result;

    int old_i = get_current_index();
    set_active_camera(i);
    result = get_current_camera();
    set_active_camera(old_i);

    return result;
}

std::vector<Perspective_camera> Turntable_camera::to_cameras() const
{
    std::vector<Perspective_camera> result(size());

    for(size_t i = 0; i < size(); i++)
    {
        result[i] = operator[](i);
    }

    return result;
}

/**
 * Take a set of individually calibrated cameras arranged in a turntable configuration and return the "best fit" Turntable camera.
 *
 * In reality, we don't minimize any metric, do it heuristically by finding mean camara parameters.
 *
 * @note Position of the cameras is assumed to be evenly spaced around the circle, that is, angle increment is 360 / camreas_.size()
 *
 * @TODO detect angles too.  for now, it assumes clockwise rotation.
 * */
Turntable_camera regularize_turntable_cameras(const std::vector<Calibrated_camera>& cameras_)
{

// EXTRACT ALL DIFFERENCES OF ROTATIONS BETWEEN CAMERAS.
    std::vector<Quaternion> rotations_;
    rotations_.reserve(cameras_.size() * cameras_.size() - 1);

    for(size_t i = 0; i < cameras_.size(); ++i)
    {
        const Calibrated_camera& cur_camera_1 = cameras_[i];
        for(size_t j = 0; j < cameras_.size(); ++j)
        {
            const Calibrated_camera& cur_camera_2 = cameras_[j];
            if(&cur_camera_1 == &cur_camera_2)
                continue;

            rotations_.push_back(kjb::difference(
                    cur_camera_1.get_orientation(),
                    cur_camera_2.get_orientation()));
        }
    }

    

// EXTRACT ALL AXES AND ANGLES
//
    std::vector<Vector> axes_(
            rotations_.size(),
            kjb::Vector(3));

    // get rotation axes
    transform(rotations_.begin(),
            rotations_.end(),
            axes_.begin(),
            std::mem_fun_ref(&kjb::Quaternion::get_axis));

    std::vector<double> angles_(
            rotations_.size(),
            0.0);

    // get rotation angles
    transform(rotations_.begin(),
            rotations_.end(),
            angles_.begin(),
            std::mem_fun_ref(&kjb::Quaternion::get_angle));

    // make sure axes all point in same direction
    const kjb::Vector& first = axes_[0];

    for(size_t i = 0; i < axes_.size(); ++i)
    {
        kjb::Vector& axis = axes_[i];
//            std::cout << axis << std::endl;
//            std::cout << first << std::endl;
//            std::cout << dot(axis, first) << std::endl;
        if(dot(axis, first) < 0.0)
        {
            axis *= -1;
            angles_[i] *= -1;
        }
    }

// FIND MEAN ROTATION AXIS
    Vector mean_axis_ = kjb::mean(axes_.begin(), axes_.end());
    mean_axis_.normalize();

//        double mean_angle_ = kjb::mean(angles_.begin(), angles_.end());


// EXTRACT ALL CAMERA TRANSLATIONS
    // find camera locations and compare
    std::vector<kjb::Vector> ccenters(cameras_.size());
    transform(cameras_.begin(),
            cameras_.end(),
            ccenters.begin(),
            std::mem_fun_ref(&kjb::Calibrated_camera::get_camera_centre));

    // convert to non-homogeneous coordinates
    transform(ccenters.begin(),
            ccenters.end(),
            ccenters.begin(),
            kjb::geometry::projective_to_euclidean);


    // find average camera position. (center of gravity)
    kjb::Vector mean_c = kjb::mean(ccenters.begin(), ccenters.end());

    // Find turntable origin:
    // find plane containing origin, in direction of mean axis
    // and find intersection of rotation axis with plane. 
    // see: http://local.wasp.uwa.edu.au/~pbourke/geometry/planeline/
    const kjb::Vector& N = mean_axis_;
    const kjb::Vector  P3(0.0,0.0,0.0);
    const kjb::Vector& P1 = mean_c.resize(3);
    const kjb::Vector& P2 = mean_c + mean_axis_;

    double u = dot(N, P3 - P1) / dot(N, P2 - P1);
    Vector turntable_origin_ = P1 + mean_axis_ * u;

    //line:  mean_c + t * mean_axis

    double ANGLE_INCREMENT = 2 * M_PI / cameras_.size();

    Interval_sequence turntable_angles(0, ANGLE_INCREMENT, 2 * M_PI);
// REMOVE TURNTABLE ROTATION FROM CAMERA POSITIONS, TO FIND THE MEAN "NEUTRAL" CAMERA POSITION
    for(size_t i = 0; i < ccenters.size(); ++i)
    {
        kjb::Vector& ccenter = ccenters[i];
        kjb::Rotation_axis r(turntable_origin_, mean_axis_);
        ccenter = r.rotate(ccenter, -turntable_angles[i]);
    }



    // find mean location
    Vector mean_camera_position_ = mean(ccenters.begin(), ccenters.end());
    // translate to be relative to turntable origin
//        mean_camera_position_ -= turntable_origin_;




// GET GENERIC CAMERA ORIENTATION
    std::vector<kjb::Quaternion> camera_orientations(cameras_.size());
    transform(cameras_.begin(),
            cameras_.end(),
            camera_orientations.begin(),
            std::mem_fun_ref(&kjb::Calibrated_camera::get_orientation));

    // remove part contributed by turntable
    for(size_t i = 0; i < camera_orientations.size(); ++i)
    {
        kjb::Quaternion& q = camera_orientations[i];
        kjb::Quaternion turntable_unrotation(mean_axis_, -turntable_angles[i]);
        // notice that the order is reversed and so is the angle.  We want to undo the rotation in the
        // _camera_.  This the opposite of undoing the rotation in a world point.  Analogy: translating world
        // points vs. translating camera.  Transformations are inverted and applied in reverse.
        q = q * turntable_unrotation.conj() ;
    }

    Quaternion mean_orientation = kjb::mean(
            camera_orientations.begin(),
            camera_orientations.end());

    double mean_focal_length_x = 0;
    double mean_focal_length_y = 0;
    Vector mean_principal_point(2, 0.0);
    double mean_skew = 0;

    for(size_t i = 0; i < cameras_.size(); i++)
    {
        double cur_focal_length_x = cameras_[i].get_focal_length();
        double cur_focal_length_y = cur_focal_length_x * cameras_[i].get_aspect_ratio();
        Vector cur_principal_point = cameras_[i].get_principal_point();
        double cur_skew = cameras_[i].get_skew();

        // the signs should all be the same.  check that here
        // (candidate for eventual deletion)
        if(i > 0)
        {
            ASSERT((cur_focal_length_x > 0) == (mean_focal_length_x > 0));
            ASSERT((cur_focal_length_y > 0) == (mean_focal_length_y > 0));
        }

        mean_focal_length_x += cur_focal_length_x;
        mean_focal_length_y += cur_focal_length_y;
        mean_principal_point += cur_principal_point;
        mean_skew += cur_skew;
    }

    mean_focal_length_x /= cameras_.size();
    mean_focal_length_y /= cameras_.size();
    mean_principal_point /= cameras_.size();
    mean_skew /= cameras_.size();

    Perspective_camera neutral_camera(
            Vector(3, 0.0),
            0, 0, 0, // set angles later
            mean_focal_length_x,
            mean_principal_point[0],
            mean_principal_point[1],
            mean_skew,
            mean_focal_length_y / mean_focal_length_x
            );

    neutral_camera.set_orientation(mean_orientation);
    neutral_camera.set_camera_centre(mean_camera_position_);

//        neutral_camera.set_principal_point(cameras_[0].get_principal_point());
//        neutral_camera.set_orientation(cameras_[0].get_orientation());
//        neutral_camera.set_camera_centre(cameras_[0].get_camera_centre());
//        neutral_camera.set_principal_point(cameras_[0].get_principal_point());

    return Turntable_camera(
            neutral_camera,
            mean_axis_,
            turntable_origin_,
            cameras_.size(),
            false);
}


}
//namespce kjb
