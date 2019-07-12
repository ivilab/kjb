/* Test code for m/m_stat */

#include "m/m_incl.h"

int test_get_correlation_coefficient() 
{
    Vector* a = NULL;
    Vector* b = NULL;

    double correlation;

    EPETE(read_vector(&a, "vector_a.txt"))
    EPETE(read_vector(&b, "vector_b.txt"))
    EPETE(get_correlation_coefficient(a, b, &correlation))

    pso("The correlation is %f.\n", correlation);
    ASSERT_IS_EQUAL_DBL(correlation, 0.99758440784538738)
    return NO_ERROR;
}

int test_get_mahalanobis_distance_sqrd()
{
    Vector* x = NULL;
    Vector* mu = NULL;
    Vector* variance = NULL;

    double dist_sqrd;

    EPETE(read_vector(&x, "vector_x.txt"))
    EPETE(read_vector(&mu, "vector_mu.txt"))
    EPETE(read_vector(&variance, "vector_variance.txt"))
    EPETE(get_malhalanobis_distance_sqrd(x, mu, variance, &dist_sqrd))

    pso("The Mahalanobis distance squared is %f.\n", dist_sqrd);
    ASSERT_IS_EQUAL_DBL(dist_sqrd, 0.26930437635930227)

    return NO_ERROR;
}

int test_get_mahalanobis_distance()
{
    Vector* x = NULL;
    Vector* mu = NULL;
    Vector* variance = NULL;

    double distance;

    EPETE(read_vector(&x, "vector_x.txt"))
    EPETE(read_vector(&mu, "vector_mu.txt"))
    EPETE(read_vector(&variance, "vector_variance.txt"))
    EPETE(get_malhalanobis_distance(x, mu, variance, &distance))

    pso("The Mahalanobis distance is %f.\n", distance);
    ASSERT_IS_EQUAL_DBL(distance, 0.51894544641927653)

    return NO_ERROR;
}

int test_get_log_gaussian_density()
{
    Vector* x = NULL;
    Vector* mu = NULL;
    Vector* variance = NULL;

    double log_gaussian_density;

    EPETE(read_vector(&x, "vector_x.txt"));
    EPETE(read_vector(&mu, "vector_mu.txt"));
    EPETE(read_vector(&variance, "vector_variance.txt"));
    EPETE(get_log_gaussian_density(x, mu, variance,
                                    &log_gaussian_density));

    pso("The logarithm of the gaussian pdf is %f.\n", log_gaussian_density);
    ASSERT_IS_EQUAL_DBL(log_gaussian_density, -1.5392897592431749)

    return NO_ERROR;
   
}

int test_get_gaussian_density()
{
    Vector* x = NULL;
    Vector* mu = NULL;
    Vector* variance = NULL;

    double gaussian_density;

    EPETE(read_vector(&x, "vector_x.txt"));
    EPETE(read_vector(&mu, "vector_mu.txt"));
    EPETE(read_vector(&variance, "vector_variance.txt"));
    EPETE(get_gaussian_density(x, mu, variance, &gaussian_density));

    pso("The value of the gaussian pdf is %f.\n", gaussian_density);
    ASSERT_IS_NEARLY_EQUAL_DBL(gaussian_density, 0.21453341770691653, 10e-14)

    return NO_ERROR;
   
}

int test_get_mahalanobis_distance_sqrd_2()
{
    Vector* x = NULL;
    Vector* mu = NULL;
    Vector* variance = NULL;

    double dist_sqrd;
    double max_norm_var = 0.1;

    EPETE(read_vector(&x, "vector_x.txt"))
    EPETE(read_vector(&mu, "vector_mu.txt"))
    EPETE(read_vector(&variance, "vector_variance.txt"))
    EPETE(get_malhalanobis_distance_sqrd_2(x, mu, variance, max_norm_var,
                &dist_sqrd))

    pso("The Mahalanobis distance squared (using version 2) is %f.\n", dist_sqrd);
    ASSERT_IS_NEARLY_EQUAL_DBL(dist_sqrd, 0.210570137142, 10e-12)

    return NO_ERROR;
}

int main(int argc, char **argv)
{
    EPETE(test_get_correlation_coefficient())
    EPETE(test_get_mahalanobis_distance_sqrd())
    EPETE(test_get_mahalanobis_distance())
    EPETE(test_get_log_gaussian_density())
    EPETE(test_get_gaussian_density())

    EPETE(test_get_mahalanobis_distance_sqrd_2())

    kjb_exit(EXIT_SUCCESS);
}
