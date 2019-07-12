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

/* $Id: linear_state_space.cpp 22573 2019-06-09 23:34:58Z adarsh $ */

#include <l/l_sys_debug.h>
#include <l_cpp/l_filesystem.h>
#include <l_cpp/l_functors.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_util.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_predictive.h>
#include <set>

#include <utility>
#include <stdexcept>
#include <iosfwd>
#include <fstream>

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/coupled_oscillator.h"
#include "dbn_cpp/data.h"
#include "dbn_cpp/util.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

using namespace kjb;
using namespace kjb::ties;

Linear_state_space::Linear_state_space
(
    const Double_v& times, 
    const State_type& init_state,
    const Coupled_oscillator_v& clos,
    const std::vector<std::string>& obs_names,
    const Vector& noise_sigmas,
    int polynomial_degree,
    const std::vector<std::string>& outcome_names,
    const Mod_map& outcomes,
    int group_index,
    bool ignore_clo
) : 
    times_(times), 
    clo_states_(times.size()),
    clos_(clos),
    drift_(false),
    gp_initialized_(false),
    gp_changed_(clos.size(), false),
    obs_names_(obs_names),
    obs_noise_sigmas_(noise_sigmas),
    changed_index_(0),
    poly_coefs_dirty_(true),
    obs_coefs_dirty_(true),
    outcome_names_(outcome_names),
    group_index_(group_index),
    ignore_clo_(ignore_clo),
    num_oscillators_(clos.front().num_oscillators())
{
    clo_states_[0] = init_state;
    if(polynomial_degree >= 0)
    {
        int D = polynomial_degree + 1;
        assert(polynomial_coefs_.empty());
        polynomial_coefs_.resize(num_oscillators_, Vector(D, 0.0));
    }
    if(clos.empty())
    {
        num_oscillators_ = 0;
    }
    else
    {
        num_oscillators_ = clos.front().num_oscillators();
        obs_coefs_.resize(obs_names.size(), 
                   std::vector<Vector>(num_oscillators_, Vector(1, 1.0)));
    }

    size_t num_outcomes = outcome_names.size();
    if(num_outcomes > 0)
    {
        outcome_means_.resize(num_outcomes, Vector((int)num_oscillators_, 0.0));
        outcome_variances_.resize(num_outcomes, Vector((int)num_oscillators_, 10.0));
        outcomes_.resize(num_outcomes, Vector((int)num_oscillators_, 0.0));
        size_t type = 0;
        BOOST_FOREACH(const std::string& name, outcome_names)
        {
            Mod_map::const_iterator it = outcomes.find(name);
            if(it == outcomes.end()) continue;
            Double_v vals = it->second;
            for(size_t i = 0; i < vals.size(); i++)
            {
                outcomes_[type][i] = vals[i];
            }
            type++;
        }
    }
} 

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Linear_state_space::read
(
    const std::string& indir, 
    double start_time
)
{
    using namespace std;
    // parse the com_params
    string param_fname(indir + "/com_params.txt");
    ifstream param_ifs(param_fname.c_str());
    IFTD(param_ifs.is_open(), IO_error, "Can't open file %s\n", 
            (param_fname.c_str()));
    clos_.clear();
    string line;
    while(getline(param_ifs, line))
    {
        Double_v vals;
        istringstream com_param_istr(line);
        copy(istream_iterator<double>(com_param_istr), 
             istream_iterator<double>(), 
             back_inserter(vals));
        bool use_modal = true;
        if(vals.size() != 6)
        {
            use_modal = false;
        }
        if(!vals.empty()) 
        {
            clos_.push_back(Coupled_oscillator(vals, use_modal));
        }
    }
    if(!clos_.empty())
    {
        num_oscillators_ = clos_.front().num_oscillators();
    }
    else
    {
        num_oscillators_ = 0;
    }

    // parse in the states
    string state_fname(indir + "/states.txt");
    ifstream state_ifs(state_fname.c_str());
    IFTD(state_ifs.is_open(), IO_error, "Can't open file %s\n", 
            (state_fname.c_str()));
    clo_states_.clear();
    while(getline(state_ifs, line))
    {
        State_type state;
        istringstream istr(line);
        copy(istream_iterator<double>(istr), istream_iterator<double>(), 
             back_inserter(state));
        clo_states_.push_back(state);
    }

    // parse in the times
    times_.resize(clo_states_.size());
    for(size_t i = start_time; i < start_time + clo_states_.size(); i++)
    {
        times_[i - start_time] = i;
    }

    // if time and clo params have different size
    // change the size of time to have the same size as clo params
    // and update the clo_states_
    if(times_.size() != clos_.size() + 1)
    {
        changed_index_ = 0;
        times_.resize(clos_.size() + 1);
        for(size_t i = 0; i <= clos_.size(); i++)
        {
            times_[i] = i;
        }
        clo_states_.resize(clos_.size() + 1);
        update_clo_states();
    }
    changed_index_ = 0;

    // Read in the polynomial coefs 
    string poly_fname(indir + "/polynomial_coefs.txt");
    if(kjb_c::is_file(poly_fname.c_str()))
    {
        polynomial_coefs_.clear();
        ifstream poly_ifs(poly_fname.c_str());
        IFTD(poly_ifs.is_open(), IO_error, "Can't open file %s\n", 
                (poly_fname.c_str()));
        while(getline(poly_ifs, line))
        {
            istringstream istr(line);
            Vector coef;
            copy(istream_iterator<double>(istr), 
                 std::istream_iterator<double>(), 
                 back_inserter(coef));
            polynomial_coefs_.push_back(coef);
        }
        poly_coefs_dirty_ = true;
        obs_coefs_dirty_ = true;
        update_poly_states();
    }

    // Read in outcomes 
    string outcome_fname(indir + "/outcomes.txt");
    if(kjb_c::is_file(outcome_fname.c_str())) 
    {
        ifstream outcome_ifs(outcome_fname.c_str());
        IFTD(outcome_ifs.is_open(), IO_error, "Can't open file %s\n", 
                (outcome_fname.c_str()));
        // parse in the names 
        getline(outcome_ifs, line);
        outcome_names_.clear();
        istringstream istr(line);
        copy(istream_iterator<string>(istr), istream_iterator<string>(), 
        back_inserter(outcome_names_));

        // parse in the outcome values 
        outcomes_.clear();
        while(getline(outcome_ifs, line))
        {
            istringstream istr(line);
            Vector coef;
            copy(istream_iterator<double>(istr), 
                 std::istream_iterator<double>(), 
                 back_inserter(coef));
            outcomes_.push_back(coef);
        }
    }

    // parse in the observation names and coefs
    string obs_fname(indir + "/obs.txt");
    ifstream obs_ifs(obs_fname.c_str());
    IFTD(obs_ifs.is_open(), IO_error, "Can't open file %s\n", 
            (obs_fname.c_str()));

    // parse the names 
    getline(obs_ifs, line);
    obs_names_.clear();
    istringstream istr(line);
    copy(istream_iterator<string>(istr), istream_iterator<string>(), 
         back_inserter(obs_names_));

    // parse the coefs 
    size_t num_obs = obs_names_.size();
    std::vector<Vector> temp_coefs;
    // clear out the default values in obs_coefs_
    obs_coefs_.clear();
    while(getline(obs_ifs, line))
    {
        Vector coef;
        istringstream istr(line);
        copy(istream_iterator<double>(istr), istream_iterator<double>(), 
             back_inserter(coef));
        temp_coefs.push_back(coef);
        if(temp_coefs.size() % num_oscillators_ == 0)
        {
            obs_coefs_.push_back(temp_coefs);
            temp_coefs.clear();
        }
    }
    if(obs_coefs_.empty())
    {
        // initialize the obs_coefs;
        obs_coefs_.resize(obs_names_.size(), 
                          vector<Vector>(num_oscillators_, Vector(1, 1.0)));
        for(size_t i = 0; i < obs_coefs_.size(); i++)
        {
            for(size_t j = 0; j < obs_coefs_[i].size(); j++)
            {
                //obs_coefs_[i][j][1] = 0.0;
            }
        }
    }
    IFTD(obs_coefs_.size() == num_obs, IO_error, 
            "file %s does not contain valid observable coefs.", 
            (obs_fname.c_str()));
    obs_coefs_dirty_ = true;

    // parse in the gp params
    string gp_fname(indir + "/gp_params.txt");
    if(kjb_c::is_file(gp_fname.c_str()))
    {
        gp_scales_.clear();
        gp_sigvars_.clear();
        ifstream gp_ifs(gp_fname.c_str());
        IFTD(gp_ifs.is_open(), IO_error, "Can't open file %s\n", 
                (gp_fname.c_str()));

        // gp_scales_
        getline(gp_ifs, line);
        istringstream istr(line);
        copy(istream_iterator<double>(istr), std::istream_iterator<double>(), 
             back_inserter(gp_scales_));

        // gp_sigvars
        getline(gp_ifs, line);
        istringstream gp_istr(line);
        copy(istream_iterator<double>(gp_istr), std::istream_iterator<double>(),
                back_inserter(gp_sigvars_));

        // gp_means
        std::vector<double> means;
        if(getline(gp_ifs, line))
        {
            istringstream istr(line);
            copy(istream_iterator<double>(istr), std::istream_iterator<double>(),
                    back_inserter(means));
        }

        std::vector<gp::Constant> gp_means(gp_scales_.size(), gp::Constant(0));
        if(!means.empty())
        {
            assert(gp_means.size() == means.size());
            copy(means.begin(), means.end(), gp_means.begin());
        }

        init_gp(gp_scales_, gp_sigvars_, gp_means);
        drift_ = true;
        gp_changed_.resize(gp_scales_.size(), false);

    }
    else
    {
        drift_ = false;
    }

    // Read in the observation noise sigmas
    string obs_noise_fname(indir + "/obs_noise_sigmas.txt");
    if(kjb_c::is_file(obs_noise_fname.c_str()))
    {
        obs_noise_sigmas_.clear();
        ifstream obs_noise_ifs(obs_noise_fname.c_str());
        IFTD(obs_noise_ifs.is_open(), IO_error, "Can't open file %s\n", 
                (obs_noise_fname.c_str()));
        getline(obs_noise_ifs, line);
        istringstream istr(line);
        copy(istream_iterator<double>(istr), 
             std::istream_iterator<double>(), 
             back_inserter(obs_noise_sigmas_));
    }
    // group index
    string group_index_fp(indir + "/group_index.txt");
    if(kjb_c::is_file(group_index_fp.c_str()))
    {
        ifstream group_ifs(group_index_fp.c_str());
        IFTD(group_ifs.is_open(), IO_error, "Can't open file %s\n",
                (group_index_fp.c_str()));
        getline(group_ifs, line);
        group_index_ = boost::lexical_cast<int>(line);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Linear_state_space::update_gp() const
{
    IFT(gp_initialized_, Runtime_error, "GP prior is not initialized");
    for(size_t i = 0; i < gp_changed_.size(); i++)
    {
        if(gp_changed_[i])
        {
            double sigma = gp_sigvars_[i];
            double scale = gp_scales_[i];
            if(scale <= FLT_EPSILON) 
            {
                std::cout << "gp_scale: " << scale << std::endl;
            }
            if(sigma <= FLT_EPSILON) 
            {
                std::cout << "gp_signal_sigma: " << sigma << std::endl;
            }
            gp_priors_[i].set_covariance_function(
                gp::Squared_exponential(scale, sigma));
        }
        gp_changed_[i] = false;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Linear_state_space::init_gp
(
    const Double_v& gp_scales,
    const Double_v& gp_sigvars,
    const std::vector<gp::Constant>& mean_funcs
) 
{
    const size_t N = mean_funcs.size();
    IFT(gp_scales.size() == N && gp_sigvars.size() == N, Illegal_argument, 
        "Dimension of gp means different from the ones of gp scales and "
        "gp varaince." );
      
    gp_scales_ = gp_scales; 
    gp_sigvars_ = gp_sigvars; 

    gp_priors_.clear();
    gp_priors_.reserve(N);
    size_t T = times_.size();
    gp_inputs_ = gp::make_inputs(0, T-2, 1); 
    for(size_t i = 0; i < N; i++)
    {
        const gp::Squared_exponential cov(gp_scales_[i], gp_sigvars_[i]);
        gp_priors_.push_back(gp::make_prior(mean_funcs[i], cov, gp_inputs_));
    }
    drift_ = true;
    gp_initialized_ = true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Linear_state_space::init_predictors
(
    const Mod_map& moderators,
    const std::vector<std::vector<std::string> >& mod_names
)
{
    predictors_.clear();
    predictors_.resize(mod_names.size());

    // for each parameter 
    for(size_t c = 0; c < mod_names.size(); c++)
    {
        predictors_[c].push_back(1.0);
        // for each moderator
        for(size_t i = 0; i < mod_names[c].size(); i++)
        {
            const std::string& name = mod_names[c][i];
            if(moderators.find(name) == moderators.end())
            {
                KJB_THROW_3(Illegal_argument, "data does not have moderator %s", 
                                                                (name.c_str()));
            }
            const Double_v& mod_values = moderators.at(name);
            // for each oscillator 
            bool new_osc = true;
            BOOST_FOREACH(const double& val, mod_values)
            {
                if(new_osc || (!new_osc && 
                               !is_shared_moderator(moderators, name)))
                {
                    predictors_[c].push_back(val);
                }
                new_osc = false;
            }
        }
    }

    // for the outcome
    for(size_t i = 0; i < num_outcomes(); i++)
    {
        predictors_.push_back(Vector(1, 1.0));
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Linear_state_space::compute_poly_states
(
    const State_vec& clo_states,
    State_vec& poly_states,
    const Double_v& times
) const
{
    size_t T = times.size();
    if(T == 0) return;
    IFT(clo_states.size() == T, Illegal_argument, 
            "CLO states and times have different sizes in compute_poly_states");
    poly_states.resize(T, State_type((int)num_oscillators_, 0.0));
    // polynomial terms
    assert(polynomial_coefs_.size() == num_oscillators_);
    for(size_t j = 0; j < num_oscillators_; j++)
    {
        const Vector& coef = polynomial_coefs_[j];
        for(size_t i = 0; i < T; i++)
        {
            double poly_term = 0.0;
            for(size_t k = 0; k < coef.size(); k++)
            {
                poly_term += coef[k] * std::pow(times[i], double(k));
            }
            poly_states[i][j] = poly_term + clo_states[i][j];
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Linear_state_space::compute_obs_states
(
    State_vec_vec& obs_states, 
    const State_vec& poly_states
) const
{
    size_t num_obs = obs_names_.size();
    size_t num_times = poly_states.size();
    obs_states.resize(num_times, State_vec(num_obs));

    for(size_t i = 0; i < num_times; i++)
    {
        State_type pred = poly_states[i]; 
        // only consider the state, the second half are first deriviatives 
        if(!polynomial_coefs_.empty())
        {
            KJB(ASSERT(pred.size() == num_oscillators_));
        }
        else
        {
            KJB(ASSERT(pred.size() == num_oscillators_ * 2));
        }
        // compare the true observable conditioned on the real hidden states
        // to the observed observables 
        State_vec cur_obs_states(num_obs, State_type((int)num_oscillators_, 0.0));
        for(size_t j = 0; j < obs_names_.size(); j++)
        {
            if(obs_coef_dim() == 0)
            {
                assert(j == 0);
                State_type states(pred.begin(),
                                  pred.begin() + num_oscillators_);
                cur_obs_states[j] = states;
            }
            else
            {
                for(size_t k = 0; k < num_oscillators_; k++)
                {
                    double state = pred[k];
                    Vector state_v((int)obs_coef_dim(), state);
                    if(obs_coef_dim() == 2) state_v[0] = 1.0;
                    cur_obs_states[j][k] = dot(obs_coefs_[j][k], state_v);
                }
            }
        }
        obs_states[i] = cur_obs_states;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Double_vv Linear_state_space::get_clo_params
(
    const Double_v& times
) const
{
    if(times.empty()) 
    {
        return Double_vv();
    }
    size_t old_co_nums = clos_.size();

    size_t N = times.size();
    assert(N >= 1);
    size_t new_co_nums = N - 1;
    Double_vv all_params(new_co_nums);
    if(!drift_)
    {
        assert(!clos_.empty());
        std::fill(all_params.begin(), all_params.end(), clos_.back().params());
        return all_params; 
    }

    update_gp();
    // params drift over time
    size_t num_params = clos_.back().num_params();
    // Set the gp inputs 
    assert(times_.size() == old_co_nums + 1);
    size_t total_len = old_co_nums + new_co_nums;
    gp::Inputs trins(old_co_nums);
    std::vector<Vector> trouts_t(old_co_nums);
    int new_part = new_co_nums - old_co_nums;

    // setting up training inputs and outputs 
    for(size_t i = 0; i < old_co_nums; i++)
    {
        trins[i].set(times[i]);
        trouts_t[i] = Vector(clos_[i].begin(), clos_[i].end());
        // copy the old values
        all_params[i].resize(num_params);
        for(size_t j = 0; j < num_params; j++)
        {
            all_params[i][j] = clos_[i].params()[j];
        }
    }
    if(new_part == 0)
    {
        return all_params;
    }
    gp::Inputs tein(new_part);
    std::vector<Vector> trouts = get_transpose(trouts_t);

    // setting up the testing inputs
    for(size_t i = old_co_nums; i < new_co_nums; i++)
    {
        size_t real_i = i - old_co_nums;
        tein[real_i].set(times[i]);
    }

    // Use gaussian process predicitive distribution
    typedef gp::Predictive_nl<gp::Constant, gp::Squared_exponential> Gp_pred; 

    const size_t num_samples = 1;
    std::vector<Vector> pred_params(num_params); 
    for(size_t i = 0; i < num_params; i++)
    {
        Gp_pred pred(get_mean_function(i), 
                     //gp::Squared_exponential(gp_scales_[i], gp_sigvars_[i]),
                     get_covariance_function(i),
                     trins.begin(),
                     trins.end(),
                     trouts[i].begin(),
                     trouts[i].end(),
                     tein.begin(),
                     tein.end());

        MV_normal_distribution pred_dist = pred.normal();
        // instead of taking samples from the predictive distribution, 
        // we use the mean of the distribution since we we already 
        // know the shape of the predictive distribution 
        pred_params[i] = pred_dist.get_mean();
    }
    for(size_t i = old_co_nums; i < new_co_nums; i++)
    {
        all_params[i].resize(num_params);
        for(size_t j = 0; j < num_params; j++)
        {
            all_params[i][j] = pred_params[j][i - old_co_nums];
        }
    }
    
    return all_params;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Linear_state_space::write(const std::string& outpath) const
{
    using namespace std;
    // output the parameter of the coupled oscillator models
    ETX(kjb_c::kjb_mkdir(outpath.c_str()));
    string param_fname(outpath + "/com_params.txt");
    ofstream param_ofs(param_fname.c_str());
    IFTD(param_ofs.is_open(), IO_error, "Can't open file %s", 
            (param_fname.c_str()));
    BOOST_FOREACH(const Coupled_oscillator& clo, clos_)
    {
        param_ofs << clo << endl;
    }
    param_ofs.close();

    // update states
    update_obs_states();

    // output the state values 
    string st_fname(outpath + "/states.txt");
    ofstream ofs(st_fname.c_str());
    IFTD(ofs.is_open(), IO_error, "Can't open file %s", (st_fname.c_str()));
    BOOST_FOREACH(const State_type& state, clo_states_)
    {
        ofs << state << endl;
    }
    ofs.close();

    // NO NEED TO OUT PUT THE POLYNOMIAL STATES SINCE WE CAN COMPUTE IT FROM THE
    // POLYNOMIAL_COEFS
    // output the polynomial states
    if(!polynomial_coefs_.empty())
    {
        string poly_st_fname(outpath + "/poly_states.txt");
        ofstream poly_ofs(poly_st_fname.c_str());
        IFTD(poly_ofs.is_open(), IO_error, "Can't open file %s", 
                                    (poly_st_fname.c_str()));
        BOOST_FOREACH(const State_type& state, poly_states_)
        {
            poly_ofs << state << endl;
        }
        poly_ofs.close();
    }

    // output the observable state values 
    string obs_st_fname(outpath + "/obs_states.txt");
    ofstream obs_st_ofs(obs_st_fname.c_str());
    IFTD(obs_st_ofs.is_open(), IO_error, "Can't open file %s", 
            (obs_st_fname.c_str()));
    // headers 
    for(size_t i = 0; i < obs_names_.size(); i++)
    {
        for(size_t j = 0; j < num_oscillators_; j++)
        {
            obs_st_ofs << obs_names_[i] << "-" << j << " ";
        }
    }
    obs_st_ofs << std::endl;
    BOOST_FOREACH(const State_vec& states, obs_states_)
    {
        // each time
        BOOST_FOREACH(const State_type& state, states)
        {
            obs_st_ofs << state << " ";
        }
        obs_st_ofs << std::endl;
    }
    obs_st_ofs.close();

    // output the observation names
    KJB(ASSERT(obs_names_.size() == obs_coefs_.size()));
    string obs_fname(outpath + "/obs.txt");
    ofstream obs_ofs(obs_fname.c_str());
    IFTD(obs_ofs.is_open(), IO_error, "Can't open file %s", (obs_fname.c_str()));
    BOOST_FOREACH(const std::string& name, obs_names_)
    {
        obs_ofs << name << " ";
    }
    obs_ofs << std::endl;

    // output the observation coefs in the following order
    // [obs 0][osc_0]
    // [obs 0][osc_1]
    // [obs 1][obs_0]
    // [obs 1][obs_1]
    for(size_t i = 0; i < obs_coefs_.size(); i++)
    {
        for(size_t j = 0; j < obs_coefs_[i].size(); j++)
        {
            obs_ofs << obs_coefs_[i][j] << endl;
        }
    }
    obs_ofs.close();

    // output gp_scales and gp_sigvars
    if(!gp_scales_.empty())
    {
        string gp_fname(outpath + "/gp_params.txt");
        ofstream gp_ofs(gp_fname.c_str());
        IFTD(gp_ofs.is_open(), IO_error, "Can't open file %s", (gp_fname.c_str()));
        gp_ofs << gp_scales_ << std::endl;
        gp_ofs << gp_sigvars_ << std::endl;

        // write out the gp_means 
        for(size_t i = 0; i < gp_priors_.size(); i++)
        {
            gp_ofs << gp_priors_[i].mean_function().value() << " ";
        }
        gp_ofs << std::endl;

        gp_ofs.close();
    }

    // output the polynomial coefs 
    if(!polynomial_coefs_.empty())
    {
        string poly_fname(outpath + "/polynomial_coefs.txt");
        ofstream poly_ofs(poly_fname.c_str());
        IFTD(poly_ofs.is_open(), IO_error, "Can't open file %s", 
                (poly_fname.c_str()));
        BOOST_FOREACH(const Vector& coef, polynomial_coefs_)
        {
            poly_ofs << coef << std::endl;
        }
        poly_ofs.close();
    }

    // output the outcome 
    if(!outcomes_.empty())
    {
        assert(!outcome_names_.empty());
        string fname(outpath + "/outcomes.txt");
        ofstream ofs(fname.c_str());
        IFTD(ofs.is_open(), IO_error, "Can't open file %s", (fname.c_str()));
        std::copy(outcome_names_.begin(), outcome_names_.end(),
                std::ostream_iterator<std::string>(ofs, " " ));
        ofs << std::endl;
        BOOST_FOREACH(const Vector& coef, outcomes_)
        {
            ofs << coef << std::endl;
        }
        ofs.close();
    }

    if(!obs_noise_sigmas_.empty())
    {
        string obs_noise_fname(outpath + "/obs_noise_sigmas.txt");
        ofstream obs_noise_ofs(obs_noise_fname.c_str());
        IFTD(obs_noise_ofs.is_open(), IO_error, "Can't open file %s", 
                (obs_noise_fname.c_str()));
        obs_noise_ofs << obs_noise_sigmas_ << std::endl;
        obs_noise_ofs.close();
    }

    ////////////////////////////////////////////////////////////////////
    /// WRITE THE MEAN of the COM PARAMS
    if(!clo_means_.empty())
    {
        string mean_param_fname(outpath + "/com_params_ind_priors.txt");
        ofstream mean_param_ofs(mean_param_fname.c_str());
        IFTD(mean_param_ofs.is_open(), IO_error, "Can't open file %s", 
                (mean_param_fname.c_str()));
        BOOST_FOREACH(const double& val, clo_means_)
        {
            mean_param_ofs << val << " " ;
        }
        mean_param_ofs << std::endl;

        BOOST_FOREACH(const double& val, clo_variances_)
        {
            mean_param_ofs << val << " " ;
        }
        mean_param_ofs << std::endl;
    }

    ////////////////////////////////////////////////////////////////////
    /// WRITE THE MEAN of the COM PARAMS
    if(!polynomial_coefs_means_.empty())
    {
        string poly_param_fname(outpath + "/polynomial_coef_priors.txt");
        ofstream poly_param_ofs(poly_param_fname.c_str());
        IFTD(poly_param_ofs.is_open(), IO_error, "Can't open file %s",
                (poly_param_fname.c_str()));
        BOOST_FOREACH(const Vector& val, polynomial_coefs_means_)
        {
            poly_param_ofs << val << ";";
        }

        poly_param_ofs << std::endl;
        BOOST_FOREACH(const Vector& val, polynomial_coefs_variances_)
        {
            poly_param_ofs << val << ";";
        }
        poly_param_ofs << std::endl;
    }
    string group_fp(outpath + "/group_index.txt");
    ofstream group_ofs(group_fp.c_str());
    IFTD(group_ofs.is_open(), IO_error, "Can't open file %s",
            (group_fp.c_str()));
    group_ofs << group_index_ << std::endl;
    std::copy(group_resps_.begin(), group_resps_.end(), 
              std::ostream_iterator<double>(group_ofs, " "));
    group_ofs << std::endl;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Linear_state_space::get_clo_prior() const
{
    double pr = 0.0;
    if(clo_means_.empty() && clo_variances_.empty()) return pr; 
    assert(!drift_);
    if(clo_means_.empty() || clo_variances_.empty()) 
    {
        std::cerr << "[WARNING]: means and variance are not set for "
                  << "Gaussian prior\n";
        return pr;
    }

    if(clo_means_.size() != clo_variances_.size())
    {
        std::cerr << "[WARINING]: different dimension for clo_means and clo_variances";
        std::cerr << clo_means_.size() << " " 
                  << clo_variances_.size() << std::endl;
    }
    assert(clo_means_.size() == clo_variances_.size());

    for(size_t i = 0; i < clo_means_.size(); i++)
    {
        if(std::isnan(clo_means_[i]) || std::isnan(clo_variances_[i]) ||
           std::isinf(clo_means_[i]) || std::isinf(clo_variances_[i]) ||
           clo_variances_[i] < 0.0 )
        {
            return -std::numeric_limits<double>::max();
        }
        assert(i < clos_.front().num_params());
        Gaussian_distribution P(clo_means_[i], std::sqrt(clo_variances_[i]));
        BOOST_FOREACH(const Coupled_oscillator& co, clos_)
        {
            // NOTE: change variance to sigma
            pr += log_pdf(P, co.get_param(i));
        }
    }

    return pr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Linear_state_space::get_drift_prior() const
{
    double pr = 0.0;
    if(!drift_)
    {
        return pr;
    }
    for(size_t i = 0; i < gp_changed_.size(); i++)
    {
        if(gp_changed_[i])
        {
            if(gp_scales_[i] <= FLT_EPSILON || gp_sigvars_[i] <= FLT_EPSILON) 
            {
                std::cerr << "[WARNING]: gp scale or gp variance is negative\n";
                std::cout << "[WARNING]: gp scale or gp variance is negative\n";
                return -std::numeric_limits<double>::max();
            }
        }
    }
    update_gp();
    std::vector<Vector> outputs = get_gp_outputs();
    for(size_t i = 0; i < gp_priors_.size(); i++)
    {
        const MV_normal_distribution& prior_P = gp_priors_[i].normal();
        assert(outputs.size() == clos_[0].num_params());
        pr += log_pdf(prior_P, outputs[i]);
    }
    return pr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Linear_state_space::parse_obs_coef(const std::string& obs_fname) const
{
    using namespace std;
    ifstream obs_ifs(obs_fname.c_str());
    IFTD(obs_ifs.is_open(), IO_error, "Can't open file %s\n",
            (obs_fname.c_str()));

    string line;
    std::set<string> obs_names;
    std::set<string>::iterator on_it = obs_names.begin();
    // parse the names 
    getline(obs_ifs, line);
    istringstream istr(line);
    copy(istream_iterator<string>(istr), istream_iterator<string>(), 
         inserter(obs_names, on_it));
    IFTD(obs_names.size() == obs_names_.size(), 
         IO_error, "%s has different number of observales than the model",
         (obs_fname.c_str()));

    for(size_t j = 0; j < obs_names_.size(); j++)
    {
        if(obs_names.find(obs_names_[j]) == obs_names.end())
        {
            KJB_THROW_3(IO_error, "observable %s is not specified in the file ",
                    (obs_names_[j].c_str()));
        }
    }

    // parse the coefs 
    size_t obs_i = 0;
    size_t osc_i = 0;
    while(getline(obs_ifs, line))
    {
        Vector coef;
        istringstream istr(line);
        copy(istream_iterator<double>(istr), istream_iterator<double>(), 
             back_inserter(coef));
        set_obs_coef(obs_i, osc_i, coef);
        osc_i++;
        if(osc_i % num_oscillators_ == 0)
        {
            obs_i++;
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool Linear_state_space::sample_clo_from_ind_gauss_prior()
{
    if(clos_.empty()) return true;

    bool valid = false;
    size_t counts = 0;
    const size_t MAX_TRIES = 1000;
    int damping_start_index = clos_[0].num_params() - num_oscillators_;
    std::cout << "damping_start_index: " << damping_start_index << std::endl;
    while(!valid && counts < MAX_TRIES)
    {
        Vector params((int)clos_[0].num_params(), 0.0);
        for(size_t i = 0; i < clos_[0].num_params(); i++)
        {
            double mean = clo_means_[i];
            double variance = clo_variances_[i];
            if(!clo_means_.empty())
            {
                assert(i < clo_means_.size());
                assert(i < clo_variances_.size());
                mean = clo_means_[i];
                variance = clo_variances_[i];
            }
            Gaussian_distribution P(mean, sqrt(variance));

            // Sample the log of the params
            double val = sample(P);
            if(i >= damping_start_index)
            {
                params[i] = val;
            }
            else 
            {
                while(val < 0)
                {
                    val = sample(P);
                }
                params[i] = val;
            }
        }
        try
        {
            for(size_t i = 0; i < params.size(); i++)
            {
                BOOST_FOREACH(Coupled_oscillator& clo, clos_)
                {
                    clo.set_param(i, params[i]);
                }
            }
            valid = true;

            std::cout << " params: ";
            std::copy(params.begin(), params.end(), std::ostream_iterator<double>(
                        std::cout, " " ));
        }
        catch(Exception& e)
        {
            valid = false;
            std::cerr << "mode angles are not valid\n";
        }

        // check to see if the f < c TODO 
        /*if(params[0] < params[0 + 2] && params[3] < params[3 + 2])
        {
            valid = true;

            for(size_t i = 0; i < params.size(); i++)
            {
                BOOST_FOREACH(Coupled_oscillator& clo, clos_)
                {
                    clo.set_param(i, params[i]);
                }
            }
            break;
        }*/
        counts++;
    }

    changed_index_ = 0;
    return valid;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool Linear_state_space::sample_clo_from_gp_prior()
{
    assert(drift_);
    update_gp();
    std::vector<Vector> params(gp_priors_.size());
    try
    {
        // get the samples
        int params_per_osc = 1 + num_oscillators_;
        for(size_t i = 0; i < gp_priors_.size(); i++)
        {
            //params[i] = gp_priors_[i].normal().get_mean();
            params[i] = gp::sample(gp_priors_[i]);
            if(i % params_per_osc == 0) 
            {
                bool valid = false;
                size_t counts = 0;
                while(!valid)
                {
                    BOOST_FOREACH(double val, params[i])
                    {
                        if(val < 0.0)
                        {
                            valid = true;
                        }
                        else
                        {
                            valid = false;
                            counts++;
                            break;
                        }
                    }
                    if(counts > 1000) return false;
                }
            }
        }

        // set the param based on the samples
        for(size_t i = 0; i < clos_.size(); i++)
        {
            for(size_t j = 0; j < clos_[i].num_params(); j++)
            {
                clos_[i].set_param(j, params[j][i]);
            }
        }
        changed_index_ = 0;
    }
    catch (Exception& err)
    {
        std::cerr << "Error in sample lss from gp prior.\n";
        return false;
    }
    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Linear_state_space::get_polynomial_prior() const
{
    double pr = 0.0;
    if(polynomial_coefs_means_.empty() && polynomial_coefs_variances_.empty()) 
    {
        return pr; 
    }
    if(polynomial_coefs_means_.empty() || polynomial_coefs_variances_.empty()) 
    {
        std::cerr << "poly means: " << polynomial_coefs_means_.size() 
                  << "poly variance: " << polynomial_coefs_variances_.size() 
                  << std::endl;
        std::cerr << "[WARNING]: means and variance are not set for Gaussian prior\n";
        return pr;
    }

    size_t N = polynomial_coefs_.size();
    assert(polynomial_coefs_means_.size() == N);
    assert(polynomial_coefs_variances_.size() == N);

    for(size_t i = 0; i < N; i++)
    {
        MV_gaussian_distribution P(polynomial_coefs_means_[i], 
                                   polynomial_coefs_variances_[i]);
        pr += log_pdf(P, polynomial_coefs_[i]);
    }

    return pr;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool Linear_state_space::sample_polynomial_coefs()
{
    if(polynomial_coefs_.empty()) return false;
    assert(polynomial_coefs_.size() == polynomial_coefs_means_.size());
    assert(polynomial_coefs_.size() == polynomial_coefs_variances_.size());
    for(size_t i = 0; i < polynomial_coefs_means_.size(); i++)
    {
        MV_gaussian_distribution P(polynomial_coefs_means_[i], 
                                   polynomial_coefs_variances_[i]);
        Vector val = sample(P);
        polynomial_coefs_[i] = val;
    }
    poly_coefs_dirty_ = true;
    obs_coefs_dirty_ = true;
    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool Linear_state_space::has_valid_params() const
{
    // is use modal representation check to see if it has valid angles 
    if(use_modal())
    {
        for(size_t c_i = 0; c_i < clos_.size(); c_i++)
        {
            for(size_t i = 0; i < num_oscillators_; i++)
            {
                double value = clos_[c_i].get_param(i);
                if(i < num_oscillators_)
                {
                    // make sure the mode angle is between [-M_PI/2.0, M_PI/2.0]
                    // this might not be the right thing to do
                    // we should reject the samples that are outside the range 
                    if(value > M_PI || value < -M_PI) 
                    {
                        std::cerr << "angle is out of the bound\n";
                        return false;
                    }
                }
            }
            if(clos_[c_i].get_param(0) > clos_[c_i].get_param(1))
            {
                std::cerr << " param 0 > param 1\n";
                return false;
            }
        }
    }
    return true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Linear_state_space::get_param(size_t index, int time) const
{
    if(index < num_clo_params())
    {
        if(time >= 0)
        {
            assert(time < clos_.size());
            return clos_[time].params()[index];
        }
        else
        {
            return clos_.front().params()[index];
        }
    }
    else if(index < num_clo_params() + num_polynomial_coefs())
    {
        size_t iindex = index - num_clo_params();
        size_t osc_index = iindex / polynomial_dim_per_osc();
        size_t poly_index = iindex % polynomial_dim_per_osc(); 
        return polynomial_coefs_[osc_index][poly_index];
    }
    else
    {
        int iindex = index - num_clo_params() - num_polynomial_coefs();
        assert(iindex >= 0 && iindex < num_outcomes());
        return get_outcome(iindex);
    }
}

