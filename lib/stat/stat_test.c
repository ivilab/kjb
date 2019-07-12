
/* $Id: stat_test.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision Group     |
|  (the authors) including                                                     |
|        Kobus Barnard.                                                        |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#include "m/m_incl.h"
#include "stat/stat_test.h"
#include "wrap_gsl/wrap_gsl_rnd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                           welch_t_test_one_sided
 *
 * One-sided Welch's t-test
 *
 * This routine performs a one-sided Welch's t-test. It is safe to use this when
 * a regular t-test is wanted. The Welch's version is slightly better when the
 * statistics of sizes of the two groups are quite different. 
 *
 * This routine provides the t-statistic, the degrees of freedom, the effect
 * size, and the p-value. The p-value is the integral of the lower or upper
 * tail, depending on whether the t-statistic is negative or positive. 
 *
 * Computing the p-value is currently handled by calling a GSL routine. If it is
 * not available, then ERROR is returned, but the values for the t-statistic and
 * the degrees of freedom should be fine. However, if only those two values are
 * needed, then better to use the routine welch_t_statistic(); 
 *
 * Any of the four pointers composing the last four parameters can be NULL.
 * This is interpreted as the calling program not being interested in the
 * result. 
 *
 * Index: statistics
 *
 * -----------------------------------------------------------------------------
*/

int welch_t_test_one_sided
(
    double  m1,
    double  m2,
    double  v1,
    double  v2,
    int     n1,
    int     n2,
    double* t_prime_ptr,
    int*    df_ptr,
    double* effect_size_ptr,
    double* p_value_ptr
)
{

    ERE(welch_t_statistic(m1, m2, v1, v2, n1, n2, t_prime_ptr, df_ptr, effect_size_ptr));

    if (p_value_ptr == NULL)
    {
        return NO_ERROR;
    }

    /*
     * In case the next routine fails, we want the two quantities computed above
     * to be useful regardless. 
    */
    *p_value_ptr = DBL_NOT_SET; 

    if (*t_prime_ptr < 0.0)
    {
        ERE(kjb_cdf_tdist_P(p_value_ptr, *t_prime_ptr, *df_ptr));
    }
    else
    {
        ERE(kjb_cdf_tdist_Q(p_value_ptr, *t_prime_ptr, *df_ptr));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           welch_t_statistic
 *
 * Computes the statistic for Welch's t-test
 *
 * This routine computes the statistic for Welch's t-test. It is safe to use
 * this when a regular t-statistic is wanted. The Welch's version is slightly
 * better when the statistics of sizes of the two groups are quite different. 
 *
 * This routine provides the t-statistic degrees of freedom, and effect size.
 * The pointer to either of this can be NULL. This is interpreted as the calling
 * program not being interested in the corresponding result. 
 *
 * If the p_value is wanted, then the routine welch_t_test_one_sided() can be
 * used. 
 *
 * Index: statistics
 *
 * -----------------------------------------------------------------------------
*/

int welch_t_statistic
(
    double  m1,
    double  m2,
    double  v1,
    double  v2,
    int     n1,
    int     n2,
    double* t_prime_ptr,
    int*    df_ptr,
    double* effect_size_ptr
)
{
    double pooled = (v1/n1) + (v2/n2);
    double d_df;

    if (t_prime_ptr != NULL)
    {
        *t_prime_ptr = (m1 - m2) / sqrt(pooled);
    }

    if (df_ptr != NULL)
    {
        d_df = ceil( (pooled*pooled) / ( (v1*v1/n1/n1/(n1-1)) + (v2*v2/n2/n2/(n2-1)) ));
        *df_ptr = (int)d_df;
    }

    if (effect_size_ptr != NULL)
    {
        double pooled_common_variance = ((n1 - 1) * v1 + (n2 - 1) * v2) / (n1 + n2 - 2);

        *effect_size_ptr = (m1 - m2) / sqrt(pooled_common_variance); 
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif




