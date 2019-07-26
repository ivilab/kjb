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

/* $Id: pt_scene_posterior.h 20945 2016-11-08 17:32:43Z jguan1 $ */

#ifndef PT_SCENE_POSTERIOR_H
#define PT_SCENE_POSTERIOR_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_position_prior.h>
#include <people_tracking_cpp/pt_direction_prior.h>
#include <people_tracking_cpp/pt_box_likelihood.h>
#include <people_tracking_cpp/pt_facemark_likelihood.h>
#include <people_tracking_cpp/pt_optical_flow_likelihood.h>
#include <people_tracking_cpp/pt_face_flow_likelihood.h>
#include <people_tracking_cpp/pt_color_likelihood.h>
#include <people_tracking_cpp/pt_entity.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <camera_cpp/perspective_camera.h>
#include <prob_cpp/prob_distribution.h>
#include <detector_cpp/d_bbox.h>
#include <iostream>

namespace kjb {
namespace pt {

/**
 * @class   Scene_posterior
 * @brief   Posterior distribution of a scene
 *
 * This class represents the posterior distribution of a scene (aka
 * the complete-data likelihood, aka the joint distribution of a fixed
 * scene).
 */
class Scene_posterior
{
public:
    Scene_posterior
    (
        const Box_likelihood& box_likelihood,
        const Facemark_likelihood& fm_likelihood,
        const Optical_flow_likelihood& of_likelihood,
        const Face_flow_likelihood& ff_likelihood,
        const Color_likelihood& color_likelihood,
        const Position_prior& pos_prior,
        const Direction_prior& dir_prior,
        const Face_direction_prior& fdir_prior,
        bool vis_off = false,
        bool use_fm_off = false,
        bool infer_head_off = false
    ) :
        box_likelihood_(box_likelihood),
        fm_likelihood_(fm_likelihood),
        of_likelihood_(of_likelihood),
        ff_likelihood_(ff_likelihood),
        color_likelihood_(color_likelihood),
        pos_prior_(pos_prior),
        dir_prior_(dir_prior),
        fdir_prior_(fdir_prior),
        N_h(get_entity_type_average_height(get_entity_type("person")),
            get_entity_type_stddev_height(get_entity_type("person"))),
        N_w(get_entity_type_average_width(get_entity_type("person")),
            get_entity_type_stddev_width(get_entity_type("person"))),
        N_g(get_entity_type_average_girth(get_entity_type("person")),
            get_entity_type_stddev_girth(get_entity_type("person"))),
        use_box_lh_(true),
        use_fm_lh_(!use_fm_off),
        use_of_lh_(true),
        use_ff_lh_(true),
        use_color_lh_(false),
        use_pos_prior_(true),
        use_dir_prior_(true),
        use_fdir_prior_(true),
        use_dim_prior_(true),
        m_vis_off(vis_off),
        m_infer_head(!infer_head_off)
    {}

public:
    /** @brief  Computes the posterior of a scene. */
    double operator()(const Scene& scene) const;

    /**
     * @brief   Computes the likelihood of a scene where only a single
     *          item moved
     */
    double local
    (
        const Target& target,
        const Scene& scene,
        size_t frame
    ) const
    {
        return local(target, target, scene, frame, frame);
    }

    /**
     * @brief   Computes the likelihood of a scene where only two
     *          items moved
     */
    double local
    (
        const Target& target1,
        const Target& target2,
        const Scene& scene,
        size_t frame1,
        size_t frame2
    ) const;

public:
    /** @brief  Returns whether or not we are using the box likelihood. */
    bool& use_box_lh() const { return use_box_lh_; }

    /** @brief  Returns whether or not we are using the facemark likelihood. */
    bool& use_fm_lh() const { return use_fm_lh_; }

    /** @brief  Returns whether or not we are using the OF likelihood. */
    bool& use_of_lh() const { return use_of_lh_; }

    /** @brief  Returns whether or not we are using the FF likelihood. */
    bool& use_ff_lh() const { return use_ff_lh_; }

    /** @brief  Returns whether or not we are using the color likelihood. */
    bool& use_color_lh() const { return use_color_lh_; }

    /** @brief  Returns whether or not we are using the position prior. */
    bool& use_pos_prior() const { return use_pos_prior_; }

    /** @brief  Returns whether or not we are using the direction prior. */
    bool& use_dir_prior() const { return use_dir_prior_; }

    /** @brief  Returns whether or not we are using the face direction prior. */
    bool& use_fdir_prior() const { return use_fdir_prior_; }

    /** @brief  Returns whether or not we are using the dimension prior. */
    bool& use_dim_prior() const { return use_dim_prior_; }

    /** @brief  Returns whether visibilities is being used. */
    bool vis_off() const { return m_vis_off; }

    /** @brief  Returns whether inferring head. */
    bool infer_head() const { return m_infer_head; }

public:
    /** @brief  Returns the position prior. */
    const Position_prior& position_prior() const { return pos_prior_; }

    /** @brief  Returns the direction prior. */
    const Direction_prior& direction_prior() const { return dir_prior_; }

    /** @brief  Returns the face direction prior. */
    const Face_direction_prior& face_direction_prior() const
    {
        return fdir_prior_;
    }

    /** @brief  Returns the height prior. */
    const Normal_distribution& height_prior() const { return N_h; }

    /** @brief  Returns the width prior. */
    const Normal_distribution& width_prior() const { return N_w; }

    /** @brief  Returns the girth prior. */
    const Normal_distribution& girth_prior() const { return N_g; }

    /** @brief  Returns the box likelihood. */
    const Box_likelihood& box_likelihood() const { return box_likelihood_; }

    /** @brief  Returns the facemark likelihood. */
    const Facemark_likelihood& fm_likelihood() const { return fm_likelihood_; }

    /** @brief  Returns the optical flow likelihood. */
    const Optical_flow_likelihood& of_likelihood() const
    {
        return of_likelihood_;
    }

    /** @brief  Returns the face flow likelihood. */
    const Face_flow_likelihood& ff_likelihood() const { return ff_likelihood_; }

    /** @brief  Returns the color likelihood. */
    const Color_likelihood& color_likelihood() const
    {
        return color_likelihood_;
    }

//private:
    /** @brief  Computes the dimension prior of a scene. */
    double dimension_prior(const Scene& scene) const;

private:
    // distributions
    const Box_likelihood& box_likelihood_;
    const Facemark_likelihood& fm_likelihood_;
    const Optical_flow_likelihood& of_likelihood_;
    const Face_flow_likelihood& ff_likelihood_;
    const Color_likelihood& color_likelihood_;
    const Position_prior& pos_prior_;
    const Direction_prior& dir_prior_;
    const Face_direction_prior& fdir_prior_;
    Normal_distribution N_h;
    Normal_distribution N_w;
    Normal_distribution N_g;

    // use distributions?
    mutable bool use_box_lh_;
    mutable bool use_fm_lh_;
    mutable bool use_of_lh_;
    mutable bool use_ff_lh_;
    mutable bool use_color_lh_;
    mutable bool use_pos_prior_;
    mutable bool use_dir_prior_;
    mutable bool use_fdir_prior_;
    mutable bool use_dim_prior_;
    bool m_vis_off;
    bool m_infer_head;
};

/**
 * @brief   Posterior adapter to work with independent gradient computation.
 */
class Scene_posterior_ind
{
public:
    Scene_posterior_ind(const Scene_posterior& post) :
        posterior_(post), adapter_(post.vis_off(), post.infer_head()) {}

    double operator()(const Scene& sc, size_t i) const
    {
        const Target* tg_p;
        size_t frame;
        boost::tie(tg_p, frame) = adapter_.target_frame(&sc, i);

        IFT(tg_p != 0, Runtime_error, "Cannot compute ind. post.: bad index.");

        return posterior_.local(*tg_p, sc, frame);
    }

    double operator()(const Scene& sc, size_t i, size_t j) const
    {
        const Target* tg_p1;
        const Target* tg_p2;
        size_t frame1;
        size_t frame2;
        boost::tie(tg_p1, frame1) = adapter_.target_frame(&sc, i);
        boost::tie(tg_p2, frame2) = adapter_.target_frame(&sc, j);

        IFT(tg_p1 != 0, Runtime_error, "Cannot compute ind. post.: bad index.");
        IFT(tg_p2 != 0, Runtime_error, "Cannot compute ind. post.: bad index.");

        return posterior_.local(*tg_p1, *tg_p2, sc, frame1, frame2);
    }

    const Scene_adapter& adapter() const { return adapter_; }

    void reset() const { adapter_.reset(); }

private:
    const Scene_posterior& posterior_;
    Scene_adapter adapter_;
};

}} // namespace kjb::pt

#endif /*PT_SCENE_POSTERIOR_H */

