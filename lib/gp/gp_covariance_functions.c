
/* $Id: gp_covariance_functions.c 14738 2013-06-13 23:37:46Z predoehl $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision Group     |
|  (the authors) including                                                     |
|        Kobus Barnard.                                                        |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|
* =========================================================================== */

#include "gp/gp_covariance_functions.h"
#include "l/l_incl.h"

#ifdef __cplusplus
extern "C" {
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      squared_exponential_covariance_function
 *
 * The Squared Exponential covariance function in multiple dimensions
 *
 * This routine is meant to be used with the fill_covariance_function. It is one
 * of the predefined covariance functions.
 *
 * For a given pair of indices t1 and t2, the matrix pointed to by cov will be
 *
 * |
 * |                              2
 * | exp(-0.5( ||t1 - t2|| / l_p )  )I
 *
 * where I is a d x d identity matrix. If the random variables of the Gaussian
 * process are vectors (i.e., if d > 1), then each component is independent of
 * the others.  The characteristic length l_p, a double, is specified by
 * passing in a const void pointer l that points to it,
 * i.e., l == (const void*) &l_p.
 *
 * If the matrix pointed to by cov is NULL, then a matrix of the appropriate
 * size is created. If it exists, but is the wrong size, then it is recreated.
 * Otherwise, the storage is recycled. 
 *
 * Returns:
 *    If the routine fails (due to storage allocation or an error in the
 * covariance function), then ERROR is returned with and error message being
 * set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int squared_exponential_covariance_function
(
    Matrix**        cov,
    const Vector*   t1,
    const Vector*   t2,
    const void*     l,
    int             d
)
{
    double dst_sq;
    int i;
    const double* l_p = (const double*)(l);

    if(t1 == NULL || t2 == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(t1->length != t2->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    get_zero_matrix(cov, d, d);
    dst_sq = vector_distance_sqrd(t1, t2);
    for(i = 0; i < d; i++)
    {
        (*cov)->elements[i][i] = exp((-1.0 / (2.0 * (*l_p) * (*l_p))) * dst_sq);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              zero_mean_function
 *
 * The zero mean function
 *
 * This routine is meant to be used with the fill_mean_vector. It is one of the
 * predefined mean functions.
 *
 * As the name suggests, the mn will be the d-dimensional 0-vector, regardless
 * of what the index t is.
 *
 * If the vector pointed to by mn is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation or an error in the covariance
 *    function), then ERROR is returned with and error message being set. Otherwise
 *    NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int zero_mean_function(Vector** mn, const Vector* t, int d)
{
    if(t == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    get_zero_vector(mn, d);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

