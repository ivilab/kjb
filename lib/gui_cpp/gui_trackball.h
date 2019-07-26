/* $Id: gui_trackball.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_CPP_GUI_TRACKBALL_H
#define KJB_CPP_GUI_TRACKBALL_H

#ifdef KJB_HAVE_OPENGL
#include <m_cpp/m_vector.h>
#include <m_cpp/m_vector_d.h>
#include <g_cpp/g_quaternion.h>
#include <gr_cpp/gr_primitive.h>
#include <gr_cpp/gr_opengl.h>
#include <camera_cpp/perspective_camera.h>

#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif

#include <boost/bind.hpp>

namespace kjb
{
namespace gui
{

// forward declaration
class Viewer;

    /**
     * This class implements trackball-style rotation of objects using a mouse.
     * A mouse-click sets a reference point on a sphere, and dragging moves that
     * point of the sphere.
     *
     * It follows the standard trackball implementation used by Glut and Blender,
     * which is basically Shoemake's classic "Arcball" interface, modified to handle clicks
     * outside the sphere.
     *
     * This is implemented to be screen-size independent, by using a normalized 
     * screen coordinate system, where the bottom-left corner is (0,0), and the top
     * right is (1.0, 1.0).  Thus, it is the caller's responsibility to scale his/her
     * mouse coordinates before calling methods in this class.
     *
     * Trackball math (in get_z() method)  Copyright (C) Silicon Graphics, Inc.
     */
class Trackball
{
    enum Mouse_mode {no_mode, rotate_mode, zoom_mode, slide_mode} ;
public:
    /**
     * @param diameter In units of screen-size.  e.g. 0.7 will have diameter of 70% of the screen.
     * @param x X-coordinate of trackball center, in normalized screen coordinates (left = 0, right = 1).
     * @param y Y-coordinate of trackball center, in normalized screen coordinates (bottom = 0, top = 1).
     */
    Trackball(double diameter = 0.7, double x = 0.5, double y = 0.5) :
        sphere_center_(x,y),
        sphere_radius_(diameter / 2.0),
        click_pt_(), // size = 0 is intentional
        drag_pt_(3),
        cur_q_(),
        delta_q_(),
        cur_mouse_x_(),
        cur_mouse_y_(),
        cur_t_(kjb::Vector(0, 0, -10)),
        xy_scale_(1.0),
        mode_(no_mode),
        offset_x_(0),
        offset_y_(0),
        width_(1),
        height_(1),
        base_cam_(1.0, 1000.0),
        cam_(base_cam_),
        cam_dirty_(false),
        fixed_fovy_(DEFAULT_FOVY * M_PI/180),
        object_origin_(0.0, 0.0, 0.0),
        ACCELERATION_FACTOR(1.0/200.0)
    {

        base_cam_.set_world_origin(kjb::Vector(cur_t_.begin(), cur_t_.end()));
        cam_ = base_cam_;

        set_focal_from_fovy_();
    }

#ifdef KJB_HAVE_GLUT
    /// setup glut mouse callbacks
    void bind(kjb::opengl::Glut_window& wnd)
    {
        using namespace boost;

        wnd.set_mouse_callback(boost::bind(&Trackball::glut_mouse_down, this, _1, _2, _3, _4));
        wnd.set_motion_callback(boost::bind(&Trackball::glut_mouse_move, this, _1, _2));

        update_viewport();
    }
#endif

    /// add mouse controls to viewer's listener queue
    void attach(Viewer& wnd);

    void update_viewport();

    void update_viewport(int x, int y, int width, int height);

    /**
     * handle a click in glut coordinates (origin top-left)
     */
    bool glut_mouse_down(int button, int state, int x, int y)
    {
        return opengl_mouse_down(button, state, x, height_ - y - 1);
    }

    /**
     * handle a click in opengl coordinates (origin bottom-left)
     */
    bool opengl_mouse_down(int button, int state, int x, int y);

    /**
     * Call this when mouse is clicked to begin a trackball rotation.
     * All coordinates are specified in "normalized screen coordinates", which
     * is independent of the screen size.  Bottom-left is (0.0, 0.0); top-right is (1.0, 1.0).
     *
     * @param x x-coordinate of mouse in "normalized screen coordinates" 
     * @param y y-coordinate of mouse in "normalized screen coordinates" 
     */
    void rotate_mouse_down(double x, double y);

    /// Handle drag event in glut coordinates (origin top-left)
    bool glut_mouse_move(int x, int y)
    {
        return opengl_mouse_move(x, height_ - y - 1);
    }

    /// Handle drag event in opengl coordinates (origin bottom-left)
    bool opengl_mouse_move(int x, int y);

    /**
     * glut mouse coordinates
     */
    void zoom_mouse_move(int x, int y);

    /**
     * glut mouse coordinates
     */
    void slide_mouse_move(int x, int y);

    /**
     * glut mouse coordinates
     */
    void translate_mouse_down(int x, int y);

    /**
     * Call this when dragging with mouse to rotate trackball.
     *
     * @note This can only be called between calls to rotate_mouse_down and mouse_up.
     * Otherwise it will throw and exception
     *
     */
    void rotate_mouse_move(double x, double y);

    // Call when dragging is done
    void rotate_mouse_up();

    /// Get the trackball orientation for sending to kjb::opengl::glRotate
    const Quaternion get_orientation()
    {
        update_camera_();
        return cam_.get_orientation();
    }

    void set_extrinsic(const kjb::Matrix& extrinsic)
    {
        set_extrinsic_dispatch_(extrinsic);
    }

    void set_extrinsic(const kjb::Matrix4& extrinsic)
    {
        set_extrinsic_dispatch_(extrinsic);
    }

    /**
     * @tparam Matrix_type must be either Matrix_d<4,4> or kjb::Matrix
     */
    template <class Matrix_type>
    void set_extrinsic_dispatch_(const Matrix_type& extrinsic);

    void set_clipping_planes(double near, double far);

    /**
     * calling this will unset fixed_fovy.  
     */
    void set_camera(const Perspective_camera& cam);

    /**
     */
    void set_camera(const Perspective_camera& cam, double height);

    void set_camera_same_fovy(const Perspective_camera& cam);

    const kjb::Perspective_camera& get_camera() const;

    /**
     * Set location of object origin in world coordinates.  Rotations
     * will be centered around this point.
     */
    void set_object_origin(const Vector3& o);

    const Vector3& get_object_origin() const;

    /// Reset view to camera
    /// @sa set_camera
    void reset();

    /// set up modelview and projection matrix
    inline void prepare_for_rendering()
    {
        return prepare_for_rendering_dispatch_(false, 0, 0, 0, 0);
    }

    /**
     * @param x cursor position in opengl coordinates (origin bottom-left)
     * @param y cursor position in opengl coordinates (origin bottom-left)
     * @param width selection window width
     * 0param height selection window height
     */
    inline void prepare_for_picking(double x, double y, double width, double height)
    {
        return prepare_for_rendering_dispatch_(
                true, x, y, width, height);
    }

    void prepare_for_rendering_dispatch_(bool selecting, double select_x, double select_y, double select_dx, double select_dy );

    void render_object_origin();

//    /// manually set the trackball orientation
//    // unclear how to handle non-zero object origin; disabling for now
//    void set_orientation(const Quaternion& q)
//    {
//        // make sure mouse isn't down
//        assert(click_pt_.size() == 0);
//
//        delta_q_.reset_identity();
//        cur_q_.reset_identity();
//
////        cam_dirty_ = true; // not needed, because this iseasier:
//        base_cam_.set_orientation(q);
//        cam_.set_orientation(q);
//    }

    /// Render a wireframe sphere representing the trackball
    void render();

    // returns what the trackball thinks the viewport height is
    int viewport_width() 
    {
        return width_;
    }

    // returns what the trackball thinks the viewport height is
    int viewport_height() 
    {
        return height_;
    }

    // returns what the trackball thinks the viewport x_offset is
    int viewport_x_offset()
    {
        return offset_x_;
    }

    // returns what the trackball thinks the viewport y_offset is
    int viewport_y_offset()
    {
        return offset_x_;
    }
private:
    // returns the focal length in pixels
    double get_focal_length_() const
    {
        return cam_.get_focal_length();
    }

    /// get factor to multiply cursor displacement to get world-origin displacement
    double get_xy_scale_() const
    {
        // z is always negative (origin is always in front of us)
        // so we negate it here.
        return -(cur_t_[2]) / get_focal_length_();
    }


    double get_z(double x, double y);

private:
    void update_camera_() const;


    double get_fovy_from_focal_(double focal_length, double image_height);

    void set_focal_from_fovy_();
private:
//    double near_;
//    double far_;
    
    Vector sphere_center_;
    double sphere_radius_;

    Vector click_pt_;
    Vector drag_pt_;

    Quaternion cur_q_;
    Quaternion delta_q_;

    int cur_mouse_x_;
    int cur_mouse_y_;

    kjb::Vector3 cur_t_;
    double xy_scale_;

    Mouse_mode mode_;

    int offset_x_;
    int offset_y_;
    int width_;
    int height_;

    kjb::Perspective_camera base_cam_;
    mutable kjb::Perspective_camera cam_;
    mutable bool cam_dirty_;

    double fixed_fovy_;
    Vector3 object_origin_;

    const double ACCELERATION_FACTOR;
    static const float DEFAULT_FOVY;
};
} // namespace gui
} // namespace kjb

#endif /* HAVE_OPENGL */
#endif 
