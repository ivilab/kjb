
/* $Id: sample_gauss.h 20841 2016-09-04 18:56:00Z kobus $ */

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

#ifndef SAMPLE_GAUSS_INCLUDED
#define SAMPLE_GAUSS_INCLUDED


#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int get_general_sv_gauss_random_matrix
(
    Matrix** mpp,
    int      num_rows,
    int      num_cols,
    double   mean,
    double   stdev 
);

int    get_gauss_random_matrix
(
    Matrix** mpp,
    int      num_rows,
    int      num_cols
);

int    get_gauss_random_matrix_2
(
    Matrix** mpp,
    int      num_rows,
    int      num_cols
);

int    get_gauss_random_vector       (Vector** vpp, int length);
int    get_gauss_random_vector_2     (Vector** vp, int length);
int    get_lookup_gauss_random_vector(Vector** vp, int length);
double gauss_rand                    (void);
double gauss_rand_2                  (void);
double lookup_gauss_rand             (void);

int gaussian_rand(double* sample, double mean, double variance);

int mv_std_gaussian_rand(Vector** sample, int len);

int mv_ind_gaussian_rand(Vector** sample, const Vector* mean, const Vector* vars);

int mv_gaussian_rand(Vector** sample, const Vector* mean, const Matrix* covar_mat);

int gaussian_pdf(double* pdf, double x, double mean, double variance);

int mv_std_gaussian_pdf(double* pdf, const Vector* x);

int mv_ind_gaussian_pdf(double* pdf, const Vector* x, const Vector* mean, const Vector* vars);

int mv_gaussian_pdf(double* pdf, const Vector* x, const Vector* mean, const Matrix* covar_mat);

int gaussian_log_pdf(double* pdf, double x, double mean, double variance);

int mv_std_gaussian_log_pdf(double* pdf, const Vector* x);

int mv_ind_gaussian_log_pdf(double* pdf, const Vector* x, const Vector* mean, const Vector* vars);

int mv_gaussian_log_pdf(double* pdf, const Vector* x, const Vector* mean, const Matrix* covar_mat);

/**************************************************************/
/* These routines should disappear...the above four functions
   replace them and add functionality, as well as standardizing
   the names. */
int get_general_gauss_random_vector
(
    Vector**        target_vpp,
    const Vector*   mu,
    const Matrix*   sigma
);

int get_density_gaussian
(
    double*         density,
    const Vector*   X,
    const Vector*   mu,
    const Matrix*   sigma
);

int get_log_density_gaussian
(
    double*         density,
    const Vector*   X,
    const Vector*   mu,
    const Matrix*   sigma
);

double log_gaussian_pdf(double x, double mean, double variance);
/**************************************************************/

int gaussian_rand_with_limits
(
    double* sample,
    double  mu,
    double  sigma,
    double  a,
    double  b
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


