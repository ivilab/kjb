/* $Id: m_vector_d.impl.h 21596 2017-07-30 23:33:36Z kobus $ */
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

#ifndef KJB_M_VECTOR_D_IMPL_H
#define KJB_M_VECTOR_D_IMPL_H

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "l_cpp/l_exception.h"
#include "l/l_sys_rand.h" /* for kjb_rand() */
#include "sample/sample_gauss.h" /* for kjb_rand() */

#include <boost/array.hpp>
#include <algorithm>
#include <numeric>
#include <iterator>

#include <iomanip>

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/array.hpp>
#endif

namespace kjb
{

template <size_t D>
Vector_d<D>::Vector_d() :
    Base()
{}

template <size_t D>
Vector_d<D>::Vector_d(const Vector_d& v) :
    Base()
{
    std::copy(v.begin(), v.end(), this->begin());
}

template <size_t D>
Vector_d<D>::Vector_d(const Vector_d<D-1>& v, double fill1) :
    Base()
{
    std::copy(v.begin(), v.end(), this->begin());
    this->operator[](D-1) = fill1;
}

template <size_t D>
Vector_d<D>::Vector_d(const Vector_d<D-1>& v, double fill1, double fill2) :
    Base()
{
    std::copy(v.begin(), v.end(), this->begin());
    this->operator[](D-2) = fill1;
    this->operator[](D-1) = fill2;
}

/// @brief Construct from "dynamic" vector.
template <size_t D>
Vector_d<D>::Vector_d(const Vector& v) :
    Base()
{
    if (v.size() != D)
    {
        KJB_THROW_3(Illegal_argument, "Cannot construct Vector_d<%d> from Vector of length %d", (D)(v.size()));
    }

    std::copy(v.begin(), v.end(), this->begin());
}

template <size_t D>
Vector_d<D>::Vector_d(double x) :
    Base()
{
    this->assign(x);
}

template <size_t D>
Vector_d<D>::Vector_d(double x, double y) :
    Base()
{
    ASSERT(D == 2);
    this->operator[](0) = x;
    this->operator[](1) = y;
}

/**
 * Construct a Vector from three values.  This constructor only exists if D == 3 
 */
template <size_t D>
Vector_d<D>::Vector_d(double x, double y, double z) :
    Base()
{
    ASSERT(D == 3);
    this->operator[](0) = x;
    this->operator[](1) = y;
    this->operator[](2) = z;
}

/**
 * Construct a Vector from four values.  This constructor only exists if D == 4 
 */
template <size_t D>
Vector_d<D>::Vector_d( double x, double y, double z, double w)
{
    ASSERT(D == 4);
    this->operator[](0) = x;
    this->operator[](1) = y;
    this->operator[](2) = z;
    this->operator[](3) = w;
}

/**
 * double x() const;
 * Returns the first element of a vector.  This method throws exception if D < 1.
 */
template <size_t D>
double Vector_d<D>::x() const 
{
    if (D < 1)
    {
        KJB_THROW_2(Runtime_error, "Can't get x: vector dimension < 1");
    }

    return this->operator[](0); 
}

/**
 * @brief lvalue of the first element of a vector.
 * @throws Runtime_error if D < 1.
 */
template <size_t D>
double& Vector_d<D>::x()
{
    if (D < 1)
    {
        KJB_THROW_2(Runtime_error, "Can't set x: vector dimension < 1");
    }
    return this->operator[](0);
}

template <size_t D>
void Vector_d<D>::set_x(double xv) 
{
    x() = xv;
}

/**
 * double y() const;
 * Returns the second element of a vector.  This method throw exception if D < 2.
 */
template <size_t D>
double Vector_d<D>::y() const 
{
    if (D < 2)
    {
        KJB_THROW_2(Runtime_error, "Can't get y: vector dimension < 1");
    }

    return this->operator[](1); 
}

/**
 * @brief lvalue of the second element of a vector.
 * @throws Runtime_error if D < 2.
 */
template <size_t D>
double& Vector_d<D>::y()
{
    if (D < 2)
    {
        KJB_THROW_2(Runtime_error, "Can't set y: vector dimension < 1");
    }

    return this->operator[](1); 
}

/// @brief Changes the second element of a vector (if it exists).
template <size_t D>
void Vector_d<D>::set_y(double yv) 
{
    y() = yv;
}

/**
 * Returns the third element of a vector.  This method throws exception if D < 3.
 */
template <size_t D>
double Vector_d<D>::z() const 
{
    if (D < 3)
    {
        KJB_THROW_2(Runtime_error, "Can't get z: vector dimension < 3");
    }
    return this->operator[](2); 
}

/**
 * @brief lvalue of the third element of a vector.
 * @throws Runtime_error if D < 3.
 */
template <size_t D>
double& Vector_d<D>::z()
{
    if (D < 3)
    {
        KJB_THROW_2(Runtime_error, "Can't set z: vector dimension < 3");
    }
    return this->operator[](2); 
}

/// @brief Changes the third element of a vector (if it exists).
template <size_t D>
void Vector_d<D>::set_z(double zv) 
{
    z() = zv;
}

/**
 * @brief read fourth element (i.e., index 3) of vector, if it exists.
 * @return copy of the value in the fourth element of the vector.
 * @throws Runtime_error if D < 4;
 */
template <size_t D>
double Vector_d<D>::w() const
{
    return this->operator[](3);
}

/**
 * @brief lvalue of the fourth element of a vector (i.e., has index 3).
 * @throws Runtime_error if D < 4.
 */
template <size_t D>
double& Vector_d<D>::w()
{
    if (D < 4)
    {
        KJB_THROW_2(Runtime_error, "Can't set w: vector dimension < 4");
    }
    return this->operator[](3); 
}

/// @brief Changes the fourth element of a vector (if it exists).
template <size_t D>
void Vector_d<D>::set_w(double wv) { w() = wv; }


/// @brief assignment
template <size_t D>
Vector_d<D>& Vector_d<D>::operator=(const Vector_d<D>& other)
{
    Base::operator=(other);
    return *this;
}

/// @brief assignment from dynamic vector
template <size_t D>
Vector_d<D>& Vector_d<D>::operator=(const Vector& other)
{
    if (other.size() != D)
    {
        KJB_THROW(Dimension_mismatch);
    }

    std::copy(other.begin(), other.end(), this->begin());

    return *this;
}


template <size_t D>
Vector_d<D>& Vector_d<D>::operator-=(const Vector_d<D>& second)
{
    std::transform(this->begin(), this->end(), second.begin(), this->begin(), std::minus<double>()); 

    return *this;
}

template <size_t D>
Vector_d<D>& Vector_d<D>::operator-=(double second)
{
    std::transform(this->begin(), this->end(), this->begin(), std::bind2nd(std::minus<double>(), second)); 

    return *this;
}

template <size_t D>
Vector_d<D>& Vector_d<D>::operator+=(const Vector_d<D>& second)
{
    std::transform(this->begin(), this->end(), second.begin(), this->begin(), std::plus<double>()); 

    return *this;
}

template <size_t D>
Vector_d<D>& Vector_d<D>::operator+=(double second)
{
    std::transform(this->begin(), this->end(), this->begin(), std::bind2nd(std::plus<double>(), second)); 

    return *this;
}

template <size_t D>
Vector_d<D>& Vector_d<D>::operator*=(double s)
{
    for (size_t i = 0; i < D; i++)
    {
        this->operator[](i) *= s;
    }

    return *this;
}

template <size_t D>
Vector_d<D>& Vector_d<D>::operator/=(double s)
{
    return (*this) *= 1/s;
}

/// @throws if n != D
template <size_t D>
void Vector_d<D>::resize(size_t n)
{
    if (n != D)
    {
        KJB_THROW_2(Illegal_argument, "Can't resize Vector_d.");
    }
}

template <size_t D>
double Vector_d<D>::norm1() const
{
    double total = 0;

    for (typename Base::const_iterator it = this->begin(); it != this->end(); ++it)
    {
        total += fabs(*it);
    }

    return total;
}

/// @brief return the squared l2-norm of this vector
template <size_t D>
double Vector_d<D>::magnitude_squared() const
{
    return std::inner_product(this->begin(), this->end(), this->begin(), 0.0);
}

/// @brief return the l2-norm of this vector
template <size_t D>
double Vector_d<D>::magnitude() const
{
    return sqrt(magnitude_squared());
}

/// @brief normalize this vector to have l2-norm of 1.0
template <size_t D>
Vector_d<D>& Vector_d<D>::normalize()
{
    *this /= this->magnitude();
    return *this;
}

/// @brief Non-mutating (functionally pure) version of normalize()
template <size_t D>
Vector_d<D> Vector_d<D>::normalized() const
{
    return Vector_d<D>(*this).normalize();
}

template <size_t D>
Vector_d<D>& Vector_d<D>::negate()
{
    for (size_t i = 0; i < this->size(); ++i)
    {
        this->operator[](i) = -this->operator[](i);
    }

    return *this;
}

#ifdef KJB_HAVE_BST_SERIAL
template <size_t D>
template <class Archive>
void Vector_d<D>::serialize(Archive &ar, const unsigned int /* version */)
{
    ar & ::boost::serialization::base_object<boost::array<double, D> >(*this);
}
#endif

////////////////////////////////////

/// @brief multiply by static vector
template <std::size_t D>
Vector operator*(const Matrix& m, const Vector_d<D> v)
{
    return multiply_matrix_and_vector_d_dispatch_(m, &v[0], D);
}

/// @brief dispatch for operator*(Matrix, Vector_d).  This deals with a circular dependency between matrix.h and vector_d.h by moving template code into a cpp file.  DO NOT CALL DIRECTLY!
Vector multiply_matrix_and_vector_d_dispatch_(const Matrix& m, const double* v, size_t size);



#if 0 
// Static matrix not implemented yet
/// @brief multiply static matrix and static vector
template <std::size_t D_out, std::size_t D_in>
Vector_d<D_out> operator*(const Matrix_d<D_out, D_in>& m, const Vector_d<D_in> v)
{
    Vector_d<D_out> result();

    for (int row = 0; row < D_out; row++)
    {
        double x = 0;
        
        for (size_t col = 0; col < D_in; col++)
        {
            x += m(row, col) * v[col];
        }

        result[row] = x;
    }

    return result;
}
#endif

//
///// slightly slower than m * v.
//Vector operator*(const Vector_d<D> v, const Matrix& m)
//{
//    return m.transpose() * v;
//}

template <std::size_t D>
Vector_d<D> operator-(const Vector_d<D>& first, const Vector_d<D>& second)
{
    return Vector_d<D>(first) -= second;
}

template <std::size_t D>
Vector_d<D> operator-(const Vector_d<D>& first, double second)
{
    return Vector_d<D>(first) -= second;
}

template <std::size_t D>
Vector_d<D> operator-(double first, const Vector_d<D>& second)
{
    return -second + first;
}

template <std::size_t D>
Vector_d<D> operator-(const Vector_d<D>& first )
{
    Vector_d<D> result = first;
    for (size_t i = 0; i < D; i++)
    {
        result[i] = -result[i];
    }

    return result;
}

template <std::size_t D>
Vector_d<D> operator+(const Vector_d<D>& first, const Vector_d<D>& second)
{
    return Vector_d<D>(first) += second;
}

template <std::size_t D>
Vector_d<D> operator+(const Vector_d<D>& first, double second)
{
    return Vector_d<D>(first) += second;
}

template <std::size_t D>
Vector_d<D> operator+(double first, const Vector_d<D>& second)
{
    return second + first;
}

template <std::size_t D>
Vector_d<D> operator*(const Vector_d<D>& v, double s)
{
    return Vector_d<D>(v) *= s;
}

template <std::size_t D>
Vector_d<D> operator*(double s, const Vector_d<D>& v)
{
    return Vector_d<D>(v) *= s;
}

template <std::size_t D>
Vector_d<D> operator/(const Vector_d<D>& v, double s)
{
    return Vector_d<D>(v) /= s;
}

template<std::size_t N> 
void swap(Vector_d<N>& first, Vector_d<N>& second)
{
    first.swap(second);
}

// COMPARATORS
template<std::size_t N> 
bool operator==(const Vector_d<N>& first, const Vector_d<N>& second)
{
    // Call boost-provided operator
    return boost::operator==<double, N>(first, second);
}

template<std::size_t N> 
bool operator!=(const Vector_d<N>& first, const Vector_d<N>& second)
{
    // Call boost-provided operator
    return boost::operator!= <double, N>(first, second);
}

template<std::size_t N> 
bool operator<(const Vector_d<N>& first, const Vector_d<N>& second)
{
    // Call boost-provided operator
    return boost::operator< <double, N>(first, second);
}

template<std::size_t N> 
bool operator>(const Vector_d<N>& first, const Vector_d<N>& second)
{
    // Call boost-provided operator
    return boost::operator> <double, N>(first, second);
}

template<std::size_t N> 
bool operator<=(const Vector_d<N>& first, const Vector_d<N>& second)
{
    // Call boost-provided operator
    return boost::operator<= <double, N>(first, second);
}

template<std::size_t N> 
bool operator>=(const Vector_d<N>& first, const Vector_d<N>& second)
{
    // Call boost-provided operator
    return boost::operator>= <double, N>(first, second);
}

// TYPEDEFS
typedef Vector_d<2> Vector2;
typedef Vector_d<3> Vector3;
typedef Vector_d<4> Vector4;

// MATH FUNCTIONS
template <size_t D>
double norm1(const kjb::Vector_d<D>& v)
{
    return v.norm1();
}

template <size_t D>
double norm2(const kjb::Vector_d<D>& v)
{
    return v.magnitude();
}


// UTILITY FUNCTIONS
template <size_t D>
Vector_d<D> create_random_vector()
{
    Vector_d<D> result;
    std::generate(result.begin(), result.end(), kjb_c::kjb_rand);
    return result;
}

template <size_t D>
Vector_d<D> create_gauss_random_vector()
{
    Vector_d<D> result;
    std::generate(result.begin(), result.end(), kjb_c::gauss_rand);
    return result;
}

template <size_t D>
Vector_d<D> create_unit_vector(size_t d)
{
    ASSERT(d < D);
    Vector_d<D> result(0.0);
    result[d] = 1.0;
    return result;
}

template <std::size_t D>
std::ostream& operator<<(std::ostream& out, const Vector_d<D>& v)
{
    
    std::streamsize w = out.width();
    std::streamsize p = out.precision();
    std::ios::fmtflags f = out.flags();

    out << std::scientific;
    for (size_t i = 0; i < v.size(); i++)
    {
        out << std::setw(16) << std::setprecision(8) << v[i];
    }

    out.width( w );
    out.precision( p );
    out.flags( f );
    return out;
}

template <std::size_t D>
std::istream& operator>>(std::istream& ist, Vector_d<D>& v)
{
    ist >> std::skipws;
    for (size_t i = 0; i < v.size(); i++)
    {
        ist >> v[i];
    }

    return ist;
} 

template <std::size_t D>
double dot(const Vector_d<D>& op1, const Vector_d<D>& op2)
{
    return std::inner_product(op1.begin(), op1.end(), op2.begin(), 0.0);
}

/**
 * @brief   Compute the Euclidian distance between two vectors
 *
 * A routine that computes the distance between two vectors
 */
template <std::size_t D>
double vector_distance(const Vector_d<D>& op1, const Vector_d<D>& op2)
{
    return (op2 - op1).magnitude();
}

/**
 * @brief   Compute the Euclidian distance between two vectors
 *
 * A routine that computes the distance between two vectors
 */
template <std::size_t D>
double vector_distance_squared(const Vector_d<D>& op1, const Vector_d<D>& op2)
{
    return (op2 - op1).magnitude_squared();
}

/** @} */

} // namespace kjb

#endif /* KJB_M_VECTOR_D_IMPL_H */
