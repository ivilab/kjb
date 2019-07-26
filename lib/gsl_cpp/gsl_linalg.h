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
   |  Author:  Kyle Simek
 * =========================================================================== */

/* $Id: gsl_linalg.h 19607 2015-07-17 00:11:07Z predoehl $ */

#ifndef KJB_CPP_GSL_LINALG_H
#define KJB_CPP_GSL_LINALG_H

/**
 * @file
 *
 * @brief Wrappers for GSL's linear algebra library.
 *
 * @author Jinyan
 */
#include <l_cpp/l_exception.h>
#include <gsl_cpp/gsl_util.h>
#include <gsl_cpp/gsl_matrix.h>

#ifdef KJB_HAVE_GSL
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_mode.h>
#else
#warning "Gnu Scientific Library is absent, yet essential to this program."
typedef unsigned int gsl_mode_t; 
#define GSL_MODE_DEFAULT 0
#endif


namespace kjb
{

inline void gsl_matrix_exponential
(
    const Gsl_matrix& A,
    Gsl_matrix& eA,
    gsl_mode_t mode = GSL_MODE_DEFAULT
)
{
#ifdef KJB_HAVE_GSL
    GSL_ETX(gsl_linalg_exponential_ss(A.const_ptr(), eA.ptr(), mode));
#else
    KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

inline void gsl_matrix_exponential
(
    const Matrix& A,
    Matrix& eA
)
{
#ifdef KJB_HAVE_GSL

    Gsl_matrix gsl_A(A);
    Gsl_matrix gsl_eA(gsl_A.num_rows(), gsl_A.num_cols());

    gsl_matrix_exponential(gsl_A, gsl_eA);
    eA.resize(gsl_A.num_rows(), gsl_A.num_cols());
    for(size_t i = 0; i < eA.get_num_rows(); i++)
    {
        for(size_t j = 0; j < eA.get_num_cols(); j++)
        {
            eA(i, j) = gsl_eA.at(i, j);
        }
    }

#else
    KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
}

} // namespace kjb

#endif /* KJB_CPP_GSL_LINALG_H */
