/* $Id: g_hull.h 19025 2015-05-08 15:05:48Z ernesto $                                                                       */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author: Jinyan Guan 
 * =========================================================================== */

#ifndef KJB_G_CPP_WRAP_HULL_H
#define KJB_G_CPP_WRAP_HULL_H

#include <h/h_hull.h>
#include <h/h_ave.h>

#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <g_cpp/g_util.h>

#include <vector>
#include <algorithm>

namespace kjb
{
/**
 * @brief Get the convex hull of points 
 * hull_vertices stores the vertices of the convex hull
 * hull_facets stores the facets of the hull
 * 
 */
int get_convex_hull
(
    const Matrix& points,
    Matrix& hull_vertices,
    std::vector<Matrix>& hull_facets
);

/**
 * @brief Compute the intersections of the vector of points
 * @param pts is a vector points on the convex hulls
 */
bool intersect_hulls
(   
    const std::vector<Matrix>& pts,
    Matrix& hull_vertices,
    std::vector<Matrix>& hull_facets
);

/**
 * @brief Get the volume of the convex hull of points
 */
inline 
double get_convex_hull_volume ( const kjb::Matrix& points)
{
    using namespace kjb_c;
    double volume = 0.0;
    Hull* hp = NULL;
    kjb_c::Vector* ave_hull = NULL;
    EPETE(get_convex_hull(&hp, points.get_c_matrix(), DEFAULT_HULL_OPTIONS));
    EPETE(get_hull_CM_and_volume(hp, &ave_hull, &volume));

    free_vector(ave_hull);
    free_hull(hp);
    return volume;
}

/**
 * @brief   Find the box with the smallest area containing a set of points.
 *
 * @param   first   Iterator to first point.
 * @param   last    One-past-the end iterator.
 * @param   out     Output iterator where sequence of points representing
 *                  the corners of the box will be placed.
 */
template<class PtIter, class OutIter>
void min_bounding_box_2d(PtIter first, PtIter last, OutIter out)
{
    // first find convex hull
    const size_t N = std::distance(first, last);
    Matrix points((int)N, 2);
    for(size_t i = 0; first != last; ++first, ++i)
    {
        points.set_row(i, *first);
    }

    Matrix hverts;
    std::vector<Matrix> hfaces;
    get_convex_hull(points, hverts, hfaces);

    // find the rectangle
    double min_area = DBL_MAX;
    std::vector<Vector> corners(4);
    const size_t M = hfaces.size();
    for(size_t i = 0; i < M; ++i)
    {
        // get angle of current edge of hull
        Vector e = hfaces[i].get_row(1) - hfaces[i].get_row(0);
        e.normalize();
        double a = -acos(e[0]);

        // rotate all vertices by the angle
        Matrix R = geometry::get_rotation_matrix(a);
        Matrix Rt = matrix_transpose(R);
        Matrix rverts = hverts * Rt;

        // find axis-aligned bounding box
        Vector x = rverts.get_col(0);
        Vector y = rverts.get_col(1);
        double left = *std::min_element(x.begin(), x.end());
        double right = *std::max_element(x.begin(), x.end());
        double bot = *std::min_element(y.begin(), y.end());
        double top = *std::max_element(y.begin(), y.end());
        double area = (right - left) * (top - bot);

        if(area < min_area)
        {
            min_area = area;

            double cx = (left + right)/2;
            double cy = (bot + top)/2;
            double w = right - left;
            double h = top - bot;
            corners[0] = Rt*Vector(cx - w/2, cy - h/2);
            corners[1] = Rt*Vector(cx + w/2, cy - h/2);
            corners[2] = Rt*Vector(cx + w/2, cy + h/2);
            corners[3] = Rt*Vector(cx - w/2, cy + h/2);
        }
    }

    std::copy(corners.begin(), corners.end(), out);
}

} //namespace kjb

#endif

