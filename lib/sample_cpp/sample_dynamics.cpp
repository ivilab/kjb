
/* $Id: sample_dynamics.cpp 10705 2011-09-29 19:52:58Z predoehl $ */

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
   Author: Luca Del Pero, Joseph Schlecht
* =========================================================================== */

#include "sample_cpp/sample_dynamics.h"

#include "prob_cpp/prob_sample.h"
#include "prob_cpp/prob_distribution.h"

namespace kjb
{

/* =============================================================================
 *                        stochastic_dynamics
 *
 * Implements stochastic dynamics sampling on a set of parameters.
 * This implementation follows the algorithm described in the 1993 Neal Paper,
 * with leapfrog discretization and stochastic transitions weighted by
 * the stochastic alpha parameter.
 *
 * @param iterations number of stochastic dynamics iterations
 * @param delta_t the step to take in the gradient direction. It can be different for
 *                each parameter we are sampling over
 * @param kick    we use this parameters to reset the momenta to the correct trajectory,
 *                thus eliminating the effect of the stochastic transitions. It specifies
 *                after how many transitions the momenta are to be reset. Not fully tested
 * @param parameters The vector with the initial parameter values
 * @param compute_energy_gradient This callback has to compute the gradient at the current
 *        sample. It takes the current sample as input, and must store the gradient in the
 *        parameter out_gradient. In case ERROR is returned, the execution will terminate.
 * @param accept_sample. This callback optionally checks that the current samples meets
 *        any constraint at higher level (if the constraints are not met, return ERROR to
 *        terminate the execution). This is very useful to avoid wasting time sampling in invalid
 *        in regions of phase space and there are complicated functional constraints between parameters.
 * @param log_sample. This callback allows the user to log the current sample and the current momenta.
 *
 * Returns:
 *     NO_ERROR on success and ERROR on failure This routine will only fail if
 *     storage allocation fails.
 *
 * Index: matrices, random
 *
 * -----------------------------------------------------------------------------
*/

int stochastic_dynamics
(
    unsigned int         iterations,
    const Vector &  delta_t,
    double               alpha,
    unsigned int         kick,
    Vector &        parameters,
    int             (*compute_energy_gradient)(const Vector & parameters, Vector & out_gradient),
    int             (*accept_sample)(const Vector & parameters),
    int             (*log_sample)(const Vector & parameters, const Vector & momenta )

)
{

    using namespace kjb_c;

    unsigned int i, num_params;


    num_params = parameters.get_length();

    kjb::Vector stochastic_momenta(num_params);

    /* We keep track of what would have happened if we had
     * followed the momenta without stochastic transitions,
     * so that we can reset the correct dynamics at any point
     * as specified by the kick parameter
     */
    kjb::Vector momenta(num_params);

    /*
     * We allocate this vectors here for efficiency reasons
     */
    kjb::Vector temp_momenta(num_params);
    kjb::Vector stochastic_transition(num_params);

    double st_alpha = sqrt(1 - alpha*alpha);

    /*This vector will contain the gradient of the energy function, computed
     * at every iteration
     */
    kjb::Vector gradients(num_params);

    //The momenta are randomly initialized
    kjb::MV_gaussian_distribution mv_gauss(num_params);
    stochastic_momenta = kjb::sample(mv_gauss);

    //TODO take actions according to error
    ERE(compute_energy_gradient(parameters, gradients));

    //We perform half an update of the momenta in the gradient direction
    gradients.ew_multiply(delta_t);
    gradients *= 0.5;
    stochastic_momenta.subtract(gradients);

    momenta = stochastic_momenta;

    for (i = 0; i < iterations; i++)
    {
        if (kick > 0 && ((i+1) % kick == 0))
        {
            //Reset if we use kick, don't bother now
            stochastic_momenta = momenta;
        }

        //Store the current momenta for logging
        temp_momenta = stochastic_momenta;

        //Perform a full update of the parameters
        temp_momenta.ew_multiply(delta_t);
        parameters = parameters.add(temp_momenta);

        //Verify that the current sample satisfies the constraints
        //TODO add error checks
        accept_sample(parameters);
        log_sample(parameters, stochastic_momenta);

        //TODO take actions according to error
        ERE(compute_energy_gradient(parameters, gradients));

        /* We update the momenta with a full step here. We
         * can do it as long as we take half a step during the
         * first iteration (see Neal's paper, in the leapfrog
         * discretization section)
         */
        gradients.ew_multiply(delta_t);
        stochastic_momenta.subtract(gradients);
        momenta.subtract(gradients);

        /* We add a stochastic perturbation to the momenta, using equation (5.23)
         * from Neal's paper
         */
        stochastic_momenta *= alpha;
        stochastic_transition =  kjb::sample(mv_gauss);
        stochastic_transition *= st_alpha;
        stochastic_momenta = stochastic_momenta + stochastic_transition;

    }

    return NO_ERROR;

}

}

