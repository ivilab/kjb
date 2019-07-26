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

/* $Id: prob_sample.cpp 21776 2017-09-17 16:44:49Z clayton $ */

/** @file
 *
 * @author Kobus Barnard
 * @author Ernesto Brau
 * @author Jinyan Guan
 *
 * @brief Definition of the non-inline sample functions.
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "prob_cpp/prob_sample.h"
#include "prob_cpp/prob_distribution.h"
#include "m_cpp/m_vector.h"

#include <cmath>
#include <boost/random.hpp>
#include <boost/bind.hpp>

#include <unistd.h>

namespace kjb {

// basic global random number generators
const unsigned int DEFAULT_SEED = static_cast<unsigned int>(std::time(0)+getpid());
Base_generator_type basic_rnd_gen(DEFAULT_SEED);
//boost::uniform_01<Base_generator_type> uni01(basic_rnd_gen);
//boost::minstd_rand basic_rnd_gen(DEFAULT_SEED);
//boost::uniform_01<boost::minstd_rand> uni01(basic_rnd_gen);

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Vector sample(const MV_gaussian_distribution& dist)
{
    Vector x(dist.get_dimension());

    if(dist.type == MV_gaussian_distribution::INDEPENDENT)
    {
        for(int i = 0; i < x.get_length(); i++)
        {
            x[i] = sample(Gaussian_distribution(dist.mean[i],
                                                std::sqrt(dist.cov_mat(i, i))));
        }
    }
    else
    {
        Gaussian_distribution phi;
        std::generate(x.begin(), x.end(), boost::bind(
            static_cast<double(*)(const Gaussian_distribution&)>(sample), phi));

        dist.update_cov_chol();
        Vector y = dist.cov_chol * x;
        x = y + dist.mean;
    }

    return x;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Chinese_restaurant_process::Type sample(const Chinese_restaurant_process& crp)
{
    const size_t n = crp.num_customers();
    const double theta = crp.concentration();

    std::vector<size_t> sizes;
    std::vector<double> probs(1, 1.0);
    Crp::Type B;
    for(size_t i = 0; i < n; i++)
    {
        Categorical_distribution<size_t> P(probs, 0);
        size_t b = sample(P);

        if(b == B.size())
        {
            B.push_back(std::vector<size_t>());
            sizes.push_back(0);
            probs.push_back(0.0);
        }

        ASSERT(B.size() == sizes.size());
        ASSERT(B.size() == probs.size() - 1);

        B[b].push_back(i);
        sizes[b]++;
        probs[B.size()] = theta / (i + 1 + theta);
        for(size_t j = 0; j < B.size(); j++)
        {
            probs[j] = static_cast<double>(sizes[j]) / (i + 1 + theta);
        }
    }

    return B;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
    
size_t sample_occupied_tables(const Chinese_restaurant_process& crp)
{
    const size_t n = crp.num_customers();
    const double theta = crp.concentration();

    size_t num_occupied_tables = 0;

    double new_table_prob = 1;
    for(size_t i = 0; i < n; i++)
    {
        Bernoulli_distribution P(new_table_prob);
        if(sample(P) == 1) ++num_occupied_tables;
        new_table_prob = theta / (i + 1 + theta);
    }
    return num_occupied_tables;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector sample(const Dirichlet_distribution& dist)
{
    const std::vector<double>& as = dist.alphas();
    size_t D = as.size();
    Vector x(D);
    // draw samples from gamma distributions
    for(size_t i = 0; i < D; i++)
    {
        Gamma_distribution gamma(as[i], 1.0);
        x[i] = sample(gamma);
    }
    // normailize the weights
    x /= x.sum_vector_elements();

    return x;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix sample_standard_wishart(double dof, size_t dim, const Matrix& C)
{
    // CTM : C is not used -- following avoids warning:
    (void) C;
    
    Matrix U(dim, dim, 0.0); 
    Normal_distribution normal(0.0, 1.0);
    for(size_t i = 0; i < dim; i++)
    {
        for(size_t j = i; j < dim; j++)
        {
            if(i == j)
            {
                // chi-squared distribution
                Chi_square_distribution chi_dist(dof - (j+1) + 1);
                U(i, j) = sqrt(sample(chi_dist));
            }
            else
            {
                U(i, j) = sample(normal);
            }
        }
    }

    return U;
}

} //namespace kjb

