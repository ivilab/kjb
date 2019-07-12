
/* $Id: wrap_gsl_sf.c 17569 2014-09-22 20:56:26Z predoehl $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision |
|  group (the authors) including                                               |
|        Andrew Predoehl,                                                      |
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.     |
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
 * This file is for wrapping routines in Special Functions (chapter 7 
 * in the manual for version 1.13). 
*/

/*
 * IMPORTANT: Each exported routine needs to have a version for both with and
 * without GSL.
*/

#include "m/m_incl.h"
#include "wrap_gsl/wrap_gsl_sf.h"

#ifdef KJB_HAVE_GSL
#    include "gsl/gsl_sf.h"
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

/* =============================================================================
 *                                   kjb_erf
 *
 * Computes error function erf(x)
 *
 * This routine is a thin wrapper to GSL code that computes, if possible,
 * the so-called "error function," a well-known special function.
 * It is defined to be equal to following definite integral:
 *
 * |
 * |               2      / x 
 * | erf(x) := ---------  |     exp( -t**2 ) dt
 * |            sqrt(pi)  / 0
 *
 * This has a close relationship to the CDF of the standard Normal
 * distribution Phi(x).  Check me on this, but I believe the equation is
 *
 * | Phi(x) = 0.5 (1 + erf(x / sqrt(2))).
 *
 * Returns:
 *   If successful, the value is written to *P_ptr and this function returns
 *   NO_ERROR.  If the routine fails -- caused by the library being unavailable
 *   during linking -- an error message is set and the return value is ERROR.
 *
 * Author: Andrew Predoehl
 *
 * Index: random, statistics, special function
 * -----------------------------------------------------------------------------
 */
int kjb_erf(
    double* P_ptr,    /* Pointer to where the result should be stored */
    double x          /* Value at which to evaluate erf               */
)
{
#ifdef KJB_HAVE_GSL
    NRE( P_ptr );
    *P_ptr = gsl_sf_erf(x);
    return NO_ERROR;
#else
    set_dont_have_gsl_error();
    return ERROR;
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   kjb_bessel_I0
 *
 * Computes modified Bessel function, order 0
 *
 * If possible, computes the modified Bessel function, order 0,
 * usually denoted by something like I_0(x).  This is defined as
 * I_0(x) = i * J_0(i*x), where J_0 is the ordinary Bessel function, order 0,
 * and little i is the imaginary unit.
 *
 * This routine is a thin wrapper to GSL code that computes this special
 * function.  If the GSL library is not available, this routine fails with a
 * return value of ERROR.  Otherwise, the value is returned in the location
 * pointed to by pointer P_ptr, which must not equal null.  The argument to
 * the function is stored in x.
 *
 * Keep in mind that I_0(x) grows very quickly with increasing x, on the order
 * of exp(x)/sqrt(x).  If you need to compute I_0 for a large value (perhaps in
 * a ratio calculation), please consider using kjb_scaled_bessel_I0() instead.
 * In this context if x has three digits, it is LARGE.
 *
 * Returns:
 *   If successful, the value is written to *P_ptr and this function returns
 *   NO_ERROR.  If the routine fails -- caused by the library being unavailable
 *   during linking -- an error message is set and the return value is ERROR.
 *
 * Author:
 *   Andrew Predoehl
 *
 * Index: special function, bessel function
 * -----------------------------------------------------------------------------
 */
int kjb_bessel_I0(
    double* P_ptr,    /* Pointer to where the result should be stored */
    double x          /* Value at which to evaluate erf               */
)
{
#ifdef KJB_HAVE_GSL
    NRE( P_ptr );
    *P_ptr = gsl_sf_bessel_I0(x);
    return NO_ERROR;
#else
    set_dont_have_gsl_error();
    return ERROR;
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          kjb_scaled_bessel_I1
 *
 * Computes a ratio:  bessel I1(x) over exp(x)
 *
 * If possible, computes a product: the modified Bessel function, order 1
 * (usually denoted by something like I_1(x) times exp(-x)).
 * The scaling factor is a practical necessity when x is large, since the
 * function grows quickly, yet we sometimes need to compute ratios of these
 * big numbers.
 *
 * This routine is a thin wrapper to GSL code that computes this special
 * function.  If the GSL library is not available, this routine fails with a
 * return value of ERROR.  Otherwise, the value is returned in the location
 * pointed to by pointer P_ptr, which must not equal null.  The argument to
 * the function is stored in x.
 *
 * Returns:
 *   If successful, the value is written to *P_ptr and this function returns
 *   NO_ERROR.  If the routine fails -- caused by the library being unavailable
 *   during linking -- an error message is set and the return value is ERROR.
 *
 * Author: Andrew Predoehl
 *
 * Index: special function, bessel function
 * -----------------------------------------------------------------------------
 */
int kjb_scaled_bessel_I1(
    double* P_ptr,    /* Pointer to where the result should be stored */
    double x          /* Value at which to evaluate erf               */
)
{
#ifdef KJB_HAVE_GSL
    NRE( P_ptr );
    *P_ptr = gsl_sf_bessel_I1_scaled(x);
    return NO_ERROR;
#else
    set_dont_have_gsl_error();
    return ERROR;
#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          kjb_scaled_bessel_I0
 *
 * Computes a ratio:  bessel I0(x) over exp(x)
 *
 * If possible, computes a product: the modified Bessel function, order 0
 * (usually denoted by something like I_0(x) times exp(-x)).
 * Please see kjb_bessel_I0 for exposition about function I0(x).
 * The reason for making this a product with exp(-x) is that I0(x) alone grows
 * very fast.  If one needs to evaluate it for large values of x (100 or
 * more), we need some scaling factor to avoid overflow.
 *
 * This routine is a thin wrapper to GSL code that computes this special
 * function.  If the GSL library is not available, this routine fails with a
 * return value of ERROR.  Otherwise, the value is returned in the location
 * pointed to by pointer P_ptr, which must not equal null.  The argument to
 * the function is stored in x.
 *
 * Returns:
 *   If successful, the value is written to *P_ptr and this function returns
 *   NO_ERROR.  If the routine fails -- caused by the library being unavailable
 *   during linking -- an error message is set and the return value is ERROR.
 *
 * Author: Andrew Predoehl
 *
 * Index: special function, bessel function
 * -----------------------------------------------------------------------------
 */
int kjb_scaled_bessel_I0(
    double* P_ptr,    /* Pointer to where the result should be stored */
    double x          /* Value at which to evaluate erf               */
)
{
#ifdef KJB_HAVE_GSL
    NRE( P_ptr );
    *P_ptr = gsl_sf_bessel_I0_scaled(x);
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

