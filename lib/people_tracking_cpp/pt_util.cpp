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
   |  Author:  Kyle Simek, Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id: pt_util.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_util.h"
#include "people_tracking_cpp/pt_data.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_box_trajectory.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_complete_state.h"
#include "people_tracking_cpp/pt_detection_box.h"
#include "people_tracking_cpp/pt_association.h"
#include "people_tracking_cpp/pt_visibility.h"
#include "people_tracking_cpp/pt_face_2d.h"
#include "people_tracking_cpp/pt_face_2d_trajectory.h"
#include "people_tracking_cpp/pt_scene.h"
#include "prob_cpp/prob_histogram.h"
#include "camera_cpp/perspective_camera.h"
#include "camera_cpp/camera_backproject.h"
#include "wrap_opencv_cpp/cv_histogram.h"
#include "detector_cpp/d_deva_detection.h"
#include "l_cpp/l_exception.h"

#include <set>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

void kjb::pt::standardize
(
    Deva_facemark& face,
    double cam_width,
    double cam_height
)
{
    // left eye
    std::vector<Vector>& left_eye = face.left_eye();
    BOOST_FOREACH(Vector& mark, left_eye)
    {
        standardize(mark, cam_width, cam_height);
    }

    // left eye mark
    Vector& lem = face.left_eye_mark();
    if(!lem.empty())
    {
        standardize(lem, cam_width, cam_height);
    }

    // right eye
    std::vector<Vector>& right_eye = face.right_eye();
    BOOST_FOREACH(Vector& mark, right_eye)
    {
        standardize(mark, cam_width, cam_height);
    }

    // right eye mark
    Vector& rem = face.right_eye_mark();
    if(!rem.empty())
    {
        standardize(rem, cam_width, cam_height);
    }

    // nose
    std::vector<Vector>& nose = face.nose();
    BOOST_FOREACH(Vector& mark, nose)
    {
        standardize(mark, cam_width, cam_height);
    }

    // nose mark
    Vector& nm = face.nose_mark();
    if(!nm.empty())
    {
        standardize(nm, cam_width, cam_height);
    }

    // mouth
    std::vector<Vector>& mouth = face.mouth();
    BOOST_FOREACH(Vector& mark, mouth)
    {
        standardize(mark, cam_width, cam_height);
    }

    // left mouth mark
    Vector& lmm = face.left_mouth_mark();
    if(!lmm.empty())
    {
        standardize(lmm, cam_width, cam_height);
    }

    // right mouth mark
    Vector& rmm = face.right_mouth_mark();
    if(!rmm.empty())
    {
        standardize(rmm, cam_width, cam_height);
    }

    // chin
    std::vector<Vector>& chin = face.chin();
    BOOST_FOREACH(Vector& mark, chin)
    {
        standardize(mark, cam_width, cam_height);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::unstandardize
(
    Deva_facemark& face,
    double cam_width,
    double cam_height
)
{
    // left eye
    std::vector<Vector>& left_eye = face.left_eye();
    BOOST_FOREACH(Vector& mark, left_eye)
    {
        unstandardize(mark, cam_width, cam_height);
    }

    // left eye mark
    Vector& lem = face.left_eye_mark();
    if(!lem.empty())
    {
        unstandardize(lem, cam_width, cam_height);
    }

    // right eye
    std::vector<Vector>& right_eye = face.right_eye();
    BOOST_FOREACH(Vector& mark, right_eye)
    {
        unstandardize(mark, cam_width, cam_height);
    }

    // right eye mark
    Vector& rem = face.right_eye_mark();
    if(!rem.empty())
    {
        unstandardize(rem, cam_width, cam_height);
    }

    // nose
    std::vector<Vector>& nose = face.nose();
    BOOST_FOREACH(Vector& mark, nose)
    {
        unstandardize(mark, cam_width, cam_height);
    }

    // nose mark
    Vector& nm = face.nose_mark();
    if(!nm.empty())
    {
        unstandardize(nm, cam_width, cam_height);
    }

    // mouth
    std::vector<Vector>& mouth = face.mouth();
    BOOST_FOREACH(Vector& mark, mouth)
    {
        unstandardize(mark, cam_width, cam_height);
    }

    // left mouth mark
    Vector& lmm = face.left_mouth_mark();
    if(!lmm.empty())
    {
        unstandardize(lmm, cam_width, cam_height);
    }

    // right mouth mark
    Vector& rmm = face.right_mouth_mark();
    if(!rmm.empty())
    {
        unstandardize(rmm, cam_width, cam_height);
    }

    // chin
    std::vector<Vector>& chin = face.chin();
    BOOST_FOREACH(Vector& mark, chin)
    {
        unstandardize(mark, cam_width, cam_height);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Ascn kjb::pt::make_gt_association
(
    const Box_data& data,
    const Box_trajectory_map& gt_btrajs,
    double width,
    double height,
    boost::optional<const Perspective_camera&> cam_p,
    boost::optional<const Facemark_data&> fm_data_p
)
{
    Ascn assoc(data);

    boost::optional<Ground_back_projector> back_project_p = boost::none;
    if(cam_p)
    {
        back_project_p = Ground_back_projector(*cam_p, 0.0);
    }

    double tht = get_entity_type_average_height(PERSON_ENTITY);
    double twt = get_entity_type_average_width(PERSON_ENTITY);
    double tgt = get_entity_type_average_girth(PERSON_ENTITY);
    size_t T = data.size();

    BOOST_FOREACH(const Box_trajectory_map::value_type& pr, gt_btrajs)
    {
        const Box_trajectory& gt_btraj = pr.second;
        int stm = gt_btraj.start_time();
        int etm = gt_btraj.end_time();
        ASSERT((stm != -1 && etm != -1));

        Target target(tht, twt, tgt, T);
        double gt_height = 0.0;
        for(int t = stm - 1; t <= etm-1; t++)
        {
            ASSERT(gt_btraj[t]);
            Bbox gt_box = gt_btraj[t]->value;
            standardize(gt_box, width, height);

            const std::set<Detection_box>& data_t = data[t];
            BOOST_FOREACH(const Detection_box& dbox, data_t)
            {
                if(similar_boxes(gt_box, dbox.bbox, 0.9, 1.1))
                {
                    target.insert(std::make_pair(t + 1, &dbox));
                }
            }

            if(cam_p)
            {
                Trajectory& traj = target.trajectory();
                Vector pos = (*back_project_p)(gt_box.get_bottom_center()[0],
                                               gt_box.get_bottom_center()[1]);
                if(!pos.empty())
                {
                    traj[t] = Complete_state(Vector3(pos));
                }
                else
                {
                    std::cerr << "WARNING: ground truth box bottom does not "
                              << "project to the ground."
                              << std::endl;
                }

                gt_height += get_3d_height(gt_box.get_bottom_center(),
                                           gt_box.get_top_center(),
                                           *cam_p);
            }
        }

        if(!target.empty())
        {
            target.set_changed_all();
            if(cam_p)
            {
                // GT hehight
                size_t rstm = target.get_start_time();
                size_t retm = target.get_end_time();
                gt_height /= (retm - rstm + 1);
                target.trajectory().height = gt_height;
                target.estimate_trajectory(*cam_p);

                // update state
                initialize_directions(target.trajectory(), 
                                      target.changed_start(), 
                                      target.changed_end(), 
                                      true);
                target.update_boxes(*cam_p);
                target.update_faces(*cam_p);
            }
            assoc.insert(target);
        }
    }

    // use facemarks to refine the bottom of the trajectory and the face
    // directions
    if(fm_data_p)
    {
        update_facemarks(assoc, *fm_data_p);
        BOOST_FOREACH(const Target& tg, assoc)
        {
            if(cam_p)
            {
                /*if(refine)
                {
                    tg.refine_trajectory(scene.camera);
                }*/

                tg.estimate_directions();
                tg.update_boxes(*cam_p);
                tg.update_faces(*cam_p);
            }
        }

        clear_facemarks(assoc);
    }

    return assoc;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Box_data kjb::pt::make_gt_data
(
    const Box_trajectory_map& gt_btrajs,
    double width,
    double height
)
{
    Box_data data(width, height);
    data.resize(gt_btrajs.duration());
    BOOST_FOREACH(const Box_trajectory_map::value_type& pr, gt_btrajs)
    {
        const Box_trajectory& gt_btraj = pr.second;
        size_t sf = gt_btraj.start_time();
        size_t ef = gt_btraj.end_time();
        for(size_t t = sf - 1; t <= ef - 1; t++)
        {
            ASSERT(gt_btraj[t]);
            Bbox box = gt_btraj[t]->value;
            standardize(box, width, height);
            Detection_box dbox(box, 1e-10, "groundtruth");
            data[t].insert(dbox);
        }
    }
    return data;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::map<int, std::vector<Propagated_2d_info> > kjb::pt::propagate_boxes_by_flow
(
    const Target& target,
    const std::vector<Integral_flow>& x_flows,
    const std::vector<Integral_flow>& y_flows,
    const std::vector<Integral_flow>& back_x_flows,
    const std::vector<Integral_flow>& back_y_flows,
    size_t duration
)
{
    size_t img_width = x_flows[0].img_width();
    size_t img_height = x_flows[1].img_height();
    typedef Target::const_iterator Tci;
    std::map<int, std::vector<Propagated_2d_info> > prop_info;
    const Trajectory& traj = target.trajectory();

    size_t sf = target.get_start_time();
    size_t ef = target.get_end_time();
    ASSERT(sf >= 1 && ef >= 1);
    for(size_t u = sf - 1; u < ef - 1; u++)
    {
        int t = u + 1;
        std::pair<Tci, Tci> rg = target.equal_range(t);
        for(Tci pr_p = rg.first; pr_p != rg.second; pr_p++)
        {
            Bbox box = pr_p->second->bbox;
            double w = box.get_width();
            double h = box.get_height();
            double f_w = w * 0.8;
            double f_h = h * 0.4;
            if(prop_info.find(t) == prop_info.end())
            {
                std::vector<Propagated_2d_info> info_v;
                prop_info[t] = info_v;
            }

            Propagated_2d_info info;
            info.x = box.get_center()[0];
            info.y_bottom = box.get_bottom_center()[1];
            info.origin_frame = t;

            prop_info[t].push_back(info);
            // set up the flow region
            Vector flow_center(2, 0.0);
            flow_center[0] = box.get_center()[0];
            flow_center[1] = box.get_bottom_center()[1] + h * 0.6;
            Bbox flow_region(flow_center, f_w, f_h);
            unstandardize(flow_region, img_width, img_height);
            double area = f_w * f_h;
            Vector cur_bottom_center = box.get_bottom_center();
            Vector cur_flow_center = flow_region.get_center();

            // Propagating the box foward using foward flow
            for(size_t d = 1; d <= duration; d++)
            {
                size_t frame = t + d;

                // if the trajectory exsits, don't need to propagate
                if(traj[frame - 1]) continue;
                if (frame > ef || frame < 2) break;
                ASSERT(frame >= 2);

                // Calculate the forward flow
                Vector cur_flow(0.0, 0.0);
                const Integral_flow& flx = x_flows[frame - 2];
                const Integral_flow& fly = y_flows[frame - 2];
                cur_flow.set(
                    flx.flow_sum(flow_region),
                    fly.flow_sum(flow_region));
                cur_flow /= area;
                cur_bottom_center += cur_flow;
                cur_flow_center += cur_flow;
                if(prop_info.find(frame) == prop_info.end())
                {
                    std::vector<Propagated_2d_info> info_v;
                    prop_info[frame] = info_v;
                }
                Propagated_2d_info info;
                info.x = cur_bottom_center[0];
                info.y_bottom = cur_bottom_center[1];
                info.origin_frame = t;
                prop_info[frame].push_back(info);
                flow_region.set_center(cur_flow_center);
            }

            // reset the box and flow location
            cur_flow_center = flow_center;
            unstandardize(cur_flow_center, img_width, img_height);
            cur_bottom_center = box.get_bottom_center();
            flow_region.set_center(cur_flow_center);

            // Propagating the box backward using backward flow
            for(size_t d = 1; d <= duration; d++)
            {
                size_t frame = t - d;
                // if the trajectory exsits, don't need to propagate
                if(traj[frame - 1]) continue;
                if (frame > ef || frame < 1) break;
                // Calculate the backward flow
                Vector cur_flow(0.0, 0.0);
                const Integral_flow& flx = back_x_flows[frame-1];
                const Integral_flow& fly = back_y_flows[frame-1];
                cur_flow.set(
                    flx.flow_sum(flow_region),
                    fly.flow_sum(flow_region));
                cur_flow /= area;
                cur_bottom_center += cur_flow;
                cur_flow_center += cur_flow;
                if(prop_info.find(frame) == prop_info.end())
                {
                    std::vector<Propagated_2d_info> info_v;
                    prop_info[frame] = info_v;
                }
                Propagated_2d_info info;
                info.x = cur_bottom_center[0];
                info.y_bottom = cur_bottom_center[1];
                info.origin_frame = t;
                prop_info[frame].push_back(info);
                flow_region.set_center(cur_flow_center);
            }
        }
    }

    return prop_info;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> Feature_score::operator()
(
    const Target* target_p,
    int t1,
    const Detection_box* candidate_p,
    int t2,
    size_t wsz
) const
{
    std::vector<double> probs;
    double p_deva = 1.0 - candidate_p->prob_noise;

    // t1 and t2 must be in the range of [1...N]
    if(t2 > m_hists.size() || t2 <= 0)
    {
        return probs;
    }
    ASSERT(target_p != NULL);

    IFT(t1 <= m_hists.size() && t2 <= m_hists.size(), Illegal_argument,
          "Index of detection box is bigger than the number of frames\n");
    Hist_map::const_iterator cand_hist_p = m_hists[t2 - 1].find(candidate_p);
    ASSERT(cand_hist_p != m_hists[t2 - 1].end());
    double p_color = 0.0;
    double p_flow = 0.0;
    size_t N1 = 0;
    size_t N2 = 0;

    Vector cand_flow;
    if(t2 < m_flows_x.size())
    {
        Bbox box(candidate_p->bbox);
        // convert to image coordinate
        unstandardize(box, m_flows_x[t2 - 1].img_width(),
                           m_flows_x[t2 - 1].img_height());
        double area = box.get_width() * box.get_height();
        double dx = m_flows_x[t2 - 1].flow_sum(box) / area;
        double dy = m_flows_y[t2 - 1].flow_sum(box) / area;
        cand_flow.set(dx, dy);
    }

    //const double flow_sigma = cand_flow.empty() ? 2.0 : cand_flow.magnitude();
    const double flow_sigma = std::max(2.0, cand_flow.magnitude());
    //std::cout << "flow sigma: " << flow_sigma << std::endl;
    double dist = fabs(t1 - t2);

    Gaussian_distribution flow_dist(0.0, flow_sigma);

    if(t1 < t2) //grow forward
    {
        typedef Target::const_reverse_iterator Trci;
        Trci pair_p = Trci(target_p->upper_bound(t1));
        IFT(pair_p->first == t1, Illegal_argument,
            "The frame passed to track_future() must be valid.");
        size_t wnd = t1 - pair_p->first;
        //std::cout << "wsz: " << wsz << std::endl;
        while(wnd < wsz)
        {
            const Detection_box* cur_box = pair_p->second;
            int cur_t =  t1 - wnd;
            ASSERT(cur_t - 1 < m_hists.size());
            // color
            Hist_map::const_iterator hist_p = m_hists[cur_t - 1].find(cur_box);
            ASSERT(hist_p != m_hists[cur_t - 1].end());
            double d = chi_square(hist_p->second, cand_hist_p->second);
            p_color += chi_squared_dist_to_prob(d);

            // flow
            if(!cand_flow.empty() && cur_t < m_flows_x.size())
            {
                Bbox box(cur_box->bbox);
                double area = box.get_width() * box.get_height();
                // convert to image coordinate
                unstandardize(box, m_flows_x[cur_t - 1].img_width(),
                                   m_flows_x[cur_t - 1].img_height());
                double dx = m_flows_x[cur_t - 1].flow_sum(box) / area;
                double dy = m_flows_y[cur_t - 1].flow_sum(box) / area;
                double flow_diff = vector_distance(cand_flow, Vector(dx, dy));
                double density = pdf(flow_dist, flow_diff);
                double noise_score = pdf(flow_dist, flow_sigma
                                                        * std::pow(dist, 0.5));
                p_flow += density/(density + noise_score);
                N2++;
            }

            N1++;
            pair_p++;
            if(pair_p == target_p->rend()) break;
            wnd = t1 - pair_p->first;
        }
    }
    else // grow backward
    {
        typedef Target::const_iterator Tci;
        Tci pair_p = target_p->lower_bound(t1);
        IFT(pair_p->first == t1, Illegal_argument,
            "The frame passed to track_future() must be valid.");
        size_t wnd = pair_p->first - t1;
        //std::cout << "wsz: " << wsz << std::endl;
        while(wnd < wsz)
        {
            const Detection_box* cur_box = pair_p->second;
            int cur_t = t1 + wnd;
            // color
            ASSERT(cur_t - 1 < m_hists.size());
            Hist_map::const_iterator hist_p = m_hists[cur_t - 1].find(cur_box);
            ASSERT(hist_p != m_hists[cur_t - 1].end());
            double d = chi_square(hist_p->second, cand_hist_p->second);
            p_color += chi_squared_dist_to_prob(d);

            // flow
            if(!cand_flow.empty() && cur_t < m_flows_x.size())
            {
                Bbox box(cur_box->bbox);
                double area = box.get_width() * box.get_height();
                // convert to image coordinate
                unstandardize(box, m_flows_x[cur_t - 1].img_width(),
                                   m_flows_x[cur_t - 1].img_height());
                double dx = m_flows_x[cur_t - 1].flow_sum(box) / area;
                double dy = m_flows_y[cur_t - 1].flow_sum(box) / area;
                double flow_diff = vector_distance(cand_flow, Vector(dx, dy));
                double density = pdf(flow_dist, flow_diff);
                double noise_score = pdf(flow_dist, flow_sigma
                                                        * std::pow(dist, 0.5));
                p_flow += density/(density + noise_score);
                N2++;
            }

            N1++;
            pair_p++;
            if(pair_p == target_p->end()) break;
            wnd = pair_p->first - t1;
        }
    }

    p_color /= N1;
    // weight the color probability by detection probability
    p_color *= p_deva;
    probs.push_back(p_color);

    if(!cand_flow.empty())
    {
        p_flow /= N2;
        probs.push_back(p_flow);
    }

    return probs;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::pt::average_box_flow
(
    const Integral_flow& flx,
    const Integral_flow& fly,
    const Bbox& bbox,
    const Visibility& vis
)
{
    const double img_width = flx.img_width();
    const double img_height = flx.img_height();

    // unstandardize box
    Bbox box = bbox;
    unstandardize(box, img_width, img_height);

    // compute total flow and area
    double vis_area = 0.0;
    Vector flow_velocity(0.0, 0.0);
    if(fabs(vis.visible - 1.0) < DBL_EPSILON)
    {
        // if whole box is visible
        flow_velocity.set(flx.flow_sum(box), fly.flow_sum(box));
        vis_area = box.get_width()*box.get_height();
    }
    else
    {
        // if some of it is occluded, add flows in visible cells
        const size_t num_vis_cells = vis.visible_cells.size();
        const double cell_area = vis.cell_width*vis.cell_height;
        vis_area = cell_area * num_vis_cells;
        for(size_t cc = 0; cc < num_vis_cells; cc++)
        {
            Bbox cell(vis.visible_cells[cc], vis.cell_width, vis.cell_height);
            unstandardize(cell, img_width, img_height);

            Vector cur_flow(flx.flow_sum(cell), fly.flow_sum(cell));
            flow_velocity += cur_flow;
        }
    }

    // average flow
    flow_velocity /= vis_area;

    return flow_velocity;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::prune_by_height
(
    std::vector<Deva_detection>& deva_boxes,
    double image_width,
    double image_height,
    const Perspective_camera& camera,
    double average_height
)
{
    Ground_back_projector back_project(camera, 0.0);
    for(size_t j = 0; j < deva_boxes.size(); j++)
    {
        // Use the pruned deva boxes
        Bbox sbox(deva_boxes[j].full_body());
        pt::standardize(sbox, image_width, image_height);
        double box_height = get_3d_height(sbox.get_bottom_center(),
                                              sbox.get_top_center(),
                                              camera);
        double max_height = average_height*2.0;
        double min_height = average_height/2.0;
        if(box_height <= max_height && box_height >= min_height)
        {
            kjb::Vector pt3d = back_project(sbox.get_bottom_center()[0],
                                            sbox.get_bottom_center()[1]);
            if(pt3d.empty())
            {
                std::vector<Deva_detection>::iterator box_j_p = deva_boxes.begin();
                box_j_p += (j--);
                deva_boxes.erase(box_j_p);
            }
        }
        else
        {
            std::vector<Deva_detection>::iterator box_j_p = deva_boxes.begin();
            box_j_p += (j--);
            deva_boxes.erase(box_j_p);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::update_facemarks(const Ascn& ascn, const Facemark_data& fms)
{
    size_t num_frames = fms.size();
    //size_t num_noise_faces = 0;
    for(size_t i = 0; i < num_frames; i++)
    {
        //std::cout << "frame: " << i + 1 << std::endl;
        const std::vector<Deva_facemark>& cur_fms = fms[i];
        //std::vector<const Deva_facemark*> cur_noise_fms;
        BOOST_FOREACH(const Deva_facemark& fm, cur_fms)
        {
            // if the facemark has a score and the score is less than -0.8
            // ignore it 
            if(fm.score() != -DBL_MAX && fm.score() < -0.8) continue;
            // for each current facemark check against each model target
            double min_dist = DBL_MAX;
            Face_2d* face_pt = NULL;
            const Body_2d* body_pt = NULL;
            const Target* tg_temp_p = 0;
            BOOST_FOREACH(const Target& tg, ascn)
            {
                if(i + 1 < tg.changed_start() || i + 1 > tg.changed_end())
                {
                    continue;
                }

                Face_2d_trajectory& ftraj = tg.face_trajectory();
                const Body_2d_trajectory& btraj = tg.body_trajectory();
                if(!tg.changed())
                {
                    if(ftraj[i]->value.facemark == &fm)
                    {
                        face_pt = 0;
                        body_pt = 0;
                        break;
                    }
                }

                // If the face model exists and does not have any assigned
                // detection facemark, check to see if we can assign this
                // detection to the current model
                if(ftraj[i] && ftraj[i]->value.facemark == NULL)
                {
                    //std::cout << "\t\t\t\tUpdate facemark for target: [" 
                    //          << (i+1) << "]" << std::endl;
                    // check to see whether the current facemark
                    // can be assigned to the current model face
                    double dist = face_distance(ftraj[i]->value, fm);
                    if(dist < min_dist)
                    {
                        min_dist = dist;
                        face_pt = &(ftraj.at(i)->value);
                        body_pt = &(btraj.at(i)->value);
                        tg_temp_p = &tg;
                    }
                }
            }

            if(face_pt != NULL)
            {
                ASSERT(body_pt != NULL);
                double thresh = face_pt->bbox.get_width() * 2.5;
                if(min_dist < thresh)
                {
                    // check to see if the facemarks are inside the body box
                    Bbox fbox = body_pt->full_bbox;
                    const Bbox& bbox = body_pt->body_bbox;
                    //double fh = fbox.get_height();
                    //fbox.set_height(fh - bbox.get_height());
                    //fbox.set_centre_y(bbox.get_centre_y() + fh/ 2.0);
                    size_t inside_num = 0;
                    size_t num_points = 0;
                    double min_y = DBL_MAX;
                    double max_y = -DBL_MAX;
                    if(!fm.left_eye_mark().empty())
                    {
                        if(fbox.contains(fm.left_eye_mark()))
                        {
                            inside_num++;
                        }
                        num_points++;
                        if(fm.left_eye_mark()[1] < min_y)
                        {
                            min_y = fm.left_eye_mark()[1];
                        }
                        if(fm.left_eye_mark()[1] > max_y)
                        {
                            max_y = fm.left_eye_mark()[1];
                        }
                    }
                    if(!fm.right_eye_mark().empty())
                    {
                        if(fbox.contains(fm.right_eye_mark()))
                        {
                            inside_num++;
                        }
                        num_points++;
                        if(fm.right_eye_mark()[1] < min_y)
                        {
                            min_y = fm.right_eye_mark()[1];
                        }
                        if(fm.right_eye_mark()[1] > max_y)
                        {
                            max_y = fm.right_eye_mark()[1];
                        }
                    }
                    if(!fm.nose_mark().empty())
                    {
                        if(fbox.contains(fm.nose_mark()))
                        {
                            inside_num++;
                        }
                        num_points++;
                        if(fm.nose_mark()[1] < min_y)
                        {
                            min_y = fm.nose_mark()[1];
                        }
                        if(fm.nose_mark()[1] > max_y)
                        {
                            max_y = fm.nose_mark()[1];
                        }
                    }
                    if(!fm.left_mouth_mark().empty())
                    {
                        if(fbox.contains(fm.left_mouth_mark()))
                        {
                            inside_num++;
                        }
                        num_points++;
                        if(fm.left_mouth_mark()[1] < min_y)
                        {
                            min_y = fm.left_mouth_mark()[1];
                        }
                        if(fm.left_mouth_mark()[1] > max_y)
                        {
                            max_y = fm.left_mouth_mark()[1];
                        }
                    }
                    if(!fm.right_mouth_mark().empty())
                    {
                        if(fbox.contains(fm.right_mouth_mark()))
                        {
                            inside_num++;
                        }
                        num_points++;
                        if(fm.right_mouth_mark()[1] < min_y)
                        {
                            min_y = fm.right_mouth_mark()[1];
                        }
                        if(fm.right_mouth_mark()[1] > max_y)
                        {
                            max_y = fm.right_mouth_mark()[1];
                        }
                    }
                    double inside_prob = inside_num * 1.0 / num_points; 
                    double est_fh = (max_y - min_y) * 3.0;
                    double max_fh = bbox.get_height()/5.0;
                    double min_fh = bbox.get_height()/10.0;
                    //std::cout << "inside_num: " << inside_prob << std::endl;
                    //std::cout << "est_fh: " << est_fh << " body height: " << bbox.get_height() << " : " << bbox.get_height() / est_fh << std::endl;

                    if(inside_prob >= 0.3 && est_fh >= min_fh && est_fh <= max_fh)
                    {
                        // we need to check if the bounding box of 
                        face_pt->facemark = &fm;
                    }
                }
                /*else
                {
                    // the face mark does not belong to any model,
                    // thus it is a false detection
                    //cur_noise_fms.push_back(&fm);
                    num_noise_faces++;
                }*/
            }
            /*else
            {
                // the face mark does not belong to any model,
                // thus it is a false detection
                //cur_noise_fms.push_back(&fm);
                num_noise_faces++;
            }*/
        }
    }

    //return num_noise_faces;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*void kjb::pt::update_facemarks(const Scene& scene, const Facemark_data& fms)
{
    scene.ns_faces = update_facemarks(scene.association, fms);
}*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::clear_facemarks(const Ascn& ascn)
{
    BOOST_FOREACH(const Target& tg, ascn)
    {
        if(!tg.changed()) continue;

        Face_2d_trajectory& ftraj = tg.face_trajectory();
        //std::cout << "\t\t\t\tclear facemark for target: [" << tg.changed_start()
        //    << " - " << tg.changed_end() << "] " << std::endl;
        for(size_t t = tg.changed_start(); t <= tg.changed_end(); ++t)
        {
            ftraj[t - 1]->value.facemark = 0;
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::find_looking_subjects(const Scene& scene, double thresh)
{
    BOOST_FOREACH(const Target& target, scene.association)
    {
        size_t sf = target.get_start_time();
        size_t ef = target.get_end_time();
        Trajectory& traj = target.trajectory();
        double ht = traj.height;
        double wt = traj.width;
        double gt = traj.girth;

        for(size_t t = sf; t <= ef; ++t)
        {
            Complete_state& cs = traj[t - 1]->value;
            double min_dst = DBL_MAX;
            cs.attn = 0;
            BOOST_FOREACH(const Target& target2, scene.association)
            {
                if(&target2 == &target) continue;

                size_t sf2 = target2.get_start_time();
                size_t ef2 = target2.get_end_time();
                if(t < sf2 || t > ef2) continue;

                const Vector3& pos2 = target2.trajectory()[t-1]->value.position;
                Vector hc(pos2[0], target2.trajectory().height, pos2[2]);
                if(looking(cs, ht, wt, gt, hc, thresh, false))
                {
                    double dst = vector_distance(cs.position, pos2);
                    if(dst < min_dst)
                    {
                        min_dst = dst;
                        cs.attn = &target2;
                    }
                }
            }

            BOOST_FOREACH(const Target& target2, scene.objects)
            {
                const Vector3& pos2 = target2.trajectory()[0]->value.position;
                Vector hc(pos2[0], pos2[1], pos2[2]);
                if(looking(cs, ht, wt, gt, hc, thresh, true))
                {
                    double dst = vector_distance(cs.position, pos2);
                    if(dst < min_dst)
                    {
                        min_dst = dst;
                        cs.attn = &target2;
                    }
                }
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::pt::locations_3d(const Scene& scene, double thresh)
{
    // check the X range in images 
    std::vector<Vector> pts;
    const double img_width = 1920.0;
    const double img_height = 1080;
    const double x_right = -img_width/ 2.0;
    const double x_left = -x_right;
    const double y_top = img_height / 2.0;
    const double y_bottom = -y_top; 

    BOOST_FOREACH(const Target& tg, scene.association)
    {
        size_t sf = tg.get_start_time();
        size_t ef = tg.get_end_time();
        const Trajectory& traj = tg.trajectory();
        for(size_t t = sf; t <= ef; ++t)
        {
            if(traj[t - 1]->value.attn != 0) continue;

            // head point
            Vector hc(
                traj[t - 1]->value.position[0],
                traj.height,
                traj[t - 1]->value.position[2]);

            // find intersection that is farthest away
            double max_dist = 0.0;
            const double max_thresh = 5.0; 
            Vector max_inter;
            BOOST_FOREACH(const Target& tg2, scene.association)
            {
                size_t sf2 = tg2.get_start_time();
                size_t ef2 = tg2.get_end_time();
                const Trajectory& traj2 = tg2.trajectory();
                for(size_t t2 = sf2; t2 <= ef2; ++t2)
                {
                    if(&tg == &tg2 && t == t2) continue;
                    if(traj2[t2 - 1]->value.attn != 0) continue;

                    Vector gi = gaze_intersection(
                                    traj[t - 1]->value,
                                    traj.height,
                                    traj.width,
                                    traj.height,
                                    traj2[t2 - 1]->value,
                                    traj2.height,
                                    traj2.width,
                                    traj2.height,
                                    thresh);

                    if(gi.empty()) continue;
                    // check to see if the point is above the ground y >= 0.0
                    // and infront of the camera z < 0.0
                    if(gi[1] < 0.0 || gi[2] >= 0.0)
                        continue;

                    // check to see if the 3D point is project inside the image
                    // plane
                    // === HACK: should fix in fugure ===
                    try
                    {
                        Vector gi2d = project_point(scene.camera, gi);
                        unstandardize(gi2d, img_width, img_height);
                        if(gi2d[0] < 0.0 || gi2d[0] > img_width ||
                                gi2d[1] < 0.0 || gi2d[1] > img_height)
                        {
                            continue;
                        }
                    }
                    catch(const Illegal_argument& iaex)
                    {
                        std::cerr << "WARN: 3D location on camera plane; "
                                  << "cannot be projected" << std::endl;
                        continue;
                    }

                    double dist = vector_distance(gi, hc);
                    if(dist > max_dist)
                    {
                        max_inter = gi;
                        max_dist = dist;
                    }
                }
            }

            if(max_dist > 0.0 && max_dist < max_thresh)
            {
                pts.push_back(max_inter);
            }
        }
    }

    return pts;
}

