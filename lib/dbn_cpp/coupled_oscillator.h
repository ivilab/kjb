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

/* $Id: coupled_oscillator.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_COUPLED_OSCILLATOR_H_
#define KJB_TIES_COUPLED_OSCILLATOR_H_

#include <m_cpp/m_vector.h>
#include <boost/numeric/odeint.hpp>
#include <gsl_cpp/gsl_linalg.h>

#include "dbn_cpp/typedefs.h"

namespace kjb {
namespace ties {

static const double DEFAULT_PERIOD = 5.0;
static const double DEFAULT_DAMPING= 0.0;

/**
 * @brief   Return the number of params for n coupled oscillators
 */
inline size_t param_length
(
    size_t num_oscillators,
    size_t use_modal = false
)
{
    // 2 * n + n * (n - 1)
    if(use_modal)
    {
        return num_oscillators * (1 + num_oscillators);
    }
    else
    {
        return num_oscillators * (2 + num_oscillators);
    }
}

/**
 * @brief   Return the number of oscillators based on the number of params.
 */
inline size_t num_oscillators_from_params
(
    const Double_v& params
)
{
    return 2;
}

Double_v coupled_oscillator_params
(
    size_t num_oscillators = 2,
    double period = DEFAULT_PERIOD,
    double damping = DEFAULT_DAMPING,
    bool use_modal = false,
    bool random = false
);

/**
 * @brief   A coupled oscillator model
 *
 *   d(x)              d(x)      d(x)
 * M ----- = - K x - C ----  -D -----
 *   dtdt               dt       d(t)
 *
 *   M = diag(m1, m2, ..., mn) 
 *   D = diag(d1, d2, ..., dn)
 *   K = | k1 + k2  -k2   0            |
 *       |-k2   k2 + k3  -k3           |
 *       | 0   -k3   k3 + k4           |
 *       |     .      .    .           | 
 *       |       .      .    .         | 
 *       |        .      .    .    -kn | 
 *       |              -kn  kn + kn+1 |   
 *
 *   C = | c1 + c2  -c2   0            |
 *       |-c2   c2 + c3  -c3           |
 *       | 0   -c3   c3 + c4           |
 *       |     .      .    .           | 
 *       |       .      .    .         | 
 *       |        .      .    .    -cn | 
 *       |              -cn  cn + cn+1 |   
 */
class Coupled_oscillator
{
    public:
        /**
        * @brief   Default constructor for Coupled_oscillator. 
        */
        Coupled_oscillator
        (
            size_t num_oscillators = 2,
            double period = DEFAULT_PERIOD,
            double damping = DEFAULT_DAMPING,
            bool use_modal = false,
            bool random = false
        ) : 
            n_(num_oscillators),
            params_(coupled_oscillator_params
                        (num_oscillators, 
                        period,
                        damping,
                        use_modal,
                        random)),
            damping_ratio_squared_((int)n_, 0.0),
            damping_ratio_cache_((int)n_, 0.0),
            w_n_((int)n_, 0.0),
            w_d_((int)n_, 0.0),
            modal_matrix_((int)n_, (int)n_, 0.0),
            M_((int)n_,(int) n_, 0.0),
            cos_psi_val_((int)n_, 0.0),
            sin_psi_val_((int)n_, 0.0),
            cos_val_((int)n_, 0.0),
            sin_val_((int)n_, 0.0),
            damping_val_((int)n_, 0.0),
            damping_val_exp_((int)n_, 0.0),
            cache_dirty_(true),
            system_matrix_dirty_(true),
            modal_rep_dirty_(true),
            use_modal_(use_modal)
        {}

        /**
        * @brief   Construct a new Coupled_oscillator from params
        */
        Coupled_oscillator
        (
            const Double_v& params,
            bool use_modal = false
        ) : 
            params_(params),
            n_(num_oscillators_from_params(params)),
            damping_ratio_squared_((int)n_, 0.0),
            damping_ratio_cache_((int)n_, 0.0),
            w_n_((int)n_, 0.0),
            w_d_((int)n_, 0.0),
            modal_matrix_((int)n_, (int)n_, 0.0),
            M_((int)n_, (int)n_, 0.0),
            cos_psi_val_((int)n_, 0.0),
            sin_psi_val_((int)n_, 0.0),
            cos_val_((int)n_, 0.0),
            sin_val_((int)n_, 0.0),
            damping_val_((int)n_, 0.0),
            damping_val_exp_((int)n_, 0.0),
            cache_dirty_(true),
            system_matrix_dirty_(true),
            modal_rep_dirty_(true),
            use_modal_(use_modal)
        {}

        /**
        * @brief   Return the number of oscillators.
        */
        size_t num_oscillator() const 
        {
            return n_;
        }

        /**
        * @brief   Return the size of the params. 
        */
        size_t num_params() const
        {
            return params_.size();
            //DEBUG TODO exclude the couple terms
            //return params_.size() - 2;
        }

        /**
        * @brief   Return the params of the coupled oscillators. 
        */
        const Double_v& params() const { return params_; } 

        /** 
        * @brief   Computes the derivative
        */
        void operator()
        (
            const State_type& x, 
            State_type& dxdt, 
            const double /* t */
        ) const;

        /**
        * @brief   Return the system matrix
        */
        const Matrix& system_matrix()
        { 
            update_system_matrix();
            return A_; 
        }

        void update_system_matrix_from_modal();

        /**
        * @brief   Return the number of oscillators.
        */
        size_t num_oscillators() const 
        {
            return n_;
        }

        /**
        * @brief   Update system matrix 
        *
        *  For example, when n_ = 3
        *  A = | 0          0       0        1   0   0 |
        *      | 0          0       0        0   1   0 |
        *      | 0          0       0        0   0   1 |
        *      |f1-c11-c12 c11      c12      d1  0   0 |
        *      |c21    f2-c21-c22   c22      0   d2  0 |
        *      |c31        c32  f3-c31-c32   0   0   d3|
        */
        void update_system_matrix();

        void update_modal_representation();

        /** 
        * @brief  Update the eigen vectors e'*M * e = 1
        *         we represent e(:,1) = [r1 * cos(theta_1), r1 * sin(theta_1)';
        *                      e(:,2) = [r2 * cos(theta_2), r1 * sin(theta_2)';   
        *                      M = [alpha 0; 0 1];
        */
        void update_modal_eigen_vector(const Double_v& params);

        /** 
        * @brief   Update the natual frequency w_n_ and the damping frequency w_d_
        */
        void update_damping_frequency(const Double_v& params);

        /**
        * @brief   Update system matrix for the given index of value param
        */
        void update_system_matrix(size_t index, double new_val);

        /**
        * @brief   Update system matrix for the given index of value param
        */
        void update_modal_representation(size_t index, double new_val);

        /** 
        * @brief   Set the param value at the specificed index 
        */
        void set_param(size_t index, double new_val);

        /**
        * @brief   Return the param value at the specified index
        */
        double get_param(size_t index) const
        {
            IFT(index < params_.size(), Illegal_argument, "Index out of bound ");
            return params_[index];
        }
        const Double_v& get_params() const { return params_; } 

        /** @brief   Return the const begin iterator */
        Double_v::const_iterator cbegin() const { return params_.begin(); }

        /** @brief   Return the const end iterator */
        Double_v::const_iterator cend() const { return params_.end(); }

        /** @brief   Return the begin iterator */
        Double_v::iterator end() { return params_.end(); }

        /** @brief   Return the end iterator */
        Double_v::iterator begin() { return params_.begin(); }

        /** @brief   Return the modal matrix */
        const Matrix& modal_matrix() const 
        { 
            assert(!modal_rep_dirty_);
            return modal_matrix_; 
        }

        /** @brief   Return the inverse of the modal matrix */
        const Matrix& modal_matrix_inv() const 
        { 
            assert(!modal_rep_dirty_);
            return modal_matrix_inv_; 
        }

        /** @brief    Return the state in modal coordinate */
        State_type get_modal_state
        (
            const State_type& initial_state, 
            double time, 
            bool use_cached_time = false
        );

        /** @brief   Return the state in real physics coordinate. */
        State_type get_state(const State_type& modal_state, bool modal_to_real) const;

        /** @brief   Return true if use modal representation. */
        bool use_modal() const { return use_modal_; }

    private:
        // member 
        mutable Double_v params_;
        std::map<size_t, size_t> coupling_coef_map;
        size_t n_;
        Vector damping_ratio_squared_;
        Vector damping_ratio_cache_;
        Vector w_n_;
        Vector w_d_;
        Matrix modal_matrix_;
        Matrix modal_matrix_inv_;
        Matrix A_;
        Matrix M_; // stiffness matrix
        Matrix K_; // stiffness matrix
        Matrix D_; // damping matrix
        mutable Vector cos_psi_val_;
        mutable Vector sin_psi_val_;
        mutable Vector cos_val_;
        mutable Vector sin_val_;
        mutable Vector damping_val_;
        mutable Vector damping_val_exp_;
        mutable bool cache_dirty_;
        mutable bool system_matrix_dirty_;
        mutable bool modal_rep_dirty_;
        bool use_modal_;
};

/**
 * @brief   Output the parameters into ostream 
 */
inline std::ostream& operator<<(std::ostream& out, const Coupled_oscillator& co)
{
    // Test program was HERE.
    std::streamsize w = out.width();
    std::streamsize p = out.precision();
    std::ios::fmtflags f = out.flags();

    out << std::scientific;
    // TODO debug
    //for(int i = 0; i < co.num_params(); i++)
    for(int i = 0; i < co.get_params().size(); i++)
    {
        out << std::setw(16) << std::setprecision(8) << co.get_param(i);
    }

    out.width( w );
    out.precision( p );
    out.flags( f );
    return out;
}

/** @brief   Records the ODE integrator logs. */
struct Integrator_observer
{
    State_vec& m_states;
    std::vector<double>& m_times;

    Integrator_observer
    (
        State_vec& states, 
        std::vector<double>& times 
    ) : m_states(states), m_times(times) 
    {}

    void operator()(const State_type &x, double t)
    {
        m_states.push_back(x);
        m_times.push_back(t);
    }
};

/** 
 * @brief   Return the states by integrating out the coupled oscillator ODE
 * @param   params          The parameters of the coupled oscillator
 * @param   states          The resulting states 
 * @param   dt              Delta t in derivative  
 * @param   start_index     Starting index in params 
 */
/*void integrate_states
(
#ifdef KJB_HAVE_TBB
std::vector<Coupled_oscillator, 
        tbb::scalable_allocator<Coupled_oscillator> > clos,
#else
    const std::vector<Coupled_oscillator>& clos, 
#endif
    State_vec& states, 
    double dt = 0.01,
    size_t start_index = 0
);*/

/** 
 * @brief   Return the states by integrating out the coupled oscillator ODE
 * @param   clos            The coupled oscillators at each time point
 * @param   init_state      The state value at initial time 0 
 * @param   states          The resulting states 
 * @param   start_index     Starting index in params 
 * @param   drift           Flag indiciating parameter drifts 
 */
void integrate_states_matrix_exp
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
    size_t start_index = 0,
    bool drift = false,
    const std::vector<size_t>& indices = std::vector<size_t>()
);

/** 
 * @brief   Return the states by integrating out the coupled oscillator 
 *          using matrix exponiential
 * @param   A               Matrix A 
 * @param   init_state      state value at time 0 
 * @param   time            the time of the return state 
 */
inline
State_type get_state_matrix_exp
(
    const Matrix& A,
    const State_type& init_state,
    double time
)
{
    IFT(time >= 0, Illegal_argument, 
            "Time need to be positive.");

    Matrix eA;
    gsl_matrix_exponential(A*time, eA);
    std::cout << "eA: " << eA << std::endl;

    Vector current_v(init_state.begin(), init_state.end());
    Vector next_state = eA * current_v;
    return State_type(next_state.begin(), next_state.end()); 
}

void integrate_states_modal
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
    size_t start_index = 0,
    bool drift = false,
    const std::vector<size_t>& indices = std::vector<size_t>()
);

void integrate_states
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
    size_t start_index = 0,
    bool drift = false,
    const std::vector<size_t>& indices = std::vector<size_t>()
);


}}

#endif
