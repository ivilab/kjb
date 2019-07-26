/* $Id: m_matrix_d.h 21776 2017-09-17 16:44:49Z clayton $ */
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

#ifndef KJB_M_MATRIX_D_H
#define KJB_M_MATRIX_D_H


#include "l_cpp/l_exception.h"
#include "l_cpp/l_cxx11.h"
#include "m_cpp/m_vector_d.h"

#ifdef TEST
#include <m_cpp/m_concept.h>
#endif 

#include <boost/array.hpp>

#include <algorithm>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>
#endif

namespace kjb
{


class Vector;
class Matrix;

/**
 * @ingroup kjbLinearAlgebra
 * @{
 */

/**
 * Statically-allocated matrix.  More limited in functionality than kjb::Matrix,
 * but can offer significantly better performance, because no operations
 * require heap allocation.  Transposes are also handle statically, so a
 * matrix can be transposed and used in operators without a copy and zero 
 * runtime overhead--the compiler simply calls a different version of the
 * operation.  
 *
 * An added benefit is that all dimension mis-matches are caught at compile time. 
 *
 * @note This is implemented as an array of Vector_d objects, which offers 
 * convenient semantics in some situations, especially with STL algorithms.
 *
 * @warning The transposed state of a matrix should be transparent to the user.  For that reasons, square-bracket operator is generally discouraged, as it doesn't 
 * respect the transposed state.  In other words, for transposed matrices, m(i,j) != m[i][j].
 *
 * @tparam NROWS number of rows in matrix
 * @tparam NCOLS number of columns in matrix
 * @tparam TRANSPOSED swap meaning of rows and columns.  You'll rarely set this manually; rather the "transpose()" method will set this.  It's actually a bit confusing to work in this mode deliberately, because the meaning of NROWS and NCOLS is swapped.  Just use m.transpose() and trust the implementation ;-)
 */
template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED = false>
class Matrix_d : public boost::array<Vector_d<NCOLS>, NROWS>
{
    typedef Matrix_d Self;
    typedef boost::array<Vector_d<NCOLS>, NROWS> Base;

public:
    template <bool condition, std::size_t value1, std::size_t value2>
    struct if_then_ { static const std::size_t value = value1;};

    template <std::size_t value1, std::size_t value2>
    struct if_then_<false, value1, value2>{ static const std::size_t value = value2;};

    static const std::size_t num_cols = if_then_<TRANSPOSED, NROWS, NCOLS>::value;
    static const std::size_t num_rows = if_then_<TRANSPOSED, NCOLS, NROWS>::value;

  // types
    using typename Base::value_type;
    using typename Base::iterator;
    using typename Base::const_iterator;
    using typename Base::reverse_iterator;
    using typename Base::const_reverse_iterator;
    using typename Base::reference;
    using typename Base::const_reference;
    using typename Base::size_type;
    using typename Base::difference_type;

    using Base::begin;
    using Base::end;
    using Base::rbegin;
    using Base::rend;
    using Base::size;
    using Base::empty;
    using Base::max_size;
    using Base::operator[];
    using Base::at;
    using Base::front;
    using Base::back;
    using Base::data;
    using Base::c_array;
    using Base::swap;
    using Base::assign;


    Matrix_d();
    Matrix_d(const Matrix_d& m);
    Matrix_d(const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& m);

    /**
     * @brief Fill constructor
     */
    template <class Iterator>
    explicit Matrix_d(Iterator begin) :
        Base()
    {
#ifdef TEST
        typedef typename std::iterator_traits<Iterator>::value_type RowVector;

        BOOST_CONCEPT_ASSERT((kjb::SimpleVector<RowVector>));
#endif /*TEST */

        Iterator it = begin;
        typename Base::iterator oit = this->begin();
        for (size_t i = 0; i < NROWS; ++i, ++it, ++oit)
        {
            if (it->size() != NCOLS)
                KJB_THROW(Dimension_mismatch);
            std::copy(it->begin(), it->end(), oit->begin());
            
        }
    }

    /**
     * @brief Construct from "dynamic" vector.
     */
    explicit Matrix_d(const Matrix& m);

    // @brief initialize all elements with value init
    explicit Matrix_d(double value);

//    template <class VectorType>
//    Matrix_d(const VectorType& row) :
//        Base()
//    {
//        KJB_STATIC_ASSERT(NROWS == 1, "Constructor invalid for NROWS!=1");
//
//#ifdef TEST
//        BOOST_CONCEPT_ASSERT((kjb::SimpleVector<VectorType>));
//#endif //TEST
//
//        assert(row.size() == NCOLS);
//
//        std::copy(row.begin(), row.end(), (*this)[0].begin());
//    }
//
//
//    /**
//     * Construct a Matrix from two rows.  This constructor only exists if NROWS == 2.  Both rows must have size == NCOLS
//     *
//     * @note in transpose mode, r1 and r2 represent columns
//     *
//     * @pre r1.size() == r2.size == NCOLS
//     */
//    template <class VectorType>
//    Matrix_d( const VectorType& r1, const VectorType& r2) :
//        Base()
//    {
//        KJB_STATIC_ASSERT(NROWS == 2, "Constructor invalid for NROWS!=2");
//
//#ifdef TEST
//        BOOST_CONCEPT_ASSERT((kjb::SimpleVector<VectorType>));
//#endif // TEST
//
//        assert(r1.size() == NCOLS);
//        assert(r2.size() == NCOLS);
//
//        std::copy(r1.begin(), r1.end(), (*this)[0].begin());
//        std::copy(r2.begin(), r2.end(), (*this)[1].begin());
//    }
//
//    /**
//     * Construct a Matrix from three rows.  This constructor only exists if NROWS == 3.  All rows must have size == NCOLS
//     *
//     * @note in transpose mode, r1 and r2 represent columns
//     *
//     * @pre r*.size() == NCOLS
//     */
//    template <class VectorType>
//    Matrix_d(
//            const VectorType& r1,
//            const VectorType& r2,
//            const VectorType& r3) :
//        Base()
//    {
//        KJB_STATIC_ASSERT(NROWS == 3, "Constructor invalid for NROWS!=3");
//#ifdef TEST
//        BOOST_CONCEPT_ASSERT((kjb::SimpleVector<VectorType>));
//#endif // TEST
//
//        assert(r1.size() == NCOLS);
//        assert(r2.size() == NCOLS);
//        assert(r3.size() == NCOLS);
//
//        std::copy(r1.begin(), r1.end(), (*this)[0].begin());
//        std::copy(r2.begin(), r2.end(), (*this)[1].begin());
//        std::copy(r3.begin(), r3.end(), (*this)[2].begin());
//    }
//
//
//    /**
//     * Construct a Matrix from four rows.  This constructor only exists if NROWS == 4.  All rows must have size == NCOLS
//     *
//     * @note in transpose mode, r1 and r2 represent columns
//     *
//     * @pre r*.size() == NCOLS
//     */
//    template <class VectorType>
//    Matrix_d(
//            const VectorType& r1,
//            const VectorType& r2,
//            const VectorType& r3,
//            const VectorType& r4) :
//        Base()
//    {
//        KJB_STATIC_ASSERT(NROWS == 4, "Constructor invalid for NROWS!=4");
//#ifdef TEST
//        BOOST_CONCEPT_ASSERT((kjb::SimpleVector<VectorType>));
//#endif // TEST
//
//        assert(r1.size() == NCOLS);
//        assert(r2.size() == NCOLS);
//        assert(r3.size() == NCOLS);
//        assert(r4.size() == NCOLS);
//
//        std::copy(r1.begin(), r1.end(), (*this)[0].begin());
//        std::copy(r2.begin(), r2.end(), (*this)[1].begin());
//        std::copy(r3.begin(), r3.end(), (*this)[2].begin());
//        std::copy(r4.begin(), r4.end(), (*this)[3].begin());
//    }

    /**
     * Implemented for compatibility with template functions that
     * also operate on dynamic matrices.  If rows != get_num_rows()
     * or cols != get_num_cols(),
     * this will be considered a bug and will trigger an abort.
     * @pre rows == get_num_rows();
     * @pre cols == get_num_cols();
     */
    void resize(size_t rows, size_t cols)
    {
        assert(rows == get_num_rows());
        assert(cols == get_num_cols());

        // CTM following turns off unused parameters warning in clang
        (void) rows;
        (void) cols;
    }

    /**
     * NOT a constant-time operation!  This will most likely
     * perform one stack allocation and two assignments, like
     * the default swap operation.  This is because data is 
     * stored on the stack, so pointers can't be moved around.
     */
    Matrix_d& swap(Matrix_d& other)
    {
        swap(other);
        return *this;
    }

    inline Matrix_d<NROWS, NCOLS, !TRANSPOSED>& transpose()
    {
        return static_cast<Matrix_d<NROWS, NCOLS, !TRANSPOSED>&>(
               static_cast<Base&>(*this));
//        return reinterpret_cast<Matrix_d<NROWS, NCOLS, !TRANSPOSED>&>(*this);
    }

    inline const Matrix_d<NROWS, NCOLS, !TRANSPOSED>& transpose() const
    {
        return static_cast<const Matrix_d<NROWS, NCOLS, !TRANSPOSED>&>(
               static_cast<const Base&>(*this));
//        return reinterpret_cast<const Matrix_d<NROWS, NCOLS, !TRANSPOSED>&>(*this);
    }

    /// @brief assignment
    inline Matrix_d& operator=(const Matrix_d& other)
    {
        Base::operator=(other);
        return *this;
    }

    /// @brief assignment
    Matrix_d& operator=(const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& other);

    /**
     * @brief assignment from dynamic matrix
     */
    Matrix_d& operator=(const Matrix& other);

    inline Matrix_d& operator-=(const Matrix_d& second)
    {
        return minus_equals_dispatch_(second);
    }

    inline Matrix_d operator-(const Matrix_d& second) const
    {
        return Matrix_d(*this) -= second;
    }

    inline Matrix_d& operator-=(const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& second)
    {
        return minus_equals_dispatch_(second);
    }

    inline Matrix_d operator-(const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& second) const
    {
        return Matrix_d(*this) -= second;
    }

    inline Matrix_d& operator-=(const Matrix& second);

    inline Matrix_d operator-(const Matrix& second) const
    {
        return Matrix_d(*this) -= second;
    }

    /// negation
    Matrix_d operator-() const;


    inline Matrix_d& operator+=(const Matrix_d& second)
    {
        return plus_equals_dispatch_(second);
    }

    inline Matrix_d operator+(const Matrix_d& second) const
    {
        return Matrix_d(*this) += second;
    }

    inline Matrix_d& operator+=(const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& second)
    {
        return plus_equals_dispatch_(second);
    }

    inline Matrix_d operator+(const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& second) const
    {
        return Matrix_d(*this) += second;
    }

    inline Matrix_d& operator+=(const Matrix& second);

    inline Matrix_d operator+(const Matrix& second) const;

    // STATIC MATRIX-MATRIX MULTIPLICATION
    // the next two methods handle the four special cases separately:
    //  1. normal * normal
    //  2. normal * transposed
    //  3. transposed * normal
    //  4. transposed * transposed
    //
    //  Cases 1 and 3 are handled by the first signature, 2 and 4 are handled by the second case.
    //  Dimension mismatches should be caught at compile time.
    //
    //  These should all be inlined, and all special cases should be resolved at compile-time, so no overhead should occur for handling these indivisually.
    //
    //  Possible improvements:
    //      A. using dot-product for cases 2 and 3
    //      B. Handle all cases in a single signature?

    /** Multiplying by a normal (non-transposed) matrix */
    template <std::size_t IN_COLS>
    Matrix_d<num_rows, IN_COLS> operator*(const Matrix_d<num_cols, IN_COLS, false>& second) const
    {
        return matrix_multiply_dispatch_(*this, second);
    }

    /** Multiplying by a transposed matrix */
    template <std::size_t IN_COLS>
    Matrix_d<num_rows, IN_COLS> operator*(const Matrix_d<IN_COLS, num_cols, true>& second) const
    {
        return matrix_multiply_dispatch_(*this, second);
    }


    /// @brief multiplication by a scalar
    Matrix_d& operator*=(double s);

    /**
     * Multiply and store in self.
     *
     * @note This is for convenience only, and has no performance 
     * benefit over a = a * b, since a temporary is still necessary 
     * to hold the result during the computation.
     *
     * @note This method will only be available if matrix is square, i.e. NROWS == NCOLS.
     */
    template <std::size_t IN_ROWS, bool IN_TRANSPOSED>
    Matrix_d& operator*=(const Matrix_d<IN_ROWS, IN_ROWS, IN_TRANSPOSED>& other)
    {
        KJB_STATIC_ASSERT(IN_ROWS == NROWS && IN_ROWS == NCOLS, "Dimension mismatch");
        return *this = *this * other;

    }

    inline Matrix_d operator*(double s) const
    {
        return Matrix_d(*this) *= s;
    }

    /**
     * multiplication by a vector.
     *
     * @note Dimensions must match for this method to exist.
     */
    // This is easier to implement as a member as opposed to a free
    // function, due to need to access static members of Self.
    Vector_d<num_rows> operator*(const Vector_d<num_cols>& v) const
    {
        Vector_d<num_rows> result(0.0);
        if (TRANSPOSED)
        {
            // should be better cache consistency indexing in this order
            for (size_t col = 0; col < get_num_cols(); col++)
            for (size_t row = 0; row < get_num_rows(); row++)
                result[row] += (*this)(row, col) * v[col];
        }
        else
        {
            for (size_t row = 0; row < get_num_rows(); row++)
            for (size_t col = 0; col < get_num_cols(); col++)
                result[row] += (*this)(row, col) * v[col];
        }

        return result;
    }

    inline Matrix_d& operator/=(double s)
    {
        return (*this) *= 1/s;
    }

    inline Matrix_d operator/(double s) const
    {
        return Matrix_d(*this) /= s;
    }

    static inline size_t get_num_rows()
    {
        return num_rows;
    }

    static inline size_t get_num_cols()
    {
        return num_cols;
    }

    inline double& operator()(size_t row, size_t col)
    {
        // "if" is optimized out
        if (TRANSPOSED)
        {
            return (*this)[col][row];
        }
        else
        {
            return (*this)[row][col];
        }
    }

    inline const double& operator()(size_t row, size_t col) const
    {
        // "if" is optimized out
        if (TRANSPOSED)
        {
            return (*this)[col][row];
        }
        else
        {
            return (*this)[row][col];
        }
    }

    Vector_d<num_cols> get_row(size_t r) const
    {
        Vector_d<num_cols> row;
        for(size_t c = 0; c < num_cols; ++c)
        {
            row[c] = TRANSPOSED ? (*this)[c][r] : (*this)[r][c];
        }

        return row;
    }

    template <size_t D>
    void set_row(size_t r, const Vector_d<D>& row);

    Vector_d<num_rows> get_col(size_t c) const
    {
        Vector_d<num_rows> col;
        for(size_t r = 0; r < num_rows; ++r)
        {
            col[r] = TRANSPOSED ? (*this)[c][r] : (*this)[r][c];
        }

        return col;
    }

    template <size_t D>
    void set_col(size_t c, const Vector_d<D>& col);

    inline bool operator==( const Matrix_d& op2) const
    {
        return matrix_equality_dispatch_(op2);
    }

    inline bool operator==( const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& op2) const
    {
        return matrix_equality_dispatch_(op2);
    }

    bool operator==(const Matrix& op2) const;

    inline bool operator!=(const Matrix_d& op2) const
    {
        return !((*this) == op2);
    }

    inline bool operator!=( const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& op2) const
    {
        return !((*this) == op2);
    }

    inline bool operator!=(const Matrix& op2) const;

private:
    ///////////////////////////////////////////////////////////
    // DISPATCH METHODS
    //
    // These should work the same for normal and transposed
    // static matrices and for dynamic kjb::Matrix's.
    //
    // The all assume that dimension checking has already occurred by
    // the caller.
    //
    //////////////////////////////////////////////////////////
    template <class Matrix_type>
    Matrix_d& assignment_dispatch_(Matrix_type& other);

    template <class Matrix_op>
    Matrix_d& plus_equals_dispatch_(const Matrix_op& second);

    template <class Matrix_op>
    Matrix_d& minus_equals_dispatch_(const Matrix_op& second);


    template <class Matrix2>
    bool matrix_equality_dispatch_(const Matrix2& m2) const;


#ifdef KJB_HAVE_BST_SERIAL
    template <class Archive>
    void serialize(Archive &ar, const unsigned int /* version */)
    {
        ar & ::boost::serialization::base_object<Base>(*this);
    }
#endif
};

typedef Matrix_d<2,2> Matrix2;
typedef Matrix_d<3,3> Matrix3;
typedef Matrix_d<4,4> Matrix4;

///////////////////////////
// SCALAR OPERATIONS
///////////////////////////

template<std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix_d<NROWS, NCOLS, TRANSPOSED>
operator*(double scalar, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& mat);

///////////////////////////
// Matrix multiplication
///////////////////////////
//
/**
 * Utility dispatch for m1 * m2 operator.  Assumes compile-time dimension checks
 * have already occurred.
 *
 * Don't call me directly!
 */
template <class Matrix_type_1, class Matrix_type_2>
Matrix_d<Matrix_type_1::num_rows, Matrix_type_2::num_cols> matrix_multiply_dispatch_(const Matrix_type_1& m1, const Matrix_type_2& m2);


template<std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix operator*(const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1, const Matrix& op2);

template<std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix operator*(const kjb::Matrix& op1, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2);


template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline bool operator==(const Matrix& op2, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1);

template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline bool operator!=(const Matrix& op2, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1);

template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix_d<NROWS, NCOLS, TRANSPOSED> operator-(const Matrix op1, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2);

template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix_d<NROWS, NCOLS, TRANSPOSED> operator+(const Matrix& second, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2);

//////////////////////////////////////////////////////////
//  I/O functions
//////////////////////////////////////////////////////////
template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
std::ostream& operator<<(std::ostream& ost, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& mat);

//////////////////////////////////////////////////////////
//  Utility functions
//////////////////////////////////////////////////////////

template <std::size_t D>
Matrix_d<D,D> create_identity_matrix();

/**
 * compute the outer produce of two vectors (v1' * v2)
 */
template <std::size_t D>
inline Matrix_d<D, D> outer_product(const Vector_d<D>& v1, const Vector_d<D>& v2);

template <std::size_t NROWS, std::size_t NCOLS>
Matrix_d<NROWS,NCOLS> create_random_matrix();

template <size_t D>
double trace(const Matrix_d<D,D>& m);

//////////////////////////////////////
// A few hard-coded numerical functions
/////////////////////////////////////

Matrix_d<2,2> matrix_inverse(const Matrix_d<2,2>& m);

Matrix_d<3,3> matrix_inverse(const Matrix_d<3,3>& m);

/** @brief get eigenvalues of symmetric matrix A in ascending order */
void symmetric_eigs(const kjb::Matrix3& A, double& eig1, double& eig2, double& eig3);

/**
 * Add elements of a matrix
 */
template <std::size_t R, std::size_t C, bool T>
double accumulate(const Matrix_d<R,C, T>& mat, double init);

/**
 * apply binary function to all elements of a matrix (same as std::accumulate, treating matrix like a long vector)
 */
template <std::size_t R, std::size_t C, bool T, class Binary_function>
double accumulate(const Matrix_d<R,C, T>& mat, double init,
        Binary_function binary_op)
{
    double result = init;
    for (size_t r = 0; r < R; r++)
    {
        for (size_t c = 0; c < C; c++)
        {
            result = binary_op(result, mat(r,c));
        }
    }

    return result;
}

template <std::size_t R, std::size_t C>
double max_abs_difference( const Matrix_d<R,C,false>& op1, const Matrix_d<R,C,false>& op2 );

/**
 * returns the maximum element of a matrix
 */
template <std::size_t R, std::size_t C, bool T>
double max(const Matrix_d<R,C, T>& mat);

/**
 * returns the minimum element of a matrix
 */
template <std::size_t R, std::size_t C, bool T>
double min(const Matrix_d<R,C, T>& mat);

/*
 * NOTE TO IMPLEMENTERS
 *
 * If oyu want to add new generic free functions, please consider moving
 * the implementation into m_matrix_d.impl.h, and adding it to the list
 * of explicit instantiations in m_matrix_d.cpp.
 *
 * This should help keep the compile-time for this file low when common
 * matrix sizes are used.
 */

/** @} */

} // namespace kjb

//#include <m_cpp/m_matrix_d.impl.h>
#endif
