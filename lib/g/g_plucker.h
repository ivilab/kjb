/* =============================================================================
 *
 * NOTE: This is a header file.
 *
 * This file contains functions related to creating and manipulating plucker
 * lines.
 *
 * Function Summary:
 *
 *
 *  int plucker_line_from_points
 *      (Vector *point_one, Vector *point_two, Vector **_vpp);
 *  Creates a 6-vector plucker line from the elements in two 4-vectors
 *  representing two points in three-dimensional space.
 *
 *  int plucker_line_from_planes
 *      (Vector *plane_one, Vector *plane_two, Vector **_vpp);
 *  Creates a 6-vector plucker line from the elements in two 4-vectors
 *  representing two planes in three-dimensional space.
 *
 *  int invert_plucker_line
 *      (Vector *line, Vector **_vpp);
 *  Creates a 6-vector plucker line that is the dual representation of an
 *  existing plucker line. (Inverts the order of the elements).
 *
 *  int ow_invert_plucker_line
 *      (Vector *line);
 *  Directly changes the given plucker line, inverting the order of the elements,
 *  changing it to the dual representation of itself.
 *
 *  int plucker_line_intersect
 *      (Vector *line_one, Vector *line_two);
 *  A boolean check to see if two plucker lines intersect. Returns 0 if false and
 *  1 if true. If either Vector is not a plucker line, will also return 0.
 *
 *  int is_plucker_line
 *      (Vector *_vp);
 *  A boolean check to see if the given Vector is a plucker line. Returns 0 if
 *  false and 1 if true.
 *
 *  int line_projection_matrix_from_planes
 *      (Vector *plane_one, Vector *plane_two, Vector *plane_three, Matrix **_mpp);
 *  Creates a 3x6 Matrix that is the line projection matrix derived from three
 *  4-vectors representing three planes.
 *
 * Author: Andrew Emmott (aemmott)
 *
 * -------------------------------------------------------------------------- */

#ifndef G_PLUCKER_H
#define G_PLUCKER_H

#include "l/l_gen.h"
#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int plucker_line_from_points(Vector **plucker_line, const Vector *point_one, const Vector *point_two);
int plucker_line_from_planes(Vector **plucker_line, const Vector *plane_one, const Vector *plane_two);
int invert_plucker_line(Vector **plucker_line, const Vector *line);
int ow_invert_plucker_line(Vector *line);
int plucker_line_intersect(const Vector *line_one, const Vector *line_two);
int is_plucker_line(const Vector *plucker_line);
int line_projection_matrix_from_planes(Matrix **line_projection_matrix, const Vector *plane_one, const Vector *plane_two, const Vector *plane_three);
int line_projection_matrix_from_point_project_matrix(Matrix **line_projection_matrix, const Matrix *point_projection_matrix);
int forward_line_projection(Vector **target_vector, const Matrix *line_projection_matrix, const Vector *plucker_line);

#define G_PLUCKER_ALLOC_ERROR add_error("g_plucker's call to get_target_vector failed.")
#define G_PLUCKER_NULL_ERROR add_error("g_plucker was passed a null Vector.")
#define G_PLUCKER_MAT_SIZE_ERROR add_error("g_plucker given an inappropriately sized line projection matrix.")
#define G_PLUCKER_SIZE_ERROR add_error("g_plucker given a Vector of inappropriate size.")
#define G_PLUCKER_MAT_ALLOC_ERROR add_error("g_plucker's call to get_target_matrix failed.")

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
