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

#ifndef B3_VISUALIZER_H
#define B3_VISUALIZER_H

#include "bbb_cpp/bbb_description.h"
#include "bbb_cpp/bbb_data.h"
#include "bbb_cpp/bbb_trajectory.h"
#include "bbb_cpp/bbb_activity_library.h"
#include "gr_cpp/gr_glut.h"
#include "l_cpp/l_exception.h"
#include "m_cpp/m_vector.h"
#include "camera_cpp/perspective_camera.h"
#include "video_cpp/video.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"

#include <algorithm>
#include <boost/optional.hpp>

namespace kjb {
namespace bbb {

//// UTILITY FUNCTIONS ////

/** @brief  Compute the min in the X-direction. */
inline
double trajectory_min_x(const Trajectory& traj)
{
    const Trajectory::vec_t& v = traj.dim(0);
    return *std::min_element(v.begin(), v.end());
}

/** @brief  Compute the max in the X-direction. */
inline
double trajectory_max_x(const Trajectory& traj)
{
    const Trajectory::vec_t& v = traj.dim(0);
    return *std::max_element(v.begin(), v.end());
}

/** @brief  Compute the min in the Z-direction. */
inline
double trajectory_min_z(const Trajectory& traj)
{
    const Trajectory::vec_t& v = traj.dim(1);
    return *std::min_element(v.begin(), v.end());
}

/** @brief  Compute the max in the Z-direction. */
inline
double trajectory_max_z(const Trajectory& traj)
{
    const Trajectory::vec_t& v = traj.dim(1);
    return *std::max_element(v.begin(), v.end());
}

/** @brief  Visualize a description and the corresponding data. */
class Visualizer
{
public:
    /**
     * @brief   Construct a visualizer with the given description, data, and
     *          camera.
     */
    Visualizer
    (
        const Description& description,
        const Data& data,
        const Perspective_camera& camera,
        const Activity_library& library
    ) :
#ifdef KJB_HAVE_OPENGL
        glwin_(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Visualize description"),
#endif /* KJB_HAVE_OPENGL */
        desc_p_(&description),
        data_p_(&data),
        cam_p_(&camera),
        lib_p_(&library),
        frame_(1),
        draw_images_(false)
    {
        init_gl();
    }

    /**
     * @brief   Construct a visualizer with the given description and data,
     *          using an overhead view of the scene.
     */
    Visualizer
    (
        const Description& description,
        const Data& data,
        const Activity_library& library
    ) :
#ifdef KJB_HAVE_OPENGL
        glwin_(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Visualize description"),
#endif /* KJB_HAVE_OPENGL */
        desc_p_(&description),
        data_p_(&data),
        cam_p_(0),
        lib_p_(&library),
        frame_(1),
        draw_images_(false)
    {
        init_gl();
        set_overhead_view();
    }

public:
    /** @brief  Set frame images from files. */
    template <class InputIterator>
    void set_frame_images(InputIterator first, InputIterator last)
    {
        IFT(std::distance(first, last) >= desc_p_->end(),
            Illegal_argument, "Not enough frames");

        video_.load_images(first, last);
        resize(video_.get_width(), video_.get_height());
    }

    /** @brief  Set an overhead view. */
    void set_alternative_camera(const Perspective_camera& cam)
    {
        alt_camera_ = cam;
    }

    /** @brief  Set an overhead view. */
    void set_overhead_view();

    /** @brief  Return to the scene's camera. */
    void clear_alternative_camera();

    /** @brief  Resize this viewer. */
    void resize(size_t width, size_t height)
    {
#ifdef KJB_HAVE_OPENGL
        glwin_.set_size(width, height);
#else /* KJB_HAVE_OPENGL */
        KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
    }

    /** @brief  Set the current frame to the given value. */
    void set_frame(size_t f)
    {
        IFT(f >= 1 && f <= desc_p_->end(),
            Illegal_argument, "Cannot set frame: value outside of range.");

        frame_ = f;
    }

    /** @brief  Advance the current frame by the given value. */
    void advance_frame(size_t df = 1)
    {
        frame_ += df;

        if(frame_ > desc_p_->end())
        {
            size_t ndf = frame_ - desc_p_->end() - 1;
            frame_ = 1;
            advance_frame(ndf);
        }
    }

    /** @brief  Rewind (move backwards) the current frame by the given value. */
    void rewind_frame(size_t df = 1)
    {
        if(df >= frame_)
        {
            size_t ndf = df - frame_;
            frame_ = desc_p_->end();
            rewind_frame(ndf);
        }
        else
        {
            frame_ -= df;
        }
    }

    /** @brief  Set the key handling callback. */
    template<class Func>
    void set_key_callback(const Func& cb)
    {
#ifdef KJB_HAVE_OPENGL
        glwin_.set_keyboard_callback(cb);
#else /* KJB_HAVE_OPENGL */
        KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
    }

    /** Set whether or not the images are rendered in the background. */
    void draw_images(bool di);

public:
    /** @brief  Get this viewer's width. */
    double width() const 
    { 
#ifdef KJB_HAVE_OPENGL
        return glwin_.get_width(); 
#else /* KJB_HAVE_OPENGL */
        KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
    }

    /** @brief  Get this viewer's height. */
    double height() const {
#ifdef KJB_HAVE_OPENGL
        return glwin_.get_height();
#else /* KJB_HAVE_OPENGL */
        KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
    }

private:
    /** @brief  Initialize GL sutff. Called from constructor. */
    void init_gl();

    /** @brief  Renders the scene. */
    void render_scene() const;

    /** @brief  Renders a trajectory. */
    void render_trajectory(const Trajectory& traj) const;

    /** @brief  Sample random color. */
    Vector random_color() const
    {
        static Uniform_distribution U;
        return Vector(sample(U), sample(U), sample(U));
    }

private:
#ifdef KJB_HAVE_OPENGL
    opengl::Glut_window glwin_;
#endif /* KJB_HAVE_OPENGL */
    const Description* desc_p_;
    const Data* data_p_;
    const Perspective_camera* cam_p_;
    const Activity_library* lib_p_;
    Video video_;
    size_t frame_;
    boost::optional<Perspective_camera> alt_camera_;
    bool draw_images_;

    typedef std::map<const Physical_activity*, Vector> Color_map;
    mutable Color_map cur_colors_;

private:
    // constants
    static const size_t DEFAULT_WIDTH = 800;
    static const size_t DEFAULT_HEIGHT = 800;
};

}} // namespace kjb::bbb

#endif /*B3_VISUALIZER_H */

