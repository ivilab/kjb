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

/* $Id: l_int_matrix.cpp 19162 2015-05-26 18:43:28Z cdawson $ */

/** @file
 *
 * @author Kobus Barnard
 * @author Luca Del Pero
 * @author Kyle Simek
 * @author Andrew Predoehl
 *
 * @brief Definition for the Int_matrix class methods.
 *
 * The Int_matrix class is a thin wrapper on the KJB Int_matrix struct
 * and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * m_matrix.cpp,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 */


#include <l_cpp/l_int_matrix.h>
#include <l_cpp/l_int_vector.h>
#include <ostream>
#include <iomanip>
#include <sstream>
#include <string>




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


namespace kjb {

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

void Int_matrix::throw_bad_bounds( int row_ix, int col_ix ) const
{
    // Test program was HERE.
    std::ostringstream msg;
    msg << "Invalid Int_matrix access at (" << row_ix << ',' << col_ix
        << ").  Int_matrix size is " << get_num_rows() << " x "
        << get_num_cols() << ".\n";
    KJB_THROW_2( Index_out_of_bounds, msg.str() );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix::Int_matrix(int rows, int cols, const Value_type* data)
    : m_matrix( 0 )
{
    // Test program was HERE.
    Int_matrix result( rows, cols );

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

Int_matrix::Int_matrix(const std::string& file_name)
    : m_matrix( 0 )
{
    if ( file_name.length() == 0 )
    {
        // Test program was HERE.
        KJB_THROW_2(Illegal_argument, "Cannot read vector from file:  "
                                            "filename is empty." );
    }
    else
    {
        // Test program was HERE.
        int err = kjb_c::read_int_matrix(&m_matrix, file_name.c_str());
        if(err)
        {
            KJB_THROW_3(kjb::IO_error, "File not found %s", (file_name.c_str()));
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix create_row_matrix( const Int_matrix::Vec_type& vec )
{
    // Test program was HERE.
    Int_matrix result( 1, vec.get_length() );
    for ( int col = 0; col < vec.get_length(); ++col )
    {
        result( 0, col ) = vec( col );
    }
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix create_diagonal_matrix(const Int_matrix::Vec_type& diagonal)
{
    kjb_c::Int_matrix* result = 0;
    ETX(kjb_c::get_diagonal_int_matrix(&result, diagonal.get_c_vector()));
    return Int_matrix(result);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix create_column_matrix( const Int_matrix::Vec_type& vec )
{
    // Test program was HERE.
    Int_matrix result( vec.get_length(), 1 );
    for ( int row = 0; row < vec.get_length(); ++row )
    {
        result( row, 0 ) = vec( row );
    }
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

Int_matrix& Int_matrix::resize(int new_rows, int new_cols, Value_type pad)
{
    // Test program was HERE.
    // This is the same logic as in get_target_matrix, so should be safe
    if ((new_cols == m_matrix->num_cols) && (new_rows <= m_matrix->num_rows))
    {
        m_matrix->num_rows = new_rows;
        return *this;
    }
    
    Int_matrix result( new_rows, new_cols, pad );

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

Int_matrix::Vec_type Int_matrix::get_row(int row) const
{
    // Test program was HERE.
    check_bounds( row, 0 );
    return Vec_type( get_num_cols(), m_matrix -> elements[ row ] );
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix::Vec_type Int_matrix::get_col(int col) const
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

Int_matrix& Int_matrix::operator/=(Int_matrix::Value_type op2)
{
    if ( 0 == op2 )
    {
        // Test program was HERE.
        KJB_THROW( Divide_by_zero );
    }
    else
    {
        // Test program was HERE.
        const int ROWSZ = get_num_rows();
        const int COLSZ = get_num_cols();
        for( int row = 0; row < ROWSZ; ++row )
        {
            for( int col = 0; col < COLSZ; ++col )
            {
                operator()( row, col ) /= op2;
            }
        }
    }
    return *this;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

bool operator==(const Int_matrix& op1, const Int_matrix::Impl_type& op2)
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
        // Test program was HERE.
        return 0 == kjb_c::max_abs_int_matrix_difference( op1.get_c_matrix(), &op2 );
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

Int_matrix& Int_matrix::mapcar( Mapper fun )
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

Int_matrix::Vec_type sum_int_matrix_rows( const Int_matrix& m )
{
    // not an exception-safe way to roll
    Int_matrix::Vec_type::Impl_type * vp = 00;
    ETX( kjb_c::sum_int_matrix_rows( &vp, m.get_c_matrix() ) );
    return Int_matrix::Vec_type( vp );
}
    

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

std::ostream & operator<<(std::ostream& out, const Int_matrix& m)
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

/** @} */

}
