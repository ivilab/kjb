/* $Id: calibrated_camera.h 18278 2014-11-25 01:42:10Z ksimek $  */
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

#ifndef KJB_CAMERA_CPP_CAMERA_CALIBRATED_CAMERA
#define KJB_CAMERA_CPP_CAMERA_CALIBRATED_CAMERA

// this should be first:
#ifdef KJB_HAVE_BST_SERIAL
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#endif

#include <camera_cpp/perspective_camera.h>
#include <g_cpp/g_camera_calibration.h>

#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>

#include <g_cpp/g_quaternion.h>

#ifdef KJB_HAVE_BST_SERIAL
//#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>
#endif

namespace kjb 
{

/**
 * A perspective camera whose parameters are defined by a camera
 * calibration matrix.
 *
 * @author Kyle Simek
 */
class Calibrated_camera : public Perspective_camera
{
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif

typedef Perspective_camera Base;
typedef Calibrated_camera Self;

public:
    Calibrated_camera(double near = 10, double far = 10000) :
        Base(near, far)
    {}

    Calibrated_camera(
            const Matrix& camera_matrix, 
            Calibration_descriptor cal,
            double near = 10,
            double far = 10000) :
        Base(near, far)
    {
        Matrix intrinsic, rotation;
        Vector translation;

        // does simple decomposition
        decompose_camera_matrix(
                camera_matrix,
                intrinsic,
                rotation,
                translation);

        // converts to "standard format" by moving principal point to center of image,
        // enforcing an upward y-axis, flipping the z-axis if the origin maps behind the camera,
        // etc.
        cal.look_down_positive_z = false;
        standardize_camera_matrices(intrinsic, rotation, translation, cal);

        // force intrinsic(2,2) to be -1
//        intrinsic *= -1;

        initialize(
                intrinsic,
                rotation,
                translation);
    }

    Calibrated_camera(const Self& other) :
        Base(other)
    { }

    virtual Self* clone() const
    {
        return new Self(*this);
    }

    virtual Self& operator=(const Self& other)
    {
        Base::operator=(other);
        return *this;
    }

    /**
     * @pre intrinsic, rotation and translation are in "Standard" form, as defined in the documentation of kjb::standardize_camera_matrices.
     */
    void initialize(
            const Matrix& intrinsic, 
            Matrix rotation,
            Vector translation)
    {
        double alpha_x = intrinsic(0,0);
        double s       = intrinsic(0,1);
        double alpha_y = intrinsic(1,1);
        double x0      = intrinsic(0,2);
        double y0      = intrinsic(1,2);

        double theta = atan2(alpha_x, -s);
        // TODO test this value
        if(theta < 0) theta += M_PI;
        if(theta > M_PI) theta -= M_PI;

        double tmp_aspect_ratio = alpha_y * sin(theta) / alpha_x;

        if(tmp_aspect_ratio < 0)
        {
            tmp_aspect_ratio *= -1;
            y0 *= -1;
        }

        // These three variables correspond to
        // how the camera is parameterized in the 
        // Parametric_camera class.  The choice of parameterization is
        // a bit esoteric, which results in us using these strance expressions
        // in order for the opengl matrix to be constructed right.
        // TODO: have an alternative parameterization that abstracts away
        // these confusing expressions.
        double skew_ = acos(-s/alpha_y);
        double focal = alpha_y * sin(skew_);
        double ar = alpha_x / focal;
        set_focal_length(focal);
        set_aspect_ratio(ar);
        set_skew(skew_);
//        set_focal_length(alpha_x);
//        set_aspect_ratio(tmp_aspect_ratio);
//        set_skew(theta);

        // we've already flipped z-coordinate during 
        // camera matrix decomposition & standardization.
        // Negating here counteracts the negating that happens
        // in set_principal_point().
        set_principal_point(Vector().set(-x0, -y0));

        set_orientation(Quaternion(rotation));

        // TODO: add set world origin() method to perspective camera so we don't need to manipulate this here
        Vector camera_center = -rotation.transpose() * translation;
        set_camera_centre(camera_center);
    }

    void set_orientation(const Quaternion& orientation) 
    {
        set_angles_from_quaternion(orientation);
    }

    const Quaternion& get_orientation() const
    { return get_rotations_as_a_quaternion(); }

private:

    template<class Archive>
    void serialize(Archive &ar, const unsigned int /* version */)
    {
#ifdef KJB_HAVE_BST_SERIAL
        ar & ::boost::serialization::base_object<Base>(*this);
#else
        KJB_THROW_2(Missing_dependency, "boost::serialization");
#endif
    }

};

} // namespace kjb


#endif 
