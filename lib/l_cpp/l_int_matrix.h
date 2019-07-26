/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Luca Del Pero, Andrew Predoehl.                                    |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: l_int_matrix.h 19162 2015-05-26 18:43:28Z cdawson $ */

#ifndef L_CPP_INT_MATRIX_WRAP_H
#define L_CPP_INT_MATRIX_WRAP_H

/** @file
 *
 * @author Luca Del Pero
 * @author Andrew Predoehl
 * @author Ernesto Brau
 *
 * @brief Definition for the Int_matrix class, a thin wrapper on the KJB
 *        Int_matrix struct and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * m_matrix.h,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 *
 * Although this class has much the same interface as class Matrix, they
 * are not derived from a common abstract interface, because (1) we want as
 * much speed as possible -- this code should be eligible to put inside a
 * tight inner loop; and (2) I don't know whether that would be useful.
 */

#include "l/l_int_matrix.h"
#include "l/l_int_mat_io.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_int_vector.h"
#include <vector>
#include <ostream>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#endif


/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


namespace kjb {

/**
 * @addtogroup kjbLinearAlgebra
 * @{
 */

class Matrix;

/**
 * @brief This class implements matrices, in the linear-algebra sense,
 *        restricted to integer-valued elements.
 *
 * For better maintainability, refer to the element type using this class's
 * Value_type typedef, instead of referring to 'int' directly.
 *
 * Most methods of this class are implemented in the C language portion of the
 * KJB library, with this class forming a thin (usually inlined) layer.
 */
class Int_matrix
{
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif
public:

    /* Class users:  to make your code as maintanable as possible, use the
     * *_type typedefs below instead of directly referencing the specific
     * type.
     */

    typedef int                 Value_type; ///< data type of the elements
    typedef int                 Size_type;   ///< 
    typedef kjb_c::Int_matrix   Impl_type;  ///< the underlying implementation
    typedef Int_matrix          Mat_type;   ///< the associated matrix type
    typedef Int_vector          Vec_type;   ///< the associated vector type
    typedef Value_type (*Mapper)(Value_type);   ///< element transformer fun

private:

    Impl_type* m_matrix;

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
     * @brief   This computes row and column indices, setting them to zero if
     *          the Matrix has no columns.
     * @pre     We assume row and col are non-nil.
     *
     * This routine is used for the one-dimensional (row-major) addressing of
     * the matrix, WITH bounds checking:  correctness has priority over speed.
     */
    void compute_row_col_carefully(
        int  index, ///< [in] Row-major index of some matrix element
        int* row,   ///< [out] Ptr to where to store its computed row index
        int* col    ///< [out] Ptr to where to store its computed column index
    ) const
    {
        if ( 0 == get_num_cols() )
        {
            // Test program was HERE.
            *row = *col = 0;
        }
        else
        {
            // Test program was HERE.
            compute_row_col( index, row, col );
        }
    }

    void throw_bad_bounds( int row_ix, int col_ix ) const;

public:

    /* ------------------------------------------------------------------
     * CONSTRUCTORS
     * ------------------------------------------------------------------ */

    /**
     * @brief   Default ctor builds a matrix of zero rows, zero columns.
     */
    Int_matrix()
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_int_matrix( &m_matrix, 0, 0 ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     */
    Int_matrix(int rows, int cols)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_int_matrix( &m_matrix, rows, cols ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     * @warning Native type for number of rows, columns is int, not unsigned.
     */
    Int_matrix(unsigned rows, unsigned cols)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        Int_matrix m( static_cast<int>( rows ), static_cast<int>( cols ) );
        swap( m );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     * @warning Native type for number of rows, columns is int, not unsigned.
     */
    Int_matrix(unsigned rows, unsigned cols, Value_type val)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        Int_matrix m( static_cast<int>( rows ), static_cast<int>( cols ), val );
        swap( m );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     * @warning Native type for number of rows, cols is int, not unsigned long.
     */
    Int_matrix(unsigned long rows, unsigned long cols)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        Int_matrix m( static_cast<int>( rows ), static_cast<int>( cols ) );
        swap( m );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     * @warning Native type for number of rows, cols is int, not unsigned long.
     */
    Int_matrix(unsigned long rows, unsigned long cols, Value_type val)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        Int_matrix m( static_cast<int>( rows ), static_cast<int>( cols ), val );
        swap( m );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified size, all entries set to num.
     */
    Int_matrix(int rows, int cols, Value_type num)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::get_initialized_int_matrix( &m_matrix, rows, cols, num ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix and initializes entries from an array.
     *
     * The data array must be of length at least (rows x cols).
     * The matrix is filled in row-major order.
     */
    Int_matrix(int rows, int cols, const Value_type* data);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Conversion ctor:  claim ownership of an existing int matrix
     *          pointer (i.e., make a shallow copy).
     *
     * This method is the proper way to say, ''Here is a kjb_c::Int_matrix
     * struct that I am responsible for deleting, and I must make sure that it
     * gets destroyed when it goes out of scope.''  This is a good way to wrap
     * a matrix "dynamically," after it has already been created.
     *
     * Anyplace you find yourself using free_int_matrix() in your C++ code, you
     * should consider using instead this class and this ctor.
     *
     * If the input pointer equals NULL then a zero size matrix is
     * constructed.
     *
     * @warning Do not create two Int_matrix objects from the same source this
     *          way or you will get a double deletion bug.
     */
    Int_matrix(Impl_type* mat_ptr)
        : m_matrix( mat_ptr )
    {
        if ( 0 == mat_ptr )
        {
            // Test program was HERE.
            ETX( kjb_c::get_target_int_matrix( &m_matrix, 0, 0 ) );
        }
        //else
        //{
        //    // Test program was HERE.
        //}
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor copies contents (i.e., a deep copy) of an existing
     *          C-struct matrix.
     * @warning This method should be seldom used:  kjb_c::Int_matrix objects
     *          should rarely be left in an unwrapped state.
     *
     * This kind of conversion is relatively expensive, thus we restrict its
     * use to explicit invocation.
     */
    explicit Int_matrix(const Impl_type& mat_ref)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::copy_int_matrix( &m_matrix, &mat_ref ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Read matrix from file
     * @see     kjb_c::read_matrix
     */
    Int_matrix(const std::string& file_name);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Copy ctor
     */
    Int_matrix(const Int_matrix& mat_ref)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::copy_int_matrix( &m_matrix, mat_ref.m_matrix ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    ~Int_matrix()
    {
        // Test program was HERE.
        kjb_c::free_int_matrix( m_matrix );
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    /**
     * @brief   Resize matrix, replace contents with the identity matrix.
     * @see     create_identity_matrix()
     *
     * The name of this method is misleading but I can't think of a better one.
     * It's not really an initialization routine, more like a re-init.
     */
    /* Int_matrix& init_identity( int rank )
    {
        // Test program was HERE.
        ETX( kjb_c::get_int_identity_matrix( &m_matrix, rank ) );
        return *this;
    } */

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    /**
     * @brief   Resize matrix, and clobber contents with zeroes.
     * @return  an lvalue of this object
     * @see     create_zero_matrix(int,int) -- more appropriate in some
     *          situations.
     */
    Int_matrix& zero_out( int num_rows, int num_cols )
    {
        // Test program was HERE.
        ETX( kjb_c::get_zero_int_matrix( &m_matrix, num_rows, num_cols ) );
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Resize matrix to be square, and clobber contents with zeroes.
     * @return  an lvalue of this object
     * @see     create_zero_matrix(int) -- more appropriate in some
     *          situations.
     */
    Int_matrix& zero_out( int rows )
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
    Int_matrix& zero_out()
    {
        // Test program was HERE.
        return zero_out( get_num_rows(), get_num_cols() );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief       Clone of zero_out(int,int)
     * @deprecated  The name is misleading; instead of this,
     *              use zero_out(int,int) in new code.
     */
    /* Int_matrix& init_zero(int num_rows, int num_cols)
    {
        // Test program was HERE.
        return zero_out( num_rows, num_cols );
    } */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    /* ------------------------------------------------------------------
     * ASSIGNMENT OPERATORS
     * ------------------------------------------------------------------ */

    /**
     * @brief   Assign contents from a kjb_c::Int_matrix, a C struct;
     *          make a deep copy.
     * @note    Consider using a shallow copy and swap() instead of
     *          assignment, to prevent unnecessary deep copying.
     *
     * Implementation note:  this routine formerly had a clause in it to
     * prevent calling copy_matrix() when mat_ref pointed to a zero-element
     * matrix.  This clause has been removed because it was later decided
     * that copy_matrix() should work even in that case.
     */
    Int_matrix& operator=(const Impl_type& mat_ref)
    {
        if ( m_matrix != &mat_ref )
        {
            // Test program was HERE.
            ETX( kjb_c::copy_int_matrix( &m_matrix, &mat_ref ) );
        }
        //else
        //{
        //    // Test program was HERE.
        //}
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Assign contents from a kjb::Int_matrix, a C++ object.
     * @note    Consider using swap() instead of assignment, to prevent
     *          unnecessary deep copying.
     */
    Int_matrix& operator=(const Int_matrix& src)
    {
        // Test program was HERE.
        // call assignment operator for kjb_c::Int_matrix
        return operator=( *src.m_matrix );
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Swap the representations of two matrices.
     */
    void swap( Int_matrix& other )
    {
        // Test program was HERE.
        std::swap( m_matrix, other.m_matrix );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return the number of rows in the matrix.
     */
    int get_num_rows() const
    {
        // Test program was HERE.
        return m_matrix -> num_rows;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return the number of columns in the matrix.
     */
    int get_num_cols() const
    {
        // Test program was HERE.
        return m_matrix -> num_cols;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return the number of elements in the matrix.
     * @warning There is a remote chance this could overflow an int.
     */
    int get_length() const // rows * cols
    {
        // Test program was HERE.
        return get_num_rows() * get_num_cols();
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Get const pointer to the underlying kjb_c::Int_matrix C struct.
     */
    const Impl_type* get_c_matrix() const
    {
        // Test program was HERE.
        return m_matrix;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Get pointer to the underlying kjb_c::Matrix C struct.
     * @warning:  This should only be used if you know what you're doing.  generally, it should only be used to write wrapper functions for c functions.
     */
    Impl_type* get_underlying_representation_with_guilt()
    {
        return m_matrix;
    }

    /** @brief Access a pointer to the underlying implementation,
     *         use with care */
    Value_type** ptr_to_storage_area() const
    {
        if(!m_matrix)
        {
            KJB_THROW_2(KJB_error,"Trying to access an integer matrix that has not been allocated");
        }
        return m_matrix->elements;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Resize this matrix, retaining previous values.
     */
    Int_matrix& resize(
        int new_rows,
        int new_cols,
        Value_type pad = Value_type(0)
    );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Resize this matrix, discarding previous values (faster than resize, because storage is reused)
     */
    Int_matrix& realloc(
        int new_rows,
        int new_cols)
    {
        ETX(kjb_c::get_target_int_matrix(&m_matrix, new_rows, new_cols));

        return *this;
    }
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Transpose this matrix, in-place.
     * @return  an lvalue to this matrix.
     */
    Int_matrix transpose()
    {
        // Test program was HERE.
        kjb_c::Int_matrix* result = 0;
        ETX(kjb_c::get_int_transpose( &result, m_matrix ) );
        return Int_matrix(result);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /* (Inverse is not supported for Int_matrix, although with "big integers"
     * one could make a Rational_matrix class, and perform potentially exact
     * inversion.  But would anyone use it?
     */

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Test whether the given row and column indices are valid.
     * @throws  Index_out_of_bounds exception, plus an explanatory message.
     *
     * Although this is a public method, users seldom will call it directly.
     * Its main purpose is to support the at() methods.
     */
    void check_bounds( int row, int col ) const
    {
        if(     row < 0 || get_num_rows() <= row
            ||  col < 0 || get_num_cols() <= col )
        {
            // Test program was HERE.
            throw_bad_bounds( row, col );
        }
        //else
        //{
        //  // Test program was HERE.
        //}
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Subscript matrix like a one-dimensional C array, e.g., A[10],
     *          using row-major ordering, and returning an lvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    Value_type& operator[](int i)
    {
        // Test program was HERE.
        return operator()( i );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Subscript matrix like a one-dimensional C array, e.g., A[10],
     *          using row-major ordering, and returning an rvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    Value_type operator[](int i) const
    {
        // Test program was HERE.
        return operator()( i );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   MATLAB-style one-dimensional subscript of matrix, e.g., A(10),
     *          using row-major ordering, and returning an lvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    Value_type& operator()(int i)
    {
        // Test program was HERE.
        int row, col;
        compute_row_col( i, &row, &col );
        return operator()( row, col );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   MATLAB-style one-dimensional subscript of matrix, e.g., A(10),
     *          using row-major ordering, and returning an rvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    Value_type operator()(int i) const
    {
        // Test program was HERE.
        int row, col;
        compute_row_col( i, &row, &col );
        return operator()( row, col );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix like a Fortran or MATLAB two-dimensional
     *          array, e.g., A(2,4), and return an lvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     *
     * The comparison to Fortran does not imply column-major ordering.
     */
    Value_type& operator()(int row, int col)
    {
        // Test program was HERE.
        return m_matrix -> elements[ row ][ col ];
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix like a Fortran or MATLAB two-dimensional
     *          array, e.g., A(2,4), and return an rvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     *
     * The comparison to Fortran does not imply column-major ordering.
     */
    Value_type operator()(int row, int col) const
    {
        // Test program was HERE.
        return m_matrix -> elements[ row ][ col ];
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix like a one-dimensional array,
     *          using row-major ordering, and returning an lvalue.
     */
    Value_type& at(int i)
    {
        // Test program was HERE.
        int row, col;
        compute_row_col_carefully( i, &row, &col );
        check_bounds( row, col );
        return operator()( row, col );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix like a one-dimensional array,
     *          using row-major ordering, and returning an rvalue.
     */
    Value_type at(int i) const
    {
        // Test program was HERE.
        int row, col;
        compute_row_col_carefully( i, &row, &col );
        check_bounds( row, col );
        return operator()( row, col );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix with bounds checking and return an lvalue.
     */
    Value_type& at(int row, int col)
    {
        // Test program was HERE.
        check_bounds( row, col );
        return operator()(row, col);
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix with bounds checking and return an rvalue.
     */
    Value_type at(int row, int col) const
    {
        // Test program was HERE.
        check_bounds( row, col );
        return operator()(row, col);
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    /* ------------------------------------------------------------------
     * EXTRACTION ROUTINES
     * ------------------------------------------------------------------ */

    /**
     * @brief   Return a specified row of this matrix, in the form of
     *          an Int_vector.
     */
    Vec_type get_row(int row) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return a specified column of this matrix, in the form of
     *          an Int_vector.
     */
    Vec_type get_col(int col) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief Replace a row of the matrix with the given vector
     */
    template <class Generic_vector>
    void set_row(int row, const Generic_vector& v);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief Dispatch for set_row.  Don't call directly
     */
    template <class Iterator>
    void set_row(int row, Iterator begin, Iterator end);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief Replace a column of the matrix with the given vector.
     * "vector" can be any collection of values convertible to int.
     */
    template <class Generic_vector>
    void set_col(int col, const Generic_vector& v);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief Dispatch for set_col.  Don't call directly
     */
    template <class Iterator>
    void set_col(int col, Iterator begin, Iterator end);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    /**
     * @brief   Writes the matrix contents to a file specified by name.
     * @see     kjb_c::write_int_matrix()
     *
     *
     * If filename is NULL or the first character is null, then the output
     * is written to standard output.
     */
    int write( const char* filename = 0 ) const
    {
        // Test program was HERE.
        return kjb_c::write_int_matrix( m_matrix, filename );
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /* ------------------------------------------------------------------
     * ARITHMETIC OPERATORS
     * Modifying arithmetic operators. That is, operators that modify
     * this matrix when applied. The non-modifying arithmetic operators
     * are non-member functions (and are below).
     * ------------------------------------------------------------------ */

    /**
     * @brief   Compute product of this matrix (on the left) and another, and
     *          replace this matrix with the result.
     * @throws  Dimension_mismatch if the number of columns of this matrix
     *          does not equal the number of rows of right factor op2.
     * @warning This is not really an in-place operation -- do not expect a
     *          speed-up using this operation instead of plain old star.
     */
    Int_matrix& operator*=(const Int_matrix& op2)
    {
        // Test program was HERE.
        ETX(multiply_int_matrices(&m_matrix, m_matrix, op2.m_matrix));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Compute product of this matrix (on the left) and another, and
     *          replace this matrix with the result, like *=.
     * @throws  Dimension_mismatch if the number of columns of this matrix
     *          does not equal the number of rows of right factor op2.
     * @warning This is not really an in-place operation -- do not expect a
     *          speed-up using this operation instead of plain old star.
     */
    Int_matrix& multiply(const Int_matrix& op2)
    {
        // Test program was HERE.
        return operator*=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Multiply this matrix by a scalar, in place.
     */
    Int_matrix& operator*= (Value_type op2)
    {
        // Test program was HERE.
        ETX(kjb_c::ow_multiply_int_matrix_by_int_scalar(m_matrix, op2));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Multiply this matrix by a scalar, in place.
     */
    Int_matrix& multiply(Value_type op2)
    {
        // Test program was HERE.
        return operator*=( op2 );
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Perform integer division on each entry in the matrix
     *          by a scalar value.
     * @return  an lvalue to this matrix
     * @warning This uses integer division, truncating fractions toward zero.
     * @throws  Divide_by_zero if the scalar value is zero.
     */
    Int_matrix& operator/= (Int_matrix::Value_type op2);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Perform integer division on each entry in the matrix
     *          by a scalar value.
     * @return  an lvalue to this matrix
     * @warning This uses integer division, truncating fractions toward zero.
     * @throws  Divide_by_zero if the scalar value is zero.
     */
    Int_matrix& divide(Int_matrix::Value_type op2)
    {
        // Test program was HERE.
        return operator/=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Add, in place, a matrix to this matrix.
     */
    Int_matrix& operator+= (const Int_matrix& op2)
    {
        // Test program was HERE.
        ETX(kjb_c::ow_add_int_matrices(m_matrix, op2.m_matrix));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Add, in place, a matrix to this matrix.
     */
    Int_matrix& add(const Int_matrix& op2)
    {
        // Test program was HERE.
        return operator+=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Subtract another matrix from this matrix, in place.
     */
    Int_matrix& operator-= (const Int_matrix& op2)
    {
        // Test program was HERE.
        ETX(kjb_c::ow_subtract_int_matrices(m_matrix, op2.m_matrix));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Subtract another matrix from this matrix, in place.
     */
    Int_matrix& subtract(const Int_matrix& op2)
    {
        // Test program was HERE.
        return operator-=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Transform this matrix into the additive inverse of its former
     *          value, in place.
     */
    Int_matrix& negate()
    {
        // Test program was HERE.
        return operator*=( Value_type(-1) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief Transform the elements of a matrix
     *
     * Apply a transformation fun to each element of the matrix, in place,
     * and return an lvalue of the modified matrix.
     */
    Mat_type& mapcar( Mapper );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */



private:
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        return kjb_serialize(ar, *this, version);
    }
};

/* ------------------------------------------------------------------
 * TEMPLATE MEMBERS  
 * ------------------------------------------------------------------ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

template <class Generic_vector>
void Int_matrix::set_row( int row, const Generic_vector& v )
{
    // Test program was HERE.
    using namespace boost;
    BOOST_CONCEPT_ASSERT((Container<Generic_vector>));

    typedef typename Generic_vector::value_type Vector_value_type;
    BOOST_CONCEPT_ASSERT((Convertible<Vector_value_type, Value_type>));

    set_row(row, v.begin(), v.end());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

template <class Iterator>
void Int_matrix::set_row( int row, Iterator begin, Iterator end )
{
    Iterator it = begin;
    for ( int col = 0; col < get_num_cols(); ++col )
    {
        if(it == end)
        {
            // range too short
            KJB_THROW(Dimension_mismatch);
        }

        operator()(row, col) = *it;
        it++;
    }

    if(it != end)
    {
        // range too long 
        KJB_THROW(Dimension_mismatch);
    }
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */


template <class Generic_vector>
void Int_matrix::set_col(int col, const Generic_vector& v)
{
    // Test program was HERE.
    using namespace boost;
    BOOST_CONCEPT_ASSERT((Container<Generic_vector>));

    typedef typename Generic_vector::value_type Vector_value_type;
    BOOST_CONCEPT_ASSERT((Convertible<Vector_value_type, Value_type>));

    set_col(col, v.begin(), v.end());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

template <class Iterator>
void Int_matrix::set_col(int col, Iterator begin, Iterator end)
{
    Iterator it = begin;
    for ( int row = 0; row < get_num_rows(); ++row )
    {
        if(it == end)
        {
            // range is too short
            KJB_THROW(Dimension_mismatch);
        }

        operator()(row, col) = *it;
        it++;
    }

    if(it != end)
    {
        // range is too short
        KJB_THROW(Dimension_mismatch);
    }

}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/* ------------------------------------------------------------------
 * "NAMED CONSTRUCTORS"
 * The "named constructor" idiom is when we use functions to create
 * objects.  The benefit is that the names indicate the purpose of the
 * method.  The following functions act as named constructors.
 * ------------------------------------------------------------------ */

/**
 * @brief   Construct an identity matrix of specified rank.
 */
inline
Int_matrix create_identity_int_matrix( int rank )
{
    // Test program was HERE.
    kjb_c::Int_matrix* result = 0;
    ETX( kjb_c::get_int_identity_matrix( &result, rank ) );
    return Int_matrix(result);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a zero matrix of specified size.
 */
inline
Int_matrix create_zero_int_matrix( int rows, int columns )
{
    // Test program was HERE.
    kjb_c::Int_matrix* result = 0;
    ETX( kjb_c::get_zero_int_matrix( &result, rows, columns ) );
    return Int_matrix(result);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a square zero matrix of specified size.
 */
inline
Int_matrix create_zero_int_matrix( int rows )
{
    // Test program was HERE.
    return create_zero_int_matrix( rows, rows );
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a one-row matrix by deep-copying a vector.
 */
Int_matrix create_diagonal_matrix( const Int_matrix::Vec_type& diagnoal );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a one-row matrix by deep-copying a vector.
 */
Int_matrix create_row_matrix( const Int_matrix::Vec_type& );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a one-column matrix by deep-copying a vector.
 */
Int_matrix create_column_matrix( const Int_matrix::Vec_type& );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/* ------------------------------------------------------------------
 * ARITHMETIC OPERATORS
 * Non-modifying arithmetic operators. That is, operators that do not
 * modify the operators when applied. The modifying arithmetic operators
 * are member functions.
 * ------------------------------------------------------------------ */

/**
 * @brief   Compute product of this matrix (on the left) and another.
 * @throws  Dimension_mismatch if the number of columns of this matrix
 *          does not equal the number of rows of right factor op2.
 */
inline
Int_matrix operator*(const Int_matrix& op1, const Int_matrix& op2)
{
    // Test program was HERE.
    return Int_matrix(op1) *= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute the product of this matrix and a scalar, yielding a new
 *          matrix.
 */
inline
Int_matrix operator* (const Int_matrix& op1, Int_matrix::Value_type op2)
{
    // Test program was HERE.
    return Int_matrix(op1) *= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute product of scalar and matrix, returning a new matrix.
 */
inline
Int_matrix operator*( Int_matrix::Value_type op1, const Int_matrix& op2 )
{
    // Test program was HERE.
    return op2 * op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute new matrix equal to lefthand matrix with each entry
 *          divided by a scalar value.
 * @throws  Divide_by_zero if the given value is zero.
 * @warning This uses integer division, truncating fractions toward zero.
 */
inline
Int_matrix operator/ (const Int_matrix& op1, Int_matrix::Value_type op2)
{
    // Test program was HERE.
    return Int_matrix(op1) /= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute matrix addition, returning a new matrix.
 */
inline
Int_matrix operator+ (const Int_matrix& op1, const Int_matrix& op2)
{
    // Test program was HERE.
    return Int_matrix(op1) += op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute matrix difference, returning a new matrix.
 */
inline
Int_matrix operator- (const Int_matrix& op1, const Int_matrix& op2)
{
    // Test program was HERE.
    return Int_matrix(op1) -= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Return the additive inverse of this matrix.
 */
inline
Int_matrix operator- (const Int_matrix& op1)
{
    // Test program was HERE.
    return op1 * (-1);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/* ------------------------------------------------------------------
 * COMPARISON OPERATORS
 * Comparison operators -- i.e., == and !=.
 * ------------------------------------------------------------------ */

/**
 * @brief   Test for exact equality between two matrices.
 * @warning Using floating point exact equality can be perilous.
 */
bool operator==(const Int_matrix& op1, const Int_matrix::Impl_type& op2);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Test for exact equality between two matrices.
 * @warning Using floating point exact equality can be perilous.
 */
inline
bool operator==(const Int_matrix& op1, const Int_matrix& op2)
{
    // Test program was HERE.
    return op1 == *op2.get_c_matrix();
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Test for any difference between two matrices.
 * @warning Using floating point exact equality can be perilous.
 */
inline
bool operator!=(const Int_matrix& op1, const Int_matrix::Impl_type& op2)
{
    // Test program was HERE.
    return !(op1 == op2);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Test for any difference between two matrices.
 * @warning Using floating point exact equality can be perilous.
 */
inline
bool operator!=(const Int_matrix& op1, const Int_matrix& op2)
{
    // Test program was HERE.
    return !(op1 == op2);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Test for exact equality between two matrices.
 * @warning Using floating point exact equality can be perilous.
 */
inline
bool operator==( const Int_matrix::Impl_type& op1, const Int_matrix& op2 )
{
    // Test program was HERE.
    return op2 == op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Test for any difference between two matrices.
 * @warning Using floating point exact equality can be perilous.
 */
inline
bool operator!=( const Int_matrix::Impl_type& op1, const Int_matrix& op2 )
{
    // Test program was HERE.
    return op2 != op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/* ------------------------------------------------------------------
 * OTHER STUFF
 * Other stuff =)
 * ------------------------------------------------------------------ */

/**
 * @brief   Test for any difference between two matrices.
 * @warning Using floating point exact equality can be perilous.
 */
inline
Int_matrix matrix_transpose( const Int_matrix& op1 )
{
    kjb_c::Int_matrix* kjb_matrix = 0;
    ETX( kjb_c::get_int_transpose( &kjb_matrix, op1.get_c_matrix() ) );
    return Int_matrix(kjb_matrix);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute the elementwise absolute value of a matrix.
 */
inline
Int_matrix abs(const Int_matrix& mat)
{
    // Test program was HERE.
    kjb_c::Int_matrix* result = 0;
    ETX( kjb_c::get_abs_of_int_matrix( &result, mat.get_c_matrix() ) );
    return Int_matrix(result);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Find the largest difference between two matrices.
 *
 * That is, this method compares two matrices of the same size,
 * computes the absolute values of the differences of corresponding
 * elements, and returns the maximum.
 *
 * @throws  Dimension_mismatch if they differ in sizes
 */
inline
Int_matrix::Value_type max_abs_difference( const Int_matrix& op1, const Int_matrix& op2 )
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
    return kjb_c::max_abs_int_matrix_difference( op1.get_c_matrix(), op2.get_c_matrix() );
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Return the minimum value in this matrix.
 */
inline
Int_matrix::Value_type min(const Int_matrix& mat)
{
    // Test program was HERE.
    return kjb_c::min_int_matrix_element(mat.get_c_matrix());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Return the maximum value in this matrix.
 */
inline
Int_matrix::Value_type max(const Int_matrix& mat)
{
    // Test program was HERE.
    return kjb_c::max_int_matrix_element(mat.get_c_matrix());
}



/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Display matrix contents in an ASCII format.
 *
 * This routine is mostly for debugging; consider one of the many
 * KJB output routines for more output in a more standardized form.
 */
std::ostream& operator<<(std::ostream& out, const Int_matrix& m);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief Compute the matrix's sum down (a.k.a. columnar sum), like MATLAB
 *
 * @return A vector of length m.get_num_cols() whose ith element is
 *          m(0,i) + m(1,i) + ... 
 *
 * This is a thin wrapper around kjb_c::sum_matrix_rows.
 */
Int_matrix::Vec_type sum_int_matrix_rows( const Int_matrix& m );
    
/** @} */

} // namespace kjb
#endif

