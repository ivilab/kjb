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
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <prob_cpp/prob_pdf.h>

#include "dbn_cpp/util.h"
#include "dbn_cpp/lss_set.h"
#include "dbn_cpp/proposer.h"
#include "dbn_cpp/posterior.h"
#include "dbn_cpp/prior.h"
#include "dbn_cpp/gradient.h"
#include "dbn_cpp/time_util.h"

#include <boost/format.hpp>

#ifdef KJB_HAVE_ERGO
#include <ergo/hmc.h>
#include <ergo/mh.h>
#else
#error "You need libergo to use this program."
#endif

using namespace kjb;
using namespace kjb::ties;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::mh_proposal_result Init_state_proposer::operator()
(
    const Linear_state_space& in,
    Linear_state_space& out
) const
{
    std::string move_name("-init-state-");
    out = in;
    State_type& init_state = out.init_state();
    size_t num_states = init_state.size();

    size_t d = sample_index_;
    init_state[d] += sample(P_dist_);
    out.changed_index() = 0;

    sample_index_++;
    if(sample_index_ == num_states) 
    {
        sample_index_ = 0;
    }

    // symetric proposal
    return ergo::mh_proposal_result(0.0, 0.0, move_name);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::mh_proposal_result Lss_polynomial_mh_proposer::operator()
(
    const Linear_state_space& in,
    Linear_state_space& out
) const
{
    std::string move_name("-polynomial-");
    out = in;
    State_type& init_state = out.init_state();
    int D = out.polynomial_degree() + 1;
    int index = sample_index_ / D;
    int iindex = sample_index_ % D;

    double val = sample(P_dist_);
    double old_val = out.polynomial_coefs()[index][iindex];
    out.set_polynomial_coefs(index, iindex, old_val + val);

    sample_index_++;
    if(sample_index_ == out.num_polynomial_coefs()) 
    {
        sample_index_ = 0;
    }

    // symetric proposal
    return ergo::mh_proposal_result(0.0, 0., move_name);

};


ergo::mh_proposal_result Lss_mh_proposer::operator()
(
    const Linear_state_space& in,
    Linear_state_space& out
) const
{
    long long start, elapsed; 
    struct timespec begin, finish;

    std::string move_name("lss-");
    out = in;
    State_type& init_state = out.init_state();
    Coupled_oscillator_v& clos = out.coupled_oscillators();
    assert(!clos.empty());
    size_t num_states = in.ignore_clo() ? 0 : init_state.size();
    size_t num_params = sample_clo_params_ ? clos[0].num_params() : 0;
    size_t num_clos = clos.size();
    if(!out.allow_drift())
    {
        num_clos = 1;
    }
    size_t num_clo_params = in.ignore_clo() ? 0 : num_params * num_clos;
    size_t num_poly_params = sample_poly_terms_ ?  out.num_polynomial_coefs() : 0;
    size_t num_total_params = num_clo_params + num_poly_params;
    size_t num_oscillators = in.num_oscillators();

    if(!sample_init_state_) num_states = 0;

    if(sample_index_ == num_total_params + num_states) 
    {
        sample_index_ = 0;
    }
    size_t d = sample_index_;
    double forward_p = 0.0;
    double x;
    double x_star;

    const double NANOS = 1e9;
    if(num_poly_params > 0 && d < num_poly_params)
    {
        int D = out.polynomial_degree() + 1;
        int index = d / D;
        int iindex = d % D;
        Gaussian_distribution P(0.0, prop_sigmas_[d]);
        double value = sample(P);
        x = out.polynomial_coefs()[index][iindex];
        x_star = x + value;
        out.set_polynomial_coefs(index, iindex, x_star);
        move_name += "polynomial-coefs";
    }
    else if(d < num_total_params)
    {
        //get_current_time(&begin);
        start = begin.tv_sec * NANOS + begin.tv_nsec;
        int c_d = d - num_poly_params;
        size_t c_i = c_d / num_params;
        size_t p_i = c_d - c_i * num_params;
        assert(p_i < num_params && c_i < num_clos);
        try
        {
            // just propose, check whether the proposed sample is valid 
            // or not when evaluating the sample
            do 
            {
                Gaussian_distribution P_clo(0.0, prop_sigmas_[d]);
                double value = sample(P_clo);
                x = clos[c_i].get_param(p_i);
                x_star = x + value;
                /*else
                {
                    // for stiffness, propose in the log space
                    x_star = std::exp(std::log(x) + value);
                }*/
                if(!out.allow_drift())
                {
                    assert(c_i == 0);
                    // frequency 
                    BOOST_FOREACH(Coupled_oscillator& clo, clos)
                    {
                        clo.set_param(p_i, x_star);
                    }
                    out.changed_index() = 0;
                }
                else
                {
                    clos[c_i].set_param(p_i, x_star);
                    out.changed_index() = c_i;
                }
            } while(!out.has_valid_params());

        }
        catch(Exception& e)
        {
            std::cerr << e.get_msg() << " kjb error\n";
            out = in;
        }
        catch(...)
        {
            std::cerr << "Unknow error\n";
        }
        move_name += "clo-params";
    }
    else if(sample_init_state_ && num_states > 0)
    {
        int c_d = d - num_total_params;
        Gaussian_distribution P_init(0.0, prop_sigmas_[d]);
        double value = sample(P_init);
        x = init_state[c_d];
        x_star = x + value; 
        init_state[c_d] = x_star;
        out.changed_index() = 0;
        move_name += "init-state";
    }

    sample_index_++;
    // symmetric proposal
    return ergo::mh_proposal_result(0.0, 0.0, move_name);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::mh_proposal_result Lss_set_mh_proposer::operator()
(
    const Lss_set& in,
    Lss_set& out
) const
{
    std::string move_name("lss-set-");
    out = in;

    double rev_p = 0.0;
    double forward_p =  0.0;

    Gaussian_distribution Q_coef(0.0, bmi_coef_sigma_);
    Gaussian_distribution Q_var(0.0, var_sigma_);

    std::vector<Vector>& pred_coefs = out.pred_coefs(group_id_);
    std::vector<double>& vars = out.variances(group_id_);
 
    size_t pred_size = out.pred_coef_size(group_id_);
    size_t nv = in.ignore_clo() ? 0 : vars.size();
    size_t num_total_params = pred_size;
    if(!in.fixed_clo())
    {
        num_total_params += nv;
    }

    size_t d = sample_index_;
    std::string spec_move = "";
    boost::format mod_name_fmt(std::string("moderator-%d"));
    boost::format sig_name_fmt(std::string("sigma-%d"));
    if(d < pred_size)
    {
        size_t pred_per = pred_coefs[0].size();
        size_t index_1 = d / pred_per;
        size_t index_2 = d % pred_per; 
        KJB(ASSERT(index_1 < pred_coefs.size()));
        KJB(ASSERT(index_2 < pred_coefs[index_1].size()));
        pred_coefs[index_1][index_2] += sample(Q_coef);
        spec_move = (move_name + (mod_name_fmt % d).str());
        out.update_means();
    }
    else if(d < nv + pred_size)
    {
        d -= pred_size;
        if(d > 0)
        {
            // transform the sigma 
            assert(d < vars.size());
            vars[d] += sample(Q_var);
        }
        if(!in.fixed_clo())
        {
            out.update_variances();
        }
        spec_move = (move_name + (sig_name_fmt % d).str());
    }

    if(sample_index_ == num_total_params - 1) 
    {
        sample_index_ = 0;
        group_id_++;
    }
    else
    {
        sample_index_++;
    }
    // reset the group id to be the first one 
    if(group_id_ == in.num_groups() - 1)
    {
        group_id_ = 0;
    }

    return ergo::mh_proposal_result(forward_p, rev_p, spec_move);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set_mean_variance_step::operator()
(  
    Lss_set& lsss, 
    double& lp
) const
{
    // COMPUTE OLD PRIOR AND UPDATE POSTERIOR
    double olpr = eval_(lsss);
    lp -= olpr;

    // UPDATE THE MEAN AND VARIANCE OF EACH CLUSTER
    std::vector<std::vector<bool> > res = eval_.prior().compute_posterior(lsss);
    assert(res.size() == lsss.num_groups());
    for(size_t g = 0; g < res.size(); g++)
    {
        // SAMPLE coefficients of the bayesian linear model for the current
        // cluster
        std::vector<Vector>& pred_coefs = lsss.pred_coefs(g);
        std::vector<double>& variances = lsss.variances(g);
        assert(pred_coefs.size() == variances.size());
        
        for(size_t i = 0; i < res[g].size(); i++)
        {
            // If we did not get valid samples, keep the original ones
            if(!res[g][i]) 
            {
                std::cout << "KEEPING OLD VAL FOR " << i << std::endl;
                continue;
            }
           
            const size_t num_samples = 1;
            for(size_t j = 0; j < num_samples; j++)
            {
                std::pair<Vector, double> res_sample = 
                    eval_.prior().blr_priors()[g][i].get_sample_from_posterior();
                // update the pred_coefs
                if(j == 0)
                {
                    pred_coefs[i] = res_sample.first; 
                    variances[i] = res_sample.second;
                }
                else
                {
                    pred_coefs[i] += res_sample.first; 
                    variances[i] += res_sample.second;
                }
            }

            pred_coefs[i] /= num_samples; 
            variances[i] /= num_samples;
        }
    } 

    // Update the design matrix when the parameters are allowed to drift
    if(lsss.allow_drift())
    {
        lsss.update_covariance_matrix();
    }

    // UPDATE THE NEW PRIOR
    lsss.update_means();
    lsss.update_variances();
    double nlpr = eval_(lsss);

    // UPDATE POSTERIOR
    lp += nlpr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_set_noise_sigma_step::operator()(Lss_set& lsss, double& lp) const
{
    using namespace std;
    const vector<Linear_state_space>& lss_vec = lsss.lss_vec();

    // COMPUTE OLD PRIOR AND UPDATE POSTERIOR
    double olpr = prior_(lsss);
    for(size_t i = 0; i < lss_vec.size(); i++)
    {
        olpr += likelihoods_[i](lss_vec[i]);
    }
    lp -= olpr;
    
    // SAMPLE NEW PARAMETERS
    // update the variance
    size_t num_lss = lss_vec.size();

    const vector<Inverse_gamma_distribution>& ps = prior_.get_dists();

    size_t N = 0;
    for(size_t i = 0; i < num_lss; i++)
    {
        N += lss_vec[i].get_times().size();
    }

    // Update each sigma
    Vector squared_errors((int)ps.size(), 0.0);
    for(size_t i = 0; i < lss_vec.size(); i++)
    {
        squared_errors += likelihoods_[i].get_squared_errors(lss_vec[i]);
    }

    Vector noise_sigmas(lsss.get_noise_sigmas());

    for(size_t i = 0; i < ps.size(); i++)
    {
        double shape = ps[i].shape() + N/2.0;
        double scale = ps[i].scale() + squared_errors[i]/2.0;
        if(scale/shape > 1e8) continue;

        Inverse_gamma_distribution P(shape, scale);
        noise_sigmas[i] = std::sqrt(sample(P));
    }
    lsss.set_noise_sigmas(noise_sigmas);

    // COMPUTE NEW PRIOR
    double nlpr = prior_(lsss);
    for(size_t i = 0; i < lss_vec.size(); i++)
    {
        nlpr += likelihoods_[i](lss_vec[i]);
    }

    // UPDATE POSTERIOR
    lp += nlpr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::mh_proposal_result Lss_set_noise_sigma_proposer::operator()
(
    const Lss_set& in, 
    Lss_set& out
) const 
{
    using namespace std;
    Vector x = in.get_noise_sigmas();
    Vector x_star = x; 
    for(size_t i = 0; i < x.size(); i++)
    {
        Gaussian_distribution P(0.0, prop_sigmas_[i]);
        double val = sample(P);
        // sample in log space 
        x_star[i] = std::exp(std::log(x[i]) + val);
    }
    out.set_noise_sigmas(x_star);
    std::string move_name("lss-set-noise-sigma-mh");
    return ergo::mh_proposal_result(0.0, 0.0, move_name);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

ergo::mh_proposal_result Lss_set_obs_coef_proposer::operator()
(
    const Lss_set& in,
    Lss_set& out
) const
{
    std::string move_name("-lss-set-obs-coef-");
    out = in;

    const Vector& obs_coefs = out.obs_coefs()[obs_index_][osc_index_];
    size_t num_osc = out.lss_vec().front().num_oscillators();
    size_t num_obs = out.lss_vec().front().obs_names().size();
    Vector new_val = obs_coefs + sample(p_dist_);
    out.set_obs_coef(obs_index_, osc_index_, new_val);
    osc_index_++ ;
    if(osc_index_ == num_osc)
    {
        obs_index_++;
        osc_index_ = 0;
    }
    if(obs_index_ == num_obs)
    {
        obs_index_ = 1;
    }

    // symetric proposal
    return ergo::mh_proposal_result(0.0, 0.0, move_name);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Cluster_step::operator()(Lss_set& lsss, double& lp) const
{
    double olpr = cluster_prior_(lsss) + blr_pos_(lsss);
    lp -= olpr;
    // UPDATE THE CLUSTER WEIGHTS 
    sample_cluster_weights(lsss);
    // UPDATE THE CLUSTER ASSIGNMENT
    sample_cluster_assignments(lsss);
    // update the design matrix since cluster's changed
    // TODO (in the future, we can update the design matrix while updating 
    // the assignment; however, due to the change in both design matrix 
    // and the parmeters, this is not easily done)
    // eval_.update_design_matrix(lsss);
    // update the priors of each lss based on the new cluster assignment

    double nlpr = cluster_prior_(lsss) + blr_pos_(lsss);
    lp += nlpr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_cluster_step::update_lss_prior
(
    Linear_state_space& lss, 
    const Group_params& param
) const
{
    // clo
    const std::vector<Vector>& preds = lss.get_predictors();
    if(!lss.ignore_clo())
    {
        assert(lss.num_clo_params() !=0 );
        for(size_t i = 0; i < lss.num_clo_params(); i++)
        {
            double val = dot(param.pred_coefs[i], preds[i]);
            lss.set_clo_mean(i, val);
            lss.set_clo_variance(i, param.variances[i]);
        }
    }
    // polynomial
    size_t cur_index = lss.num_clo_params();
    for(size_t i = cur_index; i < cur_index + lss.num_polynomial_coefs(); i++)
    {
        double val = dot(param.pred_coefs[i], preds[i]);
        size_t ii = i - cur_index;
        size_t osc_index = ii / lss.polynomial_dim_per_osc();
        size_t poly_index = ii % lss.polynomial_dim_per_osc();
        lss.set_polynomial_coefs_mean(osc_index, poly_index, val);
        lss.set_polynomial_coefs_mean(osc_index, poly_index, param.variances[i]);
    }
    // outcome
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_cluster_step::operator()(Linear_state_space& lss, double& lp) const
{
    size_t old_cluster = lss.group_index();
    double old_prior = std::log(group_params_[old_cluster].group_weight);
    double temp = old_prior;
    old_prior += lss.get_clo_prior();  

    // TODO pass exclude outcome in argument 
    std::vector<double> res = 
        get_responsibilities(group_params_, lss, exclude_outcome_);
    lss.update_group_responsibilities(res);
    Categorical_distribution<double> p_dist(res, 0);
    size_t new_cluster = sample(p_dist);
    lss.group_index() = new_cluster;
    // update the lss prior
    if(new_cluster != old_cluster)
    {
        update_lss_prior(lss, group_params_[new_cluster]);
        lp -= old_prior;
        double new_prior = std::log(group_params_[new_cluster].group_weight);
        temp = new_prior;
        new_prior += lss.get_clo_prior();
        lp += new_prior;
    }
    
    // new prior
}

