/* =========================================================================== *
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

/* $Id: prob_histogram.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "prob_cpp/prob_histogram.h"
#include "m_cpp/m_matrix.h"

using namespace kjb;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void Histogram_2d::compute_matrix() const
{
    m_hist_mat.resize(m_num_bins_1, m_num_bins_2, 0.0);

    typedef std::map<double, std::map<double, int> >::const_iterator X_map_p;
    typedef std::map<double, int>::const_iterator Y_map_p;
    size_t r = 0;
    for(X_map_p p = m_hist.begin(); p != m_hist.end(); p++, r++)
    {
        int c = 0;
        for(Y_map_p pp = p->second.begin(); pp != p->second.end(); pp++, c++)
        {
            m_hist_mat(r, c) = pp->second;
        }
    }
    m_updated = true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::chi_square(const Histogram_2d& hist_1, const Histogram_2d& hist_2)
{
    Matrix h1 = hist_1.normalized();
    Matrix h2 = hist_2.normalized();
    return chi_square(h1, h2);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::chi_square(const Matrix& h1, const Matrix& h2)
{
    ASSERT(h1.get_num_rows() == h2.get_num_rows());
    ASSERT(h1.get_num_cols() == h2.get_num_cols());
    double score = 0.0;
    for(size_t i = 0; i < h1.get_num_rows(); i++)
    {
        for(size_t j = 0 ; j < h1.get_num_cols(); j++)
        {
            double top = (h1.at(i, j) - h2.at(i, j));
            top *= top;
            double bottom = (h1.at(i, j) + h2.at(i, j));
            if(bottom > 0.0) score += (top/bottom);
        }
    }
    score *= 0.5;
    //ASSERT(score >= 0.0 && score <= 1.0);
    if(score < 0.0 || score > 1.0) 
    {
        //std::cout << score << std::endl;
    }
    return score;
}

