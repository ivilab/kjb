/* $Id: sample_adaptive_mh.h 12839 2012-08-08 23:51:05Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <sample_cpp/sample_vector.h>
#include <sample_cpp/sample_base.h>

template <class SimpleVector>
class Simple_adaptive_mh_step
{
private:
    typedef Simple_adaptive_mh_step Self;
    typedef Basic_mh_step<SimpleVector> Base_step;
public:
    typedef typename Base_step::Target_distribution Target_distribution;
    typedef Mv_gaussian_proposer<SimpleVector> Proposer;

    Simple_adaptive_mh_step(
            const Target_distribution& target,
            const kjb::Matrix& initial_covariance,
            double goal_accept_rate) :
        goal_accept_prob_(goal_accept_rate),
        i_(2), // first i_ comes from initial step
        gamma_(),
        mu_(),
        proposer_(new Proposer(initial_covariance)),
        post_callback_(),
        step_(target, boost::bind<Mh_proposal_result>(&Proposer::operator(), proposer_, _1, _2))
    {
        // a relatively conservative choice
        set_inverse_learning_rate(1, 0.5);
    }

    void set_temperature(double t)
    {
        proposer_->set_temperature(t);
        step_.set_temperature(t);
    }

    /**
     * Let \gamma_i be the adaptation learning rate at iteration i.
     * Calling this function will define the sequence of \gamma_i's to be:
     *  
     *    \gamma_i = C * i^{-\alpha}
     * Where \alpha >= 0.  Recommended values: C<=1, alpha >= 0.5.
     *
     * Smaller C corresponds to greater confidence in the initial
     * covariance matrix.  Increasing alpha corresponds to faster vanishing
     * of the adaptation term, i.e. adaptation will "stop" sooner.
     *
     * Setting alpha >= 0.5 will ensure that the rate of convergence of the Markov
     * Chain is unaffected by adaptation.  See (Andrieu and Thoms, 2008) for details.  
     */
    void set_inverse_learning_rate(double C, double alpha)
    {
        gamma_ = boost::bind(Self::inverse_i_, log(C), alpha, _1);
    }

    /**
     * Use constant learning rate, \gamma.  When iteration exceeds change_point,
     * adaptation ceases to occur. 
     *
     * Setting change_point to zero causes adaptation
     * to occur indefinitely, but the markov chain will no longer converge to the 
     * target distribution.  In some situations this is okay, because the convergent
     * distribution is a reasonable approximation to the target distribution, but use
     * with caution.
     */
    void set_constant_learning_rate(double gamma, size_t change_point = 0)
    {
        if(change_point == 0)
            gamma_ = boost::bind(Self::constant_function_, gamma, _1);
        else
            gamma_ = boost::bind(Self::step_function_, gamma, change_point, _1);
    }

    Step_log<SimpleVector> operator()(SimpleVector& in, double lt_m)
    {
        SimpleVector previous_state = in;

        double accept_prob;
        SimpleVector proposed_state;

        Step_log<SimpleVector> step_log = step_.run_step(in, lt_m, accept_prob, proposed_state);
        accept_prob = std::min(0.0, accept_prob);
        accept_prob = exp(accept_prob);
        adapt(accept_prob, previous_state, proposed_state, in);

        if(post_callback_) post_callback_(*this);
        return step_log;
    }

    /**
     * get cholesky decomposition of covariance matrix _before_ incorporating global scale.  This should be multiplied by the global scale to get the actual covariance
     */
    const kjb::Matrix& get_cholesky_covariance() const
    {
        return proposer_->get_unscaled_cholesky_covariance();
    }

    /**
     * get global scaling constant
     */
    double get_global_scale() const
    {
        return proposer_->get_global_scale();
    }

    virtual void adapt(double accept_prob, const SimpleVector& previous_state, const SimpleVector& proposed_state, const SimpleVector& accepted_state)
    {

        /// update global scale
        double gamma = gamma_(i_);
        proposer_->adapt_global_scale(gamma * (accept_prob - goal_accept_prob_));

        // update mean
        // mu_ += gamma * delta;
        if(mu_.size() == 0)
            mu_ = previous_state;

        SimpleVector delta = accepted_state;
        std::transform(delta.begin(), delta.end(), mu_.begin(), delta.begin(), std::minus<double>());

        // update covariance
        proposer_->adapt_covariance(delta, gamma);

        std::transform(delta.begin(), delta.end(), delta.begin(), std::bind2nd(std::multiplies<double>(), gamma));
        std::transform(mu_.begin(), mu_.end(), delta.begin(), mu_.begin(), std::plus<double>());

        ++i_;
    }

    /**
     * Set a function that will be called at the end of every iteration.  
     * Useful for logging/debugging of adaptation
     */
    void set_post_callback(const boost::function1<void, const Self&>& cb)
    {
        post_callback_ = cb;
    }

    /// returns the current learning rate (for debugging)
    double get_current_gamma() const
    {
        return gamma_(i_);
    }

    /// returns the log of the current scaling lambda
    double get_log_lambda() const
    {
        return log_lambda_;
    }
protected:
    void set_target(const Target_distribution& t)
    {
        step_.set_target(t);
    }
private:
    /**
     * implements a gamma sequence of:
     *    \gamma_i = C * i^{-\alpha}
     * Where \alpha \in (0,1].  Recommended values: C=1, alpha=1.
     */
    static double inverse_i_(double log_C, double alpha, double i)
    {
        return exp( log_C - alpha * log(i));
    }

    /**
     * implements a step-function f(i):
     *  
     *  f(i) = f ; if i < change_point
     *  f(i) = 0        ; otherwise
     */
    static double step_function_(double v, size_t change_point, size_t i)
    {
        if(i < change_point) return v;
        return 0.0;
    }

    /**
     * implements a constant function f(i) = v
     */
    static double constant_function_(double f, size_t)
    {
        return f;
    }


    double goal_accept_prob_;
    double log_lambda_;
    size_t i_;
    boost::function1<double, size_t> gamma_;

    SimpleVector mu_;
    boost::shared_ptr<Mv_gaussian_proposer<SimpleVector> > proposer_;

    boost::function1<void, const Self&> post_callback_;
    mutable Base_step step_;
};


template <class Model>
class Generic_adaptive_mh_step : public Simple_adaptive_mh_step<kjb::Vector>
{
public:
    typedef typename Model_evaluator<Model>::Type Target_distribution;
private:
    typedef boost::function2<void, const Model&, kjb::Vector&> To_vector;
    typedef boost::function2<void, const kjb::Vector&, Model&> From_vector;

    typedef Simple_adaptive_mh_step<kjb::Vector> Base_step;

    struct target_wrapper
    {
        target_wrapper(
                const Target_distribution& target_,
                const From_vector& from_vector_) :
            target(target_)
            ,from_vector(from_vector_)
            ,model()
#ifdef TEST
            ,called(false)
#endif
        {}

        double operator()(const kjb::Vector& v) const
        {
            from_vector(v, model);
#ifdef TEST
            called = true;
#endif
            return target(model);
        }

        Target_distribution target;
        From_vector from_vector;
        mutable Model model;
#ifdef TEST
        mutable bool called;
#endif
    };
        

            
public:
    typedef Base_step::Proposer Proposer;

    Generic_adaptive_mh_step(
            const To_vector& to_vector,
            const From_vector& from_vector,
            const Target_distribution& target,
            const kjb::Matrix& initial_covariance,
            double goal_accept_rate) :
        Base_step(boost::ref(target_wrapper_), initial_covariance, goal_accept_rate),
        target_wrapper_(target, from_vector),
        to_vector_(to_vector)
    {}

    /**
     * Copy constructor.  This ensures that the new base step's
     * target distribution links to this new object's target_wrapper,
     * not the original object's
     */
    Generic_adaptive_mh_step(const Generic_adaptive_mh_step& other) :
        Base_step(other),
        target_wrapper_(other.target_wrapper_),
        to_vector_(other.to_vector_)
    {
        Base_step::set_target(boost::ref(target_wrapper_));
    }


    Step_log<Model> operator()(Model& in, double lt_m)
    {
        target_wrapper_.model = in;
#ifdef TEST
        target_wrapper_.called = false;
#endif

        kjb::Vector v;
        to_vector_(in, v);
        Step_log<Model> log = Base_step::operator()(v, lt_m);

        assert(log.size() == 1);
#ifdef TEST
        assert(target_wrapper_.called == true);
#endif

        if(log.back().accept)
        {
            in = target_wrapper_.model;
        }

        return log;
    }

private:
    target_wrapper target_wrapper_;
    To_vector to_vector_;
};

