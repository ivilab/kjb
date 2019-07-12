/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2007-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kyle Simek, Andrew Predoehl, Ernesto Brau, Luca Del Pero           |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: m_matrix.h 20369 2016-02-14 18:17:27Z jguan1 $ */

#ifndef MATRIX_WRAP_H
#define MATRIX_WRAP_H

/**
 * @file
 * @author Kyle Simek
 * @author Andrew Predoehl
 * @author Ernesto Brau
 * @author Luca Del Pero
 *
 * @brief Definition for the Matrix class, a thin wrapper on the KJB
 *        Matrix struct and its related functionality.
 *
 * If you make changes to this file, PLEASE CONSIDER making parallel changes to
 * l_int_matrix.h,
 * whose structure closely parallels the structure of this file.
 * Tip:  use vimdiff on both files to show the parallel structure.
 *
 * Although this class has much the same interface as class Int_matrix, they
 * are not derived from a common abstract interface, because (1) we want as
 * much speed as possible -- this code should be eligible to put inside a
 * tight inner loop; and (2) I don't know whether that would be useful.
 */

#include "m/m_matrix.h"
#include "m/m_mat_basic.h"
#include "m/m_mat_io.h"
#include "m_cpp/m_serialization.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_concept.h"
#include "l_cpp/l_exception.h"

#include <iosfwd>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/serialization/access.hpp>
#endif

#ifdef TEST
#include <boost/concept_check.hpp>
#endif /*TEST */

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


namespace kjb {

/**
 * @defgroup kjbLinearAlgebra Linear Algebra
 *
 * This group tries to gather all the general-purpose matrix- and
 * vector-related functions and classes, including basic math and basic
 * utilities and manipulations.
 *
 * @{
 */

class Matrix;
class Int_matrix;
class Index_range;
template <class Matrix_type>
class Generic_matrix_view;

typedef Generic_matrix_view<Matrix> Matrix_view;
typedef const Generic_matrix_view<const Matrix> Const_matrix_view;

typedef Generic_vector_view<Matrix> Matrix_vector_view;
typedef const Generic_vector_view<const Matrix> Const_matrix_vector_view;

/**
 * @brief This class implements matrices, in the linear-algebra sense,
 *        with real-valued elements.
 *
 * For better maintainability, refer to the element type using this class's
 * Value_type typedef, instead of referring to 'double' directly.
 *
 * Most methods of this class are implemented in the C language portion of the
 * KJB library, with this class forming a thin (usually inlined) layer.
 */
class Matrix
{
#ifdef KJB_HAVE_BST_SERIAL
    friend class boost::serialization::access;
#endif
    friend bool operator<( const Matrix& op1, const Matrix& op2 );

public:

    /* Class users:  to make your code as maintanable as possible, use the
     * *_type typedefs below instead of directly referencing the specific
     * type.
     */

    typedef double          Value_type; ///< data type of the elements
    typedef Value_type     value_type; ///< data type of the elements
    typedef int             Size_type; ///< size type of the elements
    typedef kjb_c::Matrix   Impl_type;  ///< the underlying implementation
    typedef Matrix          Mat_type;   ///< the associated matrix type
    typedef Vector          Vec_type;   ///< the associated vector type
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
    )   const
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
    )   const
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
    Matrix()
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_matrix( &m_matrix, 0, 0 ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     */
    Matrix(int rows, int cols)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::get_target_matrix( &m_matrix, rows, cols ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     * @warning Native type for number of rows, columns is int, not unsigned.
     */
    Matrix(unsigned rows, unsigned cols)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        Matrix m( static_cast<int>( rows ), static_cast<int>( cols ) );
        swap( m );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     * @warning Native type for number of rows, columns is int, not unsigned.
     */
    Matrix(unsigned rows, unsigned cols, Value_type val)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        Matrix m( static_cast<int>( rows ), static_cast<int>( cols ), val );
        swap( m );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     * @warning Native type for number of rows, cols is int, not unsigned long.
     */
    Matrix(unsigned long rows, unsigned long cols)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        Matrix m( static_cast<int>( rows ), static_cast<int>( cols ) );
        swap( m );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified number of rows and columns.
     * @warning Native type for number of rows, cols is int, not unsigned long.
     */
    Matrix(unsigned long rows, unsigned long cols, Value_type val)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        Matrix m( static_cast<int>( rows ), static_cast<int>( cols ), val );
        swap( m );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix of specified size, all entries set to num.
     */
    Matrix(int rows, int cols, Value_type num)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::get_initialized_matrix( &m_matrix, rows, cols, num ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix and initializes entries from an array.
     *
     * The data array must be of length at least (rows x cols).
     * The matrix is filled in row-major order.
     */
    Matrix(int rows, int cols, const Value_type* data);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a single-column matrix from a vector.
     */
    Matrix ( const Vec_type& );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix from one with integer values.
     */
    Matrix ( const Int_matrix& );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix from a view of another matrix.
     */
    Matrix ( const Matrix_view& );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Ctor builds a matrix from a constant view of another matrix.
     */
    Matrix ( const Const_matrix_view& );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Conversion ctor:  claim ownership of an existing matrix
     *          pointer (i.e., make a shallow copy).
     *
     * This method is the proper way to say, ''Here is a kjb_c::Matrix
     * struct that I am responsible for deleting, and I must make sure that it
     * gets destroyed when it goes out of scope.''  This is a good way to wrap
     * a matrix "dynamically," after it has already been created.
     *
     * Anyplace you find yourself using free_matrix() in your C++ code, you
     * should consider using instead this class and this ctor.
     *
     * If the input pointer equals NULL then a zero size matrix is
     * constructed.
     *
     * This constructor is guaranteed not to throw whenever the input is
     * not equal to NULL.  This is an important promise for exception safety.
     *
     * @warning Do not create two Matrix objects from the same source this
     *          way or you will get a double deletion bug.
     */
    Matrix(Impl_type* mat_ptr)
        : m_matrix( mat_ptr )
    {
        if ( 0 == mat_ptr )
        {
            // Test program was HERE.
            ETX( kjb_c::get_target_matrix( &m_matrix, 0, 0 ) );
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
     * @warning This method should be seldom used:  kjb_c::Matrix objects
     *          should rarely be left in an unwrapped state.
     *
     * This kind of conversion is relatively expensive, thus we restrict its
     * use to explicit invocation.
     */
    explicit Matrix(const Impl_type& mat_ref)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::copy_matrix( &m_matrix, &mat_ref ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Read matrix from file
     * @see     kjb_c::read_matrix
     */
    Matrix(const std::string& file_name);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Copy ctor
     */
    Matrix(const Matrix& mat_ref)
        : m_matrix( 0 )
    {
        // Test program was HERE.
        ETX( kjb_c::copy_matrix( &m_matrix, mat_ref.m_matrix ) );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

#ifdef KJB_HAVE_CXX11
    /**
     * @brief   Move ctor
     */
    Matrix(Matrix&& mat_ref)
        : m_matrix( nullptr )
    {
        m_matrix = mat_ref.m_matrix;
        mat_ref.m_matrix = 0;
    }
#endif /* KJB_HAVE_CXX11 */

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    ~Matrix()
    {
        // Test program was HERE.
        kjb_c::free_matrix( m_matrix );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */



    /**
     * @brief   Resize matrix, replace contents with the identity matrix.
     * @see     create_identity_matrix()
     *
     * The name of this method is misleading but I can't think of a better one.
     * It's not really an initialization routine, more like a re-init.
     */
    /*Matrix& init_identity( int rank )
    {
        // Test program was HERE.
        ETX( kjb_c::get_identity_matrix( &m_matrix, rank ) );
        return *this;
    }*/

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    //Matrix& init_euler_zxz(float phi, float theta, float psi, bool homogeneous = false) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Resize matrix, and clobber contents with zeroes.
     * @return  an lvalue of this object
     * @see     create_zero_matrix(int,int) -- more appropriate in some
     *          situations.
     */
    Matrix& zero_out( int num_rows, int num_cols );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Resize matrix to be square, and clobber contents with zeroes.
     * @return  an lvalue of this object
     * @see     create_zero_matrix(int) -- more appropriate in some
     *          situations.
     */
    Matrix& zero_out( int rows );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Clobber contents of matrix with zeroes.
     * @return  an lvalue of this object
     * @see     create_zero_matrix() -- more appropriate in some situations.
     */
    Matrix& zero_out();

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Make sure all values in this matrix are between the two given
     *          values.
     * @return  an lvalue of this object
     */
    Matrix& limit_values(Value_type low, Value_type high);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief       Clone of square version of zero_out(int)
     * @deprecated  The name is misleading; instead of this,
     *              use zero_out(int) in new code.
     */
    /*Matrix& init_zero(int num_rows)
    {
        // Test program was HERE.
        return zero_out( num_rows );
    }*/


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    /* ------------------------------------------------------------------
     * ASSIGNMENT OPERATORS
     * ------------------------------------------------------------------ */

    /**
     * @brief   Assign contents from a kjb_c::Matrix, a C struct;
     *          make a deep copy.
     * @note    Consider using a shallow copy and swap() instead of
     *          assignment, to prevent unnecessary deep copying.
     *
     * Implementation note:  this routine formerly had a clause in it to
     * prevent calling copy_matrix() when mat_ref pointed to a zero-element
     * matrix.  This clause has been removed because it was later decided
     * that copy_matrix() should work even in that case.
     */
    Matrix& operator=(const Impl_type& mat_ref);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Assign contents from a kjb::Matrix, a C++ object.
     * @note    Consider using swap() instead of assignment, to prevent
     *          unnecessary deep copying.
     */
    Matrix& operator=(const Matrix& src)
    {
        // Test program was HERE.
        // call assignment operator for kjb_c::Matrix
        return operator=( *src.m_matrix );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

#ifdef KJB_HAVE_CXX11
    /**
     * @brief   Move assignment
     */
    Matrix& operator=(Matrix&& other)
    {
        if(this == &other)
            return *this;

        kjb_c::free_matrix( m_matrix );
        m_matrix = other.m_matrix;
        other.m_matrix = 0;
        return *this;
    }
#endif /* KJB_HAVE_CXX11 */


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Swap the representations of two matrices.
     */
    void swap( Matrix& other )
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
     * @brief   Return the number of elements in the matrix. (alias of get_length)
     * @note Included for compatibility with "vector-style" interface
     * @warning There is a remote chance this could overflow an int.
     */
    int size() const // rows * cols
    {
        // Test program was HERE.
        return get_length();
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Get pointer to the underlying kjb_c::Matrix C struct.
     * @warning:  This should only be used if you know what you're doing.  generally, it should only be used to write wrapper functions for c functions.
     */
    Impl_type*& get_underlying_representation_with_guilt()
    {
        return m_matrix;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Get const pointer to the underlying kjb_c::Matrix C struct.
     */
    const Impl_type* get_c_matrix() const
    {
        // Test program was HERE.
        return m_matrix;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Resize this matrix, retaining previous values.  Space is reused if possible.  Otherwise requires a new allocation and old space is freed.
     */
    Matrix& resize(int new_rows, int new_cols, Value_type pad = Value_type(0));

    /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

    /**
     * @brief resize this matrix, losing previous data.  Faster than resize(), because data copy is skipped.
     */
    Matrix& realloc(int new_rows, int new_cols)
    {
        ETX(kjb_c::get_target_matrix(&m_matrix, new_rows, new_cols));

        return *this;
    }

    /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

    /**
     * @brief allocate sufficient storage to hold the given dimensions, but leave actual matrix size unchanged.  Useful when calling Matrix::vertcat inside a tight loop, to avoid multiple reallocations
     */
    Matrix& reserve(int new_rows, int new_cols)
    {
        int old_rows = m_matrix->num_rows;
        int old_cols = m_matrix->num_cols;
        this->resize(
                std::max(new_rows, old_rows),
                std::max(new_cols, old_cols));
        this->resize(old_rows, old_cols);

        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Transpose this matrix
     * @return  an lvalue to this matrix.
     * @warning may disappear -- use matrix_transpose() instead
     */
    Matrix transpose() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Invert this matrix
     * @see     kjb_c::get_matrix_inverse()
     * @see     kjb_c::set_matrix_inversion_options()
     * @warning may disappear -- use matrix_inverse() instead
     */
    Matrix inverse() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Gets the absolute value of the determinant of this matrix.
     * @see     kjb_c::get_determinant_abs()
     */
    double abs_of_determinant() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Gets trace of this matrix.
     */
    double trace() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Retrieves the diagonal of this matrix; returned as a vector.
     * @see     kjb_c::get_determinant_abs()
     */
    Vec_type get_diagonal() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return the floor(3) of each element of the matrix
     * @warning You might want to check the max() and min(), to avoid overflow.
     */
    Int_matrix floor() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return the ceil(3) of each element of the matrix
     * @warning You might want to check the max() and min(), to avoid overflow.
     */
    Int_matrix ceil() const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Convert to a binary matrix; zero if value is below threshold, one if value is greater than or equal to threshold.
     */
    Int_matrix threshold(double t) const;

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
    const Value_type& operator[](int i) const
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
        return m_matrix->elements[0][i];
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   MATLAB-style one-dimensional subscript of matrix, e.g., A(10),
     *          using row-major ordering, and returning an rvalue.
     * @warning No bounds checking!
     * @see     at() method, which provides bounds checking.
     */
    const Value_type& operator()(int i) const
    {
        // Test program was HERE.
        return m_matrix->elements[0][i];
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
    const Value_type& operator()(int row, int col) const
    {
        // Test program was HERE.
        return m_matrix -> elements[ row ][ col ];
    }

    /*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

    /**
     * Access a submatrix using Matlab-like syntax.  Returned object has
     * Matrix semantics, but operates on the elements of this matrix indexed
     * by the rows and cols parameters.
     *
     * @note Accepted input types are int, Index_range.all, string, Int_vector,
     * or vector<int>
     * @warning Matrix_view is fairly lightweight, but constructing Matrix_view
     * and Index_range adds overhead compared to integer-based indexing.  In addition,
     * bounds checking occurs at EVERY index, so this shouldn't be used where
     * performance is a concern.
     */
    Matrix_view operator()(const Index_range& rows, const Index_range& cols);

    Const_matrix_view operator()(const Index_range& rows, const Index_range& cols) const;

    Matrix_vector_view operator[](const Index_range& i);

    Const_matrix_vector_view operator[](const Index_range& i) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix like a one-dimensional array,
     *          using row-major ordering, and returning an lvalue.
     */
    Value_type& at(int i);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix like a one-dimensional array,
     *          using row-major ordering, and returning an rvalue.
     */
    const Value_type& at(int i) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix with bounds checking and return an lvalue.
     */
    Value_type& at(int row, int col);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Access matrix with bounds checking and return an rvalue.
     */
    const Value_type& at(int row, int col) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Replace a submatrix of this matrix with the given matrix.
     */

    Matrix& replace(int row, int col, const Matrix& A);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Concat this matrix with another vertically.
     *          (named after the equivalent Matlab function)
     */
    Matrix& vertcat(const Matrix& A);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Concat this matrix with another horizontally. 
     *          (named after the equivalent Matlab function)
     *          Because of how memory is arranged, this is 
     *          generally slower than vertcat. 
     */
    Matrix& horzcat(const Matrix& A);

    /* ------------------------------------------------------------------
     * EXTRACTION ROUTINES
     * ------------------------------------------------------------------ */

    /**
     * @brief   Return a specified row of this matrix, in the form of
     *          a Vector.
     */
    Vec_type get_row(int row) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return a specified column of this matrix, in the form of
     *          a Vector.
     */
    Vec_type get_col(int col) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return all rows of this matrix into the provided iterator.
     */
    template<typename OutputIterator>
    OutputIterator get_all_rows(OutputIterator result) const;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Return all columns of this matrix into the provided iterator.
     */
    template<typename OutputIterator>
    OutputIterator get_all_cols(OutputIterator result) const;

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
     * "vector" can be any collection of values convertible to double.
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

    /*
     * @brief fill all elements of a row
     */
    void fill_row(int row, Value_type x);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /*
     * @brief fill all elements of a column
     */
    void fill_col(int col, Value_type x);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


    /**
     * @brief   Get the submatrix given by the parameters; i.e., get
     *          A(row, col, row + num_rows, col + num_cols), where A is
     *          this matrix.
     */

    Matrix submatrix(int row, int col, int num_rows, int num_cols) const
    {
        Matrix result;
        ETX(copy_matrix_block(&result.m_matrix, m_matrix, row, col, num_rows, num_cols));
        return result;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Reads the matrix contents from a file specified by name.
     * @see     kjb_c::read_matrix()
     *
     * This routine reads the matrix contents from the specified file and
     * overwrites the current contents.
     *
     * If filename is NULL or the first character is null, then the input
     * is read from standard input.
     */
    int read(const char* filename = 0)
    {
        // Test program was HERE.
        return kjb_c::read_matrix( &m_matrix, filename );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Writes the matrix contents to a file specified by name.
     * @see     kjb_c::write_matrix_full_precision()
     *
     * If filename is NULL or the first character is null, then the output
     * is written to standard output.
     */
    int write( const char* filename = 0 ) const
    {
        // Test program was HERE.
        return kjb_c::write_matrix_full_precision( m_matrix, filename );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Writes the matrix contents to a file specified by name.
     * @see     kjb_c::write_raw_matrix()
     *
     * If filename is NULL or the first character is null, then the output
     * is written to standard output.
     */
    int write_raw( const char* filename = 0 ) const
    {
        // Test program was HERE.
        return kjb_c::write_raw_matrix( m_matrix, filename );
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
    Matrix& operator*=(const Matrix& op2)
    {
        ETX(kjb_c::multiply_matrices(&m_matrix, m_matrix, op2.m_matrix));
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
    Matrix& multiply(const Matrix& op2)
    {
        // Test program was HERE.
        return operator*=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Multiply this matrix by a scalar, in place.
     */
    Matrix& operator*= (Value_type op2)
    {
        // Test program was HERE.
        ETX(kjb_c::ow_multiply_matrix_by_scalar(m_matrix, op2));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Multiply this matrix by a scalar, in place.
     */
    Matrix& multiply(Value_type op2)
    {
        // Test program was HERE.
        return operator*=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Divide each entry in the matrix
     *          by a scalar value.
     * @return  an lvalue to this matrix
     *
     * @throws  Divide_by_zero if the scalar value is zero.
     */
    Matrix& operator/= (Value_type op2)
    {
        if( 0 == op2 )
        {
            // Test program was HERE.
            KJB_THROW(Divide_by_zero);
        }
        //else
        //{
        //  // Test program was HERE.
        //}
        return operator*=( 1.0 / op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Divide each entry in the matrix
     *          by a scalar value.
     * @return  an lvalue to this matrix
     *
     * @throws  Divide_by_zero if the scalar value is zero.
     */
    Matrix& divide(Value_type op2)
    {
        // Test program was HERE.
        return operator/=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    
    /// @brief something luca will document eventually
    Matrix& scale_matrix_rows_by_sums()
    {
        ETX(kjb_c::ow_scale_matrix_rows_by_sums(m_matrix));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Add, in place, a matrix to this matrix.
     */
    Matrix& operator+= (const Matrix& op2)
    {
        // if this is omitted, dimension mismatch might not be caught
        // int ow_add_matrices, if dimensions are an integer
        // multiple of one another
        if( op2.get_num_rows() != get_num_rows() || 
                op2.get_num_cols() != get_num_cols())
            KJB_THROW( Dimension_mismatch );

        // Test program was HERE.
        ETX(kjb_c::ow_add_matrices(m_matrix, op2.m_matrix));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Subtract a scalar from all elements of this matrix, in place.
     */
    Matrix& operator+= (double x);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Add, in place, a matrix to this matrix.
     */
    Matrix& add(const Matrix& op2)
    {
        // Test program was HERE.
        return operator+=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Subtract another matrix from this matrix, in place.
     */
    Matrix& operator-= (const Matrix& op2)
    {
        // if this is omitted, dimension mismatch might not be caught
        // int ow_subtract_matrices, if dimensions are an integer
        // multiple of one another
        if( op2.get_num_rows() != get_num_rows() || 
                op2.get_num_cols() != get_num_cols())
            KJB_THROW( Dimension_mismatch );

        // Test program was HERE.
        ETX(kjb_c::ow_subtract_matrices(m_matrix, op2.m_matrix));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Shift each row by a vector, in place
     * @author Colin Dawson
     */
    Matrix& shift_rows_by(const Vector& v);
    
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    
    /**
     * @brief   Shift each column by a vector, in place
     * @author Colin Dawson
     */
    Matrix& shift_columns_by(const Vector& v);
    
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    
    /**
     * @brief   Elementwise multiply each row by a vector, in place
     * @author Colin Dawson
     */
    Matrix& ew_multiply_rows_by(const Vector& v);
    
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    
    /**
     * @brief   Elementwise multiply by another matrix
     * @author Colin Dawson
     */
    Matrix& ew_multiply_by(const Matrix& m);
    
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    
    /**
     * @brief  Remove a row of this matrix
     * @author Colin Dawson
     */
    Matrix& remove_row(int row_num)
    {
        ETX(kjb_c::remove_matrix_row(NULL, m_matrix, row_num));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    
    /**
     * @brief  Remove a column of this matrix
     * @author Colin Dawson
     */
    Matrix& remove_column(int col_num)
    {
        ETX(kjb_c::remove_matrix_col((kjb_c::Vector**) 0, m_matrix, col_num));
        return *this;
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    
    /**
     * @brief  Insert a column of zeroes into this matrix
     * @author Colin Dawson
     */
    Matrix& insert_zero_row(int row_num)
    {
        // resize(m_matrix->num_rows + 1, m_matrix->num_cols);
        ETX(kjb_c::insert_zero_row_in_matrix(m_matrix, row_num));
        return *this;
    }
    
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    
    /**
     * @brief  Insert a column of zeroes into this matrix
     * @author Colin Dawson
     */
    Matrix& insert_zero_column(int col_num)
    {
        // resize(m_matrix->num_rows, m_matrix->num_cols + 1);
        kjb_c::insert_zero_col_in_matrix(m_matrix, col_num);
        return *this;
    }

    
    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Subtract a scalar from all elements of this matrix, in place.
     */
    Matrix& operator-= (double x);

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Subtract another matrix from this matrix, in place.
     */
    Matrix& subtract(const Matrix& op2)
    {
        // Test program was HERE.
        return operator-=( op2 );
    }

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /**
     * @brief   Transform this matrix into the additive inverse of its former
     *          value, in place.
     */
    Matrix& negate()
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


    // GRAPHICS FUNCTIONS

    /** @brief Creates an euler rotation matrix without reallocating memory, when possible */
    void convert_to_euler_rotation_matrix
    (
        float      phi,
        float      theta,
        float      psi
    );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /** @brief Creates an euler rotation matrix and stores it into an instance of claa Matrix */
    static Matrix create_euler_rotation_matrix
    (
        float      phi,
        float      theta,
        float      psi
    );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_euler_homo_rotation_matrix
    (
        float      phi,
        float      theta,
        float      psi
    );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_euler_homo_rotation_matrix
    (
        float      phi,
        float      theta,
        float      psi
    );

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_rotation_matrix
    (
        double      phi,
        double      x,
        double      y,
        double      z
    ) ;


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

     void convert_to_2d_rotation_matrix
     (
         double          phi
     ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_rotation_matrix
    (
        double     phi,
        double     x,
        double     y,
        double     z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

      static Matrix create_2d_rotation_matrix
      (
          double     phi
      ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_rotation_matrix
    (
        double          phi,
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_rotation_matrix_from_vector
    (
        double          phi,
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_rotation_matrix
    (
        double          phi,
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_rotation_matrix_from_vector
    (
        double          phi,
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_rotation_matrix
    (
        double      phi,
        double      x,
        double      y,
        double      z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */
    void convert_to_2d_homo_rotation_matrix
    (
        double      phi
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_rotation_matrix
    (
        double      phi,
        double      x,
        double      y,
        double      z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

      static Matrix create_2d_homo_rotation_matrix
      (
          double     phi
      ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_rotation_matrix
    (
        double          phi,
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_rotation_matrix
    (
        double          phi,
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_rotation_matrix_from_vector
    (
        double          phi,
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_rotation_matrix_from_vector
    (
        double          phi,
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_scaling_matrix
    (
        double      x,
        double      y,
        double      z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_scaling_matrix
    (
        double     x,
        double     y,
        double     z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_scaling_matrix
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_scaling_matrix
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_scaling_matrix_from_vector
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_scaling_matrix_from_vector
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_scaling_matrix
    (
        double      x,
        double      y,
        double      z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_scaling_matrix
    (
        double     x,
        double     y,
        double     z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_scaling_matrix
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_scaling_matrix
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_scaling_matrix_from_vector
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_scaling_matrix_from_vector
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_translation_matrix
    (
        double      x,
        double      y,
        double      z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_translation_matrix
    (
        double     x,
        double     y,
        double     z
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_translation_matrix
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_translation_matrix
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    void convert_to_3d_homo_translation_matrix_from_vector
    (
        const Vector&   vec
    ) ;

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    static Matrix create_3d_homo_translation_matrix_from_vector
    (
        const Vector&   vec
    ) ;


    int display(const char* title = NULL) const;
    

    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    /// @deprecated use mapcar 
    Matrix map(Value_type (*f)(Value_type)) const
    {
        Matrix m(*this);
        return m.mapcar(f);
    }


    /* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

    Vec_type filter(bool (*f)(Value_type));
    

    /**
     * @brief call function f using every element of the matrix as an input
     * @param init Initial value
     */
    Value_type reduce(
        Value_type (*f)(Value_type x, Value_type y),
        Value_type init = 0.0
    )   const;


    /// @brief add a scalar value to each element of a matrix, in place
    /// @note should we add an operator+= for this? -kyle
    void ow_add_scalar(Value_type c);

    /// @brief add a row vector to each row of a matrix, in place
    void ow_add_row_vector(const Vec_type v);

    /// @brief add a column vector to each column of a matrix, in place
    void ow_add_col_vector(const Vec_type v);

    /// @brief multiply a column vector by each column of a matrix, elementwise, in place
    void ow_multiply_col_vector_ew(const Vec_type v);

    /// @brief multiply a row vector by each row of a matrix, elementwise, in place
    void ow_multiply_row_vector_ew(const Vec_type v);

    /// @brief flip the matrix vertically (swap last, first row, etc.)
    void ow_vertical_flip();

    /// @brief flip the matrix horizontally (swap left, right columns, etc.)
    void ow_horizontal_flip();

private:
    /// two constructors call this: Matrix(Matrix_view) and Matrix(Const_matrix_view).  Implementations are identical, thus the template implementation here.
    template <class View_type>
    void init_from_view_(const View_type& mat_view);

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        return kjb_serialize(ar, *this, version);
    }

    friend Value_type sum_elements(const Matrix& mat);
};


/* ------------------------------------------------------------------
 * TEMPLATE MEMBERS  
 * ------------------------------------------------------------------ */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

template<typename OutputIterator>
OutputIterator Matrix::get_all_rows(OutputIterator result) const
{
    for(int i = 0; i < get_num_rows(); i++)
    {
        *result++ = get_row(i);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

template<typename OutputIterator>
OutputIterator Matrix::get_all_cols(OutputIterator result) const
{
    for(int i = 0; i < get_num_cols(); i++)
    {
        *result++ = get_col(i);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

template <class Generic_vector>
void Matrix::set_row( int row, const Generic_vector& v )
{
    // Test program was HERE.
    using namespace boost;
#ifdef TEST
    BOOST_CONCEPT_ASSERT((kjb::SimpleVector<Generic_vector>));

    typedef typename Generic_vector::value_type Vector_value_type;
    BOOST_CONCEPT_ASSERT((Convertible<Vector_value_type, Value_type>));
#endif /*TEST  */

    set_row(row, v.begin(), v.end());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

template <class Iterator>
void Matrix::set_row( int row, Iterator begin, Iterator end )
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
void Matrix::set_col(int col, const Generic_vector& v)
{
    // Test program was HERE.
    using namespace boost;
#ifdef TEST
    BOOST_CONCEPT_ASSERT((kjb::SimpleVector<Generic_vector>));

    typedef typename Generic_vector::value_type Vector_value_type;
    BOOST_CONCEPT_ASSERT((Convertible<Vector_value_type, Value_type>));
#endif /*TEST  */

    set_col(col, v.begin(), v.end());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

template <class Iterator>
void Matrix::set_col(int col, Iterator begin, Iterator end)
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

template <class View_type>
void Matrix::init_from_view_(const View_type& mat_view)
{
    Matrix result(mat_view.get_num_rows(), mat_view.get_num_cols() );
    for( int row = 0; row < mat_view.get_num_rows(); ++row )
    {
        for( int col = 0; col < mat_view.get_num_cols(); ++col )
        {
            result( row, col ) = mat_view( row, col );
        }
    }
    swap( result );
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
Matrix create_identity_matrix( int rank )
{
    // Test program was HERE.
    kjb_c::Matrix* result = 0;
    ETX( kjb_c::get_identity_matrix( &result, rank ) );
    return Matrix(result);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a rank-deficient identity matrix with
 *          the specified number of rows and columns.
 */
inline
Matrix create_identity_matrix( int num_rows, int num_cols )
{
    Matrix M = create_identity_matrix(num_rows);
    M.resize(num_rows, num_cols, 0.0);
    return M;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a zero matrix of specified size.
 */
inline
Matrix create_zero_matrix( int rows, int columns )
{
    // Test program was HERE.
    kjb_c::Matrix* result = 0;
    ETX( kjb_c::get_zero_matrix( &result, rows, columns ) );
    return Matrix(result);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a square zero matrix of specified size.
 */
inline
Matrix create_zero_matrix( int size )
{
    // Test program was HERE.
    return create_zero_matrix( size, size );
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * Construct a matrix with uniform random values in [0,1].  
 * Numbers are generated from kjb_rand(), which is seeded by
 * kjb_seed_rand().
 *
 * @see kjb_rand, kjb_seed_rand
 */
inline
Matrix create_random_matrix( int num_rows, int num_cols )
{
    kjb_c::Matrix* c_mat = NULL;
    c_mat = kjb_c::create_random_matrix(num_rows, num_cols);
    return Matrix(c_mat);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * Construct a matrix with gaussian random values.
 * Numbers are generated from kjb_rand(), which is seeded by
 * kjb_seed_rand().
 *
 * @see gauss_rand, kjb_seed_rand
 */
Matrix create_gauss_random_matrix( int num_rows, int num_cols );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a diagonal matrix from the specified vector.
 */
Matrix create_diagonal_matrix(const Matrix::Vec_type& diagonal);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a "repeating" diagonal matrix from the specified vector.
 */
Matrix create_diagonal_matrix(const Matrix::Vec_type& diagonal, size_t n);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a diagonal matrix of the specified size, where
            the diagonal elements are set to the given value.
 */
inline
Matrix create_diagonal_matrix(int size, Matrix::Value_type a)
{
    kjb_c::Matrix* result = 0;
    kjb_c::get_identity_matrix(&result, size);
    kjb_c::ow_multiply_matrix_by_scalar(result, a);
    return Matrix(result);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a one-row matrix by deep-copying a vector.
 */
Matrix create_row_matrix( const Vector& );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a one-column matrix by deep-copying a vector.
 */
Matrix create_column_matrix( const Vector& );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/* ------------------------------------------------------------------
 * ARITHMETIC OPERATORS
 * Non-modifying arithmetic operators. That is, operators that do not
 * modify this matrix when applied. The modifying arithmetic operators
 * are member functions.
 * ------------------------------------------------------------------ */

/**
 * @brief   Compute product of this matrix (on the left) and another.
 * @throws  Dimension_mismatch if the number of columns of this matrix
 *          does not equal the number of rows of right factor op2.
 */
inline
Matrix operator*(const Matrix& op1, const Matrix& op2)
{
    // Test program was HERE.
    return Matrix(op1) *= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute the product of this matrix and a scalar, yielding a new
 *          matrix.
 */
inline
Matrix operator* (const Matrix& op1, Matrix::Value_type op2)
{
    // Test program was HERE.
    return Matrix(op1) *= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute product of scalar and matrix, returning a new matrix.
 */
inline
Matrix operator*( Matrix::Value_type op1, const Matrix& op2 )
{
    // Test program was HERE.
    return op2 * op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */


/**
 * @brief   Compute new matrix equal to lefthand matrix with each entry
 *          divided by a scalar value.
 * @throws  Divide_by_zero if the given value is zero.
 *
 */
inline
Matrix operator/ (const Matrix& op1, Matrix::Value_type op2)
{
    return Matrix(op1) /= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute matrix addition, returning a new matrix.
 */
inline
Matrix operator+ (const Matrix& op1, const Matrix& op2)
{
    // Test program was HERE.
    return Matrix(op1) += op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute matrix addition, returning a new matrix.
 */
inline
Matrix operator+ (const Matrix& op1, double op2)
{
    // Test program was HERE.
    return Matrix(op1) += op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute matrix difference, returning a new matrix.
 */
inline
Matrix operator- (const Matrix& op1, const Matrix& op2)
{
    // Test program was HERE.
    return Matrix(op1) -= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @author Colin Dawson
 * @brief   Add the same vector to each row, return a new Matrix
 */
inline
Matrix shift_rows_by(const Matrix& op1, const Vector& op2)
{
    // Test program was HERE.
    return Matrix(op1).shift_rows_by(op2);
}

/**
 * @author Colin Dawson
 * @brief   Add the same vector to each column, return a new Matrix
 */
inline
Matrix shift_columns_by(const Matrix& op1, const Vector& op2)
{
    // Test program was HERE.
    return Matrix(op1).shift_columns_by(op2);
}
    
/**
 * @brief   Elementwise multiply each row of op1 by a op2, return a new Matrix
 * @author Colin Dawson
 */
inline
Matrix ew_multiply_rows_by(const Matrix& op1, const Vector& op2)
{
    // Test program was HERE.
    return Matrix(op1).ew_multiply_rows_by(op2);
}

/**
 * @brief  Elementwise product of two matrices
 *
 * @author Colin Dawson
 */
inline Matrix ew_multiply(const Matrix& op1, const Matrix& op2)
{
    return Matrix(op1).ew_multiply_by(op2);
}
    
    
/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute matrix difference, returning a new matrix.
 */
inline
Matrix operator- (const Matrix& op1, double op2)
{
    // Test program was HERE.
    return Matrix(op1) -= op2;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Return the additive inverse of this matrix.
 */
inline
Matrix operator- (const Matrix& op1)
{
    // Test program was HERE.
    return op1 * (-1.0);
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
bool operator==(const Matrix& op1, const Matrix::Impl_type& op2);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Test for exact equality between two matrices.
 * @warning Using floating point exact equality can be perilous.
 */
inline
bool operator==(const Matrix& op1, const Matrix& op2)
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
bool operator!=(const Matrix& op1, const Matrix::Impl_type& op2)
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
bool operator!=(const Matrix& op1, const Matrix& op2)
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
bool operator==( const Matrix::Impl_type& op1, const Matrix& op2 )
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
bool operator!=( const Matrix::Impl_type& op1, const Matrix& op2 )
{
    // Test program was HERE.
    return op2 != op1;
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Returns true if a is lexicographically less-than b
 */
inline
bool operator<( const Matrix& op1, const Matrix& op2 )
{
    return std::lexicographical_compare(
            &op1.m_matrix->elements[0][0], 
            &op1.m_matrix->elements[0][0] + op1.get_length(),
            &op2.m_matrix->elements[0][0], 
            &op2.m_matrix->elements[0][0] + op1.get_length());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/* ------------------------------------------------------------------
 * OTHER STUFF
 * Other stuff =)
 * ------------------------------------------------------------------ */

/**
 * @brief   Invert this matrix.
 */
Matrix matrix_inverse( const Matrix& op1 );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Transpose this matrix.
 */
inline
Matrix matrix_transpose( const Matrix& op1 )
{
    kjb_c::Matrix* kjb_matrix = 0;
    ETX( kjb_c::get_matrix_transpose( &kjb_matrix, op1.get_c_matrix() ) );
    return Matrix(kjb_matrix);
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Compute the elementwise absolute value of a matrix.
 */
inline
Matrix abs(const Matrix& mat)
{
    // Test program was HERE.
    kjb_c::Matrix* result = 0;
    ETX( kjb_c::get_abs_of_matrix( &result, mat.get_c_matrix() ) );
    return Matrix(result);
}

/**
 * @brief   Compute the sum of squared elements.
 */
inline
double sum_squared_elements(const Matrix& mat)
{
    // Test program was HERE.
    kjb_c::Matrix* result = 0;
    ETX( kjb_c::multiply_matrices_ew( &result, mat.get_c_matrix(), mat.get_c_matrix() ) );
    double res = kjb_c::sum_matrix_elements(result);
    kjb_c::free_matrix(result);

    return res;
}

/**
 * @brief   Compute the sum of elements.
 */
inline
Matrix::Value_type sum_elements(const Matrix& mat)
{
    return kjb_c::sum_matrix_elements(mat.m_matrix);
}
    
/**
 * @brief   Compute the sum of squared elements.
 */
inline
Matrix get_squared_elements(const Matrix& mat)
{
    kjb_c::Matrix* result = 0;
    ETX( kjb_c::multiply_matrices_ew( &result, mat.get_c_matrix(), mat.get_c_matrix() ) );
    return Matrix(result);
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
Matrix::Value_type max_abs_difference( const Matrix& op1, const Matrix& op2 );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Return the minimum value in this matrix.
 */
inline
Matrix::Value_type min(const Matrix& mat)
{
    // Test program was HERE.
    return kjb_c::min_matrix_element(mat.get_c_matrix());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Return the maximum value in this matrix.
 */
inline
Matrix::Value_type max(const Matrix& mat)
{
    // Test program was HERE.
    return kjb_c::max_matrix_element(mat.get_c_matrix());
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Display matrix contents in an ASCII format.
 *
 * This routine is mostly for debugging; consider one of the many
 * KJB output routines for more output in a more standardized form.
 */
std::ostream& operator<<(std::ostream& out, const Matrix& m);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Read matrix to an output stream so it can be read with
 * stream_read_matrix
 *
 * If you aren't using boost serialization, this is the preferred way to
 * serialize a matrix for later reading with stream_read_matrix.  
 *
 * @warning In general, using operator<< is NOT recommended for 
 * serialization, because the matrix length is not written
 *
 * @author Jinyan Guan (This was adapted from stream_read_matrix)
 *
 */
std::istream& stream_read_matrix(std::istream& ist, Matrix& m);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Write matrix to an output stream so it can be read with
 * stream_read_matrix
 *
 * If you aren't using boost serialization, this is the preferred way to
 * serialize a matrix for later reading with stream_read_matrix.  
 *
 * @warning In general, using operator<< is NOT recommended for 
 * serialization, because the matrix length is not written
 *
 * @author Jinyan Guan (This was adapted from stream_write_matrix)
 *
 */
std::ostream& stream_write_matrix(std::ostream& ost, const Matrix& m);

/**
 * Compute the matrix determinant.
 *
 * @note This is not implemented for matrices larger than 3x3.  Even if it was,
 * it would probably be slow.  Prefer Matrix::abs_of_determinant() over this
 * if you don't care about the sign of the determinant
 */
double det(const Matrix& mat);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * Compute the outer-product (dyadic product) of two vectors.
 */
inline Matrix outer_product(const Vector& v1, const Vector& v2)
{
    // these are copied in as row vectors
    Matrix m1(v1);
    Matrix m2(v2);

    return m1 * m2.transpose();
}

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief Compute the matrix's sum across columns (a.k.a. row-wise sum)
 *
 * @return A vector of length m.get_num_rows() whose ith element is
 *          m(i,0) + m(i,1) + ...
 *
 * This is a thin wrapper around kjb_c::sum_matrix_cols.
 */
Matrix::Vec_type sum_matrix_cols( const Matrix& );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief Compute the matrix's sum down (a.k.a. columnar sum), like MATLAB
 *
 * @return A vector of length m.get_num_cols() whose ith element is
 *          m(0,i) + m(1,i) + ... 
 *
 * This is a thin wrapper around kjb_c::sum_matrix_rows.
 */
Matrix::Vec_type sum_matrix_rows( const Matrix& );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct an Int_matrix with elements equal to the floor() of a
 *          given floating-point Matrix.
 * @warning If you ought to check for overflow you must do it yourself.
 * @note This was moved from l_int_matrix.h to avoid a circular dependency.
 */
Int_matrix create_int_matrix_from_matrix_floor( const Matrix& );

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * alias for create_int_matrix_from_matrix_floor()
 */
Int_matrix floor( const Matrix& m);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 * @brief   Construct a matrix with elements equal to the ceiling, ceil()
 *          of a given floating-point Matrix.
 * @warning If you ought to check for overflow you must do it yourself.
 * @note This was moved from l_int_matrix.h to avoid a circular dependency.
 */
Int_matrix create_int_matrix_from_matrix_ceil( const Matrix& );

/**
 * @brief returns a matrix with 1s and 0s. 1 if either has a non zero value in that position, 
 * 0 otherwise
 *
 */
Matrix logical_or(Matrix a,Matrix b);

/**
 * @brief returns a matrix with 1s and 0s. 1 if both matricies have a non zero value in that position, 
 * 0 otherwise
 *
 */
Matrix logical_and(Matrix a,Matrix b);

/**
 * @brief returns a matrix with 1s and 0s. 1 if the value is 0, and 0 otherwise.
 *
 */
Matrix logical_not(Matrix a);

/**
 *@brief Construct a tiling of the given matrix 
 *@author Josh Bowdish
 *@note should be roughly the same as matlab's repmat()
 */
Matrix tile_matrix(Matrix trixy,unsigned m,unsigned n);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/**
 *@brief Construct a tiling of the given matrix, in this case, a matrix of the given number with the specified size 
 *@author Josh Bowdish
 *@note should be roughly the same as matlab's repmat()
 *
 * @deprecated
 * @warning do not use this, just use a constructor instead.
 *
 * Deprecated on 17 July 2013.  Remove in 6 months (after 16 January 2014).
 */
Matrix tile_matrix(int tehnum,unsigned m,unsigned n);

/* /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\  /\ */

/** @} */

/** @brief  Non-member swap function. */
inline void swap(Matrix& m1, Matrix& m2) { m1.swap(m2); }

} //namespace kjb

#endif

