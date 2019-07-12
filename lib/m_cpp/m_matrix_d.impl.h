/* $Id: m_matrix_d.impl.h 21596 2017-07-30 23:33:36Z kobus $ */
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

#ifndef KJB_M_MATRIX_D_IMPL_H
#define KJB_M_MATRIX_D_IMPL_H

#include <numeric>
#include <functional>
#include <cmath>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_vector_d.h>
#include "l_cpp/l_functors.h"
#include "l_cpp/l_cxx11.h"

namespace kjb {

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>::Matrix_d() :
    Base()
{ }

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>::Matrix_d(const Matrix_d& m) :
    Base(m)
{
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>::Matrix_d(const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& m) :
    Base()
{
    *this = m;
}


template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>::Matrix_d(const Matrix& m) :
    Base()
{
    *this = m;
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>::Matrix_d(double value) :
    Base()
{
    typename Base::iterator it;
    // call row.init(value) on all rows
    for(it = begin(); it < end(); ++it)
    {
        std::fill_n(it->begin(), it->size(), value);
    }
}


template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS, NCOLS, TRANSPOSED>& Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator=(const Matrix_d<NCOLS, NROWS, !TRANSPOSED>& other)
{
    return assignment_dispatch_(other);
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>& Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator=(const Matrix& other)
{
    if (
             other.get_num_rows() != (Matrix::Size_type) get_num_rows() ||
             other.get_num_cols() != (Matrix::Size_type) get_num_cols() )
    {
        KJB_THROW(Dimension_mismatch);
    }

    return assignment_dispatch_(other);
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>& Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator-=(const Matrix& second)
{
    if (second.get_num_rows() != (Matrix::Size_type) get_num_rows() ||
        second.get_num_cols() != (Matrix::Size_type) get_num_cols())
    {
        KJB_THROW(Dimension_mismatch);
    }

    return minus_equals_dispatch_(second);
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED> Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator-() const
{
    Matrix_d result;
    for (size_t row = 0; row < NROWS; row++)
    {
        for (size_t col = 0; col < NCOLS; col++)
        {
            result[row][col] = -(*this)[row][col];
        }
    }
    return result;
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>& Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator+=(const Matrix& second)
{
    if (second.get_num_rows() != (Matrix::Size_type) get_num_rows() ||
       second.get_num_cols() != (Matrix::Size_type) get_num_cols())
    {
        KJB_THROW(Dimension_mismatch);
    }

    return plus_equals_dispatch_(second);
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED> Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator+(const Matrix& second) const
{
    return Matrix_d(*this) += second;
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
Matrix_d<NROWS,NCOLS,TRANSPOSED>& Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator*=(double s)
{
    for (size_t i = 0; i < NROWS; i++)
    {
        (*this)[i] *= s;
    }

    return *this;
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
template <size_t D>
void Matrix_d<NROWS,NCOLS,TRANSPOSED>::set_row(size_t r, const Vector_d<D>& row)
{
    KJB_STATIC_ASSERT((D == Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_cols), "Dimension mismatch");
    for(size_t c = 0; c < num_cols; ++c)
    {
        if(!TRANSPOSED)
        {
            (*this)[r][c] = row[c];
        }
        else
        {
            (*this)[c][r] = row[c];
        }
    }
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
template <size_t D>
void Matrix_d<NROWS,NCOLS,TRANSPOSED>::set_col(size_t c, const Vector_d<D>& col)
{
    KJB_STATIC_ASSERT((D == Matrix_d<NROWS,NCOLS,TRANSPOSED>::num_rows), "Dimension mismatch");
    for(size_t r = 0; r < num_rows; ++r)
    {
        if(!TRANSPOSED)
        {
            (*this)[r][c] = col[r];
        }
        else
        {
            (*this)[c][r] = col[r];
        }
    }
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
bool Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator==(const Matrix& op2) const
{
    if ((Matrix::Size_type) get_num_rows() != op2.get_num_rows()) 
    {
        return false;
    }
    if ((Matrix::Size_type) get_num_cols() != op2.get_num_cols()) 
    {
        return false;
    }

    return matrix_equality_dispatch_(op2);
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
bool Matrix_d<NROWS,NCOLS,TRANSPOSED>::operator!=(const Matrix& op2) const
{
    return !(*this == op2);
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
template <class Matrix_type>
Matrix_d<NROWS,NCOLS,TRANSPOSED>& Matrix_d<NROWS,NCOLS,TRANSPOSED>::assignment_dispatch_(Matrix_type& other)
{
    // assume dimension checks have already occurred
    
    // this should still work for if _this_ is transposed.
    for (size_t row = 0; row < get_num_rows(); ++row)
    {
        for (size_t col = 0; col < get_num_cols(); ++col)
        {
            (*this)(row, col) = other(row, col);
        }
    }

    return *this;
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
template <class Matrix_op>
Matrix_d<NROWS,NCOLS,TRANSPOSED>& Matrix_d<NROWS,NCOLS,TRANSPOSED>::plus_equals_dispatch_(const Matrix_op& second)
{
    // assume dimension checks have occurred already
    for (size_t row = 0; row != get_num_rows(); row++)
    {
        for (size_t col = 0; col != get_num_cols(); col++)
        {
            (*this)(row, col) += second(row, col);
        }
    }

    return *this;
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
template <class Matrix_op>
Matrix_d<NROWS,NCOLS,TRANSPOSED>& Matrix_d<NROWS,NCOLS,TRANSPOSED>::minus_equals_dispatch_(const Matrix_op& second)
{
    // assume dimension checks have occurred already
    for (size_t row = 0; row != get_num_rows(); row++)
    {
        for (size_t col = 0; col != get_num_cols(); col++)
        {
            (*this)(row, col) -= second(row, col);
        }
    }

    return *this;
}

template <class Matrix_type_1,  class Matrix_type_2>
Matrix_d<Matrix_type_1::num_rows, Matrix_type_2::num_cols> matrix_multiply_dispatch_(const Matrix_type_1& m1, const Matrix_type_2& m2)
{
    KJB_STATIC_ASSERT(Matrix_type_1::num_cols == Matrix_type_2::num_rows, "Dimension mismatch");

    static const size_t OUT_ROWS = Matrix_type_1::num_rows;
    static const size_t IN_ROWS = Matrix_type_1::num_cols;
    static const size_t IN_COLS= Matrix_type_2::num_cols;

    Matrix_d<OUT_ROWS, IN_COLS> result(0.0);

    for (size_t out_row = 0; out_row < OUT_ROWS; ++out_row)
    {
        for (size_t in_row  = 0; in_row  < IN_ROWS; ++in_row)
        {
            for (size_t in_col  = 0; in_col  < IN_COLS; ++in_col)
            {
                result(out_row, in_col) += m1(out_row, in_row) * m2(in_row, in_col);
            }
        }
    }

    return result;
}

template <size_t NROWS, size_t NCOLS, bool TRANSPOSED>
template <class Matrix2>
bool Matrix_d<NROWS,NCOLS,TRANSPOSED>::matrix_equality_dispatch_(const Matrix2& m2) const
{
    // assume dimension checks have been done
    for (size_t row = 0; row != get_num_rows(); row++)
    {
        for (size_t col = 0; col != get_num_cols(); col++)
        {
            if ((*this)(row,col) != m2(row, col)) return false;
        }
    }

    return true;
}

/// don't call me directly
template<class Matrix_1, class Matrix_2>
Matrix matrix_multiply_dynamic_dispatch_(const Matrix_1& m1, const Matrix_2& m2)
{
    if ((Matrix::Size_type) m1.get_num_cols() != (Matrix::Size_type) m2.get_num_rows())
    {
        KJB_THROW(Dimension_mismatch);
    }

    static size_t OUT_ROWS = m1.get_num_rows();
    static size_t IN_ROWS = m2.get_num_rows();
    static size_t IN_COLS = m2.get_num_cols();

    // assume all checks have already occurred
    Matrix result(OUT_ROWS, IN_COLS, 0);

    for (size_t out_row = 0; out_row < OUT_ROWS; ++out_row)
    {
        for (size_t in_row  = 0; in_row  < IN_ROWS; ++in_row)
        {
            for (size_t in_col  = 0; in_col  < IN_COLS; ++in_col)
            {
                result(out_row, in_col) += m1(out_row, in_row) * m2(in_row, in_col);
            }
        }
    }

    return result;
}


template<std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix_d<NROWS, NCOLS, TRANSPOSED>
operator*(double scalar, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& mat)
{
    return Matrix_d<NROWS, NCOLS, TRANSPOSED>(mat) *= scalar;
}

template<std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix operator*(const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1, const Matrix& op2)
{
    return matrix_multiply_dynamic_dispatch_(op1, op2);
}

template<std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix operator*(const kjb::Matrix& op1, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2)
{
    return matrix_multiply_dynamic_dispatch_(op1, op2);
}


template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline bool operator==(const Matrix& op2, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1)
{
    return op1 == op2;
}

template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline bool operator!=(const Matrix& op2, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op1)
{
    return op1 != op2;
}

template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix_d<NROWS, NCOLS, TRANSPOSED> operator-(const Matrix op1, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2)
{
    return -op2 + op1;
}

template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
inline Matrix_d<NROWS, NCOLS, TRANSPOSED> operator+(const Matrix& second, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& op2)
{
    return Matrix_d<NROWS, NCOLS, TRANSPOSED>(op2) += second;
}

template <std::size_t NROWS, std::size_t NCOLS, bool TRANSPOSED>
std::ostream& operator<<(std::ostream& ost, const Matrix_d<NROWS, NCOLS, TRANSPOSED>& mat)
{
    for (size_t r = 0; r < mat.get_num_rows(); ++r)
    {
        for (size_t c = 0; c < mat.get_num_cols(); ++c)
        {
            ost << mat(r,c) << '\t';
        }
        ost << '\n';
    }
    return ost;
}

template <std::size_t D>
Matrix_d<D,D> create_identity_matrix()
{
    Matrix_d<D,D> result(0.0);

    for (size_t i = 0; i < D; i++)
    {
        result(i,i) = 1.0;
    }

    return result;
}

template <std::size_t D>
inline Matrix_d<D, D> outer_product(const Vector_d<D>& v1, const Vector_d<D>& v2)
{
    // these are copied in as row vectors
    Matrix_d<1,D> m1(&v1);
    Matrix_d<1,D> m2(&v2);

    return m1.transpose() * m2;
}

template <std::size_t NROWS, std::size_t NCOLS>
Matrix_d<NROWS,NCOLS> create_random_matrix()
{
    Matrix_d<NROWS,NCOLS> result(0.0);

    for(size_t i = 0; i < result.size(); ++i)
        result[i] = create_random_vector<NCOLS>();

    return result;
}

template <size_t D>
double trace(const Matrix_d<D,D>& m)
{
    double out = 0;
    for(size_t i = 0; i < D; ++i)
    {
        out += m(i,i);
    }
    return out;
}

template <std::size_t R, std::size_t C, bool T>
double accumulate(const Matrix_d<R,C, T>& mat, double init)
{
    double result = init;
    for (size_t r = 0; r < R; r++)
    {
        for (size_t c = 0; c < C; c++)
        {
            result += mat(r,c);
        }
    }

    return result;
}

template <std::size_t R, std::size_t C>
double max_abs_difference( const Matrix_d<R,C,false>& op1, const Matrix_d<R,C,false>& op2 )
{
    Matrix_d<R,C,false> diff = op1 - op2;

    double out = -DBL_MAX;
    for(size_t r = 0; r < R; ++r)
    for(size_t c = 0; c < C; ++c)
    {
        out = std::max(fabs(diff(r,c)), out);
    }
    return out;
}

template <std::size_t R, std::size_t C, bool T>
inline double max(const Matrix_d<R,C, T>& mat)
{
    return accumulate(mat, DBL_MIN, kjb::maximum<double>());
}

template <std::size_t R, std::size_t C, bool T>
inline double min(const Matrix_d<R,C, T>& mat)
{
    return accumulate(mat, DBL_MAX, kjb::minimum<double>());
}

} // namespace kjb

#endif /* KJB_M_MATRIX_D_IMPL_H */
