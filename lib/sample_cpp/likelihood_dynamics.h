
/* $Id: likelihood_dynamics.h 18278 2014-11-25 01:42:10Z ksimek $ */

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
|  Author: Luca Del Pero
* =========================================================================== */

#ifndef LIKELIHOOD_DYNAMICS_INCLUDED
#define LIKELIHOOD_DYNAMICS_INCLUDED

#include "sample_cpp/abstract_dynamics.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <l_cpp/l_int_matrix.h>

namespace kjb {

/** @class Likelihood_dynamics Implements stochastic dynamics sampling on a set
 *  of parameters.This implementation follows the algorithm described in
 *  the 1993 Neal Paper, with leapfrog discretization and stochastic transitions
 *  weighted by the stochastic alpha parameter. Here the user has to provide a
 *  likelihood function in order to compute the gradient of the energy function
 */
class Likelihood_dynamics : public Abstract_dynamics
{
public:

    /**
     * @param ialpha This parameter tweaks the contribution of the stochastic transition.
     *        If it is close to one, the random transition has little weight, and the dynamics
     *        mostly follow the momenta. The closer it gets to zero, the more the stochastic
     *        transitions will influence the selected trajectory.
     * @param ikick This parameters is used to reset the momenta to the correct trajectory,
     *        thus eliminating the effect of the stochastic transitions. It specifies
     *        after how many transitions the momenta are to be reset. This was not fully tested
     */
    Likelihood_dynamics(double ialpha = 0.99, unsigned int ikick = 0) :
        Abstract_dynamics(ialpha, ikick)
    {

    }

    /**
     * @param iparameters The initial values of the parameters to sample over
     * @param ideltas The size of the step to take in the gradient direction. This step
     *        size is different for each parameter we are sampling over
     * @param ialpha This parameter tweaks the contribution of the stochastic transition.
     *        If it is close to one, the random transition has little weight, and the dynamics
     *        mostly follow the momenta. The closer it gets to zero, the more the stochastic
     *        transitions will influence the selected trajectory.
     * @param ikick this parameters is used to reset the momenta to the correct trajectory,
     *        thus eliminating the effect of the stochastic transitions. It specifies
     *        after how many transitions the momenta are to be reset. This was not fully tested
     */
    Likelihood_dynamics
    (
        kjb::Vector iparameters,
        kjb::Vector ideltas,
        kjb::Vector ietas,
        double ialpha = 0.99,
        unsigned int ikick = 0
    ) : Abstract_dynamics(iparameters, ideltas, ialpha, ikick)
    {
        if(ietas.size() != iparameters.size())
        {
            KJB_THROW_2(Illegal_argument, "The number of eta steps does not match the number of parameters");
        }
        etas = ietas;
    }

    Likelihood_dynamics(const Likelihood_dynamics & src);

    Likelihood_dynamics & operator=(const Likelihood_dynamics & src);

    virtual ~Likelihood_dynamics() { }

    virtual void run(unsigned int iterations);

    virtual void set_index(unsigned int iindex)
    {
    }
    
    virtual void set_index(Int_vector iindex)
    {
    }


protected:

    virtual void log_sample() { };

    virtual void compute_energy_gradient();

    virtual double compute_likelihood() = 0;

    /** @brief The size of the step to use when computing the gradient of the
     *  energy function, in a two point estimate fashion. This varies for each
     *  parameter we are sampling over */
    kjb::Vector etas;

    /** @brief We have a callback for each parameter we are sampling over.
     *  Each of them returns a void and accepts a double. Each callback
     *  should change the model according to the sampled parameter value
     */
    std::vector < boost::function1<void, double> > callbacks;

    /** @brief We have a callback for each parameter we are sampling over.
     *  Each of them returns a double. Each callback
     *  should get the parameter value to initialize the sampling
     */
    std::vector < boost::function0<double> > parameter_getters;

};


}

#endif


