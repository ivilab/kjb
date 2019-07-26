
/* $Id: abstract_dynamics.cpp 13172 2012-10-18 22:39:32Z predoehl $ */

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
   Author: Luca Del Pero
* =========================================================================== */

#include "sample_cpp/abstract_dynamics.h"

using namespace kjb;

/**
 * @param src The Abstract_dynamics to copy into this one
 */
Abstract_dynamics::Abstract_dynamics(const Abstract_dynamics & src) :
    parameters(src.parameters),
    deltas(src.deltas),
    gradients(src.gradients),
    stochastic_momenta(src.stochastic_momenta),
    momenta(src.momenta),
    temp_momenta(src.temp_momenta),
    stochastic_transition(src.stochastic_transition),
    alpha(src.alpha),
    st_alpha(src.st_alpha),
    kick(src.kick)
{
    mv_gauss = NULL;
    if(src.mv_gauss != NULL)
    {
        mv_gauss = new kjb::MV_gaussian_distribution(*(src.mv_gauss));
    }
}

/**
 * @param src The Abstract_dynamics to assign to this one
 */
Abstract_dynamics & Abstract_dynamics::operator=(const Abstract_dynamics & src)
{
    parameters = src.parameters;
    deltas = src.deltas;
    gradients = src.gradients;
    stochastic_momenta = src.stochastic_momenta;
    momenta = src.momenta;
    temp_momenta = src.temp_momenta;
    stochastic_transition = src.stochastic_transition;
    alpha = src.alpha;
    st_alpha = src.st_alpha;
    kick = src.kick;
    mv_gauss = NULL;
    if(src.mv_gauss != NULL)
    {
        mv_gauss = new kjb::MV_gaussian_distribution(*(src.mv_gauss));
    }

    return (*this);
}

/**
 * Sets the number of parameters we are sampling over and prepares the
 * data structures needed for the computation accordingly
 *
 * @param num_params the number of parameters
 */
void Abstract_dynamics::set_num_parameters(unsigned int num_params)
{
    if((size_t) parameters.size() != num_params)
    {
        /** The parameters might have been already stored,
         *  this check introduces no harm
         */
        parameters.zero_out(num_params);
    }
    if((size_t) deltas.size() != num_params)
    {
        /** The deltas might have been already stored,
         *  this check introduces no harm
         */
        deltas.zero_out(num_params);
    }

    /** We set all this temporary variables to the right size */
    gradients.zero_out(num_params);
    stochastic_momenta.zero_out(num_params);
    momenta.zero_out(num_params);
    stochastic_momenta.zero_out(num_params);
    temp_momenta.zero_out(num_params);
    if(mv_gauss != NULL )
    {
        if((size_t) mv_gauss->get_dimension() != num_params)
        {
            mv_gauss = new kjb::MV_gaussian_distribution(num_params);
        }
    }
    else
    {
        mv_gauss = new kjb::MV_gaussian_distribution(num_params);
    }

}

/** Implements stochastic dynamics sampling on a set of parameters.
 *  This implementation follows the algorithm described in the 1993 Neal Paper,
 *  with leapfrog discretization and stochastic transitions weighted by
 *  the stochastic alpha parameter.
 *
 *  @param iterations The  number of iterations
 */
void Abstract_dynamics::run
(
    unsigned int         iterations
)
{
    using namespace kjb_c;

    unsigned int i, num_params;
    num_params = parameters.get_length();

    if(num_params == 0)
    {
        std::cout << "Stochastic dynamics, no parameters to sample over" << std::endl;
        return;
    }

    if(!mv_gauss)
    {
        mv_gauss = new  kjb::MV_gaussian_distribution(num_params);
    }

    //double st_alpha = sqrt(1 - alpha*alpha);

    stochastic_momenta = kjb::sample(*mv_gauss);
    stochastic_momenta.resize(num_params);
    for(unsigned int kk = 0; kk < num_params; kk++)
    {
        stochastic_momenta(kk) = 0.0;
    }

    //TODO take actions according to error
    try
    {
        compute_energy_gradient();
    } catch (KJB_error e)
    {
        //TODO what do we do here?
    }

    //We perform half an update of the momenta in the gradient direction
    gradients.ew_multiply(deltas);
    gradients *= 0.5;
    stochastic_momenta.subtract(gradients);

    /* We keep track of what would have happened if we had
     * followed the momenta without stochastic transitions,
     * so that we can reset the correct dynamics at any point
     * as specified by the kick parameter
     */
    momenta = stochastic_momenta;

    for (i = 0; i < iterations; i++)
    {
        if ( (kick > 0) && ( ((i+1) % kick) == 0))
        {
            //Reset if we use kick, don't bother now
            stochastic_momenta = momenta;
        }

        //Store the current momenta for logging
        temp_momenta = stochastic_momenta;

        //Perform a full update of the parameters
        temp_momenta.ew_multiply(deltas);
        parameters = parameters.add(temp_momenta);

        //Verify that the current sample satisfies the constraints
        try
        {
            log_sample();
        } catch(KJB_error e)
        {
            //TODO What do we do here?
        }

        try
        {
            compute_energy_gradient();
        } catch (KJB_error e)
        {
            //TODO What do we do here?
        }
        /* We update the momenta with a full step here. We
         * can do it as long as we take half a step during the
         * first iteration (see Neal's paper, in the leapfrog
         * discretization section)
         */
        gradients.ew_multiply(deltas);
        stochastic_momenta.subtract(gradients);
        momenta.subtract(gradients);

        /* We add a stochastic perturbation to the momenta, using equation (5.23)
         * from Neal's paper
         */
        stochastic_momenta *= alpha;
        stochastic_transition =  kjb::sample(*mv_gauss);
        stochastic_transition *= st_alpha;
        stochastic_momenta = stochastic_momenta + stochastic_transition;

    }

}
