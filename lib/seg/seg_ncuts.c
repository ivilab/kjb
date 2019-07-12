
/* $Id: seg_ncuts.c 15487 2013-10-03 22:04:16Z predoehl $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|        Prasad Gabbur.                                                        |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#include "seg/seg_ncuts.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              ncut_dense_bipartition
 *
 * Ncuts bi-partitioning of dense weight matrix.
 *
 * This routine implements the Normalized Cut partitioning technique for
 * splitting up a set of points into two sub-sets such that the normalized cut
 * between the two sub-sets is minimum. For details about the Normalized Cut
 * criterion for bipartitioning a set of points, please refer to the website:
 * http://www.cs.berkeley.edu/projects/vision/grouping. The input to this
 * routine is a square matrix containing the weights between each point-pair of
 * the set of points. It returns a soft approximation to the actual bipartition
 * that minimizes the normalized cut value of any bipartition.  Further
 * thresholding of the vector is needed to break up the set into two groups.
 *
 * Warning:
 *    The assumption is that the weight values between point pairs indicate
 *    affinity between them. The weight values are assumed to be positive. It is
 *    not guaranteed to work in a system where weight values may be negative.
 *    Also the weight matrix is assumed to be symmetric. If it is not, then it
 *    is forced to be symmetric by replicating the upper-right triangle into the
 *    lower-left triangle. This routine does not take advantage of sparseness of
 *    an input weight matrix (if it exists). It does brute force Normalized Cut
 *    approximation using the eigen solver from the LAPACK library. This
 *    routine is not adequately tested.
 *
 *    This routine results in memory leaks when fails to execute completely.
 *
 * Returns:
 *    On error this routine returns ERROR with an error message being set.
 *    On success it returns NO_ERROR.
 *
 * Index: segmentation
 *
 * -----------------------------------------------------------------------------
*/

int ncut_dense_bipartition(Vector** softpartition_vpp, const Matrix* weight_mp)
{

    Matrix* D_mp            = NULL;
    Matrix* D_invsqrt_mp    = NULL;
    Matrix* D_minus_W_mp    = NULL;
    Matrix* intermediate_mp = NULL;
    Matrix* product_mp      = NULL;
    Matrix* eigenvec_mp     = NULL;
    Vector* eigenval_vp     = NULL;
    Vector* Z_vp            = NULL;

    int N;
    int i, j;

    if ( weight_mp->num_rows != weight_mp->num_cols )
    {
        set_error("Ill-conditioned weight matrix: It is not square.");
        return ERROR;
    }

    N = weight_mp->num_rows;

    ERE(get_zero_matrix(&D_mp, N, N));
    ERE(get_zero_matrix(&D_invsqrt_mp, N, N));

    /*Forcing the weight matrix to be symmetric by copying the
    * elements from upper-right triangle to lower-left triangle.
    */
    for (i = 0; i < N; i++)
    {
        for (j = 0; j <= i ; j++)
        {
            weight_mp->elements[i][j] = weight_mp->elements[j][i];
        }
    }


    /*Generating the D matrix*/
    for (i = 0; i < N; i++)
    {
        D_mp->elements[i][i] = 0.0;

        for (j = 0; j < N; j++)
        {
            D_mp->elements[i][i] = D_mp->elements[i][i] + weight_mp->elements[i][j];
        }
    }


    /*Forming the D_invsqrt matrix*/
    for (i = 0; i < N; i++)
    {
        if ( D_mp->elements[i][i] != 0.0 )
        {
           D_invsqrt_mp->elements[i][i] = 1.0/(sqrt(D_mp->elements[i][i]));
        }
        else
        {
           set_error("Ill-conditioned weight matrix: At least one row of elements sums to zero.");
           add_error("Sum of %dth row = %f", i, D_mp->elements[i][i]);
           return ERROR;
        }
    }


    ERE(subtract_matrices(&D_minus_W_mp, D_mp, weight_mp));
    ERE(multiply_matrices(&intermediate_mp, D_minus_W_mp, D_invsqrt_mp));
    ERE(multiply_matrices(&product_mp, D_invsqrt_mp, intermediate_mp));

    ERE(diagonalize(product_mp, &eigenvec_mp, &eigenval_vp));

/*
    if ( softpartition_vpp != NULL )
    {
       free_vector(*softpartition_vpp);
    }

    ERE(get_target_vector(softpartition_vpp, N));

    for (i = 0; i < N; i++)
    {
        (*softpartition_vpp)->elements[i] = eigenvec_mp->elements[i][N-2];
    }
*/

    ERE(get_target_vector(&Z_vp, N));

    for (i = 0; i < N; i++)
    {
        Z_vp->elements[i] = eigenvec_mp->elements[i][N-2];
    }

    ERE(multiply_matrix_and_vector(softpartition_vpp, D_invsqrt_mp, Z_vp));

    free_matrix(D_mp);
    free_matrix(D_minus_W_mp);
    free_matrix(D_invsqrt_mp);
    free_matrix(intermediate_mp);
    free_matrix(product_mp);
    free_matrix(eigenvec_mp);
    free_vector(eigenval_vp);
    free_vector(Z_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

