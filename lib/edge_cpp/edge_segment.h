/* $Id: edge_segment.h 18301 2014-11-26 19:17:13Z ksimek $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Declarations for Edge segment class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef KJB_EDGE_CPP_EDGE_SEGMENT_H
#define KJB_EDGE_CPP_EDGE_SEGMENT_H

#include "edge/edge_base.h"
#include "gr_cpp/gr_line_segment.h"
#include "edge_cpp/edge.h"
#include <iosfwd>

namespace kjb {

/** @class Edge_segment Class to manipulate a line segment associated to an
 *  edge detected in an image. We fit a line segment to the edge points
 *  associated to an edge detected in the image. This class offers all
 *  the functionalities of a line segment, and contains a pointer
 *  to the edge points the line segment was fit to.
 *  The line segment is fit to the edge points using least squares,
 *  the error being computed as the sum of the squares of the perpendicular
 *  distances between each point and the fit line.
 */
class Edge_segment : public Line_segment
{
public:

    /**
     * @param iedge the detected image edge
     * @param use_num_pts_as_length     when this option is true, use the
     *                                  number of edge points as the length.
     */
    Edge_segment(
        const kjb::Edge & iedge,
        bool use_num_pts_as_length = true
    )
    :   Line_segment(),
        edge_pts(iedge)
    {
        fit_to_edge_points_with_least_squares(iedge, use_num_pts_as_length);
    }

    Edge_segment(
        const kjb::Edge & iedge,
        const kjb::Line_segment & ls
    )
    :   Line_segment(ls),
        edge_pts(iedge)
    {
        least_squares_fitting_error = 0.0;
        strength = 0.0;
    }

    /**
     * @param iedge the detected image edge
     * @param in the input stream to read this edge_segment from
     */
    Edge_segment(
        const kjb::Edge & iedge,
        std::istream& in
    );

    Edge_segment(const Edge_segment & src)
    :   Line_segment(src),
        least_squares_fitting_error(src.least_squares_fitting_error),
        strength(src.strength),
        edge_pts(src.edge_pts)
    {}

    Edge_segment & operator=(const Edge_segment & src)
    {
        Line_segment::operator=(src);
        least_squares_fitting_error = src.least_squares_fitting_error;
        strength = src.strength;
        edge_pts = src.edge_pts;
        return (*this);
    }

    ~Edge_segment() {}

    bool operator==(const Edge_segment & edg)
    {
        if(!Line_segment::operator==(edg))
        {
            return false;
        }
        if(fabs(strength - edg.strength) > FLT_EPSILON)
        {
            return false;
        }
        if( fabs(least_squares_fitting_error - edg.least_squares_fitting_error)
                > FLT_EPSILON
          )
        {
            return false;
        }

        return true;
    }

    /** @brief returns the set of edge points this line segment was fit to */
    inline const kjb::Edge & get_edge()
    {
        return edge_pts;
    }

    /** @ brief returns the ith edge point forming this edge segment */
    inline kjb::Edge_point get_edge_point(unsigned int i)
    {
        return edge_pts.get_edge_point(i);
    }

    /**
     * @brief returns the sum of square residuals between the edge points and
     *          the fitted line segment.
     */
    inline double get_least_squares_fitting_error() const
    {
        return least_squares_fitting_error;
    }

    /** @brief returns the average gradient magnitude of this edge */
    inline double get_strength() const
    {
        return strength;
    }

     /** @brief Reads this Edge_segment from an input stream. */
    void read(std::istream& in);

    /** @brief Writes this Edge_segment to an output stream. */
    void write(std::ostream& out) const;

    friend std::ostream& operator<<(std::ostream& out, const Edge_segment& es);

    /** @brief  Returns true if the edge_segments are overlapping, ie one
     *  is mostly contained in the other.
     */
    bool is_overlapping
    (
        const Edge_segment & es,
        double collinear_threshold = 0.12,
        double overlapping_threshold = 10
    )   const;

    /** @brief Sets the pointer to the edge points this edge_segment was fit to
     *
     * @param iedge_pts The pointer to the edge points
     */
    void set_edge(const kjb::Edge & iedge_pts)
    {
        edge_pts = iedge_pts;
    }


private:

    /** @brief fits this line segment to an input collection of edge points
     *  @param use_num_pts_as_length    when this option is true, use the
     *                                  number of edge points as the length.
     */
     void fit_to_edge_points_with_least_squares(
        const kjb::Edge & edge,
        bool use_num_pts_as_length = true
    );

    /** @brief The average square residual between the edge points and the
     *          fitted line segment
     */
    double least_squares_fitting_error;

    /** @brief The average edge strength */
    double strength;

    /** @brief A reference to the edge points this edge segment was fit to */
    kjb::Edge edge_pts;
};

std::ostream& operator<<(std::ostream& out, const Edge_segment& es);


} // namespace kjb
#endif /* KJB_EDGE_CPP_EDGE_SEGMENT_H */
