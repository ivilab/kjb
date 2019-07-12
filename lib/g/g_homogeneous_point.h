/* =============================================================================
 *
 * NOTE: This is a header file.
 *
 * The functions in this file are meant to offer a number of ways to homogenize,
 * vectors. All functions add error messages as appropriate, but
 * leave error-handling to the caller of the function. Vectors with a z-value of
 * zero will cause errors.
 *
 * Function Summary:
 *    ow_homogenize_vector - changes the values of the vector directly,
 *        leaving z = 1.0. Returns ERROR or NO_ERROR as appropriate.
 *    homogenize_vector - places the homogenized values into a given Vector.
 *    Returns ERROR or NO_ERROR as appropriate.
 *
 * Author: Andrew Emmott (aemmott)
 *
 * -------------------------------------------------------------------------- */

#ifndef G_HOMOGENEOUS_POINT_H
#define G_HOMOGENEOUS_POINT_H

#include "l/l_gen.h"
#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int ow_homogenize_vector(Vector *vector_to_homogenize);
int ow_homogenize_vector_at_index(Vector *vector_to_homogenize, int index);
int homogenize_vector(Vector **homogenized_vector_address, const Vector *vector_to_homogenize);
int homogenize_vector_at_index(Vector **homogenized_vector_address, const Vector *vector_to_homogenize, int index);

#define G_HOMOGENEOUS_POINT_ALLOC_ERROR add_error("g_homogeneous_point's call to get_target_vector failed.")
#define G_HOMOGENEOUS_POINT_NULL_ERROR add_error("g_homogeneous_point was passed a null Vector.")
#define G_HOMOGENEOUS_POINT_Z_ERROR add_error("g_homogeneous_point given a Vector with a z-value of 0.")

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif


#endif
