
/* $Id: sample_gauss.c 21596 2017-07-30 23:33:36Z kobus $ */

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

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "l/l_math.h"
#include "sample/sample_gauss.h"
#include "sample/sample_misc.h"
#include "n/n_invert.h"
#include "n/n_svd.h"
#include "n/n_cholesky.h"
#include "n/n_det.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
 * #define REGRESS_07_10_21
*/ 
#define GAUSSIAN_NOISE_LUT_SIZE   10000
#define ROOT_ONE_HALF_PIE         1.253314137
#define GAUSSIAN_NOISE_LUT_JUMP   (ROOT_ONE_HALF_PIE / GAUSSIAN_NOISE_LUT_SIZE)
#define GAUSSIAN_NOISE_LUT_RESOLUTION 100.0
#define LOG_SQRT_2_PI_DOUBLE  .91893853320467274178032973640561
#define GAUSSIAN_MIN_LOG_ARG 4.47628e-309
/* -------------------------------------------------------------------------- */

static double* fs_gaussian_distribution_lut = NULL;

/* Will be at least one less than the table size. */
static int   fs_gaussian_distribution_lut_size;

/* -------------------------------------------------------------------------- */

static int initialize_gaussian_distribution_lut(void);

#ifdef TRACK_MEMORY_ALLOCATION
    static void prepare_memory_cleanup(void);
    static void free_gaussian_distribution_lut(void);
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                        get_general_sv_gauss_random_matrix
 *
 * Gets a general single variate Gaussian random matrix
 *
 * This routine gets a matrix of the specified dimensions, and fills it with
 * gaussian random values with the specified mean and standard deviation. The
 * routine kjb_rand() is used as the random number source (via the gauss_rand()
 * function).
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices, random
 *
 * -----------------------------------------------------------------------------
*/

int get_general_sv_gauss_random_matrix(Matrix** mpp, int num_rows, int num_cols, double mean, double stdev)
{


    ERE(get_gauss_random_matrix(mpp, num_rows, num_cols));

    ERE(ow_multiply_matrix_by_scalar(*mpp, stdev));

    ERE(ow_add_scalar_to_matrix(*mpp, mean)); 

    return NO_ERROR;
}


/* =============================================================================
 *                        get_gauss_random_matrix
 *
 * Gets a Gaussian random matrix
 *
 * This routine gets a matrix of the specified dimensions, and fills it with
 * gaussian random values with mean 0.0 and variance 1.0. The routine kjb_rand()
 * is used as the random number source (via the gauss_rand() function).
 *
 * The first argument is the adress of the target matrix. If the target matrix
 * itself is null, then a matrix of the appropriate size is created. If the
 * target matrix is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices, random
 *
 * -----------------------------------------------------------------------------
*/

int get_gauss_random_matrix(Matrix** mpp, int num_rows, int num_cols)
{
    int i, j;


    ERE(get_target_matrix(mpp, num_rows, num_cols));

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            (*mpp)->elements[ i ][ j ] = gauss_rand();
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_gauss_random_matrix_2
 *
 * Gets a Gaussian random matrix
 *
 * This routine is exaclty like get_gauss_random_matrix(), exept that the
 * alternative random stream (e.g. kjb_rand_2()) is used.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices, random
 *
 * -----------------------------------------------------------------------------
*/

int get_gauss_random_matrix_2(Matrix** mpp, int num_rows, int num_cols)
{
    int i, j;


    ERE(get_target_matrix(mpp, num_rows, num_cols));

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            (*mpp)->elements[ i ][ j ] = gauss_rand_2();
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_gauss_random_vector
 *
 * Gets a Gaussian random vector
 *
 * This routine gets a matrix of the specified length, and fills it with
 * Gaussian random values with mean 0.0 and variance 1.0. The routine kjb_rand()
 * is used for the random stream.
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is null, then a vector  of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: vectors, random
 *
 * -----------------------------------------------------------------------------
*/

int get_gauss_random_vector(Vector** vp, int length)
{
    int i;

    ERE(get_target_vector(vp, length));

    for (i = 0; i < length; i++)
    {
        (*vp)->elements[ i ] = gauss_rand();
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_gauss_random_vector_2
 *
 * Gets a Gaussian random vector
 *
 * This routine is exaclty like get_gauss_random_vector(), exept that the
 * alternative random stream (e.g. kjb_rand_2()) is used.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: vectors, random
 *
 * -----------------------------------------------------------------------------
*/

int get_gauss_random_vector_2(Vector** vp, int length)
{
    int i;

    ERE(get_target_vector(vp, length));

    for (i = 0; i < length; i++)
    {
        (*vp)->elements[ i ] = gauss_rand_2();
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Alternative, non-closed form way, that uses lookup_gauss_rand(). Not
 * recomended, but used for sanity checking. It is also faster than using
 * gauss_rand(), but the loss of purity is not likely warranted.
*/
int get_lookup_gauss_random_vector(Vector** vp, int length)
{
    int i;

    ERE(get_target_vector(vp, length));

    for (i = 0; i < length; i++)
    {
        (*vp)->elements[ i ] = lookup_gauss_rand();
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               gauss_rand
 *
 * Returns a Gaussian distributed, random number
 *
 * This routine returns an approximately  Gaussian distributed, random number,
 * with mean zero and variance one. The random stream from kjb_rand() is used.
 *
 * Polar version of the Box-Muller method with sample caching. Every run
 * of Box-Muller algorithm generates two independent samples. In order to save
 * computation, the second sample is cached in a static variable. It is returned
 * on the next call to the routine, thus saving computation. More efficient and
 * numerically more stable. Details of this method can be found at:
 * http://en.wikipedia.org/wiki/Box-Muller_transform
 * http://www.taygeta.com/random/gaussian.html
 *
 * Index: random, gaussian distribution
 *
 * Related:
 *     kjb_rand, gauss_rand_2
 *
 * Documentor:  Prasad
 *
 * -----------------------------------------------------------------------------
*/

#ifdef REGRESS_07_10_21
/*
 * Use the Box-Muller method. We used the random stream from kjb_rand(). Guard
 * against floating point exceptions.
*/
double gauss_rand()
{
  double theta = M_PI * kjb_rand();
  /*
   *  Get exponantially distributed (lamda=0.5) r_squared
   *  (-1/p)*log(uniform) is distributed as exp(); here p is 1/2.
  */
  double r_squared = -2.0 * SAFE_LOG(kjb_rand());

  if (r_squared <= 0.0)
  {
      return 0.0;
  }
  else
  {
      return sqrt(r_squared) * cos(theta);
  }
}
#else
double gauss_rand()
{
    static int cached = 0;
    double u1, u2;
    double r_squared, s;
    double z1;
    static double z2;

    if (cached == 0)
    {
        do
        {
            u1 = (2.0 * kjb_rand()) - 1.0;
            u2 = (2.0 * kjb_rand()) - 1.0;

            r_squared = (u1 * u1) + (u2 * u2);
        } 
        while ( r_squared >= 1.0 );

        if (r_squared <= 0.0)
        {
            return 0.0;
        }

        s = sqrt((-2.0 * SAFE_LOG(r_squared)) / r_squared);

        z1 = u1 * s;
        z2 = u2 * s;

        cached = 1;

        return z1;
    }
    else
    {
        cached = 0;

        return z2;
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               gauss_rand_2
 *
 * Returns a Gaussian distributed, random number
 *
 * This routine returns an approximately  Gaussian distributed, random number,
 * with mean zero and variance one. The random stream from kjb_rand_2() is used.
 *
 * This routine is exactly the same as gauss_rand(), except that the alternative
 * random stream kjb_rand_2() is used.
 *
 * Index: random, gaussian distribution
 *
 * Related:
 *     kjb_rand, gauss_rand
 *
 * -----------------------------------------------------------------------------
*/

#ifdef REGRESS_07_10_21
/*
 * Use the Box-Muller method. We used the random stream from kjb_rand(). Gaurd
 * against floating point exceptions.
*/
double gauss_rand_2()
{
  double theta = M_PI * kjb_rand_2();
  /*
   *  Get exponantially distributed (lamda=0.5) r_squared
   *  (-1/p)*log(uniform) is distributed as exp(); here p is 1/2.
  */
  double r_squared = -2.0 * SAFE_LOG(kjb_rand_2());

  if (r_squared <= 0.0)
  {
      return 0.0;
  }
  else
  {
      return sqrt(r_squared) * cos(theta);
  }
}
#else
/* Prasad: Polar version of the Box-Muller method with sample caching. Every run
 * of Box-Muller algorithm generates two independent samples. In order to save
 * computation, the second sample is cached in a static variable. It is returned
 * on the next call to the routine, thus saving computation. More efficient and
 * numerically more stable. Details of this method can be found at:
 * http://en.wikipedia.org/wiki/Box-Muller_transform
 * http://www.taygeta.com/random/gaussian.html
*/
double gauss_rand_2()
{
    static int cached = 0;
    double u1, u2;
    double r_squared, s;
    double z1;
    static double z2;

    if (cached == 0)
    {
        do
        {
            u1 = (2.0 * kjb_rand_2()) - 1.0;
            u2 = (2.0 * kjb_rand_2()) - 1.0;

            r_squared = (u1 * u1) + (u2 * u2);
        } 
        while ( r_squared >= 1.0 );

        if (r_squared <= 0.0)
        {
            return 0.0;
        }

        s = sqrt((-2.0 * SAFE_LOG(r_squared)) / r_squared);

        z1 = u1 * s;
        z2 = u2 * s;

        cached = 1;

        return z1;
    }
    else
    {
        cached = 0;

        return z2;
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * I wrote this version before I was aware that there was closed form solution.
 * Keep this for now, mainly for debugging. It is approximately twice as fast as
 * gauss_rand(), but at the expense of quality of the random numbers. It could
 * be useful where extreme speed is necessary, or as a template for sampling a
 * distribution where there is no closed form.
*/
double lookup_gauss_rand(void)
{
    double uniform_random_number;
    double gaussian_random_number;
    int    negative_noise         = FALSE;
    double a, x, y;
    int    index;


    EPETE(initialize_gaussian_distribution_lut());

    /*
    // We use kjb_rand, as opposed to kjb_rand_2, to make this routine part of
    // the first random stream.
    */
    uniform_random_number = kjb_rand();

    if (uniform_random_number >= 0.5)
    {
        negative_noise = TRUE;
        uniform_random_number -= 0.5;
    }

    index = (int)(uniform_random_number * 2.0 * fs_gaussian_distribution_lut_size);

    if ((index < 0) || (index >= fs_gaussian_distribution_lut_size))
    {
        SET_CANT_HAPPEN_BUG();
        kjb_exit(EXIT_FAILURE);
    }

    ASSERT(fs_gaussian_distribution_lut_size < GAUSSIAN_NOISE_LUT_SIZE);

    x = fs_gaussian_distribution_lut[ index ];
    y = fs_gaussian_distribution_lut[ index + 1];
    /*
    // We use kjb_rand, as opposed to kjb_rand_2, to make this routine part of
    // the first random stream.
    */
    a = kjb_rand();

    gaussian_random_number = a * x + (1.0 - a) * y;

    if (negative_noise)
    {
        gaussian_random_number *= -1.0;
    }

    return gaussian_random_number;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_gaussian_distribution_lut(void)
{
    long_double    sum      = 0.0;
    int            count    = 0;
    double         x        = 0.0;
    double         step     = GAUSSIAN_NOISE_LUT_JUMP /
                                                  GAUSSIAN_NOISE_LUT_RESOLUTION;
    double         dist_val = 1.0;
    int            num_its  = 0;


    if (fs_gaussian_distribution_lut != NULL) return NO_ERROR;

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    NRE(fs_gaussian_distribution_lut = DBL_MALLOC(GAUSSIAN_NOISE_LUT_SIZE));

    fs_gaussian_distribution_lut[ 0 ] = 0.0;

    /*CONSTCOND*/
    while (TRUE)
    {
        if ((num_its > 10000) || (count >= GAUSSIAN_NOISE_LUT_SIZE - 1))
        {
            break;
        }

        if (sum > GAUSSIAN_NOISE_LUT_JUMP)
        {
            sum -= GAUSSIAN_NOISE_LUT_JUMP;
            count++;

            fs_gaussian_distribution_lut[ count ] = x;
            step = GAUSSIAN_NOISE_LUT_JUMP / dist_val /
                                                  GAUSSIAN_NOISE_LUT_RESOLUTION;
            num_its = 0;
        }

        x += step;
        dist_val = exp(-x*x/2.0);
        sum += dist_val * step;

        num_its ++;
    }

    fs_gaussian_distribution_lut_size = count;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void prepare_memory_cleanup(void)
{
    static int first_time = TRUE;


    if (first_time)
    {
        add_cleanup_function(free_gaussian_distribution_lut);
        first_time = FALSE;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void free_gaussian_distribution_lut(void)
{
    kjb_free(fs_gaussian_distribution_lut);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              gaussian_rand
 *
 * Samples a gaussian random number
 *
 * This routine generates a normally-distributed number with mean 'mean' and
 * variance 'variance'. This routine uses kjb_rand().
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Related:
 *    gauss_rand
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random, gaussian distribution
 *
 * -----------------------------------------------------------------------------
*/

int gaussian_rand(double* sample, double mean, double variance)
{
    if(sample == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(variance < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(variance == 0)
    {
        *sample = mean;
        return NO_ERROR;
    }

    *sample = mean + (sqrt(variance) * (gauss_rand()));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              mv_std_gaussian_rand
 *
 * Samples a standard gaussian random vector
 *
 * This routine generates a normally-distributed vector of length len with mean
 * zero and identity covariance matrix. This routine uses kjb_rand(). [NOTE: this
 * provides the exact same functionality as get_gauss_random_vector().]
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is NULL, then a vector  of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Related:
 *    gauss_rand
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random, gaussian distribution
 *
 * -----------------------------------------------------------------------------
*/

int mv_std_gaussian_rand(Vector** sample, int len)
{
    int i;

    ERE(get_target_vector(sample, len));

    for (i = 0; i < len; i++)
    {
        (*sample)->elements[i] = gauss_rand();
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              mv_ind_gaussian_rand
 *
 * Samples an element-wise independent gaussian random vector
 *
 * This routine generates a normally-distributed vector of length len with mean
 * mean and covariance matrix diag[vars]. In other words, the elements of the
 * sample are indepenent of each other. This routine uses kjb_rand(). 
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is NULL, then a vector  of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Related:
 *    gauss_rand
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random, gaussian distribution
 *
 * -----------------------------------------------------------------------------
*/

int mv_ind_gaussian_rand(Vector** sample, const Vector* mean, const Vector* vars)
{
    double d;
    int i;

    if(mean->length != vars->length || mean->length == 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector(sample, mean->length));

    for (i = 0; i < mean->length; i++)
    {
        ERE(gaussian_rand(&d, mean->elements[i], vars->elements[i]));
        (*sample)->elements[i] = d;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              mv_gaussian_rand
 *
 * Samples a gaussian random vector
 *
 * This routine generates a normally-distributed vector of with mean
 * 'mean' and covariance matrix 'covar_mat', where 'covar_mat' is positive-definite.
 * For the special case where the covariance is diagonal, use the much faster
 * routine mv_ind_gaussian_rand(). This routine uses kjb_rand(). 
 * 
 * Note: It obtains a standard gaussian vector using mv_std_gaussian_rand and uses
 * the Cholesky decomposition method to convert it into one with the specified
 * mean and covariance matrix. 
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is NULL, then a vector  of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Related:
 *    gauss_rand
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random, gaussian distribution
 *
 * -----------------------------------------------------------------------------
*/

int mv_gaussian_rand(Vector** sample, const Vector* mean, const Matrix* covar_mat)
{
    Vector* stand_gauss = NULL;
    Matrix* L = NULL;

    if(mean == NULL || covar_mat == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(covar_mat->num_rows != covar_mat->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(mean->length != covar_mat->num_rows)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(mv_std_gaussian_rand(&stand_gauss, mean->length));
    if(cholesky_decomposition(&L, covar_mat) == ERROR)
    {
        add_error("Cannot sample from gaussian: covariance matrix meanst be positive-definite.");
        free_vector(stand_gauss);
        return ERROR;
    }

    multiply_matrix_and_vector(sample, L, stand_gauss);
    ow_add_vectors(*sample, mean);

    free_vector(stand_gauss);
    free_matrix(L);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          gaussian_pdf
 *
 * Evaluates the Gaussian pdf
 *
 * This function computes the value of f(x), where f is the Gaussian pdf with 
 * mean 'mean' and variance 'variance'.
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise
 *
 * Related:
 *    mv_std_gaussian_pdf, mv_ind_gaussian_pdf, mv_gaussian_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int gaussian_pdf(double* pdf, double x, double mean, double variance)
{
    if(variance < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(variance == 0)
    {
        if(x == mean)
        {
            *pdf = DBL_MAX;
            return NO_ERROR;
        }
        *pdf = 0;
        return NO_ERROR;
    }

    *pdf = (1.0 / sqrt(variance * 2 * M_PI)) * exp(-(((x - mean) * (x - mean))/(2 * variance)));
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          mv_std_gaussian_pdf
 *
 * Evaluates the multivariate standard Gaussian pdf
 *
 * This function computes the value of f(x), where f is the multivariate Gaussian
 * pdf with zero mean and identity covariance matrix, of dimension len.
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise
 *
 * Related:
 *    gaussian_pdf, mv_ind_gaussian_pdf, mv_gaussian_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int mv_std_gaussian_pdf(double* pdf, const Vector* x)
{
    double local_pdf;
    int i;

    if(x == NULL || x->length <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    *pdf = 1.0;
    for(i = 0; i < x->length; i++)
    {
        gaussian_pdf(&local_pdf, x->elements[i], 0, 1);
        *pdf *= local_pdf;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          mv_ind_gaussian_pdf
 *
 * Evaluates the multivariate element-wise independent Gaussian pdf
 *
 * This function computes the value of f(x), where f is the multivariate Gaussian
 * pdf with mean 'mean' and covariance matrix diag[vars].
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise
 *
 * Related:
 *    gaussian_pdf, mv_std_gaussian_pdf, mv_gaussian_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int mv_ind_gaussian_pdf(double* pdf, const Vector* x, const Vector* mean, const Vector* vars)
{
    double local_pdf;
    int i;

    if(x == NULL || mean == NULL || vars == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(mean->length != vars->length || mean->length != x->length || mean->length <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    *pdf = 1.0;
    for(i = 0; i < x->length; i++)
    {
        gaussian_pdf(&local_pdf, x->elements[i], mean->elements[i], vars->elements[i]);
        *pdf *= local_pdf;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          mv_gaussian_pdf
 *
 * Evaluates the multivariate Gaussian pdf
 *
 * This function computes the value of f(x), where f is the multivariate Gaussian
 * pdf with mean 'mean' and covariance matrix 'covar_mat'.
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise
 *
 * Related:
 *    gaussian_pdf, mv_std_gaussian_pdf, mv_ind_gaussian_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int mv_gaussian_pdf(double* pdf, const Vector* x, const Vector* mean, const Matrix* covar_mat)
{
    Matrix* sigma_inv = NULL;
    Vector* temp_vector = NULL;
    Vector* temp_vector2 = NULL;
    double denom;

    if(x == NULL || mean == NULL || covar_mat == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(covar_mat->num_rows != covar_mat->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(mean->length != covar_mat->num_rows || mean->length != x->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_matrix_inverse(&sigma_inv, covar_mat));
    subtract_vectors(&temp_vector, x, mean);
    multiply_matrix_and_vector(&temp_vector2, sigma_inv, temp_vector);
    get_dot_product(temp_vector, temp_vector2, pdf);
    *pdf = exp(-0.5 * *pdf);

    /* Don't need to check for zero-valued determinant; we were able to invert it above */
    ERE(get_determinant_abs(covar_mat, &denom));
    denom = sqrt(denom);
    denom *= pow(2 * M_PI, x->length / 2.0);
    *pdf *= 1.0 / denom;

    free_matrix(sigma_inv);
    free_vector(temp_vector);
    free_vector(temp_vector2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          gaussian_log_pdf
 *
 * Evaluates the log of the Gaussian pdf
 *
 * This function computes the value of log f(x), where f is the Gaussian pdf with 
 * mean 'mean' and variance 'variance'.
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise
 *
 * Related:
 *    mv_std_gaussian_log_pdf, mv_ind_gaussian_log_pdf, mv_gaussian_log_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int gaussian_log_pdf(double* pdf, double x, double mean, double variance)
{
    if(variance < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(variance == 0)
    {
        if(x == mean)
        {
            *pdf = DBL_MAX;
            return NO_ERROR;
        }
        *pdf = -DBL_MAX;
        return NO_ERROR;
    }

    *pdf = (log(1.0) - log(sqrt(variance * 2 * M_PI))) -(((x - mean) * (x - mean))/(2 * variance));
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          mv_std_gaussian_log_pdf
 *
 * Evaluates the log of the multivariate standard Gaussian pdf
 *
 * This function computes the value of log f(x), where f is the multivariate Gaussian
 * pdf with zero mean and identity covariance matrix, of dimension len.
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise
 *
 * Related:
 *    gaussian_log_pdf, mv_ind_gaussian_log_pdf, mv_gaussian_log_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int mv_std_gaussian_log_pdf(double* pdf, const Vector* x)
{
    double local_pdf;
    int i;

    if(x == NULL || x->length <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    *pdf = 0.0;
    for(i = 0; i < x->length; i++)
    {
        gaussian_log_pdf(&local_pdf, x->elements[i], 0, 1);
        *pdf += local_pdf;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          mv_ind_gaussian_log_pdf
 *
 * Evaluates the log of the multivariate element-wise independent Gaussian pdf
 *
 * This function computes the value of log f(x), where f is the multivariate Gaussian
 * pdf with mean 'mean' and covariance matrix diag[vars].
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise
 *
 * Related:
 *    gaussian_log_pdf, mv_std_gaussian_log_pdf, mv_gaussian_log_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int mv_ind_gaussian_log_pdf(double* pdf, const Vector* x, const Vector* mean, const Vector* vars)
{
    double local_pdf;
    int i;

    if(x == NULL || mean == NULL || vars == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(mean->length != vars->length || mean->length != x->length || mean->length <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    *pdf = 0.0;
    for(i = 0; i < x->length; i++)
    {
        gaussian_log_pdf(&local_pdf, x->elements[i], mean->elements[i], vars->elements[i]);
        *pdf += local_pdf;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          mv_gaussian_log_pdf
 *
 * Evaluates the log of the multivariate Gaussian pdf
 *
 * This function computes the value of log f(x), where f is the multivariate Gaussian
 * pdf with mean 'mean' and covariance matrix 'covar_mat'.
 *
 * Returns:
 *    ERROR on error; NO_ERROR otherwise
 *
 * Related:
 *    gaussian_log_pdf, mv_std_gaussian_log_pdf, mv_ind_gaussian_log_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int mv_gaussian_log_pdf(double* pdf, const Vector* x, const Vector* mean, const Matrix* covar_mat)
{
    Matrix* sigma_inv = NULL;
    Vector* temp_vector = NULL;
    Vector* temp_vector2 = NULL;
    double det;

    if(x == NULL || mean == NULL || covar_mat == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(covar_mat->num_rows != covar_mat->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(mean->length != covar_mat->num_rows || mean->length != x->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_matrix_inverse(&sigma_inv, covar_mat));
    subtract_vectors(&temp_vector, x, mean);
    multiply_matrix_and_vector(&temp_vector2, sigma_inv, temp_vector);
    get_dot_product(temp_vector, temp_vector2, pdf);
    *pdf *= -0.5;

    /* Don't need to check for zero-valued determinant; we were able to invert it above */
    get_log_determinant_of_PD_matrix(covar_mat, &det);
    *pdf += (-0.5 * det);
    *pdf += ((-mean->length / 2.0) * log(2 * M_PI));

    free_matrix(sigma_inv);
    free_vector(temp_vector);
    free_vector(temp_vector2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_general_gauss_random_vector
 *
 * Samples a gaussian random vector
 *
 * This routine generates a normally-distributed vector with mean mu and covariance
 * matrix sigma. It obtains a standard gaussian vector using and uses the Cholesky
 * decomposition method to convert it into one with the specified mean and
 * covariance matrix. 
 *
 * The first argument is the adress of the target vector. If the target vector
 * itself is NULL, then a vector  of the appropriate size is created. If the
 * target vector is the wrong size, it is resized. Finally, if it is the right
 * size, then the storage is recycled, as is.
 *
 * Kobus adds: This routine might eventually get renamed with the "mv_" prefix
 * to be consistant with other routines in this group. 
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Related:
 *    get_gauss_random_vector
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: vectors, random
 *
 * -----------------------------------------------------------------------------
*/

int get_general_gauss_random_vector
(
    Vector**        target_vpp,
    const Vector*   mu,
    const Matrix*   sigma
)
{
    Vector* stand_gauss = NULL;
    Matrix* L = NULL;

    if(mu == NULL || sigma == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(sigma->num_rows != sigma->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(mu->length != sigma->num_rows)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_gauss_random_vector(&stand_gauss, mu->length));
    if(cholesky_decomposition(&L, sigma) == ERROR)
    {
        add_error("Cannot sample from gaussian: covariance matrix must be positive-definite.");
        free_vector(stand_gauss);
        return ERROR;
    }

    multiply_matrix_and_vector(target_vpp, L, stand_gauss);
    ow_add_vectors(*target_vpp, mu);

    free_vector(stand_gauss);
    free_matrix(L);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_density_gaussian
 *
 * Gets the gaussian pdf value for a vector
 *
 * Finds the (mu mean and sigma covariance matrix) gaussian pdf value at X. 
 * sigma must be a square matrix with dimension equal to the length of mu. sigma
 * must be positive definite.
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Related:
 *    get_log_density_gaussian
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int get_density_gaussian
(
    double*         density,
    const Vector*   X,
    const Vector*   mu,
    const Matrix*   sigma
)
{
    Matrix* sigma_inv = NULL;
    Vector* temp_vector = NULL;
    Vector* temp_vector2 = NULL;
    double denom;

    if(X == NULL || mu == NULL || sigma == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(sigma->num_rows != sigma->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(mu->length != sigma->num_rows || mu->length != X->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_matrix_inverse(&sigma_inv, sigma));
    subtract_vectors(&temp_vector, X, mu);
    multiply_matrix_and_vector(&temp_vector2, sigma_inv, temp_vector);
    get_dot_product(temp_vector, temp_vector2, density);
    *density = exp(-0.5 * *density);

    /* Don't need to check for zero-valued determinant; were able to invert it above */
    ERE(get_determinant_abs(sigma, &denom));
    denom = sqrt(denom);
    denom *= pow(2 * M_PI, X->length / 2.0);
    *density *= 1.0 / denom;

    free_matrix(sigma_inv);
    free_vector(temp_vector);
    free_vector(temp_vector2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_log_density_gaussian
 *
 * Gets the log of the gaussian pdf value for a vector
 *
 * Finds the log of the (mu mean and sigma covariance matrix) gaussian pdf value
 * at X. sigma must be a square matrix with dimension equal to the length of mu.
 * sigma must be positive definite.
 *
 * Returns:
 *    Returns ERROR on failure, setting the the errors string accordingly, and 
 *    NO_ERROR otherwise.
 *
 * Related:
 *    get_density_gaussian
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int get_log_density_gaussian
(
    double*         density,
    const Vector*   X,
    const Vector*   mu,
    const Matrix*   sigma
)
{
    Matrix* sigma_inv = NULL;
    Vector* temp_vector = NULL;
    Vector* temp_vector2 = NULL;
    double det;

    if(X == NULL || mu == NULL || sigma == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(sigma->num_rows != sigma->num_cols)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(mu->length != sigma->num_rows || mu->length != X->length)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_matrix_inverse(&sigma_inv, sigma));
    subtract_vectors(&temp_vector, X, mu);
    multiply_matrix_and_vector(&temp_vector2, sigma_inv, temp_vector);
    get_dot_product(temp_vector, temp_vector2, density);
    *density *= -0.5;

    /* Don't need to check for zero-valued determinant; were able to invert it above */
    /* get_determinant_abs(sigma, &det); */
    get_log_determinant_of_PD_matrix(sigma, &det);
    *density += (-0.5 * det);
    *density += ((-mu->length / 2.0) * log(2 * M_PI));

    free_matrix(sigma_inv);
    free_vector(temp_vector);
    free_vector(temp_vector2);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          log_gaussian_pdf
 *
 * Computes Gaussian pdf value
 *
 * This function computes the log of the value of f(x), where f is the Gaussian pdf with
 * mean 'mean' and variance 'variance'.
 *
 * Returns:
 *    Returns the log of the value of f(x).
 *
 * Related:
 *    gaussian_pdf
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/
double log_gaussian_pdf(double x, double mean, double variance)
{
    double log_variance;

    ASSERT(variance > 1.0e-16);

    log_variance = log(variance);

    return - LOG_SQRT_2_PI_DOUBLE - log_variance -
        (0.5*(x - mean)*(x - mean)) / (variance*variance);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          gaussian_rand_with_limits
 *
 * Samples from a Gaussian distribution in an interval
 *
 * Samples from a Gaussian distribution with mean mu and variance sigma
 * The sampling interval is [a,b]. rand() is not seeded
 *
 * Returns:
 *    Returns the gaussian sample.
 *
 * Related:
 *    gaussian_pdf
 *
 * Author:
 *    Luca Del Pero
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int gaussian_rand_with_limits
(
    double* sample,
    double  mu,
    double  sigma,
    double  a,
    double  b
)
{
    int accept;
    double s, l, u;
    double max;

    gaussian_pdf(&max, mu, mu, sigma);
    if(GAUSSIAN_MIN_LOG_ARG > max)
    {
        set_error("gaussian_rand_with_limits: bad sigma");
        return ERROR;
    }

    accept = 0;

    while (!accept)
    {
        s = sample_from_uniform_distribution(a, b);
        gaussian_log_pdf(&l, s, mu, sigma);
        u = log(sample_from_uniform_distribution(GAUSSIAN_MIN_LOG_ARG, max));

        if (u <= l)
        {
            accept = 1;
        }
    }

    *sample = s;
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

