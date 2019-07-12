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

/* $Id: pt_scene.h 20912 2016-10-30 17:52:31Z ernesto $ */

#ifndef PT_SCENE_H
#define PT_SCENE_H

#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_data.h>
#include <camera_cpp/perspective_camera.h>
#include <l_cpp/l_int_matrix.h>
#include <string>
#include <iterator>
#include <boost/format.hpp>

namespace kjb {
namespace pt {

/**
 * @class   Scene
 *
 * @brief   Class that represents a full scene in the PT universe.
 */
class Scene
{
public:
    Scene
    (
        const Ascn& assoc,
        const Perspective_camera& cam,
        double k,
        double t,
        double l
    ) :
        association(assoc),
        camera(cam),
        kappa(k),
        theta(t),
        lambda(l)
    {}

    Ascn association;
    std::vector<Target> objects;
    Perspective_camera camera;
    double kappa;
    double theta;
    double lambda;
    Int_matrix attn_matrix;
};

/** @brief  Swap two scenes. */
inline
void swap(Scene& s1, Scene& s2)
{
    using std::swap;

    swap(s1.association, s2.association);
    swap(s1.objects, s2.objects);
    swap(s1.camera, s2.camera);
    swap(s1.kappa, s2.kappa);
    swap(s1.theta, s2.theta);
    swap(s1.lambda, s2.lambda);
    swap(s1.attn_matrix, s2.attn_matrix);
}

/** @brief  Computes the number of variables in this scene. */
size_t dims
(
    const Scene& scene,
    bool respect_changed = true,
    bool infer_head = true
);

/**
 * @brief   Read a scene.
 *
 * @param   s           Target scene variable.
 * @param   tracks_dp   Directory where trajectory files will be read.
 * @param   ass_fp      Path where association file will be written; it is
 *                      usually in the directory tracks_dp.
 * @param   cam_fp      Path where camera file will be read; it is usually
 *                      in the directory tracks_dp.
 * @param   params_fp   Path where parameters file will be read; it is
 */
void read_scene
(
    Scene& s,
    const std::string& tracks_dp,
    const std::string& ass_fp,
    const std::string& cam_fp,
    const std::string& params_fp,
    const std::string& attn_fp,
    bool infer_head = true
);

/**
 * @brief   Read a scene using standard subdirectory structure; e.g.,
 *          the association is read from "association.txt".
 *
 * @param   scene       Target scene variable.
 * @param   tracks_dp   Directory where trajectory files will be read.
 */
inline
void read_scene
(
    Scene& scene,
    const std::string& tracks_dp,
    bool infer_head = true
)
{
    read_scene(
            scene,
            tracks_dp,
            tracks_dp + "/association.txt",
            tracks_dp + "/camera.txt",
            tracks_dp + "/params.txt",
            tracks_dp + "/attns.txt",
            infer_head);
}

/**
 * @brief   Write a scene.
 *
 * @param   s           Scene to be written.
 * @param   tracks_dp   Directory where trajectory files will be written.
 * @param   ass_fp      Path where association file will be written; it is
 *                      usually in the directory tracks_dp.
 * @param   cam_fp      Path where camera file will be written; it is usually
 *                      in the directory tracks_dp.
 * @param   params_fp   Path where parameters file will be written; it is
 *                      usually in the directory tracks_dp.
 */
void write_scene
(
    const Scene& s,
    const std::string& tracks_dp,
    const std::string& ass_fp,
    const std::string& cam_fp,
    const std::string& params_fp,
    const std::string& attn_fp
);

/**
 * @brief   Write a scene using standard subdirectory structure; e.g.,
 *          the association is written to "association.txt".
 *
 * @param   scene       Scene to be written.
 * @param   tracks_dp   Directory where trajectory files will be written.
 */
inline
void write_scene(const Scene& scene, const std::string& tracks_dp)
{
    write_scene(
            scene,
            tracks_dp,
            tracks_dp + "/association.txt",
            tracks_dp + "/camera.txt",
            tracks_dp + "/params.txt",
            tracks_dp + "/attns.txt");
}

/** @brief  Set attentions from attention matrix. */
void parse_attention_matrix(const Scene& scene);

/** @brief  Convert scene attentions to matrix. */
void update_attention_matrix(Scene& scene);

/** @brief  Record a series of scenes to a directory. */
class Scene_recorder
{
public:
    Scene_recorder(const std::string& dir) : parent_dir(dir), i(1){}

    /** @brief  Record a scene. */
    void operator()(const Scene& scene) const
    {
        std::string cur_dir = boost::str(boost::format("/%04d") % i++);
        write_scene(scene, parent_dir + cur_dir);
    }

    /** @brief  Reset current sample number. */
    void reset() const
    {
        i = 1;
    }

private:
    std::string parent_dir;
    mutable size_t i;
};

/** @brief  Record a series of scenes to a directory. */
class Write_scene_iterator :
    public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:
    /** @brief  Fake container type. Needed for recorder to find the type. */
    struct container_type
    {
        typedef Scene value_type;
    };

    /** @brief  Construct an iterator. */
    Write_scene_iterator(const std::string& dir) : parent_dir(dir), i(1), n(i){}

    /** @brief  Construct an iterator. */
    Write_scene_iterator(const std::string& dir, size_t& sn) :
        parent_dir(dir), n(sn){}

    /** @brief  Upon assignment, write a scene. */
    void operator=(const Scene& scene)
    {
        std::string cur_dir = boost::str(boost::format("/%04d") % n);
        write_scene(scene, parent_dir + cur_dir);
    }

    /**
     * @brief   Return itself on redirection. This complies with the
     *          output iterator concept, as *it = sc produces the
     *          correct result.
     */
    Write_scene_iterator& operator*()
    {
        return *this;
    }

    /** @brief  Increments scene index. */
    Write_scene_iterator& operator++(int)
    {
        n++;
        return *this;
    }

    /** @brief  Increments scene index. */
    Write_scene_iterator& operator++()
    {
        return (*this)++;
    }

    /** @brief  Reset current sample number. */
    void reset()
    {
        n = 1;
    }

private:
    std::string parent_dir;
    size_t i;
    size_t& n;
};

/**
 * @brief   Function that changes a single trajectory at a single frame, and
 *          performs all the necessary updates.
 */
void set_trajectory_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    const Vector3& v,
    bool vis_off,
    bool infer_head = true
);

/**
 * @brief   Function that changes a pair of trajectories at a pair of frames,
 *          and performs all the necessary updates.
 */
void set_trajectories_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    const Vector3& v1,
    const Vector3& v2,
    bool vis_off,
    bool infer_head = true
);

/**
 * @brief   Function that moves a single trajectory at a single frame, and
 *          performs all the necessary updates.
 */
inline
void move_trajectory_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    const Vector3& dv,
    bool vis_off,
    bool infer_head = true
)
{
    const Vector3& v = target.trajectory()[frame - 1]->value.position;
    set_trajectory_at_frame(scene, target, frame, v + dv, vis_off, infer_head);
}

/**
 * @brief   Function that moves a pair of trajectories at a pair of frames,
 *          and performs all the necessary updates.
 */
inline
void move_trajectories_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    const Vector3& dv1,
    const Vector3& dv2,
    bool vis_off,
    bool infer_head = true
)
{
    const Vector3& v1 = target1.trajectory()[frame1 - 1]->value.position;
    const Vector3& v2 = target2.trajectory()[frame2 - 1]->value.position;

    set_trajectories_at_frames(scene,
                               target1, target2,
                               frame1, frame2,
                               v1 + dv1, v2 + dv2,
                               vis_off,
                               infer_head);
}

/**
 * @brief   Function that changes a single direction at a single frame, and
 *          performs all the necessary updates.
 */
void set_trajectory_dir_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    double dir,
    bool vis_off
);

/**
 * @brief   Function that changes a pair of directions at a pair of frames,
 *          and performs all the necessary updates.
 */
void set_trajectory_dirs_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    double dir1,
    double dir2,
    bool vis_off
);

/**
 * @brief   Function that moves a single direction at a single frame, and
 *          performs all the necessary updates.
 */
inline
void move_trajectory_dir_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    double dd,
    bool vis_off
)
{
    double d = target.trajectory()[frame - 1]->value.body_dir;
    set_trajectory_dir_at_frame(scene, target, frame, d + dd, vis_off);
}

/**
 * @brief   Function that moves a pair of directions at a pair of frames,
 *          and performs all the necessary updates.
 */
inline
void move_trajectory_dirs_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    double dd1,
    double dd2,
    bool vis_off
)
{
    double d1 = target1.trajectory()[frame1 - 1]->value.body_dir;
    double d2 = target2.trajectory()[frame2 - 1]->value.body_dir;

    set_trajectory_dirs_at_frames(scene,
                                  target1, target2,
                                  frame1, frame2,
                                  d1 + dd1, d2 + dd2,
                                  vis_off);
}

/**
 * @brief   Function that changes a single face direction at a single frame, and
 *          performs all the necessary updates.
 */
void set_trajectory_face_dir_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    const Vector2& dir,
    bool vis_off
);

/**
 * @brief   Function that changes a pair of face directions at a pair of frames,
 *          and performs all the necessary updates.
 */
void set_trajectory_face_dirs_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    const Vector2& dir1,
    const Vector2& dir2,
    bool vis_off
);

/**
 * @brief   Function that moves a single face direction at a single frame, and
 *          performs all the necessary updates.
 */
inline
void move_trajectory_face_dir_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    const Vector2& dd,
    bool vis_off
)
{
    const Vector2& d = target.trajectory()[frame - 1]->value.face_dir;
    set_trajectory_face_dir_at_frame(scene, target, frame, d + dd, vis_off);
}

/**
 * @brief   Function that moves a pair of directions at a pair of frames,
 *          and performs all the necessary updates.
 */
inline
void move_trajectory_face_dirs_at_frames
(
    const Scene& scene,
    const Target& target1,
    const Target& target2,
    size_t frame1,
    size_t frame2,
    const Vector2& dd1,
    const Vector2& dd2,
    bool vis_off
)
{
    const Vector2& d1 = target1.trajectory()[frame1 - 1]->value.face_dir;
    const Vector2& d2 = target2.trajectory()[frame2 - 1]->value.face_dir;

    set_trajectory_face_dirs_at_frames(scene,
                                       target1, target2,
                                       frame1, frame2,
                                       d1 + dd1, d2 + dd2,
                                       vis_off);
}

/** @brief  Helper function for the set_trajectory_* functions. */
void update_body_model_directions
(
    const Target& target,
    size_t frame,
    const Perspective_camera& cam
);

/** @brief  Helper function for the set_trajectory_* functions. */
void update_face_model_directions
(
    const Target& target,
    size_t frame,
    const Perspective_camera& cam
);

/** @brief  Helper function for the set_trajectory_* functions. */
inline void update_model_directions
(
    const Target& target,
    size_t frame,
    const Perspective_camera& cam
)
{
    update_body_model_directions(target, frame, cam);
    update_face_model_directions(target, frame, cam);
}

}} //namespace kjb::pt

#endif /*PT_SCENE_H */

