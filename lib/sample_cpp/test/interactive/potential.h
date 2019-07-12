/* $Id$ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 * =========================================================================== */

#ifndef POTENTIAL_H 
#define POTENTIAL_H 
/** @brief Calculates Muller's potential with double precision. */
double mullers_potential_d(double x, double y);

/** 
 * @brief Calculates the gradient of Muller's potential with double 
 * precision.
 */
void grad_mullers_potential_d
(
    double* dx_V_out, 
    double* dy_V_out,
    double  x, 
    double  y
);
#endif
