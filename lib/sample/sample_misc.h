
/* $Id: sample_misc.h 8192 2011-01-12 18:56:16Z delpero $ */

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

#ifndef SAMPLE_MISC_INCLUDED
#define SAMPLE_MISC_INCLUDED


#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int kjb_rand_int(int lb, int ub);

int sample_from_discrete_distribution(int* idx, const Vector* dist);

double sample_from_uniform_distribution(double a, double b);

int sample_random_unit_vector(Vector** v);

int poisson_rand(double lambda);

double gamma_pdf(double x, double alpha, double beta);

double log_gamma_pdf(double x, double alpha, double beta);

double sample_from_gamma_distribution
(
    double alpha,
    double beta,
    double a,
    double b
);

double sample_from_gamma_distribution_2(double alpha, double beta);

int pick_m_from_n(Int_vector ** m_indexes, int m, int n);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


