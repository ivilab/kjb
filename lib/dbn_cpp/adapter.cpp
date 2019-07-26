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

/* $Id: adapter.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <l/l_sys_def.h>
#include <l/l_sys_debug.h>
#include <l_cpp/l_util.h>

#include <boost/foreach.hpp>
#include <vector>

#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/coupled_oscillator.h"
#include "dbn_cpp/adapter.h"


using namespace kjb;
using namespace kjb::ties;

double Lss_adapter::get(const Linear_state_space* lss, size_t i) const
{
    using namespace std;
    const State_type& init_state = lss->init_state();
    const Coupled_oscillator_v& clos = lss->coupled_oscillators();
    KJB(ASSERT(!init_state.empty()));
    KJB(ASSERT(!clos.empty()));
    size_t num_params = clos.front().num_params();
    size_t num_clos = clos.size();
    if(!lss->allow_drift())
    {
        num_clos = 1;
    }
    size_t n1 = sample_clo_params_? num_clos * num_params : 0; 
    size_t n2 = sample_init_state_? init_state.size() : 0;
    //size_t n_obs = sample_obs_coef_ ? lss->obs_names().size() : 0; 
    if(i < n1)
    {
        size_t c_i = i / num_params;
        size_t p_i = i - c_i * num_params;
        KJB(KJB(ASSERT(p_i < num_params && c_i < num_clos)));
        if(!lss->allow_drift())
        {
            KJB(ASSERT(c_i == 0));
        }
        return clos.at(c_i).get_param(p_i);
    }
    else if(i < n1 + n2) 
    {
        // i >= n1  if n2 == 0, will not enter this part
        KJB(ASSERT(sample_init_state_));
        size_t n_i = i - n1;
        return init_state[n_i];
    }
    else
    {
        KJB_THROW_2(Runtime_error, "Should never reach here");
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Lss_adapter::set(Linear_state_space* lss, size_t i, double value) const
{
    using namespace std;
    State_type& init_state = lss->init_state();
    Coupled_oscillator_v& clos = lss->coupled_oscillators();
    KJB(ASSERT(!init_state.empty()));
    KJB(ASSERT(!clos.empty()));

    size_t num_params = clos[0].num_params();
    size_t num_clos = clos.size();
    if(!lss->allow_drift())
    {
        num_clos = 1;
    }
    size_t n1 = sample_clo_params_? num_clos * num_params : 0; 
    size_t n2 = sample_init_state_ ? init_state.size() : 0;
    //size_t n_obs = lss->obs_names().size(); 
    if(i < n1)
    {
        size_t c_i = i / num_params;
        size_t p_i = i - c_i * num_params;
        KJB(ASSERT(p_i < num_params && c_i < num_clos));
        if(!lss->allow_drift())
        {
            KJB(ASSERT(c_i == 0));
            BOOST_FOREACH(Coupled_oscillator& clo, clos)
            {
                clo.set_param(p_i, value);
            }
            lss->changed_index() = 0;
        }
        else
        {
            clos[c_i].set_param(p_i, value);
            lss->changed_index() = c_i;
        }
    }
    else if(i < n1 + n2)
    {
        if(sample_init_state_)
        {
            size_t n_i = i - n1;
            init_state[n_i] = value;
            lss->changed_index() = 0;
        }
    }
    else
    {
        KJB_THROW_2(Runtime_error, "Should never reach here");
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

size_t Lss_adapter::size(const Linear_state_space* lss) const
{
    using namespace std;
    const State_type& init_state = lss->init_state();
    const Coupled_oscillator_v& clos = lss->coupled_oscillators();
    size_t N = 0;
    if(sample_clo_params_)
    {
        if(lss->allow_drift())
        {
            BOOST_FOREACH(const Coupled_oscillator& co, clos)
            {
                N += co.num_params();
            }
        }
        else
        {
            N += clos.front().num_params();
        }
    }
    N += sample_init_state_ ? init_state.size() : 0;

    return N;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Shared_param_adapter::get(const Lss_set* lsss, size_t i) const
{
    size_t group_id = i / lsss->shared_param_size(0, exclude_variance_);
    const std::vector<Vector>& pred_coefs = lsss->pred_coefs(group_id);
    const std::vector<double>& vars = lsss->variances(group_id);

    size_t pred_size = lsss->ignore_clo() ? 0 : lsss->pred_coef_size(group_id);
    size_t nv = lsss->fixed_clo() ? 0 : vars.size();
    if(i < pred_size)
    {
        size_t pred_per = pred_coefs[0].size();
        size_t index_1 = i / pred_per;
        size_t index_2 = i % pred_per; 
        KJB(ASSERT(index_1 < pred_coefs.size()));
        KJB(ASSERT(index_2 < pred_coefs[index_1].size()));
        return pred_coefs[index_1][index_2];
    }
    else
    {
        assert(i < nv + pred_size);
        KJB(ASSERT(!exclude_variance_));
        KJB(ASSERT(!lsss->fixed_clo()));
        size_t d = i - pred_size;
        KJB(ASSERT(d < nv));
        return vars[d];
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Shared_param_adapter::set(Lss_set* lsss, size_t i, double value) const
{
    size_t group_id = i / lsss->shared_param_size(0, exclude_variance_);
    std::vector<Vector>& pred_coefs = lsss->pred_coefs(group_id);
    std::vector<double>& variances = lsss->variances(group_id); 
    size_t nv = lsss->fixed_clo() ? 0 : variances.size();
    size_t pred_size = lsss->ignore_clo() ? 0 : lsss->pred_coef_size(group_id);

    if(i < pred_size)
    {
        size_t pred_per = pred_coefs[0].size();
        size_t index_1 = i / pred_per;
        size_t index_2 = i % pred_per; 
        KJB(ASSERT(index_1 < pred_coefs.size()));
        KJB(ASSERT(index_2 < pred_coefs[index_1].size()));
        pred_coefs[index_1][index_2] = value;
        lsss->update_means();
    }
    else 
    {
        assert(i < nv + pred_size);
        KJB(ASSERT(!exclude_variance_));
        KJB(ASSERT(!lsss->fixed_clo()));
        size_t d = i - pred_size;
        KJB(ASSERT(d < nv));
        variances[d] = value;
        lsss->update_variances();
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Shared_obs_coef_adapter::compute_param_map
(
    size_t num_obs,
    size_t num_osc,
    size_t obs_coef_dim
)
{
    assert(num_obs >= 1);
    if(obs_coef_dim == 0) return;
    size_t num_params = num_osc * (num_obs - 1) * obs_coef_dim;
    size_t obs_index = 1;
    size_t osc_index = 0;
    size_t param_index = 0;
    for(size_t i = 0; i < num_params; i++)
    {
        param_ind_map_[i] = std::vector<size_t>(3, 0);
        param_ind_map_[i][0] = obs_index;
        param_ind_map_[i][1] = osc_index;
        param_ind_map_[i][2] = param_index;
        param_index++;
        if(param_index == obs_coef_dim)
        {
            osc_index++;
            param_index = 0;
        }
        if(osc_index == num_osc)
        {
            obs_index++;
            osc_index = 0;
        }
    }
    assert(obs_index == num_obs);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Person_param_adapter::set
(   
    Linear_state_space* lss, 
    size_t i, 
    double value
) const 
{
    Coupled_oscillator_v& clos = lss->coupled_oscillators();
    KJB(ASSERT(!clos.empty()));
    KJB(ASSERT(i < person_indices_.size()));
    size_t index = person_indices_[i];
    BOOST_FOREACH(Coupled_oscillator& clo, clos)
    {
        clo.set_param(index, value);
    }
}

