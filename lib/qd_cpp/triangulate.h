/**
 * @file
 * @brief declarations to monotonize and triangulate polygons
 * @author Andrew Predoehl
 *
 * This is an implementation of the algorithm of Lee and Preparata, "Location
 * of a point in a planar subdivision and its applications," (SIAM J. Comp.,
 * vol. 6, pp. 594-606, 1977), but based on the exposition in de Berg et al.,
 * Computational Geometry/2e (Springer, 2000) and Laszlo,
 * Computational Geometry and Computer Graphics in C++ (Prentice-Hall, 1996).
 *
 * See de Berg et al. section 3.2 for a discussion of the definition of
 * "y-monotone" for a simple polygon.  Basically it means that if you were
 * filling the polygon by using horizontal rasters, every raster would be a
 * connected set -- no gaps -- regardless of scale or location.  This is
 * sometimes called a bitonic path.  A one-legged person with no arms would
 * probably have a y-monotone silhouette, but two legs or any arms would
 * probably spoil the y-axis monotonicity.
 * The "helper" terminology (in the implementation here)
 * comes from de Berg et al.
 *
 * A monotone polygon is easy to triangulate.  This make-them-monotone step
 * is often just an intermediate phase on the way to full triangulation.
 * This code can monotonize an arbitrary planar subdivision represented by
 * a finite DCEL.  Triangulation isn't working yet but it is designed to
 * do likewise.
 *
 * This also implements an algorithm to triangulate a y-monotone polygon.
 * The algorithm is by Garey, Johnson, Preparata and Tarjan, "Triangulating a
 * simple polygon," (Info. Proc. Lett., 7:175-179, 1978).  My implementation
 * is based on the exposition in de Berg et al. (op cit.).
 * The time complexity of each is O(n log n) where n is the complexity of
 * the face.  Unfortunately the merge step we use is still quadratic time,
 * so until that is fixed, the time complexity is not so great.
 *
 * We can easily measure the area of a triangulated polygon, so there are
 * functions to do that as well declared in this header.
 */
/*
 * $Id: triangulate.h 20139 2015-11-30 05:50:36Z predoehl $
 */

#ifndef QD_CPP_MONOTONE_H_INCLUDED_IVILAB
#define QD_CPP_MONOTONE_H_INCLUDED_IVILAB 1

#include <qd_cpp/dcel.h>

namespace kjb
{
namespace qd
{

/// @brief test whether a DCEL face, specified by index, is y-monotone.
bool is_face_ymonotone(const Doubly_connected_edge_list&, size_t);


#if MONOTONE_CPP_DEBUG
std::string db_fence_labels(
    const Doubly_connected_edge_list&,
    size_t,
    float=1.0f
);
#endif


/// @brief return list of edges needed to make a face y-monotone.
std::vector< RatPoint_line_segment > edges_to_ymonotonize(
    const Doubly_connected_edge_list&,
    size_t
);


/**
 * @brief return list of edges needed to triangulate a face
 *
 * The algorithm is by Garey, Johnson, Preparata and Tarjan, "Triangulating a
 * simple polygon," (Info. Proc. Lett., 7:175-179, 1978).  My implementation
 * is based on the exposition in de Berg et al., Computational Geometry/2e
 * (Springer, 2000).
 */
std::vector< RatPoint_line_segment > edges_to_triangulate(
    const Doubly_connected_edge_list&,
    size_t
);


/**
 * @brief return a DCEL like the input but with face fi now y-monotone.
 *
 * This is an implementation of the algorithm of Lee and Preparata, "Location
 * of a point in a planar subdivision and its applications," (SIAM J. Comp.,
 * vol. 6, pp. 594-606, 1977), based on the exposition in de Berg et al.,
 * Computational Geometry/2e (Springer, 2000) and Laszlo,
 * Computational Geometry and Computer Graphics in C++ (Prentice-Hall, 1996).
 */
Doubly_connected_edge_list make_a_face_ymonotone(
    const Doubly_connected_edge_list& dcel,
    size_t fi
);


/// @brief return a DCEL like the input but with all faces now y-monotone.
Doubly_connected_edge_list make_faces_ymonotone(
    const Doubly_connected_edge_list& dcel
);


/// @brief test whether a face is a triangle (without inner components).
bool is_face_triangle(const Doubly_connected_edge_list&, size_t);


RatPoint::Rat area_of_face(const Doubly_connected_edge_list&, size_t);


}
}

#endif /* QD_CPP_MONOTONE_H_INCLUDED_IVILAB */

