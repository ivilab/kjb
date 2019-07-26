
/* $Id: */

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

#ifndef SEQUENTIAL_PARTICLES_INCLUDED
#define SEQUENTIAL_PARTICLES_INCLUDED

#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

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
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

