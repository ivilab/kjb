
#ifndef G_CAMERA_MATRIX_H
#define G_CAMERA_MATRIX_H
/* =============================================================================
 *                              g_camera_matrix
 *
 * Provides functions for generating camera matrices given known values.
 *
 * Summary:
 *      get_camera_matrix_from_line_correspondences - Given a set of six line
 * correspondences, (arranged as two matrices), usees a simple psuedo-matrix
 * strategy to find the camera matrix that would produce the correspondences.
 *
 *      get_camera_matrix_from_point_and_line_correspondences - a flexible
 * function that will find a camera matrix given any mix of point and line
 * correspondences. Takes all correspondences as arrays of Vectors, with an
 * additional value denoting the number of correspondences. 3d lines are given
 * as pairs of points on the line. This function will ahve some degree of error,
 * but can take any number of total correspondences provided there are at least
 * 6 total.
 *
 * Remaining function in this file are helpers to the above.
 *
 * Author: Andrew Emmott (aemmott)
 *
 * -------------------------------------------------------------------------- */

#include "l/l_gen.h"
#include "l/l_int_vector.h"
#include "m/m_gen.h"
#include "n/n_invert.h"
#include "n/n_diagonalize.h"
#include "sample/sample_misc.h"
#include "g/g_homogeneous_point.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int get_camera_matrix_from_point_and_line_correspondences
    (Matrix** camera, const Vector_vector* points2d, const Vector_vector* points3d, int numPoints,
     const Vector_vector* lines2d, const Vector_vector* lines3da, const Vector_vector* lines3db, int numLines);
int put_point_corrs(Matrix* fill, int rowOffset, const Vector* point2d, const Vector* point3d);
int put_line_corrs(Matrix* fill, int rowOffset, const Vector* line2d, const Vector* line3da, const Vector* line3db);
int ransac_calibrate_camera_from_corrs(Matrix** camera,double* error,const Vector_vector* points2d,const Vector_vector* points3d,
        int numPoints,const Vector_vector* lines2d,const Vector_vector* lines3da,const Vector_vector* lines3db,int numLines,
        int iterations, double tolerance);
double error_check_vectors(const Vector* vp1, const Vector* vp2, int i);

#define G_ add_error("g")

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

