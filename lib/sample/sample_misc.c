
/* $Id: sample_misc.c 21596 2017-07-30 23:33:36Z kobus $ */

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
#include "sample/sample_misc.h"
#include "l/l_sys_rand.h"
#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GAMMA_MIN_LOG_ARG 4.47628e-309

static double sample_gamma_pdf_small_alpha(double alpha, double beta);
static double sample_gamma_pdf_large_alpha(double alpha, double beta);
/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                          kjb_rand_int
 *
 * Samples random integer
 *
 * This function uniformly samples a random integer between lower bound lb
 * and upper bound ub, inclusive.
 * If lb exceeds ub, this calls the bug handler and (if that returns)
 * returns ERROR.
 *
 * Returns:
 *    Returns a random integer, provided ub >= lb, otherwise ERROR.
 *
 * Related:
 *    kjb_rand
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int kjb_rand_int(int lb, int ub)
{
    int k = (int)(kjb_rand() * (1 + ub - lb));

    if (lb > ub)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (k == 1 + ub - lb)
    {
        /* Only occurs when kjb_rand() returns 1 -- almost never! */
        return ub;
    }

    return k + lb;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          sample_from_discrete_distribution
 *
 * Samples from a one-dimensional discrete distribution
 *
 * This funciton generates a sample from a 1D discrete distribution, given by
 * the vector dist. The index of the sample is given by *idx. The elements
 * of dist must sum to 1.
 *
 * Returns:
 *    ERROR on failure, NO_ERROR on success.
 *
 * Related:
 *    kjb_rand
 *
 * Author:
 *    Ernesto Brau
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/

int sample_from_discrete_distribution(int *idx, const Vector* dist)
{
    double cdf = 0;
    double u = kjb_rand();
    const double DISTRIBUTION_EPSILON = 0.00001;

    int i;

    if(dist == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* if(sum_vector_elements(dist) <= 0.9999 || sum_vector_elements(dist) >= 1.0001) */
    if(fabs(sum_vector_elements(dist) - 1.0) > DISTRIBUTION_EPSILON)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 0; i < dist->length; i++)
    {
        cdf += dist->elements[i];
        if(u <= cdf)
        {
            *idx = i;
            break;
        }
    }
    
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          sample_from_uniform_distribution
 *
 * Samples from a one-dimensional uniform distribution over the inverval [a,b]
 *
 * Does not seed the rand() function. If this is desired, srand() must
 * be called prior to calling this function.
 *
 * Returns:
 *    the sampled value
 *
 * Related:
 *    kjb_rand
 *
 * Author:
 *    Luca Del Pero
 *
 * Index: random
 *
 * -----------------------------------------------------------------------------
*/
double sample_from_uniform_distribution(double a, double b)
{
    if (a == b)
    {
        return a;
    }

    return (b - a)*kjb_rand() + a;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               poisson_rand
 *
 * Returns a Poisson-distributed random number
 *
 * This routine returns a Poisson-distributed random integer,
 * with parameter lambda. The random stream from kjb_rand() is used.
 *
 * Returns:
 *    A Poisson-distributed integer. If lambda is negative, it 
 *    calls the bug handler and (if that returns), returns ERROR.
 *
 * We advise you to use another algorithm if lambda is large;
 * perhaps use a normal approximation with mean = variance = lambda.
 *
 * This implements Knuth's algorithm in TAOCP vol. 2.
 *
 * Author:
 *   Ernesto Brau
 *
 * Index: random
 *
 * Related:
 *     kjb_rand
 *
 * -----------------------------------------------------------------------------
*/

int poisson_rand(double lambda)
{
    double L = exp(-lambda); /* underflow trouble if lambda is big. */
    int k = 0;
    double p = 1.0;

    if (lambda <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    do
    {
        k += 1;
        p *= kjb_rand();
    }
    while (p > L);

    return k - 1;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               gamma_pdf
 *
 * Returns the value of f(x) where f is a gamma distribution
 *
 * Returns the value of f(x) where f is a gamma distribution with shape
 * alpha, scale beta, computed in x.
 *
 * Returns:
 *   The value of f(x)
 *
 * Author:
 *   Luca Del Pero
 *
 * Index: random
 *
 * Related:
 *     gaussian_pdf
 *
 * -----------------------------------------------------------------------------
*/
double gamma_pdf(double x, double alpha, double beta)
{
    ASSERT(x > 1.0e-16);
    ASSERT(alpha > 1.0e-16);
    ASSERT(beta > 1.0e-16);

    /* Temporary patch, before we can figure oout how to use tgamma
     * TODO fix it! 
     */
    return pow(beta, alpha) / exp(lgamma(alpha)) * pow(x, alpha - 1) * exp(-beta*x);
    /* return pow(beta, alpha) / tgamma(alpha) * pow(x, alpha - 1) * exp(-beta*x); */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               log_gamma_pdf
 *
 * Returns the log of the value of f(x) where f is a gamma distribution
 *
 * Returns the log of the value of f(x) where f is a gamma distribution with shape
 * alpha, scale beta, computed in x.
 *
 * Returns:
 *   The log of the value of f(x)
 *
 * Author:
 *   Luca Del Pero
 *
 * Index: random
 *
 * Related:
 *     gamma_pdf
 *
 * -----------------------------------------------------------------------------
*/
double log_gamma_pdf(double x, double alpha, double beta)
{
    ASSERT(x > 1.0e-16);
    ASSERT(alpha > 1.0e-16);
    ASSERT(beta > 1.0e-16);

    return alpha*log(beta) - lgamma(alpha) + (alpha - 1)*log(x) - beta*x;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               sample_from_gamma_distribution
 *
 * Samples from a gaussian distribution
 *
 * Samples from  a gamma distribution with shape alpha, and scale beta
 * over the sampling interval [a,b].
 *
 * Note:
 *   This approach uses rejection sampling with a uniform distribution as the
 *   model distribution.  As such, running time depends heavilly on choice of
 *   a,b and the peakiness of the distribution (i.e. size of alpha).  For 
 *   distributions with sharp peaks (large alpha),  
 *   sample_from_gamma_distribution_2() is recommended, as it makes guarantees
 *   on the expected running time.
 *
 * Documentor:
 *   Luca Del Pero, Kyle Simek
 *
 * Returns:
 *   The sampled value
 *
 * Author:
 *   Luca Del Pero
 *
 * Index: random
 *
 * Related:
 *     gamma_pdf
 *
 * -----------------------------------------------------------------------------
*/
double sample_from_gamma_distribution
(
    double alpha,
    double beta,
    double a,
    double b
)
{
    int accept;
    double s, l, u;
    double max;

    if(alpha < 20 && beta < 20)
    {
        max = (alpha > 1) ? gamma_pdf((alpha - 1)/beta, alpha, beta) : 1;
    }
    else
    {
        /* for large alpha and beta, intermediate computations are
        more robust in the log space. */
        max = exp(log_gamma_pdf( (alpha - 1)/beta, alpha, beta));
    }

    ASSERT(GAMMA_MIN_LOG_ARG <= max);
    accept = 0;

    while (!accept)
    {
        s = sample_from_uniform_distribution(a, b);
        l = log_gamma_pdf(s, alpha, beta);
        u = log(sample_from_uniform_distribution(GAMMA_MIN_LOG_ARG, max));

        if (u <= l)
        {
            accept = 1;
        }
    }

    return s;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double sample_gamma_pdf_small_alpha(double alpha, double beta)
{
    /* method for sampling from gamma distribution with small alpha */
    /* see wikipedia: Gamma Distribution, c. Sept 15, 2009 */
    double x;

    while(alpha >= 1)
    {
        double u;
        u = sample_from_uniform_distribution(0, 1);
        x -= log(u);

        alpha--;
    }

    /* Kobus: 16/10/10: Changed abs()->fabs() and FLT_EPSILON to 5.0 *
     * DBL_EPSILON, although the corect semantics that we want are not precisely
     * clear.  */

    /* if alpha was a whole number, we're done */
    if(fabs(alpha) < 5.0 * DBL_EPSILON) return x;

    {
        double xi, eta;

        double e = exp(1);
        double v0 = e / (e + alpha);

        double alpha_inv = 1 / alpha;
        double exp_neg_xi, pow_xi_alpha;

        alpha -= 1;
        while(1)
        {
            double v1 = sample_from_uniform_distribution(0, 1);
            double v2 = sample_from_uniform_distribution(0, 1);
            double v3 = sample_from_uniform_distribution(0, 1);

            if(v1 <= v0)
            {
                xi = pow(v2, alpha_inv);

                pow_xi_alpha = pow(xi, alpha);
                exp_neg_xi = exp(-xi);
                eta = v3 * pow_xi_alpha;
            }
            else
            {
                xi = 1 - log(v2);
                pow_xi_alpha = pow(xi, alpha);
                exp_neg_xi = exp(-xi);
                eta = v3 * exp_neg_xi;
            }

            if(eta <= pow_xi_alpha * exp_neg_xi)
            {
                return (x + xi) / beta;
            }
        }
    }
}

double sample_gamma_pdf_large_alpha(double alpha, double beta)
{
    /* Rejection algorithm using clever proposal distribtuion. */
    /* */
    /* Algorithm from Knuth vol 2. */
    /* */
    /* Loop will execute 1.9 times on average for alpha > 3. */
    while(TRUE)
    {
        double u = sample_from_uniform_distribution(0,1);
        double y = tan(M_PI * u);
        double x = sqrt(2 * alpha - 1) * y + alpha - 1;

        if(x <= 0) continue;

        {
            double compare;
            double v;

            compare = (1 + y * y) * 
                exp(
                    (alpha - 1) * log(x / (alpha - 1)) 
                    - sqrt(2 * alpha - 1) * y
            );

            v = sample_from_uniform_distribution(0,1);


            if(v <= compare) return x / beta;
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* =============================================================================
 *                               sample_from_gamma_distribution_2
 *
 * Samples from a gamma distribution
 *
 * Samples from  a gamma distribution with shape alpha, and scale beta.  This 
 * version chooses between two different implementations depending on the value
 * of the alpha parameter.  Both implementations use rejection sampling, but 
 * the expected number of iterations is always less than 2 for any value of alpha.
 *
 * Note:
 *  For alpha > 2, the algorithm used is that of Knuth's AoCP, vol. 2.  It's
 *  unclear what the license is on this, as it isn't mentioned in the text.
 *
 * Returns:
 *   The sampled value
 *
 * Author:
 *   Kyle Simek
 *
 * Index: random
 *
 * Related:
 *     gamma_pdf
 *
 * -----------------------------------------------------------------------------
*/
double sample_from_gamma_distribution_2(double alpha, double beta)
{
    if(alpha <= 2 || alpha == 3)
    {
        /* this is only faster for a very small number of cases */
        return sample_gamma_pdf_small_alpha(alpha, beta);
    }

    return sample_gamma_pdf_large_alpha(alpha, beta);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/* =============================================================================
 *                              pick_m_from_n
 *
 * Picks m values in the interval [0,n-1] without repetitions
 *
 * Returns:
 *   NO_ERROR in case of success, ERROR otherwise
 *
 * Author:
 *   Luca Del Pero
 *
 * Index: random
 *
 * Related:
 *
 * -----------------------------------------------------------------------------
*/
int pick_m_from_n(Int_vector ** m_indexes, int m, int n)
{
    Int_vector * used = NULL;
    int  i,j,value, count= 0;
    if(m > n)
    {
        add_error("Cannot choose m values, m > n");
    }

    ERE(get_target_int_vector(m_indexes,m));
    ERE(get_target_int_vector(&used,n));

    for(i = 0; i < m; i++)
    {
        used->elements[i] = 0;
    }

    for(i = 0; i < m; i++)
    {
        value = kjb_rand_int(0, n-i-1);
        count = 0;
        for(j=0; j < n; j++)
        {
            if(used->elements[j] == 1)
            {
                continue;
            }
            if(count == value)
            {
                used->elements[j] = 1;
                (*m_indexes)->elements[i] = j;
                break;
            }
            else
            {
                count++;
            }
        }

    }

    free_int_vector(used);
    return NO_ERROR;


}

#ifdef __cplusplus
}
#endif

