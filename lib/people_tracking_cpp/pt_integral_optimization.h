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

/* $Id: pt_integral_optimization.h 20911 2016-10-30 17:50:20Z ernesto $ */

#ifndef PT_INTEGRAL_OPTIMIZATION_H
#define PT_INTEGRAL_OPTIMIZATION_H

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_scene_adapter.h>
#include <people_tracking_cpp/pt_sample_scenes.h>
#include <people_tracking_cpp/pt_scene_posterior.h>
#include <people_tracking_cpp/pt_scene_diff.h>
#include <string>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/ref.hpp>

#ifdef KJB_HAVE_ERGO
#include <ergo/hmc.h>
#include <ergo/mh.h>
#include <ergo/record.h>
#endif

namespace kjb {
namespace pt {

/**
 * @class   Optimize_likelihood
 * @brief   Approximate complete-data likelihood via HMC optimziation.
 *
 * This class approximates the complete-data likelihood using an
 * optimization technique. For more details, see XXX.
 */
class Optimize_likelihood
{
public:
    Optimize_likelihood
    (
        const Scene_posterior& posterior, 
        bool infer_head_off = false
    ) :
        sample_scenes_(posterior, !infer_head_off),
        hess_step_size_(0.01),
        estimate_hess_step_size_(true),
        record_log_(false),
        record_samples_(false),
        record_proposals_(false),
        record_info_(false),
        infer_head_(!infer_head_off),
        hess_nthreads_(1)
    {}

    /** @brief  Have HMC log its steps to the given file. */
    void set_output_directory(const std::string& output_dir)
    {
        out_dir_ = output_dir;
    }

    /** @brief  Have HMC log its steps to the given file. */
    void set_hess_step_size(double hss)
    {
        hess_step_size_ = hss;
        estimate_hess_step_size_ = false;
    }

    /** @brief  Turn recording of logs on/off. */
    void record_log(bool rl) { record_log_ = rl; }

    /** @brief  Turn recording of samples on/off. */
    void record_samples(bool rs) { record_samples_ = rs; }

    /** @brief  Turn recording of proposals on/off. */
    void record_proposals(bool rp) { record_proposals_ = rp; }

    /** @brief  Turn recording scene info on/off. */
    void record_scene_info(bool ri) { record_info_ = ri; }

    /** @brief  Return the internal sampler used to compute likelihood. */
    const Sample_scenes& sampler() const { return sample_scenes_; }

    /**
     * @brief   Return (by ref) the internal sampler used to compute likelihood.
     */
    Sample_scenes& sampler() { return sample_scenes_; }

    /** @brief  Optimize likelihood. */
    double operator()(const Scene& scene) const;

    /** @brief  Set the number of threads in gradient computation of
     *          sample_scene. 
     */
    void set_grad_num_threads(size_t nthreads) 
    {
        sample_scenes_.set_num_threads(nthreads);
    }

    /** @brief  Set the number of threads in hessian computation. */
    void set_hess_num_threads(size_t nthreads) { hess_nthreads_ = nthreads; }

private:
    Sample_scenes sample_scenes_;
    double hess_step_size_;
    bool estimate_hess_step_size_;
    std::string out_dir_;
    bool record_log_;
    bool record_samples_;
    bool record_proposals_;
    bool record_info_;
    bool infer_head_;
    size_t hess_nthreads_;
};

}} // namespace kjb::pt

#endif /*PT_INTEGRAL_OPTIMIZATION_H */

