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

#include <diff_cpp/diff_fourier.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace kjb;

Matrix kjb::fourier_basis(size_t D, size_t num_basis_funcs)
{
    size_t nb = std::min(num_basis_funcs, D);

    Matrix Q(D, nb, 1.0);

    // if the number of vectors is even, we need another cosine at the end
    if(nb % 2  == 0)
    {
        for(size_t k = 0; k < D; k++)
        {
            size_t n = nb / 2;
            Q(k, nb - 1) = std::cos(2*n*k*M_PI/D);
        }
    }

    size_t n = 1;
    for(size_t j = 1; j < (nb + 1) / 2; j++, n += 2)
    {
        for(size_t k = 0; k < D; k++)
        {
            Q(k, n) = std::cos(2.0*j*k*M_PI/D);
            Q(k, n + 1) = std::sin(2.0*j*k*M_PI/D);
        }
    }

    for(size_t j = 0; j < nb; j++)
    {
        Vector c = Q.get_col(j);
        Q.set_col(j, c.normalize());
    }

    return Q;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

//Matrix fourier_basis(size_t D, const std::vector<size_t>& indices)
//{
//}

