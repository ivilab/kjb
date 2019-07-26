
/* $Id: gp_gaussian_processes.h 5464 2010-02-17 17:25:35Z ernesto $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision Group     |
|  (the authors) including                                                     |
|        Kobus Barnard.                                                        |
|        Ernesto Brau                                                          |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

#ifndef GP_GAUSSIAN_PROCESSES_H_INCLUDED
#define GP_GAUSSIAN_PROCESSES_H_INCLUDED


#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int fill_covariance_matrix
(
    Matrix**             cov,
    const Vector_vector* indices,
    const Vector_vector* indices_2,
    const void*          hyper_params,
    int                  d,
    int                  (*covariance)(Matrix**, const Vector*, const Vector*, const void*, int)
);

int fill_mean_vector
(
    Vector**             mean,
    const Vector_vector* indices,
    int                  (*mean_func)(Vector**, const Vector*, int),
    int                  d
);

int sample_from_gaussian_process_prior
(
    Vector_vector**      sample,
    const Vector_vector* indices,
    int                  (*mean_func)(Vector**, const Vector*, int),
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    int                  d
);

int sample_from_gaussian_process_prior_i
(
    Vector_vector**      sample,
    const Vector_vector* indices,
    int                  (*mean_func)(Vector**, const Vector*, int),
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    int                  d
);

int sample_from_gaussian_process_predictive
(
    Vector_vector**      sample,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    const Vector_vector* test_indices,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
);

int sample_from_gaussian_process_predictive_i
(
    Vector_vector**      sample,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    const Vector_vector* test_indices,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
);

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
);

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
);

int get_gaussian_process_posterior_distribution
(
    Vector**             mu,
    Matrix**             sigma,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
);

int get_gaussian_process_posterior_distribution_i
(
    Vector_vector**      mus,
    Matrix_vector**      sigmas,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    double               noise_sigma,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params
);

int compute_gaussian_process_likelihood
(
    double*              density,
    const Vector_vector* train_data,
    const Vector_vector* function_values,
    double               noise_sigma
);

int compute_gaussian_process_likelihood_i
(
    double*              density,
    const Vector_vector* train_data,
    const Vector_vector* function_values,
    double               noise_sigma
);

int compute_gaussian_process_marginal_likelihood
(
    double*              density,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    double               noise_sigma
);

int compute_gaussian_process_marginal_likelihood_i
(
    double*              density,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    double               noise_sigma
);

int compute_gaussian_process_marginal_log_likelihood
(
    double*              density,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    double               noise_sigma
);

int compute_gaussian_process_marginal_log_likelihood_i
(
    double*              density,
    const Vector_vector* train_indices,
    const Vector_vector* train_data,
    int                  (*cov_func)(Matrix**, const Vector*, const Vector*, const void*, int),
    const void*          hyper_params,
    double               noise_sigma
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif /* GAUSSIAN_PROCESSES_H_ */

