/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kyle Simek, Andrew Predoehl.                                       |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: m_matrix.cpp 21596 2017-07-30 23:33:36Z kobus $ */

/**
 * @file
 * @author Kyle Simek
 * @author Andrew Predoehl
 * @brief Definition for the Matrix class methods.
 *
 * The Matrix class is a thin wrapper on the KJB Matrix struct
 * and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * l_int_matrix.cpp,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m/m_mat_metric.h"
#include "m/m_mat_flip.h"
#include "m/m_mat_stat.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_mat_view.h"
#include "m_cpp/m_vec_view.h"
#include "n/n_svd.h"
#include "n/n_invert.h"
#include "i/i_float_io.h"
#include "l_cpp/l_int_matrix.h"
#include "l_cpp/l_util.h"
#include "sample/sample_gauss.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <sstream>



namespace {

void throw_cannot( const std::string& task )
{
    std::string whine( "Could not create 3d " );
    whine += task + " matrix";
    KJB_THROW_2( kjb::KJB_error, whine.c_str() );
}

void throw_cannot_rot()
{
    throw_cannot("rotation");
}


void throw_cannot_scale()
{
    throw_cannot("scaling");
}


void throw_cannot_homog_scale()
{
    throw_cannot("homogeneous scaling");
}


void throw_cannot_homog_xlate()
{
    throw_cannot("homogeneous translation");
}

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


namespace kjb {

void Matrix::throw_bad_bounds( int row_ix, int col_ix ) const
{
    // Test program was HERE.
    std::ostringstream msg;
    msg << "Invalid Matrix access at (" << row_ix << ',' << col_ix
        << ").  Matrix size is " << get_num_rows() << " x "
        << get_num_cols() << ".\n";
    KJB_THROW_2( Index_out_of_bounds, msg.str() );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Matrix(int rows, int cols, const Value_type* data)
    : m_matrix( 0 )
{
    // Test program was HERE.
    Matrix result( rows, cols );

    for ( int row = 0; row < rows; ++row )
    {
        for ( int col = 0; col < cols; ++col )
        {
            result( row, col ) = *data++;
        }
    }
    swap( result );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Matrix(const std::string& file_name)
    : m_matrix( 0 )
{
    if ( file_name.length() == 0 )
    {
        // Test program was HERE.
        KJB_THROW_2(Illegal_argument, "Cannot read matrix from file:  "
                                            "filename is empty." );
    }
    else
    {
        // Test program was HERE.
        int err = kjb_c::read_matrix(&m_matrix, file_name.c_str());
        if(err)
        {
            KJB_THROW_3(kjb::IO_error, "File not found %s", (file_name.c_str()));
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Matrix( const Vec_type& vec)
    : m_matrix(0)
{
    Matrix result(vec.size(), 1);
    for(int i = 0; i < vec.size(); i++)
    {
        result(i, 0) = vec[i];
    }

    swap( result );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Matrix( const Int_matrix& imat )
    : m_matrix( 0 )
{
    // Test program was HERE.
    Matrix result( imat.get_num_rows(), imat.get_num_cols() );
    for( int row = 0; row < imat.get_num_rows(); ++row )
    {
        for( int col = 0; col < imat.get_num_cols(); ++col )
        {
            result( row, col ) = imat( row, col );
        }
    }
    swap( result );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Matrix( const Matrix_view& mat_view) :
    m_matrix(0)
{
    init_from_view_(mat_view);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Matrix( const Const_matrix_view& mat_view) :
    m_matrix(0)
{
    init_from_view_(mat_view);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix& Matrix::operator=(const Impl_type& mat_ref)
{
    if ( m_matrix != &mat_ref )
    {
        // Test program was HERE.
        ETX( kjb_c::copy_matrix( &m_matrix, &mat_ref ) );
    }
    //else
    //{
    //    // Test program was HERE.
    //}
    return *this;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix_vector_view Matrix::operator[](const Index_range& i)
{
    return Matrix_vector_view(*this, i);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Const_matrix_vector_view Matrix::operator[](const Index_range& i) const
{
    return Const_matrix_vector_view(*this, i);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix_view Matrix::operator()(const Index_range& rows, const Index_range& cols)
{
    return Matrix_view(*this, rows, cols);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Const_matrix_view Matrix::operator()(const Index_range& rows, const Index_range& cols) const
{
    return Const_matrix_view(*this, rows, cols);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Value_type& Matrix::at(int i)
{
    // Test program was HERE.
    int row, col;
    compute_row_col_carefully( i, &row, &col );
    check_bounds( row, col );
    return operator()( row, col );
}

const Matrix::Value_type& Matrix::at(int i) const
{
    // Test program was HERE.
    int row, col;
    compute_row_col_carefully( i, &row, &col );
    check_bounds( row, col );
    return operator()( row, col );
}

Matrix::Value_type& Matrix::at(int row, int col)
{
    // Test program was HERE.
    check_bounds( row, col );
    return operator()(row, col);
}

const Matrix::Value_type& Matrix::at(int row, int col) const
{
    // Test program was HERE.
    check_bounds( row, col );
    return operator()(row, col);
}

Matrix& Matrix::zero_out( int num_rows, int num_cols )
{
    // Test program was HERE.
    ETX( kjb_c::get_zero_matrix( &m_matrix, num_rows, num_cols ) );
    return *this;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Resize matrix to be square, and clobber contents with zeroes.
 * @return  an lvalue of this object
 * @see     create_zero_matrix(int) -- more appropriate in some
 *          situations.
 */
Matrix& Matrix::zero_out( int rows )
{
    // Test program was HERE.
    return zero_out( rows, rows );
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Clobber contents of matrix with zeroes.
 * @return  an lvalue of this object
 * @see     create_zero_matrix() -- more appropriate in some situations.
 */
Matrix& Matrix::zero_out()
{
    // Test program was HERE.
    return zero_out( get_num_rows(), get_num_cols() );
}

Matrix& Matrix::limit_values(Value_type low, Value_type high)
{
    for(int i = 0; i < get_length(); i++)
    {
        if(this->operator()(i) < low)
        {
            this->operator()(i) = low;
        }

        if(this->operator()(i) > high)
        {
            this->operator()(i) = high;
        }
    }

    return *this;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix create_row_matrix( const Matrix::Vec_type& vec )
{
    // Test program was HERE.
    Matrix result( 1, vec.get_length() );
    for ( int col = 0; col < vec.get_length(); ++col )
    {
        result( 0, col ) = vec( col );
    }
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix create_column_matrix( const Matrix::Vec_type& vec )
{
    // Test program was HERE.
    Matrix result( vec.get_length(), 1 );
    for ( int row = 0; row < vec.get_length(); ++row )
    {
        result( row, 0 ) = vec( row );
    }
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix create_gauss_random_matrix( int num_rows, int num_cols )
{
    kjb_c::Matrix* c_mat = NULL;
    ETX(kjb_c::get_gauss_random_matrix(&c_mat, num_rows, num_cols));
    return Matrix(c_mat);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix create_diagonal_matrix(const Matrix::Vec_type& diagonal)
{
    // Test program was HERE.
    kjb_c::Matrix* result = 0;
    ETX(kjb_c::get_diagonal_matrix(&result, diagonal.get_c_vector()));
    return Matrix(result);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix create_diagonal_matrix(const Matrix::Vec_type& diagonal, size_t n)
{
    // Test program was HERE.
    Matrix diag = create_diagonal_matrix(diagonal);
    size_t d = diagonal.get_length();
    Matrix result(n * d, n * d, 0.0);
    for(size_t i = 0; i < n; i++)
    {
        result.replace(i * d, i * d, diag);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

//Matrix& Matrix::init_euler_zxz(float phi, float theta, float psi, bool homogeneous)
//{
//    //!!KJB(UNTESTED_CODE());
//
//    int err;
//
//    if(homogeneous) {
//        ETX(kjb_c::get_euler_matrix(&m_matrix, phi, theta, psi, KJB_EULER_4_ZXZ));
//    } else {
//        ETX(kjb_c::get_euler_matrix(&m_matrix, phi, theta, psi, KJB_EULER_3_ZXZ));
//    }
//
//    return *this;
//}

Matrix Matrix::transpose() const
{
    // Test program was HERE.
    kjb_c::Matrix* result = 0;
    ETX( kjb_c::get_matrix_transpose( &result, m_matrix ) );
    return Matrix(result);
    //return matrix_transpose(*this);
}

Matrix Matrix::inverse() const
{
    // Test program was HERE.
    kjb_c::Matrix* result = 0;
    ETX( kjb_c::get_matrix_inverse( &result, m_matrix ) );
    return Matrix(result);
    //return matrix_inverse(*this);
}

double Matrix::abs_of_determinant() const
{
    double det;
    // Test program was HERE.
    ETX( kjb_c::get_determinant_abs( m_matrix, &det ) );
    return det;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ **/

Int_matrix Matrix::ceil() const
{
    // Test program was HERE.
    return create_int_matrix_from_matrix_ceil( *this );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix Matrix::floor() const
{
    // Test program was HERE.
    return create_int_matrix_from_matrix_floor( *this );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix Matrix::threshold(double t) const
{
    const int num_rows = get_num_rows();
    const int num_cols = get_num_cols();

    Int_matrix result(num_rows, num_cols);

    for(int row = 0; row < num_rows; row++)
    for(int col = 0; col < num_cols; col++)
    {
        result(row, col) = (operator()(row, col) >= t ? 1 : 0);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

Matrix& Matrix::resize(int new_rows, int new_cols, Value_type pad)
{
    // Test program was HERE.

    int num_elements = new_cols * new_rows;
    if(num_elements <= m_matrix->max_num_elements)
    {
        // This is the same logic as in get_target_matrix, so should be safe
        if (new_cols == m_matrix->num_cols && 
            new_rows <= m_matrix->max_num_rows) 
        {
            // if num_rows increases, pad it (if not, no-op)
            int old_rows = m_matrix->num_rows;

            for(int i = old_rows; i < new_rows; ++i)
            {
                std::fill(
                        m_matrix->elements[i],
                        m_matrix->elements[i] + new_cols,
                        pad);
            }

            m_matrix->num_rows = new_rows;
            return *this;
        }

#ifdef TEST
        const kjb_c::Matrix* old_m_ptr;
        const double* old_d_ptr;
#endif /*TEST */
        int old_rows = m_matrix->num_rows;
        int old_cols = m_matrix->num_cols;

#ifdef TEST
        if(num_elements > 0)
        {
            old_m_ptr = m_matrix;
            old_d_ptr = m_matrix->elements[0];
        }
#endif /*TEST */

        std::vector<double*> old_row_pointers(m_matrix->elements, m_matrix->elements + old_rows);
        ETX(get_target_matrix(&m_matrix, new_rows, new_cols));

#ifdef TEST
        if(num_elements > 0)
        {
            ASSERT(old_m_ptr == m_matrix);
            ASSERT(old_d_ptr == m_matrix->elements[0]);
        }
#endif /*TEST */

        if(new_cols < old_cols)
        {
            for(int r = 0; r < old_rows; ++r)
            {
                if(old_row_pointers[r] != m_matrix->elements[r])
                {
                    std::copy(
                            old_row_pointers[r],
                            old_row_pointers[r] + new_cols,
                            m_matrix->elements[r]);
                }
            }
        }
        else if(new_cols == old_cols)
        {
            // this case is possible if the max_num_elements had excess unused
            // space before resizing, but max_num_rows did not, so a call to
            // get_target_matrix  was required
            ASSERT(new_rows > old_rows);
        }
        else
        {
            ASSERT(new_cols > old_cols); 

            const int end_row = std::min(new_rows, old_rows)-1;
            for(int r = end_row; r >= 0; --r)
            {
                if(old_row_pointers[r] != m_matrix->elements[r])
                {
                    std::copy_backward(
                            old_row_pointers[r],
                            old_row_pointers[r] + old_cols,
                            m_matrix->elements[r] + old_cols);
                }

                if(new_cols > old_cols)
                {
                    std::fill(
                            m_matrix->elements[r] + old_cols,
                            m_matrix->elements[r] + new_cols,
                            pad);
                }
            }

        }

        // row padding
        for(int r = old_rows; r < new_rows; ++r)
        {
            std::fill(
                    m_matrix->elements[old_rows],
                    m_matrix->elements[new_rows-1] + new_cols,
                    pad);
        }


        return *this;


    }

    // DESIGN DISCUSSION
    //
    // There's an opportunity to avoid an allocation here, by re-using space and
    // moving data around.
    //
    // Two branches are required.  If reallocation is unavoidable, the code below is the
    // best we can do.  If reallocation can be avoided, some logic to re-arrange existing data 
    // is possible.
    //
    // The trouble is, the C library provides no way of determining if a realloc is needed.  Could
    // see if get_target_matrix returns a different pointer, but at that point, we've lost the data
    // we hoped to preserve.  We could take get_target_matrix's reallocation logic and mirror it here, 
    // but that creates a maintainability nightmare if get_target_matrix ever changes.  Best solution
    // is to expose a function (or macro) called "needs_realloc" that is called in get_target_matrix
    // and in this function.  I don't have the time to commit to implementing and testing this right now,
    // but if anyone reading this is game, please consider it!
    // -Kyle, May 12, 2014
    // 
    // Update: this is implemented above, making assumptions about 
    // how get_target_matrix is implemented.  I used asserts to check
    // these assumptions in development mode.
    // -Kyle, September 18 2014
    
    Matrix result( new_rows, new_cols, pad );

    int copy_rows = std::min( new_rows, get_num_rows() );
    int copy_cols = std::min( new_cols, get_num_cols() );

    for ( int row = 0; row < copy_rows; ++row )
    {
        for ( int col = 0; col < copy_cols; ++col )
        {
            result( row, col ) = operator()( row, col );
        }
    }
    swap( result );
    return *this;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Vec_type Matrix::get_row(int row) const
{
    // Test program was HERE.
    check_bounds( row, 0 );
    return Vec_type( get_num_cols(), m_matrix -> elements[ row ] );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Vec_type Matrix::get_col(int col) const
{
    // Test program was HERE.
    check_bounds( 0, col );

    Vec_type result( get_num_rows() );
    for ( int row = 0; row < get_num_rows(); ++row )
    {
        result( row ) = operator()( row, col );
    }
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void Matrix::fill_row(int row, Value_type x)
{
    check_bounds(row, 0);
    
    std::fill_n(
            m_matrix->elements[row],
            this->get_num_cols(),
            x);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void Matrix::fill_col(int col, Value_type x)
{
    check_bounds(0, col);
    
    for(int i = 0; i < this->get_num_rows(); ++i)
    {
        m_matrix->elements[i][col] = x;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Vec_type Matrix::get_diagonal() const
{
    int dlen = std::min(get_num_rows(), get_num_cols());
    Vec_type d(dlen);

    for(int i = 0; i < dlen; i++)
    {
        d[i] = this->operator()(i, i);
    }

    return d;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

double Matrix::trace() const
{
    IFT(get_num_rows() == get_num_cols(), kjb::Dimension_mismatch,
        "Trace is only defined for square matrices.");

    double tr = 0.0;
    for(int i = 0; i < get_num_rows(); i++)
    {
        tr += this->operator()(i, i);
    }

    return tr;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @kludge Should have a separate function for approximate equality with a threshold.  Consult w/Luca, as it's his change. --Kyle CVPR 2010
 *
 */
bool operator==(const Matrix& op1, const Matrix::Impl_type& op2)
{
    if ( op1.get_c_matrix() == &op2 )
    {
        // Test program was HERE.
        // If matrices are the SAME object then they are trivially equal.
        return true;
    }
    else if ( 0 == op1.get_c_matrix() || 0 == &op2 )
    {
        /* If you reach here, they are not both nil but one of them is.
         * You really should NEVER reach here since we try hard to prevent
         * m_matrix from storing NULL, and op2 should not also not equal NULL.
         * But, if you DID reach here, the answer clearly should be "false."
         */

        KJB(UNTESTED_CODE()); // Unreachable line (as of 11 Feb. 2010)
        return false;
    }
    else if (       op1.get_num_rows() != op2.num_rows
                ||  op1.get_num_cols() != op2.num_cols )
    {
        // Test program was HERE.
        return false;
    }
    else
    {
#if 1
        // Exact equality
        // Test program was HERE.
        return 0 == kjb_c::max_abs_matrix_difference(op1.get_c_matrix(), &op2);
#else
        // Luca's approximate equality code
        double threshold = 1e-5;
        double thediff =  kjb_c::max_abs_matrix_difference( op1.get_c_matrix(), &op2 );
        //std::cout << "Diff:" << thediff << " and threshold is:" << threshold << std::endl;
        return (thediff < threshold);
#endif
    }
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

Matrix matrix_inverse( const Matrix& op1 )
{
    kjb_c::Matrix* kjb_matrix = 0;
    ETX( kjb_c::get_matrix_inverse( &kjb_matrix, op1.get_c_matrix() ) );
    return Matrix(kjb_matrix);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix& Matrix::mapcar( Mapper fun )
{
    // Test program was HERE.
    const int ROWSIZE = get_num_rows();
    const int COLSIZE = get_num_cols();
    for( int rrr = 0; rrr < ROWSIZE; ++rrr )
    {
        for( int ccc = 0; ccc < COLSIZE; ++ccc )
        {
            Value_type newval = fun( operator()( rrr, ccc ) );
            operator()( rrr, ccc ) = newval;
        }
    }
    return *this;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix& Matrix::replace(int row, int col, const Matrix& A)
{
    check_bounds(row + A.get_num_rows() - 1, col + A.get_num_cols() - 1);

    for(int i = 0; i < A.get_num_rows(); i++)
    {
        for(int j = 0; j < A.get_num_cols(); j++)
        {
            m_matrix->elements[row + i][col + j] = A(i, j);
        }
    }

    return *this;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix& Matrix::vertcat(const Matrix& A)
{
    if(this->get_num_cols() == 0 && this->get_num_rows() == 0)
        return (*this) = A;

    if(A.get_num_rows() == 0 && A.get_num_cols() == 0)
        return *this;

    IFT(A.get_num_cols() == this->get_num_cols(),
            kjb::Dimension_mismatch,
            "Dimensions of matrices being concatenated are not consistent.");

    size_t row = this->get_num_rows();
    this->resize(this->get_num_rows() + A.get_num_rows(), this->get_num_cols());
    this->replace(row, 0, A);

    return *this;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix& Matrix::horzcat(const Matrix& A)
{
    if(this->get_num_cols() == 0 && this->get_num_rows() == 0)
        return (*this) = A;

    if(A.get_num_rows() == 0 && A.get_num_cols() == 0)
        return *this;

    IFT(A.get_num_rows() == this->get_num_rows(),
            kjb::Dimension_mismatch,
            "Dimensions of matrices being concatenated are not consistent.");

    size_t col = this->get_num_cols();
    this->resize( this->get_num_rows(), this->get_num_cols() + A.get_num_cols());
    this->replace(0, col, A);

    return *this;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

Matrix& Matrix::operator+= (double x)
{
    size_t N = get_length();
    for(size_t i = 0; i < N; ++i)
        m_matrix->elements[0][i] += x;
    return *this;
}


/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

Matrix& Matrix::operator-= (double x)
{
    size_t N = get_length();
    for(size_t i = 0; i < N; ++i)
        m_matrix->elements[0][i] -= x;
    return *this;
}

Matrix& Matrix::shift_rows_by(const Vector& v)
{
    size_t N = get_length();
    for(size_t i = 0; i < N; ++i)
        m_matrix->elements[0][i] += v[i % v.size() ];
    return *this;
}
    
Matrix& Matrix::shift_columns_by(const Vector& v)
{
    int N = get_length();
    for(int i = 0; i < N; ++i)
        m_matrix->elements[0][i] += v[i / v.size() ];
    return *this;
}
    
Matrix& Matrix::ew_multiply_rows_by(const Vector& v)
{
    int N = get_length();
    for(int i = 0; i < N; ++i)
        m_matrix->elements[0][i] *= v[i % v.size() ];
    return *this;
}

Matrix& Matrix::ew_multiply_by(const Matrix& m)
{
    ETX(kjb_c::ow_multiply_matrices_ew(m_matrix, m.m_matrix));
    return *this;
}
    
    
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates an euler rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  All rotation angles are in radian
 *
 *  @param phi    rotation angle around the z axis
 *  @param theta  rotation angle around the x axis
 *  @param psi    rotation angle around the y axis
 *
 */
void Matrix::convert_to_euler_rotation_matrix
(
    float      phi,
    float      theta,
    float      psi
)
{
    // Test program was HERE.
    // Yes, I checked -- the following fun does return void.
    kjb_c::get_euler_rotation_matrix(&m_matrix, phi, theta, psi);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates an euler rotation matrix and stores it into an instance of class Matrix
 *  All rotation angles are in radian
 *
 *  @param phi    rotation angle around the z axis
 *  @param theta  rotation angle around the x axis
 *  @param psi    rotation angle around the y axis
 *
 *  @return an euler rotation matrix
 */
Matrix Matrix::create_euler_rotation_matrix
(
    float      phi,
    float      theta,
    float      psi
)
{
    // Test program was HERE.
    Matrix erm;
    erm.convert_to_euler_rotation_matrix( phi, theta, psi );
    return erm;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates an euler homogeneous rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  All rotation angles are in radian
 *
 *  @param phi    rotation angle around the z axis
 *  @param theta  rotation angle around the x axis
 *  @param psi    rotation angle around the y axis
 *
 */
void Matrix::convert_to_euler_homo_rotation_matrix
(
    float      phi,
    float      theta,
    float      psi
)
{
    // Test program was HERE.
    kjb_c::get_euler_homo_rotation_matrix(&m_matrix, phi, theta, psi);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a homogeneous euler rotation matrix, and stores it into an instance of
 *  class Matrix. All rotation angles are in radian
 *
 *  @param phi    rotation angle around the z axis
 *  @param theta  rotation angle around the x axis
 *  @param psi    rotation angle around the y axis
 *
 *  @return a homogeneous euler rotation matrix
 */
Matrix Matrix::create_euler_homo_rotation_matrix
(
    float      phi,
    float      theta,
    float      psi
)
{
    // Test program was HERE.
    Matrix erm;
    erm.convert_to_euler_homo_rotation_matrix( phi, theta, psi );
    return erm;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param x      x-coordinate of the vector to rotate around
 *  @param y      y-coordinate of the vector to rotate around
 *  @param z      z-coordinate of the vector to rotate around
 *
 */
void Matrix::convert_to_3d_rotation_matrix
(
    double      phi,
    double      x,
    double      y,
    double      z
)
{
    int rc = kjb_c::get_3d_rotation_matrix_2(&m_matrix, phi, x, y, z);

    if ( rc != kjb_c::NO_ERROR )
    {
        // Test program was HERE.
        throw_cannot_rot();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counter clockwise)
 *                around the origin
 */
void Matrix::convert_to_2d_rotation_matrix
(
    double      phi
)
{
    int rc = kjb_c::get_2d_rotation_matrix(&m_matrix, phi);

    if ( rc != kjb_c::NO_ERROR )
    {
        // Test program was HERE.
        throw_cannot_rot();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d rotation matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param x      x-coordinate of the vector to rotate around
 *  @param y      y-coordinate of the vector to rotate around
 *  @param z      z-coordinate of the vector to rotate around
 *
 *  @return a 3d rotation matrix
 */
Matrix Matrix::create_3d_rotation_matrix
(
    double   phi,
    double     x,
    double     y,
    double     z
)
{
    // Test program was HERE.
    Matrix r;
    r.convert_to_3d_rotation_matrix( phi, x, y, z );
    return r;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 2d rotation matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counter clockwise) around
 *                the origin
 *
 *  @return a 2d rotation matrix
 */
Matrix Matrix::create_2d_rotation_matrix
(
    double   phi
)
{
    // Test program was HERE.
    Matrix r;
    r.convert_to_2d_rotation_matrix( phi);
    return r;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param vec    the vector to rotate around
 *
 *  @deprecated   Use convert_to_3d_rotation_matrix( phi, vec ) in new code.
 */
void Matrix::convert_to_3d_rotation_matrix_from_vector
(
    double          phi,
    const Vector&   vec
)
{
    // Test program was HERE.
    convert_to_3d_rotation_matrix( phi, vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param vec    the vector to rotate around
 *
 */
void Matrix::convert_to_3d_rotation_matrix
(
    double          phi,
    const Vector&   vec
)
{
    // Test program was HERE.
    int rc=kjb_c::get_3d_rotation_matrix_1(&m_matrix, phi, vec.get_c_vector());

    if ( rc != kjb_c::NO_ERROR )
    {
        // Test program was HERE.
        throw_cannot_rot();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d rotation matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle
 *  @param vec    the vector to rotate around
 *
 *  @return a 3d rotation matrix
 *
 *  @deprecated     Use create_3d_rotation_matrix( phi, vec ) in new code.
 */
Matrix Matrix::create_3d_rotation_matrix_from_vector
(
    double          phi,
    const Vector&   vec
)
{
    // Test program was HERE.
    return create_3d_rotation_matrix( phi, vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d rotation matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle
 *  @param vec    the vector to rotate around
 *
 *  @return a 3d rotation matrix
 */
Matrix Matrix::create_3d_rotation_matrix
(
    double          phi,
    const Vector&   vec
)
{
    // Test program was HERE.
    Matrix r;
    r.convert_to_3d_rotation_matrix( phi, vec );
    return r;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle
 *  @param x      x-coordinate of the vector to rotate around
 *  @param y      y-coordinate of the vector to rotate around
 *  @param z      z-coordinate of the vector to rotate around
 *
 */
void Matrix::convert_to_3d_homo_rotation_matrix
(
    double      phi,
    double      x,
    double      y,
    double      z
)
{
    // Test program was HERE.
    int rc = kjb_c::get_3d_homo_rotation_matrix_2(&m_matrix, phi, x, y, z);

    if ( rc != kjb_c::NO_ERROR )
    {
        // Test program was HERE.
        throw_cannot_rot();
    }
    // else // Test program was HERE.
}

/**
 *  Creates a 3d homogeneous rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counter clockwise) around
 *  the origin
 *
 */
void Matrix::convert_to_2d_homo_rotation_matrix
(
    double      phi
)
{
    // Test program was HERE.
    int rc = kjb_c::get_2d_homo_rotation_matrix(&m_matrix, phi);

    if ( rc != kjb_c::NO_ERROR )
    {
        // Test program was HERE.
        throw_cannot_rot();
    }
    // else // Test program was HERE.
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous rotation matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian.
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param x      x-coordinate of the vector to rotate around
 *  @param y      y-coordinate of the vector to rotate around
 *  @param z      z-coordinate of the vector to rotate around
 *
 *  @return a 3d homogeneous rotation matrix
 */
Matrix Matrix::create_3d_homo_rotation_matrix
(
    double  phi,
    double  x,
    double  y,
    double  z
)
{
    // Test program was HERE.
    Matrix r;
    r.convert_to_3d_homo_rotation_matrix( phi, x, y, z );
    return r;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous rotation matrix and stores it into an instance
 *  of class Matrix. The rotation angle is in radian.
 *
 *  @param phi    rotation angle (positive means counterclockwise) around the
 *                origin
 *
 *  @return a 2d homogeneous rotation matrix
 */
Matrix Matrix::create_2d_homo_rotation_matrix
(
    double  phi
)
{
    // Test program was HERE.
    Matrix r;
    r.convert_to_2d_homo_rotation_matrix( phi);
    return r;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param vec    the vector to rotate around
 *
 *  @deprecated Use convert_to_3d_homo_rotation_matrix( phi, vec) in new code.
 */
void Matrix::convert_to_3d_homo_rotation_matrix_from_vector
(
    double          phi,
    const Vector&   vec
)
{
    // Test program was HERE.
    convert_to_3d_homo_rotation_matrix( phi, vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous rotation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param vec    the vector to rotate around
 *
 */
void Matrix::convert_to_3d_homo_rotation_matrix
(
    double          phi,
    const Vector&   vec
)
{
    // Test program was HERE.
    int rc = kjb_c::get_3d_homo_rotation_matrix_1( &m_matrix, phi,
                                                        vec.get_c_vector() );
    if ( rc != kjb_c::NO_ERROR )
    {
        // Test program was HERE.
        throw_cannot_rot();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous rotation matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param vec    the vector to rotate around
 *
 *  @return a 3d homogeneous rotation matrix
 *
 *  @deprecated Use create_3d_homo_rotation_matrix( phi, vec ) in new code.
 */
Matrix Matrix::create_3d_homo_rotation_matrix_from_vector
(
    double          phi,
    const Vector&   vec
)
{
    // Test program was HERE.
    return create_3d_homo_rotation_matrix( phi, vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous rotation matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian
 *
 *  @param phi    rotation angle (positive means counterclockwise)
 *  @param vec    the vector to rotate around
 *
 *  @return a 3d homogeneous rotation matrix
 */
Matrix Matrix::create_3d_homo_rotation_matrix
(
    double          phi,
    const Vector&   vec
)
{
    // Test program was HERE.
    Matrix r;
    r.convert_to_3d_homo_rotation_matrix( phi, vec );
    return r;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d scaling matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param x      the amount of scaling along the x axis
 *  @param y      the amount of scaling along the y axis
 *  @param z      the amount of scaling along the z axis
 *
 */
void Matrix::convert_to_3d_scaling_matrix
(
    double      x,
    double      y,
    double      z
)
{
    // Test program was HERE.
    int rc = kjb_c::get_3d_scaling_matrix_2(&m_matrix, x, y, z);

    if ( rc != kjb_c::NO_ERROR )
    {
        KJB(UNTESTED_CODE()); // Unreachable line (as of 11 Feb. 2010)
        throw_cannot_scale();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d scaling matrix and stores it into an instance of class Matrix
 *
 *  @param x      x-coordinate of the vector to rotate around
 *  @param y      y-coordinate of the vector to rotate around
 *  @param z      z-coordinate of the vector to rotate around
 *
 *  @return a 3d scaling matrix
 */
Matrix Matrix::create_3d_scaling_matrix
(
    double     x,
    double     y,
    double     z
)
{
    // Test program was HERE.
    Matrix s;
    s.convert_to_3d_scaling_matrix( x, y, z );
    return s;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d scaling matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param vec    the vector containing the amount of scaling
 *                along the x, y, and z axes respectively
 *
 *  @deprecated Use convert_to_3d_scaling_matrix( vec ) in new code.
 */
void Matrix::convert_to_3d_scaling_matrix_from_vector
(
    const Vector& vec
)
{
    // Test program was HERE.
    convert_to_3d_scaling_matrix( vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d scaling matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param vec    the vector containing the amount of scaling
 *                along the x, y, and z axes respectively
 *
 */
void Matrix::convert_to_3d_scaling_matrix
(
    const Vector& vec
)
{
    // Test program was HERE.
    int rc = kjb_c::get_3d_scaling_matrix_1(&m_matrix, vec.get_c_vector());

    if ( rc != kjb_c::NO_ERROR )
    {
        // Test program was HERE.
        throw_cannot_scale();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d scaling matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian
 *
 *  @param vec    the vector to rotate around
 *
 *  @return a 3d scaling matrix
 *
 *  @deprecated Use create_3d_scaling_matrix( vec ) in new code.
 */
Matrix Matrix::create_3d_scaling_matrix_from_vector
(
    const Vector& vec
)
{
    // Test program was HERE.
    return create_3d_scaling_matrix( vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d scaling matrix and stores it into an instance of class Matrix
 *  The rotation angle is in radian
 *
 *  @param vec    the vector to rotate around
 *
 *  @return a 3d scaling matrix
 */
Matrix Matrix::create_3d_scaling_matrix
(
    const Vector& vec
)
{
    // Test program was HERE.
    Matrix s;
    s.convert_to_3d_scaling_matrix( vec );
    return s;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous scaling matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param x      the amount of scaling along the x axis
 *  @param y      the amount of scaling along the y axis
 *  @param z      the amount of scaling along the z axis
 *
 */
void Matrix::convert_to_3d_homo_scaling_matrix
(
    double      x,
    double      y,
    double      z
)
{
    // Test program was HERE.
    int rc = kjb_c::get_3d_homo_scaling_matrix_2(&m_matrix, x, y, z);

    if ( rc != kjb_c::NO_ERROR )
    {
        KJB(UNTESTED_CODE()); // Unreachable line (as of 11 Feb. 2010)
        throw_cannot_homog_scale();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous scaling matrix and stores it into an instance of class Matrix
 *
 *  @param x      the amount of scaling along the x axis
 *  @param y      the amount of scaling along the y axis
 *  @param z      the amount of scaling along the z axis
 *
 *  @return a 3d homogeneous scaling matrix
 */
Matrix Matrix::create_3d_homo_scaling_matrix
(
    double     x,
    double     y,
    double     z
)
{
    // Test program was HERE.
    Matrix s;
    s.convert_to_3d_homo_scaling_matrix( x, y, z );
    return s;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous scaling matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param vec    the vector containing the amount of scaling
 *                along the x, y, and z axes respectively
 *  @deprecated   Use convert_to_3d_homo_scaling_matrix( vec ) in new code.
 */
void Matrix::convert_to_3d_homo_scaling_matrix_from_vector(const Vector & vec)
{
    // Test program was HERE.
    convert_to_3d_homo_scaling_matrix( vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous scaling matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param vec    the vector containing the amount of scaling
 *                along the x, y, and z axes respectively
 */
void Matrix::convert_to_3d_homo_scaling_matrix
(
    const Vector&   vec
)
{
    // Test program was HERE.
    int rc = kjb_c::get_3d_homo_scaling_matrix_1(&m_matrix,vec.get_c_vector());

    if ( rc != kjb_c::NO_ERROR)
    {
        // Test program was HERE.
        throw_cannot_homog_scale();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous scaling matrix and stores it into an instance of class Matrix
 *
 *  @param vec    the vector containing the amount of scaling
 *                along the x, y, and z axes respectively
 *
 *  @return a 3d rotation matrix
 *  @deprecated   Use create_3d_homo_scaling_matrix( vec ) in new code.
 */
Matrix Matrix::create_3d_homo_scaling_matrix_from_vector( const Vector& vec )
{
    // Test program was HERE.
    return create_3d_homo_scaling_matrix( vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous scaling matrix and stores it into an instance of class Matrix
 *
 *  @param vec    the vector containing the amount of scaling
 *                along the x, y, and z axes respectively
 *
 *  @return a 3d rotation matrix
 */
Matrix Matrix::create_3d_homo_scaling_matrix
(
    const Vector&   vec
)
{
    // Test program was HERE.
    Matrix s;
    s.convert_to_3d_homo_scaling_matrix( vec );
    return s;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous translation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param x      the amount of translation along the x axis
 *  @param y      the amount of translation along the y axis
 *  @param z      the amount of translation along the z axis
 *
 */
void Matrix::convert_to_3d_homo_translation_matrix
(
    double      x,
    double      y,
    double      z
)
{
    // Test program was HERE.
    int rc = kjb_c::get_3d_homo_translation_matrix_2(&m_matrix, x, y, z);

    if ( rc != kjb_c::NO_ERROR)
    {
        KJB(UNTESTED_CODE()); // Unreachable line (as of 11 Feb. 2010)
        throw_cannot_homog_xlate();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous translation matrix and stores it into an instance of class Matrix
 *
 *  @param x      the amount of translation along the x axis
 *  @param y      the amount of translation along the y axis
 *  @param z      the amount of translation along the z axis
 *
 *  @return a 3d homogeneous scaling matrix
 */
Matrix Matrix::create_3d_homo_translation_matrix
(
    double     x,
    double     y,
    double     z
)
{
    // Test program was HERE.
    Matrix t;
    t.convert_to_3d_homo_translation_matrix( x, y, z );
    return t;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous translation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param vec    the vector containing the amount of translation
 *                along the x, y, and z axes respectively
 *  @deprecated   Use convert_to_3d_homo_translation_matrix( vec ) in new code.
 */
void Matrix::convert_to_3d_homo_translation_matrix_from_vector
(
    const Vector&   vec
)
{
    // Test program was HERE.
    convert_to_3d_homo_translation_matrix( vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous translation matrix and stores it into m_matrix.
 *  m_matrix is not reallocated if its dimensions are 3X3
 *
 *  @param vec    the vector containing the amount of translation
 *                along the x, y, and z axes respectively
 *
 */
void Matrix::convert_to_3d_homo_translation_matrix
(
    const Vector&   vec
)
{
    // Test program was HERE.
    int rc = kjb_c::get_3d_homo_translation_matrix_1( &m_matrix,
                                                        vec.get_c_vector() );

    if ( rc != kjb_c::NO_ERROR)
    {
        // Test program was HERE.
        throw_cannot_homog_xlate();
    }
    // else // Test program was HERE.
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous translation matrix and stores it into an instance of class Matrix
 *
 *  @param vec    the vector containing the amount of translation
 *                along the x, y, and z axes respectively
 *
 *  @return a 3d translation matrix
 *  @deprecated   Use create_3d_homo_translation_matrix( vec ) in new code.
 */
Matrix Matrix::create_3d_homo_translation_matrix_from_vector
(
    const Vector& vec
)
{
    // Test program was HERE.
    return create_3d_homo_translation_matrix( vec );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 *  Creates a 3d homogeneous translation matrix and stores it into an instance of class Matrix
 *
 *  @param vec    the vector containing the amount of translation
 *                along the x, y, and z axes respectively
 *
 *  @return a 3d translation matrix
 */
Matrix Matrix::create_3d_homo_translation_matrix
(
    const Vector&   vec
)
{
    // Test program was HERE.
    Matrix t;
    t.convert_to_3d_homo_translation_matrix( vec );
    return t;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


int Matrix::display(const char* title) const
{
    int result;

    ETX((result = kjb_c::display_matrix(m_matrix,title)));
    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Vec_type Matrix::filter(bool (*f)(Matrix::Value_type x))
{
    Vector v;
    
    const int rows = get_num_rows();
    const int cols = get_num_cols();
    
    for(int i = 0; i < rows; i++)
    {
        for(int j = 0; j < cols; j++)
        {
            if(f((*this)(i, j)))
            {
                v.push_back((*this)(i, j));
            }
        }
    }
    
    return v;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Value_type Matrix::reduce(
    Matrix::Value_type (*f)(Matrix::Value_type x, Matrix::Value_type y),
    Matrix::Value_type init
)   const
{
    KJB(UNTESTED_CODE());
    const int rows = get_num_rows();
    const int cols = get_num_cols();
    
    Matrix::Value_type x = f(init, (*this)(0, 0));

    for (int i = 0; i < rows; i++)
    {
        for (int j = i == 0 ? 1 : 0; j < cols; j++)
        {
            x = f(x, (*this)(i, j));
        }
    }
    return x;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void Matrix::ow_add_scalar(Value_type c)
{
    ETX(kjb_c::ow_add_scalar_to_matrix(m_matrix, c));
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/// @brief add a row vector to each row of a matrix, in place
void Matrix::ow_add_row_vector(const Vec_type v)
{
    ETX(kjb_c::ow_add_row_vector_to_matrix(m_matrix, v.get_c_vector()));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void Matrix::ow_add_col_vector(const Vec_type v)
{
    ETX(kjb_c::ow_add_col_vector_to_matrix(m_matrix, v.get_c_vector()));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void Matrix::ow_multiply_col_vector_ew(const Vec_type v)
{
    ETX(kjb_c::ow_multiply_matrix_by_col_vector_ew(m_matrix, v.get_c_vector()));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void Matrix::ow_multiply_row_vector_ew(const Vec_type v)
{
    ETX(kjb_c::ow_multiply_matrix_by_row_vector_ew(m_matrix, v.get_c_vector()));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void Matrix::ow_vertical_flip()
{
    ETX(kjb_c::ow_vertical_flip_matrix(m_matrix));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

void Matrix::ow_horizontal_flip()
{
    ETX(kjb_c::ow_horizontal_flip_matrix(m_matrix));
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Matrix::Value_type max_abs_difference( const Matrix& op1, const Matrix& op2 )
{
    if (    op1.get_num_rows() != op2.get_num_rows()
         || op1.get_num_cols() != op2.get_num_cols() )
    {
        // Test program was HERE.
        KJB_THROW( Dimension_mismatch );
    }
    //else
    //{
    //    // Test program was HERE.
    //}
    return kjb_c::max_abs_matrix_difference( op1.get_c_matrix(), op2.get_c_matrix() );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

std::ostream & operator<<(std::ostream& out, const Matrix& m)
{
    // Test program was HERE.
    std::streamsize w = out.width();
    std::streamsize p = out.precision();
    std::ios::fmtflags f = out.flags();

    out << std::scientific;
    for(int row = 0; row < m.get_num_rows(); row++)
    {
        for(int col = 0; col < m.get_num_cols(); col++)
        {
            out << std::setw (16) << std::setprecision(8) << m(row, col);
        }
        out << std::endl;
    }

    out.width( w );
    out.precision( p );
    out.flags( f );
    return out;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

std::istream& stream_read_matrix(std::istream& ist, Matrix& m)
{
    kjb::Matrix::Size_type num_rows, num_cols;
    ist >> num_rows;
    ist >> num_cols;
    m.resize(num_rows, num_cols);
    
    for(kjb::Matrix::Size_type i = 0; i < num_rows; ++i)
    {
        for(kjb::Matrix::Size_type j = 0; j < num_rows; ++j)
        {
            ist >> m(i, j);
        }
    }

    return ist;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

std::ostream& stream_write_matrix(std::ostream& ost, const Matrix& m)
{
    // Test program was HERE.
    std::streamsize p = ost.precision();
    ost << m.get_num_rows() << " ";
    ost << m.get_num_cols() << std::endl;
    ost << std::setprecision( std::numeric_limits<double>::digits10+2);

    ost << std::scientific;
    for(int row = 0; row < m.get_num_rows(); row++)
    {
        for(int col = 0; col < m.get_num_cols(); col++)
        {
            ost << std::setw (16) << std::setprecision(8) << m(row, col);
        }
        ost << std::endl;
    }

    ost.precision( p );
    return ost;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */
double det(const Matrix& mat)
{
    if(mat.get_num_cols() != mat.get_num_rows())
    {
        KJB_THROW_2(Illegal_argument, "Can't find determinant of a non-square matrix.");
    }

    // this is CRYING for a better implementation...
    switch(mat.get_num_rows())
    {
        case 0:
            KJB_THROW_2(Illegal_argument, "Can't find determinanat of an empty matrix.");
        case 1:
            return mat(0,0);
        case 2:
            return mat(0,0) * mat(1,1) - mat(0,1)*mat(1,0);
        case 3:
        {
            int a,b,c,d,e,f,g,h,i;
            a = 0; b = 1; c = 2;
            d = 3; e = 4; f = 5;
            g = 6; h = 7; i = 8;

            return
                mat(a)*mat(e)*mat(i) - 
                mat(a)*mat(f)*mat(h) + 
                mat(b)*mat(f)*mat(g) - 
                mat(b)*mat(d)*mat(i) + 
                mat(c)*mat(d)*mat(h) - 
                mat(c)*mat(e)*mat(g);
        }
        default:
            KJB_THROW_2(Not_implemented, "Determinant isn't implemented for dimension > 4.  Consider using Matrix::abs_of_determinant");
            
    }

}


/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

Matrix::Vec_type sum_matrix_cols( const Matrix& m )
{
    // not an exception-safe way to roll
    Matrix::Vec_type::Impl_type * vp = 00;
    ETX( kjb_c::sum_matrix_cols( &vp, m.get_c_matrix() ) );
    return Matrix::Vec_type( vp );
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

Matrix::Vec_type sum_matrix_rows( const Matrix& m )
{
    // not an exception-safe way to roll
    Matrix::Vec_type::Impl_type * vp = 00;
    ETX( kjb_c::sum_matrix_rows( &vp, m.get_c_matrix() ) );
    return Matrix::Vec_type( vp );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ **/

Int_matrix create_int_matrix_from_matrix_ceil( const Matrix& mat )
{
    // Test program was HERE.
    Int_matrix result( mat.get_num_rows(), mat.get_num_cols() );

    for( int r = 0; r < mat.get_num_rows(); ++r )
    {
        for( int c = 0; c < mat.get_num_cols(); ++c )
        {
            result( r, c ) = static_cast<int>( std::ceil( mat( r, c ) ) );
        }
    }
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix create_int_matrix_from_matrix_floor( const Matrix& mat )
{
    // Test program was HERE.
    Int_matrix result( mat.get_num_rows(), mat.get_num_cols() );

    for( int r = 0; r < mat.get_num_rows(); ++r )
    {
        for( int c = 0; c < mat.get_num_cols(); ++c )
        {
            result( r, c ) = static_cast<int>( std::floor( mat( r, c ) ) );
        }
    }
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix floor( const Matrix& m)
{
    return create_int_matrix_from_matrix_floor(m); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */
Matrix logical_or(Matrix a, Matrix b){

    /*
    std::cout << "a.get_num_rows(): " << a.get_num_rows() << std::endl;
    std::cout << "a.get_num_cols(): " << a.get_num_cols() << std::endl;
    std::cout << "b.get_num_rows(): " << b.get_num_rows() << std::endl;
    std::cout << "b.get_num_cols(): " << b.get_num_cols() << std::endl;
    */
    
    Matrix toreturn = create_zero_matrix(a.get_num_rows(),a.get_num_cols());

    if((a.get_num_rows() != b.get_num_rows()) ||
        a.get_num_cols() != b.get_num_cols()){
        std::cout << "The matricies don't have the same dimensions\n";
    //  return toreturn;
    }
    int total_a = 0;    
    int total_b = 0;    
    int total_return = 0;
    for(int i=0;i<a.get_num_rows();i++){
        for(int j=0;j<a.get_num_cols();j++){

            toreturn(i,j) = 0;

            if(a(i,j)!=0){
                total_a++;
                toreturn(i,j) = 1;
            }
            if(b(i,j)!=0){
                total_b++;
                toreturn(i,j)=1;
            }
        }   
    }

    for(int i=0;i<toreturn.get_num_rows();i++){
        for(int j=0;j<toreturn.get_num_cols();j++){
            if(toreturn(i,j)!=0){
                total_return++;
            }
        }
    }   
    /*
    std::cout << "total_a: "<< total_a << std::endl;
    std::cout << "total_b: "<< total_b << std::endl;
    std::cout << "total_return: "<< total_return << std::endl;
    */
    
    
    return toreturn;
    
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */
Matrix logical_and(Matrix a, Matrix b){

    Matrix toreturn = create_zero_matrix(a.get_num_rows(),a.get_num_cols());
    if((a.get_num_rows() != b.get_num_rows()) ||
        a.get_num_cols() != b.get_num_cols()){
        std::cout << "The matricies don't have the same dimensions\n";
    //  return toreturn; 
    
    }
    for(int i=0;i<a.get_num_rows();i++){
        for(int j=0;j<a.get_num_cols();j++){
            if((a(i,j)!=0 && b(i,j))!=0){
                toreturn(i,j) = 1;
            }
            else{
                toreturn(i,j) = 0;
            }
        }   
    }   
    return toreturn;    
}

Matrix logical_not(Matrix a){

    Matrix toreturn = create_zero_matrix(a.get_num_rows(),a.get_num_cols());
    
    for(int i=0;i<a.get_num_rows();i++){
        for(int j=0;j<a.get_num_cols();j++){
            if(a(i,j) == 0 ){
                toreturn(i,j) = 1;
            }
            else{
                toreturn(i,j) = 0;
            }
        }   
    }   
    return toreturn;    
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

//@author Josh Bowdish
Matrix tile_matrix(Matrix orig,unsigned m,unsigned n){

    //int numorows = orig.get_num_rows();
    //int numocols = orig.get_num_cols();
    int totalrows = orig.get_num_rows()*m;
    //std::cout << "totalrows: " << totalrows << std::endl;
    int totalcols = orig.get_num_cols()*n;
    //std::cout << "totalcols: " << totalcols << std::endl;
    Matrix toreturn = create_zero_matrix(totalrows,totalcols);

    //Going from the top down...
    for(int i=0;i<totalrows;i++){
        for(int j=0;j<totalcols;j++){
            toreturn(i,j) = orig(i%(orig.get_num_rows()),j%(orig.get_num_cols()));
        }   
    }   
    return toreturn;

}//tile_matrix(Matrix,unsigned,unsigned)

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


Matrix tile_matrix(int tehnum, unsigned m, unsigned n)
{
#if 0
    Matrix toreturn = create_zero_matrix(m,n);
    for(unsigned i=0;i<m;i++)
    {
        for(unsigned j=0;j<n;j++)
        {
            toreturn(i,j) = tehnum;
        }
    }
    return toreturn;
#else
    KJB(UNTESTED_CODE());
    KJB(TEST_PSE(("(%s:%d) This function is deprecated.\n",
                                                    __FILE__, __LINE__)));
    return Matrix(m, n, Matrix::Value_type(tehnum));
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

} //namespace kjb
