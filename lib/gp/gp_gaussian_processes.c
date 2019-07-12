
/* $Id: gp_gaussian_processes.c 6352 2010-07-11 20:13:21Z kobus $ */

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

#include "gp/gp_gaussian_processes.h"
#include "l/l_incl.h"
#include "n/n_incl.h"
#include "sample/sample_incl.h"

#ifdef __cplusplus
extern "C" {
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                fill_covariance_matrix
 *
 * Fills a covariance matrix for a Gaussian process
 *
 * This routine creates a covariance matrix for a Gaussian process where the
 * dimension of the indices is given by the dimension of the vectors of indices
 * and indices_2, and the dimension of the variables is d. Then, it fills *cov 
 * with submatrices that are the result of calling the given covariance function
 * for a pair of indices.
 *
 * For example, the submatrix of *cov of elements 0,1,...,d is A, where A is the
 * matrix gotten in the call
 *
 *        covariance(&A, indices->elements[0], indices_2->elements[0], d).
 *
 * Naturally, indices and indices_2 must have the same length, and all of their
 * vectors must have equal lengths. Also, covariance must "return" a dxd matrix 
 * (there are some predefined covariance functions in the library). If the routine
 * succeeds, cov will be a square matrix of dimension indices->elements[0]->length*d.
 * hyper_params holds any hyper parameters that the covariance function needs. The
 * covariance function itself should cast it to the correct type.
 *
 * If the matrix pointed to by cov is NULL, then a matrix of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation or an error in the covariance
 *    function), then ERROR is returned with and error message being set. Otherwise
 *    NO_ERROR is returned.
 *
 * Related:
 *    squared_exponential_covariance_function
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int fill_covariance_matrix
(
    Matrix**             cov,
    const Vector_vector* indices,
    const Vector_vector* indices_2,
    const void*          hyper_params,
    int                  d,
    int                  (*covariance)(Matrix**, const Vector*, const Vector*, const void*, int)
)
{
    Matrix* temp_matrix = NULL;
    double pd_eps = 0.000001;
    int i, j;

    if(indices == NULL || indices_2 == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    get_target_matrix(cov, indices->length * d, indices_2->length * d);

    for(i = 0; i < indices->length; i++)
    {
        for(j = 0; j < indices_2->length; j++)
        {
            ERE(covariance(&temp_matrix, indices->elements[i], indices_2->elements[j], hyper_params, d));
            ow_copy_matrix(*cov, i * d, j * d, temp_matrix);
        }
    }

    for(i = 0; i < (*cov)->num_rows; i++)
    {
        (*cov)->elements[i][i] += pd_eps;
    }

    free_matrix(temp_matrix);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                fill_mean_vector
 *
 * Fills a mean vector for a Gaussian process
 *
 * This routine creates a mean vector for a Gaussian process where the dimension
 * of the indices is given by the dimension of the vectors of indices, and the
 * dimension of the variables is d. Then, it fills *mean with subvectors that are
 * the result of calling the given mean function for that index.
 *
 * For example, the subvector of *mean of elements 0,1,...,d is v, where v is the
 * vector gotten in the call
 *
 *                      mean_func(&v, indices->elements[0], d).
 *
 * Naturally, all of the vectors of indices must have equal lengths. Also,
 * mean_func must "return" d-vector (there are some predefined mean functions in
 * the library). If the routine succeeds, mean will be of length
 * indices->elements[0]->length * d.
 *
 * If the vector pointed to by mean is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation or an error in the mean
 *    function), then ERROR is returned with and error message being set. Otherwise
 *    NO_ERROR is returned.
 *
 * Related:
 *    zero_mean_vector
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int fill_mean_vector
(
    Vector**             mean,
    const Vector_vector* indices,
    int                  (*mean_func)(Vector**, const Vector*, int),
    int                  d
)
{
    Vector* temp_vector = NULL;
    int i;

    if(indices == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    get_target_vector(mean, indices->length * d);

    for(i = 0; i < indices->length; i++)
    {
        ERE(mean_func(&temp_vector, indices->elements[i], d));
        ow_copy_vector(*mean, i * d, temp_vector);
    }

    free_vector(temp_vector);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      sample_from_gaussian_process_prior
 *
 * Samples from the prior of a Gaussian process
 *
 * This routine samples from the prior distribution of a Gaussian process, whose
 * mean function and covariance function are given by mean_func and cov_func,
 * respectively. The sample will be in the indices given by the vectors of indices,
 * and will each be of dimension d.
 *
 * Naturally, the vectors of indices must all be of equal length, and mean_func and
 * cov_func must return a d-vector and a square matrix of dimension d, respectively.
 * The library provides a few widely-used mean and covariance functions. Also, sample
 * will have indices->length vectors, each of length d.
 *
 * If the vector pointed to by sample is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the mean or
 *    covariance functions, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Related:
 *    squared_exponential_covariance_function, zero_mean_function,
 *    sample_from_gaussian_process_prior_i
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int sample_from_gaussian_process_prior
(
    Vector_vector**      sample,
    const Vector_vector* indices,
    int                  (*mean_func)(Vector**, const Vector*, int),
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    int                  d
)
{
    Vector* mu = NULL;
    Matrix* sigma = NULL;
    Vector* temp_sample = NULL;

    ERE(fill_mean_vector(&mu, indices, mean_func, d));
    ERE(fill_covariance_matrix(&sigma, indices, indices, hyper_params, d, cov_func));

    ERE(mv_gaussian_rand(&temp_sample, mu, sigma));
    vector_vector_from_vector(sample, temp_sample, d);

    free_vector(mu);
    free_matrix(sigma);
    free_vector(temp_sample);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      sample_from_gaussian_process_prior_i
 *
 * Samples from the prior of a Gaussian process with independent dimensions
 *
 * This routine samples from the prior distribution of a Gaussian process, whose
 * mean function and covariance function are given by mean_func and cov_func,
 * respectively, and whose dimensions are independent. The sample will be in the
 * indices given by the vectors of indices, and will each be of dimension d.
 *
 * Naturally, the vectors of indices must all be of equal length. The library
 * provides a few widely-used mean and covariance functions. Also, sample
 * will have indices->length vectors, each of length d.
 *
 * If the vector pointed to by sample is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the mean or
 *    covariance functions, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Related:
 *    squared_exponential_covariance_function, zero_mean_function,
 *    sample_from_gaussian_process_prior
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int sample_from_gaussian_process_prior_i
(
    Vector_vector**      sample,
    const Vector_vector* indices,
    int                  (*mean_func)(Vector**, const Vector*, int),
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    int                  d
)
{
    Vector* mu = NULL;
    Matrix* sigma = NULL;
    Vector_vector* temp_samples = NULL;
    int i;

    get_target_vector_vector(&temp_samples, d);

    for(i = 0; i < d; i++)
    {
        ERE(fill_mean_vector(&mu, indices, mean_func, 1));
        ERE(fill_covariance_matrix(&sigma, indices, indices, hyper_params, 1, cov_func));

        ERE(mv_gaussian_rand(&temp_samples->elements[i], mu, sigma));
    }
    get_vector_vector_transpose(sample, temp_samples);

    free_vector(mu);
    free_matrix(sigma);
    free_vector_vector(temp_samples);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                  sample_from_gaussian_process_predictive
 *
 * Samples from the predictive distribution of a Gaussian process
 *
 * This routine samples from the predictive distribution of a Gaussian process,
 * whose covariance function is given by cov_func. The sample will be in the indices
 * given by the vectors of test_indices, and will each be of the dimension of the
 * vectors in train_data.
 *
 * The necessary information to sample from the predictive distribution is:
 *
 * train_indices - the indices where the training data comes from
 * train_data - the training data
 * noise_sigma - the variance of the gaussian noise process
 * test_indices - where the prediction will take place.
 *
 * Naturally, the vectors of test_indices must all be of equal length, as must the
 * vectors of train_indices and train_data, and cov_func must return square
 * matrix of dimension d. Also, sample will have test_indices->length vectors,
 * each of length train_data->elements[0]->length.
 *
 * If the vector pointed to by sample is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int sample_from_gaussian_process_predictive
(
    Vector_vector**      sample,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    const Vector_vector* test_indices,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
)
{
    Vector* mu = NULL;
    Matrix* sigma = NULL;
    Vector* temp_sample = NULL;
    int d = train_data->elements[0]->length;

    ERE(get_gaussian_process_predictive_distribution(&mu, &sigma, train_indices, train_data, noise_sigma, test_indices, cov_func, hyper_params));

    ERE(mv_gaussian_rand(&temp_sample, mu, sigma));
    vector_vector_from_vector(sample, temp_sample, d);

    free_vector(mu);
    free_matrix(sigma);
    free_vector(temp_sample);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                  sample_from_gaussian_process_predictive_i
 *
 * Samples from the predictive distribution of a Gaussian process
 *
 * This routine samples from the predictive distribution of a Gaussian process,
 * whose covariance function is given by cov_func and whose dimensions are
 * independent. The sample will be in the indices given by the vectors of
 * test_indices, and will each be of the dimension of the vectors in train_data.
 *
 * The necessary information to sample from the predictive distribution is:
 *
 * train_indices - the indices where the training data comes from
 * train_data - the training data
 * noise_sigma - the variance of the gaussian noise process
 * test_indices - where the prediction will take place.
 *
 * Naturally, the vectors of test_indices must all be of equal length, as must the
 * vectors of train_indices and train_data. Also, sample will have test_indices->length
 * vectors, each of length train_data->elements[0]->length.
 *
 * If the vector pointed to by sample is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int sample_from_gaussian_process_predictive_i
(
    Vector_vector**      sample,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    const Vector_vector* test_indices,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
)
{
    Vector_vector* mus = NULL;
    Matrix_vector* sigmas = NULL;
    Vector_vector* temp_samples = NULL;
    int d = train_data->elements[0]->length;
    int i;

    get_target_vector_vector(&temp_samples, d);

    ERE(get_gaussian_process_predictive_distribution_i(&mus, &sigmas, train_indices, train_data, noise_sigma, test_indices, cov_func, hyper_params));

    for(i = 0; i < d; i++)
    {
        ERE(mv_gaussian_rand(&temp_samples->elements[i], mus->elements[i], sigmas->elements[i]));
    }
    get_vector_vector_transpose(sample, temp_samples);

    free_vector_vector(mus);
    free_matrix_vector(sigmas);
    free_vector_vector(temp_samples);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                  get_gaussian_process_predictive_distribution
 *
 * Computes the predictive distribution of a GP
 *
 * This routine computes the predictive distribution of a Gaussian process,
 * whose covariance function is given by cov_func. mu is the mean and sigma is 
 * covariance matrix of the distribution.
 *
 * The necessary information to compute the predictive distribution is:
 *
 * train_indices - the indices where the training data comes from
 * train_data - the training data
 * noise_sigma - the variance of the gaussian noise process
 * test_indices - where the prediction will take place.
 *
 * Naturally, the vectors of test_indices must all be of equal length, as must the
 * vectors of train_indices and train_data, and cov_func must return square
 * matrix of dimension d. 
 *
 * If the vector pointed to by sample is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int get_gaussian_process_predictive_distribution
(
    Vector**             mu,
    Matrix**             sigma,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    const Vector_vector* test_indices,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
)
{
    Vector* y = NULL;
    Vector* temp_vector = NULL;
    Matrix* k_t_t = NULL;
    Matrix* k_ts_t = NULL;
    Matrix* k_t_ts = NULL;
    Matrix* k_ts_ts = NULL;
    Matrix* temp_matrix = NULL;
    Matrix* temp_matrix_2 = NULL;

    int n = train_data->length;
    int d = train_data->elements[0]->length;

    ERE(flatten_vector_vector(&y, train_data));

    ERE(fill_covariance_matrix(&k_t_t, train_indices, train_indices, hyper_params, d, cov_func));
    ERE(fill_covariance_matrix(&k_ts_t, test_indices, train_indices, hyper_params, d, cov_func));
    ERE(fill_covariance_matrix(&k_t_ts, train_indices, test_indices, hyper_params, d, cov_func));
    ERE(fill_covariance_matrix(&k_ts_ts, test_indices, test_indices, hyper_params, d, cov_func));

    get_identity_matrix(&temp_matrix, d * n);
    ow_multiply_matrix_by_scalar(temp_matrix, noise_sigma);
    ow_add_matrices(temp_matrix, k_t_t);
    ERE(get_matrix_inverse(&temp_matrix_2, temp_matrix));
    multiply_matrices(&temp_matrix, k_ts_t, temp_matrix_2);
    multiply_matrix_and_vector(mu, temp_matrix, y);

    multiply_matrices(&temp_matrix_2, temp_matrix, k_t_ts);
    subtract_matrices(sigma, k_ts_ts, temp_matrix_2);

    free_vector(y);
    free_vector(temp_vector);
    free_matrix(k_t_t);
    free_matrix(k_ts_t);
    free_matrix(k_t_ts);
    free_matrix(k_ts_ts);
    free_matrix(temp_matrix);
    free_matrix(temp_matrix_2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                  get_gaussian_process_predictive_distribution_i
 *
 * Computes the predictive distribution of a GP
 *
 * This routine computes the predictive distribution of a Gaussian process,
 * whose covariance function is given by cov_func and whose dimensions are
 * independent. mus is the vector of means and sigmas the vector of covariance
 * matrices of the independent Gaussians.
 *
 * The necessary information to compute the predictive distribution is:
 *
 * train_indices - the indices where the training data comes from
 * train_data - the training data
 * noise_sigma - the variance of the gaussian noise process
 * test_indices - where the prediction will take place.
 *
 * Naturally, the vectors of test_indices must all be of equal length, as must the
 * vectors of train_indices and train_data.
 *
 * If the vector pointed to by sample is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. 
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int get_gaussian_process_predictive_distribution_i
(
    Vector_vector**      mus,
    Matrix_vector**      sigmas,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    const Vector_vector* test_indices,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
)
{
    Vector_vector* ys = NULL;
    Vector* temp_vector = NULL;
    Matrix* k_t_t = NULL;
    Matrix* k_ts_t = NULL;
    Matrix* k_t_ts = NULL;
    Matrix* k_ts_ts = NULL;
    Matrix* temp_matrix = NULL;
    Matrix* temp_matrix_2 = NULL;

    int n = train_data->length;
    int d = train_data->elements[0]->length;
    int i;

    ERE(get_vector_vector_transpose(&ys, train_data));
    get_target_vector_vector(mus, d);
    get_target_matrix_vector(sigmas, d);

    for(i = 0; i < d; i++)
    {
        ERE(fill_covariance_matrix(&k_t_t, train_indices, train_indices, hyper_params, 1, cov_func));
        ERE(fill_covariance_matrix(&k_ts_t, test_indices, train_indices, hyper_params, 1, cov_func));
        ERE(fill_covariance_matrix(&k_t_ts, train_indices, test_indices, hyper_params, 1, cov_func));
        ERE(fill_covariance_matrix(&k_ts_ts, test_indices, test_indices, hyper_params, 1, cov_func));

        get_identity_matrix(&temp_matrix, n);
        ow_multiply_matrix_by_scalar(temp_matrix, noise_sigma);
        ow_add_matrices(temp_matrix, k_t_t);
        ERE(get_matrix_inverse(&temp_matrix_2, temp_matrix));
        multiply_matrices(&temp_matrix, k_ts_t, temp_matrix_2);
        multiply_matrix_and_vector(&(*mus)->elements[i], temp_matrix, ys->elements[i]);

        multiply_matrices(&temp_matrix_2, temp_matrix, k_t_ts);
        subtract_matrices(&(*sigmas)->elements[i], k_ts_ts, temp_matrix_2);
    }

    free_vector_vector(ys);
    free_vector(temp_vector);
    free_matrix(k_t_t);
    free_matrix(k_ts_t);
    free_matrix(k_t_ts);
    free_matrix(k_ts_ts);
    free_matrix(temp_matrix);
    free_matrix(temp_matrix_2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                  get_gaussian_process_posterior_distribution
 *
 * Gets the mean and covariance for the posterior of a Gaussian process
 *
 * This routine gives the mean vector (in mu) and covariance matrix (in sigma) of
 * the posterior distribution of a Gaussian process with covariance function 
 * cov_func, and whose training data train_data exists at indices train_indices. 
 * Finally, the (Gaussian) noise has variance noise_sigma. mu will have dimension
 * given by
 *
 *      train_indices->length * train_data->elements[0]->length,
 *
 * and sigma will be a square matrix of the same dimension.
 *
 * Naturally, the vectors of train_indices must all be of equal length, as must
 * the vectors of train_data, and cov_func must return square matrix of dimension d.
 *
 * It's worth noting that the mean of the posterior is the MAP estimate of the GP.
 *
 * If the vector pointed to by mu is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. The same goes for the matrix pointed to by sigma.
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int get_gaussian_process_posterior_distribution
(
    Vector**             mu,
    Matrix**             sigma,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
)
{
    Matrix* k_inv = NULL;
    Matrix* temp_matrix = NULL;
    Matrix* sigma_loc = NULL;
    Vector* mu_loc = NULL;
    Vector* train_data_ext = NULL;

    int d = train_data->elements[0]->length;
    int n = train_data->length;

    if(mu == NULL && sigma == NULL)
    {
        return NO_ERROR;
    }

    ERE(flatten_vector_vector(&train_data_ext, train_data));
    ERE(fill_covariance_matrix(&temp_matrix, train_indices, train_indices, hyper_params, d, cov_func));
    ERE(get_matrix_inverse(&k_inv, temp_matrix));
    get_identity_matrix(&temp_matrix, n * d);
    ow_multiply_matrix_by_scalar(temp_matrix, 1.0 / noise_sigma);
    ow_add_matrices(temp_matrix, k_inv);
    ERE(get_matrix_inverse(&sigma_loc, temp_matrix));

    multiply_matrix_and_vector(&mu_loc, sigma_loc, train_data_ext);
    ow_multiply_vector_by_scalar(mu_loc, 1.0 / noise_sigma);

    if(sigma != NULL)
    {
        copy_matrix(sigma, sigma_loc);
    }

    if(mu != NULL)
    {
        copy_vector(mu, mu_loc);
    }

    free_matrix(k_inv);
    free_matrix(temp_matrix);
    free_matrix(sigma_loc);
    free_vector(mu_loc);
    free_vector(train_data_ext);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                  get_gaussian_process_posterior_distribution_i
 *
 * Gets the mean and covariance for the posterior of a Gaussian process
 *
 * This routine gives the mean vectors (in mus) and covariance matrices (in sigmas)
 * of the posterior distribution of a Gaussian process with covariance function 
 * cov_func, and whose training data train_data exists at indices train_indices,
 * and whose dimensions are statistically independent. Finally, the (Gaussian)
 * noise has variance noise_sigma. each vector in mus will have dimension given by
 * train_indices->length and sigma will be a square matrix of the same dimension.
 *
 * Naturally, the vectors of train_indices must all be of equal length, as must
 * the vectors of train_data, and cov_func must return square matrix of dimension d.
 *
 * It's worth noting that the mean of the posterior is the MAP estimate of the GP.
 *
 * If the vector pointed to by mu is NULL, then a vector of the appropriate size
 * is created. If it exists, but is the wrong size, then it is recreated. Otherwise,
 * the storage is recycled. The same goes for the matrix pointed to by sigma.
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int get_gaussian_process_posterior_distribution_i
(
    Vector_vector**      mus,
    Matrix_vector**      sigmas,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
)
{
    Matrix* k_inv = NULL;
    Matrix* temp_matrix = NULL;
    Matrix* sigma_loc = NULL;
    Vector* mu_loc = NULL;
    Vector_vector* train_datas = NULL;

    int d = train_data->elements[0]->length;
    int n = train_data->length;
    int i;

    if(mus == NULL && sigmas == NULL)
    {
        return NO_ERROR;
    }

    get_target_vector_vector(mus, d);
    get_target_matrix_vector(sigmas, d);
    get_vector_vector_transpose(&train_datas, train_data);

    for(i = 0; i < d; i++)
    {
        ERE(fill_covariance_matrix(&temp_matrix, train_indices, train_indices, hyper_params, 1, cov_func));
        ERE(get_matrix_inverse(&k_inv, temp_matrix));
        get_identity_matrix(&temp_matrix, n);
        ow_multiply_matrix_by_scalar(temp_matrix, 1.0 / noise_sigma);
        ow_add_matrices(temp_matrix, k_inv);
        ERE(get_matrix_inverse(&sigma_loc, temp_matrix));
        multiply_matrix_and_vector(&mu_loc, sigma_loc, train_datas->elements[i]);
        ow_multiply_vector_by_scalar(mu_loc, 1.0 / noise_sigma);
        if(mus != NULL)
        {
            copy_vector(&(*mus)->elements[i], mu_loc);
        }
        if(sigmas != NULL)
        {
            copy_matrix(&(*sigmas)->elements[i], sigma_loc);
        }
    }

    free_matrix(k_inv);
    free_matrix(temp_matrix);
    free_matrix(sigma_loc);
    free_vector(mu_loc);
    free_vector_vector(train_datas);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      compute_gaussian_process_likelihood
 *
 * Computes the likelihood of the data given parameters
 *
 * This routine computes the likelihood of the data given the parameters. In the
 * context of Gaussian processes, this means computing the likelihood of the 
 * training data (train_data) give some function values (function_values) at time
 * points train_indices. noise_sigma is the variance of the noise process.
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int compute_gaussian_process_likelihood
(
    double*              density,
    const Vector_vector* train_data,
    const Vector_vector* function_values,
    double               noise_sigma
)
{
    Matrix* sigma = NULL;
    Vector* temp_vec1 = NULL;
    Vector* temp_vec2 = NULL;
    int d = train_data->elements[0]->length;
    int n = train_data->length;


    get_identity_matrix(&sigma, n * d);
    ow_multiply_matrix_by_scalar(sigma, noise_sigma);

    flatten_vector_vector(&temp_vec1, train_data);
    flatten_vector_vector(&temp_vec2, function_values);
    ERE(mv_gaussian_pdf(density, temp_vec1, temp_vec2, sigma));

    free_matrix(sigma);
    free_vector(temp_vec1);
    free_vector(temp_vec2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      compute_gaussian_process_likelihood_i
 *
 * Computes the likelihood of a GP the data given parameters
 *
 * This routine computes the likelihood of the data given the parameters. In the
 * context of Gaussian processes, this means computing the likelihood of the 
 * training data (train_data) give some function values (function_values) at time
 * points train_indices. noise_sigma is the variance of the noise process. This
 * routine assumes that the dimensions of the data are (statistically) independent.
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int compute_gaussian_process_likelihood_i
(
    double*              density,
    const Vector_vector* train_data,
    const Vector_vector* function_values,
    double               noise_sigma
)
{
    Matrix* sigma = NULL;
    Vector_vector* temp_vv1 = NULL;
    Vector_vector* temp_vv2 = NULL;
    int d = train_data->elements[0]->length;
    int n = train_data->length;
    double temp_density;
    int i;

    ERE(get_vector_vector_transpose(&temp_vv1, train_data));
    ERE(get_vector_vector_transpose(&temp_vv2, function_values));
    *density = 1.0;
    for(i = 0; i < d; i++)
    {
        get_identity_matrix(&sigma, n);
        ow_multiply_matrix_by_scalar(sigma, noise_sigma);

        ERE(mv_gaussian_pdf(&temp_density, temp_vv1->elements[i], temp_vv2->elements[i], sigma));
        *density *= temp_density;
    }

    free_matrix(sigma);
    free_vector_vector(temp_vv1);
    free_vector_vector(temp_vv2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                  compute_gaussian_process_marginal_likelihood
 *
 * Computes the marginal likelihood of the data
 *
 * This routine computes the marginal likelihood of the data given a GP, p(data).
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int compute_gaussian_process_marginal_likelihood
(
    double*              density,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    double               noise_sigma
)
{
    Matrix* sigma = NULL;
    Matrix* id = NULL;
    Vector* zeros = NULL;
    Vector* temp_vec1 = NULL;
    int d = train_data->elements[0]->length;
    int n = train_data->length;

    get_zero_vector(&zeros, n * d);
    ERE(fill_covariance_matrix(&sigma, train_indices, train_indices, hyper_params, d, cov_func));
    get_identity_matrix(&id, n * d);
    ow_multiply_matrix_by_scalar(id, noise_sigma);
    ow_add_matrices(sigma, id);

    flatten_vector_vector(&temp_vec1, train_data);
    ERE(mv_gaussian_pdf(density, temp_vec1, zeros, sigma));

    free_matrix(sigma);
    free_matrix(id);
    free_vector(zeros);
    free_vector(temp_vec1);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                  compute_gaussian_process_marginal_likelihood_i
 *
 * Computes the marginal likelihood of the data
 *
 * This routine computes the marginal likelihood of the data given a GP, p(data),
 * whose dimensions are independent of each other.
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int compute_gaussian_process_marginal_likelihood_i
(
    double*              density,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    double               noise_sigma
)
{
    Matrix* sigma = NULL;
    Matrix* id = NULL;
    Vector* zeros = NULL;
    Vector_vector* train_datas = NULL;
    int d = train_data->elements[0]->length;
    int n = train_data->length;
    int i;
    double temp_density;

    ERE(get_vector_vector_transpose(&train_datas, train_data));

    *density = 1.0;
    for(i = 0; i < d; i++)
    {
        get_zero_vector(&zeros, n);
        ERE(fill_covariance_matrix(&sigma, train_indices, train_indices, hyper_params, 1, cov_func));
        get_identity_matrix(&id, n);
        ow_multiply_matrix_by_scalar(id, noise_sigma);
        ow_add_matrices(sigma, id);

        ERE(mv_gaussian_pdf(&temp_density, train_datas->elements[i], zeros, sigma));
        *density *= temp_density;
    }

    free_matrix(sigma);
    free_matrix(id);
    free_vector(zeros);
    free_vector_vector(train_datas);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *              compute_gaussian_process_marginal_log_likelihood
 *
 * Computes the marginal log-likelihood of the data
 *
 * This routine computes the marginal log-likelihood of the data of a GP, log p(data).
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int compute_gaussian_process_marginal_log_likelihood
(
    double*              density,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    double               noise_sigma
)
{
    Matrix* sigma = NULL;
    Matrix* id = NULL;
    Vector* zeros = NULL;
    Vector* temp_vec1 = NULL;
    int d = train_data->elements[0]->length;
    int n = train_data->length;

    get_zero_vector(&zeros, n * d);
    ERE(fill_covariance_matrix(&sigma, train_indices, train_indices, hyper_params, d, cov_func));
    get_identity_matrix(&id, n * d);
    ow_multiply_matrix_by_scalar(id, noise_sigma);
    ow_add_matrices(sigma, id);

    flatten_vector_vector(&temp_vec1, train_data);
    ERE(mv_gaussian_log_pdf(density, temp_vec1, zeros, sigma));

    free_matrix(sigma);
    free_matrix(id);
    free_vector(zeros);
    free_vector(temp_vec1);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *              compute_gaussian_process_marginal_log_likelihood_i
 *
 * Computes the marginal log-likelihood of the data
 *
 * This routine computes the marginal log-likelihood of the data of a GP, log p(data),
 * whose dimensions are (statistically) independent.
 *
 * Returns :
 *    If the routine fails (due to storage allocation, an error in the 
 *    covariance function, or a mismatch in the sizes of the indices), then ERROR
 *    is returned with and error message being set. Otherwise NO_ERROR is returned.
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: gaussian processes
 *
 * -----------------------------------------------------------------------------
 */

int compute_gaussian_process_marginal_log_likelihood_i
(
    double*              density,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    double               noise_sigma
)
{
    Matrix* sigma = NULL;
    Matrix* id = NULL;
    Vector* zeros = NULL;
    Vector_vector* train_datas = NULL;
    int d = train_data->elements[0]->length;
    int n = train_data->length;
    int i;
    double temp_density;

    ERE(get_vector_vector_transpose(&train_datas, train_data));
    *density = 0.0;
    for(i = 0; i < d; i++)
    {
        get_zero_vector(&zeros, n);
        ERE(fill_covariance_matrix(&sigma, train_indices, train_indices, hyper_params, 1, cov_func));
        get_identity_matrix(&id, n);
        ow_multiply_matrix_by_scalar(id, noise_sigma);
        ow_add_matrices(sigma, id);

        /* flatten_vector_vector(&temp_vec1, train_data); */
        ERE(mv_gaussian_log_pdf(&temp_density, train_datas->elements[i], zeros, sigma));
        *density += temp_density;
    }

    free_matrix(sigma);
    free_matrix(id);
    free_vector(zeros);
    free_vector_vector(train_datas);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

