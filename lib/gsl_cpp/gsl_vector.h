/**
 * @file
 * @brief C++ wrapper on GSL vector class to prevent resource leaks
 * @author Andrew Predoehl
 */

/*
 * $Id: gsl_vector.h 20022 2015-11-03 07:39:05Z predoehl $
 */

#ifndef GSL_VECTOR_WRAP_H_KJBLIB_UARIZONAVISION
#define GSL_VECTOR_WRAP_H_KJBLIB_UARIZONAVISION

#include <l/l_sys_sys.h>
#include <m_cpp/m_cpp_incl.h>
#include "gsl_cpp/gsl_util.h"

//include <iterator>

#ifdef KJB_HAVE_GSL
#include "gsl/gsl_vector.h" /* no need for extern "C" */
#else
#warning "GNU GSL is absent, so GNU GSL wrapper will not work properly."
#include "gsl_cpp/stub_vector.h" /* stand-in types (not executed) */
#endif

#if ! defined( KJB_HAVE_ISNAN ) || ! defined( KJB_HAVE_FINITE )
#warning "This code tests for NAN and finite values but your system lacks"
#warning "this feature.  Method Gsl_Vector::is_normal will always return true."
#endif

#include <ostream>

#include <cmath>                /* Required for isnormal(3) macro,
                                 * so let's use it to avoid isnan() woes too.
                                 */

namespace kjb {

/**
 * @brief RAII wrapper for GSL vector objects.
 *
 * Note the "operator gsl_vector*()" method, which makes it transparent.
 * GSL is the Gnu Scientific Library.  Ordinary GSL vectors are a resource with
 * an API in C, so they must be allocated and freed.  This C++ wrapper will
 * automatically free the object when control flow leaves the object's scope.
 * (That's just the basic RAII paradigm.)
 *
 * Implementation invariant:  pointer m_vec is never to be left equal to NULL.
 */
class Gsl_Vector {

    gsl_vector* m_vec;

    /*
     * this private method should be called shortly after each invocation of
     * gsl_vector_alloc() because we would like to maintain the invariant that
     * m_vec is never allowed to remain equal to NULL.
     */
    void throw_if_null()
    {
        ETX_2( 00 == m_vec, "Memory allocation failure for Gsl_Vector" );
    }

    // called by the at() methods
    void check_bounds( size_t index ) const
    {
        /*
         * GCC complains about this test:  "that could never happen!!"
        if ( index < 0 )
        {
            KJB_THROW( Index_out_of_bounds );
        }
        */
        if ( size() <= index )
        {
            KJB_THROW( Index_out_of_bounds );
        }
    }

public:

    /// @brief ctor to build an uninitialized vector of a given size
    Gsl_Vector( size_t dims )
    :   m_vec( gsl_vector_alloc( dims ) )
    {
#ifdef KJB_HAVE_GSL
        throw_if_null();
#else
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }


    /// @brief conversion ctor makes a copy of a KJB vector
    explicit Gsl_Vector( const kjb::Vector& kv )
    :   m_vec( gsl_vector_alloc( kv.size() ) )
    {
#ifdef KJB_HAVE_GSL
        throw_if_null();
        size_t KVSZ = std::max( kv.size(), 0 );
        for( size_t iii = 0; iii < KVSZ; ++iii )
        {
            at( iii ) = kv.at( iii );
        }
#else
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }


    /// @brief copy ctor
    Gsl_Vector( const Gsl_Vector& src )
    :   m_vec( gsl_vector_alloc( src.size() ) )
    {
#ifdef KJB_HAVE_GSL
        throw_if_null();
        GSL_ETX( gsl_vector_memcpy( m_vec, src.m_vec ) );
#else
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }


    /// @brief ctor copies an unwrapped GSL vector, does NOT take ownership.
    Gsl_Vector( const gsl_vector& vec_to_be_copied )
    :   m_vec( gsl_vector_alloc( vec_to_be_copied.size ) )
    {
#ifdef KJB_HAVE_GSL
        throw_if_null();
        GSL_ETX( gsl_vector_memcpy( m_vec, &vec_to_be_copied ) );
#else
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }


    /// @brief assignment operator
    Gsl_Vector& operator=( const Gsl_Vector& src )
    {
#ifdef KJB_HAVE_GSL
        if ( m_vec != src.m_vec )
        {
            if ( size() == src.size() )
            {
                GSL_ETX( gsl_vector_memcpy( m_vec, src.m_vec ) );
            }
            else
            {
                Gsl_Vector clone( src );    // copy ctor makes a duplicate
                swap( clone );              // then, become that duplicate
            }
        }
#endif
        return *this;
    }


/**
 * Sadly we cannot have a proper begin(), end() interface.
 * Consider using front() and back() instead.
 *
 * A proper end() method would point to one-past-the-last member.
 * Unfortunately to point into the vector via the GSL API, we must use
 * gsl_vector_ptr() or gsl_vector_const_ptr(),
 * but those methods are normally range-checked and call the error handler if
 * you go past the last member.  Although we could hack around this difficulty,
 * I think that would be inappropriate for library code.  (This header should
 * wrap the available GSL vector API, not extend it.)
 *
 * An alternative would be to define end() contrary to the usual STL semantics,
 * but that would cause enormous confusion to any new programmer.
 *
 * In general, stride might not equal 1, but if it does, &v.back()+1 should
 * work as semantically equivalent to end().  If stride is bigger than 1, STL
 * code generally won't work anyway, which is the primary use case of end().
 */
#if 0
    double* begin()
    {
#ifdef KJB_HAVE_GSL
        return gsl_vector_ptr( m_vec, 0 );
#else
        return 00;
#endif
    }

    const double* begin() const
    {
#ifdef KJB_HAVE_GSL
        return gsl_vector_const_ptr( m_vec, 0 );
#else
        return 00;
#endif
    }

    double* end()
    {
#ifdef KJB_HAVE_GSL
        int length = std::max(0, (int)size() - 1);
        return gsl_vector_ptr( m_vec, length); // weird semantics!
#else
        return 00;
#endif
    }

    const double* end() const
    {
#ifdef KJB_HAVE_GSL
        int length = std::max(0, (int)size() - 1);
        return gsl_vector_const_ptr( m_vec, length); // weird semantics!
#else
        return 00;
#endif
    }

#endif /* No begin(), end() interface. */



    /// @brief swap the contents of two vectors
    void swap( Gsl_Vector& v )
    {
        using std::swap;
        KJB(ASSERT( m_vec ));
        KJB(ASSERT( v.m_vec ));
        swap( m_vec, v.m_vec );
    }


    /// @brief dtor releases the resources in a vector
    ~Gsl_Vector()
    {
        KJB(ASSERT( m_vec ));
#ifdef KJB_HAVE_GSL
        gsl_vector_free( m_vec );
#else
        delete m_vec;
#endif
    }


    /// @brief ctor constructs from a pair of input iterators
    template < class Iterator >
    Gsl_Vector( Iterator begin, Iterator end )
    :   m_vec( gsl_vector_alloc( std::distance( begin, end ) ) )
    {
#ifdef KJB_HAVE_GSL
        throw_if_null();
        size_t iii = 0;
        for( Iterator cur = begin; cur != end; ++cur )
        {
            at( iii++ ) = *cur;
        }
#else
        KJB_THROW_2( Missing_dependency, "GNU GSL" );
#endif
    }


    /// @brief returns the size of the vector
    size_t size() const
    {
        KJB(ASSERT( m_vec ));
        return m_vec -> size;
    }


    /*
     * Because of the transparent convert-to-pointer operator, we lose the
     * ability to set the vector using square brackets, because square brackets
     * are ambiguous -- are they an implicit pointer dereference, or do they
     * refer to operator[] applied to the object?
     * But you can still use at().
     *
    double operator[]( size_t index ) const
    {
        KJB(ASSERT( m_vec ));
        return gsl_vector_get( m_vec, index );
    }

    double& operator[]( size_t index )
    {
        KJB(ASSERT( m_vec );
        return *gsl_vector_ptr( m_vec, index );
    }
    */


    /// @brief element access for the vector, returning an rvalue
    double at( size_t index ) const
    {
        KJB(ASSERT( m_vec ));
#ifdef KJB_HAVE_GSL
        check_bounds( index );
        return gsl_vector_get( m_vec, index );
#else
        return m_vec -> at( index );
#endif
    }


    /// @brief element access for the vector, returning an lvalue
    double& at( size_t index )
    {
        KJB(ASSERT( m_vec ));
#ifdef KJB_HAVE_GSL
        check_bounds( index );
        return *gsl_vector_ptr( m_vec, index );
#else
        return m_vec -> at( index );
#endif
    }


    /// @brief access the first element of the vector, returning an lvalue
    double& front()
    {
        KJB(ASSERT( m_vec ));
        return at( 0 );
    }


    /// @brief access the first element of the vector, returning an rvalue
    double front() const
    {
        KJB(ASSERT( m_vec ));
        return at( 0 );
    }


    /// @brief access the last element of the vector, returning an lvalue
    double& back()
    {
        KJB(ASSERT( m_vec ));
        return at( size() - 1 );
    }


    /// @brief access the last element of the vector, returning an rvalue
    double back() const
    {
        KJB(ASSERT( m_vec ));
        return at( size() - 1 );
    }


    /// @brief set all vector members to a given value
    Gsl_Vector& set_all( double val )
    {
        KJB(ASSERT( m_vec ));
#ifdef KJB_HAVE_GSL
        gsl_vector_set_all( m_vec, val );
#endif
        return *this;
    }


    /// @brief set all vector members to zero
    Gsl_Vector& set_zero()
    {
        KJB(ASSERT( m_vec ));
#ifdef KJB_HAVE_GSL
        gsl_vector_set_zero( m_vec );
#endif
        return *this;
    }


    /** @brief Init to the (j+1)th column of identity matrix, given argument j.
     *
     * For example, a call to v.set_basis(0) will set vector v to equal
     * column 1 of the n x n identity matrix, assuming n is the number of elts
     * in the vector v.
     */
    Gsl_Vector& set_basis( size_t index )
    {
        KJB(ASSERT( m_vec ));
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_vector_set_basis( m_vec, index ) );
#endif
        return *this;
    }


    /**
     * @brief Automatically (danger!) convert a wrapped object into a pointer.
     *
     * @warning Do not free the object explicitly using this method.
     *
     * With great power comes great responsibility.  This method gives you
     * great power, and you the client are responsible for avoiding the
     * inherent pitfalls.  For example, don't gsl_vector_free().
     */
    operator gsl_vector*()
    {
        return m_vec;
    }


    /// @brief Export contents in a KJB Vector (the C++ kind).
    kjb::Vector vec() const
    {
        kjb::Vector kv( size() );
        for( size_t iii = 0; iii < size(); ++iii )
        {
            kv.at( iii ) = at( iii );
        }
        return kv;
    }


    /**
     * @brief Perform element-wise multiplcation of two vectors, same size.
     *
     * This method is similar to kjb_c::multiply_vectors(), and basically a
     * wrapped up version of gsl_vector_mul().
     *
     * This is not overloaded star because multiplication between two vectors
     * is not self explanatory (inner, outer, scalar, elementwise)?
     * Thus the name needs to be descriptive.
     * The name "ew_multiply" is not my favorite, but lib/m_cpp uses something
     * similar.  There are two differences between this and that function:
     *  -   That ew_multiply permits two vectors of different sizes to be
     *      arguments, but GSL does not; this follows GSL.
     *  -   That ew_multiply overwrites one of the original KJB vectors; this
     *      is const.
     */
    Gsl_Vector ew_multiply( const Gsl_Vector& that ) const
    {
        if ( size() != that.size() )
        {
            KJB_THROW( Dimension_mismatch );
        }
        Gsl_Vector product( *this );    // copy this vector
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_vector_mul( product.m_vec, that.m_vec ) );
#endif
        return product;
    }


    /**
     * @brief Return the sum of the elements of the vector.
     */
    double sum() const
    {
        double sum_vec = 0;
        for( size_t iii = 0; iii < size(); ++iii )
        {
            sum_vec += at( iii );
        }
        return sum_vec;
    }


    /**
     * @brief Return the dot product of two vectors.
     */
    double dot( const Gsl_Vector& that ) const
    {
        Gsl_Vector this_x_that( ew_multiply( that ) );
        return this_x_that.sum();
    }


    /**
     * @brief Return the L2 norm (the Euclidean norm) of the vector.
     */
    double l2_norm() const
    {
        using std::sqrt;
        return sqrt( dot( *this ) );
    }


    /**
     * @brief Test whether all vector entries are "normal" values.
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
        if ( 0 == m_vec )
        {
            return true;
        }
        for( size_t iii = 0; iii < size(); ++iii )
        {
            double x = at( iii );
            // The following uses KJB-lib wraps on the lower level facilities.
            // On systems lacking these capabilities, all values seem normal.

            // 3 Nov. 2015 -- predoehl -- this macro is causing trouble again. :-(
            // Hopefully the std namespace will get the job done.
            if ( /*isnand( x )*/ std::isnan(x) || ! finite( x ) )
            {
                return false;
            }
        }
        return true;
    }


    /// @brief Add a vector of the same size to this vector, elementwise
    Gsl_Vector& operator+=( const Gsl_Vector& that )
    {
#ifdef KJB_HAVE_GSL
        if ( that.size() != size() )
        {
            KJB_THROW( Dimension_mismatch );
        }
        GSL_ETX( gsl_vector_add( m_vec, that.m_vec ) );
#endif
        return *this;
    }


    /// @brief Return the sum of this vector and another vector of same size
    Gsl_Vector operator+( const Gsl_Vector& that ) const
    {
        Gsl_Vector vsum( *this );
        return vsum += that;
    }


    /// @brief Subtract a vector of the same size from this vector, elementwise
    Gsl_Vector& operator-=( const Gsl_Vector& that )
    {
#ifdef KJB_HAVE_GSL
        if ( that.size() != size() )
        {
            KJB_THROW( Dimension_mismatch );
        }
        GSL_ETX( gsl_vector_sub( m_vec, that.m_vec ) );
#endif
        return *this;
    }


    /// @brief Return the difference of two vectors of the same size
    Gsl_Vector operator-( const Gsl_Vector& that ) const
    {
        Gsl_Vector vdif( *this );
        return vdif -= that;
    }


    /**
     * @brief Scale by a real value, like kjb_c::ow_multiply_vector_by_scalar.
     *
     * For brevity, you might prefer to use operator*= instead of this method.
     */
    Gsl_Vector& ow_multiply_vector_by_scalar( double scalar )
    {
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_vector_scale( m_vec, scalar ) );
#endif
        return *this;
    }


    /// @brief Scale by a real value, like ow_multiply_vector_by_scalar.
    Gsl_Vector& operator*=( double scalar )
    {
        return ow_multiply_vector_by_scalar( scalar );
    }


    /// @brief Reverse the order of the elements of the vector, in place.
    Gsl_Vector& ow_reverse()
    {
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_vector_reverse( m_vec ) );
#endif
        return *this;
    }


    /// @brief Scale by a real value and return the result
    Gsl_Vector operator*( double scalar ) const
    {
        Gsl_Vector product( *this );
        return product *= scalar;
    }


    /// @brief add scalar to each elt. of this vector, in-place
    Gsl_Vector& operator+=( double scalar )
    {
#ifdef KJB_HAVE_GSL
        GSL_ETX( gsl_vector_add_constant( m_vec, scalar ) );
#endif
        return *this;
    }


    /// @brief Return copy of this vector with a scalar added to each elt.
    Gsl_Vector operator+( double scalar ) const
    {
        Gsl_Vector vsum( *this );
        return vsum += scalar;
    }

    /// @brief subtract scalar from each elt. of this vector, in-place
    Gsl_Vector& operator-=( double scalar )
    {
        return operator+=( -scalar );
    }


    /// @brief Return copy of this vector w/ a scalar subtracted from each elt.
    Gsl_Vector operator-( double scalar ) const
    {
        Gsl_Vector vsum( *this );
        return vsum -= scalar;
    }


    /// @brief Return copy of this vector with every element negated
    Gsl_Vector operator-() const
    {
        return *this*-1.0;
    }


    /// @brief return the maximum value in the vector
    double max() const
    {
#ifdef KJB_HAVE_GSL
        KJB(ASSERT( m_vec ));
        return gsl_vector_max( m_vec );
#endif
    }


    /// @brief return the minimum value in the vector
    double min() const
    {
#ifdef KJB_HAVE_GSL
        KJB(ASSERT( m_vec ));
        return gsl_vector_min( m_vec );
#endif
    }


    /*
     * There are more GSL vector operations that are not wrapped:
     * fprintf, fscanf, fread, fwrite, additional math operations.
     */
};


/// @brief multiply scalar and vector, scalar written on the left side
inline
Gsl_Vector operator*( double scalar, const Gsl_Vector& vector )
{
    return vector * scalar;
}


} // namespace kjb


/// @brief Print out a vector as a column of ASCII-rendered values + newlines.
inline
std::ostream& operator<<( std::ostream& os, const kjb::Gsl_Vector& v )
{
    for( size_t iii = 0; iii < v.size(); ++iii )
    {
        os << v.at( iii ) << '\n';
    }
    return os;
}

/**
 * @brief Read in ASCII-rendered, WS-delimited contents to a presized vector.
 *
 * This is not super-robust:  prior to using this routine, vector v must
 * already be set to the size of the number of values that you want to read in.
 *
 * Input values are ASCII-rendered decimal real values, delimited by white
 * space.
 */
inline
std::istream& operator>>( std::istream& is, kjb::Gsl_Vector& v )
{
    // To repeat:  v must already know what size it is supposed to be!
    for( size_t iii = 0; iii < v.size(); ++iii )
    {
        is >> v.at( iii );
    }
    return is;
}


namespace std {

    /**
     * @brief Swap two vectors
     *
     * The vectors referred to here are data structures of the Gnu Scientific
     * Library (GSL) that have C++ wrappers of my own invention.
     */
    template<>
    inline void swap( kjb::Gsl_Vector& v1, kjb::Gsl_Vector& v2 )
    {
        v1.swap( v2 );
    }
}


#endif /* GSL_VECTOR_WRAP_H_KJBLIB_UARIZONAVISION */
