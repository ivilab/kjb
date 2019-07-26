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

/* $Id: pt_scene.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_body_2d_trajectory.h"
#include "people_tracking_cpp/pt_entity.h"
#include "people_tracking_cpp/pt_association.h"
#include "people_tracking_cpp/pt_visibility.h"
#include "people_tracking_cpp/pt_data.h"
#include "people_tracking_cpp/pt_util.h"
#include "l_cpp/l_serialization.h"
#include "l_cpp/l_int_matrix.h"
#include "l/l_sys_io.h"
#include "detector_cpp/d_deva_facemark.h"

#include <string>
#include <fstream>
#include <boost/foreach.hpp>

using namespace kjb;
using namespace kjb::pt;

size_t kjb::pt::dims
(   
    const Scene& scene, 
    bool respect_changed,
    bool infer_head
)
{
    size_t nd = 0;
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        IFT(!tg.empty(), Runtime_error,
            "Cannot process scene: has empty targets.");

        nd += dims(tg, respect_changed, infer_head);
    }

    return nd;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::read_scene
(
    Scene& s,
    const std::string& tracks_dp,
    const std::string& ass_fp,
    const std::string& cam_fp,
    const std::string& params_fp,
    const std::string& attn_fp,
    bool infer_head 
)
{
    // read association and camera
    s.association.read(ass_fp, Target(1.0, 1.0, 1.0, 1));
    load(s.camera, cam_fp);

    // read params
    std::ifstream ifs(params_fp.c_str());
    IFTD(ifs, IO_error, "Cannot open file %s.", (params_fp.c_str()));
    ifs >> s.kappa >> s.theta >> s.lambda;
    ifs.close();

    // read person trajectories
    Trajectory_map tmap;
    tmap.parse(tracks_dp, "person");
    Ascn::iterator track_p = s.association.begin();
    BOOST_FOREACH(const Trajectory_map::value_type& pr, tmap)
    {
        // set changes to all so it will update boxes and faces
        track_p->set_changed_all();
        track_p->trajectory() = pr.second;
        track_p->update_boxes(s.camera);
        if(infer_head)
        {
            track_p->update_faces(s.camera);
            track_p->update_walking_angles();
        }
        track_p++;
    }

    update_visibilities(s, infer_head);
    
    // read person hessian vectors if they exist
    boost::format hess_fmt(tracks_dp + "/hessian_%d.txt");
    size_t idx = 1;
    BOOST_FOREACH(const Target& tg, s.association)
    {
        std::string hess_fp = (hess_fmt % idx).str();
        if(kjb_c::is_file(hess_fp.c_str()))
        {
            size_t dim = dims(tg, false, infer_head);
            Vector hd(hess_fp);
            if(dim == hd.size())
            {
                tg.hessian_diagonal() = hd;
                tg.set_unchanged();
            }
        }
        ++idx;
    }

    //if(!infer_head) return;

    // read objects
    Trajectory_map obj_tmap;
    obj_tmap.parse(tracks_dp, "object");
    BOOST_FOREACH(const Trajectory_map::value_type& pr, obj_tmap)
    {
        // set changes to all so it will update boxes and faces
        Target tg(1.0, 1.0, 1.0, obj_tmap.duration());
        tg.trajectory() = pr.second;
        s.objects.push_back(tg);
    }

    // read attention matrix
    if(kjb_c::is_file(attn_fp.c_str()))
    {
        s.attn_matrix = Int_matrix(attn_fp);
        parse_attention_matrix(s);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::write_scene
(
    const Scene& s,
    const std::string& tracks_dp,
    const std::string& ass_fp,
    const std::string& cam_fp,
    const std::string& params_fp,
    const std::string& attn_fp
)
{
    // write persons
    const Entity_type person_et = get_entity_type("person");
    Trajectory_map per_tmap(s.association.get_data().size());

    kjb_c::kjb_mkdir(tracks_dp.c_str());
    boost::format hess_fmt(tracks_dp + "/hessian_%d.txt");

    size_t idx = 1;
    BOOST_FOREACH(const Target& tg, s.association)
    {
        std::string hess_fp = (hess_fmt % idx).str();
        tg.hessian_diagonal().write(hess_fp.c_str());
        Entity_id eid(person_et, idx++);
        per_tmap[eid] = tg.trajectory();
    }

    per_tmap.write(tracks_dp);

    // write objects
    const Entity_type object_et = get_entity_type("object");
    Trajectory_map obj_tmap(s.association.get_data().size());

    idx = 1;
    BOOST_FOREACH(const Target& tg, s.objects)
    {
        Entity_id eid(object_et, idx++);
        obj_tmap[eid] = tg.trajectory();
    }

    obj_tmap.write(tracks_dp);

    // write person association
    s.association.write(ass_fp);
    try
    {
        save(s.camera, cam_fp);
    }
    catch(...)
    {
        s.camera.write(cam_fp.c_str());
    }

    // write params
    std::ofstream ofs(params_fp.c_str());
    ofs << s.kappa << " " << s.theta << " " << s.lambda;

    // write attention matrix
    if(s.attn_matrix.get_num_rows() != 0) s.attn_matrix.write(attn_fp.c_str());
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::parse_attention_matrix(const Scene& scene)
{
    const Int_matrix& attns = scene.attn_matrix;
    if(attns.get_num_rows() == 0 || attns.get_num_cols() == 0) return;

    // parse matrix
    const Ascn& ascn = scene.association;
    size_t idx = 0;
    BOOST_FOREACH(const Target& tg, ascn)
    {
        int sf = tg.get_start_time();
        int ef = tg.get_end_time();
        ASSERT(sf != -1 && ef != -1);

        for(size_t t = sf; t <= ef; ++t)
        {
            size_t tg_idx = attns(idx, t - 1);

            if(tg_idx != 0)
            {
                if(tg_idx <= ascn.size())
                {
                    Ascn::const_iterator tg_p = ascn.begin();
                    std::advance(tg_p, tg_idx - 1);
                    tg.trajectory()[t - 1]->value.attn = &(*tg_p);
                }
                else
                {
                    std::vector<Target>::const_iterator tg_p = scene.objects.begin();
                    std::advance(tg_p, tg_idx - ascn.size() - 1);
                    tg.trajectory()[t - 1]->value.attn = &(*tg_p);
                }
            }
        }

        ++idx;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::update_attention_matrix(Scene& scene)
{
    const Ascn& ascn = scene.association;

    // empty association
    if(ascn.empty())
    {
        scene.attn_matrix = Int_matrix();
        return;
    }

    // build target-index map
    std::map<const Target*, size_t> tg_idxs;
    size_t idx = 1;
    BOOST_FOREACH(const Target& tg, ascn)
    {
        tg_idxs[&tg] = idx++;
    }

    BOOST_FOREACH(const Target& tg, scene.objects)
    {
        tg_idxs[&tg] = idx++;
    }

    // fill in matrix
    scene.attn_matrix = Int_matrix(ascn.size(), ascn.get_data().size(), 0);
    bool alo = false;
    idx = 1;
    BOOST_FOREACH(const Target& tg, ascn)
    {
        int sf = tg.get_start_time();
        int ef = tg.get_end_time();
        ASSERT(sf != -1 && ef != -1);

        for(size_t t = sf; t <= ef; ++t)
        {
            const Target* tg_p = tg.trajectory()[t - 1]->value.attn;
            if(tg_p != 0)
            {
                scene.attn_matrix(idx - 1, t - 1) = tg_idxs[tg_p];
                alo = true;
            }
        }

        ++idx;
    }

    if(!alo) scene.attn_matrix = Int_matrix();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::set_trajectory_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    const Vector3& v,
    bool vis_off,
    bool infer_head
)
{
    const Perspective_camera& cam = scene.camera;
    Trajectory& traj = target.trajectory();
    Body_2d_trajectory& btraj = target.body_trajectory();

    int sf = target.get_start_time();
    int ef = target.get_end_time();

    ASSERT(sf != -1 && ef != -1);
    IFT(ef >= sf && frame <= ef, Illegal_argument,
        "Cannot set trajectory value; invalid frame.");

    // change position
    traj[frame - 1]->value.position = v;

    // update body
    btraj[frame - 1]->value = project_cstate(traj[frame - 1]->value,
                            cam, traj.height, traj.width, traj.girth);

    // update face
    if(infer_head)
    {
        Face_2d_trajectory& ftraj = target.face_trajectory();
        const Deva_facemark* fm_p = ftraj[frame - 1]->value.facemark;
        ftraj[frame - 1]->value = project_cstate_face(traj[frame - 1]->value,
                                cam, traj.height, traj.width, traj.girth);
        ftraj[frame - 1]->value.facemark = fm_p;
    }

    // update body dirs
    update_body_model_directions(target, frame, cam);

    // update face dirs
    if(infer_head) update_face_model_directions(target, frame, cam);

    // update visibility
    if(!vis_off)
    {
        update_visibilities(scene, frame, infer_head);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::set_trajectories_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    const Vector3& v1,
    const Vector3& v2,
    bool vis_off,
    bool infer_head
)
{
    if(&target1 == &target2 && frame1 == frame2)
    {
        if(v1 != v2)
        {
            std::cerr << "WARNING: setting same trajectory variable to "
                      << "different values."
                      << std::endl;
        }

        set_trajectory_at_frame(scene, target1, frame1, v1, vis_off, infer_head);
        return;
    }

    set_trajectory_at_frame(scene, target1, frame1, v1, vis_off, infer_head);
    set_trajectory_at_frame(scene, target2, frame2, v2, vis_off, infer_head);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::set_trajectory_dir_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    double dir,
    bool vis_off
)
{
    const Perspective_camera& cam = scene.camera;
    Trajectory& traj = target.trajectory();
    Body_2d_trajectory& btraj = target.body_trajectory();
    Face_2d_trajectory& ftraj = target.face_trajectory();

    int sf = target.get_start_time();
    int ef = target.get_end_time();

    ASSERT(sf != -1 && ef != -1);
    IFT(ef >= sf && frame <= ef, Illegal_argument,
        "Cannot set direction value; invalid frame.");

    // change direction
    traj[frame - 1]->value.body_dir = dir;

    // update body
    btraj[frame - 1]->value = project_cstate(traj[frame - 1]->value,
                            cam, traj.height, traj.width, traj.girth);

    // update face
    const Deva_facemark* fm_p = ftraj[frame - 1]->value.facemark;
    ftraj[frame - 1]->value = project_cstate_face(traj[frame - 1]->value,
                            cam, traj.height, traj.width, traj.girth);
    ftraj[frame - 1]->value.facemark = fm_p;

    // update model dirs
    update_model_directions(target, frame, cam);

    // update visibility
    if(!vis_off)
    {
        update_visibilities(scene, frame);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::set_trajectory_dirs_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    double dir1,
    double dir2,
    bool vis_off
)
{
    if(&target1 == &target2 && frame1 == frame2)
    {
        if(dir1 != dir2)
        {
            std::cerr << "WARNING: setting same direction variable to "
                      << "different values."
                      << std::endl;
        }

        set_trajectory_dir_at_frame(scene, target1, frame1, dir1, vis_off);
        return;
    }

    set_trajectory_dir_at_frame(scene, target1, frame1, dir1, vis_off);
    set_trajectory_dir_at_frame(scene, target2, frame2, dir2, vis_off);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::set_trajectory_face_dir_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    const Vector2& dir,
    bool vis_off
)
{
    const Perspective_camera& cam = scene.camera;
    Trajectory& traj = target.trajectory();
    Face_2d_trajectory& ftraj = target.face_trajectory();

    int sf = target.get_start_time();
    int ef = target.get_end_time();

    ASSERT(sf != -1 && ef != -1);
    IFT(ef >= sf && frame <= ef, Illegal_argument,
        "Cannot set direction value; invalid frame.");

    // change direction
    traj[frame - 1]->value.face_dir = dir;

    // update face
    const Deva_facemark* fm_p = ftraj[frame - 1]->value.facemark;
    ftraj[frame - 1]->value = project_cstate_face(traj[frame - 1]->value,
                            cam, traj.height, traj.width, traj.girth);
    ftraj[frame - 1]->value.facemark = fm_p;

    // update model dirs
    if(frame != ef && traj[frame] && 
            valid_face_dir(traj[frame - 1]->value) &&
            valid_face_dir(traj[frame]->value))
    {
        ftraj[frame - 1]->value.model_dir = face_model_direction(
                                                    traj[frame - 1]->value,
                                                    traj[frame]->value,
                                                    cam,
                                                    traj.height,
                                                    traj.width,
                                                    traj.girth);
    }

    if(frame != sf && traj[frame - 2] &&
            valid_face_dir(traj[frame - 2]->value) &&
            valid_face_dir(traj[frame - 1]->value))
    {
        ftraj[frame - 2]->value.model_dir = face_model_direction(
                                                    traj[frame - 2]->value,
                                                    traj[frame - 1]->value,
                                                    cam,
                                                    traj.height,
                                                    traj.width,
                                                    traj.girth);
    }

    // update visibility
    if(!vis_off)
    {
        update_visibilities(scene, frame);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::set_trajectory_face_dirs_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    const Vector2& dir1,
    const Vector2& dir2,
    bool vis_off
)
{
    if(&target1 == &target2 && frame1 == frame2)
    {
        if(dir1 != dir2)
        {
            std::cerr << "WARNING: setting same direction variable to "
                      << "different values."
                      << std::endl;
        }

        set_trajectory_face_dir_at_frame(scene, target1, frame1, dir1, vis_off);
        return;
    }

    set_trajectory_face_dir_at_frame(scene, target1, frame1, dir1, vis_off);
    set_trajectory_face_dir_at_frame(scene, target2, frame2, dir2, vis_off);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::update_body_model_directions
(
    const Target& target,
    size_t frame,
    const Perspective_camera& cam
)
{
    Trajectory& traj = target.trajectory();
    Body_2d_trajectory& btraj = target.body_trajectory();

    int sf = target.get_start_time();
    int ef = target.get_end_time();

    if(frame != ef && traj[frame] && valid_body_dir(traj[frame - 1]->value))
    {
        btraj[frame - 1]->value.model_dir = model_direction(
                                                    traj[frame - 1]->value,
                                                    traj[frame]->value,
                                                    cam,
                                                    traj.height,
                                                    traj.width,
                                                    traj.girth);
    }

    if(frame != sf && traj[frame - 2] && valid_body_dir(traj[frame - 2]->value))
    {
        btraj[frame - 2]->value.model_dir = model_direction(
                                                    traj[frame - 2]->value,
                                                    traj[frame - 1]->value,
                                                    cam,
                                                    traj.height,
                                                    traj.width,
                                                    traj.girth);

    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::update_face_model_directions
(
    const Target& target,
    size_t frame,
    const Perspective_camera& cam
)
{
    Trajectory& traj = target.trajectory();
    Face_2d_trajectory& ftraj = target.face_trajectory();

    int sf = target.get_start_time();
    int ef = target.get_end_time();

    if(frame != ef && traj[frame] &&
            valid_face_dir(traj[frame - 1]->value) &&
            valid_face_dir(traj[frame]->value))
    {

        ftraj[frame - 1]->value.model_dir = face_model_direction(
                                                    traj[frame - 1]->value,
                                                    traj[frame]->value,
                                                    cam,
                                                    traj.height,
                                                    traj.width,
                                                    traj.girth);
    }

    if(frame != sf && traj[frame - 2] &&
            valid_face_dir(traj[frame - 2]->value) &&
            valid_face_dir(traj[frame - 1]->value))
    {
        ftraj[frame - 2]->value.model_dir = face_model_direction(
                                                    traj[frame - 2]->value,
                                                    traj[frame - 1]->value,
                                                    cam,
                                                    traj.height,
                                                    traj.width,
                                                    traj.girth);
    }
}
