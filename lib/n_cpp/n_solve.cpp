/* $Id$ */
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

#include <n_cpp/n_solve.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>

namespace kjb {

Vector forward_substitution(const Matrix& L, const Vector& b)
{
    Vector x(L.get_num_cols());

    for(int i = 0; i < x.get_length(); i++)
    {
        x[i] = b[i];
        for(int j = 0; j <= i - 1; j++)
        {
            x[i] -= (L(i, j) * x[j]);
        }
        x[i] /= L(i, i);
    }

    return x;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector back_substitution(const Matrix& U, const Vector& b)
{
    Vector x(U.get_num_cols());

    for(int i = U.get_num_cols() - 1; i >= 0; i--)
    {
        x[i] = b[i];
        for(int j = U.get_num_cols() - 1; j >= i + 1; j--)
        {
            x[i] -= (U(i, j) * x[j]);
        }
        x[i] /= U(i, i);
    }

    return x;
}

} // namespace kjb

