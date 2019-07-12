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
   |  Author:  Jinyan Guan
 * ===========================================================================
 */

/* $Id: gsl_matrix.h 20313 2016-02-02 06:14:36Z predoehl $ */

#ifndef KJB_CPP_GSL_MATRIX_H
#define KJB_CPP_GSL_MATRIX_H

#include <l/l_sys_sys.h>
#include <l_cpp/l_util.h>
#include <m_cpp/m_matrix.h>
#include "gsl_cpp/gsl_util.h"

#ifdef KJB_HAVE_GSL
#include "gsl/gsl_matrix.h" /* no need for extern "C" */
#else
#warning "GNU GSL is absent, so GNU GSL wrapper will not work properly."
#include "gsl_cpp/stub_vector.h"
#endif

#include <ostream>
#include <cmath>

namespace kjb
{

/**
 * @brief RAII wrapper for GSL matrix objects.
 *
 * Note the "operator gsl_matrix*()" method, which makes it transparent.
 * GSL is the Gnu Scientific Library.  Ordinary GSL matrixs are a resource with
 * an API in C, so they must be allocated and freed.  This C++ wrapper will
 * automatically free the object when control flow leaves the object's scope.
 * (That's just the basic RAII paradigm.)
 *
 * Implementation invariant:  pointer m_mat is never to be left equal to NULL.
 */

class Gsl_matrix
{
    gsl_matrix* m_mat;

    /*
     * this private method should be called shortly after each invocation of
     * gsl_matrix_alloc() because we would like to maintain the invariant that
     * m_mat is never allowed to remain equal to NULL.
     */
    void throw_if_null()
    {
        ETX_2( 00 == m_mat, "Memory allocation failure for Gsl_matrix" );
    }

    // called by the at() methods
    void check_bounds(size_t index1, size_t index2) const
    {
        /*
         * GCC complains about this test:  "that could never happen!!"
        if ( index < 0 )
        {
            KJB_THROW( Index_out_of_bounds );
        }
        */
        if (num_rows() <= index1 || num_cols() <= index2)
        {
            KJB_THROW( Index_out_of_bounds );
        }
    }

public:

    /** @brief ctor to build an uninitialized matrix of a given size */
    Gsl_matrix(size_t num_rows, size_t num_cols )
#ifdef KJB_HAVE_GSL
    :   m_mat( gsl_matrix_alloc(num_rows, num_cols) )
    {
        throw_if_null();
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }

    /** @brief conversion ctor makes a copy of a KJB matrix */
    explicit Gsl_matrix( const Matrix& kv )
#ifdef KJB_HAVE_GSL
    :   m_mat( gsl_matrix_alloc(kv.get_num_rows(), kv.get_num_cols() ) )
    {
        throw_if_null();
        size_t KV_rows = std::max(kv.get_num_rows(), 0);
        size_t KV_cols = std::max(kv.get_num_cols(), 0);
        for( size_t i = 0; i < KV_rows; ++i)
        {
            for( size_t ii = 0; ii < KV_cols; ++ii)
            {
                gsl_matrix_set(m_mat, i, ii, kv(i, ii));
            }
        }
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }

    /** @brief copy ctor */
    Gsl_matrix( const Gsl_matrix& src )
#ifdef KJB_HAVE_GSL
    :   m_mat( gsl_matrix_alloc(src.num_rows(), src.num_cols()))
    {
        throw_if_null();
        GSL_ETX( gsl_matrix_memcpy( m_mat, src.m_mat ) );
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }

    /** @brief ctor copies an unwrapped GSL matrix, does NOT take ownership.*/
    Gsl_matrix(const gsl_matrix& vec )
#ifdef KJB_HAVE_GSL
    :   m_mat(gsl_matrix_alloc(vec.size1, vec.size2) )
    {
        throw_if_null();
        GSL_ETX( gsl_matrix_memcpy( m_mat, &vec) );
#else
    {
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }


    /** @brief assignment operator */
    Gsl_matrix& operator=( const Gsl_matrix& src )
    {
#ifdef KJB_HAVE_GSL
        if ( m_mat != src.m_mat )
        {
            if ( num_rows() == src.num_rows() && num_cols() == src.num_cols() )
            {
                GSL_ETX( gsl_matrix_memcpy( m_mat, src.m_mat ) );
            }
            else
            {
                Gsl_matrix clone( src );    // copy ctor makes a duplicate
                swap( clone );              // then, become that duplicate
            }
        }
#endif
        return *this;
    }


    /* For reason similar to those in gsl_vector.h, the begin() and end()
       interface to gsl_matrix is very problematic -- either you have
       nonstandard semantics, or a hack that unreliably extends the API,
       or you invoke the error handler.  It is better just to use the API
       that GSL offers, rather than the API we wished that they offered.
     */
#if 0
    double* begin()
    {
#ifdef KJB_HAVE_GSL
        return gsl_matrix_ptr( m_mat, 0, 0 );
#else
        return 00;
#endif
    }

    const double* begin() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_matrix_const_ptr( m_mat, 0, 0);
#else
        return 00;
#endif
    }

    double* end()
    {
#ifdef KJB_HAVE_GSL
        size_t last_row = std::max(0, (int)num_rows() - 1);
        size_t last_col = std::max(0, (int)num_cols() - 1);
        return gsl_matrix_ptr( m_mat, last_row, last_col);
#else
        return 00;
#endif
    }

    const double* end() const
    {
#ifdef KJB_HAVE_GSL
        size_t last_row = std::max(0, (int)num_rows() - 1);
        size_t last_col = std::max(0, (int)num_cols() - 1);
        return gsl_matrix_const_ptr( m_mat, last_row, last_col);
#else
        return 00;
#endif
    }
#endif

    /** @brief swap the contents of two matrixs */
    void swap( Gsl_matrix& v )
    {
        using std::swap;
        KJB(ASSERT( m_mat ));
        KJB(ASSERT( v.m_mat ));
        swap( m_mat, v.m_mat );
    }


    /// @brief dtor releases the resources in a matrix
    ~Gsl_matrix()
    {
        KJB(ASSERT( m_mat ));
#ifdef KJB_HAVE_GSL
        gsl_matrix_free( m_mat );
#else
        delete m_mat;
#endif
    }

    /** @brief returns the size of the matrix */
    size_t num_rows() const
    {
        KJB(ASSERT( m_mat ));
        return m_mat -> size1;
    }

    /** @brief returns the size of the matrix */
    size_t num_cols() const
    {
        KJB(ASSERT( m_mat ));
        return m_mat -> size2;
    }

    /*
     * Because of the transparent convert-to-pointer operator, we lose the
     * ability to set the matrix using square brackets, because square brackets
     * are ambiguous -- are they an implicit pointer dereference, or do they
     * refer to operator[] applied to the object?
     * But you can still use at().
     *
    double operator[]( size_t index ) const
    {
        KJB(ASSERT( m_mat ));
        return gsl_matrix_get( m_mat, index );
    }

    double& operator[]( size_t index )
    {
        KJB(ASSERT( m_mat );
        return *gsl_matrix_ptr( m_mat, index );
    }
    */


    /** @brief element access for the matrix, returning an rvalue */
    double at( size_t row, size_t col ) const
    {
        KJB(ASSERT( m_mat ));
#ifdef KJB_HAVE_GSL
        check_bounds( row, col);
        return gsl_matrix_get( m_mat, row, col);
#endif
    }

    /** @brief element access for the matrix, returning an lvalue */
    double& at( size_t row, size_t col)
    {
        KJB(ASSERT( m_mat ));
#ifdef KJB_HAVE_GSL
        check_bounds( row, col );
        return *gsl_matrix_ptr( m_mat, row, col );
#endif
    }

    /** @brief access the first element of the matrix, returning an lvalue */
    double& front()
    {
        KJB(ASSERT( m_mat ));
        return at(0, 0);
    }


    /** @brief access the first element of the matrix, returning an rvalue */
    double front() const
    {
        KJB(ASSERT( m_mat ));
        return at(0, 0);
    }


    /** @brief access the last element of the matrix, returning an lvalue */
    double& back()
    {
        KJB(ASSERT( m_mat ));
        size_t last_row = std::max(0, (int)num_rows() - 1);
        size_t last_col = std::max(0, (int)num_cols() - 1);
        return at(last_row, last_col);
    }


    /** @brief access the last element of the matrix, returning an rvalue */
    double back() const
    {
        KJB(ASSERT( m_mat ));
        size_t last_row = std::max(0, (int)num_rows() - 1);
        size_t last_col = std::max(0, (int)num_cols() - 1);
        return at(last_row, last_col);
    }

    /** @brief set all matrix members to a given value */
    Gsl_matrix& set_all( double val )
    {
        KJB(ASSERT( m_mat ));
#ifdef KJB_HAVE_GSL
        gsl_matrix_set_all( m_mat, val );
#endif
        return *this;
    }


    /** @brief set all matrix members to zero */
    Gsl_matrix& set_zero()
    {
        KJB(ASSERT( m_mat ));
#ifdef KJB_HAVE_GSL
        gsl_matrix_set_zero( m_mat );
#endif
        return *this;
    }

    /** @brief returns the underline C poninter */
    const gsl_matrix* const_ptr() const
    {
        return m_mat;
    }

    gsl_matrix* ptr()
    {
        return m_mat;
    }

    /** @brief Export contents in a KJB matrix (the C++ kind). */
    Matrix mat() const
    {
        Matrix kv(num_rows(), num_cols());
        for(size_t i = 0; i < num_rows(); ++i)
        {
            for(size_t ii = 0; ii < num_cols(); ++ii)
            {
                kv.at( i, ii ) = at( i, ii );
            }
        }
        return kv;
    }

    /**
     * @brief Perform element-wise multiplcation of two matrixs, same size.
     *
     * This method is similar to kjb_c::multiply_matrixs(), and basically a
     * wrapped up version of gsl_matrix_mul().
     *
     * This is not overloaded star because multiplication between two matrixs
     * is not self explanatory (inner, outer, scalar, elementwise)?
     * Thus the name needs to be descriptive.
     * The name "ew_multiply" is not my favorite, but lib/m_cpp uses something
     * similar.  There are two differences between this and that function:
     *  -   That ew_multiply permits two matrixs of different sizes to be
     *      arguments, but GSL does not; this follows GSL.
     *  -   That ew_multiply overwrites one of the original KJB matrixs; this
     *      is const.
     */
    Gsl_matrix ew_multiply( const Gsl_matrix& that ) const
    {
        if ( num_rows() != that.num_rows() || num_cols() != that.num_cols() )
        {
            KJB_THROW( Dimension_mismatch );
        }
        Gsl_matrix product( *this );    // copy this matrix
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_matrix_mul_elements( product.m_mat, that.m_mat ) );
#endif
        return product;
    }

    /**
     * @brief Test whether all matrix entries are "normal" values.
     *
     * Infinity (plus or minus) is abnormal; Not-a-Number (NaN) is
     * abnormal; and rational numbers with exponents too extreme to normalize
     * are abnormal.  To me, all other values (including zero) that can be
     * represented by a double are normal.
     *
     * This method is similar to isnormal(3), but there is one significant
     * difference:  isnormal(3) does not define zero as a normal value.
     * This method does regard zero as a normal value.
     */
    bool is_normal() const
    {
        if ( 0 == m_mat )
        {
            return true;
        }
        for( size_t i = 0; i < num_rows(); ++i )
        {
            for( size_t ii = 0; ii < num_cols(); ++ii )
            {
                double x = at(i, ii);
                // The following uses KJB-lib wraps on the lower level
                // facilities. On systems lacking these capabilities,
                // all values seem normal.

                // 2016-02-01 Predoehl -- sorry, cannot use the kjb wrapper;
                // this macro is causing trouble again. :-(
                // Hopefully the std namespace will get the job done.
                if ( /*isnand( x )*/ std::isnan(x) || ! finite( x ) )
                {
                    return false;
                }
            }
        }
        return true;
    }

    /** @brief Add a matrix of the same size to this matrix, elementwise */
    Gsl_matrix& operator+=( const Gsl_matrix& that )
    {
#ifdef KJB_HAVE_GSL
        if ( that.num_rows() != num_rows() || that.num_cols() != num_cols())
        {
            KJB_THROW( Dimension_mismatch );
        }
        GSL_ETX( gsl_matrix_add( m_mat, that.m_mat ) );
#endif
        return *this;
    }


    /** @brief Return the sum of this matrix and another matrix of same size */
    Gsl_matrix operator+( const Gsl_matrix& that ) const
    {
        Gsl_matrix vsum( *this );
        return vsum += that;
    }


    /** @brief Subtract a matrix of the same size from this matrix
     *        elementwise
     */
    Gsl_matrix& operator-=( const Gsl_matrix& that )
    {
#ifdef KJB_HAVE_GSL
        if ( that.num_rows() != num_rows() || that.num_cols() != num_cols())
        {
            KJB_THROW( Dimension_mismatch );
        }
        GSL_ETX( gsl_matrix_sub( m_mat, that.m_mat ) );
#endif
        return *this;
    }


    /** @brief Return the difference of two matrixs of the same size */
    Gsl_matrix operator-( const Gsl_matrix& that ) const
    {
        Gsl_matrix vdif( *this );
        return vdif -= that;
    }


    /**
     * @brief Scale by a real value, like kjb_c::ow_multiply_matrix_by_scalar.
     *
     * For brevity, you might prefer to use operator*= instead of this method.
     */
    Gsl_matrix& ow_multiply_matrix_by_scalar( double scalar )
    {
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_matrix_scale( m_mat, scalar ) );
#endif
        return *this;
    }


    /** @brief Scale by a real value, like ow_multiply_matrix_by_scalar. */
    Gsl_matrix& operator*=( double scalar )
    {
        return ow_multiply_matrix_by_scalar( scalar );
    }


    /** @brief Scale by a real value and return the result */
    Gsl_matrix operator*( double scalar ) const
    {
        Gsl_matrix product( *this );
        return product *= scalar;
    }


    /** @brief add scalar to each elt. of this matrix, in-place */
    Gsl_matrix& operator+=( double scalar )
    {
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_matrix_add_constant( m_mat, scalar ) );
#endif
        return *this;
    }

    /** @brief Return copy of this matrix with a scalar added to each elt. */
    Gsl_matrix operator+( double scalar ) const
    {
        Gsl_matrix vsum( *this );
        return vsum += scalar;
    }

    /** @brief subtract scalar from each elt. of this matrix, in-place */
    Gsl_matrix& operator-=( double scalar )
    {
        return operator+=( -scalar );
    }

    /**
     * @brief Return copy of this matrix w/ a scalar subtracted from each elt.
     */
    Gsl_matrix operator-( double scalar ) const
    {
        Gsl_matrix vsum( *this );
        return vsum -= scalar;
    }

    /** @brief Return copy of this matrix with every element negated */
    Gsl_matrix operator-() const
    {
        return *this*-1.0;
    }

    /** @brief return the maximum value in the matrix */
    double max() const
    {
#ifdef KJB_HAVE_GSL
        KJB(ASSERT( m_mat ));
        return gsl_matrix_max( m_mat );
#endif
    }

    /**  @brief return the minimum value in the matrix */
    double min() const
    {
#ifdef KJB_HAVE_GSL
        KJB(ASSERT( m_mat ));
        return gsl_matrix_min( m_mat );
#endif
    }

    /*
     * There are more GSL matrix operations that are not wrapped:
     * fprintf, fscanf, fread, fwrite, additional math operations.
     */
};

} //namespace kjb

/// @brief Print out a vector as a column of ASCII-rendered values + newlines.
inline
std::ostream& operator<<( std::ostream& os, const kjb::Gsl_matrix& v )
{
    for(size_t i = 0; i < v.num_rows(); i++)
    {
        for(size_t j = 0; j < v.num_cols(); j++)
        {
            os << v.at(i, j) << ' ';
        }
        os << '\n';
    }
    return os;
}

#endif /* KJB_CPP_GSL_MATRIX_H */
