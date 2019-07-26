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
|     Ernesto Brau, Jinyan Guan
|
* =========================================================================== */

/* $Id$ */

#ifndef PT_SAMPLE_SCENES_H
#define PT_SAMPLE_SCENES_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_scene_info.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_recorder.h>
#include <prob_cpp/prob_distribution.h>
#include <gp_cpp/gp_posterior.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <l_cpp/l_exception.h>
#include <vector>
#include <algorithm>
#include <string>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#ifdef KJB_HAVE_ERGO
#include <ergo/hmc.h>
#include <ergo/mh.h>
#include <ergo/record.h>
#endif

namespace kjb {
namespace pt {

/** @brief  Helper fcn; returns true if the scene has only new targets. */
bool all_new_targets(const Scene& scene);

///** @brief  Get trajectory from target as kjb::Vector. */
//Vector get_trajectory(const Target& tg, size_t idx);
//
///** @brief  Get trajectory from target as kjb::Vector. */
//void set_trajectory(Target& tg, size_t idx, const Vector& tj);
//
///**
// * @class   Propose_trajectory
// * @brief   Proposal distribution/mechanism for trajectory of targets.
// */
//template<class Mean, class Covariance>
//class Propose_trajectory
//{
//private:
//    typedef Cp_proposer<Mean, Covariance> Cp_prop;
//
//public:
//    Propose_trajectory(size_t num_cpts) :
//        nctrls_(num_cpts), cur_prop_(0), cur_gp_(0) {}
//
//#ifdef KJB_HAVE_ERGO
//    /** @brief  Propose scene. */
//    ergo::mh_proposal_result operator()(const Scene& in, Scene& out);
//#endif
//
//private:
//    void update_proposers(const Scene& sc);
//
//private:
//    size_t nctrls_;
//    std::vector<Cp_prop> proposers_;
//    size_t cur_prop_;
//    size_t cur_gp_;
//};
//
///* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//template<class M, class C>
//ergo::mh_proposal_result
//    Propose_trajector<M, C>::operator()(const Scene& in, Scene& out)
//{
//    update_proposers(in);
//
//    // get current trajectory
//    Ascn::const_iterator tg_p = in.association.begin();
//    std::advance(tg_p, cur_prop_);
//    Vector traj_in = get_trajectory(*tg_p, cur_gp_);
//    Vector traj_out;
//
//    // sample it
//    ergo::mh_proposal_result res = proposers_[cur_prop_](traj_in, traj_out);
//    res.name = "traj-" + std::to_string(cur_prop_)
//                + "-gp-" + std::to_string(cur_gp_)
//                + "-" + res.name;
//
//    // set trajectory back
//    out = in;
//    Ascn::iterator tg_q = out.association.begin();
//    std::advance(tg_q, cur_prop_);
//    set_trajectory(*tg_q, cur_gp_, traj_out);
//
//    return res;
//}
//
///* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
//
//template<class M, class C>
//void Propose_trajector<M, C>::update_proposers(const Scene& sc)
//{
//    // first time used
//    if(proposers_.empty())
//    {
//        proposers_.reserve(sc.association.size());
//        BOOST_FOREACH(const Target& tg, sc.association)
//        {
//            proposers_.push_back(
//                Cp_prop(
//                    ins.begin(),
//                    ins.end(),
//                    nctrls_,
//                    tg.pos_prior().mean_function(),
//                    tg.pos_prior().covariance_function()));
//        }
//
//        cur_prop_ = 0;
//        cur_gp_ = 0;
//
//        return;
//    }
//
//    // otherwise...
//    Ascn::const_iterator tg_p = in.association.begin();
//    std::advance(tg_p, cur_prop_);
//    const size_t cursz = tg_p->get_end_time() - tg_p->get_start_time() + 1;
//    if(proposers_[cur_prop_].current_control() == cursz)
//    {
//        ++cur_gp_;
//        if(cur_gp_ == 5)
//        {
//            ++cur_prop_;
//            cur_gp_ = 0;
//
//            if(cur_prop_ == proposers_.size())
//            {
//                cur_prop_ = 0;
//            }
//        }
//    }
//}
//
/**
 * @class   Propose_person_size
 * @brief   Proposal distribution/mechanism for the size of targets.
 */
class Propose_person_size
{
public:
    Propose_person_size
    (
        double hsdv,
        double wsdv,
        double gsdv,
        bool infer_head = true
    )
        : N_height(0.0, hsdv),
          N_width(0.0, wsdv),
          N_girth(0.0, gsdv),
          m_infer_head(infer_head)
    {}

#ifdef KJB_HAVE_ERGO
    ergo::mh_proposal_result operator()(const Scene& in, Scene& out);
#endif

private:
    Normal_distribution N_height;
    Normal_distribution N_width;
    Normal_distribution N_girth;
    bool m_infer_head;
};

/**
 * @class   Propose_point_location
 * @brief   Proposal distribution/mechanism for the position of interesting points.
 */
class Propose_point_location
{
public:
    Propose_point_location (double sdv) :
        N_x(0.0, sdv), N_y(0.0, sdv), N_z(0.0, sdv) {}

    Propose_point_location (double sdv_x, double sdv_y, double sdv_z) :
        N_x(0.0, sdv_x), N_y(0.0, sdv_y), N_z(0.0, sdv_z) {}

#ifdef KJB_HAVE_ERGO
    ergo::mh_proposal_result operator()(const Scene& in, Scene& out);
#endif

private:
    Normal_distribution N_x;
    Normal_distribution N_y;
    Normal_distribution N_z;
};

/**
 * @class   Sample_scenes
 * @brief   Use HMC to draw samples from the scene posterior using HMC.
 */
class Sample_scenes
{
public:
    enum Sample_method { HMC, MH };
    typedef ergo::hmc_step<Scene> Hmc_step;
    typedef ergo::mh_step<Scene> Mh_step;

public:
    Sample_scenes(const Scene_posterior& posterior, bool infer_head = true) :
        posterior_p_(&posterior),
        sample_method_(HMC),
        num_iterations_(10),
        num_leapfrog_steps_(5),
        pos_opt_step_size_(0.00001),
        pos_grad_step_size_(0.01),
        estimate_grad_step_size_(true),
        height_sdv_(0.02),
        width_sdv_(0.01),
        girth_sdv_(0.01),
        pos_sdv_(0.1),
        infer_head_(infer_head),
        adapt_num_iters_(false),
        nthreads_(1)
    {}

public:
    /** @brief  Get posterior distribution. */
    const Scene_posterior& scene_posterior() const { return *posterior_p_; }

    /** @brief  Set posterior distribution. */
    void set_scene_posterior(const Scene_posterior& post)
    {
        posterior_p_ = &post;
    }

    /** @brief  Set the sampling method. */
    void set_sample_method(Sample_method method) { sample_method_ = method; }

    /**
     * @brief   Set the number of iterations and the fraction of HMC
     *          steps (vis-a-vis MH steps).
     */
    void set_num_iterations(size_t num_its) { num_iterations_ = num_its; }

    /** @brief  Set the number of leapfrog steps. */
    void set_num_leapfrog_steps(size_t lf_steps)
    {
        num_leapfrog_steps_ = lf_steps;
    }

    /** @brief  Set the HMC step sizes. */
    void set_hmc_step_size(double pss) { pos_opt_step_size_ = pss; }

    /** @brief  Set the gradient approximation step sizes. */
    void set_gradient_step_size(double pss)
    {
        pos_grad_step_size_ = pss;
        estimate_grad_step_size_ = false;
    }

    /** @brief  Set the MH proposal sigmas. */
    void set_size_proposal_sigmas(double hsdv, double wsdv, double gsdv)
    {
        height_sdv_ = hsdv;
        width_sdv_ = wsdv;
        girth_sdv_ = gsdv;
    }

    /** @brief  Set the adapt_num_iters_ */
    void set_adapt_num_iters(bool adapt) { adapt_num_iters_ = adapt; }

    /** @brief  Set the number of threads in gradient computation. */
    void set_num_threads(size_t nthreads) { nthreads_ = nthreads; }

    /** @brief  Sample from the full likelihood. */
    template<
        class StepIterator,
        class SampleIterator,
        class ProposalIterator,
        class InfoIterator>
    void operator()
    (
        const Scene& initial_scene,
        boost::optional<StepIterator> step_out,
        boost::optional<SampleIterator> sample_out,
        boost::optional<ProposalIterator> proposed_out,
        boost::optional<InfoIterator> pd_out
    ) const;

    /** @brief  Sample from the full likelihood. */
    void operator()(const Scene& initial_scene) const
    {
#ifdef KJB_HAVE_ERGO
        using boost::make_optional;
        this->operator()(
            initial_scene,
            make_optional(false, std::vector<ergo::step_detail>::iterator()),
            make_optional(false, Write_scene_iterator("")),
            make_optional(false, Write_scene_iterator("")),
            make_optional(false, std::vector<Scene_info>::iterator()));
#else
    KJB_THROW_2(Missing_dependency, "libergo");
#endif
    }

private:
#ifdef KJB_HAVE_ERGO
    /** @brief  Helper function that creates an HMC trajectory step. */
    Hmc_step make_hmc_traj_step
    (
        const Scene_adapter& scene_adapter,
        Scene& best_scene
    ) const;

    /** @brief  Helper function that creates an MH trajectory step. */
    Mh_step make_mh_traj_step() const;

    /** @brief  Helper function that creates an MH size step. */
    Mh_step make_mh_size_step(bool infer_head) const;

    /** @brief  Helper function that creates an MH size step. */
    Mh_step make_mh_pos_step() const;

    /** @brief  Helper function that runs the trajectory and size steps. */
    template<class TrajStep, class TRecIter, class SRecIter>
    void run_steps
    (
        Scene& scene,
        double& lt,
        const TrajStep& traj_step,
        const Mh_step& pos_step,
        const Mh_step& size_step,
        TRecIter tfirst,
        TRecIter tlast,
        SRecIter sfirst,
        SRecIter slast
    ) const;
#endif

private:
    const Scene_posterior* posterior_p_;
    Sample_method sample_method_;
    size_t num_iterations_;
    size_t num_leapfrog_steps_;
    double pos_opt_step_size_;
    double pos_grad_step_size_;
    bool estimate_grad_step_size_;
    double height_sdv_;
    double width_sdv_;
    double girth_sdv_;
    double pos_sdv_;
    bool infer_head_;
    bool adapt_num_iters_;
    size_t nthreads_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<
    class StepIterator,
    class SampleIterator,
    class ProposalIterator,
    class InfoIterator>
void Sample_scenes::operator()
(
    const Scene& initial_scene,
    boost::optional<StepIterator> step_out,
    boost::optional<SampleIterator> sample_out,
    boost::optional<ProposalIterator> proposed_out,
    boost::optional<InfoIterator> info_out
) const
{
#ifdef KJB_HAVE_ERGO
    typedef std::vector<Hmc_step::record_t> Hmc_rec_vector;
    typedef std::vector<Mh_step::record_t> Mh_rec_vector;

    if(num_iterations_ == 0) return;

    // size of scene
    Scene_adapter scene_adapter(posterior_p_->vis_off(), infer_head_);
    size_t scene_size = scene_adapter.size(&initial_scene);

    if(scene_size == 0) return;

    // make current and best scene and posterior value
    Scene cur_scene = initial_scene;
    double cur_lt = (*posterior_p_)(cur_scene);
    Scene best_scene = initial_scene;

    // steps
    Hmc_step traj_step = make_hmc_traj_step(scene_adapter, best_scene);
    //Mh_step traj_step = make_mh_traj_step(best_scene);
    Mh_step size_step = make_mh_size_step(infer_head_);
    Mh_step pos_step = make_mh_pos_step();

    // recorders
    Hmc_rec_vector hrv;
    Mh_rec_vector mrv;
    hrv.reserve(4);
    mrv.reserve(4);

    // best scene
    hrv.push_back(ergo::make_best_sample_recorder(&best_scene).replace());
    mrv.push_back(ergo::make_best_sample_recorder(&best_scene).replace());

    // log recorder
    if(step_out)
    {
        hrv.push_back(ergo::make_hmc_detail_recorder(*step_out));
        mrv.push_back(ergo::make_mh_detail_recorder(*step_out));
    }

    // sample recorder
    if(sample_out)
    {
        hrv.push_back(ergo::make_sample_recorder(*sample_out));
        mrv.push_back(ergo::make_sample_recorder(*sample_out));
    }

    // proposal recorder
    if(proposed_out)
    {
        hrv.push_back(ergo::make_proposed_recorder(*proposed_out));
        mrv.push_back(ergo::make_proposed_recorder(*proposed_out));
        traj_step.store_proposed();
        size_step.store_proposed();
        pos_step.store_proposed();
    }

    // scene info recorder
    if(info_out)
    {
        hrv.push_back(make_si_recorder(*info_out, *posterior_p_));
        mrv.push_back(make_si_recorder(*info_out, *posterior_p_));
    }

    // run steps
    run_steps(
        cur_scene,
        cur_lt,
        traj_step,
        pos_step,
        size_step,
        hrv.begin(),
        hrv.end(),
        mrv.begin(),
        mrv.end());

    // swap in best trajectories
    Ascn::const_iterator tg_p = best_scene.association.begin();
    BOOST_FOREACH(const Target& itarget, initial_scene.association)
    {
        const Target& btarget = *tg_p++;

        using std::swap;
        swap(btarget.trajectory(), itarget.trajectory());
        swap(btarget.body_trajectory(), itarget.body_trajectory());
        swap(btarget.face_trajectory(), itarget.face_trajectory());
    }

#else
    KJB_THROW_2(Missing_dependency, "libergo");
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_ERGO

template<class TrajStep, class TRecIter, class SRecIter>
void Sample_scenes::run_steps
(
    Scene& scene,
    double& lt,
    const TrajStep& traj_step,
    const Mh_step& pos_step,
    const Mh_step& size_step,
    TRecIter tfirst,
    TRecIter tlast,
    SRecIter sfirst,
    SRecIter slast
) const
{
    bool rss = !all_new_targets(scene);

    size_t num_iters = num_iterations_;
    if(adapt_num_iters_)
    {
        int D = dims(scene, true);
        //num_iters = std::max(5, D / 5);
        num_iters = std::max(5, D / 3);
        num_iters = std::min(250, (int)num_iters);
    }

    double best_lt = lt;
    double prev_best_lt = lt;
    for(size_t i = 1; i <= num_iters; i++)
    {
        // run hmc step and record
        traj_step(scene, lt);
        for(TRecIter rec_p = tfirst; rec_p != tlast; ++rec_p)
        {
            (*rec_p)(traj_step, scene, lt);
        }

        // run position step and record
        if(!scene.objects.empty())
        {
            pos_step(scene, lt);
            for(SRecIter rec_p = sfirst; rec_p != slast; ++rec_p)
            {
                (*rec_p)(pos_step, scene, lt);
            }
        }

        // run mh step and record
        if(rss)
        {
            size_step(scene, lt);
            for(SRecIter rec_p = sfirst; rec_p != slast; ++rec_p)
            {
                (*rec_p)(size_step, scene, lt);
            }
        }
        if(lt > best_lt) best_lt = lt;

        if((i + 1) % 100 == 0)
        {
            if(best_lt > prev_best_lt)
                prev_best_lt = best_lt;
            else break;
        }
    }
}

#endif

}} // namespace kjb::pt

#endif /*PT_SAMPLE_SCENES_H */

