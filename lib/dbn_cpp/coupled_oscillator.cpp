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

/* $Id: coupled_oscillator.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <l_cpp/l_util.h>
#include <l/l_sys_def.h>
#include <gsl_cpp/gsl_matrix.h>
#include <gsl_cpp/gsl_linalg.h>
#include <boost/numeric/odeint.hpp>
#include <boost/foreach.hpp>
#include "dbn_cpp/coupled_oscillator.h"

using namespace kjb;
using namespace kjb::ties;

Double_v kjb::ties::coupled_oscillator_params
(
    size_t num_oscillators,
    double period,
    double damping,
    bool use_modal,
    bool random
)
{
    Double_v params(param_length(num_oscillators, use_modal));
    double freq = (2 * M_PI) / period;
    size_t start_index = 0;
    if(!use_modal)
    {
        ASSERT(params.size() == 8);
        // frequency
        for(size_t i = 0; i < num_oscillators; i++)
        {
            params[start_index++] = freq * freq;
        }
        // coupling for frequency
        size_t num_couples = (num_oscillators - 1) * num_oscillators;
        for(size_t i = 0; i < num_couples; i++)
        {
            params[start_index++] = 0.0;
        }

        // damping
        for(size_t i = 0; i < num_oscillators; i++)
        {
            params[start_index++] = damping;
        }
        // coupling for damping 
        for(size_t i = 0; i < num_couples; i++)
        {
            params[start_index++] = 0.0;
        }
        ASSERT(start_index == 8);
    }
    else
    {
        ASSERT(params.size() == 6);
        IFT(num_oscillators == 2, Illegal_argument, 
                "Modal reprsentation only work with two oscillator now");
        // mode angle
        params[start_index++] = 0.1;
        params[start_index++] = M_PI/2.0 + 0.1;
        // frequency
        for(size_t i = 0; i < num_oscillators; i++)
        {
            params[start_index++] = freq;
        }
        // damping
        for(size_t i = 0; i < num_oscillators; i++)
        {
            params[start_index++] = damping;
        }
        ASSERT(start_index == 6);
    }

    if(random)
    {
        for(size_t i = 0; i < params.size(); i++)
        {
            params[i] += kjb_c::kjb_rand() * 0.1 * params[i];
        }
    }
    return params;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::operator()
(
    const State_type& x, 
    State_type& dxdt, 
    const double /* t */
) const 
{
    IFT(!params_.empty(), Runtime_error, 
            "The parameters of coupled oscillator is not set.");
    KJB(ASSERT(x.size() == 2 * n_));
    const size_t param_per_ddx = n_ + 1; // == 2 + n_ - 1; 
    size_t start_index = 0;
    for(size_t i = 0; i < n_; i++) 
    {
        // first deriviative
        dxdt[i] = x[n_ + i];

        // second deriviative
        double freq = params_[start_index++];
        double damp = params_[start_index++];

        dxdt[n_ + i] = freq * x[i] + damp * dxdt[i];

        // coupling 
        for(size_t j = 0; j < i ; j++)
        {
            dxdt[n_ + i] += params_[start_index++] * (x[j] - x[i]);
        }
        for(size_t j = i + 1; j < n_; j++)
        {
            dxdt[n_ + i] += params_[start_index++] * (x[j] - x[i]);
        }
        KJB(ASSERT((start_index) % param_per_ddx == 0));
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::update_system_matrix()
{
    if(!system_matrix_dirty_) return;
    IFT(!params_.empty(), Runtime_error, 
            "The parameters of coupled oscillator is not set.");
    const size_t param_per_ddx = n_ + 1; // == 2 + n_ - 1; 
    size_t N = n_ * 2;
    A_.resize(N, N, 0.0);
    
    // First deriviative
    for(size_t i = 0; i < n_; i++) 
    {
        A_(i, n_ + i) = 1.0;
    }

    // Second derivative
    size_t start_index = 0;

    double f1 = params_[start_index++];
    double f2 = params_[start_index++];
    double c1 = params_[start_index++];
    double c2 = params_[start_index++];
    double d1 = params_[start_index++];
    double d2 = params_[start_index++];
    double cd1 = params_[start_index++];
    double cd2 = params_[start_index++];

    Matrix M_inv_K(n_, n_);
    M_inv_K(0, 0) = f1 + c1;
    M_inv_K(0, 1) = -c1;
    M_inv_K(1, 0) = -c2;
    M_inv_K(1, 1) = f2 + c2;

    Matrix M_inv_D(n_, n_, 0.0);
    M_inv_D(0, 0) = d1 + cd1;
    M_inv_D(0, 1) = -cd1;
    M_inv_D(1, 0) = -cd2;
    M_inv_D(1, 1) = d2 + cd2;
    
    A_.replace(n_, 0, -M_inv_K);
    A_.replace(n_, n_, -M_inv_D);
    system_matrix_dirty_ = false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::update_modal_eigen_vector(const Double_v& params)
{
    size_t i = 0;
    double alpha = -1.0;
    std::vector<double> mode_angle(n_);
    for(size_t n = 0; n < n_; n++)
    {
        mode_angle[n] = params_[i++];
    }
    for(size_t n = 0; n < n_; n++)
    {
        //TODO check modal angles to make sure they are in the support area
        if(fabs(mode_angle[n] - M_PI/2.0) < FLT_EPSILON || 
           fabs(mode_angle[n] + M_PI/2.0) < FLT_EPSILON)
        {
            alpha = 1.0;
            if(n == 0)
            {
                mode_angle[1] == 0.0;
            }
            else
            {
                mode_angle[0] == 0.0;

            }
            break;
        }
        else
        {
            alpha *= tan(mode_angle[n]);
        }
    }
    if(alpha == 0)
    {
        alpha = 1.0;
    }
    //IFT(alpha > 0, Runtime_error, "alpha is not positive");
    // real mass, stiffness, and damping matrix 
    M_(0, 0) = alpha;
    M_(1, 1) = 1.0;
    for(size_t n = 0; n < n_; n++)
    {
        double radius = pow(cos(mode_angle[n]), 2.0) * alpha + 
                        pow(sin(mode_angle[n]), 2.0);
        // when radius is negative, take its absolute value. 
        if(radius < 0)
        {
            radius = -radius;
        }
        IFT(radius > 0, Runtime_error, "radius is not positive");
        radius = sqrt(1.0/radius);
        double x = radius * cos(mode_angle[n]);
        double y = radius * sin(mode_angle[n]);
        modal_matrix_.set_col(n, Vector(x, y));
    }
    // The modal matrix might not be invertible 
    try
    {
        modal_matrix_inv_ = modal_matrix_.inverse();
    }
    catch(Exception& ex)
    {
        std::cerr << "Modal matrix is not invertible.\n";
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::update_damping_frequency(const Double_v& params)
{
    size_t i = n_; // start from the "n-"th param
    std::vector<double> d_v(n_);
    for(size_t n = 0; n < n_; n++)
    {
        ASSERT(i < params_.size());
        w_n_[n] = params_[i];
        IFT(w_n_[n] > 0, Runtime_error, "angular velocity is not positive");
        ASSERT(i + n_ < params_.size());
        d_v[n] = params_[i + n_]; 
        double damping_ratio = d_v[n] / (2.0 * w_n_[n]); 
        damping_ratio_squared_[n] = damping_ratio * damping_ratio;
        double temp = 1.0 - damping_ratio_squared_[n];
        damping_ratio_cache_[n] = sqrt(temp);
        w_d_[n] = w_n_[n] * (temp);
        i++;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::update_modal_representation()
{
    using namespace std;
    if(!modal_rep_dirty_) return;
    // update modal eigen vector
    update_modal_eigen_vector(params_);

    // update damping ratio and the related quantity
    update_damping_frequency(params_);

    cache_dirty_ = true; 
    modal_rep_dirty_ = false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::update_modal_representation(size_t index, double new_val)
{
    ASSERT(index < params_.size());
    params_[index] = new_val;

    if(index < n_)
    {
        // update the modal angle
        update_modal_eigen_vector(params_);
    }
    else 
    {
        update_damping_frequency(params_);
    }
    cache_dirty_ = true; 
    modal_rep_dirty_ = false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::update_system_matrix(size_t index, double new_val)
{
    if(!system_matrix_dirty_) return;
    params_[index] = new_val;
    update_system_matrix();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::update_system_matrix_from_modal()
{
    if(!system_matrix_dirty_)
    {
        return;
    }
    if(modal_rep_dirty_)
    {
        update_modal_eigen_vector(params_);
        update_damping_frequency(params_);
    }
    std::vector<double> d_v(n_);
    size_t param_length = params_.size();
    size_t damping_start = param_length - n_;
    for(size_t n = 0; n < n_; n++)
    {
        d_v[n] = params_(damping_start + n);
    }
    
    Matrix W = create_diagonal_matrix(w_n_);
    Matrix D_v = create_diagonal_matrix(d_v);
    // square W 
    K_ = modal_matrix_inv_.transpose() * get_squared_elements(W) * 
         modal_matrix_inv_;
    D_ = modal_matrix_inv_.transpose() * D_v * modal_matrix_inv_; 

    Matrix M_inv(M_);
    for(size_t n = 0; n < n_; n++)
    {
        M_inv(n, n) = 1.0 / M_inv(n, n);
    }
    Matrix M_inv_C = M_inv * D_; 
    Matrix M_inv_K = M_inv * K_;

    std::cout << "D: " << D_ << std::endl;
    std::cout << "K: " << K_ << std::endl;
    if(A_.get_num_rows() == 0)
    {
        A_.resize(n_ * 2, n_ * 2, 0.0);
        for(size_t i = 0; i < n_; i++) 
        {
            A_(i, n_ + i) = 1.0;
        }
    }
    A_.replace(n_, 0, -M_inv_K);
    A_.replace(n_, n_, -M_inv_C);
    system_matrix_dirty_ = false;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Coupled_oscillator::set_param(size_t index, double new_val)
{
    IFT(index < params_.size(), Illegal_argument, "Index out of bound ");
    params_[index] = new_val;
    system_matrix_dirty_ = true;
    modal_rep_dirty_ = true;
    cache_dirty_ = true;
    if(!use_modal_)
    {
        update_system_matrix(index, new_val);
    }
    else
    {
        // check the mode angles to make sure they are within 
        // -pi/2.0 to pi/2.0 ? 
        if(index == 0 || index == 1)
        {
            ASSERT(params_[index] <= M_PI);
            ASSERT(params_[index] >= -M_PI);
        }
        try
        {
            update_modal_representation(index, new_val);
        }
        catch(Exception& e)
        {
            std::cerr << e.get_msg() << " kjb error failed to set param\n";
        }
    }
}

State_type Coupled_oscillator::get_modal_state
(
    const State_type& initial_state, 
    double time,
    bool use_cached_
) 
{
    // check to see modal representation is updated 
    State_type next = initial_state;
    if(!use_cached_ || cache_dirty_)
    {
        // recompute the cached value
        for(size_t n = 0; n < n_; n++)
        {
            ASSERT(damping_ratio_squared_.size() == n_);
            ASSERT(w_d_.size() == n_);
            ASSERT(damping_val_.size() == n_);
            ASSERT(cos_psi_val_.size() == n_);
            double damping_ratio = sqrt(damping_ratio_squared_[n]);
            double temp = 1.0 - damping_ratio_squared_[n];
            double psi = atan(damping_ratio/temp);
            cos_psi_val_[n] = cos(w_d_[n] * time - psi);
            sin_psi_val_[n] = sin(w_d_[n] * time - psi);
            cos_val_[n] = cos(w_d_[n] * time);
            sin_val_[n] = sin(w_d_[n] * time);
            damping_val_[n] = -damping_ratio * w_n_[n] * time;
            damping_val_exp_[n] = exp(damping_val_[n]);
        }
        cache_dirty_ = false;
    }

    for(size_t n = 0; n < n_; n++)
    {
        double term_1 = initial_state[n] / damping_ratio_cache_[n];
        double term_2 = initial_state[n + n_] / w_d_[n];

        double part_1 = term_1 * cos_psi_val_[n] +  term_2 * sin_val_[n];
        double part_2 = damping_val_exp_[n];

        next[n] = part_1 * part_2; 
        next[n + n_] = next[n] * (damping_val_[n]/time) + 
            part_2 * (-term_1 * sin_psi_val_[n] * w_d_[n] + initial_state(n + n_) * cos_val_[n]); 
    }

    return next;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

State_type Coupled_oscillator::get_state
(
    const State_type& modal_state,
    bool modal_to_real
) const
{
    State_type state((int)n_ * 2, 0.0);
    for(size_t c = 0; c < modal_matrix_.get_num_cols(); c++)
    {
        for(size_t rr = 0; rr < modal_matrix_.get_num_rows(); rr++)
        {
            double val;
            if(modal_to_real)
            {
                val = modal_matrix_(c, rr);
            }
            else
            {
                val = modal_matrix_inv_(c, rr);
            }
            state[c] += modal_state[rr] * val;
            state[c + n_] += modal_state[rr + n_] * val;
        }
    }

    return state;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/* 

/*
 * =============================================================================
 *                   integrate_states_matrix_exp
 *
 * Use matrix exponentiation to compute latent state vector at all subsequent
 * time points, given the initial state.
 *
 * -----------------------------------------------------------------------------
 */

void kjb::ties::integrate_states_matrix_exp
(
#ifdef KJB_HAVE_TBB
    std::vector<Coupled_oscillator, 
        tbb::scalable_allocator<Coupled_oscillator> >& clos,
#else
    std::vector<Coupled_oscillator>& clos, 
#endif
    double init_time,
    const Double_v& times,
    const State_type& init_state,
    State_vec& states, 
    size_t start_index,
    bool drift,
    const std::vector<size_t>& indices
)
{
    ASSERT(!clos.empty());
    if(clos.front().use_modal())
    {
        BOOST_FOREACH(Coupled_oscillator& clo, clos)
        {
            clo.update_system_matrix_from_modal();
        }
    }
    size_t N = clos.size(); 
    IFT(start_index <= N, Illegal_argument, 
            "start_index is greater than the size of CLO params.");
    IFT(times.size() <= N + 1 && times.size() >= 1, 
            Illegal_argument,
            "times and clos are wrong");
    // if the start_index is at the last index of params, none of the states
    // are needed to be updated
    if(start_index == N) return;
    // make the states dimension to be the same as the times
    states.resize(times.size());

    // index of the last element
    size_t end_index = indices.empty() ? times.size() : indices.size();
    if(times[0] == init_time)
    {
        states[0] = init_state;
        start_index++;
        ASSERT(end_index > 0);
        end_index--;
    }

    if(!indices.empty())
    {
        ASSERT(start_index < indices.size() - 1);
    }
    size_t cur_index = indices.empty() ? start_index : indices[start_index];
    int prev_index = cur_index - 1;
    ASSERT(prev_index >= 0 && prev_index <= clos.size() - 1);
    State_type prev_state = states[prev_index]; 
    Matrix A = clos[prev_index].system_matrix();
    Matrix eA; 
    // Since currently the consecutive time points are equally spaced 
    double delta_t = times[cur_index] - times[prev_index]; 
    A *= delta_t;
    gsl_matrix_exponential(A, eA);
    for(size_t i = start_index; i <= end_index; i++)
    {
        // update the state
        states[cur_index] = eA * prev_state;
        // update 
        prev_index++;
        cur_index++;
        prev_state = states[prev_index]; 
        if(prev_index == end_index) break;

        // check delta time 
        delta_t = times[cur_index] - times[prev_index];
        if(indices.empty())
        {
            ASSERT(fabs(delta_t - 1.0) < FLT_EPSILON);
        }
        if(drift || !indices.empty())
        {
            A = clos[prev_index].system_matrix() * delta_t;
            gsl_matrix_exponential(A, eA);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::integrate_states_modal
(
#ifdef KJB_HAVE_TBB
    std::vector<Coupled_oscillator, 
        tbb::scalable_allocator<Coupled_oscillator> >& clos,
#else
    std::vector<Coupled_oscillator>& clos, 
#endif
    double init_time,
    const Double_v& times,
    const State_type& init_state,
    State_vec& states, 
    size_t start_index,
    bool drift,
    const std::vector<size_t>& indices
)
{
    ASSERT(!clos.empty());
    size_t N = clos.size(); 
    IFT(start_index <= N, Illegal_argument, 
            "start_index is greater than the size of CLO params.");
    IFT(times.size() <= N + 1 && times.size() >= 1, 
            Illegal_argument,
            "times and clos are wrong");
    // if the start_index is at the last index of params, none of the states
    // are needed to be updated
    if(start_index == N) return;
    // make the states dimension to be the same as the times
    states.resize(times.size());

    // index of the last element
    size_t end_index = indices.empty() ? times.size() : indices.size();
    if(times[0] == init_time)
    {
        states[0] = init_state;
        start_index++;
        ASSERT(end_index > 0);
        end_index--;
    }

    size_t cur_index = indices.empty() ? start_index : indices[start_index];
    int prev_index = cur_index - 1;
    ASSERT(prev_index >= 0 && prev_index < clos.size() - 1);

    int num_osc = init_state.size() / 2;
    try
    {
        clos[prev_index].update_modal_representation();
    }
    catch (Exception& e)
    {
        std::cerr << e.get_msg() << " kjb error failed compute states\n";
    }
    // transferm the real physics coordinate to modal coordinate 
    // "false" represent from real physics coordinate to modal coordinate
    State_type prev_state = clos[prev_index].get_state(states[prev_index], false); 

    double delta_t = times[cur_index] - times[prev_index]; 
    // Since currently the consecutive time points are equally spaced 
    size_t clo_index = prev_index;
    bool use_cache = drift ? false : true;
    for(size_t i = start_index; i <= end_index; i++)
    {
        State_type next_modal_state = clos[clo_index].get_modal_state
                            (prev_state, delta_t, use_cache);
        states[cur_index] = clos[clo_index].get_state(next_modal_state, true);
        prev_state = next_modal_state;
        prev_index++;
        cur_index++;
        if(prev_index == end_index) break;

        // check delta time 
        delta_t = times[cur_index] - times[prev_index];
        if(indices.empty())
        {
            ASSERT(fabs(delta_t - 1.0) < FLT_EPSILON);
        }
        if(drift || !indices.empty())
        { 
            clo_index = prev_index;
            try
            {
                clos[clo_index].update_modal_representation();
            }
            catch (Exception& e)
            {
                std::cerr << e.get_msg() << " kjb error compute states\n";
            }

        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::integrate_states
(
#ifdef KJB_HAVE_TBB
    std::vector<Coupled_oscillator, 
        tbb::scalable_allocator<Coupled_oscillator> >& clos,
#else
    std::vector<Coupled_oscillator>& clos, 
#endif
    double init_time,
    const Double_v& times,
    const State_type& init_state,
    State_vec& states, 
    size_t start_index,
    bool drift,
    const std::vector<size_t>& indices
)
{
    if(clos.empty()) return;
    bool use_modal = clos.front().use_modal();
    if(use_modal)
    {
        integrate_states_modal(clos, init_time, times, 
                            init_state, states, start_index, drift, indices);
    }
    else
    {
        integrate_states_matrix_exp(clos, init_time, times, 
                            init_state, states, start_index, drift, indices);
    }

}
