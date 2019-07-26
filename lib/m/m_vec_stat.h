
/* $Id: m_vec_stat.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef M_VEC_STAT_INCLUDED
#define M_VEC_STAT_INCLUDED


#include "m/m_gen.h"
#include "m/m_stat.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

typedef struct Vector_stat
{
    int n;
    Vector *mean_vp;
    Vector *stdev_vp;
    Vector *min_vp;
    Vector *max_vp;
}
Vector_stat;

/* -------------------------------------------------------------------------- */

int get_vector_mean(const Vector* vp, double* result_ptr);

int get_vector_mean_square(const Vector* vp, double *result_ptr);

int          get_vector_stats
(
    const Vector* vp,
    const Int_vector* skip_vp,
    double*       mean_ptr,
    double*       stdev_ptr,
    int*          n_ptr,
    double*       min_ptr,
    double*       max_ptr
);

int          get_vector_stats_2   (Stat* stat_ptr, const Vector* vp);
Stat*        get_vector_stats_3   (const Vector* vp);
int          add_vector_data_point(const Vector*);
Vector_stat* get_vector_data_stats(void);
void         free_vector_stat     (Vector_stat*);
int          clear_vector_stats   (void);
void         cleanup_vector_stats (void);

int ow_square_vector_elements(Vector* source_vp);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

