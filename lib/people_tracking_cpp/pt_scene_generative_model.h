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
|     Ernesto Brau
|
* =========================================================================== */

/* $Id$ */

#ifndef PT_SCENE_GENERATIVE_MOODEL_H
#define PT_SCENE_GENERATIVE_MOODEL_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_direction_prior.h>
#include <people_tracking_cpp/pt_camera_prior.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_color_likelihood.h>
#include <people_tracking_cpp/pt_util.h>
#include <mcmcda_cpp/mcmcda_prior.h>
#include <flow_cpp/flow_integral_flow.h>
#include <camera_cpp/perspective_camera.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <gp_cpp/gp_sample.h>
#include <detector_cpp/d_bbox.h>
#include <detector_cpp/d_deva_facemark.h>
#include <vector>
#include <algorithm>
#include <functional>
#include <utility>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace kjb {
namespace pt {

/** @brief  Helper function that determines if a pixel is visible. */
inline
bool pixel_visible(int x, int y, const Visibility& vis)
{
    if(fabs(vis.visible - 0.0) < 1e-6) return false;
    if(fabs(vis.visible - 1.0) < 1e-6) return true;

    for(size_t i = 0; i < vis.visible_cells.size(); i++)
    {
        Bbox cell(vis.visible_cells[i], vis.cell_width, vis.cell_height);
        if(cell.contains(Vector().set(x, y))) return true;
    }

    return false;
}

/** @brief  Helper function; needed because, occasionally, C++ is stupid. */
inline bool empty_target(const Target& target) { return target.size() < 2; }

/** @brief  Helper function that fixes a track when it leaves the view. */
inline
void fix_target(Target& target, double imw, double imh)
{
    const Trajectory& traj = target.trajectory();
    const Body_2d_trajectory& btraj = target.body_trajectory();
    Bbox imbox(Vector(0.0, 0.0), imw, imh);
    for(Target::iterator pr_p = target.begin(); pr_p != target.end(); pr_p++)
    {
        size_t t = pr_p->first;

        const Vector3& pos = traj[t - 1]->value.position;
        const Bbox& fbox = btraj[t - 1]->value.full_bbox;
        if(pos[2] >= -1.0
            || get_rectangle_intersection(imbox, fbox) < 0.5*fbox.get_area())
        {
            // kill track from here on
            target.erase(pr_p, target.end());
            if(!empty_target(target))
            {
                sync_state(target);
            }
            return;
        }
    }
}

/**
 * @brief   Sample a scene from the prior (and a given sequence of tracks).
 *
 * This function samples the 3D configuration of a set of tracks by filling
 * in their trajectories. That is, it samples from p(z | w).
 */
template<class TargetIterator>
void sample
(
    const Position_prior& pos_prior,
    const Direction_prior& dir_prior,
    const Face_direction_prior& fdir_prior,
    const Normal_distribution& height_prior,
    const Normal_distribution& width_prior,
    const Normal_distribution& girth_prior,
    TargetIterator first,
    TargetIterator last
)
{
    for(TargetIterator target_p = first; target_p != last; target_p++)
    {
        Target& target = *target_p;
        Trajectory& traj = target.trajectory();
        int sf = target.get_start_time();
        int ef = target.get_end_time();

        // set trajectory
        target.update_pos_gp(pos_prior.scale(), pos_prior.signal_variance());
        std::vector<Vector> F_x(2);

        F_x[0] = kjb::gp::sample(target.pos_prior());
        F_x[1] = kjb::gp::sample(target.pos_prior());
        for(size_t f = sf; f <= ef; f++)
        {
            traj[f - 1] = Trajectory_element();
            traj[f - 1]->value.position[0] = F_x[0][f - sf];
            traj[f - 1]->value.position[2] = F_x[1][f - sf] - 5.0;;
        }

        // set body direction
        target.update_dir_gp(dir_prior.scale(), dir_prior.signal_variance());
        Vector F_d = kjb::gp::sample(target.dir_prior());
        for(size_t f = sf; f <= ef; f++)
        {
            traj[f - 1]->value.body_dir = F_d[f - sf];
        }

        // set face directions
        target.update_fdir_gp(fdir_prior.scale(), fdir_prior.signal_variance());
        Vector F_p = kjb::gp::sample(target.fdir_prior());
        Vector F_y = kjb::gp::sample(target.fdir_prior());
        for(size_t f = sf; f <= ef; f++)
        {
            traj[f - 1]->value.face_dir[0] = F_p[f - sf];
            traj[f - 1]->value.face_dir[1] = F_y[f - sf];
        }

        // set height, width and girth
        traj.height = kjb::sample(height_prior);
        traj.width = kjb::sample(width_prior);
        traj.girth = kjb::sample(girth_prior);
    }
}

/** @brief  Sample from camera prior. */
inline
Perspective_camera sample(const Camera_prior& cam_prior)
{
    Perspective_camera cam(0.1, 10000);
    cam.set_camera_centre_y(kjb::sample(cam_prior.height_prior()));
    cam.set_pitch(kjb::sample(cam_prior.pitch_prior()));
    cam.set_focal_length(kjb::sample(cam_prior.focal_length_prior()));

    return cam;
}

/**
 * @brief   Sample a single data box from a model box.
 *
 * This function samples a detection box from the box likelihood and a given
 * model box. That is, it samples from p(b | h).
 */
inline
Detection_box sample
(
    const Box_likelihood& box_lh,
    const Bbox& mbox,
    const std::string& type
)
{
    Vector params_x(4, 0.0);
    Vector params_top(4, 0.0);
    Vector params_bot(4, 0.0);
    box_lh.get_params(type, params_x, params_top, params_bot);

    double height = mbox.get_top() - mbox.get_bottom();
    if(height <= 0.0)
    {
        std::cerr << "WARNING: person behind camera!" << std::endl;
        height = -height;
    }

    double mean = params_x[0] + params_x[2] * height;
    double b = params_x[1] + params_x[3] * height;
    Laplace_distribution P_x(mean, b);

    mean = params_top[0] + params_top[2] * height;
    b = params_top[1] + params_top[3] * height;
    Laplace_distribution P_top(mean, b);

    mean = params_bot[0] + params_bot[2] * height;
    b = params_bot[1] + params_bot[3] * height;
    Laplace_distribution P_bot(mean, b);

    double dx = kjb::sample(P_x);
    double dtop = kjb::sample(P_top);
    double dbot = kjb::sample(P_bot);

    Vector c = mbox.get_center();
    c[0] += dx;
    double top = mbox.get_top() + dtop;
    double bot = mbox.get_bottom() + dbot;

    return Detection_box(Bbox(c, mbox.get_width(), top - bot), 0.0, type);
}

/**
 * @brief   Sample detection data from likelihood.
 *
 * This function samples box data from the box likelihood (and a given
 * set of tracks and 3D scene). That is, it samples from p(B | z, w).
 */
template<class TargetIterator>
void sample
(
    const Box_likelihood& box_lh,
    Box_data& box_data,
    TargetIterator first,
    TargetIterator last,
    const std::vector<size_t> fa_count
)
{
    const size_t T = fa_count.size();

    box_data.clear();
    box_data.resize(T);

    // true detections
    for(TargetIterator target_p = first; target_p != last; target_p++)
    {
        Target& target = *target_p;
        Body_2d_trajectory& btraj = target.body_trajectory();

        BOOST_FOREACH(Target::value_type& pr, target)
        {
            const Target::key_type& t = pr.first;
            Target::mapped_type& b_p = pr.second;

            const Body_2d& b2d = btraj[t - 1]->value;
            const Bbox& fbox = b2d.full_bbox;

            Detection_box dbox = sample(box_lh, fbox, "deva_box");
            b_p = &(*box_data[t - 1].insert(dbox).first);
        }
    }

    // false alarms
    double imw = box_data.image_width();
    double imh = box_data.image_height();
    for(size_t t = 0; t < T; t++)
    {
        for(size_t i = 0; i < fa_count[t]; i++)
        {
            Uniform_distribution U_h(-imh/2, imh/2);
            Uniform_distribution U_w(-imw/2, imw/2);

            double top = kjb::sample(U_h);
            double bot = kjb::sample(U_h);
            double xc = kjb::sample(U_w);
            double yc = (top - bot)/2;

            double h = fabs(top - bot);
            double w = h/3;
            Vector c(xc, yc);
            box_data[t].insert(Detection_box(Bbox(c, w, h), 0.8, "noise"));
        }
    }
}

/**
 * @brief   Sample facemark data from likelihood.
 */
template<class FmvIterator>
void sample
(
    const Facemark_likelihood& fm_lh,
    const Scene& scene,
    FmvIterator fm_out
)
{
    const Ascn& w = scene.association;
    for(size_t t = 0; t < w.get_data().size(); t++)
    {
        std::vector<Deva_facemark> facemarks_t;
        BOOST_FOREACH(const Target& tg, w)
        {
            const Face_2d_trajectory ftraj = tg.face_trajectory();
            if(ftraj[t])
            {
                const Face_2d& f2d = ftraj[t]->value;
                double fd = tg.trajectory()[t]->value.face_dir[0];
                fd += tg.trajectory()[t]->value.body_dir;
                while(fd > M_PI) fd -= M_PI;
                while(fd < -M_PI) fd += M_PI;
                double yaw = (180*fd)/M_PI;
                if(f2d.visibility.visible >= 0.5)
                {
                    if(kjb::sample(Uniform_distribution()) <= 0.5)
                    {
                        Vector le_mark;
                        Vector re_mark;
                        Vector n_mark;
                        Vector lm_mark;
                        Vector rm_mark;

                        const Normal_distribution& N_ex = fm_lh.eye_x_dist();
                        const Normal_distribution& N_ey = fm_lh.eye_y_dist();
                        const Normal_distribution& N_nx = fm_lh.nose_x_dist();
                        const Normal_distribution& N_ny = fm_lh.nose_y_dist();
                        const Normal_distribution& N_mx = fm_lh.mouth_x_dist();
                        const Normal_distribution& N_my = fm_lh.mouth_y_dist();

                        bool all_empty = true;
                        if(!f2d.left_eye.empty())
                        {
                            le_mark.set(f2d.left_eye[0] + kjb::sample(N_ex),
                                        f2d.left_eye[1] + kjb::sample(N_ey));
                            all_empty = false;
                        }

                        if(!f2d.right_eye.empty())
                        {
                            re_mark.set(f2d.right_eye[0] + kjb::sample(N_ex),
                                        f2d.right_eye[1] + kjb::sample(N_ey));
                            all_empty = false;
                        }

                        if(!f2d.nose.empty())
                        {
                            n_mark.set(f2d.nose[0] + kjb::sample(N_nx),
                                       f2d.nose[1] + kjb::sample(N_ny));
                            all_empty = false;
                        }

                        if(!f2d.left_mouth.empty())
                        {
                            lm_mark.set(f2d.left_mouth[0] + kjb::sample(N_mx),
                                        f2d.left_mouth[1] + kjb::sample(N_my));
                            all_empty = false;
                        }

                        if(!f2d.right_mouth.empty())
                        {
                            rm_mark.set(f2d.right_mouth[0] + kjb::sample(N_mx),
                                        f2d.right_mouth[1] + kjb::sample(N_my));
                            all_empty = false;
                        }

                        if(!all_empty)
                        {
                            facemarks_t.push_back(build_deva_facemark(le_mark,
                                                                      re_mark,
                                                                      n_mark,
                                                                      lm_mark,
                                                                      rm_mark,
                                                                      yaw));
                        }
                    }
                }
            }
        }

        *fm_out++ = facemarks_t;
    }
}

/**
 * @brief   Sample optical flow data from likelihood.
 *
 * This function samples OF data from the OF likelihood and a given scene
 * That is, it samples from p(I | z, w).
 */
template<class FlowIterator>
void sample
(
    const Optical_flow_likelihood& of_lh,
    const Scene& scene,
    FlowIterator xflow_out,
    FlowIterator yflow_out
)
{
    const Ascn& w = scene.association;
    const Box_data& data = static_cast<const Box_data&>(w.get_data());
    const size_t T = data.size();
    const double image_width = data.image_width();
    const double image_height = data.image_height();

    // BG flow
    const Laplace_distribution& L_bg_x = of_lh.bg_x_dist();
    const Laplace_distribution& L_bg_y = of_lh.bg_y_dist();

    Matrix bg_flow_x((size_t)image_height, (size_t)image_width);
    Matrix bg_flow_y((size_t)image_height, (size_t)image_width);
    for(size_t i = 0; i < (size_t)image_width; i++)
    {
        for(size_t j = 0; j < image_height; j++)
        {
            bg_flow_x(j, i) = kjb::sample(L_bg_x);
            bg_flow_y(j, i) = kjb::sample(L_bg_y);
        }
    }

    // FG flow
    const Laplace_distribution& L_x = of_lh.x_dist();
    const Laplace_distribution& L_y = of_lh.y_dist();
    for(size_t f = 0; f < T - 1; f++)
    {
        Matrix flow_x = bg_flow_x;
        Matrix flow_y = bg_flow_y;
        BOOST_FOREACH(const Target& target, w)
        {
            const Body_2d_trajectory& btraj = target.body_trajectory();
            const Face_2d_trajectory& ftraj = target.face_trajectory();

            if(!btraj[f] || !btraj[f + 1]) continue;

            // get body model vector
            const Bbox& curbox = btraj[f]->value.body_bbox;
            const Bbox& nxtbox = btraj[f + 1]->value.body_bbox;
            const Visibility& vis = btraj[f]->value.visibility;
            Vector model_vel = nxtbox.get_center() - curbox.get_center();
            model_vel[0] += kjb::sample(L_x);
            model_vel[1] += kjb::sample(L_y);

            int bl = std::max(curbox.get_left(), -image_width/2.0);
            int br = std::min(curbox.get_right(), image_width/2.0 - 1);
            int bb = std::max(curbox.get_bottom(), -image_height/2.0 + 1);
            int bt = std::min(curbox.get_top(), image_height/2.0);

            // for every pixel, set flow
            for(int x = bl; x <= br; x++)
            {
                for(int y = bb; y <= bt; y++)
                {
                    if(pixel_visible(x, y, vis))
                    {
                        Vector vs = Vector().set(x, y);
                        Vector vd = vs + model_vel;
                        unstandardize(vs, image_width, image_height);
                        unstandardize(vd, image_width, image_height);
                        flow_x(vs[1], vs[0]) = (vd - vs)[0];
                        flow_y(vs[1], vs[0]) = (vd - vs)[1];
                    }
                }
            }

            // get face model vector
            const Bbox& facebox = ftraj[f]->value.bbox;
            const Visibility& facevis = ftraj[f]->value.visibility;
            model_vel = ftraj[f]->value.model_dir;
            model_vel[0] += kjb::sample(L_x);
            model_vel[1] += kjb::sample(L_y);

            bl = std::max(facebox.get_left(), -image_width/2.0);
            br = std::min(facebox.get_right(), image_width/2.0 - 1);
            bb = std::max(facebox.get_bottom(), -image_height/2.0 + 1);
            bt = std::min(facebox.get_top(), image_height/2.0);

            // for every pixel, set flow
            for(int x = bl; x <= br; x++)
            {
                for(int y = bb; y <= bt; y++)
                {
                    if(pixel_visible(x, y, facevis))
                    {
                        Vector vs = Vector().set(x, y);
                        Vector vd = vs + model_vel;
                        unstandardize(vs, image_width, image_height);
                        unstandardize(vd, image_width, image_height);
                        flow_x(vs[1], vs[0]) = (vd - vs)[0];
                        flow_y(vs[1], vs[0]) = (vd - vs)[1];
                    }
                }
            }
        }

        // generate integral flow
        *xflow_out++ = Integral_flow(flow_x, 4);
        *yflow_out++ = Integral_flow(flow_y, 4);
    }
}

/**
 * @brief   Sample from full generative model.
 *
 * This function samples from the joint p(B, I, x, w) using ancestral
 * sampling.
 */
template<class FmvIterator, class FlowOutIterator>
void sample
(
    const mcmcda::Prior<Target>& w_prior,
    const Camera_prior& cam_prior,
    const Scene_posterior& posterior,
    size_t T,
    Scene& scene,
    Box_data& box_data,
    FmvIterator fm_out,
    FlowOutIterator xflow_out,
    FlowOutIterator yflow_out,
    size_t max_tracks = 0
)
{
    using namespace std;

    Perspective_camera& cam = scene.camera;
    Ascn& w = scene.association;
    scene.kappa = w_prior.kappa();
    scene.theta = w_prior.theta();
    scene.lambda = w_prior.lambda_N();

    // distributions
    const Position_prior& pos_prior = posterior.position_prior();
    const Direction_prior& dir_prior = posterior.direction_prior();
    const Face_direction_prior& fdir_prior = posterior.face_direction_prior();
    const Normal_distribution& height_prior = posterior.height_prior();
    const Normal_distribution& width_prior = posterior.width_prior();
    const Normal_distribution& girth_prior = posterior.girth_prior();
    const Box_likelihood& box_lh = posterior.box_likelihood();
    const Facemark_likelihood& fm_lh = posterior.fm_likelihood();
    const Optical_flow_likelihood& of_lh = posterior.of_likelihood();

    // sample tracks
    double avg_ht = get_entity_type_average_height(PERSON_ENTITY);
    double avg_wt = get_entity_type_average_width(PERSON_ENTITY);;
    double avg_gt = get_entity_type_average_girth(PERSON_ENTITY);;
    Target canon_target(avg_ht, avg_wt, avg_gt, T);
    canon_target.trajectory().id.type = PERSON_ENTITY;

    pair<vector<Target>, vector<size_t> >
        wfa = mcmcda::sample(w_prior, T, canon_target);
    vector<Target>& targets = wfa.first;
    const vector<size_t>& false_alarms = wfa.second;

    // sample camera
    cam = sample(cam_prior);

    // sample 3D scene
    sample(
        pos_prior,
        dir_prior,
        fdir_prior,
        height_prior,
        width_prior,
        girth_prior,
        targets.begin(),
        targets.end());

    // fix targets (they cannot go off-image)
    typedef std::vector<Target>::iterator Tv_it;
    double imw = box_data.image_width();
    double imh = box_data.image_height();
    std::for_each(
        targets.begin(),
        targets.end(),
        boost::bind(&Target::set_changed_all, _1));
    std::for_each(
        targets.begin(),
        targets.end(),
        boost::bind(&Target::update_boxes, _1, boost::cref(cam)));
    std::for_each(
        targets.begin(),
        targets.end(),
        boost::bind(fix_target, _1, imw, imh));

    Tv_it ltg_p = std::remove_if(targets.begin(), targets.end(), empty_target);
    if(max_tracks != 0 && std::distance(targets.begin(), ltg_p) > max_tracks)
    {
        ltg_p = targets.begin();
        std::advance(ltg_p, max_tracks);
    }

    // update state
    std::for_each(
        targets.begin(),
        ltg_p,
        boost::bind(&Target::set_changed_all, _1));
    std::for_each(
        targets.begin(),
        ltg_p,
        boost::bind(&Target::update_boxes, _1, boost::cref(cam)));
    std::for_each(
        targets.begin(),
        ltg_p,
        boost::bind(&Target::update_faces, _1, boost::cref(cam)));

    std::for_each(
        targets.begin(),
        ltg_p,
        boost::bind(&Target::update_walking_angles, _1));

    // sample box data
    sample(box_lh, box_data, targets.begin(), ltg_p, false_alarms);

    // update association
    w.set_data(box_data);
    w.insert(targets.begin(), ltg_p);
    update_visibilities(scene);

    // sample FM data
    sample(fm_lh, scene, fm_out);

    // sample OF data
    sample(of_lh, scene, xflow_out, yflow_out);
}

}} // namespace kjb::pt

#endif /*PT_SCENE_GENERATIVE_MOODEL_H */

