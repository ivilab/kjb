
/* $Id: n_det.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "n/n_det.h"
#include "n/n_diagonalize.h"
#include "n/n_invert.h"

#ifdef __cplusplus
extern "C" {
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                          get_log_determinant_of_PD_matrix
 *
 * Computes the log of the determinant of a positive-definite matrix.
 *
 * This routine finds the log of the determinant of a positive-definite matrix.
 * The matrix pointed to by A must be positive definite.
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: matrices
 *
 * -----------------------------------------------------------------------------
*/

int get_log_determinant_of_PD_matrix(const Matrix* A, double* det)
{
    Matrix* V = NULL;
    Matrix* V_inv = NULL;
    Vector* D = NULL;
    Matrix* logD = NULL;
    Matrix* temp_matrix = NULL;
    Matrix* logA = NULL;

    ERE(diagonalize_symmetric(A, &V, &D));
    ERE(get_matrix_inverse(&V_inv, V));

    ERE(ow_log_vector(D));
    get_diagonal_matrix(&logD, D);

    multiply_matrices(&temp_matrix, V, logD);
    multiply_matrices(&logA, temp_matrix, V_inv);

    get_matrix_trace(logA, det);

    free_matrix(V);
    free_matrix(V_inv);
    free_matrix(logD);
    free_matrix(temp_matrix);
    free_matrix(logA);
    free_vector(D);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

