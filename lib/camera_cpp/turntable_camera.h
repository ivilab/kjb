/* $Id: turntable_camera.h 22174 2018-07-01 21:49:18Z kobus $ */
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

#ifndef KJB_ST_TURNTABLE_CAMERA_H
#define KJB_ST_TURNTABLE_CAMERA_H

#include <vector>
#include <string>
#include <m_cpp/m_vector.h>
#include <camera_cpp/perspective_camera.h>


#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#else
/* This file gets assert from boost. */
#define assert(X) ASSERT(X)
#endif

namespace kjb
{
    class Calibrated_camera;
/**
 * A class representing a collection of cameras, oriented in a turntable-style configuration.
 *
 * This class is based on a set of assumptions that are true in turntable scenarios:
 *  <pre>
 *  * All cameras have same intrinsic parameters
 *  * All cameras positions are related by pure rotation around the same axis.
 *  * All camera orientations are related by rotation around the same axis.
 *  </pre>
 *
 *  This could also apply to a robot turning an object in it's hand.
 *
 *  @author Kyle Simek
 */
class Turntable_camera
{
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif

public:
    /**
     * Create a null Turntable camera.  Attempting to use this camera as-is will
     * result in an abort (in DEVELOPMENT mode) or undefined behavior 
     * (in PRODUCTION mode).  As such, it must be initialized to a valid camera
     * using the assignment operator before using.
     */
    Turntable_camera();

    /**
     * Create a turntable camera assuming equal spacing of cameras.  
     *
     * @cam The "Neutral" camera, i.e. camera at position 0
     * @rotation_axis  A direction vector representing the turntable's rotation axis.
     * @turntable_origin A position vector representing the turntable's origin, i.e. the center of rotation.  This can be any point lying on the rotation line.
     * @num_positions The number of positions where images were captured, a.k.a "number of cameras". This number of cameras will be evenly spaced around the 390 degree circle.
     * @clockwise  Whether or not turntable rotation is clockwise.  Clockwise-ness is determined by looking in the direction of the rotation axis.  Flipping the direction of the rotation axis will require flipping the _clockwise_ boolean.
     *
     * @pre rotation_axis.size() == turntable_origin.size() == 3
     * */
    Turntable_camera(
        const Perspective_camera& cam,
        const Vector& rotation_axis,
        const Vector& turntable_origin,
        int num_positions,
        bool clockwise = true);

    /**
     * Create a turntable camera from an arbitrary collection of angles.  Angles
     * are relative to the rotation axis direction, following a right-handed 
     * convention; i.e. positive angles correspond to counter-clockwise rotation
     * when looking in the direction of the axis vector.
     *
     *
     * The call below will create a configuration with the world origin coinciding with the turntable origin; the turntable axis pointing upward; using the angles stored in the collection "angles":
     *  
     *  @code
     *  Perspective_camera cam0;
     *  cam0.set_intrinsic_parameters(...);
     *  cam0.set_camera_centre(...);
     *  cam0.set_orientation(Quaternion(...));
     *
     *  Vector angles(4);
     *  angles[0] = 0.0;
     *  angles[1] = M_PI / 2; 
     *  angles[2] = 2 * M_PI / 2;
     *  angles[3] = 3 * M_PI / 2;
     *
     *  Turntable_camera my_camera(
     *      cam0,
     *      Vector(0.0,0.0,1.0),
     *      Vector(3, 0.0),
     *      angles.begin(),
     *      angles.end();
     *  @endcode
     *
     *
     * @param cam The "Neutral" camera, i.e. camera at position 0
     * @param rotation_axis  A direction vector representing the turntable's rotation axis.
     * @param turntable_origin A position vector representing the turntable's origin, i.e. the center of rotation.  This can be any point lying on the rotation line.  
     * @param angles_begin An iterator representing the beginning of a sequence of angles.
     * @param angles_end  An iterator representing one-past-the-end of a sequence of angles.
     *
     * @pre rotation_axis.size() == turntable_origin.size() == 3
     */
    template <typename Iterator>
    Turntable_camera(
        const Perspective_camera& cam,
        const Vector& rotation_axis,
        const Vector& turntable_origin,
        Iterator angles_begin,
        Iterator angles_end) :
            neutral_camera_(cam),
            turntable_origin_(turntable_origin),
            rotation_axis_(rotation_axis),
            angles_(angles_begin, angles_end),
            true_angles_(angles_),
            current_index_(0),
            current_camera_(neutral_camera_),
            indices_()
    {

        indices_.resize(angles_.size());
        for(size_t i = 0; i < angles_.size(); ++i)
            indices_[i] = i;
    }

    /**
     * Standard copy constructor.
     */
    Turntable_camera(const Turntable_camera& other);

    /**
     * Simple assignment
     */
    Turntable_camera& operator=(const Turntable_camera& other)
    {
        Turntable_camera result(other);
        swap(result);
        return *this;
    }

    /**
     * Swap state with another Turntable_camera.  Running time is O(c).
     */
    virtual void swap(Turntable_camera& other);

    /**
     * Returns the principal point of the camera.  Same for all cameras.
     */
    const Vector& get_principal_point() const
    {
        assert(angles_.size() > 0);
        return neutral_camera_.get_principal_point();
    }

    /**
     * Set the principal point of all cameras.
     */
    void set_principal_point(const Vector& pt)
    {
        assert(angles_.size() > 0);
        neutral_camera_.set_principal_point(pt);
    }

    /**
     * Set the i-th camera to be active.
     *
     * @pre This camera has been initialized (is not a Default-constructed camera).
     * @pre This camera has at least one angle.
     *
     * @throws Index_out_of_bounds if i exceeds the number of turntable angles.
     */
    void set_active_camera(size_t i) const;

    /**
     * Set the next camera to be active, or if the last camera is active, 
     * activate the first camera.
     *
     * @pre Object is not a null camera.
     */
    void set_next();

    /**
     * Set the previous camera to be active, or if the first camera is active, 
     * activate the last camera.
     *
     * @pre Object is not a null camera.
     */
    void set_previous();

    /**
     * Send modelview and projection matrices to opengl corresponding to the active
     * camera's extrinsic and intrinsic parameters.
     *
     * @throws Missing_dependency if code was compiled without OpenGL.
     *
     * @pre Object is not a null camera
     */
    void pass_to_opengl(bool clean_buffers = false) const
    {
        // ensure this has been initialized (not constructed with Default ctor)
        assert(angles_.size() > 0);
        current_camera_.prepare_for_rendering(clean_buffers);
    }

    /**
     * Returns the neutral camera, a.k.a. camera 0
     */
    const Perspective_camera& get_neutral_camera() const
    {
        assert(angles_.size() > 0);
        return neutral_camera_;
    }


    const Vector& get_origin() const
    {
        return turntable_origin_;
    }

    const Vector& get_rotation_axis() const
    {
        return rotation_axis_;
    }

    const std::vector<double>& get_angles() const
    {
        return angles_;
    }

    size_t get_current_index() const
    {
        return current_index_;
    }

    const Perspective_camera& get_current_camera() const
    {
        return current_camera_;
    }

    size_t size() const
    {
        return angles_.size();
    }

    const std::vector<size_t>& index_list()
    {
        return indices_;
    }

    /// If not all angles should be used, use this to specify the indices of angles to use.
    void set_index_list(const Index_range& indices);

    void evenly_space_cameras(size_t n);

    virtual const Perspective_camera operator[](int i) const;

    std::vector<Perspective_camera> to_cameras() const;


private: 
    Perspective_camera neutral_camera_;
    Vector turntable_origin_;
    Vector rotation_axis_;
    std::vector<double> angles_;
    std::vector<double> true_angles_;

    mutable size_t current_index_;
    mutable Perspective_camera current_camera_;

    std::vector<size_t> indices_;

private:

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
#ifdef KJB_HAVE_BST_SERIAL
        if(version == 0)
        {
            // no saving to old formats!
            assert(Archive::is_loading::value);

            using namespace boost;
            ar & neutral_camera_;
            ar & turntable_origin_;
            ar & rotation_axis_;
            ar & angles_;

            ar & current_index_;
            ar & current_camera_;

            // fill-in fields added since version 1
            true_angles_ = angles_;
            indices_.resize(angles_.size());
            for(size_t i =0; i < angles_.size(); ++i)
            {
                indices_[i] = i;
            }
        }
        else
        {
            // version 1
            using namespace boost;
            ar & neutral_camera_;
            ar & turntable_origin_;
            ar & rotation_axis_;
            ar & angles_;
            ar & true_angles_;

            ar & current_index_;
            ar & current_camera_;
            ar & indices_;
        }
#else
        KJB_THROW_2(Missing_dependency, "boost::serialization");
#endif
    }
};

/**
 * Receives a collection of calibrated cameras that are assumed to be in a turntable configuration, but due to noise, don't strictly comply with the Turntable_camera assumptions (e.g. coplanarity, pure rotation, etc. ).  This function returns a Turntable_camera that tries to minimize the distance between its cameras and the original cameras.
 *
 * @cameras A set of cameras that have been individually calibrated from a turntable calibration set.
 * @returns Turntable_camera  A turntable camera representing the input cameras, with noise averaged out.
 */
Turntable_camera regularize_turntable_cameras(const std::vector<Calibrated_camera>& cameras);


} // namespace kjb

#ifdef KJB_HAVE_BST_SERIAL
// set serializaztion version
namespace boost {
namespace serialization {
    template <>
    struct version<kjb::Turntable_camera >
    {
        typedef mpl::int_<1> type;
        typedef mpl::integral_c_tag tag;
        BOOST_STATIC_CONSTANT(int, value = version::type::value);
    };
}
}
#endif

#endif
