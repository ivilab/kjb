/* $Id */

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
|  Author: Luca Del Pero
* =========================================================================== */


#include "g_cpp/g_line.h"

namespace kjb
{


/**
 *  Constructor from two points on the line
 *
 *  @param point_1 The first point
 *  @param point_2 The second point
 */
Line::Line(const kjb::Vector & point_1, const kjb::Vector & point_2) : line_params(3, 1.0)
{
    if( (point_1.size() < 2) || (point_2.size() < 2) || (point_1.size() > 3) || (point_2.size() > 3))
    {
        KJB_THROW_2(Illegal_argument, "Build line from two points, points size is wrong");
    }
    double x1 = point_1(0);
    double y1 = point_1(1);
    double x2 = point_2(0);
    double y2 = point_2(1);

    if(point_1.size() == 3)
    {
        if( fabs(point_1(2) ) < DBL_EPSILON )
        {
            KJB_THROW_2(Illegal_argument, "Build line from two points, homogeneous coordinate is zero");
        }
        x1 /= point_1(2);
        y1 /= point_1(2);
    }
    if(point_2.size() == 3)
    {
        if( fabs(point_2(2) ) < DBL_EPSILON )
        {
            KJB_THROW_2(Illegal_argument, "Build line from two points, homogeneous coordinate is zero");
        }
        x2 /= point_2(2);
        y2 /= point_2(2);
    }

    double dx = x2 - x1;
    double dy = y2 - y1;

    if(fabs(dx) < DBL_EPSILON)
    {
        line_params(0) = 1.0;
        line_params(1) = 0.0;
        line_params(2) = -x1;
    }
    else if(fabs(dy) < DBL_EPSILON)
    {
        line_params(0) = 0;
        line_params(1) = 1.0;
        line_params(2) = -y1;
    }
    else
    {
        line_params(0) = -(dy/dx);
        line_params(1) = 1.0;
        line_params(2) = ( -line_params(0)*x1 ) - y1;
    }



}

/**
 * Finds the intersection between two lines.
 *
 * @param l1 the first line
 * @param l2 the second line
 * @param ints will store the intersection point
 *
 * @return true if the lines intercept, false if thet are parallel
 */
bool Line::find_line_intersection(const Line & l1, const Line & l2, kjb::Vector & ints)
{
    /**
     *  Given that line 1  and line 2 are defined as
     *  a1x + b1y + c1 = 0   line1
     *  a2x + b2y + c2 = 0   line2
     *
     *  x = (-b2c1 + b1d2)/(a1b2 - a2b1)
     *  y = (-a1c2 + a2c1)/(a1b2 - a2b1)
     *
     *  If (a1b2 - a2b1) is very close to zero, the lines are parallel and thus
     *  do not intercept -> we return false
     */

    double denominator = l1.get_a()*l2.get_b() - l2.get_a()*l1.get_b();
    if(fabs(denominator) < FLT_EPSILON)
    {
        return false;
    }

    /** We make sure the vector for storing the intersection point is big enough */
    if(ints.size() < 2)
    {
        ints.resize(2,0.0);
    }

    ints(0) = (-l2.get_b()*l1.get_c() + l1.get_b()*l2.get_c()) / denominator;
    ints(1) = (-l1.get_a()*l2.get_c() + l2.get_a()*l1.get_c()) / denominator;

    return true;
}

/**
 * Finds the perpendicular distance between a line and a point
 *
 * @param point the point to compute the distance for
 * @return the distance between the line and the point
 */
double Line::find_distance_to_point(const kjb::Vector & point) const
{
    if(point.size() < 2)
    {
        KJB_THROW_2(Illegal_argument, "Cannot find distance between line and point, input point vector size < 2");
    }

    return fabs(point(0)*line_params(0) + point(1)*line_params(1) + line_params(2))/
             sqrt(line_params(0)*line_params(0) + line_params(1)*line_params(1));

}


/** @brief Project a point onto a line 
 *  @return The projected point on the line
 */
Vector Line::project_point_onto_line(const Vector& point) const
{
    Vector proj_point(3, 1.0);
  
    if(fabs(get_a()) < FLT_EPSILON ) //horizontal line 
    {
        proj_point(0) = point(0);
        proj_point(1) = -get_c()/get_b(); 
    }
    else
        if(fabs(get_b()) < FLT_EPSILON) //vertical line
        {
            proj_point(0) = -get_c()/get_a();
            proj_point(1) = point(1);
        }
        else
        {
            double denominator = get_a()*get_a() + get_b()*get_b(); 
            proj_point(0) = (get_b()*get_b()*point(0) - get_a()*get_b()*point(1) - get_a()*get_c())/denominator;
            proj_point(1) = (get_a()*get_a()*point(1) - get_a()*get_b()*point(0) - get_b()*get_c())/denominator;
        }
    return proj_point;
}

/**
 * TODO: Document me!
 */
bool intersect_3D_line_with_plane
(
    kjb::Vector & intersection,
    double &t,
    const kjb::Vector & point,
    const kjb::Vector & direction,
    const kjb::Vector & plane
)
{
    if( (point.size() < 3) || (direction.size() < 3) || (plane.size() != 4) )
    {
        KJB_THROW_2(Illegal_argument, "Intersect 3D line with plane, bad inputs");
    }
    double dp1 = 0.0;
    double dp2 = 0.0;
    for(unsigned int i = 0; i < 3; i++)
    {
        dp1 += point(i)*plane(i);
        dp2 += plane(i)*direction(i);
    }
    if(fabs(dp2) < DBL_EPSILON )
    {
        /** The line does not intersect the plane **/
        return false;
    }
    t = - ( (plane(3) + dp1)/dp2 );
    intersection = point;
    for(unsigned int i = 0; i < 3; i++)
    {
        intersection(i) += t*direction(i);
    }
    if(intersection.size() > 3)
    {
        intersection(3) = 1.0;
    }

    return true;
}



} // namespace kjb
