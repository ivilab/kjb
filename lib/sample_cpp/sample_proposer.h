/* $Id: sample_proposer.h 17393 2014-08-23 20:19:14Z predoehl $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek, Ernesto Brau
 * =========================================================================== */

#ifndef SAMPLE_PROPOSER_H_INCLUDED
#define SAMPLE_PROPOSER_H_INCLUDED

#include "sample_cpp/sample_concept.h"
#include "sample_cpp/sample_base.h"
#include "sample_cpp/sample_default.h"
#include "sample_cpp/sample_vector.h"
#include "prob_cpp/prob_conditional_distribution.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_sample.h"
#include "prob_cpp/prob_pdf.h"
#include "l_cpp/l_exception.h"


/* =================================================================================== *
 * USEFUL, GENERIC METROPOLIS-HASTINGS PROPOSERS                                       *
 * =================================================================================== */

/**
 * @class Conditional_distribution_proposer
 *
 * @tparam ConditionalDistribution A conditional distribution type.
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * Conditional_distribution_proposer is a proposer that proposes models
 * based on a conditional distribution.
 *
 * This class constructs a proposer object from a conditional distribution object.
 * In other words, given a conditional distribution q, it constructs a proposer
 * object that proposes m' ~ q(m' | m), where m is of type Model. This
 * class is meant to be used with the standard distributions (in distributions.h),
 * but can be used with any type that conforms to the conditional distribution
 * concept.
 */
template<typename ConditionalDistribution, typename Model>
class Conditional_distribution_proposer
{
public:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    Conditional_distribution_proposer(const ConditionalDistribution& cond_dist) :
        cd(cond_dist)
    {}

    /**
     * @brief Proposes new model
     *
     * See Mh_model_proposer concept for more information
     */
    Mh_proposal_result operator()(const Model& m, Model& mp) const
    {
        mp = conditional_sample(cd, m);
        double fwd = conditional_log_pdf(cd, mp, m);
        double rev = conditional_log_pdf(cd, m, mp);

        return Mh_proposal_result(fwd, rev);
    }

private:
    ConditionalDistribution cd;
};


/**
 * @class Multi_proposer_proposer
 *
 * @tparam Model The model type. Must comply with BaseModel concept.
 *
 * Multi_proposer_proposer is a proposer that proposes models by u.a.r.
 * choosing a proposer from a set of proposers at each iteration. Naturally,
 * the proposers of the proposer vector must be of the same model type.
 *
 */

template<typename Model>
class Multi_proposer_proposer
{
private:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

    typedef typename Mh_model_proposer<Model>::Type Proposer;

public:
    typedef std::vector<Proposer> Proposer_vector;

private:
    Proposer_vector m_proposers;
    kjb::Categorical_distribution<int> m_dist;

public:
    /**
     * @brief Constructs a Multi_proposer_proposer with a
     * uniform distribution
     */
    Multi_proposer_proposer(const Proposer_vector& proposers) :
        m_proposers(proposers),
        m_dist(0, proposers.size() - 1, 1)
    {}

    /**
     * @brief Constructs a Multi_proposer_proposer with the
     * given proposers and their distribution.
     */
    Multi_proposer_proposer(const Proposer_vector& proposers, const kjb::Categorical_distribution<int>& dist) :
        m_proposers(proposers),
        m_dist(dist)
    {
        if(m_proposers.size() != m_dist.size())
        {
            KJB_THROW_2(kjb::Illegal_argument, "Multi_proposer_proposer:"
                                               "distribution and proposer vector must have same size.");
        }
    }

    /**
     * @brief Proposes new model
     *
     * See Mh_model_proposer concept for more information
     */
    Mh_proposal_result operator()(const Model &m, Model& m_p)
    {
        //int num_prop = m_proposers.size();
        double fwd;
        double rev;
        bool keep_going = true;

        while(keep_going)
        {
            int j = kjb::sample(m_dist);

            Mh_proposal_result res = m_proposers[j](m, m_p);
            if(res.no_change)
            {
                fwd = res.fwd_prob + kjb::log_pdf(m_dist, j);
                rev = res.rev_prob + kjb::log_pdf(m_dist, j);
                keep_going = false;
            }
        }

        return Mh_proposal_result(fwd, rev);
    }
};

/**
 * @class Single_dimension_proposer
 *
 * @tparam Model The model type.  Must comply with BaseModel concept.
 *
 * Single_dimension_proposer is a proposer that proposes models by changing
 * a single dimension -- chosen according to a distribution -- of the given model. It receives
 * a vector of proposers, one for each dimension of the model. At each iteration
 * it chooses a dimension at random and proposes that dimension of the new model
 * using the corresponding proposer. The rest of the proposed model is the
 * same as the given model. Clearly, the number of proposers must be equal
 * to the dimension of the model.
 *
 * There is one exception to this: if given only ONE proposer,
 * Single_dimension_proposer will use that proposer no matter what dimension
 * of the model was chosen.
 *
 * Note that the user must provide a parameter setter and a parameter getter;
 * otherwise, vector getters and setters are used.
 *
 */

template<typename Model>
class Single_dimension_proposer
{
private:
    BOOST_CONCEPT_ASSERT((BaseModel<Model>));

public:
    typedef typename Mh_model_proposer<double>::Type Proposer;
    typedef typename Get_model_parameter<Model>::Type Get_param;
    typedef typename Set_model_parameter<Model>::Type Set_param;
    typedef typename Model_dimension<Model>::Type Dimension;

    typedef std::vector<Proposer> Proposer_vector;

private:
    Proposer_vector m_proposers;
    kjb::Categorical_distribution<int> m_dist;
    Get_param get_p;
    Set_param set_p;
    Dimension m_dim;
    mutable int last_modified_dim;

public:
    /**
     * @brief Constructs a Single_dimension_proposer with a
     * uniform distribution
     */
    Single_dimension_proposer
    (
        const Proposer_vector& proposers,
        const Get_param& get_param = get_vector_model_parameter<Model>,
        const Set_param& set_param = set_vector_model_parameter<Model>,
        const Dimension& model_dimension = get_vector_model_dimension<Model>
    ) :
        m_proposers(proposers),
        m_dist(0, proposers.size() - 1, 1),
        get_p(get_param),
        set_p(set_param),
        m_dim(model_dimension),
        last_modified_dim(-1)
    {}

    /**
     * @brief Constructs a Single_dimension_proposer with the
     * given proposers and their distribution.
     */
    Single_dimension_proposer
    (
        const Proposer_vector& proposers,
        const kjb::Categorical_distribution<int>& dist,
        const Get_param& get_param = get_vector_model_parameter<Model>,
        const Set_param& set_param = set_vector_model_parameter<Model>,
        const Dimension& model_dimension = get_vector_model_dimension<Model>
    ) :
        m_proposers(proposers),
        m_dist(dist),
        get_p(get_param),
        set_p(set_param),
        m_dim(model_dimension),
        last_modified_dim(-1)
    {
        if(m_proposers.size() != 1 && m_proposers.size() != m_dist.size())
        {
            KJB_THROW_2(kjb::Illegal_argument, 
                "Single_dimension_proposer: distribution and proposer vector "
                "must have same size.");
        }
    }

    /**
     * @brief Proposes new model
     *
     * See Mh_model_proposer concept for more information
     */
    Mh_proposal_result operator()(const Model &m, Model& m_p) const
    {
        int D = m_dim(m);
        int num_prop = m_proposers.size();

        if(num_prop != 1)
        {
            if(D != num_prop)
            {
                KJB_THROW_2(kjb::Illegal_argument,
                    "Single_dimension_proposer: model dimension must be "
                    "equal to number of proposers.");
            }
        }
        else
        {
            if(D != m_dist.size())
            {
                KJB_THROW_2(kjb::Illegal_argument,
                    "Single_dimension_proposer: model dimension and "
                    "distribution size must be equal.");
            }
        }

        m_p = m;
        int j = kjb::sample(m_dist);
        int k = (D == num_prop) ? j : 0;

        //Mh_proposal_result res = m_proposers[k](m[j], m_p[j]);
        double m_j = get_p(m, j);
        double m_p_j;
        Mh_proposal_result res = m_proposers[k](m_j, m_p_j);
        set_p(m_p, j, m_p_j);

        double fwd = res.fwd_prob + kjb::log_pdf(m_dist, j);
        double rev = res.rev_prob + kjb::log_pdf(m_dist, j);

        last_modified_dim = j;

        return Mh_proposal_result(fwd, rev);
    }

    /** @brief  Returns the most recently changed dimension of the model. */
    int get_last_modified_dimension() const
    {
        IFT(last_modified_dim != -1, kjb::KJB_error,
            "Single dimension proposer: cannot get latest modified dimension; "
            "proposer has not been called yet.");

        return last_modified_dim;
    }
};

/**
 * @class Gaussian_random_walk_proposer
 *
 * Simple single-dimensional Gaussian random-walk proposer. Given a value x,
 * this object will propose x' ~ N(x, s), where s is provided at construction.
 */

class Gaussian_random_walk_proposer
{
private:
    double sigma;

public:
    /** @brief  Construct a GRWP. */
    Gaussian_random_walk_proposer(double variance) : sigma(variance) {}

    /** @brief  Propose a new value. */
    Mh_proposal_result operator()(const double& m, double& mp) const
    {
        kjb::Normal_distribution N(m, sqrt(sigma));
        mp = kjb::sample(N);
        double p = kjb::pdf(N, mp);

        return Mh_proposal_result(p, p);
    }
};

#endif /*SAMPLE_PROPOSER_H_INCLUDED */

