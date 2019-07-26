/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */


/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Luca Del Pero
|
* =========================================================================== */


/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief St_perspective_camera for modeling a perspective camera using the classic
 *         Forsyth and Ponce parametrization
 *
 * The Gl_perspective_camera is defined by a camera centre, three rotation angles (pitch,
 * yaw and roll), focal length, aspect ratio, principal point coordinates in the image plane,
 * skew angle.
 */

#ifndef CAMERA_CPP_PERSPECTIVE_CAMERA_H_
#define CAMERA_CPP_PERSPECTIVE_CAMERA_H_

#define FRAME_CAMERA_WORLD_COORDINATES 0
#define FRAME_CAMERA_CAMERA_COORDINATES 1

#define CAMERA_PITCH_INDEX 0
#define CAMERA_YAW_INDEX 1
#define CAMERA_ROLL_INDEX 2

#include <gr_cpp/gr_camera.h>
#include <l_cpp/l_readable.h>
#include <l_cpp/l_writeable.h>
#include <l_cpp/l_cloneable.h>
#include <m_cpp/m_vector.h>

#ifdef KJB_HAVE_BST_SERIAL
//#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>
#endif

namespace kjb
{

class Perspective_camera : public Cloneable, public Readable, public Writeable
{
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif

public:

    Perspective_camera(double inear = 10, double ifar = 10000);

    Perspective_camera(
            double icentre_x,
            double icentre_y,
            double icentre_z,
            double ipitch,
            double iyaw,
            double iroll,
            double ifocal_length,
            double iprincipal_point_x,
            double iprincipal_point_y,
            double iskew,
            double iaspect_ratio,
            double world_scale = 1.0,
            double inear = 10,
            double ifar = 10000);

    Perspective_camera(const kjb::Vector & icamera_centre,
                       double ipitch,
                       double iyaw,
                       double iroll,
                       double ifocal_length,
                       double iprincipal_point_x,
                       double iprincipal_point_y,
                       double iskew,
                       double iaspect_ratio,
                       double world_scale = 1.0,
                       double inear = 10,
                       double ifar = 10000);

     /** @brief Reads a camera from an input file. */
    Perspective_camera(const char* fname);

    /** @brief Reads a camera from an input stream. */
    Perspective_camera(std::istream& in);

    Perspective_camera(const Perspective_camera & pc);

    virtual Perspective_camera & operator = (const Perspective_camera & pc);
    virtual Perspective_camera * clone() const;

    /** Swap contents with another Perspective_camera.
     * This is implemented to run much more quickly than:
     *         Perspective_camera tmp = b;
     *         b = a;
     *         a = tmp;
     */
    virtual void swap(Perspective_camera& other);

    virtual ~Perspective_camera() { }

    /** @brief Reads this camera from an input stream. */
    virtual void read(std::istream& in);

    /** @brief Reads this camera from a file */
    virtual void read(const char * fname)
    {
        Readable::read(fname);
    }


    /** @brief Writes this camera to an output stream. */
    virtual void write(std::ostream& out) const
      ;

    /** @brief Writes this camera to an output stream. */
    virtual void write(const char * fname) const
    {
        Writeable::write(fname);
    }



    /** @brief This funcation makes sure that the rendering interface is
     * consistent with the camera parameters stored in this class
     */
    virtual void update_rendering_interface() const;


    /* Getters */
    /** @brief get near plane z-distance from camera center (always positive) */
    double get_near() const { return rendering_interface.get_near(); }
    void set_near(double near) { rendering_interface.set_near_clipping_plane(near); }

    /** @brief get farplane z-distance from camera center (always positive) */
    double get_far() const { return rendering_interface.get_far(); }
    void set_far(double far) { rendering_interface.set_far_clipping_plane(far); }

    /** @brief returns the world origin in camera coordinates */
    Vector get_world_origin() const;

    /** @brief returns the camera centre */
    const kjb::Vector & get_camera_centre() const {return camera_centre;}

    /** @brief returns the x-coordinate of the camera centre */
    double get_camera_centre_x() const {return camera_centre(0);}

    /** @brief returns the y-coordinate of the camera centre */
    double get_camera_centre_y() const {return camera_centre(1);}

    /** @brief returns the y-coordinate of the camera centre */
    double get_camera_centre_z() const {return camera_centre(2);}

    /** @brief returns the pitch angle in radian */
    double get_pitch() const {return rotation_angles(CAMERA_PITCH_INDEX);}

    /** @brief returns the yaw angle in radian */
    double get_yaw() const {return rotation_angles(CAMERA_YAW_INDEX);}

    /** @brief returns the roll angle in radian */
    double get_roll() const {return rotation_angles(CAMERA_ROLL_INDEX);}

    /** @brief returns the rotation angles [pitch, yaw, roll] */
    const Vector & get_rotation_angles() const { return rotation_angles;}

    /** @brief returns the focal length */
    double get_focal_length() const {return focal_length;}

    /** @brief returns the principal point */
    const kjb::Vector & get_principal_point() const { return principal_point;}

    /** @brief returns the coordinate of the principal point specified by the input index*/
    double get_principal_point(unsigned int index) const { return principal_point(index);}


    /** @brief returns the x-coordinate of the principal point */
    double get_principal_point_x() const {return principal_point(0);}

    /** @brief returns the y-coordinate of the principal point */
    double get_principal_point_y() const {return principal_point(1);}

    /** @brief returns the skew in radian */
    double get_skew() const {return skew;}

    /** @brief returns the aspect ratio */
    double get_aspect_ratio() const {return aspect_ratio;}

    /** @brief returns the world scale */
    double get_world_scale() const {return world_scale;}

    /** @brief This returns a 3X4 camera matrix,
     * built from the intrinsic and extrinsic parameters
     */
    // commented this out, since no implementation exists -- Kyle Mar 5, 2011
//    void get_camera_matrix(kjb::Matrix & camera_matrix);

    /** Setters */

    /** @brief sets the world origin in camera coordinates. */
    void set_world_origin(const kjb::Vector& origin);

    /** @brief sets the camera centre */
    virtual void set_camera_centre(const kjb::Vector & icentre);

    /** @brief sets the coordinate of the camera centre specified by the input index*/
    virtual void set_camera_centre(unsigned int index, double ivalue);

    /** @brief sets the x-coordinate of the camera centre */
    virtual void set_camera_centre_x(double ix);

    /** @brief sets the y-coordinate of the camera centre */
    virtual void set_camera_centre_y(double iy);

    /** @brief sets the y-coordinate of the camera centre */
    virtual void set_camera_centre_z(double iz);

    /** @brief sets the pitch angle in radian */
    virtual void set_pitch(double ipitch);

    /** @brief sets the yaw angle in radian */
    virtual void set_yaw(double iyaw);

    /** @brief sets the roll angle in radian */
    virtual void set_roll(double iroll);

    /** @brief rotates the camera around its x-axis */
    virtual void rotate_around_x_axis(double theta);

    /** @brief rotates the camera around its x-axis */
    virtual void rotate_around_y_axis(double theta);

    /** @brief rotates the camera around its x-axis */
    virtual void rotate_around_z_axis(double theta);

    /** @brief rotates the camera around its x,y,z axes in this order */
    virtual void rotate_around_camera_axes(double thetax, double thetay, double thetaz);

    /** @brief sets pitch, yaw and roll for this camera, in radian */
    virtual void set_rotation_angles(double ipitch, double iyaw, double iroll);

    /** @brief sets the focal length */
    virtual void set_focal_length(double ifocal);

    /** @brief */
    virtual void update_focal_with_scale(double ifocal);

    /** @brief sets the principal point */
    virtual void set_principal_point(const kjb::Vector & ip);

    /** @brief sets the coordinate of the principal point specified by the input index*/
    virtual void set_principal_point(unsigned int index, double ip);

    /** @brief sets the x-coordinate of the principal point */
    virtual void set_principal_point_x(double ix);

    /** @brief sets the y-coordinate of the principal point */
    virtual void set_principal_point_y(double iy);

    /** @brief sets the skew angle */
    virtual void set_skew(double is);

    /** @brief sets the aspect ratio*/
    virtual void set_aspect_ratio(double iar);

    /** @brief sets the world scale */
    virtual void set_world_scale(double iscale);

    /** @brief sets the position and orientation with the semantics similar to gluLookAt */
    virtual void set_look_at(double deyex, double deyey, double deyez, double dlookx, double dlooky, double dlookz,
            double dupx, double dupy, double dupz);

    /** @brief sets the position and orientation with the semantics similar to gluLookAt in a more convenient form */
    virtual void set_look_at(const kjb::Vector & eye, const kjb::Vector & look, const kjb::Vector & up);

    virtual void translate(double dx, double dy, double dz, unsigned int frame = FRAME_CAMERA_WORLD_COORDINATES );

    /* interface to rendering mechanism */

    /** This function makes sure that the rendering mechanism (OpenGL in this case)
     * is set properly so that anything that will be rendered will be seen
     * through a camera having the parameters specified in this class
     */
    virtual void prepare_for_rendering(bool clean_buffers) const;

    /**
     * Return opengl modelview matrix, which can be passed to
     * kjb::opengl::glMultMatrix()
     */
    const Matrix& get_modelview_matrix() const
    {
        update_rendering_interface();
        return rendering_interface.get_modelview_matrix();
    }

    /**
     * Return opengl projection matrix, which can be passed to
     * kjb::opengl::glMultMatrix().  Note that this needs to be 
     * preceeded by a call to glOrtho()
     */
    inline const Matrix& get_projection_matrix() const
    {
        update_rendering_interface();
        return rendering_interface.get_projection_matrix();
    }

    /**
     * Return opengl projection matrix, which can be passed to
     * kjb::opengl::glMultMatrix().  This is the exact matrix that is
     * passed to opengl; i.e. the effect of the glOrtho call is included
     * in this matrix.
     */
    inline Matrix get_gl_projection_matrix() const
    {
        update_rendering_interface();
        return rendering_interface.get_gl_projection_matrix();
    }

    void mult_projection_matrix() const
    {
        update_rendering_interface();
        rendering_interface.mult_gl_projection();
    }

    void mult_modelview_matrix() const;

    virtual Parametric_camera_gl_interface & get_rendering_interface() const
    {
        update_rendering_interface();
        return rendering_interface;
    }

    /** Sets the rotation mode. This will have an impact on how
     * pitch, yaw and roll are interpreted (standard is Euler
     * Mode XYZ with rotating axes, see class Quaternion).
     * No matter what mode you specify here, pitch
     * will be interpreted as Euler angle 1, yaw as Euler angle 2,
     * and roll as Euler angle 3. This function (except for
     * standard mode XYZR) is not adequately tested
     */
    void set_rotation_mode(kjb::Quaternion::Euler_mode imode)
    {
        update_rendering_interface();
        rendering_interface.set_rotation_mode(imode);
    }

    /** @brief sets the rotation angles from an input quaternion */
    virtual void set_angles_from_quaternion(const kjb::Quaternion & q);

    /** @brief Alias of set_angles_from_quaternion() */
    virtual void set_orientation(const kjb::Quaternion & q)
    {
        set_angles_from_quaternion(q);
    }


    /** @brief returns the rotations of this camera as a quaternion */
    inline const kjb::Quaternion & get_rotations_as_a_quaternion() const
    {
        update_rendering_interface();
        return rendering_interface.get_orientation();
    }

    /** @brief Alias of get_angles_as_a_quaternion() */
    inline const kjb::Quaternion & get_orientation() const
    {
        return get_rotations_as_a_quaternion();
    }

    /** @brief Rotates the input point into the camera coordinate system*/
    inline void rotate_point_to_camera_frame(kjb::Vector &ipoint)
    {
        update_rendering_interface();
        rendering_interface.rotate_point_to_camera_frame(ipoint);
    }

    /** @brief Given a point in camera coordinates, it converts it to world coordinates */
    inline void get_point_in_world_coordinates
    (
        const kjb::Vector & point_in_camera_coordinates,
        kjb::Vector & point_in_world_coordinates
    ) const
    {
        update_rendering_interface();
        rendering_interface.get_point_in_world_coordinates(point_in_camera_coordinates, point_in_world_coordinates);
        point_in_world_coordinates(3) = 1.0;
    }

    /** @brief Given a point in world coordinates, it returns true if the point is in the viewing frustum,
     *  false otherwise. Notice that the OpenGL context will be set in this function, since it is needed
     *  for the computation */
    inline bool is_point_in_camera_frustum
    (
        const kjb::Vector & point_in_world_coordinates,
        double & x_,
		double & y_,
        unsigned int num_image_rows,
        unsigned int num_image_cols
    ) const
    {
        update_rendering_interface();
        prepare_for_rendering(true);
        return rendering_interface.is_point_in_camera_frustum(point_in_world_coordinates, x_, y_, num_image_rows, num_image_cols);
    }


    /** @brief Given a point in world coordinates, it converts it to camera coordinates */
    inline void get_point_in_camera_coordinates
    (
        kjb::Vector & io_point
    ) const
    {
        update_rendering_interface();
        rendering_interface.transform_point_to_camera_frame(io_point);
    }

    void compute_new_euler_angles_on_rotations(double dpitch, double dyaw, double droll);

    /** Get the camera matrix corresponding to this camera*/
    /* Note: I modified this class to cache the camera matrix. It
     * seems to be working for now.
     */
    const Matrix& get_camera_matrix() const
    {
        update_camera_matrix();
        return m_camera_matrix;
    }

    /** 
     * Returns the camera matrix. This is a legacy call. get_camera_matrix()
     * is preferred.
     *
     * Note: I left this class for backward-compatability. However, it is
     * now much faster (it simply returns the matrix, updating if
     * necessary. (Ernesto -- 2013/10/23)
     */
    const Matrix& build_camera_matrix() const
    {
        return get_camera_matrix();
    }

    Vector to_camera_coordinates(const Vector& v) const;

    /**
     * Apply Tsai's radial distortion to point
     */
    Vector radial_distortion(const Vector& x);

private:

    /* Extrinsic parameters */

    /* 3D position of the camera centre in homogeneous coordinates.
     * We store it in homogeneous coordinates for convenience. It is always to be kept normalized
     * such that the homgeneous coordinate is 1 */
    kjb::Vector camera_centre;

    /* Vector storing the rotation angles around the object's x axis (pitch),
     * the object's y axis (yaw), and the object's z axis (roll). Sorted in this order*/
    kjb::Vector rotation_angles;

    /* Intrinsic parameters */

    /* The focal length, distance between camera center and image plane */
    double focal_length;

    /* The 2D position of the principal point in the image plane */
    kjb::Vector principal_point;

    /* The skew defined as the angle between the two image axis (usually 90 degrees
     * or very close). Stored in radian
     */
    double skew;

    /* The ratio between the vertical pixel length and the horizontal pixel length.
     * It is often 1
     */
    double aspect_ratio;

    /*
     * The scale of the world
     */
    double world_scale;


    /*
     * The radial distortion coefficients
     */
    double m_rd_k1;
    double m_rd_k2;
    double m_rd_k3;
    double m_rd_p1;
    double m_rd_p2;

protected:
    /* Rendering */

    /** Interface to opengl */
    mutable Parametric_camera_gl_interface rendering_interface;

    /** True if the gl interface was not updated to reflect the changes to an intrinsic parameter */
    mutable bool intrinsic_dirty;

    /** True if the gl interface was not updated to reflect the changes to an extrinsic parameter */
    mutable bool extrinsic_dirty;

    /** True if the camera matrix was not updated to reflect changes. */
    mutable bool cam_matrix_dirty;

    /** Camera matrix */
    mutable Matrix m_camera_matrix;

private:
    template<class Archive>
    void serialize(Archive &ar, const unsigned int /* version */)
    {
#ifdef KJB_HAVE_BST_SERIAL
        double near;
        double far;
        if(Archive::is_saving::value == true)
        {
            // don't want to write full serialization routine for
            // Parametric_camera_gl_interface, so just save the
            // near and far; rest will be updated when needed.
            near = rendering_interface.get_near();
            far = rendering_interface.get_far();
        }

        ar & camera_centre;
        ar & rotation_angles;
        ar & focal_length;
        ar & principal_point;
        ar & skew;
        ar & aspect_ratio;
        ar & world_scale;
        ar & near;
        ar & far;

        // if reading, set dirties to true
        if(Archive::is_loading::value == true)
        {
            rendering_interface = Parametric_camera_gl_interface(near, far);
            intrinsic_dirty = true;
            extrinsic_dirty = true;
            cam_matrix_dirty = true;

            update_rendering_interface();
        }
#else
    KJB_THROW_2(Missing_dependency, "boost::serialization");
#endif
    }

    /** Updates the cached camera matrix.
     * Note: I added this in order to speed up some of my code. I left
     * build_camera_matrix() there for backward-compatibility.
     * Ernesto (2013/10/23).
     */
    void update_camera_matrix() const;
};

/** @brief  Swap two cameras. */
inline void swap(Perspective_camera& cam1, Perspective_camera& cam2)
{
    cam1.swap(cam2);
}

} // namespace kjb

#endif  /* CAMERA_CPP_PERSPECTIVE_CAMERA_H_ */
