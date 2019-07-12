
/* $Id: n_diagonalize.c 15481 2013-10-03 00:33:04Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
* =========================================================================== */

#include "n/n_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "wrap_lapack/wrap_lapack.h"
#include "n/n_diagonalize.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* =============================================================================
 *                              diagonalize
 *
 * Diagonalizes a matrix
 *
 * Given a matrix A, this routine computes A = E*D*inv(E), where D is diagonal,
 * and contains the eigenvalues of A, and the columns of E contain the
 * eigenvectors. If A is symmetric, then the eigenvalues are real. If A is not
 * symmetric, then the eigenvalues may, or may not be real. Currently, this
 * routine only returns the real parts of the eigenvalues, and prints a warning
 * message if any of the imaginary components of the eigenvalues exceed a
 * certain threshold. For many applications, the routines
 * diagonalize_symmetric(), and diagonalize_2() are prefered (see below).
 *
 * The matrix of eigenvectors is put into the matrix pointed to by *E_mpp,
 * which is created or resized as needed. The diagonal matrix of eigenvalues is
 * put into the vector pointed to by *D_vpp, which is also created or
 * resized as needed.
 *
 * Note:
 *     The underlying assumption of this routine is that the matrix to be
 *     diagonalized is NOT symmetric, BUT you are expecting essentially real
 *     eigenvectors (this was the case for the first application which led to
 *     this wrapper). If the matrix is symmetric, this routine will work, but
 *     diagonalize_symmetric() should be better. If the matrix is suspected of
 *     having complex eigenvectors/eigenvalues, then use the routine
 *     diagonalize_2() as this one will print a warning for each eigenvalue with
 *     absolute value of the imaginary part greater than 0.0001 of the maximum
 *     absolute value of all the eigenvalues.
 *
 * Note:
 *     This routine sorts the eigenvalues and eigenvectors from largest to
 *     smallest (opposite of Matlab). Also, the matrix of eigenvectors is
 *     ambigous up to a sign, so if you compare the results with other
 *     eigensolvers, you may have to multiply one of the results by -1.
 *
 * Note:
 *     This routine requires that that the LAPACK library is available. If this
 *     file was compiled without the presence of that library, then this routine
 *     will return ERROR.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: diagonalization, eigenvalues, matrix decomposition
 *
 * -----------------------------------------------------------------------------
*/

int diagonalize(const Matrix* mp, Matrix** E_mpp, Vector** D_vpp)
{

    if (mp == NULL) 
    {
        if (E_mpp != NULL)
        {
            free_matrix(*E_mpp);
            *E_mpp = NULL;
        }

        if (D_vpp != NULL)
        {
            free_vector(*D_vpp);
            *D_vpp = NULL;
        }

        return NO_ERROR; 
    }

    return lapack_diagonalize(mp, E_mpp, D_vpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              diagonalize_2
 *
 * Diagonalizes a matrix
 *
 * Given a matrix A, this routine computes A = E*D*inv(E), where D is diagonal,
 * and contains the eigenvalues of A, and the columns of E contain the
 * eigenvectors. If A is symmetric, then the eigenvalues are real. However, if
 * this is the case, then use the routine diagonalize_symmetric(). If
 * A is non-symmetric, but the eigenvalues are expected to be real regardless,
 * then use the routine diagonalize(). In general, this routine assumes that all
 * quantities may be complex.
 *
 * The matrix of eigenvectors is put into the matrix pointed to by *E_re_mpp and
 * *E_im_mpp which are created or resized as needed. The diagonal matrix of
 * eigenvalues is put into the vectors pointed to by *D_re_vpp and *D_im_vpp,
 * which are also created or resized as needed. If any of these quantities are
 * not needed, the arguments may be set to NULL.
 *
 * Note:
 *     Unlike diagonalize() and diagonalize_symmetric() this routine does NOT
 *     sort eigenvalues/eigenvectors.
 *
 * Note:
 *     Technically, we compute the "right" eigenvectors. If needed, this routine
 *     could be trivially modified to provide another diagonalization routine,
 *     diagonalize_3() which could provide, both "left" and "right"
 *     eigenvectors.
 *
 * Note:
 *     This routine requires that that the LAPACK library is available. If this
 *     file was compiled without the presence of that library, then this routine
 *     will return ERROR.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: diagonalization, eigenvalues, matrix decomposition
 *
 * -----------------------------------------------------------------------------
*/

int diagonalize_2
(
    const Matrix* mp,
    Matrix**      E_re_mpp,
    Matrix**      E_im_mpp,
    Vector**      D_re_vpp,
    Vector**      D_im_vpp
)
{
    return lapack_diagonalize_2(mp, E_re_mpp, E_im_mpp, D_re_vpp, D_im_vpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              diagonalize_symmetric
 *
 * Diagonalizes a symmetric matrix
 *
 * Given a symmetric matrix A, this routine computes A = E*D*inv(E), where D is
 * diagonal, and contains the eigenvalues of A, and the columns of E contain the
 * eigenvectors. Since A is symmetric, everything is real. If only the
 * eigenvalues are needed, E_mpp can be set to NULL.
 *
 * The matrix of eigenvectors is put into the matrix pointed to by *E_mpp,
 * which is created or resized as needed. The diagonal matrix of eigenvalues is
 * put into the vector pointed to by *D_vpp, which is also created or
 * resized as needed.
 *
 * For development code (compiled with -DTEST), this routine prints a warning if
 * the matrix is non symmetric (outside a threshold). For computing the
 * eigenvalues, only the upper triangular part of the matrix is used.
 *
 * Note:
 *     This routine sorts the eigenvalues and eigenvectors from largest to
 *     smallest (opposite of Matlab). Also, the matrix of eigenvectors is
 *     ambigous up to a sign, so if you compare the results with other
 *     eigensolvers, you may have to multiply one of the results by -1.
 *
 * Note:
 *     This routine requires that that the LAPACK library is available. If this
 *     file was compiled without the presence of that library, then this routine
 *     will return ERROR.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: diagonalization, eigenvalues, matrix decomposition
 *
 * -----------------------------------------------------------------------------
*/

int diagonalize_symmetric(const Matrix* mp, Matrix** E_mpp, Vector** D_vpp)
{
    return lapack_diagonalize_symmetric(mp, E_mpp, D_vpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

