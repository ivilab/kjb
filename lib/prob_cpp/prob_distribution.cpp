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
|     Ernesto Brau
|
* =========================================================================== */

/* $Id: prob_distribution.cpp 20242 2016-01-20 22:36:29Z jguan1 $ */

#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_conditional_distribution.h"
#include <string>
#include <boost/format.hpp>
#include <algorithm>
#include "l_cpp/l_exception.h"
#include "l_cpp/l_index.h"
#include "m_cpp/m_mat_view.h"

namespace kjb {

const Gaussian_distribution STD_NORMAL;

void MV_gaussian_distribution::update_log_abs_det() const
{
    if(log_abs_det == -std::numeric_limits<double>::max())
    {
        update_cov_chol();
        log_abs_det = 0.0;
        for(Matrix::Size_type i = 0; i < cov_chol.get_num_rows(); i++)
        {
            log_abs_det += std::log(cov_chol(i, i));
        }

        log_abs_det *= 2;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

MV_gaussian_conditional_distribution
    MV_gaussian_distribution::conditional(int i) const
{
    IFT(i >= 0 && i < get_dimension(), Index_out_of_bounds,
        "Cannot get conditional distribution: index out of bounds");

    // TODO: replace string-based indexing with
    //       kjb::Index_range objects
    Vector mu(get_dimension() - 1);
    std::copy(mean.begin(), mean.begin() + i, mu.begin());
    std::copy(mean.begin() + i + 1, mean.end(), mu.begin() + i);

    using boost::str;
    using boost::format;
    std::string range_str = "";
    if(i != 0)
    {
        range_str += str(format("0:%d") % (i - 1));
    }
    
    if(i != 0 && i != get_dimension() - 1)
    {
        range_str += ",";
    }
    
    if(i != get_dimension() - 1)
    {
        range_str += str(format("%d:%d") % (i + 1) % (get_dimension() - 1));
    }

    std::string range_str2 = str(format("%d:%d") % i % i);
    Matrix Sigma = cov_mat(range_str, range_str);
    Matrix Sigma_i = cov_mat(range_str2, range_str);

    return MV_gaussian_conditional_distribution(
                MV_normal_on_normal_dependence(
                    MV_gaussian_distribution(
                        Vector(1, mean[i]), Matrix(1, 1, cov_mat(i, i))),
                    MV_gaussian_distribution(mu, Sigma), Sigma_i));
}

} //namespace kjb

