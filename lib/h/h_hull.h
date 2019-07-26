
/* $Id: h_hull.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef H_HULL_INCLUDED
#define H_HULL_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define HULL_EPSILON   (1.0e-6)


/* =============================================================================
 *                                   Hull
 *
 * Convex hull type
 *
 * This type is the convex hull type for the KJB library. Most fields are self
 * explanatory. The rows of the matri *vertex_mp contain the
 * vertice vectors. Similarly, the normal vectors are the rows of the matrix
 * *normal_mp. The values *b_value_vp derived values. For each facet, they are
 * the dot products of the points on the facet with the normal. This dot product
 * should be the same for each facet point -- the routine find_convex_hull
 * computes the average of the values for each facet for improved accuracy. Each
 * facet of the hull is a space constraint expressed as X.N <= b, where b is the
 * value in the corresponding row of *b_value_vp.
 *
 * Each facet is a set of points forming the rows of a matrix in the matrix
 * array facets.
 *
 * Finally, if the geovmiew geometry is used, it is stored in the string
 * geom_view_geometry. This field is rarely used, as the geomview program did
 * not turn out to be that useful (we don't currently have a version on any
 * comonly used platform, as none were available when I was interested, perhaps
 * one is now)
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Hull
{
    int           dimension;
    int           num_vertices;
    int           num_facets;
    Matrix*       vertex_mp;
    Matrix*       normal_mp;
    Vector*       b_value_vp;
    Matrix_vector* facets;
    Vector*       facet_angles_vp;
    char*         geom_view_geometry;
    int           filled_resolution;
    double          min_x;
    double          min_y;
    double          min_z;
    double          max_x;
    double          max_y;
    double          max_z;
    double          filled_scale_x;
    double          filled_scale_y;
    double          filled_scale_z;
    unsigned char**       filled_2D_array;
    unsigned char***      filled_3D_array;
}
Hull;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#define HULL_VERTICES                 ((unsigned long)0x1)
#define HULL_NORMALS                 (((unsigned long)0x1)<<1)
#define HULL_B_VALUES                (((unsigned long)0x1)<<2)
#define HULL_FACETS                  (((unsigned long)0x1)<<3)
#define HULL_FILL                    (((unsigned long)0x1)<<4)
#define ORDER_2D_HULL_FACETS         (((unsigned long)0x1)<<5)
#define COMPUTE_2D_HULL_FACET_ANGLES (((unsigned long)0x1)<<6)
#define HULL_GEOM_VIEW_GEOMETRY      (((unsigned long)0x1)<<7)

#define DEFAULT_HULL_OPTIONS \
                  (HULL_VERTICES | HULL_NORMALS | HULL_B_VALUES | HULL_FACETS)


int set_hull_options(const char* option, const char* value);

Hull* create_hull(void);

int copy_hull(Hull** target_hp_ptr, const Hull* hp);

void free_hull(Hull*);

int get_convex_hull(Hull** hp_ptr, const Matrix* mp, unsigned long options);

Hull* find_convex_hull(const Matrix*, unsigned long);
int fill_hull(Hull* hp);

int is_hull_inside_hull(const Hull* outer_hp, const Hull* inner_hp);

int is_point_inside_hull(const Hull* hp, const Vector* test_vp);
int is_point_inside_hull_2
(
    const Hull*   hp,
    const Vector* test_vp,
    double        abs_tol,
    double        rel_tol,
    double*       error_ptr
);

int find_hull_bounds(const Hull* hp, Vector** min_vpp, Vector** max_vpp);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

