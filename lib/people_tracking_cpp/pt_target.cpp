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
* =========================================================================== */

/* $Id: pt_target.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_body_2d_trajectory.h"
#include "people_tracking_cpp/pt_util.h"
#include "m_cpp/m_vector.h"
#include "camera_cpp/perspective_camera.h"
#include "camera_cpp/camera_backproject.h"
#include "gp_cpp/gp_base.h"
#include "gp_cpp/gp_normal.h"
#include "gp_cpp/gp_posterior.h"
#include "gp_cpp/gp_covariance.h"
#include "gp_cpp/gp_mean.h"
#include "prob_cpp/prob_stat.h"
#include "flow_cpp/flow_integral_flow.h"
#include "detector_cpp/d_deva_facemark.h"

#include <vector>
#include <utility>
#include <algorithm>
#include <boost/none.hpp>
#include <boost/bind.hpp>
#include <boost/optional.hpp>

using namespace kjb;
using namespace kjb::pt;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Target::estimate_trajectory
(
    const Perspective_camera& cam,
    double nsx,
    double nsz
) const
{
    typedef Target::const_iterator Tci;

    size_t sz = traj.size();

    // if empty, trajectory must be empty as well
    if(empty())
    {
        traj.clear();
        traj.resize(sz);

        return;
    }

    size_t sf = get_start_time();
    size_t ef = get_end_time();

    // kill everything before first frame
    for(size_t u = 0; u < sf - 1; ++u)
    {
        traj[u] = boost::none;
    }

    // kill everything after last frame
    for(size_t u = ef; u < sz; ++u)
    {
        traj[u] = boost::none;
    }

    // if there is nothing new
    if(!changed()) return;

    // estimate bottoms in changed range
    size_t rsf = changed_start();
    size_t ref = changed_end();

    // this estimate only uses box bottoms
    Ground_back_projector back_project(cam, 0.0);
    size_t num_set = 0;
    for(size_t t = rsf; t <= ref; ++t)
    {
        if(traj[t - 1])
        {
            ++num_set;
            continue;
        }

        Vector new_x(3, 0.0);
        size_t total = 0;

        // average back-projected box bottoms
        std::pair<Tci, Tci> rg = this->equal_range(t);
        for(Tci pr_p = rg.first; pr_p != rg.second; ++pr_p)
        {
            Vector pt = back_project(
                pr_p->second->bbox.get_bottom_center()[0],
                pr_p->second->bbox.get_bottom_center()[1]);

            if(!pt.empty())
            {
                new_x += pt;
                total++;
            }
        }

        if(total == 0)
        {
            traj[t - 1] = boost::none;
        }
        else
        {
            new_x /= total;
            traj[t - 1] = Trajectory_element();
            traj[t - 1]->value.position = new_x;
            ++num_set;
        }
    }

    // smooth the trajectory
    double scale = gp_pos_prior.covariance_function().scale();
    double sgvar = gp_pos_prior.covariance_function().signal_sigma();
    Vector res_x = smooth_trajectory(
                                rsf, ref,
                                std::vector<double>(num_set, nsx),
                                scale, sgvar,
                                boost::bind(&Target::get_position, this, _1, 0),
                                boost::bind(&Target::get_zero, this, _1));

    Vector res_z = smooth_trajectory(
                                rsf, ref,
                                std::vector<double>(num_set, nsz),
                                scale, sgvar,
                                boost::bind(&Target::get_position, this, _1, 2),
                                boost::bind(&Target::get_zero, this, _1));

    // set it
    for(size_t t = rsf; t <= ref; ++t)
    {
        if(!traj[t - 1]) traj[t - 1] = Trajectory_element();
        traj[t - 1]->value.position[0] = res_x[t - rsf];
        traj[t - 1]->value.position[2] = res_z[t - rsf];
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Target::estimate_trajectory
(
    const Perspective_camera& cam,
    const std::vector<Integral_flow>& x_flows,
    const std::vector<Integral_flow>& y_flows,
    const std::vector<Integral_flow>& back_x_flows,
    const std::vector<Integral_flow>& back_y_flows,
    size_t frame_rate,
    double nsx,
    double nsz
) const
{
    size_t sz = traj.size();

    // if empty, trajectory must be empty as well
    if(empty())
    {
        traj.clear();
        traj.resize(sz);

        return;
    }

    size_t sf = get_start_time();
    size_t ef = get_end_time();

    // kill everything before first frame
    for(size_t u = 0; u < sf - 1; ++u)
    {
        traj[u] = boost::none;
    }

    // kill everything after last frame
    for(size_t u = ef; u < sz; ++u)
    {
        traj[u] = boost::none;
    }

    // if there is nothing new
    if(!changed()) return;

    // estimate bottoms in changed range
    size_t rsf = changed_start();
    size_t ref = changed_end();

    // Propagate 2D boxes using flow
    size_t duration = std::floor(frame_rate / 2.0);
    typedef std::map<int, std::vector<Propagated_2d_info> > P2i_map;
    P2i_map prop_info = propagate_boxes_by_flow(
        *this, x_flows, y_flows, back_x_flows, back_y_flows, duration);

    // Sort the boxes by y_bottom and compute the weighted average of x
    std::map<size_t, Vector> new_2d;
    P2i_map::const_iterator it = prop_info.begin();
    for(; it != prop_info.end(); ++it)
    {
        size_t frame = it->first;
        std::vector<Propagated_2d_info> info = it->second;
        std::vector<Propagated_2d_info>::const_iterator iter;
        Gaussian_distribution P(frame, duration/3.0);
        double x = 0.0;
        double total_weights = 0.0;
        for(iter = info.begin(); iter != info.end(); ++iter)
        {
            total_weights += pdf(P, iter->origin_frame);
        }

        //double norm = 1.0 / (pdf(P, frame + duration)/total_weights);
        for(iter = info.begin(); iter != info.end(); iter++)
        {
            x += ((pdf(P, iter->origin_frame) * iter->x) / total_weights);
        }

        std::sort(info.begin(), info.end(), Comparator_2d());
        int mid_index = std::ceil(info.size() * 0.5);
        double y_bottom = info[mid_index - 1].y_bottom;
        new_2d[frame] = Vector(x, y_bottom);
    }

    // now back project points computed above to get trajectory points
    size_t num_set = 0;
    Ground_back_projector back_project(cam, 0.0);
    for(size_t u = rsf - 1; u <= ref - 1; u++)
    {
        size_t t = u + 1;

        std::map<size_t, Vector>::const_iterator vp = new_2d.find(t);
        if(vp == new_2d.end())
        {
            traj[u] = boost::none;
            continue;
        }

        const Vector& pos_2d = vp->second;
        Vector new_x = back_project(pos_2d[0], pos_2d[1]);
        if(new_x.empty())
        {
            traj[u] = boost::none;
            continue;
        }

        traj[u] = Trajectory_element();
        traj[u]->value.position = new_x;
        ++num_set;
    }

    // smooth the trajectory
    double scale = gp_pos_prior.covariance_function().scale();
    double sgvar = gp_pos_prior.covariance_function().signal_sigma();
    Vector res_x = smooth_trajectory(
                                rsf, ref,
                                std::vector<double>(num_set, nsx),
                                scale, sgvar,
                                boost::bind(&Target::get_position, this, _1, 0),
                                boost::bind(&Target::get_zero, this, _1));

    Vector res_z = smooth_trajectory(
                                rsf, ref,
                                std::vector<double>(num_set, nsz),
                                scale, sgvar,
                                boost::bind(&Target::get_position, this, _1, 2),
                                boost::bind(&Target::get_zero, this, _1));

    // set it
    for(size_t t = rsf; t <= ref; ++t)
    {
        traj[t - 1] = Trajectory_element();
        traj[t - 1]->value.position[0] = res_x[t - rsf];
        traj[t - 1]->value.position[2] = res_z[t - rsf];
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Target::estimate_height(const Perspective_camera& cam) const
{
    typedef Target::const_iterator Tci;

    size_t sf = get_start_time();
    size_t ef = get_end_time();

    Ground_back_projector back_project(cam, 0.0);
    std::vector<double> heights_3d;
    for(size_t u = sf - 1; u <= ef - 1; u++)
    {
        int t = u + 1;

        std::pair<Tci, Tci> rg = this->equal_range(t);
        for(Tci pr_p = rg.first; pr_p != rg.second; pr_p++)
        {
            Vector pt = back_project(
                pr_p->second->bbox.get_bottom_center()[0],
                pr_p->second->bbox.get_bottom_center()[1]);

            // ignore the height if the bottom of the box is not
            // on the ground
            if(pt.empty()) continue;
            heights_3d.push_back(get_3d_height(
                pr_p->second->bbox.get_bottom_center(),
                pr_p->second->bbox.get_top_center(),
                cam));
        }
    }

    // get median (in linear time)
    if(!heights_3d.empty()) // Added by Jinyan
    {
        size_t mid_index = heights_3d.size() / 2;
        std::nth_element(
                heights_3d.begin(),
                heights_3d.begin() + mid_index,
                heights_3d.end());
        traj.height = heights_3d[mid_index];
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Target::estimate_directions(bool infer_head, double nsb) const
{
    // although this should not be called if there is no changed,
    // we should probably still check for it
    if(!changed()) return;
    size_t sf = changed_start();
    size_t ef = changed_end();

    // update the walking angles
    //update_walking_angles();

    size_t num_set = 0;
    for(int t = sf; t <= ef; ++t)
    {
        /*traj[t - 1]->value.body_dir = atraj[t - 1]->value;
        traj[t - 1]->value.face_dir[0] = 0.0;
        traj[t - 1]->value.face_dir[1] = 0.0;
        continue;*/
        if(traj[t - 1]->value.body_dir != std::numeric_limits<double>::max())
        {
            ++num_set;
            // at the beginning need to check if the previous walking direction is set
            if(t == sf && t - 2 >= 0 && atraj[t - 2])
            {
                //std::cout << "t: " << t << " uses previous walking dir\n";
                traj[t - 1]->value.body_dir = closest_angle(
                                                atraj[t - 2]->value,
                                                traj[t - 1]->value.body_dir);
            }
            else if(t >= sf + 1)
            {
                ASSERT(t - 2 >= 0);
                if(traj[t - 2]->value.body_dir != std::numeric_limits<double>::max())
                { 
                    ASSERT(traj[t - 2]);
                    traj[t - 1]->value.body_dir = closest_angle(
                                                    traj[t - 2]->value.body_dir,
                                                    traj[t - 1]->value.body_dir);
                    //std::cout << "t: " << t << " uses previous body dir\n";
                }
                else
                {
                    ASSERT(atraj[t - 2]);
                    traj[t - 1]->value.body_dir = closest_angle(
                                                    atraj[t - 2]->value,
                                                    traj[t - 1]->value.body_dir);
                    //std::cout << "t: " << t << " uses previous walking dir\n";
                }
            }
        }
        else
        {
            ASSERT(traj[t - 1]->value.face_dir[0]
                    == std::numeric_limits<double>::max());
            ASSERT(traj[t - 1]->value.face_dir[1]
                    == std::numeric_limits<double>::max());
        }
    }

    double scale = gp_dir_prior.covariance_function().scale();
    double sgvar = gp_dir_prior.covariance_function().signal_sigma();
    Vector res_b = smooth_trajectory(
                            sf, ef,
                            std::vector<double>(num_set, nsb),
                            scale, sgvar,
                            boost::bind(&Target::get_bdir, this, _1),
                            //boost::bind(&Target::get_zero, this, _1));
                            boost::bind(&Target::get_walking_angle, this, _1));
    if(res_b.empty())
    {
        for(int t = sf; t <= ef; ++t)
        {
            // set the body dir according to the previous walking 
            if(t - 2 >= 0 && traj[t - 2])
            {
                ASSERT(traj[t - 2]->value.body_dir != std::numeric_limits<double>::max());
                traj[t - 1]->value.body_dir = closest_angle(traj[t - 2]->value.body_dir, 0.0);
            }
            else
            {
                traj[t - 1]->value.body_dir = 0.0;
            }
            traj[t - 1]->value.face_dir[0] = 0.0;
        }
    }
    else
    {
        for(size_t t = sf; t <= ef; ++t)
        {
            traj[t - 1]->value.body_dir = res_b[t - sf];
            traj[t - 1]->value.face_dir[0] = 0.0;
        }
    }

    scale = gp_fdir_prior.covariance_function().scale();
    sgvar = gp_fdir_prior.covariance_function().signal_sigma();
    Vector res_f = smooth_trajectory(
                            sf, ef,
                            std::vector<double>(num_set, nsb),
                            scale, sgvar,
                            boost::bind(&Target::get_fdir, this, _1, 1),
                            boost::bind(&Target::get_zero, this, _1));

    if(res_f.empty())
    {
        for(size_t t = sf; t <= ef; ++t)
        {
            traj[t - 1]->value.face_dir[1] = 0.0;
        }
    }
    else
    {
        for(size_t t = sf; t <= ef; ++t)
        {
            traj[t - 1]->value.face_dir[1] = res_f[t - sf];
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Target::update_boxes(const Perspective_camera& cam) const
{
    size_t sf = get_start_time();
    size_t ef = get_end_time();

    if(btraj.size() != traj.size())
    {
        btraj.resize(traj.size());
    }

    for(size_t t = 1; t < sf; t++)
    {
        btraj[t - 1] = boost::none;
    }

    size_t sz = btraj.size();
    for(size_t t = ef + 1; t <= sz; t++)
    {
        btraj[t - 1] = boost::none;
    }

    if(!changed()) return;

    for(size_t t = changed_start(); t <= changed_end(); t++)
    {
        btraj[t - 1] = Body_2d_trajectory_element();
        btraj[t - 1]->value = project_cstate(
                                        traj[t - 1]->value,
                                        cam,
                                        traj.height,
                                        traj.width,
                                        traj.girth);

        if(t != ef)
        {
            btraj[t - 1]->value.model_dir = model_direction(
                                                    traj[t - 1]->value,
                                                    traj[t]->value,
                                                    cam,
                                                    traj.height,
                                                    traj.width,
                                                    traj.girth);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Target::update_faces(const Perspective_camera& cam) const
{
    size_t sf = get_start_time();
    size_t ef = get_end_time();

    if(ftraj.size() != traj.size())
    {
        ftraj.resize(traj.size());
    }

    for(size_t t = 1; t < sf; t++)
    {
        ftraj[t - 1] = boost::none;
    }

    size_t sz = ftraj.size();
    for(size_t t = ef + 1; t <= sz; t++)
    {
        ftraj[t - 1] = boost::none;
    }

    if(!changed()) return;

    for(size_t t = changed_start(); t <= changed_end(); t++)
    {
        ftraj[t - 1] = Face_2d_trajectory_element();
        ftraj[t - 1]->value = project_cstate_face(
                                            traj[t - 1]->value,
                                            cam,
                                            traj.height,
                                            traj.width,
                                            traj.girth);

        if(t != ef)
        {
            ftraj[t - 1]->value.model_dir = face_model_direction(
                                                    traj[t - 1]->value,
                                                    traj[t]->value,
                                                    cam,
                                                    traj.height,
                                                    traj.width,
                                                    traj.girth);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*void Target::update_walking_angles() const
{
    size_t sf = get_start_time();
    size_t ef = get_end_time();

    if(atraj.size() != traj.size())
    {
        atraj.resize(traj.size());
    }

    for(size_t t = 1; t < sf; t++)
    {
        atraj[t - 1] = boost::none;
    }

    size_t sz = atraj.size();
    for(size_t t = ef + 1; t <= sz; t++)
    {
        atraj[t - 1] = boost::none;
    }

    if(!changed()) return;

    for(int t = changed_start(); t < ef; t++)
    {
        atraj[t - 1] = Angle_trajectory_element();
        atraj[t - 1]->value = get_initial_direction(traj, t);

        // if the prev exists, adjust the current so that the difference
        // is less than PI
        if(t - 2 >= 0 && atraj[t - 2])
        {
            atraj[t - 1] = closest_angle(atraj[t - 2]->value, 
                                         atraj[t - 1]->value);
        }
    }
    // update the walking direction of the last frame
    atraj[ef - 1] = Angle_trajectory_element();
    atraj[ef - 1]->value = atraj[ef - 2]->value;
}*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Target::update_walking_angles() const
{
    size_t sf = get_start_time();
    size_t ef = get_end_time();

    if(atraj.size() != traj.size())
    {
        atraj.resize(traj.size());
    }

    for(size_t t = 1; t < sf; t++)
    {
        atraj[t - 1] = boost::none;
    }

    size_t sz = atraj.size();
    for(size_t t = ef + 1; t <= sz; t++)
    {
        atraj[t - 1] = boost::none;
    }

    if(!changed()) return;

    if(sf == ef)
    {
        std::cerr << "Warning: track only has one trajectory,\n";
        std::cerr << "can't compute the walking angle, initialize it to 0.0.\n";
        atraj[sf - 1] = Angle_trajectory_element();
        atraj[sf - 1]->value= 0.0;
    }

    const double thresh_dist = 0.1;
    size_t prev_time = changed_start();

    for(int t = changed_start() + 1; t < ef; t++)
    {
        double dist = vector_distance(traj[prev_time - 1]->value.position,
                traj[t - 1]->value.position);
        if(dist >= thresh_dist)
        {
            double angle = get_initial_direction(traj, prev_time, t);
            // if the prev exists, adjust the current so that the difference
            // is less than PI
            if(atraj[prev_time - 1])
            {
                angle = closest_angle(atraj[prev_time - 1]->value, angle);
            }
            for(size_t j = t; j >= prev_time; j--)
            {
                atraj[j - 1] = Angle_trajectory_element();
                atraj[j - 1]->value = angle;
            }
            prev_time = t;
        }
    }
    // if nothing has been initiliazed 
    if(prev_time == changed_start())
    {
        ASSERT(prev_time != ef);
        atraj[prev_time - 1] = Angle_trajectory_element();
        atraj[prev_time - 1]->value= get_initial_direction(traj, prev_time, prev_time + 1);
    }

    // update the walking direction from pref_time to ef 
    double prev_angle = atraj[prev_time - 1]->value;
    for(size_t t = prev_time; t <= ef; t++)
    {
        atraj[t - 1] = Angle_trajectory_element();
        atraj[t - 1] = prev_angle;
    }

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Target::update_gp(Gpp& gpp, double sc, double sv) const
{
    if(this->empty())
    {
        return;
    }

    if(gpp.covariance_function().scale() != sc
            || gpp.covariance_function().signal_sigma() != sv)
    {
        gpp.set_covariance_function(gp::Sqex(sc, sv));
    }

    size_t len = get_end_time() - get_start_time() + 1;
    size_t gp_len = std::distance(gpp.in_begin(), gpp.in_end());

    if(len != gp_len)
    {
        gp::Inputs X = gp::make_inputs(get_start_time(), get_end_time());
        gpp.set_inputs(X.begin(), X.end());
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Target::closest_angle(double prev, double cur) const
{
    // if cur > prev by more than PI, decrease cur
    double diff = cur - prev;
    while(diff > M_PI)
    {
        cur -= 2.0 * M_PI;
        diff = cur - prev;
    }

    // if cur < prev by more than PI, increase cur
    diff = prev - cur;
    while(diff > M_PI)
    {
        cur += 2.0 * M_PI;
        diff = prev - cur;
    }

    ASSERT(fabs(cur - prev) <= M_PI);

    return cur;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::sync_state(const Target& tg)
{
    for(size_t f = 0; f < tg.get_start_time() - 1; ++f)
    {
        tg.trajectory()[f] = boost::none;
        tg.body_trajectory()[f] = boost::none;
        tg.face_trajectory()[f] = boost::none;
    }

    for(size_t f = tg.get_end_time(); f < tg.trajectory().size(); ++f)
    {
        tg.trajectory()[f] = boost::none;
        tg.body_trajectory()[f] = boost::none;
        tg.face_trajectory()[f] = boost::none;
    }
}

