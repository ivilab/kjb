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

#ifndef DIFF_FOURIER_H
#define DIFF_FOURIER_H

#include <m_cpp/m_matrix.h>
#include <vector>

namespace kjb {

/**
 * @brief   Returns a matrix whose rows form a Fourier basis in R^D.
 *
 * This function returns a set of basis vectors that form a Fourier
 * basis in D-dimensional space. If num_basis_funcs is specified, only the
 * first num_basis_funcs are included.
 */
Matrix fourier_basis(size_t D, size_t num_basis_funcs);

/**
 * @brief   Returns a matrix whose rows form a Fourier basis in R^D.
 *
 * This function returns a set of basis vectors that form a Fourier
 * basis in D-dimensional space.
 *
 * @param   indices     Only these basis vectors will be included; i.e.,
 *                      the indices specified here between 0 and D - 1.
 */
//Matrix fourier_basis(size_t D, const std::vector<size_t>& indices);

} //namespace kjb

#endif /*DIFF_FOURIER_H */


