/* =============================================================================
 * 
 * Note: This is a header file.
 *
 * g_point_projection includes functions pertaining to creating point projection
 * matrices from plucker line representations/line projection matrices.
 *
 * int points_from_plucker(Vector **point_one, Vector **point_two, const Vector *plucker_line) -
 * Derives two points, (As 4-vectors), from a plucker line representation.
 *
 * int distance_between_lines_as_points(double *distance, int *is_parallel, const Vector *line_one_point_one,
 *         const Vector *line_one_point_two, const Vector *line_two_point_one, const Vector *line_two_point_two) -
 * Calculates the distance between two lines. Used to check for intersection and
 * parallelism.
 *
 * int plane_from_lines_as_points(Vector **plane, const Vector *line_one_point_one,
 *         const Vector *line_one_point_two, const Vector *line_two_point_one, const Vector *line_two_point_two) -
 * Will create a 4-vector representation of a plane from four coplanar points.
 *
 * int plane_from_plucker_lines(Vector **plane, const Vector *plucker_one, const Vector *plucker_two) -
 * Derives a 4-vector plane from two plucker line representations.
 *
 * int point_projection_matrix_from_line_projection_matrix(Matrix **point_projection_matrix, const Matrix *line_projection_matrix) -
 * Creates a 3x4 point projection matrix from a 3x6 line projection matrix.
 *
 * Author: Andrew Emmott (aemmott)
 *
 * -------------------------------------------------------------------------- */

#ifndef G_POINT_PROJECTION_H
#define G_POINT_PROJECTION_H

#include "l/l_gen.h"
#include "m/m_gen.h"
#include "g/g_plucker.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int points_from_plucker(Vector **point_one, Vector **point_two, const Vector *plucker_line);
int distance_between_lines_as_points(double *distance, int *is_parallel, const Vector *line_one_point_one,
        const Vector *line_one_point_two, const Vector *line_two_point_one, const Vector *line_two_point_two);
int plane_from_lines_as_points(Vector **plane, const Vector *line_one_point_one,
        const Vector *line_one_point_two, const Vector *line_two_point_one, const Vector *line_two_point_two);
int plane_from_plucker_lines(Vector **plane, const Vector *plucker_one, const Vector *plucker_two);
int point_projection_matrix_from_line_projection_matrix(Matrix **point_projection_matrix, const Matrix *line_projection_matrix);

#define G_POINT_PROJECTION_SIZE_ERROR add_error("g_point_projection given a vector of inappropriate length.")
#define G_POINT_PROJECTION_MAT_SIZE_ERROR add_error("g_point_projection given a matrix of inappropriate length.")
#define G_POINT_PROJECTION_ALLOC_ERROR add_error("g_point_projection's call to get_target_vector failed.")
#define G_POINT_PROJECTION_MAT_ALLOC_ERROR add_error("g_point_projection's call to get_target_matrix failed.")
#define G_POINT_PROJECTION_PLUCKER_ERROR add_error("g_point_projection was given an invalid plucker line.")
#define G_POINT_PROJECTION_NOT_COPLANAR add_error("g_point_projection was given lines that are not coplanar.")
#define G_POINT_PROJECTION_NORM_ERROR add_error("g_point_projection was given an un-homogenized vector.")

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
