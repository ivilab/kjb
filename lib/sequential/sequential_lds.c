
/* $Id: sequential_lds.c 10664 2011-09-29 19:51:58Z predoehl $ */

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

#include "l/l_incl.h"
#include "sequential/sequential_lds.h"
#include "n/n_invert.h"
#include "sample/sample_gauss.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          sample_from_LDS
 *
 * Creates a LDS sample
 *
 * This routine generates N samples from an LDS with the given parameters. In
 * other words, it creates vectors x_k and y_k, k=1,...,N, such that:
 *
 * x_1 = mu_0 + u,
 * x_k = A * x_{k-1} + w
 * y_k = H * z_k + v
 *
 * where w ~ N(0, Q) and v ~ N(0, R) and u ~ N(0, S_0), and the A
 * and Q are nxn matrices, the H is an mxn matrix and R is an mxm matrix, with
 * n being the dimension of the state variable.
 *
 * As usual, *x and *y are reused if possible and created if needed, according
 * to the KJB allocation semantics.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * Index: LDS
 *
 * -----------------------------------------------------------------------------
*/

int sample_from_LDS
(
    Vector_vector**      x,
    Vector_vector**      y,
    int                  N,
    const Matrix*        A,
    const Matrix*        Q,
    const Matrix*        H,
    const Matrix*        R,
    const Vector*        mu_0,
    const Matrix*        S_0
)
{
    Vector* x_0 = NULL;
    Vector* temp_vector = NULL;
    Vector* prev_x;
    int k;

    if(A == NULL || Q == NULL || H == NULL || R == NULL || mu_0 == NULL || S_0 == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    get_target_vector_vector(x, N);
    get_target_vector_vector(y, N);

    ERE(get_general_gauss_random_vector(&x_0, mu_0, S_0));

    for(k = 0; k < N; k++)
    {
        prev_x = (k == 0) ? x_0 : (*x)->elements[k - 1];

        ERE(multiply_matrix_and_vector(&temp_vector, A, prev_x));
        ERE(get_general_gauss_random_vector(&(*x)->elements[k], temp_vector, Q));
        ERE(multiply_matrix_and_vector(&temp_vector, H, (*x)->elements[k]));
        ERE(get_general_gauss_random_vector(&(*y)->elements[k], temp_vector, R));
    }

    free_vector(x_0);
    free_vector(temp_vector);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          sample_from_LDS_2
 *
 * Creates a LDS sample
 *
 * This routine generates N samples from an LDS with the given parameters. In
 * other words, it creates vectors x_k and y_k, k=1,...,N, such that:
 *
 * x_1 = mu_0 + u,
 * x_k = A_k * x_{k-1} + w
 * y_k = H_k * z_k + v
 *
 * where w ~ N(0, Q_k) and v ~ N(0, R_k) and u ~ N(0, S_0), and the A_k
 * and Q_k are nxn matrices, the H_k is an mxn matrix and R_k is an mxm matrix, with
 * n being the dimension of the state variable.
 *
 * As usual, *x and *y are reused if possible and created if needed, according
 * to the KJB allocation semantics.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * Index: LDS
 *
 * -----------------------------------------------------------------------------
*/

int sample_from_LDS_2
(
    Vector_vector**      x,
    Vector_vector**      y,
    int                  N,
    const Matrix_vector* A,
    const Matrix_vector* Q,
    const Matrix_vector* H,
    const Matrix_vector* R,
    const Vector*        mu_0,
    const Matrix*        S_0
)
{
    Vector* x_0 = NULL;
    Vector* temp_vector = NULL;
    Vector* prev_x;
    int k;

    if(A == NULL || Q == NULL || H == NULL || R == NULL || mu_0 == NULL || S_0 == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(A->length != N || Q->length != N || H->length != N || R->length != N)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    get_target_vector_vector(x, N);
    get_target_vector_vector(y, N);

    ERE(get_general_gauss_random_vector(&x_0, mu_0, S_0));

    for(k = 0; k < N; k++)
    {
        prev_x = (k == 0) ? x_0 : (*x)->elements[k - 1];

        ERE(multiply_matrix_and_vector(&temp_vector, A->elements[k], prev_x));
        ERE(get_general_gauss_random_vector(&(*x)->elements[k], temp_vector, Q->elements[k]));
        ERE(multiply_matrix_and_vector(&temp_vector, H->elements[k], (*x)->elements[k]));
        ERE(get_general_gauss_random_vector(&(*y)->elements[k], temp_vector, R->elements[k]));
    }

    free_vector(x_0);
    free_vector(temp_vector);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          compute_kalman_filter
 *
 * Calculates the marginal posteriors of an LDS
 *
 * This routine calculates the marginal posterior distributions for a linear
 * dynamical system model. Here, y is the set of observations and let x be
 * the set of (latent) state variables. This routine computes the distributions
 *
 *                      p(x_k | y_k, ..., y_k)
 *
 * for k = 1, ..., N, where the x_k are n-vectors and the y_k are m-vectors.
 * Since these distributions are normal, it suffices to compute
 * their means and covariances, which this routine puts in *means
 * and *covariances. The rest of the parameters are best explained by
 * seeing the equations of motion of the LDS:
 *
 *   x_1 = mu_0 + u,
 *   x_k = A * x_{k-1} + w
 *   y_k = H * z_k + v
 *
 * where w ~ N(0, Q) and v ~ N(0, R) and u ~ N(0, S_0), and the A
 * and Q are nxn matrices, the H is an mxn matrix and R is an mxm matrix.
 * Finally, this routine also computes the incomplete-data log-likelihood (LOG!)
 * of the LDS, i.e.,
 *
 *              log p(y_1, y_2, ..., y_N | A, H, Q, R, mu_0, S_0).
 *
 * As usual, *means and *covariances are reused if possible and created
 * if needed, according to the KJB allocation semantics. Any result that is
 * not desired can be omitted by passing NULL to the rouitne.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * Index: LDS
 *
 * -----------------------------------------------------------------------------
*/

int compute_kalman_filter
(
    Vector_vector**      means,
    Matrix_vector**      covariances,
    double*              likelihood,
    const Vector_vector* y,
    const Matrix*        A,
    const Matrix*        Q,
    const Matrix*        H,
    const Matrix*        R,
    const Vector*        mu_0,
    const Matrix*        S_0
)
{
    Vector_vector* mu = NULL;
    Vector_vector* mu_p = NULL;
    Matrix_vector* S = NULL;
    Matrix_vector* S_p = NULL;
    Matrix_vector* K = NULL;
    Matrix* temp_matrix_1 = NULL;
    Matrix* temp_matrix_2 = NULL;
    Matrix* temp_matrix_3 = NULL;
    Vector* temp_vector_1 = NULL;
    Vector* temp_vector_2 = NULL;
    Vector* lh_mu = NULL;
    Matrix* lh_sigma = NULL;
    double loc_lh;
    int k;
    int n, m;

    /********* Error checking and initialization and stuff ***********/

    if(means == NULL && covariances == NULL && likelihood == NULL)
    {
        return NO_ERROR;
    }

    if(y == NULL || A == NULL || Q == NULL || H == NULL || R == NULL || mu_0 == NULL || S_0 == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(!is_vector_vector_consistent(y))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* are square matrices square? */
    if(A->num_rows != A->num_cols || Q->num_rows != Q->num_cols || R->num_rows != R->num_cols || S_0->num_rows != S_0->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    n = A->num_rows;
    m = y->elements[0]->length;

    /* do the sizes match up? */
    if(H->num_rows != m || H->num_cols != n || Q->num_rows != n || R->num_rows != m || S_0->num_rows != n || mu_0->length != n)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(means != NULL)
    {
        get_target_vector_vector(means, y->length);
        mu = *means;
        get_target_vector_vector(&mu_p, y->length);
    }
    else
    {
        if(likelihood != NULL)
        {
            get_target_vector_vector(&mu, y->length);
            get_target_vector_vector(&mu_p, y->length);
        }
    }

    if(covariances == NULL)
    {
        get_target_matrix_vector(&S, y->length);
    }
    else
    {
        get_target_matrix_vector(covariances, y->length);
        S = *covariances;
    }

    get_target_matrix_vector(&S_p, y->length);
    get_target_matrix_vector(&K, y->length);

    /********* Start of algorithm ***********/

    *likelihood = 0.0;
    for(k = 0; k < y->length; k++)
    {
        /** Prediction stage **/
        /* compute {mu-prime}_k */
        if(mu != NULL)
        {
            if(k != 0)
            {
                multiply_matrix_and_vector(&mu_p->elements[k], A, mu->elements[k - 1]);
            }
            else
            {
                copy_vector(&mu_p->elements[k], mu_0);
            }
        }

        /* compute {sigma-prime}_k */
        if(k != 0)
        {
            multiply_by_transpose(&temp_matrix_1, S->elements[k - 1], A);
            multiply_matrices(&temp_matrix_2, A, temp_matrix_1);
            add_matrices(&S_p->elements[k], temp_matrix_2, Q);
        }
        else
        {
            copy_matrix(&S_p->elements[k], S_0);
        }

        /** Weighting stage **/
        /* compute K_k */
        multiply_by_transpose(&temp_matrix_1, S_p->elements[k], H);
        multiply_matrices(&temp_matrix_3, H, temp_matrix_1);
        ow_add_matrices(temp_matrix_3, R);
        ERE(get_matrix_inverse(&temp_matrix_1, temp_matrix_3));
        multiply_with_transpose(&temp_matrix_2, H, temp_matrix_1);
        multiply_matrices(&K->elements[k], S_p->elements[k], temp_matrix_2);

        /** Correction stage **/
        /* compute  mu_k */
        if(mu != NULL)
        {
            multiply_matrix_and_vector(&temp_vector_1, H, mu_p->elements[k]);
            subtract_vectors(&temp_vector_2, y->elements[k], temp_vector_1);
            multiply_matrix_and_vector(&temp_vector_1, K->elements[k], temp_vector_2);
            add_vectors(&mu->elements[k], mu_p->elements[k], temp_vector_1);
        }

        /* compute sigma_k */
        multiply_matrices(&temp_matrix_1, K->elements[k], H);
        get_identity_matrix(&temp_matrix_2, temp_matrix_1->num_rows);
        ow_subtract_matrices(temp_matrix_2, temp_matrix_1);
        multiply_matrices(&S->elements[k], temp_matrix_2, S_p->elements[k]);

        /* update likelihood */
        multiply_matrix_and_vector(&lh_mu, H, mu_p->elements[k]);
        copy_matrix(&lh_sigma, temp_matrix_3);
        get_log_density_gaussian(&loc_lh, y->elements[k], lh_mu, lh_sigma);
        /* get_log_density_gaussian(&loc_lh, lh_mu, lh_mu, lh_sigma); */
        *likelihood += loc_lh;
    }

    /********* Memory cleanup ***********/

    if(means != NULL)
    {
        free_vector_vector(mu_p);
    }
    else
    {
        if(likelihood != NULL)
        {
            free_vector_vector(mu);
            free_vector_vector(mu_p);
        }
    }

    if(covariances == NULL)
    {
        free_matrix_vector(S);
    }

    if(likelihood != NULL)
    {
        free_vector(lh_mu);
        free_matrix(lh_sigma);
    }

    free_matrix_vector(S_p);
    free_matrix_vector(K);
    free_matrix(temp_matrix_1);
    free_matrix(temp_matrix_2);
    free_matrix(temp_matrix_3);
    free_vector(temp_vector_1);
    free_vector(temp_vector_2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          compute_kalman_filter_stable
 *
 * Calculates the marginal posteriors of an LDS
 *
 * This routine calculates the marginal posterior distributions for a linear
 * dynamical system model. Here, y is the set of observations and let x be
 * the set of (latent) state variables. This routine computes the distributions
 *
 *                      p(x_k | y_k, ..., y_k)
 *
 * for k = 1, ..., N, where the x_k are n-vectors and the y_k are m-vectors.
 * Since these distributions are normal, it suffices to compute
 * their means and covariances, which this routine puts in *means
 * and *covariances. The rest of the parameters are best explained by
 * seeing the equations of motion of the LDS:
 *
 *   x_1 = mu_0 + u,
 *   x_k = A * x_{k-1} + w
 *   y_k = H * z_k + v
 *
 * where w ~ N(0, Q) and v ~ N(0, R) and u ~ N(0, S_0), and the A
 * and Q are nxn matrices, the H is an mxn matrix and R is an mxm matrix.
 * Finally, this routine also computes the incomplete-data log-likelihood (LOG!)
 * of the LDS, i.e.,
 *
 *              log p(y_1, y_2, ..., y_N | A, H, Q, R, mu_0, S_0).
 *
 * The difference between this and compute_kalman_filter is that the covariance
 * matrices in this routine are computed using a more numerically stable method.
 * Specifically, it computes S_k as
 *
 * S_k = (I - K_k*H)*S'_k*(I-K_k*H)^T + K_k*R*K_k^T,
 * 
 * where as compute_kalman_filter uses the more traditional
 *
 * S_k = (I-K_k*H)S'_k.
 *
 * As usual, *means and *covariances are reused if possible and created
 * if needed, according to the KJB allocation semantics. Any result that is
 * not desired can be omitted by passing NULL to the rouitne.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * Index: LDS
 *
 * -----------------------------------------------------------------------------
*/

int compute_kalman_filter_stable
(
    Vector_vector**      means,
    Matrix_vector**      covariances,
    double*              likelihood,
    const Vector_vector* y,
    const Matrix*        A,
    const Matrix*        Q,
    const Matrix*        H,
    const Matrix*        R,
    const Vector*        mu_0,
    const Matrix*        S_0
)
{
    Vector_vector* mu = NULL;
    Vector_vector* mu_p = NULL;
    Matrix_vector* S = NULL;
    Matrix_vector* S_p = NULL;
    Matrix_vector* K = NULL;
    Matrix* temp_matrix_1 = NULL;
    Matrix* temp_matrix_2 = NULL;
    Matrix* temp_matrix_3 = NULL;
    Vector* temp_vector_1 = NULL;
    Vector* temp_vector_2 = NULL;
    Vector* lh_mu = NULL;
    Matrix* lh_sigma = NULL;
    double loc_lh;
    int k;
    int n, m;

    /********* Error checking and initialization and stuff ***********/

    if(means == NULL && covariances == NULL && likelihood == NULL)
    {
        return NO_ERROR;
    }

    if(y == NULL || A == NULL || Q == NULL || H == NULL || R == NULL || mu_0 == NULL || S_0 == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(!is_vector_vector_consistent(y))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* are square matrices square? */
    if(A->num_rows != A->num_cols || Q->num_rows != Q->num_cols || R->num_rows != R->num_cols || S_0->num_rows != S_0->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    n = A->num_rows;
    m = y->elements[0]->length;

    /* do the sizes match up? */
    if(H->num_rows != m || H->num_cols != n || Q->num_rows != n || R->num_rows != m || S_0->num_rows != n || mu_0->length != n)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(means != NULL)
    {
        get_target_vector_vector(means, y->length);
        mu = *means;
        get_target_vector_vector(&mu_p, y->length);
    }
    else
    {
        if(likelihood != NULL)
        {
            get_target_vector_vector(&mu, y->length);
            get_target_vector_vector(&mu_p, y->length);
        }
    }

    if(covariances == NULL)
    {
        get_target_matrix_vector(&S, y->length);
    }
    else
    {
        get_target_matrix_vector(covariances, y->length);
        S = *covariances;
    }

    get_target_matrix_vector(&S_p, y->length);
    get_target_matrix_vector(&K, y->length);

    /********* Start of algorithm ***********/

    *likelihood = 0.0;
    for(k = 0; k < y->length; k++)
    {
        /** Prediction stage **/
        /* compute {mu-prime}_k */
        if(mu != NULL)
        {
            if(k != 0)
            {
                multiply_matrix_and_vector(&mu_p->elements[k], A, mu->elements[k - 1]);
            }
            else
            {
                copy_vector(&mu_p->elements[k], mu_0);
            }
        }

        /* compute {sigma-prime}_k */
        if(k != 0)
        {
            multiply_by_transpose(&temp_matrix_1, S->elements[k - 1], A);
            multiply_matrices(&temp_matrix_2, A, temp_matrix_1);
            add_matrices(&S_p->elements[k], temp_matrix_2, Q);
        }
        else
        {
            copy_matrix(&S_p->elements[k], S_0);
        }

        /** Weighting stage **/
        /* compute K_k */
        multiply_by_transpose(&temp_matrix_1, S_p->elements[k], H);
        multiply_matrices(&temp_matrix_3, H, temp_matrix_1);
        ow_add_matrices(temp_matrix_3, R);
        ERE(get_matrix_inverse(&temp_matrix_1, temp_matrix_3));
        multiply_with_transpose(&temp_matrix_2, H, temp_matrix_1);
        multiply_matrices(&K->elements[k], S_p->elements[k], temp_matrix_2);

        /** Correction stage **/
        /* compute  mu_k */
        if(mu != NULL)
        {
            multiply_matrix_and_vector(&temp_vector_1, H, mu_p->elements[k]);
            subtract_vectors(&temp_vector_2, y->elements[k], temp_vector_1);
            multiply_matrix_and_vector(&temp_vector_1, K->elements[k], temp_vector_2);
            add_vectors(&mu->elements[k], mu_p->elements[k], temp_vector_1);
        }

        /* compute sigma_k */
        multiply_matrices(&temp_matrix_1, K->elements[k], H);
        get_identity_matrix(&temp_matrix_2, temp_matrix_1->num_rows);
        ow_subtract_matrices(temp_matrix_2, temp_matrix_1);
        multiply_matrices(&temp_matrix_1, temp_matrix_2, S_p->elements[k]);
        multiply_by_transpose(&S->elements[k], temp_matrix_1, temp_matrix_2);
        multiply_by_transpose(&temp_matrix_1, R, K->elements[k]);
        multiply_matrices(&temp_matrix_2, K->elements[k], temp_matrix_1);
        ow_add_matrices(S->elements[k], temp_matrix_2);

        /* update likelihood */
        multiply_matrix_and_vector(&lh_mu, H, mu_p->elements[k]);
        copy_matrix(&lh_sigma, temp_matrix_3);
        get_log_density_gaussian(&loc_lh, y->elements[k], lh_mu, lh_sigma);
        /* get_log_density_gaussian(&loc_lh, lh_mu, lh_mu, lh_sigma); */
        *likelihood += loc_lh;
    }

    /********* Memory cleanup ***********/

    if(means != NULL)
    {
        free_vector_vector(mu_p);
    }
    else
    {
        if(likelihood != NULL)
        {
            free_vector_vector(mu);
            free_vector_vector(mu_p);
        }
    }

    if(covariances == NULL)
    {
        free_matrix_vector(S);
    }

    if(likelihood != NULL)
    {
        free_vector(lh_mu);
        free_matrix(lh_sigma);
    }

    free_matrix_vector(S_p);
    free_matrix_vector(K);
    free_matrix(temp_matrix_1);
    free_matrix(temp_matrix_2);
    free_matrix(temp_matrix_3);
    free_vector(temp_vector_1);
    free_vector(temp_vector_2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          compute_kalman_filter_2
 *
 * Calculates the marginal posteriors of an LDS
 *
 * This routine calculates the marginal posterior distributions for a linear
 * dynamical system model. Here, y is the set of observations and let x be
 * the set of (latent) state variables. This routine computes the distributions
 *
 *                      p(x_k | y_k, ..., y_k)
 *
 * for k = 1, ..., N, where the x_k are n-vectors and the y_k are m-vectors.
 * Since these distributions are normal, it suffices to compute
 * their means and covariances, which this routine puts in *means
 * and *covariances. The rest of the parameters are best explained by
 * seeing the equations of motion of the LDS:
 *
 *   x_1 = mu_0 + u,
 *   x_k = A_k * x_{k-1} + w_k
 *   y_k = H_k * z_i + v_k
 *
 * where w_k ~ N(0, Q_k) and v_k ~ N(0, R_k) and u ~ N(0, S_0), and the A_k
 * and Q_k are nxn matrices, the H_k is an mxn matrix and R_k is an mxm matrix.
 * Finally, this routine also computes the incomplete-data log-likelihood (LOG!)
 * of the LDS, i.e.,
 *
 *              log p(y_1, y_2, ..., y_N | A, H, Q, R, mu_0, S_0).
 *
 * As usual, *means and *covariances are reused if possible and created
 * if needed, according to the KJB allocation semantics. Any result that is
 * not desired can be omitted by passing NULL to the rouitne.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * Index: LDS
 *
 * -----------------------------------------------------------------------------
*/

int compute_kalman_filter_2
(
    Vector_vector**      means,
    Matrix_vector**      covariances,
    double*              likelihood,
    const Vector_vector* y,
    const Matrix_vector* A,
    const Matrix_vector* Q,
    const Matrix_vector* H,
    const Matrix_vector* R,
    const Vector*        mu_0,
    const Matrix*        S_0
)
{
    Vector_vector* mu = NULL;
    Vector_vector* mu_p = NULL;
    Matrix_vector* S = NULL;
    Matrix_vector* S_p = NULL;
    Matrix_vector* K = NULL;
    Matrix* temp_matrix_1 = NULL;
    Matrix* temp_matrix_2 = NULL;
    Matrix* temp_matrix_3 = NULL;
    Vector* temp_vector_1 = NULL;
    Vector* temp_vector_2 = NULL;
    Vector* lh_mu = NULL;
    Matrix* lh_sigma = NULL;
    double loc_lh;
    int k;
    int n, m;

    /********* Error checking and initialization and stuff ***********/

    if(means == NULL && covariances == NULL && likelihood == NULL)
    {
        return NO_ERROR;
    }

    if(y == NULL || A == NULL || Q == NULL || H == NULL || R == NULL || mu_0 == NULL || S_0 == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* are there enough parameters? */
    if(A->length < y->length || Q->length < y->length || H->length < y->length || R->length < y->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* are all corresponding matrices and vectors the same size? */
    if(!is_matrix_vector_consistent(A) || !is_matrix_vector_consistent(Q) || !is_matrix_vector_consistent(H) || !is_matrix_vector_consistent(R) || !is_vector_vector_consistent(y))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* are square matrices square? */
    if(A->elements[0]->num_rows != A->elements[0]->num_cols || Q->elements[0]->num_rows != Q->elements[0]->num_cols || R->elements[0]->num_rows != R->elements[0]->num_cols || S_0->num_rows != S_0->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    n = A->elements[0]->num_rows;
    m = y->elements[0]->length;

    /* do the sizes match up? */
    if(H->elements[0]->num_rows != m || H->elements[0]->num_cols != n || Q->elements[0]->num_rows != n || R->elements[0]->num_rows != m || S_0->num_rows != n || mu_0->length != n)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(means != NULL)
    {
        get_target_vector_vector(means, y->length);
        mu = *means;
        get_target_vector_vector(&mu_p, y->length);
    }
    else
    {
        if(likelihood != NULL)
        {
            get_target_vector_vector(&mu, y->length);
            get_target_vector_vector(&mu_p, y->length);
        }
    }

    if(covariances == NULL)
    {
        get_target_matrix_vector(&S, y->length);
    }
    else
    {
        get_target_matrix_vector(covariances, y->length);
        S = *covariances;
    }

    get_target_matrix_vector(&S_p, y->length);
    get_target_matrix_vector(&K, y->length);

    /********* Start of algorithm ***********/

    *likelihood = 0.0;
    for(k = 0; k < y->length; k++)
    {
        /** Prediction stage **/
        /* compute {mu-prime}_k */
        if(mu != NULL)
        {
            if(k != 0)
            {
                multiply_matrix_and_vector(&mu_p->elements[k], A->elements[k - 1], mu->elements[k - 1]);
            }
            else
            {
                copy_vector(&mu_p->elements[k], mu_0);
            }
        }

        /* compute {sigma-prime}_k */
        if(k != 0)
        {
            multiply_by_transpose(&temp_matrix_1, S->elements[k - 1], A->elements[k - 1]);
            multiply_matrices(&temp_matrix_2, A->elements[k - 1], temp_matrix_1);
            add_matrices(&S_p->elements[k], temp_matrix_2, Q->elements[k - 1]);
        }
        else
        {
            copy_matrix(&S_p->elements[k], S_0);
        }

        /** Weighting stage **/
        /* compute K_k */
        multiply_by_transpose(&temp_matrix_1, S_p->elements[k], H->elements[k]);
        multiply_matrices(&temp_matrix_3, H->elements[k], temp_matrix_1);
        ow_add_matrices(temp_matrix_3, R->elements[k]);
        ERE(get_matrix_inverse(&temp_matrix_1, temp_matrix_3));
        multiply_with_transpose(&temp_matrix_2, H->elements[k], temp_matrix_1);
        multiply_matrices(&K->elements[k], S_p->elements[k], temp_matrix_2);

        /** Correction stage **/
        /* compute  mu_k */
        if(mu != NULL)
        {
            multiply_matrix_and_vector(&temp_vector_1, H->elements[k], mu_p->elements[k]);
            subtract_vectors(&temp_vector_2, y->elements[k], temp_vector_1);
            multiply_matrix_and_vector(&temp_vector_1, K->elements[k], temp_vector_2);
            add_vectors(&mu->elements[k], mu_p->elements[k], temp_vector_1);
        }

        /* compute sigma_k */
        multiply_matrices(&temp_matrix_1, K->elements[k], H->elements[k]);
        get_identity_matrix(&temp_matrix_2, temp_matrix_1->num_rows);
        ow_subtract_matrices(temp_matrix_2, temp_matrix_1);
        multiply_matrices(&S->elements[k], temp_matrix_2, S_p->elements[k]);

        /* update likelihood */
        multiply_matrix_and_vector(&lh_mu, H->elements[k], mu_p->elements[k]);
        copy_matrix(&lh_sigma, temp_matrix_3);
        get_log_density_gaussian(&loc_lh, y->elements[k], lh_mu, lh_sigma);
        /* get_log_density_gaussian(&loc_lh, lh_mu, lh_mu, lh_sigma); */
        *likelihood += loc_lh;
    }

    /********* Memory cleanup ***********/

    if(means != NULL)
    {
        free_vector_vector(mu_p);
    }
    else
    {
        if(likelihood != NULL)
        {
            free_vector_vector(mu);
            free_vector_vector(mu_p);
        }
    }

    if(covariances == NULL)
    {
        free_matrix_vector(S);
    }

    if(likelihood != NULL)
    {
        free_vector(lh_mu);
        free_matrix(lh_sigma);
    }

    free_matrix_vector(S_p);
    free_matrix_vector(K);
    free_matrix(temp_matrix_1);
    free_matrix(temp_matrix_2);
    free_matrix(temp_matrix_3);
    free_vector(temp_vector_1);
    free_vector(temp_vector_2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      compute_kalman_filter_2_stable
 *
 * Calculates the marginal posteriors of an LDS
 *
 * This routine calculates the marginal posterior distributions for a linear
 * dynamical system model. Here, y is the set of observations and let x be
 * the set of (latent) state variables. This routine computes the distributions
 *
 *                      p(x_k | y_k, ..., y_k)
 *
 * for k = 1, ..., N, where the x_k are n-vectors and the y_k are m-vectors.
 * Since these distributions are normal, it suffices to compute
 * their means and covariances, which this routine puts in *means
 * and *covariances. The rest of the parameters are best explained by
 * seeing the equations of motion of the LDS:
 *
 *   x_1 = mu_0 + u,
 *   x_k = A_k * x_{k-1} + w_k
 *   y_k = H_k * z_i + v_k
 *
 * where w_k ~ N(0, Q_k) and v_k ~ N(0, R_k) and u ~ N(0, S_0), and the A_k
 * and Q_k are nxn matrices, the H_k is an mxn matrix and R_k is an mxm matrix.
 * Finally, this routine also computes the incomplete-data log-likelihood (LOG!)
 * of the LDS, i.e.,
 *
 *              log p(y_1, y_2, ..., y_N | A, H, Q, R, mu_0, S_0).
 *
 * The difference between this and compute_kalman_filter_2 is that the covariance
 * matrices in this routine are computed using a more numerically stable method.
 * Specifically, it computes S_k as
 *
 * S_k = (I - K_k*H_k)*S'_k*(I-K_k*H_k)^T + K_k*R_k*K_k^T,
 * 
 * where as compute_kalman_filter uses the more traditional
 *
 * S_k = (I-K_k*H_k)S'_k.
 *
 * As usual, *means and *covariances are reused if possible and created
 * if needed, according to the KJB allocation semantics. Any result that is
 * not desired can be omitted by passing NULL to the rouitne.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Author: Ernesto Brau
 *
 * Documentor: Ernesto Brau
 *
 * Index: LDS
 *
 * -----------------------------------------------------------------------------
*/

int compute_kalman_filter_2_stable
(
    Vector_vector**      means,
    Matrix_vector**      covariances,
    double*              likelihood,
    const Vector_vector* y,
    const Matrix_vector* A,
    const Matrix_vector* Q,
    const Matrix_vector* H,
    const Matrix_vector* R,
    const Vector*        mu_0,
    const Matrix*        S_0
)
{
    Vector_vector* mu = NULL;
    Vector_vector* mu_p = NULL;
    Matrix_vector* S = NULL;
    Matrix_vector* S_p = NULL;
    Matrix_vector* K = NULL;
    Matrix* temp_matrix_1 = NULL;
    Matrix* temp_matrix_2 = NULL;
    Matrix* temp_matrix_3 = NULL;
    Vector* temp_vector_1 = NULL;
    Vector* temp_vector_2 = NULL;
    Vector* lh_mu = NULL;
    Matrix* lh_sigma = NULL;
    double loc_lh;
    int k;
    int n, m;

    /********* Error checking and initialization and stuff ***********/

    if(means == NULL && covariances == NULL && likelihood == NULL)
    {
        return NO_ERROR;
    }

    if(y == NULL || A == NULL || Q == NULL || H == NULL || R == NULL || mu_0 == NULL || S_0 == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* are there enough parameters? */
    if(A->length < y->length || Q->length < y->length || H->length < y->length || R->length < y->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* are all corresponding matrices and vectors the same size? */
    if(!is_matrix_vector_consistent(A) || !is_matrix_vector_consistent(Q) || !is_matrix_vector_consistent(H) || !is_matrix_vector_consistent(R) || !is_vector_vector_consistent(y))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* are square matrices square? */
    if(A->elements[0]->num_rows != A->elements[0]->num_cols || Q->elements[0]->num_rows != Q->elements[0]->num_cols || R->elements[0]->num_rows != R->elements[0]->num_cols || S_0->num_rows != S_0->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    n = A->elements[0]->num_rows;
    m = y->elements[0]->length;

    /* do the sizes match up? */
    if(H->elements[0]->num_rows != m || H->elements[0]->num_cols != n || Q->elements[0]->num_rows != n || R->elements[0]->num_rows != m || S_0->num_rows != n || mu_0->length != n)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(means != NULL)
    {
        get_target_vector_vector(means, y->length);
        mu = *means;
        get_target_vector_vector(&mu_p, y->length);
    }
    else
    {
        if(likelihood != NULL)
        {
            get_target_vector_vector(&mu, y->length);
            get_target_vector_vector(&mu_p, y->length);
        }
    }

    if(covariances == NULL)
    {
        get_target_matrix_vector(&S, y->length);
    }
    else
    {
        get_target_matrix_vector(covariances, y->length);
        S = *covariances;
    }

    get_target_matrix_vector(&S_p, y->length);
    get_target_matrix_vector(&K, y->length);

    /********* Start of algorithm ***********/

    *likelihood = 0.0;
    for(k = 0; k < y->length; k++)
    {
        /** Prediction stage **/
        /* compute {mu-prime}_k */
        if(mu != NULL)
        {
            if(k != 0)
            {
                multiply_matrix_and_vector(&mu_p->elements[k], A->elements[k - 1], mu->elements[k - 1]);
            }
            else
            {
                copy_vector(&mu_p->elements[k], mu_0);
            }
        }

        /* compute {sigma-prime}_k */
        if(k != 0)
        {
            multiply_by_transpose(&temp_matrix_1, S->elements[k - 1], A->elements[k - 1]);
            multiply_matrices(&temp_matrix_2, A->elements[k - 1], temp_matrix_1);
            add_matrices(&S_p->elements[k], temp_matrix_2, Q->elements[k - 1]);
        }
        else
        {
            copy_matrix(&S_p->elements[k], S_0);
        }

        /** Weighting stage **/
        /* compute K_k */
        multiply_by_transpose(&temp_matrix_1, S_p->elements[k], H->elements[k]);
        multiply_matrices(&temp_matrix_3, H->elements[k], temp_matrix_1);
        ow_add_matrices(temp_matrix_3, R->elements[k]);
        ERE(get_matrix_inverse(&temp_matrix_1, temp_matrix_3));
        multiply_with_transpose(&temp_matrix_2, H->elements[k], temp_matrix_1);
        multiply_matrices(&K->elements[k], S_p->elements[k], temp_matrix_2);

        /** Correction stage **/
        /* compute  mu_k */
        if(mu != NULL)
        {
            multiply_matrix_and_vector(&temp_vector_1, H->elements[k], mu_p->elements[k]);
            subtract_vectors(&temp_vector_2, y->elements[k], temp_vector_1);
            multiply_matrix_and_vector(&temp_vector_1, K->elements[k], temp_vector_2);
            add_vectors(&mu->elements[k], mu_p->elements[k], temp_vector_1);
        }

        /* compute sigma_k */
        multiply_matrices(&temp_matrix_1, K->elements[k], H->elements[k]);
        get_identity_matrix(&temp_matrix_2, temp_matrix_1->num_rows);
        ow_subtract_matrices(temp_matrix_2, temp_matrix_1);
        multiply_matrices(&temp_matrix_1, temp_matrix_2, S_p->elements[k]);
        multiply_by_transpose(&S->elements[k], temp_matrix_1, temp_matrix_2);
        multiply_by_transpose(&temp_matrix_1, R->elements[k], K->elements[k]);
        multiply_matrices(&temp_matrix_2, K->elements[k], temp_matrix_1);
        ow_add_matrices(S->elements[k], temp_matrix_2);

        /* update likelihood */
        multiply_matrix_and_vector(&lh_mu, H->elements[k], mu_p->elements[k]);
        copy_matrix(&lh_sigma, temp_matrix_3);
        get_log_density_gaussian(&loc_lh, y->elements[k], lh_mu, lh_sigma);
        /* get_log_density_gaussian(&loc_lh, lh_mu, lh_mu, lh_sigma); */
        *likelihood += loc_lh;
    }

    /********* Memory cleanup ***********/

    if(means != NULL)
    {
        free_vector_vector(mu_p);
    }
    else
    {
        if(likelihood != NULL)
        {
            free_vector_vector(mu);
            free_vector_vector(mu_p);
        }
    }

    if(covariances == NULL)
    {
        free_matrix_vector(S);
    }

    if(likelihood != NULL)
    {
        free_vector(lh_mu);
        free_matrix(lh_sigma);
    }

    free_matrix_vector(S_p);
    free_matrix_vector(K);
    free_matrix(temp_matrix_1);
    free_matrix(temp_matrix_2);
    free_matrix(temp_matrix_3);
    free_vector(temp_vector_1);
    free_vector(temp_vector_2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

