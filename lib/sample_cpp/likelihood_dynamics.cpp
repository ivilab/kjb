
/* $Id: likelihood_dynamics.cpp 10704 2011-09-29 19:52:57Z predoehl $ */

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

#include "sample_cpp/likelihood_dynamics.h"

using namespace kjb;

Likelihood_dynamics::Likelihood_dynamics(const Likelihood_dynamics & src) :
    Abstract_dynamics(src),
    etas(src.etas)
{

}

Likelihood_dynamics & Likelihood_dynamics::operator=(const Likelihood_dynamics & src)
{
    Abstract_dynamics::operator=(src);
    etas = src.etas;
    return (*this);
}


void Likelihood_dynamics::compute_energy_gradient()
{
    double ll = 0;
    double ll2 = 0;

    if( callbacks.size() != (size_t) parameters.size() )
    {
        throw KJB_error("The number of callbacks does not match the number of parameters");
    }
    try{
        for(int i = 0; i < parameters.size(); i++)
        {
             /**
              * We here compute the gradient of the energy function with respect to the ith parameter.
              * We compute the likelihood value ll in
              * p1 = (ith_parameter + step)
              * and the likelihood value ll2 in
              * p2 = (ith_parameter + step)
              * The gradient is then
              * gradient = - (d likelihood/d parameter) = - (ll - ll2)/(2*step)
              * There is a different step per parameter, and they are stored in the vector etas
              */
             callbacks[i](parameters(i) + etas(i));
             ll = compute_likelihood();
             callbacks[i](parameters(i) - etas(i));
             ll2 = compute_likelihood();
             gradients(i) = -(ll-ll2)/(2*etas(i));

             /** We set the parameter to its original value prior to the likelihood computations */
             callbacks[i](parameters(i));
        }
    } catch (KJB_error e)
    {
        // TODO What do we do here?
    }
}

void Likelihood_dynamics::run(unsigned int iterations)
{
    if(parameter_getters.size() != (unsigned int)parameters.size())
    {
        throw KJB_error("The number of parameter getters does not match the number of parameters");
    }
    for(int i = 0; i < parameters.size(); i++)
    {
        parameters(i) = parameter_getters[i]();
    }

    Abstract_dynamics::run(iterations);
}
