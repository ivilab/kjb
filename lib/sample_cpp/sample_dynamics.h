
/* $Id: sample_dynamics.h 10651 2011-09-29 19:51:42Z predoehl $ */

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
|  Author: Luca Del Pero, Joseph Schlecht
* =========================================================================== */

#ifndef SAMPLE_DYNAMICS_INCLUDED
#define SAMPLE_DYNAMICS_INCLUDED

#include "m_cpp/m_vector.h"

namespace kjb {

/** @brief Double precision leapfrog stochastic dynamics. */

    int stochastic_dynamics
    (
        unsigned int    iterations,
        const kjb::Vector &  delta_t,
        double          alpha,
        unsigned int    kick,
        kjb::Vector &        parameters,
        int             (*compute_energy_gradient)(const Vector & parameters, Vector & out_gradient),
        int             (*accept_sample)(const Vector & parameters),
        int             (*log_sample)(const Vector & parameters, const Vector & momenta )
    );


}

#endif


