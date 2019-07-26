
/* $Id: abstract_dynamics.h 10645 2011-09-29 19:51:35Z predoehl $ */

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

#ifndef ABSTRACT_DYNAMICS_INCLUDED
#define ABSTRACT_DYNAMICS_INCLUDED

#include "m_cpp/m_vector.h"
#include "prob_cpp/prob_sample.h"
#include "prob_cpp/prob_distribution.h"

namespace kjb {

/** @class Abstract_dynamics Implements stochastic dynamics sampling on a set
 *  of parameters.This implementation follows the algorithm described in
 *  the 1993 Neal Paper, with leapfrog discretization and stochastic transitions
 *  weighted by the stochastic alpha parameter.
 */
class Abstract_dynamics
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
    Abstract_dynamics(double ialpha = 0.99, unsigned int ikick = 0) : mv_gauss(NULL)
    {
        if(ialpha <0 || ialpha > 1)
        {
            throw kjb::Illegal_argument("Alpha must be between 0 and 1");
        }
        alpha = ialpha;
        st_alpha = sqrt(1 - alpha*alpha);
        kick = ikick;
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
    Abstract_dynamics
    (
        kjb::Vector iparameters,
        kjb::Vector ideltas,
        double ialpha = 0.99,
        unsigned int ikick = 0
    ) : mv_gauss(NULL)
    {
        if(ialpha <0 || ialpha > 1)
        {
            throw kjb::Illegal_argument("Alpha must be between 0 and 1");
        }
        if(ideltas.size() != iparameters.size())
        {
            KJB_THROW_2(Illegal_argument, "The number of delta steps does not match the number of parameters");
        }
        alpha = ialpha;
        st_alpha = sqrt(1 - alpha*alpha);
        kick = ikick;
        parameters = iparameters;
        deltas = ideltas;
    }

    Abstract_dynamics(const Abstract_dynamics & src);

    Abstract_dynamics & operator=(const Abstract_dynamics & src);

    virtual ~Abstract_dynamics()
    {
        delete mv_gauss;
    }

    /** @brief Sets the number of parameters we will be sampling over
     */
    void set_num_parameters(unsigned int num_params);

protected:

    /** @brief Implements stochastic dynamics sampling on a set of parameters.
     *  This implementation follows the algorithm described in the 1993 Neal Paper,
     *  with leapfrog discretization and stochastic transitions weighted by
     *  the stochastic alpha parameter. This function MUST be kept protected so that
     *  only classes that inherit from this class can use it. Each of this derived class
     *  will have to provide its own public interface to this method
     */
    void run(unsigned int iterations);

    virtual void compute_energy_gradient() = 0;

    virtual void log_sample()
    {
        return;
    }

    /** @brief The parameters to sample over */
    kjb::Vector parameters;
    /** @brief The size of the step to take in the gradient direction. This step
        size is different for each parameter we are sampling over */
    kjb::Vector deltas;

    /* @brief This vector will contain the gradient of the energy function,
     * computed at every iteration
     */
    kjb::Vector gradients;

    /* @brief The following members are used during the sampling, and
     * are class members for efficiency reasons. */
    kjb::Vector stochastic_momenta;
    kjb::Vector momenta;
    kjb::Vector temp_momenta;
    kjb::Vector stochastic_transition;
    kjb::MV_gaussian_distribution * mv_gauss;

    /* @brief This parameter tweaks the contribution of the stochastic transition.
     * If it is close to one, the random transition has little weight, and the dynamics
     * mostly follow the momenta. The closer it gets to zero, the more the stochastic
     * transitions will influence the selected trajectory.*/
    double alpha;
    double st_alpha;

    /* @brief This parameters is used to reset the momenta to the correct trajectory,
     * thus eliminating the effect of the stochastic transitions. It specifies
     * after how many transitions the momenta are to be reset. This was not fully tested */
    unsigned int kick;

};

}

#endif


