
/* $Id: m_stat.c 21704 2017-08-14 19:42:50Z adarsh $ */

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

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "m/m_vec_stat.h"
#include "m/m_vec_metric.h"
#include "m/m_stat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
// CHECK
//
// Perhaps these should be long_double, not double. (But currently, long_double
// is normally double because quad precision is not normally supported in
// hardware on your basic machine.
*/
static double fs_data_sum         = 0.0;
static double fs_data_min;
static double fs_data_max;
static double fs_data_squared_sum = 0.0;
static int    fs_num_data_points  = NOT_SET;
static int    fs_dirty_stat_data  = FALSE;

/* -------------------------------------------------------------------------- */

int add_data_point(double data_value)
{

    fs_num_data_points++;
    fs_data_sum += data_value;
    fs_data_squared_sum += (data_value * data_value);

    if (fs_num_data_points == 1)
    {
        fs_data_min = fs_data_max = data_value;
    }
    else
    {
        if      (data_value < fs_data_min) fs_data_min = data_value;
        else if (data_value > fs_data_max) fs_data_max = data_value;
    }

    fs_dirty_stat_data = TRUE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_data_stats
(
    double* mean_ptr,
    double* stdev_ptr,
    int*    n_ptr,
    double* min_ptr,
    double* max_ptr
)
{
    double temp;


    if (fs_num_data_points == 0)
    {
        set_bug("Call to get_data_stats before add_data_point.");
        return ERROR;
    }

    if (n_ptr != NULL)
    {
        *n_ptr = fs_num_data_points;
    }

    if (mean_ptr != NULL)
    {
        *mean_ptr = (fs_data_sum / (double)fs_num_data_points);
    }

    if (stdev_ptr != NULL)
    {
        if (fs_num_data_points == 1)
        {
            *stdev_ptr  = DBL_NOT_SET;
        }
        else
        {
            temp = fs_data_squared_sum;
            temp -= fs_data_sum * fs_data_sum / fs_num_data_points;
            temp /= (fs_num_data_points - 1);
            *stdev_ptr = sqrt(temp);
        }
    }

    if (min_ptr != NULL)
    {
        *max_ptr = fs_data_min;
    }

    if (max_ptr != NULL)
    {
        *max_ptr = fs_data_max;
    }

    fs_dirty_stat_data = FALSE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_data_stats_2(Stat* stat_ptr)
{
    double temp;


    if (fs_num_data_points == 0)
    {
        set_bug("Call to get_data_stats before add_data_point.");
        return ERROR;
    }

    stat_ptr->n = fs_num_data_points;

    stat_ptr->mean = (fs_data_sum / (double)fs_num_data_points);

    if (fs_num_data_points == 1)
    {
        stat_ptr->stdev = DBL_NOT_SET;
    }
    else
    {
        temp = fs_data_squared_sum;
        temp -= fs_data_sum * fs_data_sum / fs_num_data_points;
        temp /= (fs_num_data_points - 1);
        stat_ptr->stdev = sqrt(temp);
    }

    stat_ptr->min = fs_data_min;
    stat_ptr->max = fs_data_max;

    fs_dirty_stat_data = FALSE;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Stat* get_data_stats_3(void)
{
    Stat*  stat_ptr;
    double temp;


    if (fs_num_data_points == 0)
    {
        set_bug("Call to get_data_stats before add_data_point.");
        return NULL;
    }

    NRN(stat_ptr = TYPE_MALLOC(Stat));

    stat_ptr->n = fs_num_data_points;

    stat_ptr->mean = (fs_data_sum / (double)fs_num_data_points);

    if (fs_num_data_points == 1)
    {
        stat_ptr->stdev = DBL_NOT_SET;
    }
    else
    {
        temp = fs_data_squared_sum;
        temp -= fs_data_sum * fs_data_sum / fs_num_data_points;
        temp /= (fs_num_data_points - 1);
        stat_ptr->stdev = sqrt(temp);
    }

    stat_ptr->min = fs_data_min;
    stat_ptr->max = fs_data_max;

    fs_dirty_stat_data = FALSE;

    return stat_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int clear_data_stats(void)
{
    if (fs_dirty_stat_data)
    {
        set_bug("Attempt to clear data stats with dirty stat data.");
        return ERROR;
    }

    fs_num_data_points  = 0;
    fs_data_sum         = 0.0;
    fs_data_squared_sum = 0.0;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void cleanup_data_stats(void)
{
    fs_dirty_stat_data = FALSE;

    EPE(clear_data_stats());
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef NOT_CURRENTLY_USED
double add_errors(err1, err2)
    double err1;
    double err2;
{


    return sqrt( err1*err1 + err2*err2);
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            get_correlation_coefficient
 *
 * Get the Pearson correlation coefficient of two vectors. 
 *
 * Procedure implemented in this function:
 *
 * For each vector, subtract the mean of the vector from each element of the
 * vector, normalize the modified vectors, then take the scalar (dot) product
 * of the normalized vectors. This scalar product is the correlation
 * coefficient.

 * This routine takes the addresses of two Vector objects and the address of
 * a double and stores the resultant correlation coefficient in the address
 * of the double.
 
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Documenter: Adarsh Pyarelal
 * Testing level: 1 
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int get_correlation_coefficient
(
    const Vector* first_vp,
    const Vector* second_vp,
    double* result_ptr
)
{
    double    first_mean           = average_vector_elements(first_vp);
    double    second_mean          = average_vector_elements(second_vp);
    Vector* first_minus_mean_vp  = NULL;
    Vector* second_minus_mean_vp = NULL;
    /* Copy the vectors stored at the addresses pointed to by first_vp and 
     * second_vp to the addresses pointed at by first_minus_mean_vp and
     * second_minus_mean_vp */

    int     result               = copy_vector(&first_minus_mean_vp, first_vp);

    if (result != ERROR)
    {
        result = copy_vector(&second_minus_mean_vp, second_vp);
    }

    /* Subtract the mean of the first vector from each element of the first
     * vector. */
    if (result != ERROR)
    {
        result = ow_subtract_scalar_from_vector(first_minus_mean_vp,
                                                first_mean);
    }

    /* Subtract the mean of the second vector from each element of the second
     * vector. */

    if (result != ERROR)
    {
        result = ow_subtract_scalar_from_vector(second_minus_mean_vp,
                                                second_mean);
    }

    /* Normalize the vectors (perform the operation in-place). */
    if (result != ERROR)
    {
        result = ow_scale_vector_by_magnitude(first_minus_mean_vp);
    }

    if (result != ERROR)
    {
        result = ow_scale_vector_by_magnitude(second_minus_mean_vp);
    }

    /* Get the dot product of the normalized vectors, and store the result
     * at the address pointed to by result_ptr. */ 
    if (result != ERROR)
    {
        result = get_dot_product(first_minus_mean_vp, second_minus_mean_vp,
                                 result_ptr);
    }

    free_vector(first_minus_mean_vp);
    free_vector(second_minus_mean_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            get_gaussian_density
 *
 * Get the Gaussian density.
 *
 * Given a vector x of values of random variables and vectors corresponding the
 * means and variances of those random variables, this routine calculates the
 * value of the Gaussian pdf at point x. The computation is done using the 
 * function get_log_gaussian_density.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Documenter: Adarsh Pyarelal
 *
 * Testing level: 1 
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int get_gaussian_density
(
    const Vector* x_vp,
    const Vector* mu_vp,
    const Vector* var_vp,
    double*       prob_ptr
)
{
    double    log_prob;

    ERE(get_log_gaussian_density(x_vp, mu_vp, var_vp, &log_prob));

    *prob_ptr = exp(log_prob);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            get_log_gaussian_density
 *
 * Get the log gaussian density
 *
 * Given a vector x of values of random variables and vectors corresponding the
 * means and variances of those random variables, this routine calculates the
 * logarithm of the Gaussian pdf at point x. This routine calls the function
 * get_malhalanobis_distance_sqrd.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Documenter: Adarsh Pyarelal
 * Testing level: 1 
 *
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int get_log_gaussian_density
(
    const Vector* x_vp,
    const Vector* mu_vp,
    const Vector* var_vp,
    double*       log_prob_ptr
)
{
    double dist_sqrd;
    double log_var_prod = 0.0;
    /* Get the dimension of the vectors */
    int    dim          = var_vp->length;
    int    i;
    double log_prob;

    /* Get the Mahalanobis distance and store it at the address of the double
     * dist_sqrt */
    ERE(get_malhalanobis_distance_sqrd(x_vp, mu_vp, var_vp, &dist_sqrd));

    /* Add up the logarithms of the variances of each random variable in the
     * variance vector, and bind it to the variable log_var_prod */
    for (i = 0; i < dim; i++)
    {
        log_var_prod += log(var_vp->elements[ i ]);
    }

    log_prob = -0.5 * dist_sqrd - (log_var_prod / 2.0)  - (((double)dim) / 2.0) * log(2.0 * M_PI);

    *log_prob_ptr = log_prob;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            get_malhalanobis_distance
 *
 * Get the Mahalanobis distance

 * The Mahalanobis distance between a vector $\vec{x}$ and a set of vectors 
 * with mean * $\vec{\mu}$ and covariance matrix S is given by
 *
 * $\sqrt{(\vec{x}-\vec{\mu})^T S^{-1}(\vec{x}-\vec{mu})}$.
 *
 * This particular routine assumes that the covariance matrix is diagonal.
 *
 * This routine takes pointers to three vectors, and a pointer to a double to
 * store the result in. The first vector represents an * observation, the 
 * second the mean of a set of observations, and the third * the variance in 
 * that set of observations. It internally calls the function
 * get_malhalanobis_distance_sqrd. 
 
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Documenter: Adarsh Pyarelal
 * Testing level: 1 
 * 
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

int get_malhalanobis_distance
(
    const Vector* x_vp,
    const Vector* mu_vp,
    const Vector* var_vp,
    double*       dist_ptr
)
{
    double    dist_sqrd;

    ERE(get_malhalanobis_distance_sqrd(x_vp, mu_vp, var_vp, &dist_sqrd));

    *dist_ptr = sqrt(dist_sqrd);

    return NO_ERROR;
}

/*
 * =============================================================================
 *                            get_malhalanobis_distance_sqrd
 *
 * Get the square of the Mahalanobis distance.

 * The Mahalanobis distance between a vector $\vec{x}$ and a set of vectors 
 * with mean * $\vec{\mu}$ and covariance matrix S is given by
 *
 * $\sqrt{(\vec{x}-\vec{\mu})^T S^{-1}(\vec{x}-\vec{mu})}$
 *
 * This routine takes pointers to three vectors, and a pointer to a double to
 * store the result in. The first vector represents an * observation, the 
 * second the mean of a set of observations, and the third * the variance in 
 * that set of observations. 
 
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Documenter: Adarsh Pyarelal
 * 
 * Testing level: 1 
 * Index: matrices, matrix statistics
 *
 * -----------------------------------------------------------------------------
 */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_malhalanobis_distance_sqrd
(
    const Vector* x_vp,
    const Vector* mu_vp,
    const Vector* var_vp,
    double*       dist_sqrd_ptr
)
{
    Vector* d_vp      = NULL;
    Vector* temp_vp   = NULL;
    double  dist_sqrd = NOT_SET;

    /* Subtract vector pointed to by mu_vp from the vector pointed to by x_vp,
     * then store the address of the resultant vector in d_vp. */
    int     result = subtract_vectors(&d_vp, x_vp, mu_vp);

    /* Perform element-wise division of the vector pointed to by d_vp by the
     * the vector pointed to by var_vp (the variance), and store the address
     * of the resultant vector in the pointer temp_vp. */
    if (result != ERROR)
    {
        result = divide_vectors(&temp_vp, d_vp, var_vp);
    }

    /* Get the dot product of the vectors pointed to by d_vp and temp_vp, and
     * store it at the address of dist_sqrd. */
    if (result != ERROR)
    {
        result = get_dot_product(d_vp, temp_vp, &dist_sqrd);
    }

    /* Make sure the distance squared is positive, otherwise perform debug
     * printing of the vector. */
    if (dist_sqrd < 0.0)
    {
        db_rv(var_vp);
    }

    ASSERT(dist_sqrd >= 0.0);

    /* Free the memory allocated for the vectors. */
    free_vector(temp_vp);
    free_vector(d_vp);

    /* Store the result at the address pointed to by dist_sqrt_ptr */
    if (result != ERROR)
    {
        *dist_sqrd_ptr = dist_sqrd;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_gaussian_density_2(const Vector* x_vp, const Vector* u_vp,
                           const Vector* var_vp,
                           double max_norm_var,
                           double* prob_ptr)
{
    double log_prob;


    ERE(get_log_gaussian_density_2(x_vp, u_vp, var_vp, max_norm_var,
                                   &log_prob));

    *prob_ptr = exp(log_prob);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_log_gaussian_density_2(const Vector* x_vp,
                               const Vector* u_vp,
                               const Vector* var_vp,
                               double max_norm_var,
                               double* log_prob_ptr)
{
    double dist_sqrd;
    double log_var_prod = 0.0;
    int    dim          = var_vp->length;
    int    i;
    double log_prob;


    ERE(get_malhalanobis_distance_sqrd_2(x_vp, u_vp, var_vp,
                                         max_norm_var,
                                         &dist_sqrd));

    for (i = 0; i < dim; i++)
    {
        log_var_prod += log(var_vp->elements[ i ]);
    }

    log_prob = -0.5 * dist_sqrd - (log_var_prod / 2.0) - (((double)dim) / 2.0) * log(2.0 * M_PI);

    *log_prob_ptr = log_prob;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_malhalanobis_distance_2(const Vector* x_vp, const Vector* u_vp,
                                const Vector* var_vp, double max_norm_var,
                                double* dist_ptr)
{
    double    dist_sqrd;


    ERE(get_malhalanobis_distance_sqrd_2(x_vp, u_vp, var_vp,
                                         max_norm_var, &dist_sqrd));

    *dist_ptr = sqrt(dist_sqrd);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_malhalanobis_distance_sqrd_2(const Vector* x_vp,
                                     const Vector* u_vp,
                                     const Vector* var_vp,
                                     double max_norm_var,
                                     double* dist_sqrd_ptr)
{
    Vector* d_vp      = NULL;
    Vector* temp_vp   = NULL;
    double  dist_sqrd = DBL_NOT_SET;
    int     result;


    result = subtract_vectors(&d_vp, x_vp, u_vp);

    if (result != ERROR)
    {
        result = divide_vectors(&temp_vp, d_vp, var_vp);
    }

    if (result != ERROR)
    {
        result = ow_multiply_vectors(temp_vp, d_vp);
    }

    if ((result != ERROR) && (max_norm_var > 0.0))
    {
        result = ow_max_thresh_vector(temp_vp, max_norm_var);
    }

    if (result != ERROR)
    {
        dist_sqrd = sum_vector_elements(temp_vp);

        if (dist_sqrd < 0.0)
        {
            db_rv(var_vp);
        }

        ASSERT(dist_sqrd >= 0.0);
    }

    free_vector(temp_vp);
    free_vector(d_vp);

    if (result != ERROR)
    {
        *dist_sqrd_ptr = dist_sqrd;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_gaussian_density_3(const Vector* x_vp, const Vector* u_vp,
                           const Vector* var_vp,
                           double max_norm_var,
                           double* prob_ptr)
{
    double log_prob;


    ERE(get_log_gaussian_density_3(x_vp, u_vp, var_vp, max_norm_var,
                                   &log_prob));

    *prob_ptr = exp(log_prob);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_log_gaussian_density_3(const Vector* x_vp,
                               const Vector* u_vp,
                               const Vector* var_vp,
                               double max_norm_var,
                               double* log_prob_ptr)
{
    double dist_sqrd;
    double log_var_prod = 0.0;
    int    max_dim = x_vp->length;
    int    dim;
    int    i;
    double log_prob;


    ERE(dim = get_malhalanobis_distance_sqrd_3(x_vp, u_vp, var_vp,
                                               max_norm_var,
                                               &dist_sqrd));

    for (i = 0; i < max_dim; i++)
    {
        if (var_vp->elements[ i ] > 0.0)
        {
            log_var_prod += log(var_vp->elements[ i ]);
        }
    }

    log_prob = -0.5 * dist_sqrd - (log_var_prod / 2.0) - (((double)dim) / 2.0) * log(2.0 * M_PI);

    *log_prob_ptr = log_prob;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_malhalanobis_distance_3(const Vector* x_vp, const Vector* u_vp,
                                const Vector* var_vp, double max_norm_var,
                                double* dist_ptr)
{
    double    dist_sqrd;
    int dim;


    ERE(dim = get_malhalanobis_distance_sqrd_3(x_vp, u_vp, var_vp,
                                               max_norm_var, &dist_sqrd));

    *dist_ptr = sqrt(dist_sqrd);

    return dim;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_malhalanobis_distance_sqrd_3(const Vector* x_in_vp,
                                     const Vector* u_in_vp,
                                     const Vector* var_in_vp,
                                     double max_norm_var,
                                     double* dist_sqrd_ptr)
{
    Vector* d_vp      = NULL;
    Vector* temp_vp   = NULL;
    double  dist_sqrd = DBL_NOT_SET;
    int     result;
    int     dim = 0;
    Vector* x_vp = NULL;
    Vector* u_vp = NULL;
    Vector* var_vp = NULL;
    int     max_dim = x_in_vp->length;
    int     i;


    for (i = 0; i < max_dim; i++)
    {
        if (var_in_vp->elements[ i ] > 0.0)
        {
            dim++;
        }
    }

    ERE(get_target_vector(&x_vp, dim));
    ERE(get_target_vector(&u_vp, dim));
    ERE(get_target_vector(&var_vp, dim));

    dim = 0;

    for (i = 0; i < max_dim; i++)
    {
        if (var_in_vp->elements[ i ] > 0.0)
        {
            x_vp->elements[ dim ] = x_in_vp->elements[ i ];
            u_vp->elements[ dim ] = u_in_vp->elements[ i ];
            var_vp->elements[ dim ] = var_in_vp->elements[ i ];

            dim++;
        }
    }

    result = subtract_vectors(&d_vp, x_vp, u_vp);

    if (result != ERROR)
    {
        result = divide_vectors(&temp_vp, d_vp, var_vp);
    }

    if (result != ERROR)
    {
        result = ow_multiply_vectors(temp_vp, d_vp);
    }

    if ((result != ERROR) && (max_norm_var > 0.0))
    {
        result = ow_max_thresh_vector(temp_vp, max_norm_var);
    }

    if (result != ERROR)
    {
        dist_sqrd = sum_vector_elements(temp_vp);

        if (dist_sqrd < 0.0)
        {
            db_rv(var_vp);
        }

        ASSERT(dist_sqrd >= 0.0);
    }

    free_vector(temp_vp);
    free_vector(d_vp);
    free_vector(x_vp);
    free_vector(u_vp);
    free_vector(var_vp);

    if (result != ERROR)
    {
        *dist_sqrd_ptr = dist_sqrd;
    }

    if (result == ERROR)
    {
        return ERROR;
    }
    else
    {
        return dim;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int compute_mean_and_stdev(double* mean_ptr,
                           double* stdev_ptr,
                           int num_data_points,
                           double data_sum,
                           double data_sqrd_sum)
{
    double temp;


    if (mean_ptr != NULL)
    {
        if (num_data_points < 1)
        {
            set_error("Attempt to compute mean of %d data points.",
                      num_data_points);
            return ERROR;
        }

        ASSERT_IS_NUMBER_DBL(data_sum);
        ASSERT_IS_FINITE_DBL(data_sum);

        *mean_ptr = data_sum / (double)num_data_points;
    }

    if (stdev_ptr != NULL)
    {
        if (num_data_points < 2)
        {
            set_error("Attempt to compute stdev with %d data points.",
                      num_data_points);
            return ERROR;
        }

        ASSERT_IS_NUMBER_DBL(data_sum);
        ASSERT_IS_FINITE_DBL(data_sum);
        ASSERT_IS_NUMBER_DBL(data_sqrd_sum);
        ASSERT_IS_FINITE_DBL(data_sqrd_sum);

        temp = data_sqrd_sum;
        temp -= data_sum * data_sum / (double)num_data_points;
        temp /= (num_data_points - 1.0);

        ASSERT_IS_NUMBER_DBL(temp);
        ASSERT_IS_FINITE_DBL(temp);

        if (temp < 0.0)
        {
            if (ABS_OF(temp / data_sqrd_sum) > 1000.0 * DBL_EPSILON)
            {
                warn_pso("Possible precision or other problem.\n");
                dbe(data_sum);
                dbe(data_sqrd_sum);
                dbi(num_data_points);
                dbe(temp);
            }

            *stdev_ptr = 0.0;
        }
        else
        {
            *stdev_ptr = sqrt(temp);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// We ignore degrees of freedom for now. The correct formula was derived for
// CS696i (FIX)
*/
int compute_weighted_mean_and_stdev(double* mean_ptr, double* stdev_ptr,
                                    double total_weight, double data_sum,
                                    double data_sqrd_sum)
{
    double temp;
    int result = NO_ERROR;


    if (total_weight < 0.0)
    {
        set_error("Total weight is negative in compute_weighted_mean_and_stdev.");
        result = ERROR;
    }
    else if (total_weight == 0.0)
    {
        set_error("Total weight is zero in compute_weighted_mean_and_stdev.");
        result = ERROR;
    }
    else if (data_sqrd_sum < 0.0)
    {
        set_error("Sum of squares is negative in compute_weighted_mean_and_stdev.");
        result = ERROR;
    }

    if (result == ERROR)
    {
        if (mean_ptr != NULL)
        {
            *mean_ptr = DBL_NOT_SET;
        }
        if (stdev_ptr != NULL)
        {
            *stdev_ptr = DBL_NOT_SET;
        }

        return result;
    }

    if (mean_ptr != NULL)
    {
        *mean_ptr = data_sum / total_weight;
    }

    if (stdev_ptr != NULL)
    {
        ASSERT_IS_NUMBER_DBL(data_sum);
        ASSERT_IS_FINITE_DBL(data_sum);
        ASSERT_IS_NUMBER_DBL(data_sqrd_sum);
        ASSERT_IS_FINITE_DBL(data_sqrd_sum);

        temp = data_sqrd_sum;
        temp -= data_sum * data_sum / total_weight;
        temp /= total_weight;

        ASSERT_IS_NUMBER_DBL(temp);
        ASSERT_IS_FINITE_DBL(temp);

        if (temp < 0.0)
        {
            if (ABS_OF(temp / data_sqrd_sum) > 1000.0 * DBL_EPSILON)
            {
                warn_pso("Possible precision or other problem.\n");
                dbe(data_sum);
                dbe(data_sqrd_sum);
                dbe(total_weight);
                dbe(temp);
            }

            *stdev_ptr = 0.0;
        }
        else
        {
            *stdev_ptr = sqrt(temp);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_entropy(const Vector* prob_vp, double* result_ptr)
{
    double entropy_sum = 0.0;
    double p_sum = 0.0;
    double p;
    int i;

    if (prob_vp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (prob_vp->length == 0)
    {
        *result_ptr = 0.0;
        return NO_ERROR;
    }

    for (i = 0; i < prob_vp->length; i++)
    {
        p = prob_vp->elements[ i ];

        if ((p < 0.0) || (p > ADD_DBL_EPSILON(1.0)))
        {
            set_error("Attempt to compute entropy of a non-probability vector.");
            add_error("Offending element is %.3e.", p);
            return ERROR;
        }

        if (p > 0.0)
        {
            p_sum += p;
            entropy_sum -= p * kjb_log2(p);
        }
    }

    if ( ! (IS_NEARLY_EQUAL_DBL(p_sum, 1.0 , 10.0 * prob_vp->length * DBL_EPSILON)) )
    {
        set_error("Attempt to compute entropy of a non-probability vector.");
        add_error("Sum of elements is %.5e. (sum - 1.0 is %.3e).", p_sum - 1.0);
        return ERROR;
    }

    *result_ptr = entropy_sum;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_cumulative_distribution(Vector** cum_vpp, const Vector* vp)
{
    int len;
    int i;
    const Vector* cum_vp; 

    
    /* Corner case, handled consistent with library convenctions. */
    if (vp == NULL)
    {
        if (cum_vpp != NULL)
        {
            free_vector(*cum_vpp);
            *cum_vpp = NULL; 
            return NO_ERROR;
        }
    }

    len = vp->length; 

    ERE(get_target_vector(cum_vpp, len)); 

    /* Corner case, handled consistent with library convenctions. */
    if (len == 0)
    {
        return NO_ERROR;
    }

    cum_vp = *cum_vpp;

    cum_vp->elements[ 0 ] = vp->elements[ 0 ];

    for (i = 1; i < len; i++)
    {
        cum_vp->elements[ i ] = cum_vp->elements[ i-1 ] + vp->elements[ i ];
    }

    if (cum_vp->elements[ len - 1 ] > 1.0)
    {
        if (cum_vp->elements[ len - 1 ] < 1.0 + len * DBL_EPSILON)
        {
            /*
            dbw();
            dbe(cum_vp->elements[ len - 1 ] - 1.0);
            */
            cum_vp->elements[ len - 1 ] = 1.0;
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int sample_distribution_using_cumulative(const Vector* cum_dist_vp)
{
    int    last        = cum_dist_vp->length - 1;
    int    first       = 0;
    double r           = kjb_rand();
    int    mid;
    int    count       = 0;
    int    max_num_its = kjb_rint(100.0 + log((double)cum_dist_vp->length) / M_LN2);


#ifdef TEST_CORNER_CASES
    if (kjb_rand() < 0.1)
    {
        int index = MIN_OF(cum_dist_vp->length, (int)(kjb_rand() * (double)cum_dist_vp->length));

        r = cum_dist_vp->elements[ index ]; 
    }
#endif 

    /*
     * Binary search. We don't use kjb search routines because they are not
     * quite tuned for this situation because an exact match is not expected.
    */
    while (count < max_num_its)
    {
        if (first == last) 
        {
            ASSERT(r <= cum_dist_vp->elements[ first ]);
            
            if (first < 0)
            {
                ASSERT(r > cum_dist_vp->elements[ first - 1 ]);
            }
            return first; 
        }
        
        ASSERT(last > first); 

        mid = (first + last) / 2; 

        if (r <= cum_dist_vp->elements[ mid ])
        {
            last = mid;
        }
        else if (mid < cum_dist_vp->length - 1)
        {
            first = mid + 1;
        }
        else 
        {
            first = mid; 
        }

        count++; 
    }

    SET_CANT_HAPPEN_BUG();
    return ERROR;
}

#ifdef __cplusplus
}
#endif

