
/* $Id*/

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

#ifndef G_CHAMFER_H
#define G_CHAMFER_H

#include "m/m_matrix.h"
#include "i/i_matrix.h"
#include "edge/edge_base.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int chamfer_transform(
        const Matrix* in,
        int size,
        Matrix** distances_out,
        Int_matrix_vector** locations_out);

int chamfer_transform_2(
        const Edge_set* points,
        int num_rows,
        int num_cols,
        int size,
        Matrix** distances_out,
        Edge_point**** edge_map);

int sum_sq_distance(
        const Matrix* chamfer_image,
        const Edge_set* tmplate,
        int offset_x,
        int offset_y,
        double dist_bound,
        double* distance_out,
        size_t* point_count_out);

int chamfer_distance(
        const Matrix* chamfer_image,
        const Edge_set* tmplate,
        int offset_x,
        int offset_y,
        double dist_bound,
        double* distance_out,
        size_t* point_count_out);

int oriented_chamfer_distance(
        Edge_point const*** image_edge_map,
        int num_rows, int num_cols,
        const Edge_set* tmplate,
        int offset_x, 
        int offset_y,
        double* distance_out,
        size_t* point_count_out);

int oriented_sum_sq_distance(
        Edge_point const*** image_edge_map,
        int num_rows,
        int num_cols,
        const Edge_set* tmplate,
        int offset_x,
        int offset_y,
        double* distance,
        size_t* point_count_out);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

