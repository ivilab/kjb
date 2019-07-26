/*
 * test program for matrix convolution
 *
 * $Id: mat_convolve.c 21664 2017-08-05 17:53:40Z kobus $
 */

#include <l/l_init.h>
#include <m/m_mat_basic.h>
#include <m/m_mat_metric.h>
#include <m/m_convolve.h>

#define KERNEL_SIZE 21
#define MAT_SIZE 101
#define MARGIN ((MAT_SIZE-KERNEL_SIZE)/2)
#define SIGMA 3

#define FAIL(m)   do { add_error("failure: %s", (m)); return ERROR; } while(0)

/* Test impulse response of gaussian */
static int test1(double x)
{
    double e;
    Matrix *k, *m, *n;
    k = m = n = NULL;
    ERE(get_zero_matrix(&m, MAT_SIZE, MAT_SIZE));
    m -> elements[MAT_SIZE/2][MAT_SIZE/2] = x; /* this is the impulse */
    ERE(get_2D_gaussian_mask(&k, KERNEL_SIZE, SIGMA));
    ERE(gauss_convolve_matrix(&n, m, SIGMA));

    if (NULL == n) FAIL("test 1 n null");
    if (m -> num_rows != n -> num_rows) FAIL("test 1 n rows");
    if (m -> num_cols != n -> num_cols) FAIL("test 1 n cols");

    ERE(ow_multiply_matrix_by_scalar(k, x));
    ERE(ow_copy_matrix_block(m, MARGIN, MARGIN, k, 0, 0, -1, -1));

    e = max_abs_matrix_difference(m, n);
    if (fabs(e) > fabs(x) * 1e-10) FAIL("test 1 error");

    free_matrix(n);
    free_matrix(k);
    free_matrix(m);
    return NO_ERROR;
}

/* Empirically demonstrate that a gaussian convo a gaussian is a gaussian. */
static int test2(double sigma1, double sigma2)
{
    Matrix *m, *n, *p;
    double e, w = 1 + 5 * (sigma1 + sigma2);
    int v1 = 0.5 + w, v2 = v1 + (IS_EVEN(v1) ? 1 : 0);

    dbf(w);
    dbi(v1);
    dbi(v2);

    m = n = p = NULL;

    ERE(get_2D_gaussian_mask(&m, v2, sigma1));
    ERE(gauss_convolve_matrix(&p, m, sigma2));

    if (NULL == p) FAIL("test 2 p null");
    if (m -> num_rows != p -> num_rows) FAIL("test 2 p rows");
    if (m -> num_cols != p -> num_cols) FAIL("test 2 p cols");

    ERE(get_2D_gaussian_mask(&n, v2, sigma1+sigma2));

    e = max_abs_matrix_difference(p, n);
    dbe(e);
    if (fabs(e) > 1e-10) FAIL("test 2 error");

    free_matrix(n);
    free_matrix(p);
    free_matrix(m);
    return NO_ERROR;
}


/* Show that convolution works even when the mask is larger than the input.
 */
static int test3(void)
{
    double avg1, avg2;
    Matrix *m, *n;
    m = n = NULL;
    ERE(get_random_matrix(&m, 100, 2));        /* two columns: very narrow */

    ERE(gauss_convolve_matrix(&n, m, 5));      /* sigma of 5:  much wider  */
    if (m -> num_rows != n -> num_rows) FAIL("test 3 n rows");
    if (m -> num_cols != n -> num_cols) FAIL("test 3 n cols");

    avg1 = average_matrix_elements(m);
    avg2 = average_matrix_elements(n);
    if (fabs(avg2-avg1) > 1e-10) FAIL("test 3 error");

    free_matrix(n);
    free_matrix(m);
    return NO_ERROR;
}


int main(void)
{
    EPETE(kjb_init());

    EPETB(test1(1.0));
    EPETB(test1(-1.0));
    EPETB(test1(100.0));
    EPETB(test1(0.01));
    EPETB(test1(-100.0));
    EPETB(test1(-0.01));

    EPETB(test2(1.0, 1.0));
    EPETB(test2(3.0, 2.0));
    EPETB(test2(10.0, 0.1));

    EPETB(test3());

    kjb_cleanup();

    return EXIT_SUCCESS;
}
