/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Declarations for Line class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#ifndef EDGE_LINE_H
#define EDGE_LINE_H

#include "m_cpp/m_vector.h" 
#include "m_cpp/m_vector_d.h" 
#include "l_cpp/l_exception.h"
#include <limits>

namespace kjb {
/**
 * @class Line
 *
 * @brief Parametric representation of a 2D line in terms of
 *   three parameters (a,b,c) (as in ax+by+c = 0).
 */
class Line
{
    public:

        /** @brief Constructor without initializations */
        Line() : line_params(3, 1.0) {}

        /** @brief Constructor from two points on the line */
        Line(const kjb::Vector & point_1, const kjb::Vector & point_2);
        
        /** @brief Constructs a new Line_segment from line parameters a, b, c
         * as in ax + by + c = 0 */
        Line(double a, double b, double c) : line_params(3, 1.0)
        {
            line_params(0) = a;
            line_params(1) = b;
            line_params(2) = c;
        }

        /** @brief Constructs a new Line_segment from line parameters vector [a, b, c]
         * as in ax + by + c = 0 */
        Line(const kjb::Vector & iparams)
        {
            if(iparams.size() != 3)
            {
                throw kjb::Illegal_argument("Line constructor, input parameter vector must have size 3");
            }
            line_params = iparams;
        }

        /** @brief Copy constructor */
        Line(const Line & l) : line_params(l.line_params) { }

        /** @brief Assignment operator */
        Line & operator=(const Line & l)
        {
            line_params = l.line_params;
            return (*this);
        }

        /** @brief Returns the a parameter of the line as in  ax + by + c = 0*/
        inline double get_a() const
        {
            return line_params(0);
        }

        /** @brief Returns the b parameter of the line as in  ax + by + c = 0*/
        inline double get_b() const
        {
            return line_params(1);
        }

        /** @brief Returns the c parameter of the line as in  ax + by + c = 0*/
        inline double get_c() const
        {
            return line_params(2);
        }
        
        /** @brief Returns the line parameters */
        inline const kjb::Vector & get_params() const
        {
            return line_params;
        }

        /** @brief computes the y coordinate for the input x coordinate */
        double compute_y_coordinate(double ix)
        {
            if( fabs(line_params(1)) < FLT_EPSILON )
            {
                throw kjb::KJB_error("Cannot compute y coordinate for a vertical line");
            }
            return (-line_params(0)*ix - line_params(2))/line_params(1);
        }

        /** @brief Sets the a parameter of the line as in  ax + by + c = 0*/
        inline void set_a(double ia)
        {
            line_params(0) = ia;
        }

        /** @brief Sets the b parameter of the line as in  ax + by + c = 0*/
        inline void set_b(double ib)
        {
            line_params(1) = ib;
        }
        
        /** @brief Sets the c parameter of the line as in  ax + by + c = 0*/
        inline void set_c(double ic)
        {
            line_params(2) = ic;
        }
        
        /** @brief Sets the line parameters [a,b,c] of the line as in  ax + by + c = 0*/
        inline void set_line_params(const kjb::Vector & iparams)
        {
            if(iparams.size() != 3)
            {
                throw kjb::Illegal_argument("Line constructor, input parameter vector must have size 3");
            }
            line_params = iparams;
        }

        inline bool point_is_on_line(const Vector & point) const
        {
            if(point.size() < 2)
            {
                throw kjb::Illegal_argument("point_is_on_line(), point's size < 2");
            }

            return fabs(get_a()*point(0)+get_b()*point(1)+get_c()) < FLT_EPSILON;
        }

        /** @brief Find the intersection between two lines, Returns false
         *  if the lines are parallel
         */
        static bool find_line_intersection(const Line & l1, const Line & l2, kjb::Vector & ints);

        /** @brief Finds the perpendicular distance between a line and a point */
        double find_distance_to_point(const kjb::Vector & point) const;

        /** @brief Project a point onto a line 
         *  @return The projected point on the line
         */
        Vector project_point_onto_line(const Vector& point) const;
   
    private:

        /** @brief X coordinate of the centre of this line segment */
        kjb::Vector line_params;

};


bool intersect_3D_line_with_plane
(
    kjb::Vector & intersection,
    double &t,
    const kjb::Vector & point,
    const kjb::Vector & direction,
    const kjb::Vector & plane
);


/** 
 * Do not call directly.
 */
template <class VectorType>
double intersect_line_with_plane_dispatch_
(
    const VectorType& line_point,
    const VectorType& line_direction,
    const VectorType& plane_point,
    const VectorType& plane_normal
)
{
    double numerator = kjb::dot((plane_point - line_point), plane_normal);
    double denominator = kjb::dot(line_direction, plane_normal);

    if(fabs(denominator) < FLT_EPSILON)
    {
        if(fabs(numerator) > FLT_EPSILON)
            return std::numeric_limits<double>::infinity();
        else
            return std::numeric_limits<double>::quiet_NaN();
    }
    return numerator/denominator;
}


/**
 * Intersection of a D-dimensional line and plane, using point-vector
 * representation for both.  No exceptions are thrown for degenerate cases;
 * instead, mathematically sensible sentinal values are returned (see 
 * notes below for details).
 *
 * @param line_point A point on the line
 * @param line_direction The line's direction vector (not necessarilly unit-length)
 * @param plane_point A point on the plane
 * @param plane_normal The plane's normal (must be unit length (?))
 *
 * @return The scalar t such that the intersection point p can be obtained by: 
 *      p = line_point + t * line_direction;
 *
 * @note If no intersection occurs (line and plane are parallel), 
 *       we say the intersection occurs at infinity, and 
 *       std::numeric_limits<double>::infinity() is returned.
 *
 * @note If plane and line are coincident, intersection occurs at all
 *       points along the line, and no single solution exists. In this
 *       case, std::numeric_limits<double>::quiet_NaN() is returned.
 */
inline double intersect_line_with_plane
(
    const kjb::Vector& line_point,
    const kjb::Vector& line_direction,
    const kjb::Vector& plane_point,
    const kjb::Vector& plane_normal
)
{
    return intersect_line_with_plane_dispatch_(
            line_point,
            line_direction,
            plane_point,
            plane_normal);
}

/**
 * @brief intersect a D-dimensional line an plane, using point-vector representation. (static version)
 */
template <std::size_t D>
inline double intersect_line_with_plane
(
    const kjb::Vector_d<D>& line_point,
    const kjb::Vector_d<D>& line_direction,
    const kjb::Vector_d<D>& plane_point,
    const kjb::Vector_d<D>& plane_normal
)
{
    return intersect_line_with_plane_dispatch_(
            line_point,
            line_direction,
            plane_point,
            plane_normal);
}

/**
 * @brief   Project a point onto a line, in any dimension.
 *
 * @param   A   End-point of the line.
 * @param   B   End-point of the line.
 * @param   X   Point to project.
 */
inline
Vector project_point_onto_line(const Vector& A, const Vector& B, const Vector& P)
{
    IFT(A.size() == B.size() && A.size() == P.size(), Illegal_argument,
        "Cannot project point onto line: dimensions are wrong.");

    Vector AP = P - A;
    Vector AB = B - A;

    double s = dot(AP, AB) / dot(AB, AB);
    return A + s*AB;
}

/**
 * @brief   Get distance between two skew lines. Returns a negative number
 *          if lines are parallel.
 */
inline
double skew_lines_distance
(
    const Vector& x1,
    const Vector& x2,
    const Vector& y1,
    const Vector& y2
)
{
    Vector dx = x2 - x1;
    Vector dy = y2 - y1;

    Vector n = cross(dx, dy);
    double mag = n.magnitude();

    if(mag < DBL_EPSILON)
    {
        return -1.0;
    }

    n /= mag;

    return fabs(dot(n, y1 - x1));
}

/**
 * @brief   Get distance between two skew lines. Returns a negative number
 *          if lines are parallel.
 */
inline
std::pair<double, double> skew_lines_intersection
(
    const Vector& x1,
    const Vector& y1,
    const Vector& x2,
    const Vector& y2
)
{
    Vector d1 = y1 - x1;
    Vector d2 = y2 - x2;

    Vector n = cross(d1, d2);
    double mag = n.magnitude();

    if(mag < DBL_EPSILON)
    {
        return std::make_pair(DBL_MAX, DBL_MAX);
    }

    n /= mag;

    double a1 = dot(d1, d1);
    double a2 = dot(d2, d2);
    double b = dot(d1, d2);
    double c1 = dot(x1 - x2, d1);
    double c2 = dot(x1 - x2, d2);

    double u1, u2;
    if(b == 0)
    {
        u1 = -c1/a1;
        u2 = c2/a2;
    }
    else
    {
        u1 = (a2*c1 - b*c2)/(b*b - a1*a2);
        u2 = (a1*u1 + c1)/b;
    }

    return std::make_pair(u1, u2);
}

} // namespace kjb

#endif

