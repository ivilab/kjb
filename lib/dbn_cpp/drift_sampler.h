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
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: drift_sampler.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_DRIFT_SAMPLER_H
#define KJB_TIES_DRIFT_SAMPLER_H

#include <l/l_sys_debug.h>
#include <l/l_sys_def.h>
#include <n/n_cholesky.h>
#include <l_cpp/l_exception.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_sample.h>
#include <gp_cpp/gp_predictive.h>
#include <gp_cpp/gp_posterior.h>

#include <ergo/mh.h>
#include <ergo/record.h>

#include <string>
#include <iterator>
#include <fstream> 

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/prior.h"
#include "dbn_cpp/data.h"
#include "dbn_cpp/posterior.h"

namespace kjb {
namespace ties {

class Drift_sampler
{
private:
    typedef gp::Predictive_nl<gp::Constant, gp::Squared_exponential> Pred;
    typedef std::vector<Pred> Pred_vec;

    struct Model
    {
        Linear_state_space outputs;
        std::vector<Vector> c_outputs;
        Model
        (
            const Linear_state_space& out,
            const std::vector<Vector>& c_out
        ) : 
            outputs(out),
            c_outputs(c_out)
        {}
    };

    const static size_t DEFAULT_NUM_BURN_ITERATIONS = 1000;

public: 
    Drift_sampler
    (
        const Posterior& posterior,
        const Linear_state_space& init_lss,
        size_t ctr_pt_length = 6,
        size_t num_burn_its = DEFAULT_NUM_BURN_ITERATIONS,
        boost::optional<const std::string&> log_fname_p = boost::none,
        boost::optional<const std::string&> bst_fname_p = boost::none
    ) :
        posterior_(posterior),
        num_control_pts_(std::max(3, 
                    (int)init_lss.get_times().size()/(int)ctr_pt_length)),
        c_inputs_(
            gp::uniform_control_inputs(
                        init_lss.get_gp_inputs().begin(),
                        init_lss.get_gp_inputs().end(),
                        num_control_pts_)),
        model_(init_lss, init_lss.get_gp_outputs(c_inputs_)),
        best_model_(init_lss, init_lss.get_gp_outputs(c_inputs_)),
        pred_(make_predictive(init_lss)),
        c_preds_(make_control_predictives(init_lss)),
        step_(
            boost::bind(&Drift_sampler::log_posterior, this, _1),
            boost::bind(&Drift_sampler::propose, this, _1, _2)),
        num_burn_its_(num_burn_its),
        burned_in_(false),
        sample_index_(0),
        record_(false)
    {
        if(log_fname_p && bst_fname_p)
        {
            log_ofs_.open(log_fname_p->c_str(), std::ofstream::app);
            bst_ofs_.open(bst_fname_p->c_str(), std::ofstream::app);
            if(log_ofs_.fail())
            {
                KJB_THROW_3(IO_error, "Can't open file %s", 
                                      (log_fname_p->c_str()));
            }
            if(bst_ofs_.fail())
            {
                KJB_THROW_3(IO_error, "Can't open file %s", 
                                      (bst_fname_p->c_str()));
            }
            record_ = true;

            // add the MH recorder
            step_.add_recorder(ergo::make_mh_detail_recorder(
                 std::ostream_iterator<ergo::step_detail>(log_ofs_, "\n")));

            // add the best target recorder
            ergo::best_target_recorder<std::ostream_iterator<double> > 
                best_target_recorder(std::ostream_iterator<double>(bst_ofs_, "\n"));
            step_.add_recorder(best_target_recorder);

            // add the best sample recorder
            step_.add_recorder(
                    ergo::make_best_sample_recorder(&best_model_).replace());
        }

        // set cholesky decomposition method, default is "native"
        KJB(EPETE(set_cholesky_options("cholesky", "lapack")));
    }

    /** @brief  Sample until burned in. */
    void burn_in() const
    {
        if(burned_in_) return;

        log_target_ = log_posterior(model_);
        best_log_target_ = log_target_;
        for(size_t i = 0; i < num_burn_its_; i++)
        {
            step_(model_, log_target_);
            if(log_target_ > best_log_target_)
            {
                best_log_target_ = log_target_;
            }
            if(record_)
            {
                log_ofs_.flush();
                bst_ofs_.flush();
            }
        }

        burned_in_ = true;
    }

    /** @brief  Generate a sample from the posterior distribution. */
    void sample() const
    {
        //burn_in();
        log_target_ = log_posterior(model_);
        best_log_target_ = log_target_;

        step_(model_, log_target_);
        if(log_target_ > best_log_target_)
        {
            best_log_target_ = log_target_;
        }
        if(record_)
        {
            log_ofs_.flush();
            bst_ofs_.flush();
        }
    }

    /** @brief  Get current sample. */
    Linear_state_space& current() const { return model_.outputs; }

    /** @brief  Get best sample. */
    const Linear_state_space& best() const { return best_model_.outputs; }

    double best_log_target() const { return best_log_target_; }
    double log_target() const { return log_target_; }
    size_t num_control_pts() const { return num_control_pts_; }

    size_t get_sample_index() const { return sample_index_; }
    void set_sample_index(size_t s ) const { sample_index_ = s; }

private:
    /**
     * @brief   Create a model using given contol inputs; outputs are
     *          set to the mean.
     */
    Pred_vec make_predictive(const Linear_state_space& lss) const;

    /**
     * @brief   Create a model using given contol inputs; outputs are
     *          set to the mean.
     */
    std::vector<Pred_vec> make_control_predictives
    (
        const Linear_state_space& lss
    ) const;

    /** @brief  Posterior for this sampler. */
    double log_posterior(const Model& m) const
    {
        //return posterior_.likelihood()(m.outputs);
        return posterior_(m.outputs);
    }

    /** @brief  Propose a new model. */
    ergo::mh_proposal_result propose(const Model& m, Model& m_p) const;

    /** @brief  Sets the Linear_state_space params. */
    void set_params
    (
        const Vector& params, 
        size_t index, 
        Linear_state_space& lss
    ) const;

    // data members
    const Posterior& posterior_;
    size_t num_control_pts_;
    gp::Inputs c_inputs_;
    mutable Model model_;
    mutable Model best_model_;
    mutable double log_target_;
    mutable double best_log_target_;
    mutable Pred_vec pred_;
    mutable std::vector<Pred_vec> c_preds_;
    ergo::mh_step<Model> step_;
    size_t num_burn_its_;
    mutable bool burned_in_;
    mutable size_t sample_index_;
    mutable std::ofstream log_ofs_;
    mutable std::ofstream bst_ofs_;
    bool record_;
};

}} // namespace kjb::ties

#endif // KJB_TIES_DRIFT_SAMPLER_H

