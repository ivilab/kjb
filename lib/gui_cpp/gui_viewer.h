/* $Id: gui_viewer.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_GR_GENERIC_VIEWER_H
#define KJB_GR_GENERIC_VIEWER_H

#ifdef KJB_HAVE_OPENGL
#include <gui_cpp/gui_trackball.h>
#include <gui_cpp/gui_overlay.h>
#include <gui_cpp/gui_selectable.h>
#include <gui_cpp/gui_event_listener.h>
#include <gui_cpp/gui_window.h>

#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif

#include <g_cpp/g_camera.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_texture.h>
#include <gr_cpp/gr_concept.h>

#include <gr_cpp/gr_renderable.h>
#include <camera_cpp/perspective_camera.h>

#include <vector>
#include <list>
#include <utility>
#include <set>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/ref.hpp>
#include <boost/any.hpp>
#include <boost/concept_check.hpp> 
#include <boost/shared_ptr.hpp> 
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <boost/utility/in_place_factory.hpp>

namespace kjb
{
namespace gui
{

/**
 * A 3D interface for viewing a kjb::Rendable object.
 * 
 * Includes a -rotate-zoom mouse interface, 
 *
 * Mouse:
 *  drag: rotate
 *  right-click drag: "zoom" (i.e. translation in z-direciton.  up zooms out, down zooms in)
 *  right-click shift-drag: moves scene in-plane
 *
 * Keys:
 *  <space>: resets to original camera position
 *  'o': display/hide origin
 *
 * Notes:
 * Zooming-in will asymtotically approach the world origin, but will never pass it. Zooming does not affect the camera's focal length (zooming with focal length generally makes the scene look crazy!).
 *
 * Rotation is currently implementing using the "Trackball" interface proposed by Ken Shoemake.  
 * This differs from some other 3D viewers which implement what I call the "tangent space" rotation interface.  
 * The main difference is that the trackball interface has no hysteresis, i.e. draging from point A to point B, and back to point A results in no change in rotation.  
 * We may add the option of having tangent-space rotation in a later version (it's relatively easy to implement, compared to trackball)
 *
 *  In-plane movement is scaled relative to the zoom amount, so moving D pixels in the window should move the project of the world origin by D pixels.
 *
 *  TODO:
 *      control-drag should move the object origin in-plane
 */
class Viewer
{
private:
        // PRIVATE TYPES 
    typedef Viewer Self;
    enum Stereo_mode {no_stereo, horizontal_stereo, anaglyph_stereo, anaglyph_swap_stereo};

    // overlay management
    typedef boost::shared_ptr<Overlay> Overlay_ptr;
    typedef std::list<boost::any> Overlay_storage_list;
    typedef Overlay_storage_list::iterator Overlay_storage_iterator;
    typedef std::map<Overlay*, Overlay_storage_iterator> Overlay_ownership_map;
    typedef std::list<Overlay_ptr> Overlay_list;
    typedef Overlay_list::iterator Overlay_iterator;


    // listener management
    typedef boost::function4<bool, int, int, int, int> Mouse_listener;
    typedef std::list<Mouse_listener> Mouse_listeners;

    typedef boost::function2<bool, int, int> Motion_listener;
    typedef std::list<Motion_listener> Motion_listeners;

    typedef boost::function3<bool, unsigned int, int, int> Keyboard_listener;
    typedef std::list<Keyboard_listener> Keyboard_listeners;

    typedef boost::function3<bool, int, int, int> Special_listener;
    typedef std::list<Special_listener> Special_listeners;

    typedef std::list<boost::shared_ptr<Selectable> > Selectables;
public:
    typedef Keyboard_listeners::iterator Keyboard_listener_iterator;
    typedef Special_listeners::iterator Special_listener_iterator;
    typedef Motion_listeners::iterator Motion_listener_iterator;
    typedef Mouse_listeners::iterator Mouse_listener_iterator;
    typedef Selectables::iterator Selectable_iterator;
private:
    struct Event_listener_iterators
    {
        Mouse_listener_iterator mouse;
        Mouse_listener_iterator mouse_double;
        Motion_listener_iterator motion;
        Motion_listener_iterator passive_motion;
        Keyboard_listener_iterator keyboard;
        Special_listener_iterator special;
//        Selectable_iterator selectable;
    };
    typedef std::map<Event_listener*, Event_listener_iterators> Event_listeners;
public:
    typedef Event_listeners::iterator Event_listener_iterator;
private:
    typedef std::list<boost::shared_ptr<Window> > Window_stack;
    struct Window_iterators
    {
        Overlay_iterator overlay;
        Event_listener_iterator events;
        Window_stack::iterator window_stack;
    };
    typedef std::map<Window*, Window_iterators> Windows;
public:
    typedef Windows::iterator Window_iterator;

private:
    // Callback management
    
    // Render callbacks
    typedef boost::function0<void> Callback;
    typedef std::list<Callback> Callback_list;
    typedef std::map<const Callback*, char> Render_visibility_map;

    // Reshape callbacks
    typedef boost::function2<void, int, int> Reshape_callback;
    typedef std::list<Reshape_callback> Reshape_callback_list;
public:
    typedef Callback_list::iterator Render_callback_iterator;
    typedef Callback_list::const_iterator Render_callback_const_iterator;

    typedef Reshape_callback_list::iterator Reshape_callback_iterator;
    typedef Reshape_callback_list::const_iterator Reshape_callback_const_iterator;
private:
    // timer management


    struct Timer_entry
    {
        Timer_entry(
                boost::function0<void> f_,
                unsigned int period_,
                bool redraw_,
                Self* viewer_) :
            f(f_),
            period(period_),
            redraw(redraw_),
            viewer(viewer_)
        {}

        boost::function0<void> f;
        unsigned int period;
        bool redraw;
        Self* viewer;
    };

    // ANIMATION MANAGEMENT
    struct Camera_tween
    {
        Perspective_camera src;
        Perspective_camera dest;
        size_t elapsed;
        size_t duration;
    };

    struct null_deleter
    {
        void operator()(void const*) const
        { }
    };
public:
    Viewer(size_t width = 300, size_t height = 300) :
        trackball_(),
        click_time_(),
        display_origin_(false),
        render_callbacks_(),
        reshape_callbacks_(),
        render_visibility_(),
        overlays_(),
        overlay_storage_(),
        overlay_ownership_(),
        overlay_attachments_(),
        event_listeners_(),
        windows_(),
        window_stack_(),
        front_mouse_listeners_(),
        back_mouse_listeners_(),
        mouse_double_listeners_(),
        motion_listeners_(),
        passive_motion_listeners_(),
        keyboard_listeners_(),
        special_listeners_(),
        selectables_(),
        selectable_indices_(),
        old_rotation_origin_(),
        base_width_(width),
        base_height_(height),
        width_(width),
        height_(height),
        resize_ratio_(1.0),
        bg_mode_(gradient_bg),
        bg_color_(1.0, 1.0, 1.0, 1.0),
        bg_gradient_bot_(128.0/255.0, 128.0/255.0, 128.0/255.0, 1.0),
        bg_gradient_top_(0.0, 0.0, 0.0, 1.0),
        bg_image_(),
        camera_tween_(),
        stereo_mode_(no_stereo),
        stereo_eye_offset_(),
        stereo_pixel_offset_()
    {

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);

        trackball_.attach(*this);
    }

#ifdef KJB_HAVE_GLUT
    /**
     * window will be resized to the size of this viewer on next glut-loop
     */
    void attach(kjb::opengl::Glut_window& wnd)
    {
        using namespace boost;
        wnd.set_size(width(), height());

        wnd.set_display_callback(boost::bind(&Viewer::display, this));
        wnd.set_reshape_callback(boost::bind(&Viewer::reshape, this, _1, _2));
        wnd.set_keyboard_callback(boost::bind(&Self::process_keyboard_, this, _1, _2, _3));
        wnd.set_mouse_callback(boost::bind(&Self::process_mouse_, this, _1, _2, _3, _4));
        wnd.set_motion_callback(boost::bind(&Self::process_motion_, this, _1, _2));
        wnd.set_passive_motion_callback(boost::bind(&Self::process_passive_motion_, this, _1, _2));

        add_timer(boost::bind(&Self::tick, this), TICK_PERIOD);

       trackball_.update_viewport();
    }
#endif

    inline void set_extrinsic_matrix(const kjb::Matrix& m)
    {
        trackball_.set_extrinsic(m);
    }

    void set_clipping_planes(double near, double far)
    {
        trackball_.set_clipping_planes(near, far);
    }

    /**
     * Set a camera to view the scene from.  The user provides the camera's resolution
     * relative to it's focal length; this will define the field-of-view,
     * which will remain fixed until another call to set_camera.  In practice, this 
     * means that resizing the window will rescale the rendering of the scene so
     * exactly the same amount of the world is visible.  
     *
     * @param image_height The camera's vertical image resolution in pixels.
     *
     * @note height is used to fix the vertical field-of-view; horizontal field-of-view remains free, so increasing/reducing the window's width will reveal/clip some of the scene.
     */
    inline void set_camera(const kjb::Perspective_camera& cam, size_t image_height)
    {
        trackball_.set_camera(cam, image_height);
    }

    /**
     * Set a camera to view the scene from.  The current viewer dimensions are assumed
     * to be the camera's resolution, which will define it's field-of-view,
     * which will remain fixed until another call to set_camera.  In practice, this 
     * means that resizing the window will rescale the rendering of the scene so
     * exactly the same amount of the world is visible.  
     *
     * @note height is used to fix the vertical field-of-view; horizontal field-of-view remains free, so increasing/reducing the window's width will reveal/clip some of the scene.
     */
    inline void set_camera(const kjb::Perspective_camera& cam)
    {
        trackball_.set_camera(cam, height_);
    }

    inline kjb::Perspective_camera get_camera() const
    {
        return trackball_.get_camera();
    }

    /**
     * @param destination_cam Camera to move to by linearly interpolating from the current camera
     * @param duration_msec Length of animation in milliseconds
     */
    inline void animate_camera(const kjb::Perspective_camera& destination_cam, size_t duration_msec)
    {
        camera_tween_ = Camera_tween();
        camera_tween_->src = trackball_.get_camera();
        camera_tween_->dest = destination_cam;
        camera_tween_->elapsed = 0;
        camera_tween_->duration = duration_msec;
    }

    /**
     * Set a camera from which to view the scene.  Unlike set_camera(), which fixes
     * the field-of-view after being called, this version lets the field-of-view
     * remain free.  In practice, this means that resizing the window won't change
     * the size of the rendered scene, but instead will show more of the scene
     * around the borders.
     */
    inline void set_camera_free_fovy(const kjb::Perspective_camera& cam)
    {
        trackball_.set_camera(cam);
    }

    void enable_anaglyph_stereo(
            double screen_distance, 
            double eye_separation,
            double pixels_per_meter,
            double world_scale = 1.0,
            bool swap_left_right = false)
    {

        // TODO: cleanup variable names, handle focal length better
        Stereo_mode stereo_mode = (swap_left_right ? anaglyph_swap_stereo : anaglyph_stereo);

        setup_stereo(stereo_mode, screen_distance, eye_separation, pixels_per_meter, world_scale);
    }


    void enable_horizontal_stereo(
            double screen_distance, 
            double eye_separation,
            double pixels_per_meter,
            double world_scale = 1.0)
    {
        // TODO: cleanup variable names, handle focal length better
#ifdef KJB_HAVE_GLUT
        glutFullScreen();
#endif

        setup_stereo(horizontal_stereo, screen_distance, eye_separation, pixels_per_meter, world_scale);
    }

    /**
     * Setup stereo mode for virtual-reality applications.  Camera focal length will be adjusted to reflect the 
     * human viewer's relationship to the screen.  Near and far plane will also be set to 0.25 m and 250 m, respectively
     *
     * @param mode Stereo mode. options are Viewer::horizontal_stereo, Viewer::anaglyph_stereo, and Viewer::anaglyph_swap_stereo.
     * @param screen_distance Distance in meters between the viewer and the screen; used to set the correct camera focal length.
     * @param eye_separation Distance in meters between viewer's eyes.  Average humans have an eye separation of 0.06 meters.  However, it's often advisable to set this lower than the "true" value; doing so will shorten the percieved depths in the scene, reducing eye strain when viewing scenes with extreme depths (e.g. more than twice or less than half the distance to the screen).
     * @param pixels_per_meter Relationship between pixel size and screen size, as a ratio of pixels-to-meters.  To get this, just divide the screen's width in meters by the horizontal screen resolution.
     */
    void setup_stereo(
            Stereo_mode mode,
            double screen_distance,
            double eye_separation,
            double pixels_per_meter,
            double world_scale = 1.0)
    {
        stereo_mode_ = mode;

        stereo_eye_offset_ = eye_separation/2.0;
        stereo_pixel_offset_ = pixels_per_meter * stereo_eye_offset_;
        double stereo_focal_length = screen_distance * pixels_per_meter; 

        Perspective_camera cam = trackball_.get_camera();
        cam.set_focal_length(stereo_focal_length);
        cam.set_near(0.25);
        cam.set_far(250);
        cam.set_world_scale(world_scale);
        set_camera(cam);
    }

    /// reset camera to last call of set_camera
    void reset_camera()
    {
        trackball_.reset();
    }

    /************* BACKGROUND SETTINGS ***********/
    void set_background_gradient()
    {
        bg_mode_ = gradient_bg;
    }
    
    void set_background_gradient(const kjb::Vector4& bottom, const kjb::Vector4& top)
    {
        bg_mode_ = gradient_bg;
        bg_gradient_bot_ = bottom;
        bg_gradient_top_ = top;
    }

    void set_background_solid(const kjb::Vector4& color)
    {
        bg_mode_ = solid_bg;
        bg_color_ = color;
    }

    /**
     * Specify an image to show as the background of the viewer. 
     *
     * Image will be streched to screen size.
     *
     * @warning Opengl context must be initialized before calling this (e.g. by constructing a Glut_window)
     */
    void set_background_image(const kjb::Image& img)
    {
        bg_mode_ = image_bg;
        bg_image_ = boost::in_place();
        bg_image_->set(img);

        bg_image_->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
                        GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
                        GL_LINEAR);
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
        bg_image_->unbind();
    }

    /**
     * Specify an image to show as the background of the viewer using an existing
     * texture.  
     *
     *
     * Image will be streched to screen size.
     *
     * @warning Opengl context must be initialized before calling this (e.g. by constructing a Glut_window)
     *
     * @note due to the Texture object's built-in reference semantics, the texture data won't be copied; it will create a reference of itself which will be stored in this class.  Texture will be freed when no references continue to exist.
     */
    void set_background_image(const kjb::opengl::Texture& img)
    {
        bg_mode_ = image_bg;
        bg_image_.reset(kjb::opengl::Texture(img));
    }

    /********************************************
     * Rotation origin 
     *
     * Moving the rotation origin lets you,
     * rotate around a specific center point.  Since
     * being able to temporarilly move the rotation 
     * center is a common use-case, a push/pop version
     * is provided, but the stack has maximum depth of 1,
     * so be careful!
     */

    /// @brief set point to rotate around in world coordinates
    void set_rotation_origin(const Vector3& o)
    {
        trackball_.set_object_origin(o);
    }

    /// @brief gets the rotation point in world coordinates
    Vector3 get_rotation_origin() const
    {
        return trackball_.get_object_origin();
    }

    /** 
     * EXPERIMENTAL 
     *
     * modify rotation origin, and save old rotation center.
     * Old rotation center can be restored by a later call to 
     * pop_rotation center.
     *
     * @warning the "stack" has maximum depth of 1, so calling this
     * multiple times without calling pop in-between will throw an
     * exception
     *
     * @throws Stack_overflow
     */
    void push_rotation_origin(const Vector3& o)
    {
         // EXPERIMENTAL 
        if(old_rotation_origin_)
            KJB_THROW(Stack_overflow);
        old_rotation_origin_ = trackball_.get_object_origin();
        trackball_.set_object_origin(o);
    }

    /**
     * EXPERIMENTAL 
     * Restore rotation origin previously saved by a call to 
     * push_rotation_origin.
     *
     * @throw Stack_underflow
     */
    void pop_rotation_origin()
    {
         // EXPERIMENTAL 
        if(!old_rotation_origin_)
            KJB_THROW(Stack_underflow);
        trackball_.set_object_origin(*old_rotation_origin_);

        old_rotation_origin_ = boost::none;
    }

    /** 
     * Render the rotation center in world coordinates.
     */
    void render_rotation_origin()
    {
         // EXPERIMENTAL 
        trackball_.render_object_origin();
    }

    void draw()
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glClearColor(bg_color_[0], bg_color_[1], bg_color_[2], bg_color_[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_background();

        if(stereo_mode_ != no_stereo)
            draw_stereo();
        else
            draw_dispatch(trackball_.get_camera());

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }

    void draw_background()
    {
        switch(bg_mode_)
        {
            case gradient_bg:
                draw_gradient_background();
                break;
            default:
            case solid_bg:
                break;
            case image_bg:
                draw_image_background();
                break;
        }
    }

    void draw_gradient_background()
    {
        
        glPushAttrib(GL_ENABLE_BIT);
        glPushAttrib(GL_CURRENT_BIT);
        glDisable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glBegin(GL_QUADS);
        glColor3ub(127, 127, 255);
        glVertex2i(-1, -1);
        glVertex2i( 1, -1);
        glColor3ub(0, 0, 0);
        glVertex2i( 1,  1);
        glVertex2i(-1,  1);
        glEnd();

        glPopAttrib();
        glPopAttrib();
    }


    void draw_image_background()
    {
        assert(bg_image_);
        
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable( GL_TEXTURE_2D);
        bg_image_->bind();

        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex2i(-1, -1);
        glTexCoord2f(1.0, 0.0); glVertex2i( 1, -1);
        glTexCoord2f(1.0, 1.0); glVertex2i( 1,  1);
        glTexCoord2f(0.0, 1.0); glVertex2i(-1,  1);
        glEnd();

        bg_image_->unbind();

        glPopAttrib();
    }

    void draw_stereo()
    {
        Perspective_camera center = trackball_.get_camera();
        // ... TODO
        Perspective_camera left = center;
        Perspective_camera right = center;

        Vector cur_center = center.get_world_origin();
        cur_center.resize(3);

        // These need to go eleswhere
        //static size_t w = 1400 / 2;
        //static size_t h = 1050;
        //static const double W = 2.4384;

        //double delta = 0.03;    //what is this?
        //double pixel_delta = delta * w/W;

        left.set_world_origin(cur_center + Vector(stereo_eye_offset_, 0.0, 0.0));
        left.set_principal_point(Vector(-stereo_pixel_offset_, 0));

        right.set_world_origin(cur_center + Vector(-stereo_eye_offset_, 0.0, 0.0));
        right.set_principal_point(Vector(stereo_pixel_offset_, 0));

        glPushAttrib(GL_ENABLE_BIT);
        glPushAttrib(GL_VIEWPORT_BIT);
        glDisable(GL_SCISSOR_TEST);

        if(stereo_mode_ == anaglyph_stereo)
            glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
        else if(stereo_mode_ == anaglyph_swap_stereo)
            glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
        else if(stereo_mode_ == horizontal_stereo)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, 0, width_/2.0, height_);
            glViewport(0, 0, width_/2.0, height_);
        }
        else
        {
            KJB_THROW(Not_implemented);
        }

        GL_ETX();

        draw_dispatch(left);

        if(stereo_mode_ == anaglyph_stereo)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
        else if(stereo_mode_ == anaglyph_swap_stereo)
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
        }
        else if(stereo_mode_ == horizontal_stereo)
        {
            glScissor(width_/2.0, 0, width_/2.0, height_);
            glViewport(width_/2.0, 0, width_/2.0, height_);
        }
        else
        {
            KJB_THROW(Not_implemented);
        }


        GL_ETX();

        draw_dispatch(right);

        if(stereo_mode_ == anaglyph_stereo || 
           stereo_mode_ == anaglyph_swap_stereo)
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        GL_ETX();

        glPopAttrib();
        glPopAttrib();
    }

    void draw_dispatch(const kjb::Perspective_camera& cam)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glScalef(resize_ratio_, resize_ratio_, 1.0);
        cam.mult_projection_matrix();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        cam.mult_modelview_matrix();

        if(display_origin_)
            render_rotation_origin();
//        trackball_.render();


        size_t i = 0;
        BOOST_REVERSE_FOREACH(Callback& render_cb, render_callbacks_)
        {
            if(render_visibility_[&render_cb])
                render_cb();
            ++i;
        }

        BOOST_REVERSE_FOREACH(const boost::shared_ptr<Selectable> selectable, selectables_)
        {
            selectable->render(false);
        }

        // this modifies modelview and projection 
        render_overlays_();
    }

#ifdef KJB_HAVE_GLUT
    void display()
    {
        draw();
        glutSwapBuffers();
    }
#endif

    int width()
    {
        // possibly not best to let trackball be
        // the "keeper of the dimensions", since
        // the viewer seems like it should be the 
        // owner.  But also not good to store
        // the same values in 2 places. Probably need to
        // to rethink design at some point...
//        return trackball_.viewport_width();

        // EXPERIMENTAL: let viewer own its width
        return width_;
    }

    int height()
    {
//        return trackball_.viewport_height();

        // EXPERIMENTAL: let viewer own its height
        return height_;
    }

    void reshape(int width, int height)
    {
        glViewport(0, 0, width, height);

        width_ = width;
        height_ = height;

        resize_ratio_ = std::min(((double) width_)/base_width_, ((double)height_) / base_height_);

        if(stereo_mode_ == horizontal_stereo)
        {
            // stereo_mode implies side-by-side display
            // ensures fovy is set correctly if fixed_fovy mode is set
            trackball_.update_viewport(0, 0, width/2.0, height);
        }
        else
        {
            trackball_.update_viewport(0, 0, width, height);
        }

        BOOST_FOREACH(Overlay* overlay, overlay_attachments_)
        {
            overlay->set_size(width, height);
        }

        BOOST_REVERSE_FOREACH(Reshape_callback& reshape_cb, reshape_callbacks_)
        {
            reshape_cb(width, height);
        }
    }

    /**
     * Returns the number of renderables associated with this viewer
     */
    size_t num_renderables()
    {
        return render_callbacks_.size();
    }

    /**
     * Add kjb::Renderable object, store by reference
     */
    Render_callback_iterator add_renderable(const kjb::Renderable* renderable)
    {
        return add_render_callback_dispatch_(boost::bind(&kjb::Renderable::render, renderable));
    }

    /**
     * Add kjb::Renderable object, store by shared reference
     */
    Render_callback_iterator add_renderable(const boost::shared_ptr<kjb::Renderable>& renderable)
    {
        return add_render_callback_dispatch_(boost::bind(&kjb::Renderable::render, renderable));
    }

    /**
     * Add kjb::Renderable object, copy and store by value
     */
    template <class RenderableType>
    typename boost::enable_if<boost::is_convertible<RenderableType*, kjb::Renderable*>, Render_callback_iterator>::type
    copy_renderable(const RenderableType& renderable)
    {
        BOOST_CONCEPT_ASSERT((boost::Convertible<RenderableType*, kjb::Renderable*>));

        return add_render_callback_dispatch_(boost::bind(&kjb::Renderable::render, renderable));
    }

    /**
     * Add a render callback function that receives nothing and returns null.
     * Use this when your design doesn't
     * comply with kjb::Renderable base class, or RenderableObject 
     * concept.
     *
     * Callback will be copied into viewer class.  To avoid a copy,
     * pass a pointer to your function object, which will call a specialized
     * version that stores a reference.  Alternatively, wrap your object in
     * boost::cref and pass that here.
     */
    template <class Callback_type>
    Render_callback_iterator add_render_callback(Callback_type callback)
    {
        return add_render_callback_dispatch_(callback);
    }

    template <class Callback_type>
    Render_callback_iterator add_render_callback(boost::reference_wrapper<Callback_type> callback)
    {
        return add_render_callback_dispatch_(boost::bind(callback));
    }

    Render_callback_iterator add_render_callback(const Callback& callback)
    {
        return add_render_callback_dispatch_(boost::bind(callback));
    }

    Render_callback_iterator add_render_callback_dispatch_(const Callback& cb)
    {
        render_callbacks_.push_front(cb);
        render_visibility_[&render_callbacks_.front()] = true;
        return render_callbacks_.begin();
    }

    /**
     * Add a render callback function.  Use this when your design doesn't
     * comply with kjb::Renderable base class, or RenderableObject 
     * concept.
     *
     * This specialization stores the callback by reference, so it won't copy the callback object
     */
    template <class Callback>
    Render_callback_iterator add_render_callback(const Callback* callback)
    {
        // TODO this fails in clang if _callback_ is a function pointer.  should dispatch this using boost::is_function
        return add_render_callback(boost::cref(*callback));
    }

    /**
     * Remove a previously-added render callback
     */
    void remove_render_callback(Render_callback_iterator it)
    {
        render_visibility_.erase(&*it);
        render_callbacks_.erase(it);
    }


    /* Add a reshpae callback function to the end of the callback queue */
    Reshape_callback_iterator add_reshape_callback(const Reshape_callback& cb)
    {
        reshape_callbacks_.push_front(cb);
        return reshape_callbacks_.begin();
    }

    /**************************************************
     * Overlay management.
     *
     * This is a bit more difficult than with renderables. since the
     * Renderable interface consists of only a single method we can
     * wrap it in a boost::bind and store it in a collection of
     * boost::function0's.  This also let us accept objects that are
     * extrinsically renderable (i.e. render(obj) ), and render callbacks
     *
     * Overlay interface has multiple methods, so it makes sense to 
     * rely on polymorphism in this case, i.e.  store a
     * collection of pointers to Overlay base objects.  However, when 
     * the user wants to copy the object into the viewer, we need to
     * store the copied object in a list of boost::any's and then
     * store a pointer to that storage.
     */

    /**
     * Add an overlay by value.  Object is copied into this class and added
     * to the overlay list.
     *
     *
     * @return An interator to the inserted item in the overlay list.
     */
    template <class Overlay2dType>
    Overlay_iterator copy_overlay(const Overlay2dType& overlay)
    {
        overlay_storage_.push_front(overlay);

        Overlay_iterator item = add_overlay(&boost::any_cast<Overlay2dType&>(overlay_storage_.front()));

        // keep track of item so if it is removed later, we can free the associated memory
        overlay_ownership_[&**item] = overlay_storage_.begin();
        return item;
    }

    /**
     * Add overlay and lock it's dimensions to the window's dimensions.
     */
    Overlay_iterator attach_overlay(Overlay* overlay)
    {
        overlay->set_position(0,0);
        overlay->set_size(width(), height());
        Overlay_iterator it = add_overlay(overlay);
        overlay_attachments_.insert(overlay);

        return it;
    }

    /**
     * Add an item to the overlay list by-pointer.  Ownership is retained
     * by the caller; caller must ensure the item survives beyond the
     * lifetime of the viewer object. Item will not be deleted when this
     * clas is destroyed.
     *
     * @return An interator to the inserted item in the overlay list.
     */
    Overlay_iterator add_overlay(Overlay* overlay)
    {
        // prevent deleter from being called, since we don't own
        // this object
        overlays_.push_front(Overlay_ptr(overlay, null_deleter()));
        return overlays_.begin();
    }

    /**
     * Add an item to the overlay list.
     * @return An interator to the inserted item in the overlay list.
     */
    Overlay_iterator add_overlay(Overlay_ptr overlay_ptr)
    {
        overlays_.push_front(overlay_ptr);
        return overlays_.begin();
    }

    /**
     * Add an item to the overlay list.
     * @return An interator to the inserted item in the overlay list.
     */
    Overlay_iterator add_overlay_callback(const boost::function0<void>& callback, int x = 0, int y = 0, int width = -1, int height = -1)
    {
        return copy_overlay(Overlay_callback_wrapper(x, y, width, height, callback));
    }

    /**
     *  Remove an overlay from the overlay list.
     * @return An interator to the inserted item in the overlay list.
     */
    void remove_overlay(Overlay_iterator it)
    {
        if(overlay_ownership_.count(&**it))
            overlay_storage_.erase(overlay_ownership_[&**it]);
        overlay_attachments_.erase(&**it);
        overlays_.erase(it);
    }

    /**
     * Move an overlay to the top render layer, so it isn't occluded by anything.
     *
     * @note iterator will be invalid after calling this.  Use return value to keep track of new location.
     *
     * @return An interator to the new location of the item
     */
    Overlay_iterator raise_overlay(Overlay_iterator it)
    {
        if(it == overlays_.begin()) return it;

        overlays_.splice(overlays_.begin(), overlays_, it);

        return overlays_.begin();
    }

    ////////////////////////////////////////////////////
    // VISIBILITY MANAGEMENT
    //
    // Control the visibility of each renderable item
    ////////////////////////////////////////////////////
    
    /// Set the visibility of all "standard" renderables (not overlays)
    void set_renderable_visibility(bool v)
    {
        BOOST_FOREACH(Render_visibility_map::value_type& pair, render_visibility_)
        {
            pair.second = v;
        }
    }

    /// Set the visibility of a renderable, using the hande returned from add_renderable()
    bool get_renderable_visibility(Render_callback_const_iterator it) const
    {
        if(render_visibility_.count(&*it) == 0)
            KJB_THROW(kjb::Index_out_of_bounds);

        return render_visibility_.at(&*it);
    }

    /// Get the visibility of a renderable, using it's index (as defined by its order in the render queue)
    bool get_renderable_visibility(size_t i) const
    {
        if(i >= render_callbacks_.size())
            KJB_THROW(kjb::Index_out_of_bounds);

        // (renderables are stored in reverse order)
        i = render_callbacks_.size() - i - 1;
        Render_callback_const_iterator it = render_callbacks_.begin();
        std::advance(it, i);
        return get_renderable_visibility(it);
    }

    /// Set the visibility of a renderable, using it's handle 
    void set_renderable_visibility(Render_callback_iterator it, bool v)
    {
        if(render_visibility_.count(&*it) == 0)
            KJB_THROW(kjb::Index_out_of_bounds);

        render_visibility_[&*it] = v;
    }

    /// Set the visibility of a renderable, using it's index (as defined by its order in the render queue) 
    void set_renderable_visibility(size_t i, bool v)
    {
        if(i >= render_callbacks_.size())
            KJB_THROW(kjb::Index_out_of_bounds);

        // (renderables are stored in reverse order)
        i = render_callbacks_.size() - i - 1;

        Render_callback_iterator it = render_callbacks_.begin();
        std::advance(it, i);
        set_renderable_visibility(it, v);
    }

    ////////////////////////////////////////////////////
    //  WINDOWS
    ////////////////////////////////////////////////////
    
    Window_iterator add_window(Window* window)
    {
        return add_window(boost::shared_ptr<Window>(window, null_deleter()));
    }

    /**
     * @note adding an existing window will simply move window to top of stack
     */
    Window_iterator add_window(boost::shared_ptr<Window> window)
    {
        // TODO: remove window if already there
        Window_iterators its;

        // if already in stack, remove and move to top
        if(windows_.count(window.get()))
            remove_window(window.get());

        its.overlay = add_overlay(boost::static_pointer_cast<Overlay>(window));
        its.events = add_event_listener(boost::static_pointer_cast<Event_listener>(window));

        // add window to front of stack, old top window must yield focus
        window_stack_.front()->yield_focus();
        window_stack_.push_front(window);
        window->claim_focus();

        its.window_stack = window_stack_.begin();

        // add window
        typedef Windows::value_type Pair;
        std::pair<Window_iterator, bool> item = windows_.insert(Pair(window.get(), its));
        assert(item.second == true);  // always insert, not overwrite

        // when close button is pressed, call viewer's remove_window() method
        window->set_close_callback(boost::bind(
            static_cast<void (Self::*)(const Window_iterator)>(&Self::remove_window),
            this, item.first));

        // when window loses focus, tell me to raise it in the render and callback stack
        window->set_clicked_callback(boost::bind(
            static_cast<void (Self::*)(const Window_iterator)>(&Self::raise_window),
            this, item.first));

        return item.first;
    }

    void remove_window(const Window_iterator window)
    {
        remove_event_listener(window->second.events);
        remove_overlay(window->second.overlay);
        window_stack_.erase(window->second.window_stack);
        windows_.erase(window);

#ifdef TEST
        std::cerr << "Window removed. Number of windows: " << windows_.size() << std::endl;
#endif
    }

    void remove_window(boost::shared_ptr<Window> window)
    {
        remove_window(window.get());
    }

    void remove_window(Window* window)
    {
        Window_iterator item = windows_.find(window);
        if(item == windows_.end())
            KJB_THROW_2(Illegal_argument, "window not found");
        remove_window(item);
    }

    void raise_window(Window_iterator window)
    {
        raise_overlay(window->second.overlay);
        raise_event_listener(window->second.events);

        // move window to top of stack.  Old top yields focus,
        // and this window claims focus
        window_stack_.front()->yield_focus();
        window_stack_.splice(
                window_stack_.begin(),
                window_stack_,
                window->second.window_stack);
        window->second.window_stack = window_stack_.begin();
        window_stack_.front()->claim_focus();

        redisplay();
    }

    /*******************************************************************
     * Interaction.
     *
     * Objects can receive interaction events in two ways:
     *  1. Inherit from Event_listener and call viewer.attach(&obj)
     *  2. Add listener callbacks to the viewer: 
     *  @code
     *  void Object::attach(Viewer& viewer)
     *  {
     *      viewer.add_mouse_listener(boost::bind(&Object::click, this, _1, _2, _3, _4));
     *      viewer.add_keyboard_listener(...)
     *      ...
     *  }
     *  @endcode
     *      
     * There's advantages to both; in approach 1. the object needs to know
     * nothing about the viewer.  In Approach 2, the viewer needs to know
     * nothing about the object.
     */

    /**
     * Add this listener to the front of the listener queue.  
     * Adding a listener you've already added will remove it from the
     * queue and add it to the front.
     */
    Event_listener_iterator add_event_listener(boost::shared_ptr<Event_listener> listener)
    {
        // see if user wants to move existing events to the top
        if(event_listeners_.count(listener.get()))
            remove_event_listener(listener.get());

//        add_selectable(boost::static_pointer_cast<Selectable>(listener)); // this also makes it render

        // add each listener to the callback lists, store the iterators
        // so we can remove them later.
        Event_listener_iterators iterators;
        iterators.mouse = add_mouse_listener(
                boost::bind(
                    &Event_listener::mouse_event,
                    listener,
                    _1,
                    _2,
                    _3,
                    _4));
        iterators.mouse_double = add_mouse_double_listener(
                boost::bind(
                    &Event_listener::mouse_double_event,
                    listener,
                    _1,
                    _2,
                    _3,
                    _4));
        iterators.motion = add_motion_listener(
                boost::bind(
                    &Event_listener::motion_event,
                    listener,
                    _1,
                    _2));
        iterators.passive_motion = add_passive_motion_listener(
                boost::bind(
                    &Event_listener::passive_motion_event,
                    listener,
                    _1,
                    _2));
        iterators.keyboard = add_keyboard_listener(
                boost::bind(
                    &Event_listener::keyboard_event,
                    listener,
                    _1,
                    _2,
                    _3));
        iterators.special = add_special_listener(
                boost::bind(
                    &Event_listener::special_event,
                    listener,
                    _1,
                    _2,
                    _3));

        typedef Event_listeners::value_type Pair;
        std::pair<Event_listeners::iterator, bool> item = event_listeners_.insert(Pair(listener.get(), iterators));
        assert(item.second == true);  // always insert, not overwrite
        return item.first;
    }

    /**
     * Add this listener to the front of the listener queue.  
     * Adding a listener you've already added will remove it from the
     * queue and add it to the front.
     */
    inline Event_listener_iterator add_event_listener(Event_listener* listener)
    {
        boost::shared_ptr<Event_listener> ptr_wrapper(listener, null_deleter());
        return add_event_listener(ptr_wrapper);
    }

    /** 
     * Remove a previously-added event listener.
     * @param listener An iterator to the listener, which was returned from
     * add_event_listener.
     */
    void remove_event_listener(const Event_listener_iterator listener)
    {
        remove_mouse_listener(listener->second.mouse);
        remove_mouse_double_listener(listener->second.mouse_double);
        remove_motion_listener(listener->second.motion);
        remove_passive_motion_listener(listener->second.passive_motion);
        remove_keyboard_listener(listener->second.keyboard);
        remove_special_listener(listener->second.special);
//        remove_selectable(listener->second.selectable);
        event_listeners_.erase(listener);
    }

    /** 
     * Remove a previously-added event listener.
     * @param listener An pointer to the listener to remove. Will throw Illegal_argument exception if listener isn't present
     */
    void remove_event_listener(boost::shared_ptr<Event_listener> listener)
    {
        remove_event_listener(listener.get());
    }

    /** 
     * Remove a previously-added event listener.
     * @param listener An pointer to the listener to remove. Will throw Illegal_argument exception if listener isn't present
     */
    void remove_event_listener(Event_listener* listener)
    {
        Event_listeners::iterator item = event_listeners_.find(listener);
        if(item == event_listeners_.end())
            KJB_THROW_2(Illegal_argument, "listener not found");
        remove_event_listener(item);
    }

    void raise_event_listener(Event_listener_iterator listener)
    {
        front_mouse_listeners_.splice(
                front_mouse_listeners_.begin(),
                front_mouse_listeners_,
                listener->second.mouse);
        listener->second.mouse = front_mouse_listeners_.begin();

        mouse_double_listeners_.splice(
                mouse_double_listeners_.begin(),
                mouse_double_listeners_,
                listener->second.mouse_double);
        listener->second.mouse_double = mouse_double_listeners_.begin();

        motion_listeners_.splice(
                motion_listeners_.begin(),
                motion_listeners_,
                listener->second.motion);
        listener->second.motion = motion_listeners_.begin();

        passive_motion_listeners_.splice(
                passive_motion_listeners_.begin(),
                passive_motion_listeners_,
                listener->second.passive_motion);
        listener->second.passive_motion = passive_motion_listeners_.begin();

        keyboard_listeners_.splice(
                keyboard_listeners_.begin(),
                keyboard_listeners_,
                listener->second.keyboard);
        listener->second.keyboard = keyboard_listeners_.begin();

        special_listeners_.splice(
                special_listeners_.begin(),
                special_listeners_,
                listener->second.special);
        listener->second.special = special_listeners_.begin();
    }

#ifdef KJB_HAVE_GLUT
    template <class Callback>
    void add_timer(const Callback& cb, size_t msec, bool redraw = false)
    {
        boost::function0<void> f;
        f = cb;
        timers_.push_back(Timer_entry(f, msec, redraw, this));
        glutTimerFunc(msec, Self::tick_timer, timers_.size()-1);
    }
#endif

    void redisplay()
    {
#ifdef KJB_HAVE_GLUT
        glutPostRedisplay();
#endif
    }

    void key(unsigned int key, int , int )
    {
        switch(key)
        {
            case ' ':
                reset_camera();
                redisplay();
                break;
            case 'o':
                display_origin_ = !display_origin_;
                redisplay();
            default:
                break;
        }
    }


    /**
     * Add a listener for mouse clicks.  These will be processed before
     * object selection/picking occurs.  If the mouse listener consumes the
     * click event, selection/picking won't occur.  If you want to add a "fall-through"
     * click listener thats processed *after* the selection/picking,
     * use add_after_mouse_listener.
     *
     * @sa add_after_mouse_listener
     */
    Mouse_listener_iterator 
    add_mouse_listener( 
            const boost::function4<bool, int, int, int, int>& callback)
    {
        front_mouse_listeners_.push_front(callback);
        return front_mouse_listeners_.begin();
    }

    /**
     * Add a listener for mouse clicks that only occurs if all other mouse clicks
     * fail to consume the click event.  Most importantly, this occurs *after*
     * selection/picking, if no item was successfully picked.  This is useful
     * for "default" click events, e.g. the trackball mouse rotation, which
     * should only occur if nothing else is clicked.
     */
    Mouse_listener_iterator 
    add_after_mouse_listener( 
            const boost::function4<bool, int, int, int, int>& callback)
    {
        back_mouse_listeners_.push_front(callback);
        return back_mouse_listeners_.begin();
    }

    /**
     * Add a listener for mouse double-clicks.
     */
    Mouse_listener_iterator 
    add_mouse_double_listener( 
            const boost::function4<bool, int, int, int, int>& callback)
    {
        mouse_double_listeners_.push_front(callback);
        return mouse_double_listeners_.begin();
    }

    /**
     * Add a listener for mouse motion ("drag") events
     */
    Motion_listener_iterator 
    add_motion_listener( 
            const boost::function2<bool, int, int>& callback)
    {
        motion_listeners_.push_front(callback);
        return motion_listeners_.begin();
    }

    /**
     * Add a listener for mouse passive motion ("hover") events
     */
    Motion_listener_iterator 
    add_passive_motion_listener( 
            const boost::function2<bool, int, int>& callback)
    {
        passive_motion_listeners_.push_front(callback);
        return passive_motion_listeners_.begin();
    }

    /**
     * Add a listener for keyboard events
     */
    Keyboard_listener_iterator 
    add_keyboard_listener( 
            const boost::function3<bool, unsigned int, int, int>& callback)
    {
        keyboard_listeners_.push_front(callback);
        return keyboard_listeners_.begin();
    }

    /**
     * Add a listener for keyboard events
     */
    Special_listener_iterator 
    add_special_listener( 
            const boost::function3<bool, int, int, int>& callback)
    {
        special_listeners_.push_front(callback);
        return special_listeners_.begin();
    }


// REMOVING LISTENERS

    void remove_mouse_listener( Mouse_listener_iterator item)
    {
        front_mouse_listeners_.erase(item);
    }

    void remove_after_mouse_listener(Mouse_listener_iterator item)
    {
        back_mouse_listeners_.erase(item);
    }

    /**
     * Add a listener for mouse double-clicks.
     */
    void remove_mouse_double_listener(Mouse_listener_iterator item)
    {
        mouse_double_listeners_.erase(item);
    }

    /**
     * Add a listener for mouse motion ("drag") events
     */
    void remove_motion_listener(Motion_listener_iterator item)
    {
        motion_listeners_.erase(item);
    }

    /**
     * Add a listener for mouse passive motion ("hover") events
     */
    void remove_passive_motion_listener(Motion_listener_iterator item)
    {
        passive_motion_listeners_.erase(item);
    }

    /**
     * Add a listener for keyboard events
     */
    void remove_keyboard_listener(Keyboard_listener_iterator item)
    {
        keyboard_listeners_.erase(item);
    }

    /**
     * Add a listener for keyboard events
     */
    void remove_special_listener(Special_listener_iterator item)
    {
        special_listeners_.erase(item);
    }

    /**
     * Add a selectable object. This object will be added to the render
     * list, and if user clicks on this object,
     * it will be notified by a callback to its "selection_hit" method.
     */
    Selectable_iterator
    add_selectable(
            boost::shared_ptr<Selectable> listener)
    {
        selectables_.push_front(listener);
        selectable_indices_.push_back(selectables_.begin());
        return selectables_.begin();
    }

    /**
     * Add a selectable object. This object will be added to the render
     * list, and if user clicks on this object,
     * it will be notified by a callback to its "selection_hit" method.
     */
    Selectable_iterator
    add_selectable(
            Selectable* listener)
    {
        return add_selectable(boost::shared_ptr<Selectable>(listener, null_deleter()));
    }

    void remove_selectable(Selectable_iterator item)
    {
        std::remove(
                selectable_indices_.begin(),
                selectable_indices_.end(),
                item);
        selectables_.erase(item);
    }

    /** 
     * Tick method for viewer's internal animation routines
     */
    void tick()
    {
        tick_camera_tween_();
    }

private:
    void process_mouse_(int button, int state, int x, int y)
    {
        // might eventually be able to use this with something besides glut,
        // but would need to replace all the glut macros with something more sensible
#ifdef KJB_HAVE_GLUT
        if(state == GLUT_DOWN)
        {
            static const double DBLCLICK_INTERVAL = 250;
            int cur_time = glutGet(GLUT_ELAPSED_TIME);
            int elapsed = cur_time  - click_time_;
            click_time_ = cur_time;
            if(elapsed < DBLCLICK_INTERVAL)
                return process_mouse_double_(button, state, x, y);
        }
#endif

        BOOST_REVERSE_FOREACH(const Mouse_listener& callback, front_mouse_listeners_)
        {
            if(callback(button, state, x, height() - y - 1)) 
                return;
        }

        if(selectables_.size() > 0)
        {
            if(process_selection_click_(button, state, x, height() - y - 1))
                return;
        }

        BOOST_REVERSE_FOREACH(const Mouse_listener& callback, back_mouse_listeners_)
        {
            if(callback(button, state, x, height() - y - 1)) 
                return;
        }
    }

    void process_mouse_double_(int button, int state, int x, int y)
    {
        BOOST_REVERSE_FOREACH(const Mouse_listener& callback, mouse_double_listeners_)
        {
            if(callback(button, state, x, height() - y - 1)) 
                return;
        }
    }

    void process_motion_(int x, int y)
    {
        BOOST_REVERSE_FOREACH(const Motion_listener& callback, motion_listeners_)
        {
            if(callback(x, height() - y - 1)) 
                return;
        }
    }

    void process_passive_motion_(int x, int y)
    {
        BOOST_REVERSE_FOREACH(const Motion_listener& callback, passive_motion_listeners_)
        {
            if(callback(x, height() - y - 1)) 
                return;
        }
    }

    void process_keyboard_(unsigned int k, int x, int y)
    {
        BOOST_REVERSE_FOREACH(const Keyboard_listener& callback, keyboard_listeners_)
        {
            if(callback(k, x, height() - y - 1)) 
                return;
        }

        this->key(k, x, height() - y - 1);
    }

    // TODO: process_special_


    bool process_selection_click_(int button, int state, int x, int y)
    {
        static const size_t BUFSIZE = 512;
        GLuint selectBuf[BUFSIZE];
        GLint hits;

        glSelectBuffer (BUFSIZE, selectBuf);
        (void) glRenderMode (GL_SELECT);
    
        glInitNames();
        glPushName(0);

        int MARGIN = 5;

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        trackball_.prepare_for_picking(x, y, MARGIN, MARGIN);

        // render selection
        size_t i = 0;
        // objects are stored in reverse
        BOOST_REVERSE_FOREACH(boost::shared_ptr<Selectable> selectable, selectables_)
        {
            glLoadName(i++);
            glPushName(0);
            selectable->render(true);
            glPopName();
        }

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glFlush ();

        hits = glRenderMode (GL_RENDER);

        if(hits == 0)
            return false;

        // todo: pass mouse button and state 
        return process_selection_hits_(hits, selectBuf, button, state);
    }

    bool process_selection_hits_(GLint hits, GLuint buffer[], int button, int state)
    {
        GLuint* ptr = buffer;

        GLuint num_names;
        float near_depth;
        float far_depth;
        GLuint name;

        // dispatch hit info to the clicked item
        for(int i = 0; i < hits; i++)
        {
            num_names = *ptr++; 
            near_depth =  (float) *ptr++/0x7fffffff;
            far_depth = (float) *ptr++/0x7fffffff;
            name = *ptr++;

            GLuint* next = ptr + num_names-1;
            assert(name < selectable_indices_.size());
            bool processed = (*selectable_indices_[name])->selection_hit(ptr, next, button, state);

            if(processed)
                return true;
            ptr = next;
        }

        return false;
    }

#ifdef KJB_HAVE_GLUT
    /**
     * Advance an externally-specified timer
     */
    static void tick_timer(int i)
    {
        const Timer_entry& t = timers_[i];
        // execute callback
        t.f();
    
        if(t.redraw)
        {
            t.viewer->redisplay();
        }

        // restart timer
        glutTimerFunc(t.period, Self::tick_timer, i);
    }
#endif

    /**
     * Tick camera animation if one is in-progress.
     */
    void tick_camera_tween_()
    {
        // TODO: currently only animates extrinsic parameters.  Eventually intrinsic parameter tweening would be nice
        if(camera_tween_)
        {
            camera_tween_->elapsed += TICK_PERIOD;
            if(camera_tween_->elapsed > camera_tween_->duration)
            {
                // done tweening
                set_camera(camera_tween_->dest);

//                set_extrinsic_matrix(camera_tween_->dest.get_modelview_matrix());
                camera_tween_.reset(); 
            }
            else
            {
                // linearly interpolate between cameras' intrinsic matrices
                static const bool USE_SLERP = false;

                set_extrinsic_matrix( 
                    lerp_extrinsic_camera_matrix(
                            camera_tween_->src.get_modelview_matrix(),
                            camera_tween_->dest.get_modelview_matrix(),
                            (double)(camera_tween_->elapsed) / camera_tween_->duration,
                            USE_SLERP));
            }
            redisplay();
        }
    }



private:
    /**
     * @post modelview and projection matrices will be modified (caller is responsible for saving/restoring)
     * */
    void render_overlays_()
    {
        GL_ETX();

        if(overlays_.size() == 0) return;

        glPushAttrib(GL_ENABLE_BIT); // for GL_DEPTH_TEST

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_DEPTH_TEST); // we'll use the painter algorithm instead.

        // set up othographic projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        // We want glVertex2d(0.0, 0.0) to represent the bottom-left of the
        // viewport, wherever it is. Therefore...
        // DO THIS:
        ::glOrtho(0, width(), 0, height(), -1, 1); 
        // NOT THIS:
        // glOrtho(vp_x, vp_y, vp_width, vp_height); 

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // render overlays
        Overlay_list::reverse_iterator it;
        for(it = overlays_.rbegin(); it != overlays_.rend(); ++it)
        {
            (*it)->render();
        }

        glPopAttrib();

        GL_ETX();
    }

private:
    // PRIVATE FIELDS

    Trackball trackball_;

    int click_time_;

    bool display_origin_;

    Callback_list render_callbacks_;
    Reshape_callback_list reshape_callbacks_;
    Render_visibility_map render_visibility_;

    std::list<Overlay_ptr> overlays_;
    Overlay_storage_list overlay_storage_;
    Overlay_ownership_map overlay_ownership_;
    std::set<Overlay*> overlay_attachments_;

    Event_listeners event_listeners_;
    Windows windows_;
    Window_stack window_stack_;

    Mouse_listeners front_mouse_listeners_;
    Mouse_listeners back_mouse_listeners_;
    Mouse_listeners mouse_double_listeners_;
    Motion_listeners motion_listeners_;
    Motion_listeners passive_motion_listeners_;
    Keyboard_listeners keyboard_listeners_;
    Special_listeners special_listeners_;
    Selectables selectables_;
    std::vector<Selectable_iterator> selectable_indices_;

    boost::optional<Vector3> old_rotation_origin_;

    size_t base_width_;
    size_t base_height_;

    size_t width_;
    size_t height_;

    float resize_ratio_;

    enum Bg_mode { solid_bg, gradient_bg, image_bg };

    Bg_mode bg_mode_;
    kjb::Vector4 bg_color_;
    kjb::Vector4 bg_gradient_bot_;
    kjb::Vector4 bg_gradient_top_;
    boost::optional<kjb::opengl::Texture> bg_image_;

    // ANIMATION STATE
    boost::optional<Camera_tween> camera_tween_;

    // stereo
    Stereo_mode stereo_mode_;
    double stereo_eye_offset_;
    double stereo_pixel_offset_;

    // timers
    static std::vector<Timer_entry> timers_;
    static const size_t TICK_PERIOD;

};

} // namespace opengl 
} // namespace kjb


namespace kjb
{
namespace opengl
{
    // for backward compatibility (temporary)
    using kjb::gui::Viewer;
}
}
#endif /* KJB_HAVE_OPENGL */
#endif

// TODO
// Move trackball functionality out of class; use dependency injection to add as-neded
