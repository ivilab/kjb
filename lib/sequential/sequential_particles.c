
/* $Id: sequential_particles.c 10665 2011-09-29 19:51:59Z predoehl $ */

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
#include "sequential/sequential_particles.h"
#include "sample/sample_incl.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int resample(Vector_vector* original_samples, const Vector* distribution);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              SIR_particle_filter
 *
 * Implements a SIR particle filter
 *
 * This routine implements a SIR (sample-importance-resample) particle filter. It
 * generates L samples at each time-step of the process, which it puts in *samples.
 * It uses the prior distribution p(x_k | x_{k-1}) as a proposal, from which it
 * samples using sample_from_prior. It also needs to evaluate the likelihood
 * p(y_k | x_k), which it does by calling likelihood. The context parameters
 * are provided in case the prior or the likelihood depend on extra variables.
 *
 * As usual, *samples is reused if possible and created if needed, according
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

int SIR_particle_filter
(
    V_v_v**              samples,
    Vector_vector**      weights,
    int                  L,
    const Vector_vector* y,
    int                  (*sample_from_prior)(Vector**, const Vector*, const void*),
    const void*          prior_context,
    int                  (*likelihood)(double*, const Vector*, const Vector*, const void*),
    const void*          likelihood_context
)
{
    Vector_vector* x_k;
    Vector_vector* x_k_1;
    Vector* w_k;
    double lh;
    int N;
    int k, l;

    N = y->length;

    get_target_v3(samples, N);
    get_target_vector_vector(weights, N);

    for(k = 0; k < N; k++)
    {
        get_target_vector_vector(&(*samples)->elements[k], L);
        get_target_vector(&(*weights)->elements[k], L);
        x_k = (*samples)->elements[k];
        x_k_1 = k == 0 ? NULL : (*samples)->elements[k - 1];
        w_k = (*weights)->elements[k];

        for(l = 0; l < L; l++)
        {
            sample_from_prior(&x_k->elements[l], k != 0 ? x_k_1->elements[l] : NULL, prior_context);
            likelihood(&lh, y->elements[k], x_k->elements[l], likelihood_context);
            w_k->elements[l] = lh;
        }
        ow_normalize_vector(w_k, NORMALIZE_BY_SUM);

        resample(x_k, w_k);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int resample(Vector_vector* original_samples, const Vector* distribution)
{
    Vector_vector* copy = NULL;
    int L;
    int l;
    int i;

    if(original_samples == NULL || distribution == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    L  = original_samples->length;
    copy_vector_vector(&copy, original_samples);

    for(i = 0; i < L; i++)
    {
        ERE(sample_from_discrete_distribution(&l, distribution));
        copy_vector(&original_samples->elements[i], copy->elements[l]);
    }

    free_vector_vector(copy);

    return NO_ERROR;
}

#ifdef __cplusplus
}
#endif

