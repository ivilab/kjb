/* $Id: gr_line_segment.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include "gr_cpp/gr_line_segment.h"
#include <cmath>
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include "l_cpp/l_exception.h"
#include "i_cpp/i_image.h"
#include "g_cpp/g_line.h"
#include <sstream>
#include <ostream>


#define KJB_LS_MAX_RGB_VALUE 255

namespace kjb {

Matrix Line_segment::_line_segment_rotation(3,3);

/**
 * @param icentre_x the x coordinate of the centre
 * @param icentre_y the y coordinate of the centre
 * @param iorientation the orientation of the segment,
 *        defined as the angle between the x axis and the
 *        segment, positive meaning counter clockwise
 *        rotation. The value will be converted to
 *        the range [0-180] degrees
 * @param ilength the length of the segment
 */
Line_segment::Line_segment
(
    double      icentre_x, 
    double      icentre_y,
    double      iorientation, 
    double      ilength
) : Readable(),
    Writeable(),
    centre(3, 1.0),
    orientation(iorientation),
    length(ilength),
    start_point(3,1.0),
    end_point(3, 1.0),
    line()
{
    init_from_centre_and_orientation(icentre_x, icentre_y, iorientation, ilength);
}

Line_segment::Line_segment
(
    const Vector&   start, 
    const Vector&   end
) : Readable(),
    Writeable(),
    centre(3, 1.0),
    start_point(start), 
    end_point(end),
    line()
{
    if(start_point.size() == 3)
    {
        init_from_end_points(start_point[0]/start_point[2], start_point[1]/start_point[2], end_point[0]/end_point[2], end_point[1]/end_point[2]);
    }
    else
    {
        init_from_end_points(start_point[0], start_point[1], end_point[0], end_point[1]);
    }
}

Line_segment::Line_segment(std::istream & in)
:   Readable(),
    Writeable(),
    centre(3,1.0),
    start_point(3,1.0),
    end_point(3,1.0),
    line()
{
    read(in);
}

/**
 * @param ls the line segment to copy into this one
 */
Line_segment::Line_segment
(
    const Line_segment & ls
) : Readable(), Writeable(),
    centre(ls.centre), start_point(ls.start_point),end_point(ls.end_point), line(ls.line)
{
    orientation = ls.orientation;
    length = ls.length;
}

/**
 * @param ls the line segment to assign to this one
 */
Line_segment & Line_segment::operator=(const Line_segment & ls)
{
    centre = ls.centre;
    orientation = ls.orientation;
    length = ls.length;

    start_point = ls.start_point;
    end_point = ls.end_point;

    line = ls.line;

    return (*this);

}

/**
 * @param ls the line segment to compare to this one
 */
bool Line_segment::operator==(const Line_segment & ls) const
{
    if( ! (centre == ls.centre) ) return false;
    if( ! (start_point == ls.start_point) ) return false;
    if( ! (end_point == ls.end_point) ) return false;
    if( ! (line.get_params() == ls.line.get_params()) ) return false;
    if( ( fabs(orientation - ls.orientation)) > FLT_EPSILON  ) return false;
    if( ( fabs(length - ls.length)) > FLT_EPSILON ) return false;

    return true;

}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 * @param icentre_x the x coordinate of the centre
 * @param icentre_y the y coordinate of the centre
 * @param iorientation the orientation of the segment,
 *        defined as the angle between the x axis and the
 *        segment, positive meaning counter clockwise
 *        rotation. The value will be converted to
 *        the range [0-180] degrees
 * @param ilength the length of the segment
 */
void Line_segment::init_from_centre_and_orientation(double icentre_x, double icentre_y, double iorientation, double ilength)
{
    centre(0) = icentre_x;
    centre(1) = icentre_y;

    length = ilength;
    /** Convert the orienation so that it ranges between 0 and 180 degrees. */
    orientation = fmod(iorientation, M_PI);
    if(orientation < 0)
    {
        orientation = M_PI + orientation;
    }

    compute_line_parameters();
    compute_extrema();
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 * @param x_start the x coordinate of the first endpoint
 * @param x_end the x coordinate of the second endpoint
 * @param im slope of the line segment
 * @param ib y intercept
 */
void Line_segment::init_from_slope_and_intercept(double x_start, double x_end, double islope, double iintercept)
{
    if(x_start > x_end)
    {
        double temp = x_start;
        x_start = x_end;
        x_end = temp;
    }

    /**
     * We convert a line with slope m and y intercept q
     * (y = mx + q)
     * to the line form ax+by+c =0. This takes the
     * form -mx + y - q =0
     */
    line.set_a(-islope);
    line.set_b(1.0);
    line.set_c(-iintercept);

    /** We now compute the line segment extrema */
    start_point(0) = x_start;
    end_point(0) = x_end;
    centre(0) = (x_start + x_end)/2.0;

    start_point(1) = line.compute_y_coordinate(start_point(0));
    end_point(1) = line.compute_y_coordinate(end_point(0));
   
    centre(1) = line.compute_y_coordinate(centre(0));

    length = sqrt( (start_point(0) - end_point(0))*(start_point(0) - end_point(0)) + (start_point(1) - end_point(1))*(start_point(1) - end_point(1))  );

    /** atan returns a value between [-pi/2, pi/2],
     * we want it between [0, pi]
     */
    orientation = atan(-islope);
    if(orientation < 0)
    {
        orientation += M_PI;
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 * This function inits this line segment as a vertical segment
 * defined by the x location and a start and end point along the
 * y direction.
 *
 * @param y_start the y coordinate of the bottom of this segment
 * @param y_end the y coordinate of the first endpoint
 * @param x the x coordinate of the second endpoint
 */
void Line_segment::init_vertical_segment(double y_start, double y_end, double x)
{

    if(y_start > y_end)
    {
        double temp = y_start;
        y_start = y_end;
        y_end = temp;
    }

    centre(0) = x;
    start_point(0) = x;
    end_point(0) = x;

    centre(1) = (y_end + y_start) /2.0;
    start_point(1) = y_start;
    end_point(1) = y_end;

    orientation = M_PI/2.0;

    length = fabs(y_end - y_start);

    line.set_a(1.0);
    line.set_b(0.0);
    line.set_c(-x);

}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/**
 * @param x_1 the x coordinate of the first end point
 * @param y_1 the y coordinate of the first end point
 * @param x_2 the x coordinate of the second end point
 * @param y_2 the y coordinate of the second end point
 */
void Line_segment::init_from_end_points(double x_1, double y_1, double x_2, double y_2)
{
    using namespace std;

    if( (fabs(x_1 - x_2) < FLT_EPSILON ) && (fabs(y_1 - y_2) < FLT_EPSILON ) )
    {
        throw KJB_error("Init line segment, ends point coincide!");
    }

    if(fabs(x_1 - x_2) < FLT_EPSILON)
    {
        //Vertical segment
        init_vertical_segment(y_1, y_2, x_1);
    }
    else
    {
        //Regular segment
        double _slope = (y_2 - y_1)/(x_2 - x_1);
        double _intercept = -_slope*x_2 + y_2;
        init_from_slope_and_intercept(x_1, x_2, _slope, _intercept);
    }

}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */

/** @brief Reads this Line_segment from an input stream. */
void Line_segment::read(std::istream& in)
{
    using std::istringstream;

    const char* field_value;

    // Line segment centre
    if (!(field_value = read_field_value(in, "centre")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Line segment Centre");
    }
    istringstream ist(field_value);
    ist >> centre(0) >> centre(1) >> centre(2);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid line segment centre");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "orientation")))
    {
        KJB_THROW_2(Illegal_argument, "Missing orientation");
    }
    ist.str(field_value);
    ist >> orientation;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Orientation");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "length")))
    {
        KJB_THROW_2(Illegal_argument, "Missing length");
    }
    ist.str(field_value);
    ist >> length;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Length");
    }
    ist.clear(std::ios_base::goodbit);

    compute_line_parameters();
    compute_extrema();
}

/** @brief Writes this Line_segment to an output stream. */
void Line_segment::write(std::ostream& out) const
{
    out << " centre: " << centre(0) << ' ' << centre(1) << ' ' << centre(2) << '\n'
        << " orientation:" << orientation << '\n'
        << " length:" << length << '\n';
}

/** Computes the extrema from the line segment parameters.
 *  It initializes the extrema so that the segment is aligned
 *  with the x axis , centred int the origin and has the correct length.
 *  It then rotates it so that it has the correct orientation, and finally
 *  translates it to the reight position*/
void Line_segment::compute_extrema()
{
    //kjb::Matrix rotation = Matrix::create_2d_homo_rotation_matrix(orientation);
    _line_segment_rotation.convert_to_2d_homo_rotation_matrix(orientation);
    start_point(0) = - length/2.0;
    end_point(0) =  + length/2.0;
    start_point(1) = 0.0;
    end_point(1) = 0.0;

    /*start_point = rotation*start_point;
    end_point = rotation*end_point;*/
    start_point = _line_segment_rotation*start_point;
    end_point = _line_segment_rotation*end_point;

    start_point(0) += centre(0);
    end_point(0) +=  centre(0);
    start_point(1) += centre(1);
    end_point(1) += centre(1);

    /** As a convention we store in start point the leftmost point */
    if(end_point(0) < start_point(0))
    {
        kjb::Vector temp = end_point;
        end_point = start_point;
        start_point = temp;
    }

    if( is_vertical() )
    {
        if(end_point(1) < start_point(1))
        {
            kjb::Vector temp = end_point;
            end_point = start_point;
            start_point = temp;
        }
    }
}

/** Computes the slope and the y-axis intercept of this segment */
void Line_segment::compute_line_parameters()
{
    if(fabs(M_PI_2 - orientation) < FLT_EPSILON)
    {
        // The line equation is in the form x = c
        line.set_a(1.0);
        line.set_b(0.0);
        line.set_c(-centre(0));
    }
    else
    {
        line.set_a(-tan(-orientation));
        line.set_b(1.0);
        line.set_c( - (-get_slope()*centre(0) + centre(1) ));
    }
}

/**
 * Draws the line segment on an image
 * 
 * @param img the image
 * @param ir the red channel
 * @param ig the green channel
 * @param ib the blue channel
 * @param width the width of the segment to be drawn (in pixel)
 */
void Line_segment::draw(kjb::Image & img, double ir, double ig, double ib,  double width)  const
{
    if(!img.c_ptr())
    {
        throw kjb::Illegal_argument("Bad input image");
    }
    kjb_c::image_draw_segment_2(img.non_const_c_ptr(), start_point(1), start_point(0), end_point(1), end_point(0), width, ir,ig,ib);
}

/**
 * Draws the line segment using a random color
 *
 * @param img the image
 * @param width the width of the segment to be drawn (in pixel)
 */
void Line_segment::randomly_color(kjb::Image & img, double width)  const
{
    using namespace kjb_c;

    double tempr =(double)kjb_rand()*KJB_LS_MAX_RGB_VALUE;
    double tempg =(double)kjb_rand()*KJB_LS_MAX_RGB_VALUE;
    double tempb =(double)kjb_rand()*KJB_LS_MAX_RGB_VALUE;

    // We don't want the segment to be black
    if( (tempr < 50.0) && (tempg < 50.0) && (tempb < 50.0) )
    {
        tempr = 255;
    }

    draw(img, tempr, tempg, tempb, width);
}

/**
 * For debugging purposes. It checks that the line segment parameters
 * are consistent
 */
bool Line_segment::is_line_segment_consistent()
{
    double myepsilon = FLT_EPSILON*1000;
    double derived_length = sqrt( (start_point(0) - end_point(0))*(start_point(0) - end_point(0)) +
                                  (start_point(1) - end_point(1))*(start_point(1) - end_point(1)) );
    double diff = fabs(derived_length - length);
    if( diff > FLT_EPSILON)
    {
        return false;
    }

    double centre_start_dist = sqrt( (start_point(0) - centre(0))*(start_point(0) - centre(0))
                                    + (start_point(1) - centre(1))*(start_point(1) - centre(1)) );

    diff = fabs(centre_start_dist - (length/2.0));
    if( diff > FLT_EPSILON)
    {
        return false;
    }

    double centre_end_dist = sqrt( (end_point(0) - centre(0))*(end_point(0) - centre(0))
                                 + (end_point(1) - centre(1))*(end_point(1) - centre(1)) );
    diff = fabs(centre_end_dist - (length/2.0));
    if( diff > FLT_EPSILON)
    {
        return false;
    }

    if(start_point(0) > end_point(0))
    {
        return false;
    }

    if(is_vertical())
    {
        diff = fabs(start_point(0) - end_point(0));
        if( diff > FLT_EPSILON)
        {
            return false;
        }
        if(fabs(line.get_a() - 1.0) > FLT_EPSILON)
        {
            return false;
        }
        if(fabs(line.get_b()) > FLT_EPSILON)
        {
            return false;
        }
        if(fabs(-line.get_c() - end_point(0)) > FLT_EPSILON)
        {
            return false;
        }

        return true;
    }

    double derived_centre_y = line.compute_y_coordinate(centre(0));
    double derived_start_y = line.compute_y_coordinate(start_point(0));
    double derived_end_y = line.compute_y_coordinate(end_point(0));

    diff = fabs(derived_centre_y - centre(1));
    if( diff > FLT_EPSILON)
    {
        return false;
    }

    diff = fabs(derived_start_y - start_point(1));
    if( diff > myepsilon)
    {
        return false;
    }

    diff = fabs(derived_end_y - end_point(1));
    if( diff > myepsilon)
    {
        return false;
    }

    return true;
}

/**
 * @brief Check whether point is inside the ending points of this segment 
 *  Note point is collinear with this line segment 
 */
bool Line_segment::point_outside_segment(const Vector& point) const
{
    return (find_t(point) > 1||find_t(point) < 0);
}

/**
 * Returns the distance between this line segment and the input point.
 * There are two case:
 * 1) The perpendicular projection of the point onto the line the segment lies on
 * falls onto the line segment. In this case we return this perpendicular distance
 * 2) Otherwise, we return the smallest distance between one of the segment end
 * points and the input point
 *
 * @param point the input point
 */
double Line_segment::get_distance_from_point (const kjb::Vector& point) const
{
    if(point.size() < 2)
    {
        KJB_THROW_2(Illegal_argument, "Distance between line segment and point, point vector size smaller than 2");
    }
    
    double vx = start_point(0) - point(0);
    double vy = start_point(1) - point(1);

    double ux = end_point(0) - start_point(0);
    double uy = end_point(1) - start_point(1);

    double det = (-vx*ux)+(-vy*uy); //if this is < 0 or > length then its outside the line segment
    if(det<0 || det>length*length)
    {
        ux = end_point(0) - point(0);
        uy = end_point(1) - point(1);
        return sqrt(std::min<float>(vx*vx+vy*vy, ux*ux+uy*uy));
    }
    return line.find_distance_to_point(point);

}

/** If we interpret the end points p1 and p2 of this line segment
 * as the two points defining the parametric equation of
 * a line, given an input point p, this function returns
 * the parameter t such that:
 * p = p2 + t (p2 -p1)
 * Notice that this works even if p is not on the line,
 * its projection on it will be considered.
 *
 * @param point The point to compute t for
 */
double Line_segment::find_t(const kjb::Vector & point) const
{
    if(point.size() < 2)
    {
        KJB_THROW_2(Illegal_argument, "Distance between line segment and point, point vector size smaller than 2");
    }
    double vx = start_point(0) - point(0);
    double vy = start_point(1) - point(1);

    double ux = end_point(0) - start_point(0);
    double uy = end_point(1) - start_point(1);

    return  ( (-vx*ux)+(-vy*uy) )/(length*length);
}

/**
 * Returns the distance between this line segment and the input point.
 * There are two case:
 * 1) The perpendicular projection of the point onto the line the segment lies on
 * falls onto the line segment. In this case we return this perpendicular distance
 * 2) Otherwise, we return the smallest distance between one of the segment end
 * points and the input point
 *
 * @param point the input point
 * @return the perpendicular distance between the line the segment lies on and
 * the point. If we are in case (1) these two distances are the same
 */
double Line_segment::get_distance_from_point (const kjb::Vector& point, double * perp_dist) const
{
    if(point.size() < 2)
    {
        KJB_THROW_2(Illegal_argument, "Distance between line segment and point, point vector size smaller than 2");
    }
    double vx = start_point(0) - point(0);
    double vy = start_point(1) - point(1);

    double ux = end_point(0) - start_point(0);
    double uy = end_point(1) - start_point(1);

    double det = ( (-vx*ux)+(-vy*uy) ); //if this is < 0 or > length then its outside the line segment
    if(det<0 || det> (length*length))
    {
        ux = end_point(0) - point(0);
        uy = end_point(1) - point(1);
        (*perp_dist) =  line.find_distance_to_point(point);
        return sqrt(std::min<float>(vx*vx+vy*vy, ux*ux+uy*uy));
    }

    (*perp_dist) =  line.find_distance_to_point(point);
    return (*perp_dist);

}

/**
 * @brief Calculate the angle between this line segment and the input line_segment
 * @return the calculated angle  
 */ 
double Line_segment::get_angle_between_line (const Line_segment& line_segment) const
{
    Vector v1 = get_start() - get_end();
    Vector v2 = line_segment.get_start() - line_segment.get_end();
    double norm_dot = dot(v1, v2)/(v1.magnitude() * v2.magnitude());
    
    //Make sure the absolute value of the normalized dot product is not greater than 1.0 
    if (norm_dot > 1.0)
        norm_dot = 1.0;
    else if(norm_dot < -1.0)
        norm_dot = -1.0;
    double angle = acos(norm_dot);
   
    //Return the smaller angle of the two line segment 
    if(angle > M_PI_2)
        return M_PI-angle;
    else 
        return angle;

}

/**
 * Returns a vector representing the normalized direction of
 *  this line_segment
 *
 *  @param idirection This will contain the line segment direction
 */
void Line_segment::get_direction(kjb::Vector & idirection) const
{
    if(idirection.size() != 3)
    {
        idirection.resize(3);
    }
    idirection(0) = end_point(0) - start_point(0);
    idirection(1) = end_point(1) - start_point(1);
    idirection(2) = 1.0;
    double _norm = sqrt(idirection(0)*idirection(0) + idirection(1)*idirection(1));
    idirection(0) = idirection(0) / _norm;
    idirection(1) = idirection(1) / _norm;
}

std::ostream & operator<<(std::ostream& out, const Line_segment& ls)
{
    out << " centre:" << ls.get_centre_x() << " | " << ls.get_centre_y() << " orientation:"
            << ls.get_orientation() << " length:" << ls.get_length()  << " start:" << ls.get_start_x()
            << " | " << ls.get_start_y() << "  end:" << ls.get_end_x() << " | " << ls.get_end_y() << " slope: "
            << ls.get_slope();
    if(!ls.is_vertical())
    {
        out << "  intercept:" << ls.get_y_intercept() << std::endl;
    }
    else
    {
        out << " vertical segment x = " << ls.get_centre_x() << std::endl;
    }
    return out;
}

/** Finds the intersection between the line this line_segment lies ont
 *  and the line the input line_segment lies on. Return false if these
 *  two lines are parallel and therefore do not intersect
 *
 *  @param _line The input line_segment
 *  @param point_p Will contain the intersection point
 *  @return false if the lines do not intesect
 */
bool Line_segment::get_intersection
(
    const Line_segment&      _line,
    kjb::Vector&             point_p
) const
{
    return Line::find_line_intersection(_line.get_line(), line, point_p);
}

/** @brief Project a line segment onto a line 
 *  @return std::pair<Vector, Vector> The projected two ending points of the line segment
 *  
 * (not fully tested) 
 */ 
std::pair<Vector, Vector> Line_segment::project_line_segment_onto_line
(
    const Line_segment&     segment,
    const Line&             iline
) 
{
    std::pair<Vector, Vector> end_points;
    Vector start = iline.project_point_onto_line(segment.get_start());
    Vector end = iline.project_point_onto_line(segment.get_end());
    
    // Make sure the start point has smaller x position
    if(start[0] > end [0] || (start[0] == end[0] && start[1] > end[1]))
    {
        Vector temp = start; 
        start = end;
        end = temp;
    }


    end_points = std::make_pair(start, end);
    return end_points;
}


/** @brief Project a line segment onto a line segment
 *  @param segment_project The line segment that is being projected
 *  @param segment_target The line segment that is being projected to 
 *  @param projected_points The projected two ending points of the line segment
 *  @return True If both projected_points are inside the ending points of segment_target, 
 *          False If one or both the projected_points are outside the ending points
 *          the segment_target
 */
bool Line_segment::project_line_segment_onto_line_segment
(
    const Line_segment&         segment_project, 
    const Line_segment&         segment_target,
    std::pair<Vector, Vector>&  projected_points,
    double&                     length_inside,
    double&                     length_outside
)
{
    length_inside = 0.0;
    length_outside = 0.0;

    projected_points = Line_segment::project_line_segment_onto_line(segment_project, segment_target.get_line()); 
    
    Vector start = projected_points.first;
    Vector end = projected_points.second;

    if(segment_target.point_outside_segment(start))
    {
        double length1 = (start-segment_target.get_start()).magnitude();
        double length2 = (start-segment_target.get_end()).magnitude();
        //the smaller distance is the length outside the segment_target 
        length_outside = length1 < length2 ? length1 : length2;
    }
    
    if(segment_target.point_outside_segment(end))
    {
        double length1 = (end-segment_target.get_start()).magnitude();
        double length2 = (end-segment_target.get_end()).magnitude();
        //the smaller distance is the length outside the segment_target 
        length_outside += length1 < length2 ? length1 : length2;
    }
    
    length_inside = (start-end).magnitude()-length_outside;

    if(length_outside > 0.0)
        return false;
    else 
        return true;
}

/** @brief Returns true if point1.x < point2.x || (point1.x == point2.x &&
 *  point1.y < point2.y 
 */
 bool Line_segment::less_than (const Vector& point1, const Vector& point2)
{
    if(point1.size() < 2 || point2.size() < 2)
    {
        throw kjb::Illegal_argument("Invalid input point with for Line::less_than ");
    }
    
    return (point1[0] < point2[0]) || (point1[0] == point2[0] && point1[1] < point2[1]);
}

bool Line_segment::collision_detection
(
    double start_1,
    double end_1,
    double start_2,
    double end_2,
    int & direction,
    double & delta
)
{
    if(end_2 < start_1)
    {
        /*** Case 1
         *        |--------|
         *  |---|
         */
        return false;
    }

    if(start_2 > end_1)
    {
        /*** Case 2
         *        |--------|
         *  |---|
         */
        return false;
    }

    if(start_2 < start_1)
    {
        if(end_2 < end_1)
        {
            /*** Case 3
             *    |--------|
             *  |---|
             */
            delta = fabs(end_2-start_1);
            direction = -1;
            return true;
        }
        else
        {
            /*** Case 4
             *      |-----|
             *  |------------|
             */
            delta = -1.0;
            direction = 1;
            /*if( fabs(end_2 - end_1) > fabs(start_1 - start_2))
            {
                direction = 1;
                delta = fabs(end_1-start_2);
            }
            else
            {
                delta = fabs(end_2-start_1);
                direction = -1;
            }*/

            return true;
        }
    }

    if(start_2 < end_1)
    {
        if(end_2 < end_1)
        {
            /*** Case 5
             *  |------------|
             *      |-----|
             */
            /*delta = -1.0;
            direction = 1;*/
            if( fabs(end_2 - end_1) > fabs(start_1 - start_2))
            {
                direction = 1;
                delta = fabs(end_1-start_2);
            }
            else
            {
                delta = fabs(end_2-start_1);
                direction = -1;
            }
            return true;
        }
        else
        {
            /*** Case 6
             *  |------|
             *      |---------|
             */
            direction = 1;
            delta = fabs(end_1-start_2);
            return true;
        }
    }

    return false;
}

double Line_segment::get_overlap
(
    double start_1,
    double end_1,
    double start_2,
    double end_2
)
{
    if(end_2 < start_1)
    {
        /*** Case 1
         *        |--------|
         *  |---|
         */
        return 0.0;
    }

    if(start_2 > end_1)
    {
        /*** Case 2
         *        |--------|
         *  |---|
         */
        return 0.0;
    }

    if(start_2 < start_1)
    {
        if(end_2 < end_1)
        {
            /*** Case 3
             *    |--------|
             *  |---|
             */
            return fabs(end_2-start_1);
        }
        else
        {
            /*** Case 4
             *      |-----|
             *  |------------|
             */
            return fabs(end_1-start_1);;
        }
    }

    if(start_2 < end_1)
    {
        if(end_2 < end_1)
        {
            /*** Case 5
             *  |------------|
             *      |-----|
             */
             return fabs(end_2-start_2);
        }
        else
        {
            /*** Case 6
             *  |------|
             *      |---------|
             */
            return fabs(end_1-start_2);
        }
    }

    return 0.0;
}

bool Line_segment::collision_detection_with_direction
(
    double start_1,
    double end_1,
    double start_2,
    double end_2,
    int & direction,
    double & delta,
    int idirection
)
{
    if(end_2 < start_1)
    {
        /*** Case 1
         *        |--------|
         *  |---|
         */
        return false;
    }

    if(start_2 > end_1)
    {
        /*** Case 2
         *        |--------|
         *  |---|
         */
        return false;
    }

    if(start_2 < start_1)
    {
        if(end_2 < end_1)
        {
            /*** Case 3
             *    |--------|
             *  |---|
             */
            delta = fabs(end_2-start_1);
            direction = -1;
            return true;
        }
        else
        {
            /*** Case 4
             *      |-----|
             *  |------------|
             */
            delta = -1.0;
            direction = 1;
            return true;
        }
    }

    if(start_2 < end_1)
    {
        if(end_2 < end_1)
        {
            /*** Case 5
             *  |------------|
             *      |-----|
             */
            if(idirection == 1)
            {
                direction = 1;
                delta = fabs(end_1-start_2);
            }
            else
            {
                direction = -1;
                delta = fabs(end_2-start_1);
            }
            return true;
        }
        else
        {
            /*** Case 6
             *  |------|
             *      |---------|
             */
            direction = 1;
            delta = fabs(end_1-start_2);
            return true;
        }
    }

    return false;
}

bool Line_segment::is_collinear
(
    const Line_segment & ls,
    double collinear_threshold
) const
{
    kjb::Vector v1(3,0.0);
    kjb::Vector v2(3,0.0);
    get_direction(v1);
    ls.get_direction(v2);
    double cp = v1(0)*v2(1) -   v1(1)*v2(0);
    if(fabs(cp) > collinear_threshold)
    {
        return false;
    }
    return true;
}

} // namespace kjb
