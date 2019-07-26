/* $Id: gr_line_segment.h 21599 2017-07-31 00:44:30Z kobus $ */
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

#ifndef GR_CPP_LINE_SEGMENT_H
#define GR_CPP_LINE_SEGMENT_H

#include <utility>
#include <iosfwd>
#include "l_cpp/l_readable.h"
#include "l_cpp/l_writeable.h"
#include "m_cpp/m_vector.h"
#include "g_cpp/g_line.h"

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Declarations for Line segment class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

namespace kjb {

class Image;

/**
 * @class Line_segment
 *
 * @brief Class to manipulate a line segment
 * The class is parametrized in terms the position
 * of the centre, its length and orientation.
 * This is thus compatible with the output of the Berkeley
 * edge detector. We store also the start point, the end point
 * and the line parameters describing this line segment
 * (for more details on how the line parameters work,
 * please see the class line).
 * This is redundant information, but it is convenient
 * to have all these parameters precomputed and at hand.
 */
class Line_segment : public Readable, public Writeable
{
public:

    /** @brief Constructor without initializations */
    Line_segment()
    :   Readable(),
        Writeable(),
        centre(3,1.0),
        start_point(3,1.0),
        end_point(3,1.0),
        line()
    {}

    /// @brief Constructs a new Line_segment from centre, orientation, length
    Line_segment(
        double icentre_x,
        double icentre_y,
        double iorientation,
        double ilength
    );

    /// @brief Constructs a new Line_segment from a starting and ending point
    Line_segment(const Vector& start, const Vector& end);

    /// @brief Constructs this line segment by reading from an input stream
    Line_segment(std::istream & in);

    /** @brief Copy constructor */
    Line_segment(const Line_segment & ls);

    /** @brief Assignment operator */
    Line_segment & operator=(const Line_segment & ls);

    bool operator ==(const Line_segment& ls) const;

    inline bool operator <(const Line_segment& ls) const
    {
        return      (this->get_start_x() < ls.get_start_x())
                ||  (       this->get_start_x() == ls.get_start_x()
                        &&  this->get_start_y() < ls.get_start_y()
                    );
    }

    /**
     * @brief Initialize from x_start, x_end, slope, y_axis intercept.
     *
     * Notice that a vertical segment cannot be initialized with this function.
     */
    void init_from_slope_and_intercept(
        double x_start,
        double x_end,
        double islope,
        double iintercept
    );

    /// @brief Initialize a vertical Line_segment from y_start, y_end, x coord.
    void init_vertical_segment(double y_start, double y_end, double x);

    /// @brief Initialize a new Line_segment from position of its end points
    void init_from_end_points(double x_1, double y_1, double x_2, double y_2);

    void init_from_centre_and_orientation(
        double icentre_x,
        double icentre_y,
        double iorientation,
        double ilength
    );

    /** @brief Returns the x-coordinate of the mid-point */
    inline double get_centre_x() const
    {
        return centre(0);
    }

    /** @brief Returns the y-coordinate of the mid-point */
    inline double get_centre_y() const
    {
        return centre(1);
    }

    /**
     * @brief Check whether point is inside the ending points of this segment
     *  Note point is collinear with this line segment
     */
    bool point_outside_segment(const Vector& point) const;

    /** @brief Returns the mid-point */
    const kjb::Vector & get_centre() const { return centre; }

    /** @brief Returns the x-coordinate of the leftmost point of the segment*/
    inline double get_start_x() const
    {
        return start_point(0);
    }

    /** @brief Returns the y-coordinate of the leftmost point of the segment */
    inline double get_start_y() const
    {
        return start_point(1);
    }

    /** @brief Returns the leftmost point of the segment */
    const kjb::Vector & get_start() const { return start_point; }

    /// @brief Returns the x-coordinate of the rightmost point of the segment
    inline double get_end_x() const
    {
        return end_point(0);
    }

    /// @brief Returns the y-coordinate of the rightmost point of the segment
    inline double get_end_y() const
    {
        return end_point(1);
    }

    /** @brief Returns the rightmost point of the segment */
    const kjb::Vector & get_end() const { return end_point; }

    /**
     * @brief Returns the slope of the line segment.
     *
     * If the segment is vertical, the slope tends to infinity.
     */
    inline double get_slope() const
    {
        if(is_vertical())
        {
            return std::numeric_limits<double>::max() - DBL_EPSILON;
        }
        return -line.get_a();
    }

    /**
     * @brief Returns the y-intercept of the line segment.
     *
     * If the segment is vertical there is not such interception.
     */
    inline double get_y_intercept() const
    {
        if(is_vertical())
        {
            throw KJB_error("Vertical segment, there is no y intercept");
        }
        return -line.get_c();
    }

    /**
     * @brief Returns the x-intercept of the line segment.
     *
     * If the segment is horizontal there is not such interception.
     */
    inline double get_x_intercept() const
    {
        if(is_horizontal())
        {
            throw KJB_error("Horizontal segment, there is no x intercept");
        }
        return -line.get_c()/line.get_a();
    }

    inline const Vector & get_line_params() const
    {
        return line.get_params();
    }

    inline const Line & get_line() const
    {
        return line;
    }

    /** @brief Returns the orientation */
    inline double get_orientation() const
    {
        return orientation;
    }

    /** @brief Returns the length */
    inline double get_length() const
    {
        return length;
    }

    /** @brief Reads this Line segment from an input stream. */
    void read(std::istream& in);

    /** @brief Writes this Line segment to an output stream. */
    void write(std::ostream& out) const;

    /** @brief Draws this line segment */
    void draw(
        kjb::Image & img,
        double ir,
        double ig,
        double ib,
        double width = 1.0
    )  const;

    /** @brief Randomly colors this line segment on an image */
    void randomly_color(kjb::Image & img, double width = 1.0)  const;

    /** @brief Finds the intersection point of the this line_segment
     *  and the input Line_segment line. Return false if these
     *  two lines are parallel and therefore do not intersect
     *  @param line input Line_segment
     *  @param point the intersection point
     */
    bool get_intersection
    (
        const Line_segment&      line,
        kjb::Vector&             point
    ) const;

    /// @brief Returns distance between this line segment and the input point.
    double get_distance_from_point (const kjb::Vector& point) const;

    /** @brief Returns the angle between this line segment and the input line*/
    double get_angle_between_line (const Line_segment& line) const;

    /** If we interpret the end points p1 and p2 of this line segment
     * as the two points defining the parametric equation of
     * a line, given an input point p, this function returns
     * the parameter t such that:
     * p = p2 + t (p2 -p1)
     * Notice that this works even if p is not on the line,
     * its projection on it will be considered.
     */
    double find_t(const kjb::Vector & point) const;

    /// @brief Returns distance between this line segment and the input point.
    double get_distance_from_point (
        const kjb::Vector& point,
        double * perp_dist
    ) const;

    /** @brief Returns the dx of this segment */
    inline double get_dx() const
    {
        return  fabs(end_point(0) - start_point(0) );
    }

    /** @brief Returns the dy of this segment */
    inline double get_dy() const {return  (end_point(1) - start_point(1) ); }

    friend std::ostream& operator<<(std::ostream&, const Line_segment&);

    /** @brief Function object of testing whehter a line is horizontal */
    inline bool is_horizontal() const
    {
        return (fabs(orientation) <= FLT_EPSILON );
    }

    /** @brief Function object of testing whehter a line is horizontal */
    inline bool is_vertical() const
    {
        //This is a little hacky but I could not find a more elegant solution
        //if(fabs(line.get_b()) <= FLT_EPSILON)
        if(fabs(get_orientation()-M_PI_2) <= FLT_EPSILON)
        {
            return true;
        }
        return false;
    }

    /** @brief Returns a vector representing the normalized direction of
     * this line_segment */
    void get_direction(kjb::Vector & idirection) const;

    /**
     * For debug purposes, checks this line segment is consistent
     */
    bool is_line_segment_consistent();

    /**
     * @brief Project a line segment onto a line
     * @return The projected two ending points of the line segment
     */
    static std::pair<Vector, Vector> project_line_segment_onto_line
    (
        const Line_segment&     segment,
        const Line&             iline
    );

    /** @brief Project a line segment onto a line segment
     *  @param segment_project The line segment that is being projected
     *  @param segment_target The line segment that is being projected to
     *  @param projected_points The projected two ending points of the line
     *                          segment.
     *  @return True If both projected_points are inside the ending points of
     *          segment_target, False If one or both the projected_points are
     *          outside the ending points the segment_target.
     */
    static bool project_line_segment_onto_line_segment
    (
        const Line_segment&         segment_project,
        const Line_segment&         segment_target,
        std::pair<Vector, Vector>&  projected_points,
        double&                     length_inside,
        double&                     length_outside
    );

    /** @brief Returns true if point1.x < point2.x || (point1.x == point2.x &&
     *  point1.y < point2.y
     */
    static bool less_than (const Vector& point1, const Vector& point2);


    static bool collision_detection
    (
        double start_1,
        double end_1,
        double start_2,
        double end_2,
        int & direction,
        double & delta
    );

    static bool collision_detection_with_direction
    (
        double start_1,
        double end_1,
        double start_2,
        double end_2,
        int & direction,
        double & delta,
        int idirection
    );

    static double get_overlap
    (
        double start_1,
        double end_1,
        double start_2,
        double end_2
    );

    bool is_collinear
    (
        const Line_segment & ls,
        double collinear_threshold
    ) const;

protected:

    /**
     * @brief Computes the extrema of this line segments from
     * centre, orientation, and length.
     * This information is redundant, but it can be very useful
     * to have them pre-computed.
     */
    void compute_extrema();

    /** Computes the slope and the y-axis intercept of this segment
     *  This information is redundant, but it can be very useful
     *  to have it pre-computed.
     */
    void compute_line_parameters();


    /** @brief X coordinate of the centre of this line segment */
    kjb::Vector centre;

    /** @brief Orientation of this line segment, defined as the angle between
     *  the x axis and the segment, positive angle meaning counter clockwise.
     *  Ranges between 0 and 180 degrees. Stored in radian */
    double orientation;

    /** @brief length of the line segment */
    double length;

    /**
     * Start and end point of the line segment.
     * The following fields are redundant, but it can
     * be helpful to have them pre-computed.
     */
    kjb::Vector start_point;
    kjb::Vector end_point;

    /**
     * Line parameters.
     * The following parameters are redundant, but it can
     * be helpful to have them pre-computed.
     */
    Line line;

    static Matrix _line_segment_rotation;

};

std::ostream& operator<<(std::ostream& out, const Line_segment& ls);

} // namespace kjb
#endif /* GR_CPP_LINE_SEGMENT_H */
