/* $Id: psi_visualize.h 18278 2014-11-25 01:42:10Z ksimek $ */
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

#ifndef KJB_PSI_VISUALIZE_V1_H
#define KJB_PSI_VISUALIZE_V1_H


#include <l_cpp/l_exception.h>
#include <m_cpp/m_vector_d.h>
#include <gr_cpp/gr_sprite.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_opengl.h>
#include <video_cpp/video_player.h>
#include <camera_cpp/perspective_camera.h>
#include <psi_cpp/psi_weighted_box.h>
#include <psi_cpp/psi_cylinder_world.h>
#include <psi_cpp/psi_model.h>
#include <psi_cpp/psi_util.h>
#include <psi_cpp/psi_metrics.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>

#include <fstream>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace kjb
{
namespace psi
{

#ifdef KJB_HAVE_GLUT
class Track_visualizer : public Video_player
{

typedef Track_visualizer Self;
typedef Video_player Base;

public:
    Track_visualizer() :
        Base(),
        trajectories_(),
        colors_()
    {}

    void set_camera(const Perspective_camera& cam)
    {
        cam_ = cam;
    }

    void set_trajectories(const pt::Trajectory_map& trajectories);

    virtual void render() const
    {
        Base::render();

        cam_.prepare_for_rendering(false);
        render(trajectories_, colors_);
    }

    /// render a set of trajectories, using the colors provided.
    /// @param colors A vector of colors to use at each frame.  Must have size() == trajectories.duration() or 1.  If size is 1, the same color is used for the entire track.
    void render(
            const kjb::pt::Trajectory_map& trajectories,
            const std::map<pt::Entity_id,
            std::vector<Vector3> >& colors) const;

    /// Render a framgent of a single trajectory
    /// @param center the current frame of the trajectory
    /// @param radius the number of frames before and after the current frame to render
    /// @param colors The colors for every frame of this trajectory.  if colors.size() == 1, use the same color for entire track.  Otherwise, size() must be eual to traj.size()
    void render(
            const pt::Trajectory& traj,
            int center,
            int radius,
            const std::vector<Vector3>& colors = std::vector<Vector3>()) const;

protected: 
    const std::map<pt::Entity_id, std::vector<Vector3> >&
    get_colors() const
    {
        return colors_;
    }
    
    const pt::Trajectory_map& get_trajectories() const
    {
        return trajectories_;
    }

private:
    pt::Trajectory_map trajectories_;
    Perspective_camera cam_;
    std::map<pt::Entity_id, std::vector<Vector3> > colors_;

};


class Ground_truth_track_visualizer : public Track_visualizer
{
typedef Ground_truth_track_visualizer Self;
typedef Track_visualizer Base;

public:
    Ground_truth_track_visualizer(double threshold) :
        Base(),
        hyp_trajectories_(),
        hyp_colors_(),
        correspondences_(),
        threshold_(threshold)
    {}

    /// Set a pair of trajectories.  Correspondences will be rendered between the two tracks
    void set_trajectories(const pt::Trajectory_map& ground_truth, const pt::Trajectory_map& hypothesis);

    /// return ground truth colors
    const std::map<pt::Entity_id, std::vector<Vector3> >&
    gt_colors() const
    {
        return get_colors();
    }

    /// return the ground truth trajectories
    const pt::Trajectory_map& gt_trajectories() const
    {
        return get_trajectories();
    }

    /// Render both tracks, with the hypothesis as a dotted-line.  The hypothesis color will be the same as the corresponding ground-truth (which may change during a track).  Lines between corresponding points will also be drawn.
    virtual void render() const
    {
        glDisable(GL_LINE_STIPPLE);
        Base::render();
        render_correspondence_lines();

        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x00ff); // 0000 0000 1111 1111
        Base::render(hyp_trajectories_, hyp_colors_);

    }

    void render_correspondence_lines() const;

private:
    pt::Trajectory_map hyp_trajectories_;
    std::map<pt::Entity_id, std::vector<Vector3> > hyp_colors_;
    std::vector<boost::bimap<pt::Entity_id, pt::Entity_id> > correspondences_;
    double threshold_;
};

class Cylinder_world_visualizer
{
typedef Cylinder_world_visualizer Self;

public:
    Cylinder_world_visualizer(double width, double height):
        cam_(),
        entities_frames_(),
        num_frames_(0),
        sample_period_(0.0),
        cur_frame_(0),
        width_(width),
        height_(height),
        wnd_(width, height),
        animating_(true),
        frame_sprites_(),
        frame_thinning_(1),
        frames_boxes_()
    {
        using namespace boost;

        wnd_.set_display_callback(bind(&Self::display, this));
        wnd_.set_keyboard_callback(bind(&Self::key, this, _1, _2, _3));

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_BLEND);
        glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
        glLineWidth(1.5);
    }


    Cylinder_world_visualizer(double width, double height, double sample_period):
        cam_(),
        entities_frames_(),
        num_frames_(0),
        sample_period_(sample_period *1000),
        cur_frame_(0),
        wnd_(width, height),
        animating_(true),
        frame_sprites_(),
        frame_thinning_(1),
        frames_boxes_()
    {
        using namespace boost;

        wnd_.set_display_callback(bind(&Self::display, this));
        wnd_.set_keyboard_callback(bind(&Self::key, this, _1, _2, _3));

        init_animate_callback_();
    }

    void set_frame_rate(double rate)
    {
        sample_period_ = rate;
        init_animate_callback_();
    }

    void set_frames(const std::vector<kjb::opengl::Sprite>& sprites, size_t thinning)
    {
        frame_sprites_ = sprites;
        frame_thinning_ = thinning;
    }

    void set_camera(const kjb::Perspective_camera& cam)
    {
        cam_ = cam;
    }

    void display();

    void animate();

    void key(unsigned char k, int , int );

    void set_entities(const std::vector<std::vector<std::vector<Entity_state> > >& entities);

    /**
     * Set bounding boxes to visualize, in format boxes[obj_type][frame][box]
     * @note this is the opposite indexing scheme than for entity states.
     * Need to refactor entity states to be indexed this way, but will require
     * refactoring this and cylinder_world, and cylinder_sampler
     */
    void set_boxes(const std::vector<std::vector<std::vector<Bbox> > >& boxes, size_t thinning = 1)
    {
        frames_boxes_ = boxes;
        box_thinning_ = thinning;
    }

    /**
     * Set a function to be called after rendering, but before swapping
     * the buffers.  Useful for rendering a frames to images.
     */
    void set_post_render_callback(const boost::function0<void> cb)
    {
        post_render_callback_ = cb;
    }
protected:
    void draw_xz_plane();

    void init_animate_callback_()
    {
        using namespace boost;

        kjb::opengl::Glut::add_timer_callback(sample_period_, bind(&Self::animate, this));
    }

protected:
    kjb::Perspective_camera cam_;
    std::vector<std::vector<std::vector<Entity_state> > > entities_frames_;
    size_t num_frames_;
//    std::vector<std::vector<Cuboid> > cuboids_frames_;
    size_t sample_period_; // in milliseconds

    size_t cur_frame_;
    double width_;
    double height_;
    kjb::opengl::Glut_window wnd_;
    bool animating_;

    std::vector<kjb::opengl::Sprite> frame_sprites_;
    size_t frame_thinning_;

    std::vector<std::vector<std::vector<Bbox> > > frames_boxes_;
    size_t box_thinning_;

    boost::function0<void> post_render_callback_;
};
#endif
void render_ground_plane();

} // namespace psi
} // namespace kjb

#endif
