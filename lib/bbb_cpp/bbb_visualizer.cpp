/* =========================================================================== *
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#include "bbb_cpp/bbb_visualizer.h"
#include "bbb_cpp/bbb_data.h"
#include "bbb_cpp/bbb_trajectory.h"
#include "bbb_cpp/bbb_physical_activity.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "gr_cpp/gr_glut.h"
#include "gr_cpp/gr_primitive.h"
#include "camera_cpp/perspective_camera.h"
#include "l_cpp/l_exception.h"
#include "video_cpp/video_player.h"
#include "m_cpp/m_vector.h"

#include <algorithm>
#include <vector>
#include <iterator>
#include <iostream>

using namespace kjb;
using namespace kjb::bbb;
using namespace kjb::opengl;


void Visualizer::clear_alternative_camera()
{
    if(cam_p_ == 0)
    {
        std::cerr << "Warning: cannot clear alternative camera; "
                  << "true camera not set." << std::endl;
        return;
    }

    alt_camera_ = boost::none;
}

void Visualizer::draw_images(bool di)
{
    if(di && (video_.size() == 0 || alt_camera_))
    {
        std::cerr << "Warning: cannot render frames; images not set "
                  << "or true camera not set." << std::endl;
        draw_images_ = false;
        return;
    }

    draw_images_ = di;
}

void Visualizer::set_overhead_view()
{
    using namespace std;

    // get max x and z
    const Data& data = *data_p_;
    vector<double> temp(data.size());

    // get min for x
    transform(data.begin(), data.end(), temp.begin(), trajectory_min_x);
    double xmin = *min_element(temp.begin(), temp.end());

    // get max for x
    transform(data.begin(), data.end(), temp.begin(), trajectory_max_x);
    double xmax = *max_element(temp.begin(), temp.end());

    // get min for z
    transform(data.begin(), data.end(), temp.begin(), trajectory_min_z);
    double zmin = *min_element(temp.begin(), temp.end());

    // get max for z
    transform(data.begin(), data.end(), temp.begin(), trajectory_max_z);
    double zmax = *max_element(temp.begin(), temp.end());

    // this center gives a FOV of ~PI/2
    double cx = (xmax + xmin) / 2;
    double cy = std::max(zmax - zmin, xmax - xmin) / 2;
    double cz = (zmax + zmin) / 2;

    alt_camera_ = Perspective_camera(0.1, 10000);
    alt_camera_->set_camera_centre(Vector(cx, cy, cz));
    alt_camera_->set_focal_length(std::max(width(), height()) / 2);
    alt_camera_->set_roll(0.0);
    alt_camera_->set_yaw(0.0);
    alt_camera_->set_pitch(M_PI/2);

    draw_images(false);
}

void Visualizer::init_gl()
{
#ifdef KJB_HAVE_OPENGL
    // general stuff
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);

    // display callback
    glwin_.set_display_callback(
        boost::bind(&Visualizer::render_scene, this));
#else // KJB_HAVE_OPENGL
        KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif // KJB_HAVE_OPENGL
}
/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Visualizer::render_scene() const
{
#ifdef KJB_HAVE_OPENGL
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glDisable(GL_LIGHTING);

    // render the background image
    if(draw_images_)
    {
        IFT(video_.size() != 0, Runtime_error,
            "Cannot display frames; images not specified.");

        render_video_frame(video_, frame_);
    }

    if(alt_camera_) alt_camera_->prepare_for_rendering(false);
    else cam_p_->prepare_for_rendering(false);

    // render groups
    std::vector<const Physical_activity*> pas;
    desc_p_->physical_activities(std::back_inserter(pas));
    for(size_t i = 0; i < pas.size(); i++)
    {
        if(frame_ >= pas[i]->start() && frame_ <= pas[i]->end())
        {
            opengl::glColor(
                cur_colors_.insert(
                    std::make_pair(pas[i], random_color())).first->second);

            // render individual trajectories
            glLineWidth(1.0);
            const Traj_set& trajs = pas[i]->trajectories();
            for(size_t j = 0; j < trajs.size(); j++)
            {
                render_trajectory(data_p_->trajectory(j));
            }

            // render group trajectorie
            glLineWidth(3.0);
            render_trajectory(pas[i]->trajectory());
        }
    }

    // render trajectories
    //size_t J = data_p_->size();
    //for(size_t j = 0; j < J; j++)
    //{
    //    render_trajectory(j);
    //}

    glutSwapBuffers();
#else // KJB_HAVE_OPENGL
    KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif // KJB_HAVE_OPENGL
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Visualizer::render_trajectory(const Trajectory& traj) const
{
#ifdef KJB_HAVE_OPENGL
    //const Trajectory& traj = data_p_->trajectory(k);

    const size_t pf = frame_ > 20 ? frame_ - 20 : 1;
    const size_t t0 = std::max(traj.start(), pf);
    const size_t tn = std::min(traj.end(), frame_);

    glBegin(GL_LINE_STRIP);
    for(size_t t = t0; t <= tn; t++)
    {
        const Vector x = traj.pos(t);
        glVertex3d(x[0], 0.0, x[1]);
    }
    glEnd();

    opengl::Ellipse ellipse(0.1, 0.1);
    const Vector lx = traj.pos(tn);
    glPushMatrix();
        glTranslated(lx[0], 0, lx[1]);
        glRotated(90, 1.0, 0.0, 0.0);
        ellipse.solid_render();
    glPopMatrix();
#else // KJB_HAVE_OPENGL
    KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif // KJB_HAVE_OPENGL
}

