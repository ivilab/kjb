/* $Id: m_vector_d.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_vector_d.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_vector_d.impl.h"

#include <boost/preprocessor.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

/**
 * This file includes explicit template instantiation for
 * Vector_D<N> for N up to 10.  This allows m_vector_d.h to 
 * remain lightweight by containing no template implementations.
 *
 * Note: If you need a larger than 10-dimensional Vector_d, you
 * must also #include <m_cpp/m_vector_d.impl.h>
 */

#undef MAX_DIMENSION
namespace kjb
{

// This is the highest value of D that we will explicitly instanciate.
#define MAX_DIMENSION 10

// explicitly instanciate Vector_d<N> for N = 1 ... 10
#define BOOST_PP_LOCAL_MACRO(n) \
template class Vector_d<n>;
#define BOOST_PP_LOCAL_LIMITS (1,MAX_DIMENSION)
#include BOOST_PP_LOCAL_ITERATE()


#ifdef KJB_HAVE_BST_SERIAL
#define BOOST_PP_LOCAL_MACRO(nnn) \
template \
void Vector_d<nnn>::serialize(boost::archive::text_iarchive& ar, const unsigned int /* version */) ;  \
template \
void Vector_d<nnn>::serialize(boost::archive::text_oarchive& ar, const unsigned int /* version */) ;  
#define BOOST_PP_LOCAL_LIMITS (1,MAX_DIMENSION)
#include BOOST_PP_LOCAL_ITERATE()
#endif

#define BOOST_PP_LOCAL_MACRO(nnn) \
template Vector operator*(const Matrix& m, const Vector_d<nnn> v); \
template Vector_d<nnn> operator-(const Vector_d<nnn>& first, const Vector_d<nnn>& second); \
template Vector_d<nnn> operator-(const Vector_d<nnn>& first, double second); \
template Vector_d<nnn> operator-(double first, const Vector_d<nnn>& second); \
template Vector_d<nnn> operator-(const Vector_d<nnn>& first ); \
template Vector_d<nnn> operator+(const Vector_d<nnn>& first, const Vector_d<nnn>& second); \
template Vector_d<nnn> operator+(const Vector_d<nnn>& first, double second); \
template Vector_d<nnn> operator+(double first, const Vector_d<nnn>& second); \
template Vector_d<nnn> operator*(const Vector_d<nnn>& v, double s); \
template Vector_d<nnn> operator*(double s, const Vector_d<nnn>& v); \
template Vector_d<nnn> operator/(const Vector_d<nnn>& v, double s); \
template void swap(Vector_d<nnn>& first, Vector_d<nnn>& second); \
template bool operator==(const Vector_d<nnn>& first, const Vector_d<nnn>& second); \
template bool operator!=(const Vector_d<nnn>& first, const Vector_d<nnn>& second); \
template bool operator<(const Vector_d<nnn>& first, const Vector_d<nnn>& second); \
template bool operator>(const Vector_d<nnn>& first, const Vector_d<nnn>& second); \
template bool operator<=(const Vector_d<nnn>& first, const Vector_d<nnn>& second); \
template bool operator>=(const Vector_d<nnn>& first, const Vector_d<nnn>& second); \
template double norm1(const kjb::Vector_d<nnn>& v); \
template double norm2(const kjb::Vector_d<nnn>& v); \
template Vector_d<nnn> create_random_vector(); \
template Vector_d<nnn> create_gauss_random_vector(); \
template Vector_d<nnn> create_unit_vector(size_t d); \
template std::ostream& operator<<(std::ostream& out, const Vector_d<nnn>& v); \
template std::istream& operator>>(std::istream& ist, Vector_d<nnn>& v); \
template double dot(const Vector_d<nnn>& op1, const Vector_d<nnn>& op2); \
template double vector_distance(const Vector_d<nnn>& op1, const Vector_d<nnn>& op2); \
template double vector_distance_squared(const Vector_d<nnn>& op1, const Vector_d<nnn>& op2);

#define BOOST_PP_LOCAL_LIMITS (1,MAX_DIMENSION)
#include BOOST_PP_LOCAL_ITERATE()



Vector multiply_matrix_and_vector_d_dispatch_(const Matrix& m, const double* v, size_t size)
{
    if (m.get_num_cols() != (int) size)
    {
        KJB_THROW(kjb::Dimension_mismatch);
    }

    Vector result(m.get_num_rows());

    for (int row = 0; row < m.get_num_rows(); row++)
    {
        double x = 0;
        
        for (size_t col = 0; col < size; col++)
        {
            x += m(row, col) * v[col];
        }

        result[row] = x;
    }

    return result;
}

Vector2 make_vector2(const Vector& v)
{
    ASSERT(v.size() == 2);
    return Vector2(v[0], v[1]);
}

/// @pre v.size() == 3
Vector3 make_vector3(const Vector& v)
{
    ASSERT(v.size() == 3);
    return Vector3(v[0], v[1], v[2]);
}

/// @pre v.size() == 4
Vector4 make_vector4(const Vector& v)
{
    ASSERT(v.size() == 4);
    return Vector4(v[0], v[1], v[2], v[4]);
}

} // namespace kjb;
