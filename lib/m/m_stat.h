
/* $Id: m_stat.h 14346 2013-05-02 22:43:19Z jguan1 $ */

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

#ifndef M_STAT_INCLUDED
#define M_STAT_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

typedef struct Stat
{
    int n;
    double mean;
    double stdev;
    double min;
    double max;
}
Stat;

/* -------------------------------------------------------------------------- */

int   add_data_point    (double);

int   get_data_stats
(
    double* mean_ptr,
    double* stdev_ptr,
    int*    n_ptr,
    double* min_ptr,
    double* max_ptr
);

int   get_data_stats_2  (Stat* stat_ptr);
Stat* get_data_stats_3  (void);
int   clear_data_stats  (void);
void  cleanup_data_stats(void);

#ifdef NOT_CURRENTLY_USED
double add_errors(double, double);
#endif


int get_correlation_coefficient
(
    const Vector* first_vp,
    const Vector* second_vp,
    double* result_ptr
);

int get_log_gaussian_density
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double*       log_prob_ptr
);

int get_gaussian_density
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double*       prob_ptr
);

int get_malhalanobis_distance
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double*       dist_ptr
);

int get_malhalanobis_distance_sqrd
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double*       dist_sqrd_ptr
);

int get_log_gaussian_density_2
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double        max_norm_var,
    double*       log_prob_ptr
);

int get_gaussian_density_2
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double        max_norm_var,
    double*       prob_ptr
);

int get_malhalanobis_distance_2
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double        max_norm_var,
    double*       dist_ptr
);

int get_malhalanobis_distance_sqrd_2
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double        max_norm_var,
    double*       dist_sqrd_ptr
);

int get_log_gaussian_density_3
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double        max_norm_var,
    double*       log_prob_ptr
);

int get_gaussian_density_3
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double        max_norm_var,
    double*       prob_ptr
);

int get_malhalanobis_distance_3
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double        max_norm_var,
    double*       dist_ptr
);

int get_malhalanobis_distance_sqrd_3
(
    const Vector* x_vp,
    const Vector* u_vp,
    const Vector* var_vp,
    double        max_norm_var,
    double*       dist_sqrd_ptr
);

int compute_mean_and_stdev
(
    double* mean_ptr,
    double* stdev_ptr,
    int     num_data_points,
    double  data_sum,
    double  data_sqrd_sum
);

int compute_weighted_mean_and_stdev
(
    double* mean_ptr,
    double* stdev_ptr,
    double  total_weight,
    double  data_sum,
    double  data_sqrd_sum
);


int get_entropy(const Vector* prob_vp, double* result_ptr);

int get_cumulative_distribution
(
    Vector**      cum_vpp,
    const Vector* vp 
);

int sample_distribution_using_cumulative(const Vector* cum_dist_vp);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


