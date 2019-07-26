/* $Id: m_mat_calc.h 12036 2012-04-05 20:54:58Z ksimek $ */
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


#ifndef KJB_MAT_CALC
#define KJB_MAT_CALC

/* *
 * @file m_mat_calc.h
 *
 * Collection of functions implementing calculus-related matrix functionality.
 */
namespace kjb
{
    /* DEPRECATED March 22, 2012 --KS */ Matrix derivative_matrix(int degree, int N);

    inline Matrix create_derivative_matrix( int N, int degree = 1)
    {
        return derivative_matrix(degree, N);
    }

    Matrix create_integral_matrix(const std::vector<double>& deltas);
    Matrix create_integral_matrix(size_t N);
}


#endif
