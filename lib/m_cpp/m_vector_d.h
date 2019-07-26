/* $Id: m_vector_d.h 18367 2014-12-08 18:29:27Z ksimek $ */
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

#ifndef KJB_M_VECTOR_D_H
#define KJB_M_VECTOR_D_H

#include <boost/array.hpp>
#include <algorithm>
//#include <iterator>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#endif

/**
 * This header contains no implementations; those are in m_vector_d.impl.h.
 *
 * If you need a vector_d of dimension less than 11, you can skip including
 * m_vector_d.impl.h and benefit from fast compile times.  This is because
 * m_vector_d.cpp explicitly instantiates the Vector_d<D> template for D
 * up to 10.
 *
 * For dimensions greater than 10, both this file and m_vector_d.impl.h must
 * be included by your code.
 */

namespace kjb
{

class Vector;
class Matrix;
/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

template<std::size_t D>
class Vector_d : public boost::array<double, D>
{
    typedef typename boost::array<double, D> Base;

#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif

public:
    using typename Base::value_type;
    using typename Base::difference_type;
    using typename Base::size_type;
    using typename Base::reference;
    using typename Base::const_reference;
    using typename Base::iterator;
    using typename Base::const_iterator;
    /// @brief default constructor
    Vector_d();

    /// @brief Copy constructor
    Vector_d(const Vector_d& v);

    /// @brief Copy from D-1 dimensional vector, providing extra element
    Vector_d(const Vector_d<D-1>& v, double fill1);


    /// @brief Copy from D-2 dimensional vector, providing extra elements
    Vector_d(const Vector_d<D-1>& v, double fill1, double fill2);


    /// @brief Fill constructor
    template <class Iterator>
    explicit Vector_d(Iterator begin) :
        Base()
    {
        Iterator end = begin;
        std::advance(end, D);
        std::copy(begin, end, this->begin());
    }

    /// @brief Construct from "dynamic" vector.
    explicit Vector_d(const Vector& v);


    /// @brief initialize all elements with value init
    explicit Vector_d(double init);

    Vector_d(double x, double y);

    /**
     * Construct a Vector from three values.  This constructor only exists if D == 3 
     */
    Vector_d(double x, double y, double z);

    /**
     * Construct a Vector from four values.  This constructor only exists if D == 4 
     */
    Vector_d( double x, double y, double z, double w);

    /**
     * double x() const;
     * Returns the first element of a vector.  This method throws exception if D < 1.
     */
    double x() const;

    /**
     * @brief lvalue of the first element of a vector.
     * @throws Runtime_error if D < 1.
     */
    double& x();

    void set_x(double xv) ;

    /**
     * double y() const;
     * Returns the second element of a vector.  This method throw exception if D < 2.
     */
    double y() const ;

    /**
     * @brief lvalue of the second element of a vector.
     * @throws Runtime_error if D < 2.
     */
    double& y();

    /// @brief Changes the second element of a vector (if it exists).
    void set_y(double yv) ;

    /**
     * Returns the third element of a vector.  This method throws exception if D < 3.
     */
    double z() const ;

    /**
     * @brief lvalue of the third element of a vector.
     * @throws Runtime_error if D < 3.
     */
    double& z();

    /// @brief Changes the third element of a vector (if it exists).
    void set_z(double zv) ;

    /**
     * @brief read fourth element (i.e., index 3) of vector, if it exists.
     * @return copy of the value in the fourth element of the vector.
     * @throws Runtime_error if D < 4;
     */
    double w() const;

    /**
     * @brief lvalue of the fourth element of a vector (i.e., has index 3).
     * @throws Runtime_error if D < 4.
     */
    double& w();

    /// @brief Changes the fourth element of a vector (if it exists).
    void set_w(double wv);


    /// @brief assignment
    Vector_d& operator=(const Vector_d& other);

    /// @brief assignment from dynamic vector
    Vector_d& operator=(const Vector& other);


    Vector_d<D>& operator-=(const Vector_d<D>& second);
    Vector_d<D>& operator-=(double second);

    Vector_d<D>& operator+=(const Vector_d<D>& second);
    Vector_d<D>& operator+=(double second);

    Vector_d<D>& operator*=(double s);

    Vector_d<D>& operator/=(double s);

    /// @throws if n != D
    void resize(size_t n);

    double norm1() const;

    /// @brief return the squared l2-norm of this vector
    double magnitude_squared() const;

    /// @brief return the l2-norm of this vector
    double magnitude() const;

    /// @brief normalize this vector to have l2-norm of 1.0
    Vector_d<D>& normalize();

    /// @brief Non-mutating (functionally pure) version of normalize()
    Vector_d<D> normalized() const;

    Vector_d<D>& negate();

#ifdef KJB_HAVE_BST_SERIAL
    template <class Archive>
    void serialize(Archive &ar, const unsigned int /* version */);
#endif
};

// TYPEDEFS
typedef Vector_d<2> Vector2;
typedef Vector_d<3> Vector3;
typedef Vector_d<4> Vector4;



/// @brief multiply by static vector
template <std::size_t D>
Vector operator*(const Matrix& m, const Vector_d<D> v);

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
Vector_d<D> operator-(const Vector_d<D>& first, const Vector_d<D>& second);

template <std::size_t D>
Vector_d<D> operator-(const Vector_d<D>& first, double second);

template <std::size_t D>
Vector_d<D> operator-(double first, const Vector_d<D>& second);

template <std::size_t D>
Vector_d<D> operator-(const Vector_d<D>& first );

template <std::size_t D>
Vector_d<D> operator+(const Vector_d<D>& first, const Vector_d<D>& second);

template <std::size_t D>
Vector_d<D> operator+(const Vector_d<D>& first, double second);

template <std::size_t D>
Vector_d<D> operator+(double first, const Vector_d<D>& second);

template <std::size_t D>
Vector_d<D> operator*(const Vector_d<D>& v, double s);

template <std::size_t D>
Vector_d<D> operator*(double s, const Vector_d<D>& v);

template <std::size_t D>
Vector_d<D> operator/(const Vector_d<D>& v, double s);

template<std::size_t N> 
void swap(Vector_d<N>& first, Vector_d<N>& second);

// COMPARATORS
template<std::size_t N> 
bool operator==(const Vector_d<N>& first, const Vector_d<N>& second);

template<std::size_t N> 
bool operator!=(const Vector_d<N>& first, const Vector_d<N>& second);

template<std::size_t N> 
bool operator<(const Vector_d<N>& first, const Vector_d<N>& second);

template<std::size_t N> 
bool operator>(const Vector_d<N>& first, const Vector_d<N>& second);

template<std::size_t N> 
bool operator<=(const Vector_d<N>& first, const Vector_d<N>& second);

template<std::size_t N> 
bool operator>=(const Vector_d<N>& first, const Vector_d<N>& second);

// TYPEDEFS
typedef Vector_d<2> Vector2;
typedef Vector_d<3> Vector3;
typedef Vector_d<4> Vector4;

// MATH FUNCTIONS
template <size_t D>
double norm1(const kjb::Vector_d<D>& v);

template <size_t D>
double norm2(const kjb::Vector_d<D>& v);


// UTILITY FUNCTIONS
template <size_t D>
Vector_d<D> create_random_vector();

template <size_t D>
Vector_d<D> create_gauss_random_vector();

inline Vector2 create_random_vector2() 
{
    return create_random_vector<2>();
}

inline Vector3 create_random_vector3() 
{
    return create_random_vector<3>();
}

inline Vector4 create_random_vector4() 
{
    return create_random_vector<4>();
}

template <size_t D>
Vector_d<D> create_unit_vector(size_t d);

/// @pre v.size() == 2
Vector2 make_vector2(const Vector& v);

/// @pre v.size() == 3
Vector3 make_vector3(const Vector& v);

/// @pre v.size() == 4
Vector4 make_vector4(const Vector& v);


template <std::size_t D>
std::ostream& operator<<(std::ostream& out, const Vector_d<D>& v);

template <std::size_t D>
std::istream& operator>>(std::istream& ist, Vector_d<D>& v);

template <std::size_t D>
double dot(const Vector_d<D>& op1, const Vector_d<D>& op2);

/**
 * @brief   Compute the Euclidian distance between two vectors
 *
 * A routine that computes the distance between two vectors
 */
template <std::size_t D>
double vector_distance(const Vector_d<D>& op1, const Vector_d<D>& op2);

template <std::size_t D>
double vector_distance_squared(const Vector_d<D>& op1, const Vector_d<D>& op2);

/**
 * Compute cross product between two 3-vectors
 */
inline Vector3 cross(const Vector3& op1, const Vector3& op2)
{
    Vector3 out;
    out[0] = op1[1] * op2[2] - op1[2] * op2[1];
    out[1] = op1[2] * op2[0] - op1[0] * op2[2];
    out[2] = op1[0] * op2[1] - op1[1] * op2[0];
    return out;
}


/** @} */

} // namespace kjb

#endif

