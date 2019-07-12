/* $Id: n_svd.h 18278 2014-11-25 01:42:10Z ksimek $ */
/**
 * @file
 * @author Andrew Predoehl
 * @brief definition for class kjb::svd
 */

/* {{{======================================================================= *
 |
 |  Copyright (c) 2011 members of the University of Arizona Vision Group
 |
 | Personal and educational use of this code is granted, provided that this
 | header is kept intact, and that the authorship is not misrepresented, that
 | its use is acknowledged in publications, and relevant papers are cited.
 |
 | For other use contact the author (kobus AT cs DOT arizona DOT edu).
 |
 | Please note that the code in this file has not necessarily been adequately
 | tested. Naturally, there is no guarantee of performance, support, or
 | fitness for any particular task. Nonetheless, I am interested in hearing
 | about problems that you encounter.
 * ====================================================================== }}}*/


#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>

#ifndef KJB_CPP_N_SVD_H_UOFARIZONAVISION
#define KJB_CPP_N_SVD_H_UOFARIZONAVISION

namespace kjb {


/**
 * @brief a tuple that computes a singular value decomposition of a matrix
 * @see kjb_c::do_svd
 *
 * This class is just a convenience tuple of two matrices, a vector, and an int
 * that gathers the results of a singular value decomposition (SVD) of some
 * matrix.  For details about the SVD particulars, see the man page of function
 * kjb_c::do_svd(), which this class relies upon.
 *
 * The standard way to use this class is to construct it on a matrix that you
 * wish to decompose.  The ctor will launch the analysis, and then the object
 * will contain the results.  If you use it this way, you do NOT need to call
 * the "compute_svd()" method.
 *
 * The reconstruction() method will ideally return you the original matrix
 * from which the SVD was generated, modulo numerical noise.  In other words,
 * Your_input_matrix == mat_u * create_diagonal_matrix( vec_d ) * mat_vt
 * at least approximately.  Again, see the man page for kjb_c::do_svd() for
 * more details.
 */
struct Svd {
    Vector  vec_d;  ///< vector of singular values
    Matrix  mat_u,  ///< left factor in SVD product
            mat_vt; ///< eigenvectors (right factor in SVD product)
    int     rank;   ///< estimated rank of the input matrix

    /**
     * @brief wrap call to C library.
     *
     * Most of the time you do not need to call this method, because the
     * constructor calls it automatically.
     *
     * You only need to call this method if you want to recycle an Svd object.
     * That means you already have one of these objects, but you do not need
     * it anymore, and you want to reuse its current memory
     * allocation to store the SVD of a new matrix.  Then this method will save
     * you a few Matrix and Vector destruction/creation cycles.  (These sort of
     * time-savings are small, but inside a critical loop it could be worth
     * it.)
     */
    void compute_svd( const kjb_c::Matrix* pa );
    

    /// @brief overload of compute_svd( kjb_c::Matrix* ) for a wrapped Matrix.
    void compute_svd( const Matrix& mat_a );
    

    /// @brief ctor for tuple calls the library function on a wrapped Matrix
    Svd( const Matrix& mat_a );
    

    /// @brief ctor for tuple calls the library function on a C-style Matrix
    Svd( const kjb_c::Matrix* pa );

    /// @brief swap the representations of two SVD tuples
    void swap( Svd& other );
    

    /// @brief const getter for SVD left factor
    const Matrix& u() const;
    

    /// @brief const getter for vector of singular values
    const Vector& d() const;
    

    /// @brief const getter for SVD right factor
    const Matrix& vt() const;

    /// @brief this tries to reconstruct the original matrix A from its pieces
    Matrix reconstruction() const;
};


} // namespace kjb

#endif 
