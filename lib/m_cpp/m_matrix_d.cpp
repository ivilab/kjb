/* $Id: m_matrix_d.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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

#include "m_cpp/m_matrix_d.h"
#include "m_cpp/m_matrix_d.impl.h"
#include <boost/preprocessor.hpp>

namespace kjb{

#define MAX_DIMENSION 4

// explicit instanciation of square matrices up to dimension 4
#define BOOST_PP_LOCAL_MACRO(nnn) \
template class Matrix_d<nnn,nnn,true>; \
template class Matrix_d<nnn,nnn,false>; 

#define BOOST_PP_LOCAL_LIMITS (1,MAX_DIMENSION)
#include BOOST_PP_LOCAL_ITERATE()

// explicit instantiation of non-square matrices
template class Matrix_d<2,3,true>;
template class Matrix_d<2,3,false>;
template class Matrix_d<3,4,true>;
template class Matrix_d<3,4,false>;
template class Matrix_d<2,4,false>;

// The macro below list all of the free functions in m_matrix_d.cpp.impl,
// which will need to be explicity instantiated in this file.

// List of free functions
#define FREE_FUNCTION_LIST(NROWS,NCOLS,TRANSPOSED) \
template Matrix_d<NROWS, NCOLS, TRANSPOSED> operator*(double scalar, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& mat); \
template Matrix operator*(const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1, const Matrix& op2); \
template Matrix operator*(const kjb::Matrix& op1, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2); \
template bool operator==(const Matrix& op2, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1); \
template bool operator!=(const Matrix& op2, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1); \
template Matrix_d<NROWS, NCOLS, TRANSPOSED> operator-(const Matrix op1, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2); \
template Matrix_d<NROWS, NCOLS, TRANSPOSED> operator+(const Matrix& second, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2); \
template Matrix_d<Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_rows, Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_cols> Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator*(const Matrix_d<Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_cols, Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_cols, false>& second) const; \
template Matrix_d<Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_rows, Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_cols> Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator*(const Matrix_d<Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_cols, Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_cols, true>& second) const; \
template std::ostream& operator<<(std::ostream& ost, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& mat); \
template double accumulate(const Matrix_d<NROWS,NCOLS, TRANSPOSED>& mat, double init); \
template double max(const Matrix_d<NROWS,NCOLS,TRANSPOSED>& mat); \
template double min(const Matrix_d<NROWS,NCOLS,TRANSPOSED>& mat); \

// list of free functions where the "TRANSPOSED" variable is fixed
#define FREE_FUNCTION_LIST_NO_TRANSPOSE(NROWS,NCOLS) \
template Matrix_d<NROWS,NCOLS> create_random_matrix(); \
template double max_abs_difference( const Matrix_d<NROWS,NCOLS,false>& op1, const Matrix_d<NROWS,NCOLS,false>& op2 ); \

// list of free functions where the matrix dimension is square (and TRANSPOSED is fixed)
#define FREE_FUNCTION_LIST_SQUARE(D) \
template Matrix_d<D,D> create_identity_matrix(); \
template Matrix_d<D,D> outer_product(const Vector_d<D>& v1, const Vector_d<D>& v2); \
template double trace(const Matrix_d<D,D>& m);

#define FREE_FUNCTION_LIST_3DIM(A,B,C) \
    template Matrix_d<Matrix_d<A,B,false>::num_rows, Matrix_d<B,C,false>::num_cols> matrix_multiply_dispatch_(const Matrix_d<A,B,false>& m1, const Matrix_d<B,C,false>& m2); \
    template Matrix_d<Matrix_d<B,A,true>::num_rows, Matrix_d<B,C,false>::num_cols> matrix_multiply_dispatch_(const Matrix_d<B,A,true>& m1, const Matrix_d<B,C,false>& m2); \
    template Matrix_d<Matrix_d<B,A,true>::num_rows, Matrix_d<C,B,true>::num_cols> matrix_multiply_dispatch_(const Matrix_d<B,A,true>& m1, const Matrix_d<C,B,true>& m2); \
    template Matrix_d<Matrix_d<A,B,false>::num_rows, Matrix_d<C,B,true>::num_cols> matrix_multiply_dispatch_(const Matrix_d<A,B,false>& m1, const Matrix_d<C,B,true>& m2); \

// Explicit instantiation of free-functions for sq matrices up to dimension 4
#define BOOST_PP_LOCAL_MACRO(nnn) \
FREE_FUNCTION_LIST(nnn,nnn,true) \
FREE_FUNCTION_LIST(nnn,nnn,false) \
FREE_FUNCTION_LIST_NO_TRANSPOSE(nnn,nnn) \
FREE_FUNCTION_LIST_SQUARE(nnn) \
FREE_FUNCTION_LIST_3DIM(nnn,nnn,nnn) \

#define BOOST_PP_LOCAL_LIMITS (1,MAX_DIMENSION)
#include BOOST_PP_LOCAL_ITERATE()

// Explicit instantiation of free-functions for non-sq matrices up to dimension 4
FREE_FUNCTION_LIST(2,3,true);
FREE_FUNCTION_LIST(2,3,false);
FREE_FUNCTION_LIST_NO_TRANSPOSE(2,3);

FREE_FUNCTION_LIST(3,4,true);
FREE_FUNCTION_LIST(3,4,false);
FREE_FUNCTION_LIST_NO_TRANSPOSE(3,4);

FREE_FUNCTION_LIST_3DIM(3,3,4);
FREE_FUNCTION_LIST_3DIM(3,4,4);
FREE_FUNCTION_LIST_3DIM(2,2,3);
FREE_FUNCTION_LIST_3DIM(2,3,3);


///////////////////////
// Implementation of Non-generic functions
///////////////////////
inline double det(const Matrix_d<1,1>& m)
{
    return m(0,0);
}

double det(const Matrix_d<2,2>& m)
{
    return m(0,0) * m(1,1) - m(0,1)*m(1,0);
}


double det(const Matrix_d<3,3>& m)
{
    // credit:  http://www.dr-lex.be/random/matrix_inv.html
    return m(0,0) * (m(2,2) * m(1,1) - m(2,1) * m(1,2)) - 
           m(1,0) * (m(2,2) * m(0,1) - m(2,1) * m(0,2)) + 
           m(2,0) * (m(1,2) * m(0,1) - m(1,1) * m(0,2));
}

Matrix_d<2,2> matrix_inverse(const Matrix_d<2,2>& m)
{
    Matrix_d<2,2> out;

    out(0,0)=  m(1,1);
    out(0,1)= -m(0,1);
    out(1,0)= -m(1,0);
    out(1,1)=  m(0,0);

    return out / det(m);
}

Matrix_d<3,3> matrix_inverse(const Matrix_d<3,3>& m)
{
    // credit:  http://www.dr-lex.be/random/matrix_inv.html
    Matrix_d<3,3> out;
    out(0,0)=  m(2,2)*m(1,1)-m(2,1)*m(1,2);
    out(0,1)=-(m(2,2)*m(0,1)-m(2,1)*m(0,2));
    out(0,2)=  m(1,2)*m(0,1)-m(1,1)*m(0,2);

    out(1,0)=-(m(2,2)*m(1,0)-m(2,0)*m(1,2));
    out(1,1)=  m(2,2)*m(0,0)-m(2,0)*m(0,2);
    out(1,2)=-(m(1,2)*m(0,0)-m(1,0)*m(0,2));

    out(2,0)=  m(2,1)*m(1,0)-m(2,0)*m(1,1);
    out(2,1)=-(m(2,1)*m(0,0)-m(2,0)*m(0,1));
    out(2,2)=  m(1,1)*m(0,0)-m(1,0)*m(0,1);

    return out / det(m);
}
    
void symmetric_eigs(const kjb::Matrix3& A, double& eig1, double& eig2, double& eig3)
{
    // from wikipedia:
    // http://en.wikipedia.org/wiki/Eigenvalue_algorithm#3.C3.973_matrices
    // accessed 14 October, 2014
    const double p1 = A(0,1)*A(0,1) + A(0,2)*A(0,2) + A(1,2)*A(1,2);

    if (p1 == 0)
    {
       // A is diagonal.
       eig1 = A(0,0);
       eig2 = A(1,1);
       eig3 = A(2,2);
    }
    else
    {
        const double q = trace(A)/3;
        const double A11_q = (A(0,0) - q);
        const double A22_q = (A(1,1) - q);
        const double A33_q = (A(2,2) - q);
        const double p2 = A11_q*A11_q + A22_q*A22_q + A33_q * A33_q + 2 * p1;
        const double p = sqrt(p2 / 6);
        static const kjb::Matrix3 I = create_identity_matrix<3>();
        const kjb::Matrix3 B = (1 / p) * (A - q * I);
        const double r = det(B) / 2;
       
        // In exact arithmetic for a symmetric matrix  -1 <= r <= 1
        // but computation error can leave it slightly outside this range.
        double phi;
        if (r <= -1) 
           phi = M_PI / 3;
        else if (r >= 1)
           phi = 0;
        else
           phi = acos(r) / 3;
       
        // the eigenvalues satisfy eig3 <= eig2 <= eig1
        eig1 = q + 2 * p * cos(phi);
        eig3 = q + 2 * p * cos(phi + (2*M_PI/3));
        eig2 = 3 * q - eig1 - eig3;    // since trace(A) = eig1 + eig2 + eig3

        // hooray for bubble sort!
        if(eig1 > eig2) std::swap(eig1, eig2);
        if(eig2 > eig3) std::swap(eig2, eig3);
        if(eig1 > eig2) std::swap(eig1, eig2);
    }
}

} // namespace kjb
