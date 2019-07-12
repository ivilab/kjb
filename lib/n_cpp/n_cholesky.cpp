/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#include <n_cpp/n_cholesky.h>
#include <m_cpp/m_matrix.h>
#include <cmath>

using namespace kjb;

double kjb::log_det(const Matrix& M)
{
    Matrix L = cholesky_decomposition(M);

    double ld = 0.0;
    for(Matrix::Size_type i = 0; i < L.get_num_rows(); i++)
    {
        ld += std::log(L(i, i));
    }

    ld *= 2;

    return ld;
}

