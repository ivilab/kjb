/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kobus Barnard, Kyle Simek, Andrew Predoehl.                        |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: m_vector.cpp 21596 2017-07-30 23:33:36Z kobus $ */

/** @file
 *
 * @author Kobus Barnard
 * @author Kyle Simek
 * @author Andrew Predoehl
 *
 * @brief Definition for the Vector class methods.
 *
 * The Vector class is a thin wrapper on the KJB Vector struct
 * and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * l_int_vector.cpp,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_mat_view.h"
#include "m_cpp/m_vec_view.h"
#include "l_cpp/l_int_vector.h"
#include "sample/sample_gauss.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <ostream>
#include <iomanip>


namespace {

// functor used for the floor transformation, like a mythical int floor(double)
struct iFloor {
    typedef kjb::Int_vector::value_type my_int;
    typedef kjb::Vector::value_type my_real;
    my_int operator()( my_real xxx ) const
    {
        return static_cast< my_int >( floor( xxx ) );
    }
};

} // end anonymous namespace


namespace kjb {

//const char* const Vector::BAD_SEGMENT =
    //"Failure in create_from_vector_section:  allocation error or bad indices";

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

void Vector::throw_bad_bounds( int index ) const
{
    // Test program was HERE.
    std::ostringstream msg;
    msg << "Invalid Vector access at (" << index << ").  "
           "Vector size is " << get_length() << ".\n";
    KJB_THROW_2( Index_out_of_bounds, msg.str() );
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

void Vector::m_ensure_capacity(size_type c)
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

Vector::Vector(const Matrix& src)
    : m_vector(0)
{
    if( 1 == src.get_num_cols() )
    {
        // Test program was HERE.
        kjb_c::get_matrix_col(&m_vector, src.get_c_matrix(), 0);
    }
    else if ( 1 == src.get_num_rows() )
    {
        // Test program was HERE.
        kjb_c::get_matrix_row(&m_vector, src.get_c_matrix(), 0);
    }
    else
    {
        // Test program was HERE.
        KJB_THROW_2(Illegal_argument, "Cannot convert matrix to vector:  "
                                "matrix is not a column or row vector." );
    }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector::Vector(const std::vector<double>& src)
    : m_vector( NULL )
{
    Vector result(src.begin(), src.end());
    swap(result);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector::Vector( const Vector_view& vec_view) :
    m_vector(0)
{
    init_from_view_(vec_view);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Vector::Vector( const Const_vector_view& vec_view) :
    m_vector(0)
{
    init_from_view_(vec_view);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Vector::Vector(const Matrix_view& src)
    : m_vector(0)
{
    if( 1 == src.get_num_cols() )
    {
        kjb_c::get_target_vector(&m_vector, src.get_num_rows());
        for(int row = 0; row < src.get_num_rows(); row++)
            operator[](row) = src(row, 0);
    }
    else if ( 1 == src.get_num_rows() )
    {
        kjb_c::get_target_vector(&m_vector, src.get_num_cols());
        for(int col = 0; col < src.get_num_cols(); col++)
            operator[](col) = src(0, col);
    }
    else
    {
        // Test program was HERE.
        KJB_THROW_2(Illegal_argument, "Cannot convert matrix view to vector:  "
                                "matrix view is not a column or row vector." );
    }
}


Vector::Vector( const Int_vector& ivec )
    : m_vector(0)
{
    // Test program was HERE.
    Vector result( ivec.get_length() );
    for( int iii = 0; iii < ivec.get_length(); ++iii )
    {
        result( iii ) = ivec( iii );
    }
    swap( result );
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector::Vector(const std::string& file_name)
    : m_vector(0)
{
    if ( file_name.length() == 0)
    {
        // Test program was HERE.
        KJB_THROW_2(Illegal_argument, "Cannot read vector from file:  "
                                            "file name is empty");
    }
    else
    {
        // Test program was HERE.
        int err = kjb_c::read_vector(&m_vector, file_name.c_str());
        if(err)
        {
            KJB_THROW_3(kjb::IO_error, "File not found %s", (file_name.c_str()));
        }
    }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector& Vector::operator=(const Mat_view_type& vec_ref)
{
    Vector tmp(vec_ref);
    this->swap(tmp);
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector& Vector::replace(
    const Vector& iv,
    int offset,
    int start,
    int length)
{
    if(    length < 0 || offset < 0 || start < 0
        || get_length() < (offset + length)
        || iv.get_length() < (start + length) )
    {
        throw Illegal_argument("Vector::replace, bad indices");
    }
    const Value_type* src = iv.m_vector->elements + start;
    Value_type* dest = m_vector->elements + offset;
    std::copy( src, src + length, dest );
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector& Vector::resize(int new_length, Value_type pad)
{
    // Test program was HERE.
    if(new_length <= m_vector->max_length)
    {
        if(new_length > m_vector->length)
            std::fill(
                m_vector->elements + m_vector->length, 
                m_vector->elements + new_length,
                pad);

        m_vector->length = new_length;
    }
    else
    {
        Vector temp( new_length, pad );
        Value_type *src = m_vector->elements;
        int copy_length = std::min( get_length(), new_length );
        std::copy( src, src + copy_length, temp.m_vector->elements );
        swap( temp );
    }
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector_view Vector::operator[](const Index_range& idx)
{
    return Vector_view(*this, idx);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Const_vector_view Vector::operator[](const Index_range& idx) const
{
    return Const_vector_view(*this, idx);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector::iterator Vector::insert(iterator position, value_type t)
{
    const size_type offset = position - begin();
    ASSERT(offset <= size());

    m_ensure_capacity(m_vector->length + 1);
    position = begin() + offset;

    iterator old_end = this->end();

    m_vector->length++;

    std::copy(position, old_end, position+1);

    *position = t;

    return position;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

/**
 * @brief Resizes to N, filling all values with t.
 */
void Vector::assign(size_type N, value_type t)
{
    // TODO: this cold be faster.  calling resize() is overkill,
    // because it copies existing elements to the new vector, 
    // which will be immediately overwritten by std::fill_n on the 
    // next line.
    //
    // Suggest adding a "realloc" method similar to kjb::Matrix that
    // implements "resize without copy" semantics.
    this->resize(N); 
    std::fill_n(this->begin(), N, t);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

void Vector::insert(iterator position, size_type N, value_type t)
{
    const size_type offset = position - begin();
    ASSERT(offset <= size());

    m_ensure_capacity(m_vector->length + N);
    position = begin() + offset;

    iterator old_end = this->end();

    m_vector->length += N;

    // shift old data
    std::copy(position, old_end, position+N);

    // copy new data
    std::fill_n(position, N, t);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */


Vector::iterator Vector::erase(iterator position)
{
    iterator it;
    for(it = position; it != end() - 1; it++)
    {
        *it = *(it+1);
    }

    m_vector->length--;

    return position;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

void Vector::erase(iterator begin_, iterator end_)
{
    size_t N = end_ - begin_;

    for(iterator it = begin_; it != end() - N; it++)
    {
        *it = *(it+N);
    }

    m_vector->length -= N;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector& Vector::operator+= (double x)
{
    for(int i = 0; i < this->size(); ++i)
        m_vector->elements[i] += x;
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector& Vector::operator-= (double x)
{
    for(int i = 0; i < this->size(); ++i)
        m_vector->elements[i] -= x;
    return *this;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector create_gauss_random_vector(int length)
{
    // Test program was HERE.
    kjb_c::Vector* result = 0;
    ETX( kjb_c::get_gauss_random_vector( &result, length ) );
    return Vector(result);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector operator*(const Vector& op1, const Matrix& op2)
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

    kjb_c::Vector* result = 0;
    ETX(kjb_c::multiply_vector_and_matrix( &result, op1.get_c_vector(),
                                                    op2.get_c_matrix() ));
    return Vector(result);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Vector operator*(const Matrix& op1, const Vector& op2)
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
    Vector::Impl_type* result = 0;
    ETX(kjb_c::multiply_matrix_and_vector( &result, op1.get_c_matrix(),
                                                    op2.get_c_vector() ));
    return Vector( result );
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

bool operator==(const Vector& op1, const Vector::Impl_type& op2)
{
    if ( op1.get_c_vector() == &op2 )
    {
        // Test program was HERE.
        // If the vectors are the SAME vector then they are trivially equal.
        return true;
    }
#ifdef NOT_REACHED_BUT_CAUSES_WARING
    else if ( 0 == op1.get_c_vector() || 0 == &op2 )
    {
        /* If you reach here, they are not both nil, yet one of them is.
         * Actually you should never reach here since we try to maintain the
         * invariant that a Vector's m_vector pointer never equals NULL.
         * But if that invariant were to be relaxed, then the answer should be
         * false.
         */
        UNTESTED_CODE(); // Unreachable line (as of 11 Feb. 2010)
        return false;
    }
#endif 
    else if ( op1.get_length() != op2.length )
    {
        // Test program was HERE.
        return false;
    }
    else
    {
        // Test program was HERE.
        return ( kjb_c::max_abs_vector_difference( op1.get_c_vector(), &op2 ) < FLT_EPSILON);
    }
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

double Vector::get_max_abs_difference(const Vector & op2) const
{
    return kjb_c::max_abs_vector_difference( get_c_vector(), op2.get_c_vector() );
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

bool operator<(const Vector& op1, const Vector &op2)
{
    for(int i = 0; i < op1.get_length(); i++)
    {
        if(op1[i] < op2[i])
            return true;
        if(op1[i] > op2[i])
            return false;

        // else equal, try next element.
    }

    if(op1.get_length() < op2.get_length())
        return true;
    else
        return false;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector& Vector::mapcar( Mapper fun )
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

Matrix Vector::hat() const
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

    Matrix result( m_vector->length, m_vector->length, Value_type(0) );

    result(0,1) = -(*this)(2);
    result(0,2) =  (*this)(1);
    result(1,0) =  (*this)(2);
    result(1,2) = -(*this)(0);
    result(2,0) = -(*this)(1);
    result(2,1) =  (*this)(0);

    return result;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector& Vector::cross_with( const Vector& op2 )
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
    return operator=(hat() * op2);
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Vector cross( const Vector& op1, const Vector& op2 )
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

std::ostream& operator<<(std::ostream& out, const Vector& v)
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

std::ostream& stream_write_vector(std::ostream& ost, const Vector& v)
{
    // Test program was HERE.
    std::streamsize p = ost.precision();
    ost << v.size();
    ost << std::setprecision( std::numeric_limits<double>::digits10+2);
    for(int i = 0; i < v.get_length(); i++)
    {
        ost << ' ' << v(i);
    }

    ost.precision( p );
    return ost;
    
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

std::istream& stream_read_vector(std::istream& ist, Vector& v)
{
    kjb::Vector::size_type n;
    ist >> n;
    v.resize(n);
    
    for(kjb::Vector::size_type i = 0; i < n; ++i)
    {
        ist >> v[i];
    }

    return ist;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

std::vector<Vector> get_transpose(const std::vector<Vector>& m)
{
    const size_t num_vecs = m.size();
    const size_t vec_len = m[0].get_length();

    std::vector<Vector> m_t(vec_len, Vector(num_vecs));

    for(size_t i = 0; i < num_vecs; i++)
    {
        IFT(m[i].get_length() == (int) vec_len, kjb::Dimension_mismatch,
            "Vectors transpose: all Vectors must be same size.");

        for(size_t j = 0; j < vec_len; j++)
        {
            m_t[j][i] = m[i][j];
        }
    }

    return m_t;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

Int_vector floor( const Vector& realv )
{
    Int_vector intv( realv.size() );
    if ( realv.size() )
    {
        std::transform( realv.begin(), realv.end(), intv.begin(), iFloor() );
    }
    return intv;
}

/* /\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ */

//@author Josh Bowdish
kjb::Vector create_uniformly_spaced_vector(double a,double b,unsigned n){

    kjb::Vector toreturn(n);

    double stepsize;
    if(n==1){
        stepsize = b-a;
    }
    else{   
        stepsize = (b-a)/(n-1);
    }
    for(unsigned i = 0; i<n; i++){
        toreturn[i] = a+(stepsize*i);
    }

    return toreturn;
}

} //namespace kjb

