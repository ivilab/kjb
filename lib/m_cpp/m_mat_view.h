/* $Id: m_mat_view.h 21596 2017-07-30 23:33:36Z kobus $ */
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

#ifndef KJB_M_CPP_MAT_VIEW
#define KJB_M_CPP_MAT_VIEW

#include "l_cpp/l_exception.h"
#include "l_cpp/l_index.h"

namespace kjb
{

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

/**
 * Represents a filtered "view" of another matrix.  This object
 * has the same semantics as a real Matrix, but altering it will
 * alter the "visible" elements of the underlying matrix.  Here's 
 * an example
 *
 *   Matrix mat = create_identity_matrix();
 *   Matrix_view mat_view = mat(_,3);
 *   mat_view *= -1;
 *   assert(mat(2,2) == -1);
 *
 * This may seem like a roundabout way of doing it, but when we 
 * put it all on one line, the power of this approach becomes obvous:
 *
 *   mat(_,3) *= -1;
 *
 * Eventually, we would like to support matlab-like syntax using strings:
 *
 *  mat("2:8", 3) *= -1;
 */
template <class Matrix_type>
class Generic_matrix_view
{
public:
    typedef Generic_matrix_view Self;
    typedef typename Matrix_type::Value_type Value_type;
    typedef typename Matrix_type::Vec_type   Vec_type;
    typedef int Size_type;

    Generic_matrix_view(
            Matrix_type& mat,
            const Index_range& row_indices,
            const Index_range& col_indices) :
        row_indices_(row_indices),
        col_indices_(col_indices),
        mat_(mat)
    {
    }

    Generic_matrix_view(Matrix_type& mat) :
        row_indices_(true), // "all" type range
        col_indices_(true), // "all" type range
        mat_(mat)
    {
    }

    Generic_matrix_view(const Generic_matrix_view& src) :
        row_indices_(src.row_indices_), // "all" type range
        col_indices_(src.col_indices_), // "all" type range
        mat_(src.mat_)
    {
    }

    // ----------------------
    // MATRIX-LIKE INTERFACE:
    // ----------------------

    bool operator==(const Self& other)
    {
        return equality(other);
    }

    bool operator==(const Matrix_type& other)
    {
        return equality(other);
    }

    bool operator!=(const Self& other)
    {
        return !equality(other);
    }

    bool operator!=(const Matrix_type& other)
    {
        return !equality(other);
    }

    /**
     * @warning Bounds-checking is enabled, so expect a performance hit
     */
    Value_type& operator()(int r, int c) 
    { 
        int row = row_indices_[r];
        int col = col_indices_[c];
        return mat_.at(row, col);
    }

    /**
     * @warning Bounds-checking is enabled, so expect a performance hit
     */
    Value_type  operator()(int r, int c) const 
    { 
        int row = row_indices_[r];
        int col = col_indices_[c];
        return mat_.at(row,col); 
    }

    Value_type& operator()(int i)
    {
        int row, col;
        compute_row_col( i, &row, &col );
        return operator()(row, col);
    }

    Value_type operator()(int i) const
    {
        int row, col;
        compute_row_col( i, &row, &col );
        return operator()(row, col);

    }

    Value_type& operator[](int i)
    {
        return operator()(i);
    }

    Value_type operator[](int i) const
    {
        return operator()(i);
    }

    Size_type get_num_rows() const
    {
        // "all"-type ranges can't know their size, so we provide it
        if(row_indices_.all()) return mat_.get_num_rows();
        return row_indices_.size();
    }

    Size_type get_num_cols() const
    {
        // "all"-type ranges can't know their size, so we provide it
        if(col_indices_.all()) return mat_.get_num_cols();
        return col_indices_.size();
    }

    Self& operator=(const Matrix_type& mat)
    {
        if(&mat_ == &mat)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return assign(Matrix_type(mat));
        }

        return assign(mat);
    }

    Self& operator=(const Self& mat)
    {
        // do they refer to the same matrix?
        if(&mat_ == &mat.mat_)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return assign(Matrix_type(mat));
        }
        return assign(mat);
    }
 
    /**
     * Set all elements to s
     */
    Self& operator=(const Value_type& s)
    {
        const int NUM_ROWS = get_num_rows();
        const int NUM_COLS = get_num_cols();

        for(int row = 0; row < NUM_ROWS; row++)
        for(int col = 0; col < NUM_COLS; col++)
        {
            operator()(row, col) = s;
        }

        return *this;
    }
    /**
     * If this object has only 1 column or 1 row, set it equal to 
     * v or v', respectively.
     */
    Self& operator=(const Vec_type& v)
    {
        return assign_from_vector(v);
    }

    /**
     * If this object has only 1 column or 1 row, set it equal to 
     * v or v', respectively.
     */
    Self& operator=(const std::vector<double>& v)
    {
        return assign_from_vector(v);
    }

    /**
     * If this object has only 1 column or 1 row, set it equal to 
     * v or v', respectively.
     */
    Self& operator=(const std::vector<float>& v)
    {
        return assign_from_vector(v);
    }

// Disabled these, since you have to cast to a Matrix anyway, just
// let implicit conversion handle it.
//
//    Matrix_type operator*(Value_type s)
//    {
//        return Matrix(*this) *= s;
//    }

//    Matrix_type operator*(const Matrix_view& s)
//    {
//        return Matrix(*this) * Matrix(s);
//    }

    Self& operator*=(Value_type s)
    {
        const int NUM_ROWS = get_num_rows();
        const int NUM_COLS = get_num_cols();
        for(int row = 0; row < NUM_ROWS; row++)
        for(int col = 0; col < NUM_COLS; col++)
            operator()(row, col) *= s;

        return *this;
    }

    /*
     * This doesn't make sense, since the output dimension could be different from input dimension
    Self& operator*=(const Matrix_view& s)
    {
    }
     */

    Self& operator/=(Value_type s)
    {
        const int NUM_ROWS = get_num_rows();
        const int NUM_COLS = get_num_cols();
        for(int row = 0; row < NUM_ROWS; row++)
        for(int col = 0; col < NUM_COLS; col++)
            operator()(row, col) /= s;

        return *this;
    }

    Self& operator+=(const Self& s)
    {
        // do they refer to the same matrix?
        if(&mat_ == &s.mat_)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return plus_equals(Matrix_type(s));
        }
        return plus_equals(s);
    }

    Self& operator+=(const Matrix_type& s)
    {
        return plus_equals(s);
    }

    Self& operator-=(const Self& s)
    {
        // do they refer to the same matrix?
        if(&mat_ == &s.mat_)
        {
            // avoid modifying what we're reading from
            // copy first, then assign
            return minus_equals(Matrix_type(s));
        }
        return minus_equals(s);
    }

    Self& operator-=(const Matrix_type& s)
    {
        return minus_equals(s);
    }
//
//    Vector_type operator*(const Vector_type& v)
//    {
//        return Matrix(*this) * v;
//    }
protected:
    /**
     * @brief   This quickly computes row and column indices, assuming the
     *          Matrix has at least one column.
     * @pre     We assume row and col are non-nil and the number of columns
     *          is bigger than zero.
     *
     * This routine is used for the one-dimensional (row-major) addressing of
     * the matrix WITHOUT bounds checking -- for which speed is a priority.
     */
    void compute_row_col(
        int  index, ///< [in] Row-major index of some matrix element
        int* row,   ///< [out] Ptr to where to store its computed row index
        int* col    ///< [out] Ptr to where to store its computed column index
    ) const
    {
        // Test program was HERE.
        *row = index / get_num_cols();
        *col = index - *row * get_num_cols();
    }

    /**
     * generic version of operator=().
     * Made private to avoid unwanted template instantiation.
     */
    template <class Generic_matrix>
    bool equality(const Generic_matrix& mat)
    {
        const int NUM_ROWS = get_num_rows();
        const int NUM_COLS = get_num_cols();
        if(mat.get_num_rows() != NUM_ROWS ||
           mat.get_num_cols() != NUM_COLS )
        {
            KJB_THROW(Dimension_mismatch);
        }

        for(int row = 0; row < get_num_rows(); row++)
        {
            for(int col = 0; col < get_num_cols(); col++)
            {
                if(operator()(row, col) != mat(row, col)) 
                {
                    return false;
                }
            }
        }

        return true;
    }

    /**
     * generic version of operator=().
     * Made private to avoid unwanted template instantiation.
     */
    template <class Generic_matrix>
    Self& assign(const Generic_matrix& mat)
    {
        const int NUM_ROWS = get_num_rows();
        const int NUM_COLS = get_num_cols();
        if(mat.get_num_rows() != NUM_ROWS ||
           mat.get_num_cols() != NUM_COLS )
        {
            KJB_THROW(Dimension_mismatch);
        }

        for(int row = 0; row < get_num_rows(); row++)
        {
            for(int col = 0; col < get_num_cols(); col++)
            {
                operator()(row, col) = mat(row, col);
            }
        }

        return *this;
    }

    /**
     * generic version of operator=().
     * Made private to avoid unwanted template instantiation.
     */
    template <class T>
    Self& assign_from_vector(const T& v)
    {
        bool transpose = false;

        const int NUM_ROWS = get_num_rows();
        const int NUM_COLS = get_num_cols();

        if(NUM_COLS != 1)
        {
            transpose = true;

            if(NUM_ROWS != 1)
            {
                KJB_THROW(Dimension_mismatch);
            }
        }

        if(!transpose)
        {
            if(v.size() != NUM_ROWS)
            {
                KJB_THROW(Dimension_mismatch);
            }
        }
        else
        {
            if(v.size() != NUM_COLS)
            {
                KJB_THROW(Dimension_mismatch);
            }
        }

        for(int i = 0; i < v.size(); i++)
        {
            if(!transpose)
            {
                operator()(i, 0) = v[i];
            }
            else
            {
                operator()(0, i) = v[i];
            }
        }
        
        return *this;
    }


    /**
     * generic version of operator+=.
     * Made private to avoid unwanted template instantiation
     */
    template <class Generic_matrix>
    Self& plus_equals(const Generic_matrix& mat)
    {
        const int NUM_ROWS = get_num_rows();
        const int NUM_COLS = get_num_cols();
        if(NUM_ROWS != mat.get_num_rows() ||
           NUM_COLS != mat.get_num_cols())
        {
            KJB_THROW( Dimension_mismatch );
        }

        for(int row = 0; row < NUM_ROWS; row++)
        {
            for(int col = 0; col < NUM_COLS; col++)
            {
                operator()(row, col) += mat(row, col);

            }
        }

        return *this;

    }

    /**
     * generic version of operator-=().
     * Made private to avoid unwanted template instantiation
     */
    template <class Generic_matrix>
    Self& minus_equals(const Generic_matrix& mat)
    {
        const int NUM_ROWS = get_num_rows();
        const int NUM_COLS = get_num_cols();
        if(NUM_ROWS != mat.get_num_rows() ||
           NUM_COLS != mat.get_num_cols())
        {
            KJB_THROW( Dimension_mismatch );
        }

        for(int row = 0; row < NUM_ROWS; row++)
        {
            for(int col = 0; col < NUM_COLS; col++)
            {
                operator()(row, col) -= mat(row, col);

            }
        }

        return *this;

    }
protected:
    Index_range row_indices_;
    Index_range col_indices_;
    Matrix_type& mat_;

};

// "template typedef"
// see: http://www.gotw.ca/gotw/079.htm
template <class Matrix_type>
struct Generic_const_matrix_view
{
    typedef const Generic_matrix_view<const Matrix_type> Type;
};


/** @} */

} // namespace kjb

#endif
