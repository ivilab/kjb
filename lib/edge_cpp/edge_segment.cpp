/* $Id: edge_segment.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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


/* $Id $ */

/**
 * @file
 *
 * @author Luca Del Pero
 *
 * @brief Line segment class
 *
 * Copyright (c) 2003-2008 by members of University of Arizona Computer Vision
 * group. For use outside the University of Arizona Computer Vision group
 * please contact Kobus Barnard.
 */

#include "l/l_math.h"
#include "l/l_error.h"
#include "edge_cpp/edge_segment.h"
#include "gr_cpp/gr_line_segment.h"
#include "i/i_draw.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_exception.h"
#include "n/n_diagonalize.h"
#include <sstream>
#include <istream>
#include <ostream>
//#include <limits>

#define PI_TIMES_1_POINT_5  M_PI*1.5
#define VERTICAL_THRESHOLD 1
#define KJB_LS_MAX_RGB_VALUE 255
#define OVERLAPPING_COLLINEARITY_THRESHOLD 0.12
#define OVERLAPPING_MAX_DIST 10 

using namespace kjb;


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Edge_segment::Edge_segment(
    const kjb::Edge & iedge,
    std::istream& in
)
:   Line_segment(),
    edge_pts(iedge)
{
    read(in);
}

/** Fits the line segments to an input collection of edge points
 *  using least square. Here we minimize the square sum of the perpendicular
 *  distance between each point and the fitted line using
 *  homogeneous linear least squares
 *
 * @param edge the collection of edge points to fit this edge to
 * @param use_num_pts_as_length when this option is true, use the number of edge points as the length  
 */
void Edge_segment::fit_to_edge_points_with_least_squares
(
    const kjb::Edge &      edge,
    bool use_num_pts_as_length
)
{

    if(edge.get_num_points() == 0)
    {
        throw kjb::Illegal_argument("Invalid edge with zero points");
    }

    double avg_x = 0;
    double avg_y = 0;
    double m = 0;
    double error = 0;
    double num_points = edge.get_num_points();

    Matrix A(2, num_points);

    strength = 0;

    for(unsigned int i = 0; i < num_points; i++)
    {
         double col = edge.get_edge_point(i).get_col();
         double row = edge.get_edge_point(i).get_row();
         avg_x += col;
         avg_y += row;
         strength +=  edge.get_edge_point(i).get_gradient_magnitude();
    }
    avg_x /= num_points;
    avg_y /= num_points;

    for(unsigned int i = 0; i < num_points; i++)
    {
         A(0, i) = edge.get_edge_point(i).get_col() - avg_x;
         A(1, i) = edge.get_edge_point(i).get_row() - avg_y;
    }

    Matrix AA = A*A.transpose();
    kjb_c::Matrix * E_mp = 0;
    kjb_c::Vector * D_vp = 0;

    if( kjb_c::diagonalize(AA.get_c_matrix(), &E_mp, &D_vp) != kjb_c::NO_ERROR)
    {
        throw KJB_error("Line segment fitting, matrix diagonalization failed");
    }

    double denominator = E_mp->elements[1][1];
    double ilength = 0;
    if(use_num_pts_as_length)
    {
        ilength = num_points;
    }
    else
    {
         double start_x = edge.get_edge_point(0).get_col();
         double end_x = edge.get_edge_point(num_points - 1).get_col();
         double start_y = edge.get_edge_point(0).get_row();
         double end_y = edge.get_edge_point(num_points - 1).get_row();
         ilength = sqrt( (start_x - end_x)*(start_x - end_x) + (start_y - end_y)*(start_y - end_y) );
    }
    if(fabs(denominator) < FLT_EPSILON)
    {
       //Vertical segment
       init_from_centre_and_orientation(avg_x, avg_y, M_PI_2, ilength);
       orientation = M_PI_2;

       /** Since the line is vertical, the perpendicular distance is
        *  the horizontal distance between the line and the edge point
        */
       double temp_error = 0;
       for(unsigned int i = 0; i < num_points; i++)
       {
           temp_error = (edge.get_edge_point(i).get_col() - avg_x);
           error += temp_error*temp_error;
       }
       error /= num_points;
    }
    else
    {
        //Regular segment
        double d = E_mp->elements[0][1]*avg_x +E_mp->elements[1][1]*avg_y;
        m = -E_mp->elements[0][1]/denominator;
        init_from_centre_and_orientation(avg_x, avg_y, -atan(m), ilength);

        double temp_error = 0;
        Vector tempv(2,0.0);
        //Compute the error of line least square fitting
        for(unsigned int i = 0; i < num_points; i++)
        {
            temp_error = (E_mp->elements[0][1]*edge.get_edge_point(i).get_col()
                         + E_mp->elements[1][1]*edge.get_edge_point(i).get_row() - d);
            //tempv(0) = edge.get_edge_point(i).get_col();
            //tempv(1) = edge.get_edge_point(i).get_row();
            ///temp_error = get_distance_from_point(tempv);

            error += (temp_error*temp_error);
        }
        error /= num_points;
    }

    kjb_c::free_matrix(E_mp);
    kjb_c::free_vector(D_vp);

    least_squares_fitting_error = error;
    strength /= num_points;
}

/**
 *  Reads this Edge_segment from an input stream.
 *
 * @param in The input stream to read this edge segment from
 */
void Edge_segment::read(std::istream& in)
{
    using std::istringstream;

    const char* field_value;

    Line_segment::read(in);

    if (!(field_value = read_field_value(in, "strength")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Edge segment strength");
    }
    istringstream ist(field_value);
    ist >> strength;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid line segment strength");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "least_squares_fitting_error")))
    {
        KJB_THROW_2(Illegal_argument, "Missing least squares fitting error");
    }
    ist.str(field_value);
    ist >> least_squares_fitting_error;
    if (ist.fail() || (least_squares_fitting_error < 0))
    {
        KJB_THROW_2(Illegal_argument, "Invalid least squares fitting error");
    }
    ist.clear(std::ios_base::goodbit);
}

/**
 * Writes this Edge_segment to an output stream.
 *
 * @param The output stream to write this Edge segment to
 */
void Edge_segment::write(std::ostream& out) const
{
    Line_segment::write(out);

    out << " strength: " << strength << '\n'
        << " least_squares_fitting_error: " << least_squares_fitting_error << '\n';
}

/** Returns true if the edge_segments are overlapping, ie one
 *  is mostly contained in the other.
 *  Two segments are considered overlapping if their cross product
 *  is close to zero (meaning they are parallel) AND they are very
 *  close to each other AND the projection of one almost
 *  fully falss onto the other
 *
 *  @param es The edge segment to check against
 */
bool Edge_segment::is_overlapping
(
    const Edge_segment & es, 
    double collinear_threshold,
    double overlapping_threshold 
) const
{
    kjb::Vector v1(3,0.0);
    kjb::Vector v2(3,0.0);
    get_direction(v1);
    es.get_direction(v2);

    /** Check if they are parallel */
    double cp = v1(0)*v2(1) -   v1(1)*v2(0);
    if(fabs(cp) > collinear_threshold)
    {
        return false;
    }


    double t = find_t(es.get_start());
    double startt = 0;
    double endt = 1.0;
    double max_dist = overlapping_threshold;

    /** Check if they are close enough and check their projection */
    double perp_dist;
    get_distance_from_point(es.get_start(), &perp_dist);
    if( t < startt || t > endt  || (fabs(perp_dist) > max_dist)  )
    {
        t = find_t(es.get_end());
        get_distance_from_point(es.get_end(), &perp_dist);
        if( t < startt || t > endt || (fabs(perp_dist) > max_dist) )
        {
            t = es.find_t(get_start());
            es.get_distance_from_point(get_start(), &perp_dist);
            if( t < startt || t > endt || (fabs(perp_dist) > max_dist) )
            {
                t = es.find_t(get_end());
                es.get_distance_from_point(get_end(), &perp_dist);
                if( t < startt || t > endt  || (fabs(perp_dist) > max_dist))
                {
                    return false;
                }
            }

        }
    }
    return true;
    //return (v1.get_max_abs_difference(v2) < 0.001); //->good value

    /*Line_segment templs;
    templs.init_from_end_points(centre(0), centre(1), es.get_centre_x(), es.get_centre_y());
    if( fabs(templs.get_orientation() - es.get_orientation()) > 0.3)
    {
        return false;
    }
    if( fabs(templs.get_orientation() - get_orientation()) > 0.3)
    {
        return false;
    }
    return true;*/
}

std::ostream& kjb::operator<<(std::ostream& out, const Edge_segment& es)
{
    out << "Least squares error: " << es.least_squares_fitting_error << std::endl;
    out << "Strength: " << es.strength << std::endl;
    out << (const Line_segment&)es;
    return out;
}

