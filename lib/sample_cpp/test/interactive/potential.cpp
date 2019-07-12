/* $Id$ */
/**
 * This work is licensed under a Creative Commons 
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 * 
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 * 
 * You are free:
 * 
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 * 
 * Under the following conditions:
 * 
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 * 
 *    Noncommercial. You may not use this work for commercial purposes.
 * 
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 * 
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 * 
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 * 
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */
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
   |  Author:  Joe Schlecht
   |  Edited by: Kyle Simek
 * =========================================================================== */

#include <l_cpp/l_cpp_incl.h>
#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <math.h>
#include "potential.h"


using namespace std;
using namespace kjb;


#define  ERR_BUF_LEN  256


/** @brief Constant A in Muller's potential function. */
static const float A[4] = {-200.0f, -100.0f, -170.0f, 15.0f};


/** @brief Constant a in Muller's potential function. */
static const float a[4] = {-1.0f, -1.0f, -6.5f, 0.7f};


/** @brief Constant b in Muller's potential function. */
static const float b[4] = {0.0f, 0.0f, 11.0f, 0.6f};


/** @brief Constant c in Muller's potential function. */
static const float c[4] = {-10.0f, -10.0f, -6.5f, 0.7f};


/** @brief Constant X in Muller's potential function. */
static const float X[4] = {1.0f, 0.0f, -0.5f, -1.0f};


/** @brief Constant Y in Muller's potential function. */
static const float Y[4] = {0.0f, 0.5f, 1.5f, 1.0f};


/** @brief Error buffer. */
static char err_buf[ERR_BUF_LEN] = {0};



/**
 * @param  V_out  Result parameter. Muller's potential evaluated at @em x,y.
 * @param  x      Input to evaluate Muller's potential at.
 * @param  y      Input to evaluate Muller's potential at.
 *
 * @return On success, NULL is returned. On error, an Error is returned.
 * - Error_types::ERROR_INV_RESULT  Precision error.
 */
double mullers_potential_d(double x, double y)
{
    uint8_t i;
    double  V;

    V = 0;

    for (i = 0; i < 4; i++)
    {
        V += (double)A[i] * 
             exp((double)a[i] * (x - (double)X[i]) * (x - (double)X[i]) +
                 (double)b[i] * (x - (double)X[i]) * (y - (double)Y[i]) +
                 (double)c[i] * (y - (double)Y[i]) * (y - (double)Y[i]));
    }

#if defined MULLER_HAVE_ISINF && defined MULLER_HAVE_ISNAN
    if (isinf(V) || isnan(V))
    {
        KJB_THROW_3(Runtime_error, "Muller's potential precision error: x=%e y=%e", (x)(y));
    }
#endif

    return V;
}





/** 
 * @param  dx_V_out  Result parameter. Partial derivative of Muller's
 *                   potential w.r.t. @em x.
 * @param  dy_V_out  Result parameter. Partial derivative of Muller's
 *                   potential w.r.t. @em y.
 * @param  x         Input to evaluate the partial derivative of Muller's 
 *                   potential w.r.t. @em x at.
 * @param  y         Input to evaluate the partial derivative of Muller's 
 *                   potential w.r.t. @em y at.
 *
 * @return On success, NULL is returned. On error, an Error is returned.
 * - Error_types::ERROR_INV_RESULT  Precision error.
 */
void grad_mullers_potential_d
(
    double* dx_V_out, 
    double* dy_V_out,
    double  x, 
    double  y
)
{
    uint8_t i;
    double  dx_V, dy_V;

    dx_V = 0;

    for (i = 0; i < 4; i++)
    {
        dx_V += (double)A[i] * 
                (2.0 * (double)a[i] * (x - (double)X[i]) +
                       (double)b[i] * (y - (double)Y[i])) *
                exp((double)a[i] * (x - (double)X[i]) * (x - (double)X[i]) +
                    (double)b[i] * (x - (double)X[i]) * (y - (double)Y[i]) +
                    (double)c[i] * (y - (double)Y[i]) * (y - (double)Y[i]));
    }

    dy_V = 0;

    for (i = 0; i < 4; i++)
    {
        dy_V += (double)A[i] * 
                (2.0 * (double)c[i] * (y - (double)Y[i]) +
                       (double)b[i] * (x - (double)X[i])) *
                exp((double)a[i] * (x - (double)X[i]) * (x - (double)X[i]) +
                    (double)b[i] * (x - (double)X[i]) * (y - (double)Y[i]) +
                    (double)c[i] * (y - (double)Y[i]) * (y - (double)Y[i]));
    }

    if (isinf(dx_V) || isnan(dx_V) || isinf(dy_V) || isnan(dy_V))
    {
        snprintf(err_buf, ERR_BUF_LEN-1, 
                "Gradient of Muller's potential precision error: x=%e y=%e", 
                x, y);

        KJB_THROW_2(Runtime_error, err_buf);
    }

    *dx_V_out = dx_V;
    *dy_V_out = dy_V;
}
