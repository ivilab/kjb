/* $Id: psi_visualize.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "psi_cpp/psi_visualize.h"

namespace kjb
{
namespace psi
{
using namespace kjb::pt; 

void render_ground_plane()
{
    glBegin(GL_LINES);

    for(int x = -1000; x < 1000; x+=1)
    {
        glVertex3f(x, 0, -1000);
        glVertex3f(x, 0, 1000);
    }

    for(int z = -1000; z < 1000; z+=5)
    {
        glVertex3d(-1000, 0, z);
        glVertex3d(1000, 0, z);
    }

    glEnd();
}

#ifdef KJB_HAVE_GLUT
void Track_visualizer::set_trajectories(const Trajectory_map& trajectories)
{
    trajectories_ = trajectories;

    colors_.clear();

    typedef Trajectory_map::value_type Traj_pair;
    BOOST_FOREACH(const Traj_pair& pair, trajectories_)
    {
        Vector3 color(create_random_vector(3));
        color.normalize();
        colors_[pair.first].resize(1, color);
    }
}

void Track_visualizer::render(const Trajectory_map& trajectories, const std::map<Entity_id, std::vector<Vector3> >& colors) const
{
    using namespace kjb;
    using namespace kjb::opengl;
    for(int pass = 0; pass <= 1; pass++)
    {
        if(pass == 0)
        {
            glLineWidth(4.0);
            glPointSize(5.0);
        }
        else
        {
            glLineWidth(2.0);
            glPointSize(4.0);
        }

        typedef std::pair<Entity_id, Trajectory> Traj_pair;
        size_t i = 0;
        BOOST_FOREACH(const Traj_pair& traj_pair, trajectories)
        {
            const Entity_id& id = traj_pair.first;
            const Trajectory& traj = traj_pair.second;

            if(pass == 0)
                render(traj, cur_frame_, 15);
            else
                render(traj, cur_frame_, 15, colors.at(id));
            i++;
        }
    }
}

void Track_visualizer::render(
        const Trajectory& traj,
        int center,
        int radius,
        const std::vector<Vector3>& colors) const
{
    using namespace kjb::opengl;

    size_t begin = std::max(0, center - radius);
    size_t end = std::min((int) traj.size()-1, center + radius);

    Vector3 color1 = Vector3(0.0);
    Vector3 color2 = Vector3(0.0);
    Vector3 center_color = Vector3(0.0);

    if(colors.size() == 1)
    {
        color1 = color2 = center_color = colors[0];

    }
    else if(colors.size() > 1)
    {
        center_color = colors[center];
    }

    // make center color brighter
    if(colors.size() > 0)
    {
        static const float alpha = 0.75;
        center_color = alpha * Vector3(1.0) + (1.0 - alpha) * center_color;
    }

    glBegin(GL_LINES);
    for(size_t i = begin; i <= end-1; i++)
    {
        if(!traj[i]) continue;
        if(!traj[i+1]) continue;

        if(colors.size() > 1)
        {
            color1 = colors[i];
            color2 = colors[i+1];

        }
            glColor(color1);
            glVertex(traj[i]->value.position);
            glColor(color2);
            glVertex(traj[i+1]->value.position);
    }
    glEnd();

    if(traj[center])
    {
        glColor(center_color);
        glBegin(GL_POINTS);
            glVertex(traj[center]->value.position);
        glEnd();
    }
}


void Ground_truth_track_visualizer::set_trajectories(const Trajectory_map& ground_truth, const Trajectory_map& hypothesis)
{
    if(ground_truth.duration() != hypothesis.duration())
    {
        KJB_THROW_2(Illegal_argument, "ground truth and hypothesis trajectories must have same duration");
    }

    Base::set_trajectories(ground_truth);
    hyp_trajectories_ = hypothesis;

    metrics::init_correspondence(ground_truth, hypothesis, threshold_, correspondences_);

    static const Vector3 NULL_COLOR(0.5);

    // initialize all colors to null
    typedef Trajectory_map::value_type Traj_pair;
    BOOST_FOREACH(const Traj_pair& pair, hyp_trajectories_)
    {
        const Entity_id& id = pair.first;
        // initialize with blank color
        hyp_colors_[id].clear();
        hyp_colors_[id].resize(hyp_trajectories_.duration(), NULL_COLOR);
    }

    ASSERT(correspondences_.size() == hyp_trajectories_.duration());
    for(size_t frame = 0; frame < correspondences_.size(); frame++)
    {
        typedef boost::bimap<Entity_id, Entity_id>::left_const_reference Corr_pair;

        const boost::bimap<Entity_id, Entity_id>& corr = correspondences_[frame];

        BOOST_FOREACH(Corr_pair pair, corr.left)
        {
            const Entity_id& gt_id = pair.first;
            const Entity_id& hyp_id = pair.second;


            // gt colors should all be initialized by now
            ASSERT(gt_colors().count(gt_id) > 0);
            // gt should only have one color per track
            ASSERT(gt_colors().at(gt_id).size() == 1);

            // take color of corresponding point
            hyp_colors_[hyp_id][frame] = gt_colors().at(gt_id)[0];
        }


    }

}


void Ground_truth_track_visualizer::render_correspondence_lines() const
{
    using namespace kjb::opengl;

    const boost::bimap<Entity_id, Entity_id>& corr = correspondences_[cur_frame_];

    typedef boost::bimap<Entity_id, Entity_id>::left_const_reference Corr_pair;

    glLineWidth(1.0);
    glBegin(GL_LINES);
    BOOST_FOREACH(Corr_pair pair, corr.left)
    {
        const Entity_id& gt_id = pair.first;
        const Entity_id& hyp_id = pair.second;

        const Trajectory& gt_traj = gt_trajectories().at(gt_id);
        const Trajectory& hyp_traj = hyp_trajectories_.at(hyp_id);

        ASSERT(gt_traj[cur_frame_]);
        ASSERT(hyp_traj[cur_frame_]);

        // the fact that there's a correspondence implies that gt trajectory exists)
        ASSERT(gt_colors().count(gt_id));

        glColor(gt_colors().at(gt_id)[0]);

        glVertex(gt_traj[cur_frame_]->value.position);
        glVertex(hyp_traj[cur_frame_]->value.position);
    }
    glEnd();
}


void Cylinder_world_visualizer::display()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


    // draw frame image
    size_t cur_sprite = cur_frame_ / frame_thinning_;
    if(cur_sprite < frame_sprites_.size())
    {
        frame_sprites_[cur_sprite].enable_depth_test(false);
        frame_sprites_[cur_sprite].render();
    }

    // set up projection and modelview matrix from camera.
    cam_.prepare_for_rendering(false);

    glColor3f(1.0, 0.3, 0.3);
    draw_xz_plane();

    // draw cylinders
    for(size_t type = 0; type < NUM_ENTITY_TYPES; type++)
    {
        for(size_t i = 0; i < entities_frames_[type].size(); i++)
        {
            glColor3f(0.3, 1.0, 0.3);
            ASSERT(cur_frame_ < entities_frames_[type][i].size() );
            std::cout << "entity #" << i << " " << entities_frames_[type][i][cur_frame_].get_position() << " " << entities_frames_[type][i][cur_frame_].get_heading() << std::endl;
            entities_frames_[type][i][cur_frame_].render();
        }

//        // draw boxes
//        for(size_t i = 0; i < cuboids_frames_.size(); i++)
//        {
//            glColor4f(1.0, 1.0, 1.0, 0.5);
//            ASSERT(cur_frame_ < cuboids_frames_[i].size());
//            render(cuboids_frames_[i][cur_frame_]);
//#ifdef TEST
////            std::vector<Vector> corners = get_corners(cuboids_frames_[i][cur_frame_]);
////
////            glPointSize(4.0);
////            glBegin(GL_POINTS);
////            for(size_t c = 0; c < corners.size(); c++)
////            {
////                ::kjb::opengl::glVertex(corners[c]);
////            }
////            glEnd();
//#endif
//        }

//            // if user has specified bounding boxes to draw, draw them
//            if(frames_boxes_.size() > 0 && frames_boxes_[type].size() > 0 )
//            {
//    //            glLineWidth(3);
//                glColor3f(0.3, 0.3, 1.0);
//                size_t cur_box = cur_frame_ / box_thinning_;
//
//                if(cur_box < frames_boxes_[type].size())
//                {
//                    glMatrixMode(GL_PROJECTION);
//                    glLoadIdentity();
//                    gluOrtho2D(-(width_/2.0), width_/2.0, -(height_/2.0), height_/2.0);
//                    glMatrixMode(GL_MODELVIEW);
//                    glLoadIdentity();
//                    for(size_t i = 0; i < frames_boxes_[type][cur_box].size(); i++)
//                    {
//                        std::cout << "boxes #" << i << " " << frames_boxes_[type][cur_box][i].get_center() << std::endl;
//                        frames_boxes_[type][cur_box][i].wire_render();
//                    }
//
//                }
//                else
//                {
//                    // model trajectory extends beyond extent of video clip
//                }
//
//    //            glLineWidth(1);
//            }
    }

    if(post_render_callback_)
    {
        post_render_callback_();
    }

    GL_ETX();

    glutSwapBuffers();
}


void Cylinder_world_visualizer::animate()
{
    if(animating_)
    {
        using namespace boost;
        cur_frame_++;
        cur_frame_ = cur_frame_ % num_frames_;
        std::cout << "animating frame " << cur_frame_ << std::endl;

        wnd_.redisplay();
        init_animate_callback_();
    }
    else
    {
        std::cout << "not animating frame " << cur_frame_ << std::endl;

    }
}

void Cylinder_world_visualizer::key(unsigned char k, int , int )
{
    switch(k)
    {
        case ' ':
            animating_ = !animating_;
            wnd_.redisplay();
            if(animating_)
                init_animate_callback_();
            break;
        case '>':
            animating_ = false;
            cur_frame_++;
            cur_frame_ = cur_frame_ % num_frames_;
            wnd_.redisplay();
            std::cout << "current frame: " << cur_frame_ << std::endl;
            break;
        case '<':
            animating_ = false;
            cur_frame_ += num_frames_-1;
            cur_frame_ = cur_frame_ % num_frames_;
            wnd_.redisplay();
            std::cout << "current frame: " << cur_frame_ << std::endl;
            break;
        default:
            break;
    }
}

void Cylinder_world_visualizer::set_entities(const std::vector<std::vector<std::vector<Entity_state> > >& entities)
{
    entities_frames_ = entities;
    num_frames_ = 0;

    for(size_t t = 0; t < entities.size(); t++)
    {
        if(entities[t].size() > 0)
        {
            num_frames_ = entities[t][0].size();
            break;
        }
    }

    if(num_frames_ == 0)
    {
        KJB_THROW_2(Illegal_argument, "Entity set is empty.");
    }
}

void Cylinder_world_visualizer::draw_xz_plane()
{
    glBegin(GL_LINES);

    for(int x = -1000; x < 1000; x+=1)
    {
        glVertex3f(x, 0, -1000);
        glVertex3f(x, 0, 1000);
    }

    for(int z = -1000; z < 1000; z+=5)
    {
        glVertex3d(-1000, 0, z);
        glVertex3d(1000, 0, z);
    }

    glEnd();
}
#endif

} // namespace psi
} // namespace kjb
