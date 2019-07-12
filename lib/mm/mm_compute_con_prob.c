
/* $Id: mm_compute_con_prob.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|
| Copyright (c) 2000-2008 by Kobus Barnard (author), UCB, and UA. Currently,
| the use of this code is retricted to the UA and UCB image and text projects.
|
* =========================================================================== */

/* Lazy */
#include "mm/mm_incl.h" 

#ifndef __C2MAN__

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/*
 * If we implement feature weighting (or grouping for that matter, options and
 * comments and warnings in data.c should be updated. See set_data_options() and
 * weight_features().
*/

int compute_con_prob
(
    const Int_vector* con_feature_enable_vp,
    const Int_vector* con_feature_group_vp,
    const double* x_ptr,
    const double* u_ptr,
    const double* v_ptr,
    double  normalizing_factor,
    int     norm_continuous_features,
    int     limit_gaussian_deviation,
    int     max_con_norm_var,
    const int*  vector_feature_counts_ptr,
    double* log_prob_ptr,
    double* max_con_log_prob_ptr,
    int enable_missing_data
)
{
    const int num_con_features = con_feature_enable_vp->length;
    int       con_feature      = 0;
    int       con_feature_count = 0;  /* Count enabled features */
    double    d = 0.0;
    int*      con_feature_enable_ptr = NULL;
    int*      con_feature_group_ptr  = NULL;
    double    x, u, v, e, dev, temp;
    double log_prob = 0.0;
    int ind_feature_count = 0;
    int missing_count = 0;

    /* Untested since switch to having missing data handled by a paramter, which
     * makes more sense, and enabled testing compiling headers. 
    */
    UNTESTED_CODE();

    ASSERT_IS_NUMBER_DBL(normalizing_factor);
    ASSERT_IS_FINITE_DBL(normalizing_factor);

    con_feature_enable_ptr = con_feature_enable_vp->elements;
    con_feature_group_ptr = con_feature_group_vp->elements;
 
    while (con_feature < num_con_features)
    {
        /* Made pedantic to track bug. */
        /* Bug found--it was in the Red Hat 6.1 OS !!! */

        if ( ! (*con_feature_enable_ptr) )
        {
            con_feature++;
            con_feature_enable_ptr++;
            con_feature_group_ptr++;
            u_ptr++;
            v_ptr++;
            x_ptr++;
            continue;
        }

        v = *v_ptr;

        if (v < 0.0)
        {
            static int first_time = TRUE;

            if (first_time)
            {
                warn_pso("A negative variance is being interpreted as a requested feature skip.\n");
                warn_pso("Only the first occurance is being reported.\n");
                dbw();
                dbp("\n");

                first_time = FALSE;
            }

            u_ptr++;
            v_ptr++;
            x_ptr++;
            con_feature++;
            con_feature_enable_ptr++;
            con_feature_group_ptr++;
            continue;
        }

        /* Slow, just to try it. */
        if (*con_feature_group_ptr > 0)
        {
            int i;
            int count = 0;
            double sum = 0.0;
            /*
            double sum_sqr_1 = 0.0;
            double sum_sqr_2 = 0.0;
            */
            static int first_time = TRUE;


            UNTESTED_CODE();

            ASSERT_IS_NON_NEGATIVE_INT(num_con_features - con_feature);
            for (i = 0; i < num_con_features - con_feature; i++)
            {
                if (*con_feature_group_ptr != *(con_feature_group_ptr + i))
                {
                    break;
                }
                count++;
            }

            ASSERT_IS_POSITIVE_INT(count);

            for (i = 0; i < count; i++)
            {
                /* We have at least one good one, but there may be holes in the
                // vector (unlikely).
                */

                if (*con_feature_enable_ptr)
                {
                    /*
                     * Normally we would check respect_missing_values(), but we would only
                     * be here if it were true.
                    */
                    if ((enable_missing_data) && (IS_MISSING_DBL(*x_ptr)))
                    {
                        UNTESTED_CODE();  /* Un-written in fact! */
                        kjb_abort();
                    }

                    sum += (*x_ptr) * (*u_ptr);
                    /*
                    sum_sqr_1 += (*x_ptr) * (*x_ptr);
                    sum_sqr_2 += (*u_ptr) * (*u_ptr);
                    */
                }

                u_ptr++;
                v_ptr++;
                x_ptr++;
                con_feature++;
                con_feature_enable_ptr++;
                con_feature_group_ptr++;
                con_feature_count++;
            }

            log_prob += sum;
            /*
            log_prob -= log(sum);
            log_prob += 0.5 * log(sum_sqr_1);
            log_prob += 0.5 * log(sum_sqr_2);
            */

            if (first_time)
            {
                p_stderr("\nFirst vector feature had %d components.\n\n",
                         count);
                dbi(num_con_features);
                first_time = FALSE;
            }

            continue;
        }
#ifdef XXX_CURRENTLY_WE_ONLY_RE_WEIGHT
        else if (*con_feature_group_ptr > 0)
        {
            /* This group is likely dependent, but currently we have no
            // mechanism for dealing with it other than re-weighting and tying
            // variance.
            */
        }
#endif

        ASSERT_IS_NUMBER_DBL(v);
        ASSERT_IS_FINITE_DBL(v);
        ASSERT_IS_NON_NEGATIVE_DBL(v);
        ASSERT_IS_NOT_LESS_DBL(v, 10.0 * DBL_MIN);

        u = *u_ptr;

        ASSERT_IS_NUMBER_DBL(u);
        ASSERT_IS_FINITE_DBL(u);

        x = *x_ptr;

        ASSERT_IS_NUMBER_DBL(x);
        ASSERT_IS_FINITE_DBL(x);

        /*
         * Normally we would check respect_missing_values(), but we would only
         * be here if it were true.
        */
        if ((enable_missing_data) && (IS_MISSING_DBL(x)))
        {
            missing_count++;
        }
        else
        {
            dev = u - x;

            ASSERT_IS_NUMBER_DBL(dev);
            ASSERT_IS_FINITE_DBL(dev);

            temp = (dev / v) * dev;

            ASSERT_IS_NUMBER_DBL(temp);
            ASSERT_IS_FINITE_DBL(temp);
            ASSERT_IS_NON_NEGATIVE_DBL(temp);

            if (    (limit_gaussian_deviation >= 0.0)
                 && (temp > limit_gaussian_deviation)
               )
            {
                temp = limit_gaussian_deviation;
            }

            if (max_con_norm_var > 0.0)
            {
                dbw();
                temp = MIN_OF(temp, max_con_norm_var);
            }

            if (    (vector_feature_counts_ptr != NULL)
                 && (vector_feature_counts_ptr[ con_feature ] > 0)
               )
            {
                dbw();
                temp /= vector_feature_counts_ptr[ con_feature ];
            }

            d += temp;

            con_feature_count++;
            ind_feature_count++;
        }

        ASSERT_IS_NUMBER_DBL(d);
        ASSERT_IS_FINITE_DBL(d);

        u_ptr++;
        v_ptr++;
        x_ptr++;
        con_feature++;
        con_feature_enable_ptr++;
        con_feature_group_ptr++;

    }

    if (enable_missing_data)
    {
        if (missing_count == ind_feature_count)
        {
            /*
             * If all items are missing, then d is 0. This is OK as it should just
             * lead to a uniform distribution due to continous data.
            */
            warn_pso("Data item with all missing values spotted.\n");
        }
        else if (missing_count > 0)
        {
#ifdef REALLY_TEST
            dbp("------------------------------");
            dbi(missing_count);
            dbe(d);
#endif
            d *= ind_feature_count;
            d /= (ind_feature_count - missing_count);
#ifdef REALLY_TEST
            dbe(d);
            dbp("------------------------------");
#endif
        }
#ifdef TEST_XXX
        else
        {
            /* We should only be here if there is at least one missing value. */
            SET_CANT_HAPPEN_BUG();
        }
#endif
    }
    else
    {
        if (con_feature_count <= 0)
        {
            set_error("No features are enabled.");
            return ERROR;
        }
    }

    if (ind_feature_count > 0)
    {
        ASSERT_IS_NUMBER_DBL(d);
        ASSERT_IS_FINITE_DBL(d);

        e = -0.5 * d;

        ASSERT_IS_NUMBER_DBL(e);
        ASSERT_IS_FINITE_DBL(e);

        e -= normalizing_factor;

#ifdef XXX_TRY_IT_DIFFERENT
        e += extra_log_con_prob_for_original_data_space;
#endif

#ifdef XXX_LIMIT_MIN_CON_PROB
        /*
        // FIX
        //
        // Won't compile, but we may never need it!
        //
        // Figure out how likely it is that we would ever want this again!
        */

        if (num_g_probs < max_num_g_probs)
        {
            g_prob_vp->elements[ num_g_probs ] = e + con_log_norm;
            num_g_probs++;
        }
        else
        {
            warn_pso("Max num G probs reached!!!\n");
        }

        if (e + con_log_norm < min_con_log_prob)
        {
            num_g_prob_resets++;
            e = min_con_log_prob - con_log_norm;
        }
#endif

        ASSERT_IS_NUMBER_DBL(e);
        ASSERT_IS_FINITE_DBL(e);

        ASSERT_IS_NUMBER_DBL(log_prob);
        ASSERT_IS_FINITE_DBL(log_prob);

        /*
         * For now, we don't use the above, so, for regression testing, log_prob
         * should be 0.
        */
        ASSERT_IS_EQUAL_DBL(log_prob, 0.0);
        log_prob += e;
    }

    ASSERT_IS_NUMBER_DBL(log_prob);
    ASSERT_IS_FINITE_DBL(log_prob);

    if (    (max_con_log_prob_ptr != NULL)
         && (log_prob > *max_con_log_prob_ptr)
       )
    {
        *max_con_log_prob_ptr = log_prob;
    }

    /*
     * We used to apply this to the exp() and const parts of the Gaussian
     * separately, but this makes more sense. However, this means that we are
     * incompatable with old determinant vectors (of which there are very few
     * anyway since we don't use this feature much.
    */
    if ((norm_continuous_features) && (con_feature_count > 0))
    {
        log_prob /= con_feature_count;
    }

    *log_prob_ptr = log_prob;

    return NO_ERROR;
}

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif    /* #ifndef __C2MAN__   */

