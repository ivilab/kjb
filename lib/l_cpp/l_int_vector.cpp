/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Luca Del Pero, Kyle Simek, Andrew Predoehl.         |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: l_int_vector.cpp 19804 2015-09-20 04:11:36Z predoehl $ */

/** @file
 *
 * @author Kobus Barnard
 * @author Kyle Simek
 * @author Andrew Predoehl
 *
 * @brief Definition for the Int_vector class methods.
 *
 * The Int_vector class is a thin wrapper on the KJB Int_vector struct
 * and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * m_vector.cpp,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 */

#include <l_cpp/l_int_vector.h>
#include <l_cpp/l_int_matrix.h>
#include <l_cpp/l_serialization.h>
#include <sstream>
#include <ostream>
#include <iomanip>

#ifndef KJB_HAVE_BST_SERIAL
// forward declarations
namespace boost {
namespace archive {
    class text_iarchive {};
    class text_oarchive {};
} // namespace archive
} // namespace boost
#endif


namespace kjb {

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

void Int_vector::throw_bad_bounds( int index ) const
{
    // Test program was HERE.
    std::ostringstream msg;
    msg << "Invalid Int_vector access at (" << index << ").  "
           "Int_vector size is " << get_length() << ".\n";
    KJB_THROW_2( Index_out_of_bounds, msg.str() );
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

void Int_vector::m_ensure_capacity(size_type c)
{
    if(m_vector->max_length >= c) return;

    size_type cur_capacity = m_vector->max_length;

    while(cur_capacity < c)
    {
        if(cur_capacity == 0)
        {
            cur_capacity = 1;
        }
        else
        {
            cur_capacity *= 2;
        }
    }


    c = cur_capacity;

    size_type tmp_length = m_vector->length;
    resize(c);
    m_vector->length = tmp_length;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector::Int_vector(const kjb::Int_matrix& src)
    : m_vector(0)
{
    if( 1 == src.get_num_cols() )
    {
        // Test program was HERE.
        kjb_c::get_int_matrix_col(&m_vector, src.get_c_matrix(), 0);
    }
    else if ( 1 == src.get_num_rows() )
    {
        // Test program was HERE.
        kjb_c::get_int_matrix_row(&m_vector, src.get_c_matrix(), 0);
    }
    else
    {
        // Test program was HERE.
        KJB_THROW_2(Illegal_argument, "Cannot convert matrix to vector:  "
                                "matrix is not a column or row vector." );
    }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector::Int_vector(const std::string& file_name)
    : m_vector(0)
{
    if ( file_name.length() == 0 )
    {
        // Test program was HERE.
        KJB_THROW_2(Illegal_argument, "Cannot read vector from file:  "
                                            "file name is empty");
    }
    else
    {
        // Test program was HERE.
        int err = kjb_c::read_int_vector(&m_vector, file_name.c_str());
        if(err)
        {
            KJB_THROW_3(kjb::IO_error, "File not found %s", (file_name.c_str()));
        }
    }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector& Int_vector::resize(int new_length, Value_type pad_value )
{
    // Test program was HERE.
    Int_vector temp( new_length, pad_value );
    Value_type *src = m_vector -> elements;
    int copy_length = std::min( get_length(), new_length );
    std::copy( src, src + copy_length, temp.m_vector -> elements );
    swap( temp );
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector& Int_vector::randomize( int length )
{
    // Test program was HERE.
    Int_vector temp( length );
    Value_type *dst = temp.m_vector -> elements;
    for( int iii = 0; iii < length; ++iii )
    {
        *dst++ = rand();
    }
    swap( temp );
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

void Int_vector::serialize(boost::archive::text_iarchive &ar, const unsigned int version)
{
#ifndef KJB_HAVE_BST_SERIAL
    return kjb_serialize(ar, *this, version);
#else
    KJB_THROW(kjb::Runtime_error); // can't happen; "ar" isn't defined, so this can't can't be called
#endif
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

void Int_vector::serialize(
    boost::archive::text_oarchive& ar,
    const unsigned int version
)
{
#ifndef KJB_HAVE_BST_SERIAL
    return kjb_serialize(ar, *this, version);
#else
    KJB_THROW_2(kjb::Runtime_error, "Cannot happen, 'ar' is not defined");
#endif
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

/*
 * Let A' denote the transpose of A.  We rely on this identity:
 *
 * |    v * M   =   (v * M)''
 * |            =   (M' * v')'.
 *
 * We don't have a native routine to multiply a vector with a matrix on the
 * right, but we do know how to transpose, and we do have a routine to
 * multiply a matrix on the left with a vector.  So we use the above identity
 * to do the job.
 * We don't have to do any work to transpose a vector, since its orientation
 * (row-vec or column-vec) is inferred from context.
 *
 * This cannot easily be inlined, however, since it depends on Int_matrix.
 */
Int_vector operator*(const Int_vector& op1, const Int_matrix& op2)
{
    // TODO Make this function exception-safe
    if( op1.get_length() != op2.get_num_rows() )
    {
        // Test program was HERE.
        KJB_THROW(Dimension_mismatch);
    }
    //else
    //{
    //    // Test program was HERE.
    //}

    kjb_c::Int_vector* result = 0;
    ETX(kjb_c::multiply_int_vector_and_int_matrix( &result, op1.get_c_vector(),
                                                            op2.get_c_matrix() ));
    return Int_vector(result);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


Int_vector operator*(const Int_matrix& op1, const Int_vector& op2)
{
    // TODO Make this function exception-safe
    if ( op1.get_num_cols() != op2.get_length() )
    {
        // Test program was HERE.
        KJB_THROW(Dimension_mismatch);
    }
    //else
    //{
    //    // Test program was HERE.
    //}
    Int_vector::Impl_type *result = 0;
    ETX(kjb_c::multiply_int_matrix_and_int_vector( &result, op1.get_c_matrix(),
                                                            op2.get_c_vector() ));
    return Int_vector( result );
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

bool operator==(const Int_vector& op1, const Int_vector::Impl_type& op2)
{
    // Test program was HERE.
    if ( op1.get_c_vector() == &op2 )
    {
        // Test program was HERE.
        // If the vectors are the SAME vector then they are trivially equal.
        return true;
    }
    else if ( 0 == op1.get_c_vector() || 0 == &op2 )
    {
        /* If you reach here, they are not both nil, yet one of them is.
         * Actually you should never reach here since we try to maintain the
         * invariant that a Vector's m_vector pointer never equals NULL.
         * But if that invariant were to be relaxed, then the answer should be
         * false.
         */
        KJB(UNTESTED_CODE()); // Unreachable line (as of 11 Feb. 2010)
        return false;
    }
    else if ( op1.get_length() != op2.length )
    {
        // Test program was HERE.
        return false;
    }
    else
    {
        // Test program was HERE.
        return 0 == kjb_c::max_abs_int_vector_difference( op1.get_c_vector(), &op2 );
    }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

bool operator<(const Int_vector& op1, const Int_vector &op2)
{
    for(int i = 0; i < op1.get_length() && i < op2.get_length(); ++i)
    {
        if ( op1[i] != op2[i] )
        {
            return op1[i] < op2[i];
        }
    }

    return op1.get_length() < op2.get_length();
}


/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector& Int_vector::operator/=( int op2 )
{
    if ( 0 == op2 )
    {
        // Test program was HERE.
        KJB_THROW( Divide_by_zero );
    }

    // Test program was HERE.
    const int SZ = get_length();
    for( int iii = 0; iii < SZ; ++iii )
    {
        operator()( iii ) /= op2;
    }
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector& Int_vector::mapcar( Mapper fun )
{
    // Test program was HERE.
    const int SIZE = get_length();
    for( int iii = 0; iii < SIZE; ++iii )
    {
        Value_type newval = (*fun)( operator[]( iii ) );
        operator[]( iii ) = newval;
    }
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_matrix Int_vector::hat() const
{
    if( get_length() != 3)
    {
        // Test program was HERE.
        KJB_THROW_2(Not_implemented, "Hat operator is only implemented "
                                            "for vectors of dimension 3.");
    }
    //else
    //{
    //    // Test program was HERE.
    //}

    Int_matrix result( m_vector->length, m_vector->length, Value_type(0) );

    result(0,1) = -(*this)(2);
    result(0,2) =  (*this)(1);
    result(1,0) =  (*this)(2);
    result(1,2) = -(*this)(0);
    result(2,0) = -(*this)(1);
    result(2,1) =  (*this)(0);

    return result;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector& Int_vector::cross_with( const Int_vector& op2 )
{
    if ( get_length() != 3 || op2.get_length() != 3 )
    {
        // Test program was HERE.
        KJB_THROW_2( Illegal_argument,
                "Cross product is undefined for vectors not of size 3." );
    }
    //else
    //{
    //    // Test program was HERE.
    //}
    return operator=( this -> hat() * op2 );
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector cross( const Int_vector& op1, const Int_vector& op2 )
{
    if ( op1.get_length() != 3 || op2.get_length() != 3 )
    {
        // Test program was HERE.
        KJB_THROW_2( Illegal_argument,
                "Cross product is undefined for vectors not of size 3." );
    }
    //else
    //{
    //    // Test program was HERE.
    //}
    return op1.hat() * op2;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

std::ostream& operator<<(std::ostream& out, const Int_vector& v)
{
    // Test program was HERE.
    std::streamsize w = out.width();
    std::streamsize p = out.precision();
    std::ios::fmtflags f = out.flags();

    out << std::scientific;
    for(int i = 0; i < v.get_length(); i++)
    {
        out << std::setw(16) << std::setprecision(8) << v(i);
    }

    out.width( w );
    out.precision( p );
    out.flags( f );
    return out;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

/** @} */

}
