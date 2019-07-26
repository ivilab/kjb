/* $Id$ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2012 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

/* vim: tabstop=4 shiftwidth=4 foldmethod=marker */

#include "g/g_area.h"
#include <math.h>
#include <l/l_debug.h>
#include <l/l_math.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the surface area of a hypersphere in d dimensions.  I.e.,
 * passing 2 would return the circumference of a circle.
 *
 * To get the surface area of a non-unit sphere of radius r, multiply
 * result by r^(d-1)
 *
 * @warning This isn't implemented for dimensions greater than 3
 * @author Kyle Simek
 */
double unit_sphere_surface_area(unsigned int dimension)
{
    switch(dimension)
    {
        case 1: return 2;
        case 2: return 2*M_PI;
        case 3: return 4 * M_PI;
        default: break;
    }

    abort();
    /* NOTE: I just decided to  remove the code below, since I didn't have time to test it, and I couldn't find a portable implementation of tgamma function wihtout enabling C99. --kyle */
/*    UNTESTED_CODE(); */

    /* from wikipedia: */
    /*  http://en.wikipedia.org/wiki/N-sphere#Volume_and_surface_area */
    /*  TODO Test me! */
    /*  TODO perform computations in log space to avoid overflow. */
    /*  TODO this is really ugly :-( */
    /*
    if(dimension % 2 == 0)
        return dimension * pow(M_PI, dimension/2)/exp(gamma(1+dimension/2));
    else
    */
        /* d * 2*(n+1/2) * pi^((n-1)/2) / !!(d) */
    /*
        return dimension * (1 << ((dimension+1)/2)) * pow(M_PI, (dimension-1)/2)/exp(gamma(1+exp(gamma(1+dimension))));
        */
}

#ifdef __cplusplus
}
#endif
