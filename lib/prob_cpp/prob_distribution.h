/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Kyle Simek, Ernesto Brau.                           |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: prob_distribution.h 21776 2017-09-17 16:44:49Z clayton $ */

#ifndef PROB_DISTRIBUTION_H_INCLUDED
#define PROB_DISTRIBUTION_H_INCLUDED

/** @file
 *
 * @author Kobus Barnard
 * @author Ernesto Brau
 *
 * @brief Definition of various standard probability distributions.
 *
 * When we can, we are using the distributions from boost::math and typedef-ing
 * them to fit our own naming scheme. See boost's documentation for help
 * on how these work.
 *
 * In the multivariate distribution classes, the random vectors are represented
 * by kjb::Vector's. In the future, we would like to template-out this type
 * and make these distributions over any type that implements the basic vector
 * (of a vector space) functionality.
 */

#include <boost/version.hpp>
#include <boost/math/distributions.hpp>
#include <boost/math/distributions/inverse_gamma.hpp>
#include <boost/math/distributions/inverse_chi_squared.hpp>
#include <boost/math/distributions/negative_binomial.hpp>
#include <map>
#include <cmath>
#include <vector>
#include "prob_cpp/prob_util.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_vector_d.h"
#include "m_cpp/m_matrix.h"
#include "n_cpp/n_cholesky.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_std_parallel.h"

#include "g/g_area.h"
#include "g_cpp/g_quaternion.h"

namespace kjb{

/*============================================================================
  These distributions are part of the boost-math library, defined in
  boost/math/distributions.hpp. See boost's documentation for details.
  They are typdef'd to match the naming scheme of the KJB
  ----------------------------------------------------------------------------*/

typedef boost::math::bernoulli              Bernoulli_distribution;
typedef boost::math::beta_distribution<>    Beta_distribution;
typedef boost::math::binomial               Binomial_distribution;
typedef boost::math::chi_squared            Chi_square_distribution;
typedef boost::math::exponential            Exponential_distribution;
typedef boost::math::gamma_distribution<>   Gamma_distribution;
typedef boost::math::normal                 Gaussian_distribution;
typedef boost::math::laplace                Laplace_distribution;
typedef boost::math::normal                 Normal_distribution;
typedef boost::math::poisson                Poisson_distribution;
typedef boost::math::uniform                Uniform_distribution;
typedef boost::math::inverse_gamma_distribution<>   Inverse_gamma_distribution;
typedef boost::math::inverse_chi_squared_distribution<>  Inverse_chi_squared_distribution;

// standard normal distribution
extern const Gaussian_distribution STD_NORMAL;

#if BOOST_VERSION >= 104600
typedef boost::math::geometric_distribution<double> Geometric_distribution;
#else
class Geometric_distribution : public boost::math::negative_binomial
{
public:
    Geometric_distribution(double p) : 
        boost::math::negative_binomial(1, p)
    { }
};
#endif

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
class Beta_binomial_distribution
{
public:
    Beta_binomial_distribution(size_t n_, double alpha_, double beta_) : 
        n(n_), alpha(alpha_), beta(beta_)
    { }

    double n, alpha, beta;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/**
 * @brief   Generic traits struct for all distributions.
 *
 * This is the default traits struct for the distribution classes.
 * For now, it only contains the type of variable it is, and the
 * default is double.
 */
template<class Distribution>
struct Distribution_traits
{
    typedef double type;
};

/*============================================================================
  The following are univariate distributions that are not present in the
  boost library.
  ----------------------------------------------------------------------------*/

/**
 * @brief A categorical distribution.
 
 * This class implements a categorical distribution. This is a discrete
 * probability distribution that assigns a probability to each of a
 * finite number of numbers. It stores an STL map that relates values
 * with probabilities.
 *
 * @tparam  T   The type over which the distribution is over. Must be
 *              be a RealType (i.e., a field).
 */
template<class T = int>
class Categorical_distribution
{
    typedef Categorical_distribution Self;
    typedef std::pair<const T*, double> Cdf_pair;
    typedef typename std::map<T, double>::value_type Map_pair;

public:
    /// @brief Construct an empty distribution.
    Categorical_distribution() :
        db(),
        cdf_(),
        total_weight_(0)
    {}

    /**
     * @brief   Constructs a categorical distribution over the integers
     * {t1, ..., t1 + K}, where K is the length of the given vector.
     *
     * @param   ps  A std::vector of probabilities
     */
    Categorical_distribution(const std::vector<double>& ps, const T& t1 = T(1));

    /**
     * @brief   Constructs a categorical distribution over the integers
     * {1, ..., K}, where K is the length of the given vector.
     *
     * @param   ps  A kjb::Vector of probabilities.
     */
    Categorical_distribution(const Vector& ps, const T& = T(1));

    /**
     * @brief   Constructs a categorical distribution over a set of values
     *
     * @param values A set of values to define probabilities over
     * @param probabilities The probabilities corresponding to the values.
     */
    Categorical_distribution
    (
        const std::vector<T>& values,
        const std::vector<double>& probabilities
        );

    /**
     * @brief   Constructs a categorical distribution over a set of values
     *
     * @param values A set of values to define probabilities over
     * @param probabilities The probabilities corresponding to the values.
     */
    Categorical_distribution
    (
        const std::vector<T>& values,
        const Vector&         probabilities
    );


    /**
     * @brief   Constructs a categorical distribution over a set of values
     *
     * @param values_begin Beginning of a sequence of values to define probabilities over
     * @param values_end End of a sequence of values to define probabilities over
     * @param probabilities_begin The beginning of a sequence of probabilities corresponding to the values.
     *
     * @pre probability sequence has at least std::distance(values_begin, values_end) elements
     */
    /*template <class Value_iterator, class Prob_iterator>
    Categorical_distribution
    (
        Value_iterator values_begin,
        Value_iterator values_end,
        Prob_iterator probabilities_begin
    );*/

    /**
     * @brief   Constructs a categorical distribution from the
     * given map.
     *
     * @param   d  A map containing the distribution. It must
     * be such that P(X = x) = d[x].
     */
    Categorical_distribution(const std::map<T, double>& d) : db(d)
    {
        init_cdf_();
    }

    /**
     * @brief   Constructs an empirical distribution from a map of vals to counts
     *
     * @param   m  A map containing the distribution. It must
     * be such that P(X = x) is proportional to m[x].
     */
    Categorical_distribution(const std::map<T, size_t>& m)
    {
        for(typename std::map<T,size_t>::const_iterator it = m.begin();
            it != m.end(); ++it)
        {
            db[it->first] = static_cast<double>(it->second);
        }
        init_cdf_();
    }
    
    /**
     * @brief   Constructs a categorical distribution limits.
     *
     * Creates a uniform distribution over the set
     * {min, min + step, ..., max}, where each element has
     * probability  step / (max - min + 1).
     *
     * @param   min   smallest element of the support
     * @param   max   largest element of the support
     * @param   step  gap between elements of the support
     */
    Categorical_distribution(const T& min, const T& max, const T& step);

    /**
     * @brief   Creates a two-element categorical distribution.
     *
     * This constructore creates a two-element distribution, with
     * P(X = x1) = p1 and P(X = x2) = p2. Remember that p1 + p2 = 1.
     *
     * (wait, so this is just a bernoulli distribution?  suggest removing this constructor --kyle, March 13, 2012)
     */
    Categorical_distribution(const T& x1, const T& x2, double p1, double p2)
    {
        db[x1] = p1;
        db[x2] = p2;
        init_cdf_();
    }

    /** @brief   Copy-ctor: needed to make sure init_cdf_ gets called. */
    Categorical_distribution(const Categorical_distribution<T>& cd) :
        db(cd.db)
    {
        init_cdf_();
    }

    /** @brief   Assignment: needed to make sure init_cdf_ gets called. */
    Categorical_distribution<T>& operator=(const Categorical_distribution<T>& cd)
    {
        if(this != &cd)
        {
            db = cd.db;
            init_cdf_();
        }

        return *this;
    }

    /**
     * Named constructor 
     *
     * Similar to the constructor that recieves a map of value-weight pairs, but
     * in this case, the weights are given in log-space.
     *
     * This constructor converts to normalized non-log weights, without
     * losing precision even when the unnormalized weights
     * are extremely small.
     *
     * @author Kyle Simek
     */
    static Categorical_distribution<T> construct_from_log_map(
        const std::map<T, double>& log_map
        )
    {
        // NOT_USED typedef typename std::map<T, double>::value_type Pair;
        
        Categorical_distribution<T> result;

        result.db = log_map;

        // get the sum of the log-weights, without leaving log space, which mitigates precision loss
        // when weights are very small.

        std::vector<double> log_probs;
        log_probs.reserve(log_map.size());

        typedef typename std::map<T,double>::const_iterator iterator;
        for(iterator it = log_map.begin(); it != log_map.end(); ++it)
        {
            log_probs.push_back(it->second);
        }

        double log_total = kjb::log_sum(log_probs.begin(), log_probs.end());

        // normalize by subtracting log-total.  then exponentiate to get out of log-space
        for(iterator it = result.db.begin(); it != result.db.end(); ++it)
            it->second = exp(it->second - log_total);

        // TODO: init_cdf_ re-normalizes here; would be nice to factor the normalization
        // bits out of init_cdf_.  Note: we can't avoid normalizing above, because if we don't
        // we risk losing precision when exponentiating.
        result.init_cdf_();
        return result;
    }

    /**
     * @brief   Size of this distribution
     *
     * Returns the size of the support of this distribution
     */
    size_t size() const
    {
        return db.size();
    }

    /**
     * @param   weight  Remove an item from the collection.
     *
     * Cumulative distrbution will be re-constructed from scratch, requiring O(n) time.
     *
     * @throws Illegal_argument if key does not exist in the collection.
     */
    void erase(const T& key)
    {
        size_t count = db.erase(key);
        if(count == 0) 
            KJB_THROW_2(Illegal_argument, "Key not found");
        init_cdf_();
    }

    /** 
     * Alias of erase().
     *
     * Deprecated in favor of the more STL-esque erase().
     */
    void remove(const T& key)
    {
        erase(key);
    }

    /**
     * @param   weight  Unnormalized weight of new atom.  This is added to the
     *                  total of atom weights, and the normalization constant is
     *                  updated.
     *
     * Running time is O(log n)
     *
     */
    void insert (const T& key, double weight = 1.0)
    {
        size_t count = db.count(key);
        if(count > 0)
            KJB_THROW_2(Illegal_argument, "Key collision");

        const T* stored_key = &db.insert(std::make_pair(key, weight)).first->first;

        if(cdf_.empty())
            total_weight_ = weight;
        else
            total_weight_ += weight;

        cdf_.push_back(Cdf_pair(stored_key, total_weight_));
    }

    /**
     * Alias of insert.  
     *
     * Deprecated in favor of the more STL-esque name, insert().
     */
    void add(const T& key, double weight = 1.0)
    {
        insert(key, weight);
    }

    ~Categorical_distribution()
    {}

    // friends functions; these need to be friends because they
    // access the private parts
    template<class U>
    friend double pdf(const Categorical_distribution<U>& dist, const U& x);

    template<class U>
    friend double cdf(const Categorical_distribution<U>& dist, const U& x);

    template<class U>
    friend U sample(const Categorical_distribution<U>& dist);

private:

    void init_cdf_()
    {
        using namespace boost;

        cdf_.resize(db.size());
        if(cdf_.size() == 0) return;

        //typedef typename std::map<T, double>::const_iterator Const_iterator;

        // copy into cdf
        kjb_parallel_std::transform(
            db.begin(),
            db.end(),
            cdf_.begin(),
            make_cdf_pair()
        );

        // convert pdf values into cdf values
        kjb_parallel_std::partial_sum(
            cdf_.begin(),
            cdf_.end(),
            cdf_.begin(),
            partial_sum_second<Cdf_pair>()
        );

        total_weight_ = cdf_.back().second;

        // ensure that at least one element had significant mass,
        // so we can normalize the masses
        if(total_weight_ < FLT_EPSILON)
        {
            KJB_THROW_2(Illegal_argument, "Items have zero probability mass.");
        }


        // normalize to 1.0
        // update: we now store CDF unnormalized, to make dynamic
        // adding of elements easier.
//        kjb_parallel_std::for_each(
//            cdf_.begin(),
//            cdf_.end(),
//            multiply_second_by_scalar<Cdf_pair>(1.0/total_weight_)
//        );

        // update: we now store elements unnormalized, to make dynamic
        // adding of elements easier.
//        // normalize pdf map to 1.0
//        kjb_parallel_std::for_each(
//            db.begin(),
//            db.end(),
//            multiply_second_by_scalar<Map_pair>(1.0/cdf_.back().second)
//        );
    }


    // utility functors
    struct make_cdf_pair : public std::unary_function<Cdf_pair, const Map_pair&>
    {
        Cdf_pair operator()(const Map_pair& pair) const
        {
            return std::make_pair(&pair.first, pair.second);
        }
    };

    struct cdf_pair_less_than_scalar
    {
        bool operator()(const Cdf_pair& op1, double op2) const
        {
            return op1.second < op2;
        }
    };

    struct cdf_key_less_than_scalar
    {
        bool operator()(const Cdf_pair& op1, const T& op2) const
        {
            return op1.first < op2;
        }
    };

    template <class U>
    struct multiply_second_by_scalar
    {
        multiply_second_by_scalar(double scalar) :
            scalar_(scalar) 
        { }

        void operator()(U& in) const
        {
            in.second *= scalar_;
        }
        double scalar_;
    };


    template <class U>
    struct partial_sum_second : public std::binary_function<U, const U&, const U&>
    {
        U operator()(const U& op1, const U& op2)
        {
            U result = op2;
            result.second += op1.second;
            return result;
        }
    };
private:
    std::map<T, double> db;
    std::vector<Cdf_pair> cdf_;
    double total_weight_;
};

/* non-inline member functions */

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class T>
Categorical_distribution<T>::Categorical_distribution
(
    const std::vector<double>& ps,
    const T& t1
)
{
    for(std::size_t i = 0; i < ps.size(); i++)
    {
        db[static_cast<T>(i + t1)] = ps[i];
    }
    init_cdf_();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class T>
Categorical_distribution<T>::Categorical_distribution
(
    const Vector& ps,
    const T& t1
)
{
    for(int i = 0; i < ps.get_length(); i++)
    {
        db[static_cast<T>(i + t1)] = ps[i];
    }
    init_cdf_();
}


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template <class T>
Categorical_distribution<T>::Categorical_distribution
(
    const std::vector<T>& values,
    const std::vector<double>& probabilities)
{
    for(std::size_t i = 0; i < probabilities.size(); i++)
    {
        db[values[i]] = probabilities[i];
    }
    init_cdf_();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
    
template <class T>
Categorical_distribution<T>::Categorical_distribution
(
    const std::vector<T>& values,
    const Vector&         probabilities)
{
    for(int i = 0; i < probabilities.size(); i++)
    {
        db[values[i]] = probabilities[i];
    }
    init_cdf_();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*template <class T>
template <class Value_iterator, class Prob_iterator>
Categorical_distribution<T>::Categorical_distribution
(
    Value_iterator values_begin,
    Value_iterator values_end,
    Prob_iterator probabilities_begin)
{
    Prob_iterator prob_it = probabilities_begin;
    for(Value_iterator it = values_begin; it != values_end; ++it, ++prob_it)
    {
        db[*it] = *prob_it; 
    }
    init_cdf_();
}*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<class T>
Categorical_distribution<T>::Categorical_distribution
(
    const T& min,
    const T& max,
    const T& step
)
{
    double p = 1.0 / static_cast<int>((max - min + 1) / step);
    for(T x = min; x <= max; x += step)
    {
        db[x] = p;
    }
    init_cdf_();
}


/**
 * @brief   Traits for the categorical distro.
 */
template<class T>
struct Distribution_traits<Categorical_distribution<T> >
{
    typedef T type;
};

/*============================================================================
  The following are multivariate distributions that are not present in the
  boost library.
  ----------------------------------------------------------------------------*/

// forward declarations -- ignore
class MV_gaussian_distribution;
template<class T, class G, class F> class Conditional_distribution;
class MV_normal_on_normal_dependence;
typedef Conditional_distribution<MV_gaussian_distribution, MV_gaussian_distribution, MV_normal_on_normal_dependence> MV_gaussian_conditional_distribution;

/**
 * @brief Multivariate Gaussian (normal) distribution.
 
 * This class implements a multivariate Gaussian distribution.
 */
class MV_gaussian_distribution
{
public:

    /**
     * @brief   Constructs a d-dimensinoal multivariate Gaussian
     * with a mean of zero and a covariance matrix of identity.
     *
     * @param   d  The dimension of the distribution.
     */
    MV_gaussian_distribution(int d) : 
        mean(d, 0.0),
        cov_mat(create_identity_matrix(d)),
        type(INDEPENDENT),
        abs_det(1.0),
        log_abs_det(0.0)
    {}

    /**
     * @brief   Constructs a multivariate Gaussian
     * with a mean of mu and a covariance matrix of sigma. sigma must
     * be positive-definite.
     *
     * @param   mu      The mean of the distribution.
     * @param   sigma   The covariance matrix  of the distribution.
     */
    MV_gaussian_distribution(const Vector& mu, const Matrix& sigma) :
        mean(mu),
        cov_mat(sigma),
        type(NORMAL),
        abs_det(-1.0),
        log_abs_det(-std::numeric_limits<double>::max())
    {
        IFT(mean.get_length() == cov_mat.get_num_rows()
                && cov_mat.get_num_rows() == cov_mat.get_num_cols(),
            Dimension_mismatch,
            "Mean vector dimension must equal covariance matrix's"
                "number of rows and columns.");
    }

    /**
     * @brief   Constructs a multivariate Gaussian
     * with a mean of mu and a covariance matrix diag(sigma_diag).
     *
     * @param   mu          The mean of the distribution.
     * @param   sigma_diag  The diagonal of the covariance matrix.
     */
    MV_gaussian_distribution(const Vector& mu, const Vector& sigma_diag) :
        mean(mu),
        cov_mat(create_diagonal_matrix(sigma_diag)),
        type(INDEPENDENT),
        abs_det(-1.0),
        log_abs_det(-std::numeric_limits<double>::max())
    {
        IFT(mean.get_length() == cov_mat.get_num_rows(), Dimension_mismatch,
            "Mean vector dimension must equal covariance matrix's"
                "number of rows and columns.");
    }

    /**
     * @brief   Gets the mean of this distribution.
     */
    const Vector& get_mean() const 
    {
        return mean;
    }

    /**
     * @brief   Gets the covariance matrix of this distribution.
     */
    const Matrix& get_covariance_matrix() const 
    {
        return cov_mat;
    }

    /**
     * @brief   Gets the mean of this distribution.
     */
    void set_mean(const Vector& m)
    {
        IFT(cov_mat.get_num_rows() == m.get_length(), Illegal_argument,
            "mean vector must have same size as covariance matrix.");

        mean = m;
    }

    /**
     * @brief   Gets the covariance matrix of this distribution.
     */
    void set_covariance_matrix(const Matrix& S)
    {
        IFT(S.get_num_rows() == mean.get_length(), Illegal_argument,
            "mean vector must have same size as covariance matrix.");

        type = NORMAL;

        cov_mat = S;
        cov_inv.resize(0, 0);
        cov_chol.resize(0, 0);
        abs_det = -1;
        log_abs_det = -std::numeric_limits<double>::max();
    }

    /**
     * @brief   Gets the covariance matrix of this distribution.
     */
    void set_covariance_matrix(const Matrix& S, const Vector& m)
    {
        IFT(S.get_num_rows() == m.get_length(), Illegal_argument,
            "mean vector must have same size as covariance matrix.");

        type = NORMAL;

        mean = m;
        cov_mat = S;
        cov_inv.resize(0, 0);
        cov_chol.resize(0, 0);
        abs_det = -1;
        log_abs_det = -std::numeric_limits<double>::max();
    }

    /**
     * @brief   Gets the dimension of this distribution.
     */
    int get_dimension() const
    {
        return mean.get_length();
    }

    /**
     * @brief   Conditional distribution of the ith variable
     *          given the rest.
     */
    MV_gaussian_conditional_distribution conditional(int i) const;

private:
    /**
     * @brief   Update inverse of covariance matrix if necessary.
     */
    void update_cov_inv() const
    {
        if(cov_inv.get_num_rows() == 0)
        {
            cov_inv = matrix_inverse(cov_mat);
        }
    }

    /**
     * @brief   Update cholesky square root of covariance if necessary.
     */
    void update_cov_chol() const
    {
        if(cov_chol.get_num_rows() == 0)
        {
            cov_chol = cholesky_decomposition(cov_mat);
        }
    }

    /**
     * @brief   Update abs val of determinant of covariance matrix.
     */
    void update_abs_det() const
    {
        if(abs_det == -1.0)
        {
            abs_det = cov_mat.abs_of_determinant();
        }
    }

    /**
     * @brief   Update log of the abs val of determinant of covariance matrix.
     */
    void update_log_abs_det() const;

    friend double pdf(const MV_gaussian_distribution&, const Vector&);

    friend double log_pdf(const MV_gaussian_distribution&, const Vector&);

    friend Vector sample(const MV_gaussian_distribution&);

private:
    Vector mean;
    Matrix cov_mat;
    int type;
    mutable Matrix cov_inv;
    mutable Matrix cov_chol;
    mutable double abs_det;
    mutable double log_abs_det;

    const static int NORMAL = 0;
    const static int INDEPENDENT = 1;
};

//We should also be able to call it normal distribution
typedef MV_gaussian_distribution MV_normal_distribution;

/**
 * @brief Traits for the multivariate normal. Type is kjb::Vector.
 */
template<>
struct Distribution_traits<MV_gaussian_distribution>
{
    typedef Vector type;
};

/**
 * @brief Log-normal distribution
 */
class Log_normal_distribution
{
public:
    Log_normal_distribution() : 
        dist_(0, 1)
    {}

    Log_normal_distribution(double mu, double sigma) :
        dist_(mu, sigma)
    {}

    friend double pdf(const Log_normal_distribution&, double);

    friend double cdf(const Log_normal_distribution&, double);

    friend double sample(const Log_normal_distribution&);
private:
    Normal_distribution dist_;
};

template<>
struct Distribution_traits<Log_normal_distribution>
{
    typedef double type;
};


/*============================================================================
  The following are other distributions (e.g., mixture).
  ============================================================================*/

/**
 * @brief This class implements a mixture distribution. In other
 * words, it is the sum of a finite number of fractions of
 * distributions of the same type (with different parameters).
 *
 * @tparam Distribution The type of distribution that makes up
 * the different parts of the mixture
 */
template<class Distribution>
class Mixture_distribution
{
private:
    // a convenient typedef
    typedef typename Distribution_traits<Distribution>::type type;

public:

    /**
     * @brief   Constructs a mixture distribution from the given distributions
     * and mixing coefficients.
     *
     * This constructore creates a mixture distribution with the given
     * distributions and the given coefficients, where distributions[i] has
     * mixing coefficient coefficients[i].
     */
    Mixture_distribution
    (
        const std::vector<Distribution>& distributions,
        const std::vector<double>& coefficients
    ) :
        dists(distributions), coeffs(coefficients)
    {}

    /**
     * @brief   Constructs a mixture distribution with equal mixing coefficients
     *
     * This constructore creates a mixture distribution with the given
     * distributions and equal mixing coefficients. That is, distributions[i]
     * has mixing coefficient 1 / distributions.size(), for all i.
     */
    Mixture_distribution(const std::vector<Distribution>& distributions) :
        dists(distributions), coeffs(1.0, distributions.size(), 1.0)
    {}

    /**
     * @brief   Constructs two-element mixture distribution.
     *
     * This constructore creates a mixture distribution with two elements, with
     * the coefficient of the first one being pi1, and the coefficient of the
     * second one being 1 - pi1.
     */
    Mixture_distribution
    (
        const Distribution& dist1,
        const Distribution& dist2,
        double pi1
    ) :
        coeffs(1, 2, pi1, 1 - pi1)
    {
        dists.push_back(dist1);
        dists.push_back(dist2);
    }

    // friends functions; these need to be friends because they access
    // the private parts (lol)
    template<class Dist>
    friend
    double pdf
    (
        const kjb::Mixture_distribution<Dist>& dist,
        const typename Distribution_traits<Mixture_distribution<Dist> >::type& x
    );

    template<class Dist>
    friend
    double cdf
    (
        const kjb::Mixture_distribution<Dist>& dist,
        const typename Distribution_traits<Mixture_distribution<Dist> >::type& x
    );

    template<class Dist>
    friend
    typename Distribution_traits<Mixture_distribution<Dist> >::type
        sample(const kjb::Mixture_distribution<Dist>& dist);

private:
    std::vector<Distribution> dists;
    Categorical_distribution<size_t> coeffs;
};

/**
 * @brief   Traits for the mixture distro. Type is the type
 *          of the mixed distributions.
 */
template<class Distribution>
struct Distribution_traits<Mixture_distribution<Distribution> >
{
    typedef typename Distribution_traits<Distribution>::type type;
};

/* /////////////////////////////////////////////////////////////////////////// */

/**
 * Uniform-distribution over the unit-sphere in D-dimensional Euclidean space 
 * (i.e. the "(D-1)-sphere")
 */
template <size_t D>
class Uniform_sphere_distribution
{
public:
    Uniform_sphere_distribution()  {}
};

/**
 * @brief   Traits for the Uniform_sphere_distribution
 */
template<size_t D>
struct Distribution_traits<Uniform_sphere_distribution<D> >
{
    typedef typename kjb::Vector_d<D> type;
};

/* /////////////////////////////////////////////////////////////////////////// */

template <size_t D>
class Von_mises_fisher_distribution
{
public:
    /**
     * @param mean unnormalized mean direction vector
     * @param kappa "precision" parameter.
     */
    Von_mises_fisher_distribution(const kjb::Vector_d<D>& mean, double kappa) :
        mu_(mean.normalized()),
        kappa_(kappa)
    {}

    const kjb::Vector_d<D>& mu() const { return mu_; }

    double kappa() const { return kappa_; }

private:
    kjb::Vector_d<D> mu_;
    double kappa_;
}; // Von_mises_fisher_distribution

/**
 * @brief   Traits for the Von-mises-fisher distribution 
 */
template<size_t D>
struct Distribution_traits<Von_mises_fisher_distribution<D> >
{
    typedef typename kjb::Vector_d<D> type;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

class Chinese_restaurant_process
{
public:
    typedef std::vector<std::vector<size_t> > Type;

public:
    Chinese_restaurant_process(double concentration, size_t nc) :
        theta_(concentration), nc_(nc)
    {}

    double concentration() const { return theta_; }
    size_t num_customers() const { return nc_; }

private:
    double theta_;
    double nc_;
};

template<>
struct Distribution_traits<Chinese_restaurant_process>
{
    typedef Chinese_restaurant_process::Type type;
};

// useful typedef
typedef Chinese_restaurant_process Crp;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

class Dirichlet_distribution
{
public:
    Dirichlet_distribution
    (
        const std::vector<double>& alphas
    ) : alphas_(alphas) 
    {
        sum_log_gamma_alphas_ = 0.0;
        double sum = std::accumulate(alphas_.begin(), alphas_.end(), 0.0);
        log_gamma_sum_alphas_ = boost::math::lgamma(sum);
        for(size_t i = 0; i < alphas_.size(); i++)
        {
            sum_log_gamma_alphas_ += boost::math::lgamma(alphas_[i]);
        }
    }

    const std::vector<double>& alphas() const { return alphas_; }

    friend double pdf(const Dirichlet_distribution&, const Vector&);
    friend double log_pdf(const Dirichlet_distribution&, const Vector&);
    friend Vector sample(const Dirichlet_distribution&);

private:
    std::vector<double> alphas_;
    double sum_log_gamma_alphas_;
    double log_gamma_sum_alphas_;
};

template<>
struct Distribution_traits<Dirichlet_distribution>
{
    typedef Vector type;
};


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
/**
 * @brief   A Normal-inverse-Gamma distribution
 *          This is often used as a conjugate prior for a Normal distribution 
 *          with unknown mean and variance
 *
 */
class Normal_inverse_gamma_distribution
{
public:
    Normal_inverse_gamma_distribution
    (
        const Vector& mean, 
        const Matrix& covariance,
        double shape,
        double scale
    ) : 
        mean_(mean),
        covariance_(covariance),
        normal_(mean, create_diagonal_matrix(mean.size(), 1.0)),
        shape_(shape),
        scale_(scale),
        ig_(shape, scale)
    {}

    double shape() const { return shape_; }
    double scale() const { return scale_; }
    const Vector& mean() const { return mean_; }
    const Matrix& covariance() const { return covariance_; }
    
    friend double pdf
    (
        const Normal_inverse_gamma_distribution& dist, 
        const Vector& mean,
        double variance
    );

    friend double log_pdf
    (
        const Normal_inverse_gamma_distribution& dist, 
        const Vector& mean,
        double variance
    );

    friend std::pair<Vector, double> sample
    (
        const Normal_inverse_gamma_distribution& dist
    );

private:
    Vector mean_;
    Matrix covariance_;
    mutable MV_gaussian_distribution normal_;
    double shape_;
    double scale_;
    Inverse_gamma_distribution ig_;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
/**
 * @brief   A Normal-inverse-chi_squared distribution
 *          This is often used as a conjugate prior for a Normal distribution 
 *          with unknown mean and variance
 *
 */
class Normal_inverse_chi_squared_distribution
{
public:
    Normal_inverse_chi_squared_distribution
    (
        const Vector& mean, 
        const Matrix& covariance,
        double degrees_of_freedom,
        double scale
    ) : 
        mean_(mean),
        covariance_(covariance),
        normal_(mean, create_diagonal_matrix(mean.size(), 1.0)),
        degrees_of_freedom_(degrees_of_freedom),
        scale_(scale),
        ics_(degrees_of_freedom, scale)
    {}

    double degrees_of_freedom() const { return degrees_of_freedom_; }
    double scale() const { return scale_; }
    const Vector& mean() const { return mean_; }
    const Matrix& covariance() const { return covariance_; }
    
    friend double pdf
    (
        const Normal_inverse_chi_squared_distribution& dist, 
        const Vector& mean,
        double variance
    );

    friend double log_pdf
    (
        const Normal_inverse_chi_squared_distribution& dist, 
        const Vector& mean,
        double variance
    );

    friend std::pair<Vector, double> sample
    (
        const Normal_inverse_chi_squared_distribution& dist
    );

private:
    Vector mean_;
    Matrix covariance_;
    mutable MV_gaussian_distribution normal_;
    double degrees_of_freedom_;
    double scale_;
    Inverse_chi_squared_distribution ics_;
};


/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
/**
 * @brief   A Wishart distribution
 * This is a generalization of the Gamma distribution to positive definite matrices.
 * It is mostly used to model the uncercentialy in convariance matrices.
 */
class Wishart_distribution
{
public:
    Wishart_distribution
    (
        double nu,
        const Matrix& cov
    ) :
        nu_(nu),
        S_(cov)
    {
        // Need to check if cov is positive definite
        IFT(cov.get_num_cols() == cov.get_num_rows(), Illegal_argument, 
                "Scale matrix need to be a square matrix");
        D_ = cov.get_num_cols();
        double nu_2 = nu_/2.0;
        double gamma_D = multivariate_gamma_function(D_, nu_2);
        double log_gamma_D = log_multivariate_gamma_function(D_, nu_2);
        Z_ = std::pow(2, nu_2 * D_) * gamma_D * 
             std::pow(S_.abs_of_determinant(), nu_2); 

        log_Z_ = nu_2 * D_ * std::log(2.0) + log_gamma_D 
                 + nu_2 * log(S_.abs_of_determinant());
        S_Cholesky_lower_ = cholesky_decomposition(S_);
    }

friend double pdf(const Wishart_distribution&, const Matrix&);
friend double log_pdf(const Wishart_distribution&, const Matrix&);
friend Matrix sample(const Wishart_distribution&);

private:
    size_t D_;
    double nu_;
    Matrix S_;
    double Z_;
    double log_Z_;
    Matrix S_Cholesky_lower_;
};

template<>
struct Distribution_traits<Wishart_distribution>
{
    typedef Matrix type;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
/**
 * @brief   A Inverse-Wishart distribution
 * This is a generalization of the inverse Gamma distribution to 
 * positive definite matrices.
 * It is mostly used to model the uncercentialy in convariance matrices.
 * Note that if inv(Sigma) ~ WI(S, nu), then Sigma ~ IW(inv(S), nu + D + 1)
 */
class Inverse_wishart_distribution
{
public:
    Inverse_wishart_distribution
    (
        double nu,
        const Matrix& cov
    ) :
        nu_(nu),
        S_(cov)
    {
        // Need to check if cov is positive definite
        IFT(cov.get_num_cols() == cov.get_num_rows(), Illegal_argument, 
                "Scale matrix need to be a square matrix");
        D_ = cov.get_num_cols();
        IFT(nu_ > D_ - 1, Illegal_argument, "The following condition need to be"
                " true for the PDF to be well-define: nu > D - 1 ");
        double nu_2 = nu_/2.0;
        double gamma_D = multivariate_gamma_function(D_, nu_2);
        double log_gamma_D = log_multivariate_gamma_function(D_, nu_2);
        Z_ = std::pow(2, nu_2 * D_) * gamma_D * 
             std::pow(S_.abs_of_determinant(), -nu_2); 

        log_Z_ = nu_2 * D_ * std::log(2.0) + log_gamma_D 
                 - nu_2 * log(S_.abs_of_determinant());

        // used for generating samples from a inverse wishart distribution
        S_Cholesky_lower_ = cholesky_decomposition(S_.inverse());
    }

    friend double pdf(const Inverse_wishart_distribution&, const Matrix&);
    friend double log_pdf(const Inverse_wishart_distribution&, const Matrix&);
    friend Matrix sample(const Inverse_wishart_distribution&);

private:
    size_t D_;
    double nu_;
    Matrix S_;
    double Z_;
    double log_Z_;
    Matrix S_Cholesky_lower_;
};

template<>
struct Distribution_traits<Inverse_wishart_distribution>
{
    typedef Matrix type;
};

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
/**
 * @brief   A Normal-inverse-wishart distribution
 *          A congugate prior for a multivariate normal distribution 
 *          with unknown mean and variances
 *  (mean, Sigma) ~ NIW(mu, kappa, S, nu)
 *
 * @param   mu           Prior mean for mean 
 * @param   kappa        How strongly we believe the above prior
 * @param   S            Propertional to the prior mean for Sigma
 * @param   nu           How strongly we believe the above prior 
 */
class Normal_inverse_wishart_distribution
{
public:
    Normal_inverse_wishart_distribution
    (
        const Vector& mu,
        double kappa,
        const Matrix& S,
        double nu
    ) :
        mu_(mu), 
        kappa_(kappa),
        S_(S),
        nu_(nu),
        normal_(mu, create_diagonal_matrix(mu.size(), 1.0)),
        iw_(nu, S)
    {}

    const Vector& mu() const {return mu_; }
    double kappa() const {return kappa_; }
    const Matrix& S() const {return S_; }
    double nu() const { return nu_; }

    friend double pdf 
    (
         const Normal_inverse_wishart_distribution&, 
         const Vector& mean,
         const Matrix& sigma
    );

    friend double log_pdf
    (
         const Normal_inverse_wishart_distribution&, 
         const Vector& mean,
         const Matrix& sigma
    );

    friend std::pair<Vector, Matrix> 
        sample(const Normal_inverse_wishart_distribution&);

private:
    Vector mu_;
    double kappa_;
    Matrix S_;
    double nu_;
    mutable MV_gaussian_distribution normal_;
    Inverse_wishart_distribution iw_;
};

template<>
struct Distribution_traits<Normal_inverse_wishart_distribution>
{
    typedef std::pair<Vector, Matrix> type;
};

} //namespace kjb


#endif /*PROB_DISTRIBUTION_H_INCLUDED */

