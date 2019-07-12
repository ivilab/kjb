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

/* $Id: adapter.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_ADAPTER_H
#define KJB_TIES_ADAPTER_H

#include <vector>
#include "dbn_cpp/linear_state_space.h"
#include "dbn_cpp/lss_set.h"

#include <boost/foreach.hpp>
namespace kjb {
namespace ties {

/**
 * @class   Adapter for compute gradient of a Linear_state_space,
 *          parameters include CLO params, init state, and output 
 *          vector. 
 */
class Lss_adapter
{
public:
    /** @brief  Ctor. */
    Lss_adapter
    (
        bool sample_init_state = true,
        bool sample_clo_params = true
    ) : 
        sample_init_state_(sample_init_state),
        sample_clo_params_(sample_clo_params)
    {}

    /** @brief  Getter. */
    double get(const Linear_state_space* lss, size_t i) const;

    /** @brief  Setter. */
    void set(Linear_state_space* lss, size_t i, double value) const;

    /** @brief  Returns the number of parameters. */
    size_t size(const Linear_state_space* lss) const;

    /** @brief  Sets the value of two parameters (used for Hessian). */
    /*void set
    (
        Linear_state_space* lss, 
        size_t i, 
        size_t j, 
        double v, 
        double w
    ) const
    {
        IFT(i != j, Illegal_argument, 
                "WARNING: setting same dimension to two different values");
        set(lss, i, v);
        set(lss, j, w);
    }*/

private:
    bool sample_init_state_; 
    bool sample_clo_params_;
};

/**
 * @class   Adapter for compute gradient of the init state of a
 *          Linear_state_space.
 */
class Init_state_adapter
{
public:
    /** @brief    Ctor.  */
    Init_state_adapter() {}

    /** @brief  Getter. */
    double get(const Linear_state_space* lss, size_t i) const
    {
        assert(i < lss->init_state().size());
        return lss->init_state()[i];
    }

    /** @brief  Setter. */
    void set(Linear_state_space* lss, size_t i, double value) const
    {
        assert(i < lss->init_state().size());
        lss->init_state()[i] = value;
        lss->changed_index() = 0;
    }

    /** @brief  Returns the number of parameters. */
    size_t size(const Linear_state_space* lss) const
    {
        return lss->init_state().size();
    }
}; 

/** 
 * @class    Adapter for couple-shared parameters.
 */
class Shared_param_adapter
{
public:
    /** @brief    Ctor. */
    Shared_param_adapter(bool exclude_variance = false) 
        : exclude_variance_(exclude_variance)//, initialized_(false)
    {}

    /** @brief    Getter. */
    double get(const Lss_set* lsss, size_t i) const;

    /** @brief    Setter. */
    void set(Lss_set* lsss, size_t i, double value) const;

    /** @brief    Returns the number of parameters. */
    size_t size(const Lss_set* lsss) const
    {
        size_t num_params = 0;
        size_t num_groups = lsss->num_groups(); 
        for(size_t g = 0; g < num_groups; g++)
        {
            num_params += lsss->shared_param_size(g, exclude_variance_);
        }
        return num_params;
    }

private:
    bool exclude_variance_;
}; 

/** 
 * @class    Adapter for couple-shared obs noise.
 */
class Shared_obs_noise_adapter 
{
public:
    Shared_obs_noise_adapter() {}

    double get(const Lss_set* lsss, size_t i) const
    {
        IFT(i < lsss->get_noise_sigmas().size(), Illegal_argument, 
                "Index out of bound (Shared_obs_noise_adapter::get)");
        return lsss->get_noise_sigmas()[i];
    }

    void set(Lss_set* lsss, size_t i, double value) const
    {
        lsss->set_noise_sigmas(i, value);
    }

    size_t size(const Lss_set* lsss) const 
    {
        return lsss->get_noise_sigmas().size();
    }
};

/** 
 * @class    Adapter for couple-shared obs coefs.
 */
class Shared_obs_coef_adapter 
{
public:
    Shared_obs_coef_adapter
    (
        size_t num_obs,
        size_t num_osc,
        size_t obs_coef_dim
    ) : test(100)
    {
        compute_param_map(num_obs, num_osc, obs_coef_dim);
    }

    double get(const Lss_set* lsss, size_t i) const
    {
        std::map<size_t, std::vector<size_t> >::const_iterator param_it
                        = param_ind_map_.find(i);
        IFT(param_it != param_ind_map_.end(), 
                Runtime_error, "invalid index value.");
        size_t obs_index = param_it->second[0];
        size_t osc_index = param_it->second[1];
        size_t param_index = param_it->second[2];
        return lsss->obs_coefs()[obs_index][osc_index][param_index];
    }

    void set(Lss_set* lsss, size_t i, double value) const
    {
        std::map<size_t, std::vector<size_t> >::const_iterator param_it
                        = param_ind_map_.find(i);
        IFT(param_it != param_ind_map_.end(), 
                Runtime_error, "invalid index value.");
        size_t obs_index = param_it->second[0];
        size_t osc_index = param_it->second[1];
        size_t param_index = param_it->second[2];
        lsss->set_obs_coef(obs_index, osc_index, param_index, value);
    }

    size_t size(const Lss_set* lsss) const 
    {
        return lsss->num_obs_coefs();
    }

private:
    std::map<size_t, std::vector<size_t> > param_ind_map_;
    size_t test;

    void compute_param_map
    (
        size_t num_obs,
        size_t num_osc,
        size_t obs_coef_dim
    );
};

/** 
 * @class    Adapter for couple-shared gp-scale.
 */
class Shared_gp_scale_adapter 
{
public:
    Shared_gp_scale_adapter() {}

    double get(const Lss_set* lsss, size_t i) const
    {
        const Double_v& gp_scales = lsss->gp_scales();
        size_t nv = gp_scales.size();
        IFT(i < nv, Illegal_argument, 
                "Index out of bound in Shared_gp_scale_adapter::get");
        return gp_scales[i];
        // sampling in log space
        //return std::log(gp_scales[i]);
    }

    void set(Lss_set* lsss, size_t i, double value) const
    {
        // transfering back from log space
        //lsss->update_gp_scales(i, std::exp(value));
        for(size_t j = 0; j < lsss->lss_vec().size(); j++)
        {
            lsss->update_gp_scales(j, i, value);
        }
    }

    size_t size(const Lss_set* lsss) const 
    {
        return lsss->gp_scales().size();
    }
};

/** 
 * @class    Adapter for person-specific CLO parameter.
 */
class Person_param_adapter
{
public:
    /** 
     * @brief    Constructs a Person_param_adapter.
     * @param    persons    a vector of booleans with 1 indicates
     *                      person-specific index.
     */
    Person_param_adapter
    (
        const std::vector<size_t>& persons
    ) : 
        person_indices_(persons)
    {}

    /** @brief    Getter. */
    double get(const Linear_state_space* lss, size_t i) const
    {
        const Coupled_oscillator_v& clos = lss->coupled_oscillators();
        assert(!clos.empty());
        assert(!lss->allow_drift());
        assert(i < person_indices_.size());
        size_t index = person_indices_[i];
        return clos[0].params()[index];
    }

    /** @brief    Setter. */
    void set(Linear_state_space* lss, size_t i, double value) const;

    /** @brief    Return the number of parameters. */
    size_t size(const Linear_state_space* ) const
    {
        return person_indices_.size();
    }

private:
    std::vector<size_t> person_indices_;
}; 

}} //namespace kjb::ties

#endif //KJB_TIES_ADAPTER_H

