
/* $Id: wrap_gsl_rnd.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.      |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */




/*
 * This file is for wrapping routines in Random Number Distrutions (chapter 19
 * in the manual for version 1.8). 
*/

/*
 * IMPORTANT: Each exported routine needs to have a version for both with and
 * without GSL.
*/

#include "m/m_incl.h"
#include "m/m_incl.h"
#include "wrap_gsl/wrap_gsl_rnd.h"

#ifdef KJB_HAVE_GSL
#    include "gsl/gsl_cdf.h"
#endif 


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef KJB_HAVE_GSL 

static void set_dont_have_gsl_error(void)
{
    set_error("Operation failed because the program was built without the ");
    add_error("GSL libraries readily available.");
    add_error("Appropriate installation, file manipulation and re-compiling ");
    add_error("is needed to fix this.");
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int kjb_cdf_tdist_P(double* P_ptr, double x, double nu)
{
#ifdef KJB_HAVE_GSL
    *P_ptr = gsl_cdf_tdist_P(x, nu);
    return NO_ERROR;
#else
    set_dont_have_gsl_error();
    return ERROR;
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int kjb_cdf_tdist_Q(double* P_ptr, double x, double nu)
{
#ifdef KJB_HAVE_GSL
    *P_ptr = gsl_cdf_tdist_Q(x, nu);
    return NO_ERROR;
#else
    set_dont_have_gsl_error();
    return ERROR;
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

