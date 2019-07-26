
/* $Id: m_vec_norm.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef M_VEC_NORM_INCLUDED
#define M_VEC_NORM_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int normalize_vector
(
    Vector**      target_vpp,           /* Double pointer to output Vector. */
    const Vector* vp,                   /* Pointer to input Vector.         */
    Norm_method   normalization_method  /* Method to use.              */
);

int scale_vector_by_sum
(
    Vector**      target_vpp, /* Double pointer to output Vector. */
    const Vector* vp          /* Pointer to input Vector.         */
);

int scale_vector_by_mean
(
    Vector**      target_vpp, /* Double pointer to output Vector. */
    const Vector* vp          /* Pointer to input Vector.         */
);

int scale_vector_by_max_abs
(
    Vector**      target_vpp, /* Double pointer to output Vector. */
    const Vector* vp          /* Pointer to input Vector.         */
);

int scale_vector_by_max
(
    Vector**      target_vpp, /* Double pointer to output Vector. */
    const Vector* vp          /* Pointer to input Vector.         */
);

int scale_vector_by_magnitude
(
    Vector**      target_vpp, /* Double pointer to output Vector. */
    const Vector* vp
);

int ow_normalize_vector
(
    Vector*     vp,    /* Pointer to input Vector. Contents overwritten. */
    Norm_method normalization_method
);


int silent_ow_scale_vector_by_sum(Vector* vp);

#ifdef TEST
#   define safe_ow_scale_vector_by_sum(x) debug_safe_ow_scale_vector_by_sum(x, __FILE__, __LINE__)

    int debug_safe_ow_scale_vector_by_sum
    (
        Vector*     vp,
        const char* file,
        int         line
    );
#else
    int safe_ow_scale_vector_by_sum(Vector* vp);
#endif

int safe_ow_scale_vector_by_sum_2(Vector* vp, double* scale_factor_ptr);

int ow_scale_vector_by_sum
(
    Vector* vp  /* Pointer to input Vector. Contents overwritten. */
);

int ow_scale_vector_by_mean
(
    Vector* vp  /* Pointer to input Vector. Contents overwritten. */
);

int ow_scale_vector_by_max_abs
(
    Vector* vp  /* Pointer to input Vector. Contents overwritten. */
);

int ow_scale_vector_by_magnitude
(
    Vector* vp  /* Pointer to input Vector. Contents overwritten. */
);

int ow_scale_vector_by_max
(
    Vector* vp   /* Pointer to input Vector. Contents overwritten. */
);

int normalize_vector_2
(
    Vector**      target_vpp,           /* Double pointer to output Vector.*/
    const Vector* vp,                   /* Pointer to input Vector.        */
    Norm_method   normalization_method, /* Method to use.              */
    double*       scale_factor_ptr      /* Optional return of scaling */
);

int scale_vector_by_sum_2
(
    Vector**      target_vpp,       /* Double pointer to output Vector.*/
    const Vector* vp,               /* Pointer to input Vector.        */
    double*       scale_factor_ptr  /* Optional return of scaling */
);

int scale_vector_by_mean_2
(
    Vector**      target_vpp,       /* Double pointer to output Vector.*/
    const Vector* vp,               /* Pointer to input Vector.        */
    double*       scale_factor_ptr  /* Optional return of scaling */
);

int scale_vector_by_max_abs_2
(
    Vector**      target_vpp,       /* Double pointer to output Vector.*/
    const Vector* vp,               /* Pointer to input Vector.        */
    double*       scale_factor_ptr  /* Optional return of scaling */
);

int scale_vector_by_max_2
(
    Vector**      target_vpp,       /* Double pointer to output Vector.*/
    const Vector* vp,               /* Pointer to input Vector.        */
    double*       scale_factor_ptr  /* Optional return of scaling */
);

int scale_vector_by_magnitude_2
(
    Vector**      target_vpp,       /* Double pointer to output Vector.*/
    const Vector* vp,               /* Pointer to input Vector.        */
    double*       scale_factor_ptr  /* Optional return of scaling */
);

int ow_normalize_vector_2
(
    Vector*     vp,                   /* Pointer to input Vector. Contents overwritten. */
    Norm_method normalization_method,
    double*     scale_factor_ptr      /* Optional return of scaling */
);

int ow_scale_vector_by_sum_2
(
    Vector* vp,               /* Pointer to input Vector. Contents overwritten. */
    double* scale_factor_ptr  /* Optional return of scaling */
);

int ow_scale_vector_by_mean_2
(
    Vector* vp,               /* Pointer to input Vector. Contents overwritten. */
    double* scale_factor_ptr  /* Optional return of scaling */
);

int ow_scale_vector_by_max_abs_2
(
    Vector* vp,               /* Pointer to input Vector. Contents overwritten. */
    double* scale_factor_ptr  /* Optional return of scaling */
);

int ow_scale_vector_by_max_2
(
    Vector* vp,               /* Pointer to input Vector. Contents overwritten. */
    double* scale_factor_ptr  /* Optional return of scaling */
);

int ow_scale_vector_by_magnitude_2
(
    Vector* vp,               /* Pointer to input Vector. Contents overwritten. */
    double* scale_factor_ptr  /* Optional return of scaling */
);

double vector_magnitude(const Vector* vp);
double max_abs_vector_element(const Vector* vp);
double min_vector_element(const Vector* vp);
double max_vector_element(const Vector* vp);

int get_min_vector_element
(
    const Vector* vp,      /* Pointer to input Vector.   */
    double*       min_ptr  /* Pointer to min to be found */
);

int get_max_vector_element
(
    const Vector* vp,      /* Pointer to input Vector.   */
    double*       max_ptr  /* Pointer to max to be found */
);

int max_thresh_vector
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        max
);

int min_thresh_vector
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        min_val
);

int ow_max_thresh_vector(Vector* source_vp, double max);
int ow_min_thresh_vector(Vector* source_vp, double min_val);

int ow_normalize_log_prob_vp(Vector* log_vp);
double ow_exp_scale_by_sum_log_vector(Vector* log_vp);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


