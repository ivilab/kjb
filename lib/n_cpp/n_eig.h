/* $Id: n_eig.h 14050 2013-03-08 04:47:06Z jguan1 $ */
/* {{{=========================================================================== *
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <l_cpp/l_exception.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <n/n_diagonalize.h>

#ifdef KJB_HAVE_BOOST
#include <boost/tuple/tuple.hpp>
#endif

#ifndef KJB_CPP_EIG_H
#define KJB_CPP_EIG_H

namespace kjb {


/// @brief KJB c-style syntax for eigenvalue decomposition
inline
void diagonalize (const Matrix& M, Matrix& eig_vectors, Vector& eig_values, bool symmetric = false)
{
    if(M.get_num_rows() != M.get_num_cols())
        KJB_THROW_2(Illegal_argument, "diagonalize() failed: M matrix must be square.");

    // If not already the right size, resize them.
    // If already the right size, or larger, storage is re-used
    eig_values.resize(M.get_num_rows());
    eig_vectors.resize(M.get_num_rows(), M.get_num_cols());

    // Get at the underlying c-pointer, so we can pass to the c implementation.
    // This is normally discouraged, but it allows us to reuse the allocated memory if possible.
    kjb_c::Vector* c_eig_values = eig_values.get_underlying_representation_with_guilt();
    kjb_c::Matrix* c_eig_vectors = eig_vectors.get_underlying_representation_with_guilt();

    // make the C call
    if(symmetric)
        ETX(kjb_c::diagonalize_symmetric(M.get_c_matrix(), &c_eig_vectors, &c_eig_values));
    else
        ETX(kjb_c::diagonalize(M.get_c_matrix(), &c_eig_vectors, &c_eig_values));


}


/**
 * Matlab-style syntax for eigenvalue decomposition.
 *
 * @note depending on the compiler's optimizer, this might require and extra copy compared to the diagonalize() function that receives non-const references for the output values.
 */
#ifdef KJB_HAVE_BOOST
inline 
boost::tuple<Matrix, Vector> eig(const Matrix& M, bool symmetric = false)
{
    Matrix evec;
    Vector eval;

    diagonalize(M, evec, eval, symmetric);

    return boost::make_tuple(evec, eval);
}
#endif

} // namespace kjb

#endif 
