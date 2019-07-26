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

#ifndef N_CHOLESKY_H
#define N_CHOLESKY_H

#include <m_cpp/m_matrix.h>
#include <m/m_matrix.h>
#include <n/n_cholesky.h>

namespace kjb {

/**
 * For any symmetric positive-definite matrix M, returns a
 * lower-triangular triangular matrix L such that M = LL'
 */
inline
Matrix cholesky_decomposition(const Matrix& M)
{
    kjb_c::Matrix* kjb_matrix = 0;
    ETX(kjb_c::cholesky_decomposition(&kjb_matrix, M.get_c_matrix()));
    return Matrix(kjb_matrix);
}

/**
 * For any symmetric positive-definite matrix M, returns the log of
 * the determinant of M. Uses Cholesky decomposition.
 */
double log_det(const Matrix& M);

} // namespace kjb

#endif /*N_CHOLESKY_H */

