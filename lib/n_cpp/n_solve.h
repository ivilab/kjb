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

#ifndef N_SOLVE_H
#define N_SOLVE_H

#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>

namespace kjb {

/**
 * @brief   Perform forward substitution to solve the SLE Lx = b, where
 *          L is an lower triangular matrix and b is a vector.
 *
 * @note    L MUST be lower triangular; if it is not, behavior is undefined
 * @note    This is a very dumb -- and, therefore, slow -- algorithm.
 */
Vector forward_substitution(const Matrix& L, const Vector& b);

/**
 * @brief   Perform forward substitution to solve the SLE Ux = b, where
 *
 * @note    U MUST be upper triangular; if it is not, behavior is undefined
 * @note    This is a very dumb -- and, therefore, slow -- algorithm.
 */
Vector back_substitution(const Matrix& U, const Vector& b);

} // namespace kjb

#endif  /*N_SOLVE_H */

