
/* $Id: n_qr.c 17811 2014-10-22 05:22:59Z ksimek $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2010 by Kobus Barnard (author).
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
|  Author: Kyle Simek
* =========================================================================== */

#include "n/n_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "wrap_lapack/wrap_lapack.h"
#include "n/n_qr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* =============================================================================
 *                             qr_decompose 
 *
 * Decomposes a matrix into the product of an orthonormal matrix (Q) and an 
 * upper-triangular matrix (R).
 *
 * Given an m x n matrix A, this routine computes A = Q*R, where Q is an m x m
 * orthonormal matrix, and and R is an m x n upper-triangular matrix.  If 
 * m < n, R will actually be upper-trapezoidal; if m > n, rows m+1:n of R will
 * be zero.  
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
 * Index: matrix decomposition
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
*/

int qr_decompose(const Matrix* mp, Matrix** Q_mpp, Matrix** R_mpp)
{
   
    if (mp == NULL) 
    {
        if (Q_mpp != NULL)
        {
            free_matrix(*Q_mpp);
            *Q_mpp = NULL;
        }

        if (R_mpp != NULL)
        {
            free_matrix(*R_mpp);
            *R_mpp = NULL;
        }

        return NO_ERROR; 
    }


    if(Q_mpp == NULL)
    {
        get_target_matrix(Q_mpp, 0, 0);
    }

    if(R_mpp == NULL)
    {
        get_target_matrix(R_mpp, 0, 0);
    }
   

    return lapack_qr_decompose(mp, Q_mpp, R_mpp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             rq_decompose 
 *
 * Decompose a matrix M into the product of a right triangular matrix and an 
 * orthogonal matrix.
 *
 * Note: 
 *     The result is obtained directly from the result of qr_decompose(). 
 *     Let S be an antidiagonal matrix of all 1's, which 
 *     reverses the order of rows of columns of the matrix it multiplies (depending 
 *     on if it is left or right multiplied).  The dimension of S is implied from 
 *     the dimension of the other multiplicand.  Note that S * S = I and S = S'.
 *     As we see below, the RQ decomposition of M can by computed from the QR 
 *     decomposition of M' * S:
 *
 *|   M'* S = Q * R              QR decomposition
 *|   M' * S = Q * S * S * R     since (S * S) = I
 *|   S * M  = R' * S * S * Q'   transpose both sides
 *|   M = S * R' * S * S * Q'    algebra
 *|   M = (R_new) * (Q_new)      since (S * R' * S) is upper triangular
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
 * Index: matrix decomposition
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
 * */
int rq_decompose(const Matrix* mp, Matrix** R_mpp, Matrix** Q_mpp)
{
    Matrix* temp = NULL;
    Matrix* swap_N = NULL;
    Matrix* swap_M = NULL;

    int N, M, i;
   
    if (mp == NULL) 
    {
        if (Q_mpp != NULL)
        {
            free_matrix(*Q_mpp);
            *Q_mpp = NULL;
        }

        if (R_mpp != NULL)
        {
            free_matrix(*R_mpp);
            *R_mpp = NULL;
        }

        return NO_ERROR; 
    }

    N = mp->num_cols;
    M = mp->num_rows;


    ERE(get_zero_matrix(&swap_M, M, M));
    ERE(get_zero_matrix(&swap_N, N, N));

    for(i = 0; i < M; i++)
    {
        swap_M->elements[i][M - i -1] = 1;
    }

    for(i = 0; i < N; i++)
    {
        swap_N->elements[i][N - i -1] = 1;
    }

    ERE(get_matrix_transpose(&temp, mp));
    ERE(multiply_matrices(&temp, temp, swap_M));
    ERE(qr_decompose(temp, Q_mpp, R_mpp));

    ERE(get_matrix_transpose(Q_mpp, *Q_mpp));
    ERE(multiply_matrices(Q_mpp, swap_N, *Q_mpp));

    ERE(get_matrix_transpose(R_mpp, *R_mpp));
    ERE(multiply_matrices(R_mpp, *R_mpp, swap_N));
    ERE(multiply_matrices(R_mpp, swap_M, *R_mpp));


    free_matrix(temp);
    free_matrix(swap_M);
    free_matrix(swap_N);

    return NO_ERROR;
}


/* =============================================================================
 *                             ql_decompose 
 *
 * Decompose a matrix M into the product of an orthgonal matrix and a left-triangular matrix.
 *
 * Note: 
 *     The result is obtained directly from the result of rq_decompose(), using
 *     the fact that M' = (R * Q)' = Q' * R' = Q * L
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
 * Index: matrix decomposition
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
 * */
int ql_decompose(const Matrix* mp, Matrix** Q_mpp, Matrix** L_mpp)
{
    Matrix* temp = NULL;
   
    if (mp == NULL) 
    {
        if (Q_mpp != NULL)
        {
            free_matrix(*Q_mpp);
            *Q_mpp = NULL;
        }

        if (L_mpp != NULL)
        {
            free_matrix(*L_mpp);
            *L_mpp = NULL;
        }

        return NO_ERROR; 
    }

    ERE(get_matrix_transpose(&temp, mp));
    ERE(rq_decompose(temp, L_mpp, Q_mpp));
    ERE(get_matrix_transpose(L_mpp, *L_mpp));
    ERE(get_matrix_transpose(Q_mpp, *Q_mpp));

    free_matrix(temp);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             lq_decompose 
 *
 * Decompose a matrix M into the product of a left-triangular matrix L and an orthogonal matrix Q..  That is, M = L * Q.
 *
 * Note: 
 *     The result is obtained directly from the result of qr_decompose(), using
 *     the fact that M' = (Q * R)' = R' * Q' = L * Q
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
 * Index: matrix decomposition
 *
 * Author: Kyle Simek
 * -----------------------------------------------------------------------------
 * */
int lq_decompose(const Matrix* mp, Matrix** L_mpp, Matrix** Q_mpp)
{
    Matrix* temp = NULL;
   
    if (mp == NULL) 
    {
        if (Q_mpp != NULL)
        {
            free_matrix(*Q_mpp);
            *Q_mpp = NULL;
        }

        if (L_mpp != NULL)
        {
            free_matrix(*L_mpp);
            *L_mpp = NULL;
        }

        return NO_ERROR; 
    }

    ERE(get_matrix_transpose(&temp, mp));
    ERE(qr_decompose(temp, Q_mpp, L_mpp));
    ERE(get_matrix_transpose(L_mpp, *L_mpp));
    ERE(get_matrix_transpose(Q_mpp, *Q_mpp));

    free_matrix(temp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif
