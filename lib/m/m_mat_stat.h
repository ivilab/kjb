
/* $Id: m_mat_stat.h 4884 2009-11-24 22:43:59Z ernesto $ */

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

#ifndef M_MAT_STAT_INCLUDED
#define M_MAT_STAT_INCLUDED


#include "m/m_gen.h"
#include "m/m_vec_stat.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


double get_matrix_mean(const Matrix* mp);

int average_matrix_vector_rows
(
    Vector**      output_vpp,
    const Matrix_vector* input_mvp
);

int average_matrix_rows(Vector** output_vpp, const Matrix* input_mp);

int average_matrix_rows_without_negatives
(
    Vector**      output_vpp,
    const Matrix* input_mp
);

int average_matrix_rows_without_missing
(
    Vector**      output_vpp,
    const Matrix* input_mp
);

int sum_matrix_rows(Vector** output_vpp, const Matrix* input_mp);

int sum_matrix_rows_without_missing
(
    Vector**      output_vpp,
    const Matrix* input_mp
);

int sum_matrix_rows_without_negatives
(
    Vector**      output_vpp,
    const Matrix* input_mp
);

int average_matrix_cols(Vector** output_vpp, const Matrix* input_mp);

int average_matrix_cols_without_missing
(
    Vector**      output_vpp,
    const Matrix* input_mp
);

int average_matrix_cols_without_negatives
(
    Vector**      output_vpp,
    const Matrix* input_mp
);

int ow_sum_matrix_rows(Vector* output_vp, const Matrix* input_mp);

int sum_matrix_cols(Vector** output_vpp, const Matrix* input_mp);

int sum_matrix_cols_without_negatives
(
    Vector**      output_vpp,
    const Matrix* input_mp
);

int sum_matrix_cols_without_missing
(
    Vector**      output_vpp,
    const Matrix* input_mp
);

int get_matrix_row_stats
(
    const Matrix* mp,
    Vector**      mean_vpp,
    Vector**      stdev_vpp
);

int get_matrix_row_stats_2
(
    const Matrix* mp,
    Vector**      mean_vpp,
    Matrix**      cov_mpp
);

int get_fixed_clustering_of_3D_data
(
    Matrix**      result_mpp,
    const Matrix* mp,
    int           resolution
);

int get_fixed_cluster_average_of_3D_data
(
    Vector**      result_vpp,
    const Matrix* mp,
    int           resolution
);

int average_matrix_vector_elements(Matrix** target_mpp, const Matrix_vector* mvp);

int is_matrix_row_stochastic(const Matrix* M);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

