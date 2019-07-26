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

#ifndef PT_SCENE_ADAPTER_H
#define PT_SCENE_ADAPTER_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <map>

namespace kjb {
namespace pt {

/**
 * @brief   Gets the scene at the given variable. Here, d = 1 ... 5
 *          represents the different parts of the trajectory at time 'frame'.
 *          1 and 2 are the position x and z, 3 is the body direction, and
 *          4 and 5 are the face direction
 */
inline
double get_variable_at_frame
(
    const Scene&,
    const Target& target,
    size_t frame,
    size_t d,
    size_t infer_head
)
{
    if(!infer_head) { assert(d <= 1); }
    switch(d)
    {
        case 0: return target.trajectory()[frame - 1]->value.position[0];
        case 1: return target.trajectory()[frame - 1]->value.position[2];
        case 2: return target.trajectory()[frame - 1]->value.body_dir;
        case 3: return target.trajectory()[frame - 1]->value.face_dir[0];
        case 4: return target.trajectory()[frame - 1]->value.face_dir[1];

        default: KJB_THROW_2(
                    Illegal_argument,
                    "Cannot get variable: bad index");
    }
}

/**
 * @brief   Sets the scene at the given variable. Here, d = 1 ... 5
 *          represents the different parts of the trajectory at time 'frame'.
 *          1 and 2 are the position x and z, 3 is the body direction, and
 *          4 and 5 are the face direction
 */
inline
void set_variable_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    size_t d,
    double x,
    bool vo,
    bool infer_head
)
{
    // current values
    double px = target.trajectory()[frame - 1]->value.position[0];
    double pz = target.trajectory()[frame - 1]->value.position[2];
    double fd1 = 0.0;
    double fd2 = 0.0;
    if(infer_head)
    {
        fd1 = target.trajectory()[frame - 1]->value.face_dir[0];
        fd2 = target.trajectory()[frame - 1]->value.face_dir[1];
    }
    else
    {
        assert(d <= 1);
    }

    // set new values, while keeping old ones where applicable
    switch(d)
    {
        case 0: set_trajectory_at_frame(
                    scene, target, frame, Vector3(x, 0.0, pz), vo, infer_head);
        break;

        case 1: set_trajectory_at_frame(
                    scene, target, frame, Vector3(px, 0.0, x), vo, infer_head);
        break;

        case 2: set_trajectory_dir_at_frame(scene, target, frame, x, vo);
        break;

        case 3: set_trajectory_face_dir_at_frame(
                    scene, target, frame, Vector2(x, fd2), vo);
        break;

        case 4: set_trajectory_face_dir_at_frame(
                    scene, target, frame, Vector2(fd1, x), vo);
        break;

        default: KJB_THROW_2(
                    Illegal_argument,
                    "Cannot set variable: bad index");
    }
}

/**
 * @brief   Moves the scene at the given variable. Here, d = 1 ... 5
 *          represents the different parts of the trajectory at time 'frame'.
 *          1 and 2 are the position x and z, 3 is the body direction, and
 *          4 and 5 are the face direction
 */
inline
void move_variable_at_frame
(
    const Scene& scene,
    const Target& target,
    size_t frame,
    size_t d,
    double dx,
    bool vo,
    bool infer_head
)
{
    if(!infer_head)
    {
        assert(d <= 1);
    }
    switch(d)
    {
        case 0: move_trajectory_at_frame(
                    scene, target, frame, Vector3(dx, 0.0, 0.0), vo, infer_head);
        break;

        case 1: move_trajectory_at_frame(
                    scene, target, frame, Vector3(0.0, 0.0, dx), vo, infer_head);
        break;

        case 2: move_trajectory_dir_at_frame(scene, target, frame, dx, vo);
        break;

        case 3: move_trajectory_face_dir_at_frame(
                    scene, target, frame, Vector2(dx, 0.0), vo);
        break;

        case 4: move_trajectory_face_dir_at_frame(
                    scene, target, frame, Vector2(0.0, dx), vo);
        break;

        default: KJB_THROW_2(
                    Illegal_argument,
                    "Cannot move variable: bad index");
    }
}

/**
 * @class   Scene_adapter
 * @brief   Adapts a Scene into a VectorModel for HMC sampling.
 *
 * This class is made specifically for HMC sampling, which means it assumes
 * that it will be called in a certain way. If used in a different way,
 * it will most likely give incorrect results.
 */
class Scene_adapter
{
public:
    Scene_adapter(bool vis_off = false, bool infer_head = true)
        : scene_p_(0),
          m_vis_off(vis_off),
          m_infer_head(infer_head)
    {}

    /** @brief  Get the ith element of a scene. */
    double get(const Scene* s, size_t i) const
    {
        compute_vars(s);
        IFT(size(s) != 0, Runtime_error, "Cannot get: no changed targets");
        return get_variable_at_frame(
                    *s, *tgt_ps_[i], frames_[i], dims_[i], m_infer_head);
    }

    /** @brief  Set the ith element of a scene to x. */
    void set(Scene* s, size_t i, double x) const
    {
        compute_vars(s);
        IFT(size(s) != 0, Runtime_error, "Cannot set: no changed targets");
        set_variable_at_frame(
            *s, *tgt_ps_[i], frames_[i], dims_[i], x, m_vis_off, m_infer_head);
    }

    /** @brief  Set the ith element of a scene to x. */
    void set(Scene* s, size_t i, size_t j, double x, double y) const
    {
        if(i == j)
        {
            std::cerr << "Scene_adapter::set received same index" << std::endl;
        }

        if(j < i)
        {
            using std::swap;
            swap(i, j);
            swap(x, y);
        }

        set(s, i, x);
        set(s, j, y);
    }

    /** @brief  Set all elements of s to specified values. */
    void set(Scene* s, const Vector& x) const;

    /** @brief  Get the number of elements in this scene. */
    size_t size(const Scene* s) const
    {
        compute_vars(s);
        return tgt_ps_.size();
    }

    /** @brief  Restart state. */
    void reset() const
    {
        scene_p_ = 0;
        tgt_ps_.clear();
        frames_.clear();
        dims_.clear();
    }

    /** @brief  Get target and frame corresponding to variable i. */
    std::pair<const Target*, size_t>
        target_frame(const Scene* s, size_t i) const
    {
        if(i >= size(s)) return std::make_pair((const Target*)0, 0);

        return std::make_pair(tgt_ps_[i], frames_[i]);
    }

    /** @brief   Returns whether inferring head or not. */
    bool infer_head() const { return m_infer_head; }

private:
    /* @brief   Converts scene into vector. */
    void compute_vars(const Scene* s) const;

    // members
    mutable const Scene* scene_p_;
    mutable std::vector<const Target*> tgt_ps_;
    mutable std::vector<size_t> frames_;
    mutable std::vector<size_t> dims_;

    bool m_vis_off;
    bool m_infer_head;
};

}} // namespace kjb::pt

#endif /*PT_SCENE_ADAPTER_H */

