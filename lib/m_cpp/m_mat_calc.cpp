/* $Id: m_mat_calc.cpp 11961 2012-03-26 19:54:36Z ksimek $ */
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
   |  Author:  Kyle Simek
 * =========================================================================== */

#include "l_cpp/l_cpp_incl.h"
#include "m_cpp/m_matrix.h"

namespace kjb
{
Matrix derivative_matrix(int degree, int N)
{
    if(degree <= 0)
    {
        KJB_THROW_2(Illegal_argument, "Derivative degree must be > 0");
    }

    Matrix D;

    if(degree == 1)
    {
        D = Matrix(N-1, N, 0.0);
        for(int i = 0; i < N-1; i++)
        {
            D(i,i) = -1;
            D(i,i+1) = 1;
        }
    }
    else if(degree == 2)
    {
        D = Matrix(N-2, N, 0.0);
        for(int i = 0; i < N-2; i++)
        {
            D(i,i) = 1;
            D(i,i+1) = -2;
            D(i,i+2) = 1;
        }
    }
    else // should rewrite in closed-form
    {
        KJB(UNTESTED_CODE());

        D = create_identity_matrix(N);
        for(int d = 0; d < degree; d++)
        {
            D = derivative_matrix(1, N-1) * D;
        }
    }

    return D;
}

kjb::Matrix create_integral_matrix(const std::vector<double>& deltas)
{
    size_t N = deltas.size();
    kjb::Matrix result(N,N, 0.0);

    for(size_t r = 0; r < N; ++r)
    for(size_t c = 0; c <= r; ++c)
    {
        result(r,c) = deltas[c];
    }

    return result;
}

/**
 * Returns a lower-triangular matrix with all 1's.
 */
kjb::Matrix create_integral_matrix(size_t N)
{
    kjb::Matrix result(N,N, 0.0);

    for(size_t r = 0; r < N; ++r)
    for(size_t c = 0; c <= r; ++c)
    {
        result(r,c) = 1;
    }

    return result;
}

}
