/* $Id: gui_trackball.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifdef KJB_HAVE_OPENGL
#include <gui_cpp/gui_trackball.h>
#include <gui_cpp/gui_viewer.h>

#include <m_cpp/m_vector_d.h>
#include <m_cpp/m_matrix_d.h>

namespace kjb
{
namespace gui
{

const float Trackball::DEFAULT_FOVY = 90;

/**
 * Utility method for multiplying a normal 3-vector with a homogeneous (4-by-4) rotation matrix
 */
void rotate(Vector3& pt, const Matrix_d<4,4>& R)
{
    Vector3 tmp = pt;
    pt[0] = pt[1] = pt[2] = 0.0;

    for(size_t row = 0; row < 3; row++)
    for(size_t col = 0; col < 3; col++)
        pt[row] += R(row, col) * tmp[col];
}

/// add mouse controls to viewer's listener queue
void Trackball::attach(kjb::gui::Viewer& wnd)
{
    using namespace boost;
    wnd.add_after_mouse_listener(boost::bind(&Trackball::opengl_mouse_down, this, _1, _2, _3, _4));
    wnd.add_motion_listener(boost::bind(&Trackball::opengl_mouse_move, this, _1, _2));

    update_viewport();
}

void Trackball::update_viewport()
{
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    update_viewport(vp[0], vp[1], vp[2], vp[3]);

    GL_ETX();
}

void Trackball::update_viewport(int x, int y, int width, int height)
{
    offset_x_ = x;
    offset_y_ = y;
    width_ = width;
    height_ = height;

    if(fixed_fovy_ > 0.0)
    {
        set_focal_from_fovy_();
    }
}

bool Trackball::opengl_mouse_down(int button, int state, int x, int y)
{
#ifdef KJB_HAVE_GLUT
    if(button == GLUT_LEFT_BUTTON)
    {
        // ROTATE MODE
        mode_ = rotate_mode;
        if(state == GLUT_DOWN)
        {
            // convert to normalized coordinates and pass along
            rotate_mouse_down(
                ((double) x - offset_x_)/width_,
                ((double) y - offset_y_)/height_);
        }
        else
        {
            mode_ = no_mode;
            rotate_mouse_up();
        }
    }
    else if(button == GLUT_RIGHT_BUTTON)
    {
        // TRANSLATE MODE
        if(state == GLUT_DOWN)
        {
            int modifiers = glutGetModifiers();

            if(modifiers & GLUT_ACTIVE_SHIFT)
                mode_ = slide_mode;
            else
                mode_ = zoom_mode;

            translate_mouse_down(x, y);
        }
        else // mouse up;
        {
            mode_ = no_mode;
        }
    } 
    else // other button
    {
        mode_ = no_mode;
    }

    glutPostRedisplay();

    return true;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void Trackball::rotate_mouse_down(double x, double y)
{
    // convert to sphere-centric coordinates
    x -= sphere_center_[0];
    y -= sphere_center_[1];

    click_pt_.resize(3);

    // 
    click_pt_[0] = x;
    click_pt_[1] = y;
    click_pt_[2] = get_z(x,y);

}

bool Trackball::opengl_mouse_move(int x, int y)
{
#ifdef KJB_HAVE_GLUT
//        std::cout << "y: " << y << std::endl;

    if(mode_ ==  rotate_mode)
    {
        rotate_mouse_move(
            ((double) x - offset_x_)/width_,
            ((double) y - offset_y_)/height_);
    }
    else if(mode_ == zoom_mode)
    {
        zoom_mouse_move(x, y);
    }
    else if(mode_ == slide_mode)
    {
        slide_mouse_move(x, y);
    }
    else
        return false;

    glutPostRedisplay();
    return true;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void Trackball::zoom_mouse_move(int x, int y)
{
//        int dx = x - cur_mouse_x_;
    int dy = y - cur_mouse_y_;

    cur_mouse_x_ = x;
    cur_mouse_y_ = y;

    // this is a bit awkward...
    // We want to use the world origin as the point that we approach
    // asymtotically.  But if the "base camera" isn't located at the origin,
    // we have to include it's location in the asymtotic expression, but
    // remove it from the final offset.
    // Without base_z_, this woiuld look like:
    //  cur_z_ *= exp(dy * ACCELERATION_FACTOR)
    //  TODO: should base_z_ be negated

//        cur_z_ = ( cur_z_ + base_z_ ) * exp(dy * ACCELERATION_FACTOR ) - base_z_;
//        cur_z_ = std::min(cur_z_ + base_z_, -1.0) - base_z_;

    // simplified version, ignore above
    cur_t_[2] = std::min(cur_t_[2] * exp(dy * ACCELERATION_FACTOR ), -1.0);

    //std::cout << cur_t_[2] << std::endl;

    cam_dirty_ = true;

    xy_scale_ = get_xy_scale_();
}

void Trackball::slide_mouse_move(int x, int y)
{
    int dx = x - cur_mouse_x_;
    int dy = y - cur_mouse_y_;

    dx *= xy_scale_;
    dy *= xy_scale_;


    cur_mouse_x_ = x;
    cur_mouse_y_ = y;
    
    cur_t_[0] += dx;
    cur_t_[1] += dy;

    cam_dirty_ = true;
}

void Trackball::translate_mouse_down(int x, int y)
{
    cur_mouse_x_ = x;
    cur_mouse_y_ = y;
}

void Trackball::rotate_mouse_move(double x, double y)
{
    if(mode_ != rotate_mode) return;

    if(click_pt_.size() != 3)
    {
        KJB_THROW_2(Runtime_error, "Mouse_move must be called after rotate_mouse_down but before rotate_mouse_up.");
    }

    // convert to sphere-centric coordinates
    x -= sphere_center_[0];
    y -= sphere_center_[1];

    // convert to sphere coordinates
    drag_pt_[0] = x;
    drag_pt_[1] = y;
    drag_pt_[2] = get_z(x,y);


    // get rotation axis
    Vector axis = cross(click_pt_, drag_pt_).normalize();
    // get rotation angle
    double angle = (drag_pt_ - click_pt_).magnitude() / sphere_radius_;
//        double angle = acos(dot(Vector(click_pt_).normalize(), Vector(drag_pt_).normalize()));

#ifdef DEBUG
    std::cout << "axis: " << axis << '\t' << "angle: " << angle << std::endl;
#endif

    delta_q_ = Quaternion(axis, angle);

    cam_dirty_ = true;
}

void Trackball::rotate_mouse_up()
{
    // make current rotation permanent
    if(!delta_q_.is_identity())
    {
        // get object origin w.r.t. current camera
        kjb::Vector3 object_origin_cam = (cur_q_ * base_cam_.get_orientation()).rotate(object_origin_);

        // translate to camera origin, rotate, translate back, i.e.:
        // o + R_delta * -o
        cur_t_ += object_origin_cam + delta_q_.rotate(-object_origin_cam);
        cur_q_ = delta_q_ * cur_q_;

        delta_q_.init_identity();

        cam_dirty_ = true; // probably unnecessary, but just to be safe
    }

    click_pt_.resize(0);
}

template <class Matrix_type>
void Trackball::set_extrinsic_dispatch_(const Matrix_type& extrinsic)
{
    if(
        extrinsic.get_num_rows() == 4 && (
        extrinsic(3, 0) != 0 ||
        extrinsic(3, 1) != 0 ||
        extrinsic(3, 2) != 0 ||
        extrinsic(3, 3) != 1))
    {
        KJB_THROW_2(Illegal_argument, "Last row of extrinsic matrix must be {0, 0, 0, 1}");
    }

    kjb::Matrix base_rotation = kjb::create_identity_matrix(4);

    for(size_t row = 0; row < 3; row++)
    for(size_t col = 0; col < 3; col++)
    {
        base_rotation(row, col) = extrinsic(row, col);
    }

    base_cam_.set_orientation(kjb::Quaternion(base_rotation));

    kjb::Vector base_translation(4);
    for(size_t row = 0; row < 3; row++)
    {
        base_translation[row] = extrinsic(row, 3);
    }
    base_translation(3) = 1.0;

    base_cam_.set_world_origin(base_translation);

    reset(); // set cam_ = base_cam_
}

template void Trackball::set_extrinsic_dispatch_<kjb::Matrix>(const kjb::Matrix& extrinsic);
template void Trackball::set_extrinsic_dispatch_<kjb::Matrix4>(const kjb::Matrix4& extrinsic);

void Trackball::set_clipping_planes(double near, double far)
{
    base_cam_.set_near(near);
    base_cam_.set_far(far);
    cam_.set_near(near);
    cam_.set_far(far);
}

void Trackball::set_camera(const Perspective_camera& cam)
{
    base_cam_ = cam;
    fixed_fovy_ = -1;

    reset(); // cam = base_cam_;
}

void Trackball::set_camera(const Perspective_camera& cam, double height)
{
    base_cam_ = cam;

    fixed_fovy_ = get_fovy_from_focal_(cam.get_focal_length(), height);

    reset(); // cam = base_cam_;
    set_focal_from_fovy_();
}

void Trackball::set_camera_same_fovy(const Perspective_camera& cam)
{
    base_cam_ = cam;

    reset(); // cam = base_cam_;
    set_focal_from_fovy_();
}

const kjb::Perspective_camera& Trackball::get_camera() const
{
    update_camera_();

    return cam_;
}

void Trackball::set_object_origin(const Vector3& o)
{
    update_camera_();
    // convert to camera coordinates
    object_origin_ = o;

    cam_dirty_ = true;
}

const Vector3& Trackball::get_object_origin() const
{
    return object_origin_;
}

void Trackball::reset()
{
    cur_q_.init_identity();
    delta_q_.init_identity();
    cur_t_ = base_cam_.get_world_origin();

    cam_ = base_cam_;
}

/**
 * Utility method to set up opengl's matrix state prior to rendering.  This version
 * allows preparing "selection" mode, which is used for object-picking with the mouse.
 * Don't call this directly.
 *
 * @param selecting whether or not this is rendering in selection mode or regular render mode
 * @param select_x cursor position in opengl coordinates (i.e. bottom-left origin) for selection mode
 * @param select_y cursor position in opengl coordinates (i.e. bottom-left origin) for selection mode
 * @param select_dx window_size in x-direction for selection mode
 * @param select_dy window_size in y-direction for selection mode
 */
void Trackball::prepare_for_rendering_dispatch_(bool selecting, double select_x, double select_y, double select_dx, double select_dy )
{
    using namespace kjb::opengl;
    update_camera_();

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    if(selecting)
    {
       GLint viewport[4];
       glGetIntegerv(GL_VIEWPORT, viewport);

       gluPickMatrix((GLdouble) select_x, (GLdouble) select_y,
                select_dx, select_dy, viewport);
    }

    cam_.mult_projection_matrix();

    glMatrixMode(GL_MODELVIEW);
    cam_.mult_modelview_matrix();

    GL_ETX();
}


/// Will render in world coordinates
void Trackball::render_object_origin()
{
    // axis length will be at most 1/5 of the window width.
    static const double AXIS_SIZE = 0.2;
    float scale = xy_scale_; // this converts pixels distances into world distances at the origin.
    scale *= width_;
    scale *= AXIS_SIZE;
    glPushAttrib(GL_CURRENT_BIT); // color 
    glPushAttrib(GL_ENABLE_BIT); // lighting
    glDisable(GL_LIGHTING);
    glPushMatrix();
    kjb::opengl::glTranslate(object_origin_);
    glScalef(scale, scale, scale);
    glBegin(GL_LINES);
        // x axis
        glColor3f(1.0, 0.0, 0.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(1.0,0.0,0.0);
        // y axis
        glColor3f(0.0, 1.0, 0.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,1.0,0.0);
        // z axis
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,0.0,1.0);
    glEnd();
    glPopMatrix();
    glPopAttrib();
    glPopAttrib();
}



/// Render a wireframe sphere representing the trackball
void Trackball::render()
{
    if(mode_ != rotate_mode) return;

    // This method probably needs to be reworked once its clearer how
    // the trackball will be used in practice.
    
    // simple screen-base projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0.0, 1.0, 0.0, 1.0, -10, 10);

    // modelview
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    glTranslatef(sphere_center_[0], sphere_center_[1], 0);

#ifdef DEBUG
    if(mode_ == rotate_mode)
    {
        glPushAttrib(GL_ENABLE_BIT);
        glPushAttrib(GL_CURRENT_BIT);

        // show click point and drag point
        glDisable(GL_LIGHTING);

        opengl::Sphere tmp(0.05);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0.0, 1.0, 0.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(sphere_center_[0], sphere_center_[1], 0);

        glPushMatrix();
        opengl::glTranslate(click_pt_);
        glColor3f(1.0, 0.0, 0.0);
        tmp.render();
        glPopMatrix();

        glPushMatrix();
        glColor3f(0.0, 1.0, 0.0);
        opengl::glTranslate(drag_pt_);
        tmp.render();
        glPopMatrix();
//            std::cout << "clicked: " << click_pt_ << std::endl;
//            std::cout << "dragged: " << drag_pt_ << std::endl;

        glPopAttrib();
        glPopAttrib();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
#endif

    kjb::opengl::glRotate(get_orientation());


    kjb::opengl::Sphere s;
    s.set_radius(sphere_radius_);
    s.wire_render();


    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

double Trackball::get_z(double x, double y)
{
    double d = sqrt(x*x + y*y);
    const double r = sphere_radius_;

    // Gavin Bell's trackball function.
    // A smooth function that
    // looks like a circle near the center
    // but approaches zero asymptotocally as
    // mouse's distance from sphere approaches infinity
    if(d < r / M_SQRT2)
    {
        // on sphere
        return sqrt(r * r - d * d);
    }
    else
    {

        // on hyperbola
        return r * r / (2 * d);
    }
}

void Trackball::update_camera_() const
{
    if(!cam_dirty_) return;

    /// ROTATION 
    cam_.set_orientation(delta_q_ * cur_q_ * base_cam_.get_orientation());

    /// TRANSLATION
    kjb::Vector3 t(cur_t_);

    // add translation due to in-progress rotation in order to
    // keep object origin stationary
    if(!delta_q_.is_identity())
    {
        // get object origin w.r.t. current camera
        kjb::Vector3 object_origin_cam = (cur_q_ * base_cam_.get_orientation()).rotate(object_origin_);

        // translate to camera origin, rotate, translate back, i.e.:
        // o + R_delta * -o
        t += object_origin_cam + delta_q_.rotate(-object_origin_cam);
    }

    cam_.set_world_origin(kjb::Vector(t.begin(), t.end()));

    cam_dirty_ = false;
}

double Trackball::get_fovy_from_focal_(double focal_length, double image_height)
{
    double tan_theta = 0.5 * image_height / focal_length;
    double theta = atan(tan_theta);
    return 2.0 * theta;
}

void Trackball::set_focal_from_fovy_()
{
    // display will resize with viewport so field-of-view
    // doesn't change.
    double f = 0.5 * height_ / tan(fixed_fovy_/2.0);
    base_cam_.set_focal_length(f);
    cam_.set_focal_length(f);
}



} // namespace gui
} // namespace kjb

#endif /* have_opengl */
