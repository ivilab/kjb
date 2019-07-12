/* $Id: prob_histogram.h 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Ernesto Brau, Jinyan Guan
 * =========================================================================== */


#ifndef KJB_PROB_HISTOGRAM
#define KJB_PROB_HISTOGRAM

/**
 * @file Histogram class.
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "m_cpp/m_matrix.h"
#include "l_cpp/l_functors.h"
#include "l_cpp/l_exception.h"

#include <vector>
#include <iterator>
#include <algorithm>
#include <utility>
#include <map>

#include <boost/bind.hpp>

namespace kjb 
{

/**
 * @brief   A class that represents a histogram of data. ATM,
 *          the data must be doubles.
 */
class Histogram
{
private:
    std::map<double, int> m_hist;
    int m_num_bins;
    double m_bin_size;

public:
    /**
     * @brief   Create a histogram from data with the given number of bins.
     */
    template<class InputIterator>
    Histogram(InputIterator first, InputIterator last, int num_bins) :
        m_num_bins(num_bins)
    {
        if(num_bins > 0)
        {
            create_continuous(first, last);
        }
        else
        {
            create_discrete(first, last);
        }
    }

    /**
     * @brief   Return the number of bins of this histogram.
     */
    int num_bins() const
    {
        return m_num_bins;
    }

    /**
     * @brief   Return the bin size of this histogram.
     */
    double bin_size() const
    {
        return m_bin_size;
    }

    /**
     * @brief   Return a vector of the bins, represented by their lower
     *          bounds.
     */
    const std::map<double, int>& as_map() const
    {
        return m_hist;
    }

private:
    template<class InputIterator>
    void create_continuous(InputIterator first, InputIterator last);

    template<class InputIterator>
    void create_discrete(InputIterator first, InputIterator last);
};

inline std::pair<double, int> make_double_int_pair_(double d, int i)
{
    return std::make_pair(d, i);
}

template<class InputIterator>
void Histogram::create_continuous(InputIterator first, InputIterator last)
{
    double mn = *std::min_element(first, last);
    double mx = *std::max_element(first, last);

    m_bin_size = (mx - mn) / m_num_bins;

    std::generate_n(std::insert_iterator<std::map<double, int> >(m_hist, m_hist.begin()), m_num_bins,
                    boost::bind(make_double_int_pair_,
                                boost::bind<double>(Increase_by<double>(mn, m_bin_size)),
                                0));

    for(InputIterator p = first; p != last; p++)
    {
        std::map<double, int>::iterator pair_p = m_hist.upper_bound(*p);
        pair_p--;
        pair_p->second++;
    }
}

template<class InputIterator>
void Histogram::create_discrete(InputIterator first, InputIterator last)
{
    for(InputIterator p = first; p != last; p++)
    {
        std::map<double, int>::iterator pair_p = m_hist.find(*p);
        if(pair_p != m_hist.end())
        {
            pair_p->second++;
        }
        else
        {
            m_hist.insert(std::make_pair(*p, 1));
        }
    }

    m_num_bins = m_hist.size();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

class Histogram_2d
{
public:
    /**
     * @brief   Create a 2D histogram from bivirate data given the number of
     * bins of each dimension 
     */
    template<class InputIterator>
    Histogram_2d
    (
        InputIterator first, 
        InputIterator last, 
        size_t num_bins_1,
        size_t num_bins_2,
        const std::pair<double, double>& range_1,
        const std::pair<double, double>& range_2
    ) : m_num_bins_1(num_bins_1),
        m_num_bins_2(num_bins_2),
        m_range_1(range_1),
        m_range_2(range_2),
        m_updated(false)
    {
        create_map(first, last);
    }

    /**
     * @brief   Return the number of bins of this histogram.
     */
    size_t num_bins_1() const;
    size_t num_bins_2() const;

    /**
     * @brief   Return the bin size of this histogram.
     */
    double bin_size_1() const;
    double bin_size_2() const;

    /**
     * @brief   Return the ranges histogram.
     */
    const std::pair<double, double>& range_1() const;
    const std::pair<double, double>& range_2() const;

    /**
     * @brief   Return a map of the histogram
     */
    const std::map<double, std::map<double, int> >& as_map() const;

    /**
     * @brief   Return the non-const veresion  
     */
    size_t& num_bins_1();
    size_t& num_bins_2();

    double& bin_size_1();
    double& bin_size_2();

    std::pair<double, double>& range_1(); 
    std::pair<double, double>& range_2(); 

    /**
     * @brief   Return the 2D histogram as a kjb::Matrix 
     *          Note: the 2D histogrma is not normalized
     */
    const Matrix& as_matrix() const;

    /**
     * @brief   Return the normalized 2D histogram of which the sum 
     *          of all the elements 1.0
     */
    Matrix normalized() const;

    /**
     * @brief   Return the densities of the 2D histogram 
     *          for each cell p, 
     *          density(p) = counts(p) * area(p)/total_couns
     *
     */
    Matrix densities() const; 

private:
    size_t m_num_bins_1;
    size_t m_num_bins_2;
    double m_bin_size_1;
    double m_bin_size_2;
    std::pair<double, double> m_range_1;
    std::pair<double, double> m_range_2;
    std::map<double, std::map<double, int> > m_hist;
    mutable Matrix m_hist_mat;
    mutable bool m_updated; 

private:
    template<class InputIterator>
    void create_map (InputIterator first, InputIterator last);
    void compute_matrix() const;

}; //class Histogram_2d

double chi_square(const Histogram_2d& hist_1, const Histogram_2d& hist_2);

double chi_square(const Matrix& h1, const Matrix& h2);

inline size_t Histogram_2d::num_bins_1() const
{
    return m_num_bins_1;
}

inline size_t Histogram_2d::num_bins_2() const
{
    return m_num_bins_2;
}

inline double Histogram_2d::bin_size_1() const
{
    return m_bin_size_1;
}

inline double Histogram_2d::bin_size_2() const
{
    return m_bin_size_2;
}

inline const std::pair<double, double>& Histogram_2d::range_1() const
{
    return m_range_1;
}

inline const std::pair<double, double>& Histogram_2d::range_2() const
{
    return m_range_2;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

inline size_t& Histogram_2d::num_bins_1() 
{
    return m_num_bins_1;
}

inline size_t& Histogram_2d::num_bins_2() 
{
    return m_num_bins_2;
}

inline double& Histogram_2d::bin_size_1() 
{
    return m_bin_size_1;
}

inline double& Histogram_2d::bin_size_2() 
{
    return m_bin_size_2;
}

inline std::pair<double, double>& Histogram_2d::range_1() 
{
    return m_range_1;
}

inline std::pair<double, double>& Histogram_2d::range_2() 
{
    return m_range_2;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

template<typename InputIterator>
void Histogram_2d::create_map
(
    InputIterator first, 
    InputIterator last
) 
{
    double d1_range = m_range_1.second - m_range_1.first;
    double d2_range = m_range_2.second - m_range_2.first;
    ASSERT(d1_range >= 0.0 && d2_range >= 0.0);
    m_bin_size_1 = d1_range/(m_num_bins_1 - 1);
    m_bin_size_2 = d2_range/(m_num_bins_2 - 1);


    int r = 0;
    double d1 = m_range_1.first;
    for(size_t i = 0; i < m_num_bins_1; i++, d1 += m_bin_size_1)
    {
        double d2 = m_range_2.first;
        for(size_t j = 0; j < m_num_bins_2; j++, d2 += m_bin_size_2)
        {
            m_hist[d1][d2] = 0;
        }
        r++;
    }

    for(InputIterator p = first; p != last; p++)
    {
        std::map<double, std::map<double, int> >::iterator pair_p = m_hist.upper_bound((*p)[0]);
        pair_p--;
        std::map<double, int>::iterator pair_pp = (pair_p->second).upper_bound((*p)[1]);
        pair_pp--;
        pair_pp->second++;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

inline const Matrix& Histogram_2d::as_matrix() const
{
    if(!m_updated)
    {
        compute_matrix();
    }

    return m_hist_mat;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

inline Matrix Histogram_2d::normalized() const
{
    if(!m_updated)
    {
        compute_matrix();
    }
    double total_counts = sum_matrix_cols(m_hist_mat).sum_vector_elements();
    Matrix n_hist(m_hist_mat);
    n_hist /= total_counts;
    return n_hist;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

inline Matrix Histogram_2d::densities() const
{
    if(!m_updated)
    {
        compute_matrix();
    }
    double total_counts = sum_matrix_cols(m_hist_mat).sum_vector_elements();
    double area = m_bin_size_1 * m_bin_size_2;
    Matrix n_hist(m_hist_mat);
    n_hist = n_hist / total_counts * area;
    return n_hist;
}


} // namespace kjb

#endif /* KJB_PROB_HISTOGRAM */

