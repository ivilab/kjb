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


#include "edge_cpp/manhattan_world.h"
#include "edge_cpp/segment_pair.h"
#include "i/i_draw.h"
#include <g_cpp/g_orthogonal_corner.h>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace kjb;

/**
 * Draws a line on the image between the segment mid point and the vanishing point
 * the segment converges to
 *
 * @param img the image
 * @param ir the red channel
 * @param ig the green channel
 * @param ib the blue channel
 * @param width the width of the segment to be drawn (in pixel)
 */
void Manhattan_segment::draw_mid_point_to_vanishing_point
(
    kjb::Image & img,
    double ir,
    double ig,
    double ib,
    double width
)  const
{
    if(_outlier)
    {
        return;
    }


    double _x = _vp->get_x();
    double _y = _vp->get_y();
    switch(_vp->get_type())
    {
        case Vanishing_point::REGULAR:
            break;
        case Vanishing_point::INFINITY_DOWN :
            _x = _segment.get_centre_x();
            _y = 1000000;
            break;
        case Vanishing_point::INFINITY_UP :
            _x = _segment.get_centre_x();
            _y = -1000000;
            break;
        case Vanishing_point::INFINITY_RIGHT :
            _x = 1000000;
            _y = _segment.get_centre_y();
            break;
        case Vanishing_point::INFINITY_LEFT :
            _x = -1000000;
            _y = _segment.get_centre_y();
            break;
        default:
            break;
    }

    kjb_c::image_draw_segment_2(img.non_const_c_ptr(), _segment.get_centre_y(),_segment.get_centre_x(),
         _y, _x, width, ir,ig,ib);
}

unsigned int Manhattan_corner::num_available_segments() const
{
    unsigned int counter = 0;
    for(unsigned int i = 0; i < 3; i++)
    {
        if(available[i])
        {
            counter++;
        }
    }
    return counter;
}

void Manhattan_corner::draw(kjb::Image & img, bool draw_full_segments, double width) const
{
    if(draw_full_segments)
    {
        if(available[0])
        {
            segments[0]->get_edge_segment().draw(img, 255, 0, 0, width);
        }
        if(available[1])
        {
            segments[1]->get_edge_segment().draw(img, 0, 255, 0, width);
        }
        if(available[2])
        {
            segments[2]->get_edge_segment().draw(img, 0, 0, 255, width);
        }
    }
    else
    {
        if(available[0])
        {
            orthogonal_segments[0].draw(img, 255, 0, 0, width);
        }
        else
        {
            orthogonal_segments[0].draw(img, 255, 255, 255, width);
        }
        if(available[1])
        {
            orthogonal_segments[1].draw(img, 0, 255, 0, width);
        }
        else
        {
            orthogonal_segments[1].draw(img, 255, 255, 255, width);
        }
        if(available[2])
        {
            orthogonal_segments[2].draw(img, 0, 0, 255, width);
        }
        else
        {
            orthogonal_segments[2].draw(img, 255, 255, 255, width);
        }
    }
}

Manhattan_world::Manhattan_world
(
    const Edge_segment_set & isegments,
    const std::vector<Vanishing_point> & ivpts,
    double ifocal,
    double outlier_threshold
)
: _assignments(4)
{
    _vpts = ivpts;
    focal_length = ifocal;
    if(focal_length < MW_MIN_FOCAL_INIT_VALUE)
    {
        focal_length = MW_MIN_FOCAL_INIT_VALUE;
    }

    assign_segments_to_vpts(isegments, outlier_threshold);
    create_corners();
}

void Manhattan_world::assign_segments_to_vpts(const Edge_segment_set & isegments, double outlier_threshold)
{
    for(unsigned int i =0 ; i < 4; i++)
    {
        _assignments[i].clear();
    }
    _corners2.clear();
    _corners3.clear();

    unsigned int _index = 0;
    for(unsigned int i = 0; i < isegments.size(); i++)
    {
        _index = assign_to_vanishing_point(outlier_threshold, &( isegments.get_segment(i) ), _vpts);
        _assignments[_index].push_back(Manhattan_segment( isegments.get_segment(i) ));
        if(_index == Vanishing_point_detector::VPD_OUTLIER)
        {
            _assignments[_index].back().mark_as_outlier();
        }
        else
        {
            _assignments[_index].back().set_vanishing_point(& (_vpts[_index]) );
        }
    }
}

/** @brief Draws a line between the mid point of each segment (excluding outliers)
 *  and the vanishing point the segment converges to. Line segments
 *  are drawn with different colors according to the
 *  vanishing point they are assigned to (green and red for horizontal,
 *  blue for vertical, black for outliers).
 *
 * @param img the image to draw the segments on
 * @param width the width to use when drawing the line segments
 */
void Manhattan_world::draw_lines_from_segments_midpoint_to_vp(kjb::Image & img, double width) const
{
    for(unsigned int i = 0; i < _assignments[0].size(); i++ )
    {
        (_assignments[0])[i].draw_mid_point_to_vanishing_point(img, 255, 0, 0, width);
    }
    for(unsigned int i = 0; i < _assignments[1].size(); i++ )
    {
        (_assignments[1])[i].draw_mid_point_to_vanishing_point(img, 0, 255, 0, width);
    }
    for(unsigned int i = 0; i < _assignments[2].size(); i++ )
    {
        (_assignments[2])[i].draw_mid_point_to_vanishing_point(img, 0, 0, 255, width);
    }
    for(unsigned int i = 0; i < _assignments[3].size(); i++ )
    {
        (_assignments[3])[i].draw_mid_point_to_vanishing_point(img, 0, 0, 0, width);
    }
}

/** Draws the line segments with different colors according to the
 * vanishing point they are assigned to (green and red for horizontal,
 * blue for vertical, black for outliers).
 *
 * @param img the image to draw the segments on
 * @param width the width to use when drawing the line segments
 */
void Manhattan_world::draw_segments_with_vp_assignments(kjb::Image & img, double width) const
{
    for(unsigned int i = 0; i < _assignments[0].size(); i++ )
    {
        (_assignments[0])[i].draw(img, 255, 0, 0, width);
    }
    for(unsigned int i = 0; i < _assignments[1].size(); i++ )
    {
        (_assignments[1])[i].draw(img, 0, 255, 0, width);
    }
    for(unsigned int i = 0; i < _assignments[2].size(); i++ )
    {
        (_assignments[2])[i].draw(img, 0, 0, 255, width);
    }
    for(unsigned int i = 0; i < _assignments[3].size(); i++ )
    {
        (_assignments[3])[i].draw(img, 0, 0, 0, width);
    }
}

const Manhattan_segment & Manhattan_world::get_manhattan_segment(unsigned int vp_index, unsigned int segment_index) const
{
    if(vp_index > 3)
    {
        KJB_THROW_2(Illegal_argument, "Index of requested manhattan segment is out of bounds");
    }
    if(segment_index >= _assignments[vp_index].size())
    {
        KJB_THROW_2(Illegal_argument, "Index of requested manhattan segment is out of bounds");
    }
    return _assignments[vp_index][segment_index];
}

// TODO to be implemented
double Manhattan_corner_segment::compute_penalty() const
{
    /** Edge_segment elements */
    /*double strength = segment.get_edge_segment().get_strength();
    double length = segment.get_edge_segment().get_length();
    double fitting_error = segment.get_edge_segment().get_least_squares_fitting_error();*/

    /** Manhattan segments elements */
    /*double alpha = segment.get_alpha(); */

    /** Manhattan corner segment elements */
    /*double distance = distance_to_centre;
    double perp_dist = perpendicular_distance; */

    //TODO Compute penalty
    return 0.0;
}

/**
 *  Reads this Manhattan world from an input stream.
 *
 * @param in the input stream to read this Manhattan world from
 */
void Manhattan_world::read(std::istream& in, const Edge_segment_set & isegments, double outlier_threshold)
{
    for(unsigned int i = 0; i < 3; i++)
    {
        _vpts[i].read(in);
    }

    using std::istringstream;

    const char* field_value;
    if (!(field_value = Readable::read_field_value(in, "focal_length")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Manhattan world focal length");
    }
    istringstream ist(field_value);
    ist >> focal_length;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid focal length");
    }
    ist.clear(std::ios_base::goodbit);
    
    assign_segments_to_vpts(isegments, outlier_threshold);
    try
    {
        read_corners(in);
    }
    catch(...)
    {
        create_corners();
    }
}

void Manhattan_world::read(std::istream& in)
{
    for(unsigned int i = 0; i < 3; i++)
    {
        _vpts[i].read(in);
    }

    using std::istringstream;

    const char* field_value;
    if (!(field_value = Readable::read_field_value(in, "focal_length")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Manhattan world focal length");
    }
    istringstream ist(field_value);
    ist >> focal_length;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid focal length");
    }
    ist.clear(std::ios_base::goodbit);
    read_corners(in);
}

/**
 *  Reads this Manhattan world from an input stream.
 *
 * @param in the input stream to read this Manhattan world from
 */
void Manhattan_world::read(const char * filename)
{
    std::ifstream in;
    std::ostringstream ost;

    in.open(filename);
    if (in.fail())
    {
        ost << filename << ": Could not open file";
        KJB_THROW_2(IO_error,ost.str());
    }
    read(in);
    in.close();
    if (in.fail())
    {
        ost << filename << ": Could not close file";
        KJB_THROW_2(IO_error,ost.str());
    }
}

/**
 *  Reads this Manhattan world from an input file.
 *
 * @param in the input stream to read this Manhattan world from
 */
void Manhattan_world::read(const char * filename, const Edge_segment_set & isegments, double outlier_threshold)
{
    std::ifstream in;
    std::ostringstream ost;

    in.open(filename);
    if (in.fail())
    {
        ost << filename << ": Could not open file";
        KJB_THROW_2(IO_error,ost.str());
    }
    read(in, isegments, outlier_threshold);
    in.close();
    if (in.fail())
    {
        ost << filename << ": Could not close file";
        KJB_THROW_2(IO_error,ost.str());
    }
}

/**
 *  Writes this Manhattan world to an input stream.
 *
 * @param out the output stream to write this Manhattan world to
 */
void Manhattan_world::write(std::ostream& out) const
{
    for(unsigned int i = 0; i < 3; i++)
    {
        _vpts[i].write(out);
    }

    out << "focal_length: " << focal_length << "\n";
    write_corners(out);
}

void Manhattan_corner::init_missing_orthogonal_segment(int iindex, double start_x, double start_y, double end_x, double end_y)
{
        orthogonal_segments[iindex].init_from_end_points(start_x, start_y, end_x, end_y);
    available[iindex] = false;
}

void Manhattan_corner::compute_orthogonal_segment(unsigned int i)
{
    /** This function compute the little tiny segment starting from the corner
     * and going towards/away from the vanishing point. Determining the direction
     * is crucial for proposing blocks from the corner. We do it by looking
     * at the detected image segment forming the corner, and see if it is going
     * towards or away wrt the vanishing point. In the case the segment goes through
     * the corner, we consider on which side of the corner its longest bit is
     */
    if(!is_available(i))
    {
        return;
    }
    double vx, vy, factor;
    if((*v_pts)[i].is_at_infinity())
    {
        if(i == 2)
        {
             //Vertical
            vx = 0.0;
            if(is_up_corner())
            {
                vy = position(1) - 10000000;
            }
            else
            {
                vy = position(1) + 10000000;
            }
        }
        else
        {
            vy = 0.0;
            if(is_right_segment(i))
            {
                vx = position(0) + 10000000;
            }
            else
            {
                vx = position(0) - 10000000;
            }
        }
    }
    else
    {
        double t = segments[i]->get_edge_segment().find_t(position);
        double tempvx = 0.0;
        double tempvy = 0.0;
        if(t>=0 && t <=1)
        {
            if(t > 0.5)
            {
                tempvx = segments[i]->get_edge_segment().get_start_x() - position(0);
                tempvy = segments[i]->get_edge_segment().get_start_y() - position(1);
            }
            else
            {
                tempvx = segments[i]->get_edge_segment().get_end_x() - position(0);
                tempvy = segments[i]->get_edge_segment().get_end_y() - position(1);
            }
        }
        else
        {
            tempvx = segments[i]->get_edge_segment().get_centre_x() - position(0);
            tempvy = segments[i]->get_edge_segment().get_centre_y() - position(1);
        }
        vx = position(0) - (*v_pts)[i].get_x();
        vy = position(1) - (*v_pts)[i].get_y();
        double diff1 = (vx - tempvx)*(vx - tempvx) + (vy - tempvy)*(vy - tempvy);
        double diff2 = (-vx - tempvx)*(-vx - tempvx) + (-vy - tempvy)*(-vy - tempvy);
        if(diff2 < diff1)
        {
            vx = -vx;
            vy = -vy;
        }
    }
    factor = 10.0/sqrt(vx*vx + vy*vy);
    vx *= factor;
    vy *= factor;
    orthogonal_segments[i].init_from_end_points(position(0), position(1), vx+position(0), vy+position(1));
}

bool Manhattan_corner::is_right_segment(unsigned int i)
{
    if(i >= 2)
    {
        std::cout << "SHOULD BE 0 or 1!!!!" << std::endl;
        KJB_THROW_2(Illegal_argument, "is_right_segment can only be called on segment 0 or 1");
    }
    if(!is_available(i))
    {
        std::cout << "SHOULD BE AVAILABLE!!!!" << std::endl;
        KJB_THROW_2(KJB_error, "Cannot call is_right_segment on a non available segment");
    }
    double diff1 = (position(0) - segments[i]->get_edge_segment().get_start_x())*(position(0) - segments[i]->get_edge_segment().get_start_x())
                 + (position(1) - segments[i]->get_edge_segment().get_start_y())*(position(1) - segments[i]->get_edge_segment().get_start_y());
    double diff2 = (position(0) - segments[i]->get_edge_segment().get_end_x())*(position(0) - segments[i]->get_edge_segment().get_end_x())
                 + (position(1) - segments[i]->get_edge_segment().get_end_y())*(position(1) - segments[i]->get_edge_segment().get_end_y());
    if(segments[i]->get_edge_segment().get_start_x() < segments[i]->get_edge_segment().get_end_x())
    {
        if(diff1 > diff2)
        {
            return false;
        }
        return true;
    }
    else
    {
        if(diff1 > diff2)
        {
            return true;
        }
        return false;
    }
    return true;
}

void Manhattan_corner::compute_orthogonal_segments()
{
    for(unsigned int i = 0; i < 3; i++)
    {
        compute_orthogonal_segment(i);
    }
}

void Manhattan_corner::get_3D_corner
(
    double z_distance,
    double focal_length,
    double princ_x,
    double princ_y,
    kjb::Vector & corner3D_1,
    kjb::Vector & corner3D_2,
    kjb::Vector & corner3D_3,
    kjb::Vector & position_3D
) const
{
    if(focal_length <= DBL_EPSILON)
    {
        KJB_THROW_2(Illegal_argument,"Bad value for focal length");
    }

    kjb::Vector position_2D(3, 1.0);
    position_2D(0) = position(0) - princ_x;
    position_2D(1) = -(position(1) - princ_y);

    position_3D.resize(3);
    position_3D(2) = fabs(z_distance);
    position_3D(1) = (position_2D(1)*position_3D(2))/focal_length;
    position_3D(0) = (position_2D(0)*position_3D(2))/focal_length;

    std::vector<kjb::Vector> segments2d(3, Vector(3, 1.0));

    for(unsigned int i = 0; i < 3; i++)
    {
        double point_x = orthogonal_segments[i].get_end_x();
        double point_y = orthogonal_segments[i].get_end_y();
        if( ( fabs(point_x - position(0)) < 0.1) && (fabs(point_y - position(1)) < 0.1) )
        {
             point_x = orthogonal_segments[i].get_start_x();
             point_y = orthogonal_segments[i].get_start_y();
        }
        if( (i == 2) && ! (is_available(i)) )
        {
            point_x = position(0);
            if(position(1) < princ_y)
            {
                //Let's make it a down corner
                point_y = position(1) + 10.0;
            }
            else
            {
                //Let's make it an up corner
                point_y = position(1) - 10.0;
            }
        }
        segments2d[i](0) = point_x - princ_x;
        segments2d[i](1) = -(point_y - princ_y);
    }

    try
    {
        get_3D_corner_orientation_from_2D_corner_lines
        (
            segments2d[0],
            segments2d[2],
            segments2d[1],
            position_2D,
            position_3D,
            focal_length,
            1,
            corner3D_1,
            corner3D_2,
            corner3D_3
        );
    }
    catch(KJB_error e)
    {
        KJB_THROW_2(KJB_error,"Could not create orthogonal 3D corner from 2D corner");
    }

    Matrix rotation_matrix(3,3);
    kjb::Vector to_check(3);
    for(unsigned int i = 0; i < 3; i++)
    {
        rotation_matrix(0,i) = corner3D_1(i);
        rotation_matrix(1,i) = corner3D_2(i);
        rotation_matrix(2,i) = corner3D_3(i);
        to_check(i) = -position_3D(i);
    }

    to_check = rotation_matrix*to_check;
    if( (to_check(0) < 0) || (to_check(1) < 0) || (to_check(2) < 0) )
    {
        try
        {
            get_3D_corner_orientation_from_2D_corner_lines
            (
                segments2d[0],
                segments2d[2],
                segments2d[1],
                position_2D,
                position_3D,
                focal_length,
                -1,
                corner3D_1,
                corner3D_2,
                corner3D_3
            );
        }
        catch(KJB_error e)
        {
            KJB_THROW_2(KJB_error,"Could not create orthogonal 3D corner from 2D corner");
        }

        for(unsigned int i = 0; i < 3; i++)
        {
            rotation_matrix(0,i) = corner3D_1(i);
            rotation_matrix(1,i) = corner3D_2(i);
            rotation_matrix(2,i) = corner3D_3(i);
            to_check(i) = -position_3D(i);
       }

        to_check = rotation_matrix*to_check;
       if( (to_check(0) < 0) || (to_check(1) < 0) || (to_check(2) < 0) )
       {
           //KJB_THROW_2(KJB_error,"Could not create orthogonal 3D corner from 2D corner");
       }
    }

}

void Manhattan_corner::get_direction(unsigned int segment_index, kjb::Vector & direction) const
{
    if(segment_index > 2)
    {
        KJB_THROW_2(Illegal_argument, "Corner. segment index must be between 0 and 2");
    }
    orthogonal_segments[segment_index].get_direction(direction);
    double diff1 = (position(0) - orthogonal_segments[segment_index].get_start_x())*(position(0) - orthogonal_segments[segment_index].get_start_x())
        + (position(1) - orthogonal_segments[segment_index].get_start_y())*(position(1) - orthogonal_segments[segment_index].get_start_y());
    double diff2 = (position(0) - orthogonal_segments[segment_index].get_end_x())*(position(0) - orthogonal_segments[segment_index].get_end_x())
        + (position(1) - orthogonal_segments[segment_index].get_end_y())*(position(1) - orthogonal_segments[segment_index].get_end_y());
    if(diff2 < diff1)
    {
        direction *= -1.0;
    }
}

bool Manhattan_corner::is_up_corner() const
{
    double diff1 = (position(0) - orthogonal_segments[2].get_start_x())*(position(0) - orthogonal_segments[2].get_start_x()) +
            (position(1) - orthogonal_segments[2].get_start_y())*(position(1) - orthogonal_segments[2].get_start_y());
    double diff2 = (position(0) - orthogonal_segments[2].get_end_x())*(position(0) - orthogonal_segments[2].get_end_x()) +
            (position(1) - orthogonal_segments[2].get_end_y())*(position(1) - orthogonal_segments[2].get_end_y());
    if(diff1 < diff2)
    {
        //Start is the corner
        if(orthogonal_segments[2].get_end_y() < orthogonal_segments[2].get_start_y())
        {
            return true;
        }
        return false;
    }
    else
    {
        if(orthogonal_segments[2].get_end_y() < orthogonal_segments[2].get_start_y())
        {
            return false;
        }
        return true;
    }
}

double Manhattan_corner::get_avg_segment_size() const
{
    double avg_size = 0.0;
    for(unsigned int i = 0; i < 3; i++)
    {
        if(available[i])
        {
            if( fabs(avg_size) < DBL_EPSILON)
            {
                avg_size = segments[i]->get_edge_segment().get_length();
            }
            else if( segments[i]->get_edge_segment().get_length() < avg_size )
            {
                avg_size = segments[i]->get_edge_segment().get_length();
            }

        }
    }
    return avg_size;
}



void Manhattan_world::write_manhattan_corner(const Manhattan_corner & corner, std::ostream& out) const
{
    const kjb::Vector & position = corner.get_position();
    out << "position:" << position(0) << " " << position(1) << "\n";
    for(unsigned int i =0; i < 3; i++)
    {
        out << "corner_segment_available:" << corner.is_available(i) <<  "\n";
        if(corner.is_available(i))
        {
            out << "corner_segment_index:" << corner.get_index(i) <<  "\n";
            out << "corner_segment_distance:" << corner.get_segment(i)->get_distance() <<  "\n";
            out << "corner_segment_perp_distance:" << corner.get_segment(i)->get_perpendicular_distance() <<  "\n";
        }
        else
        {
            corner.get_orthogonal_segment(i).write(out);
        }
    }
}

void Manhattan_world::draw_corners2smart(kjb::Image & img, bool draw_full_segments, double width) const
{
    for(unsigned int i = 0; i < _corners2.size(); i++)
    {
        if(_corners2[i].get_avg_segment_size() > 25.0)
        {
            _corners2[i].draw(img, draw_full_segments, width);
        }
    }
}

void Manhattan_world::draw_corners3smart(kjb::Image & img, bool draw_full_segments, double width ) const
{
    for(unsigned int i = 0; i < _corners3.size(); i++)
    {
        if(_corners3[i].get_avg_segment_size() > 10.0)
        {
            _corners3[i].draw(img, draw_full_segments, width);
        }
    }
}

void Manhattan_world::draw_extra_corners(kjb::Image & img, bool draw_full_segments, double width) const
{
    for(unsigned int i = 0; i < _extra_corners.size(); i++)
    {
        _extra_corners[i].draw(img, draw_full_segments, width);
    }
}

void Manhattan_world::read_manhattan_corner(Manhattan_corner & corner, std::istream& in) const
{
    using std::istringstream;

    unsigned int index;
    bool available;
    double distance, perp_distance;

    const char* field_value;
    if (!(field_value = Readable::read_field_value(in, "position")))
    {
        KJB_THROW_2(Illegal_argument, "Missing corner position");
    }
    istringstream ist(field_value);
    kjb::Vector position(2);
    ist >> position(0) >> position(1);
    corner.set_position(position);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid corner position");
    }
    ist.clear(std::ios_base::goodbit);

    for(unsigned int i =0; i < 3; i++)
    {
        if (!(field_value = Readable::read_field_value(in, "corner_segment_available")))
        {
            KJB_THROW_2(Illegal_argument, "Missing corner segment");
        }
        ist.str(field_value);
        ist >> available;
        if (ist.fail())
        {
            KJB_THROW_2(Illegal_argument, "Invalid corner segment");
        }
        ist.clear(std::ios_base::goodbit);

        if(available)
        {
            if (!(field_value = Readable::read_field_value(in, "corner_segment_index")))
            {
                KJB_THROW_2(Illegal_argument, "Missing corner segment");
            }
            ist.str(field_value);
            ist >> index;
            if (ist.fail())
            {
                KJB_THROW_2(Illegal_argument, "Invalid corner segment distance");
            }
            ist.clear(std::ios_base::goodbit);

            if (!(field_value = Readable::read_field_value(in, "corner_segment_distance")))
            {
                KJB_THROW_2(Illegal_argument, "Missing corner segment distance");
            }
            ist.str(field_value);
            ist >> distance;
            if (ist.fail())
            {
                KJB_THROW_2(Illegal_argument, "Invalid corner segment distance");
            }
            ist.clear(std::ios_base::goodbit);

            if (!(field_value = Readable::read_field_value(in, "corner_segment_perp_distance")))
            {
                KJB_THROW_2(Illegal_argument, "Missing corner segment perpendicular distance");
            }
            ist.str(field_value);
            ist >> perp_distance;
            if (ist.fail())
            {
                KJB_THROW_2(Illegal_argument, "Invalid corner segment perpendicular distance");
            }
            ist.clear(std::ios_base::goodbit);
            try
            {
                const Manhattan_segment & ms = get_manhattan_segment(i, index);
                corner.set_segment(i, ms, distance, perp_distance);
                corner.set_index(i, index);
                corner.compute_orthogonal_segment(i);
            }
            catch(Illegal_argument e)
            {
                KJB_THROW_2(IO_error, "Could not read Manhattan corner, bad segment index");
            }
        }
        else
        {
            Line_segment ls;
            ls.read(in);
            corner.init_missing_orthogonal_segment(i, ls.get_start_x(), ls.get_start_y(), ls.get_end_x(), ls.get_end_y());
        }
    }
}

void Manhattan_world::write_corners(std::ostream& out) const
{
    out << "num_corners2:" << _corners2.size() <<  "\n";
    for(unsigned int i = 0; i < _corners2.size(); i++)
    {
        write_manhattan_corner(_corners2[i], out);
    }
    out << "num_corners3:" << _corners3.size() <<  "\n";
    for(unsigned int i = 0; i < _corners3.size(); i++)
    {
        write_manhattan_corner(_corners3[i], out);
    }
}

void Manhattan_world::read_corners(std::istream& in)
{
    using std::istringstream;

    const char* field_value;

    unsigned int num_corners;

    if (!(field_value = Readable::read_field_value(in, "num_corners2")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Number of Manhattan Corners");
    }
    istringstream ist(field_value);
    ist >> num_corners;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing Number of Manhattan Corners");
    }
    ist.clear(std::ios_base::goodbit);

    _corners2.clear();
    for(unsigned int i = 0; i < num_corners; i++ )
    {
        _corners2.push_back(Manhattan_corner(&_vpts));
        read_manhattan_corner(_corners2.back(), in);
    }

    if (!(field_value = Readable::read_field_value(in, "num_corners3")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Number of Manhattan Corners");
    }
    ist.str(field_value);
    ist >> num_corners;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Missing Number of Manhattan Corners");
    }
    ist.clear(std::ios_base::goodbit);

    _corners3.clear();
    for(unsigned int i = 0; i < num_corners; i++ )
    {
        _corners3.push_back(Manhattan_corner(&_vpts));
        read_manhattan_corner(_corners3.back(), in);
    }
}

void Manhattan_world::draw_corners(kjb::Image & img, bool draw_full_segments, double width) const
{
    draw_corners2(img, draw_full_segments, width);
    draw_corners3(img, draw_full_segments, width);
}

void Manhattan_world::draw_corners2(kjb::Image & img, bool draw_full_segments, double width) const
{
    for(unsigned int i = 0; i< _corners2.size(); i++)
    {
        _corners2[i].draw(img, draw_full_segments, width);
    }
}

void Manhattan_world::draw_corners3(kjb::Image & img, bool draw_full_segments, double width) const
{
    for(unsigned int i = 0; i< _corners3.size(); i++)
    {
        _corners3[i].draw(img, draw_full_segments, width);
    }
}

void Manhattan_world::draw_corners2up(kjb::Image & img, bool draw_full_segments, double width) const
{
    for(unsigned int i = 0; i< _corners2.size(); i++)
    {
        if(_corners2[i].is_up_corner())
        {
            _corners2[i].draw(img, draw_full_segments, width);
        }
    }
}

void Manhattan_world::draw_corners3up(kjb::Image & img, bool draw_full_segments, double width) const
{
    for(unsigned int i = 0; i< _corners3.size(); i++)
    {
        if(_corners3[i].is_up_corner())
        {
            _corners3[i].draw(img, draw_full_segments, width);
        }
    }
}

/**
  * Creates corners by checking all possible intersections of
  * two line segments
  * @param corner Will contain the corner
  * @param seg1   The first line segment
  * @param seg2   The second line segment
  * @param index1 The index of the vanishing  point seg1 converges to
  * @param index2 The index of the vanishing  point seg2 converges to
  * @param max_stretch The maximum stretch applicable to a line segment
  *                    in order to consider the intersection as a corner
  */
bool Manhattan_world::create_corner
(
    Manhattan_corner & corner,
    const Manhattan_segment & seg1,
    const Manhattan_segment & seg2,
    unsigned int index1,
    unsigned int index2,
    double max_stretch
)
{
    if(index1 == index2)
    {
        KJB_THROW_2(Illegal_argument,"Segments of a Manhattan corner must converge to different vpts");
    }
    const Edge_segment & e1 = seg1.get_edge_segment();
    const Edge_segment & e2 = seg2.get_edge_segment();
    Vector ints;

    /** If the lines do not intersect, this is not a corner */
    if(! Line::find_line_intersection(e1.get_line(), e2.get_line(), ints))
    {
        return false;
    }
    corner.set_position(ints);
    double perp1;
    double perp2;
    double dist1 = e1.get_distance_from_point(ints, &perp1);
    double dist2 = e2.get_distance_from_point(ints, &perp2);

    /** If the intersection does not lie on one of the line segments,
     * let's see by how much we have to extend them */
    if( (dist1 > max_stretch) || (dist2 > max_stretch) )
    {
        return false;
    }
    /*corner.set_segment(Manhattan_corner_segment(seg1.get_edge_segment(), dist1, perp1), index1);
    corner.set_segment(Manhattan_corner_segment(seg2.get_edge_segment(), dist2, perp2), index2);*/
    corner.set_segment(index1, seg1, dist1, perp1);
    corner.set_segment(index2, seg2, dist2, perp2);
    return true;

}

/**
  * Creates a corner from an already existing corner and a line segment.
  *  Returns false if the segment does not intersect the corner position
  *
  * @param corner The corner. It will be adapted if the third segment intersects it
  * @param seg3   The line segment
  * @param index3 The index of the vanishing  point seg3 converges to
  * @param max_stretch The maximum stretch applicable to a line segment
  *                    in order to consider the intersection as a corner
  * @param max_perp_stretch The maximum tolerated distance between the line seg3
  *                         lies on and the corner position
  */
bool Manhattan_world::create_corner3
(
   Manhattan_corner & corner,
   const Manhattan_segment & seg3,
   unsigned int index3,
   double max_stretch,
   double max_perp_stretch
)
{
    if(corner.num_available_segments() != 2)
    {
        return false;
    }
    if(corner.is_available(index3))
    {
        /** This segment must be not available */
        return false;
    }

    double dist, perp_dist;
    const kjb::Vector & position = corner.get_position();
    dist = seg3.get_edge_segment().get_distance_from_point(position, &perp_dist);
    if( (dist > max_stretch) || (perp_dist > max_perp_stretch) )
    {
        return false;
    }
    corner.set_segment(index3, seg3, dist, perp_dist);
    return true;
}

bool Manhattan_world::create_corner3_from_incomplete
(
   Manhattan_corner & corner,
   unsigned int index3,
   bool towards_vp,
   const kjb::Vector & position,
   bool check_consistency
)
{
    if(corner.num_available_segments() != 2)
    {
        if(check_consistency)
        {
            return false;
        }
    }
    if(corner.is_available(index3))
    {
        /** This segment must be not available */
        return false;
    }

    double vx, vy, factor;
    if(_vpts[index3].is_at_infinity())
    {
        if(index3 == 2)
        {
            if(towards_vp)
            {
                vy = position(1) - 10000000;
                                vx = 0.0;
            }
            else
            {
                vy = position(1) + 10000000;
                                vx = 0.0;
            }
        }
        else
        {
            if(towards_vp)
            {
                vx = position(0) - 10000000;
                vy = 0.0;
            }
            else
            {
                vx = position(0) - 10000000;
                vy = 0.0;
            }
        }
    }
    else
    {
            vx = position(0) - _vpts[index3].get_x();
        vy = position(1) - _vpts[index3].get_y();
        if(!towards_vp)
        {
            vx = -vx;
            vy = -vy;
        }
    }
    factor = 10.0/sqrt(vx*vx + vy*vy);
    vx *= factor;
    vy *= factor;
    corner.init_missing_orthogonal_segment(index3, position(0), position(1), vx+position(0), vy+position(1));
    return true;
}


void Manhattan_world::create_corners2()
{
    _corners2.clear();
    Manhattan_corner c(&_vpts);
    
    /** Between 0 and 1 */
    for(unsigned int i = 0; i < _assignments[0].size(); i++)
    {
        for(unsigned int j = 0; j < _assignments[1].size(); j++)
        {
            if( create_corner(c, _assignments[0][i], _assignments[1][j] , 0, 1) )
            {
                c.set_index(0, i);
                c.set_index(1, j);
                Manhattan_corner c2(c);
                if(create_corner3_from_incomplete(c, 2, true, c.get_position()))
                {
                    _corners2.push_back(c);
                }
                if(create_corner3_from_incomplete(c2, 2, false, c.get_position()))
                {
                    _corners2.push_back(c2);
                }
            }
        }
    }

    /** Between 0 and 2 */
    for(unsigned int i = 0; i < _assignments[0].size(); i++)
    {
        for(unsigned int j = 0; j < _assignments[2].size(); j++)
        {
            if( create_corner(c, _assignments[0][i], _assignments[2][j] , 0, 2) )
            {
                c.set_index(0, i);
                c.set_index(2, j);
                Manhattan_corner c2(c);
                if(create_corner3_from_incomplete(c, 1, true, c.get_position()))
                {
                    _corners2.push_back(c);
                }
                if(create_corner3_from_incomplete(c2, 1, false, c.get_position()))
                {
                    _corners2.push_back(c2);
                }
            }
        }
    }


    /** Between 1 and 2 */
    for(unsigned int i = 0; i < _assignments[1].size(); i++)
    {
        for(unsigned int j = 0; j < _assignments[2].size(); j++)
        {
            if( create_corner(c, _assignments[1][i], _assignments[2][j] , 1, 2) )
            {
                c.set_index(1, i);
                c.set_index(2, j);
                Manhattan_corner c2(c);
                if(create_corner3_from_incomplete(c, 0, true, c.get_position()))
                {
                    _corners2.push_back(c);
                }
                if(create_corner3_from_incomplete(c2, 0, false, c.get_position()))
                {
                    _corners2.push_back(c2);
                }
            }
        }
    }
}

void Manhattan_world::create_corners3()
{
    _corners3.clear();
    for(unsigned int i = 0; i < _corners2.size(); i++)
    {
        Manhattan_corner corner3 = _corners2[i];
        if( (corner3.is_available(0)) && (corner3.is_available(1)) && !(corner3.is_available(2)) )
        {
            for(unsigned int j = 0; j < _assignments[2].size(); j++)
            {
                if( create_corner3(corner3, _assignments[2][j], 2) )
                {
                    corner3.set_index(2, j);
                    _corners3.push_back(corner3);
                }

            }
        }
        else if( (corner3.is_available(0)) && !(corner3.is_available(1)) && (corner3.is_available(2)) )
        {
            for(unsigned int j = 0; j < _assignments[1].size(); j++)
            {
                if( create_corner3(corner3, _assignments[1][j], 1) )
                {
                    if(!corner3_exists(corner3))
                    {
                        corner3.set_index(1, j);
                        _corners3.push_back(corner3);
                    }
                }

            }
        }
        else if( !(corner3.is_available(0)) && (corner3.is_available(1)) && (corner3.is_available(2)) )
        {
            for(unsigned int j = 0; j < _assignments[0].size(); j++)
            {
                if( create_corner3(corner3, _assignments[0][j], 0) )
                {
                    corner3.set_index(0, j);
                    if(!corner3_exists(corner3))
                    {
                        _corners3.push_back(corner3);
                    }
                }

            }
        }
    }
}

bool Manhattan_world::corner3_exists(const Manhattan_corner & c)
{
    const kjb::Vector & position = c.get_position();
    for( unsigned int i = 0; i < _corners3.size(); i++)
    {
        const kjb::Vector & position2 = _corners3[i].get_position();
        double diff = position.get_max_abs_difference(position2);
        if(diff < 5.0)
        {
            if( ( _corners3[i].get_index(0) == c.get_index(0) ) &&
                ( _corners3[i].get_index(1) == c.get_index(1) ) &&
                ( _corners3[i].get_index(2) == c.get_index(2)) )
            {
                return true;
            }
            return true;
        }
    }
    return false;
}

void Manhattan_world::create_corners()
{
    create_corners2();
    create_corners3();
}

void Manhattan_world::print_corners2(std::ostream& out) const
{
    std::cout << "Corners 2:" << std::endl;
    for(unsigned int i = 0; i < _corners2.size(); i++)
    {
        out << _corners2[i] << std::endl;
    }
}

void Manhattan_world::print_corners3(std::ostream& out) const
{
    kjb::Vector  direction;
    std::cout << "Corners 3:" << std::endl;
    for(unsigned int i = 0; i < _corners3.size(); i++)
    {
        out << _corners3[i] << std::endl;
        out << "Orthogonal segments:" << std::endl;
        for(unsigned int j = 0; j < 3; j++)
        {
            _corners3[i].get_orthogonal_segment(j).get_direction(direction);
            out << direction << std::endl;
        }
    }
}

void Manhattan_world::print_vanishing_points(std::ostream& out) const
{
    for(unsigned int i = 0; i < 3; i++)
    {
        if(_vpts[i].is_at_infinity())
        {
            out << "Vpt " << i << " is at infinity" << std::endl;
        }
        else
        {
            out << "Vpt " << i << ":" << _vpts[i].get_x() << " | " << _vpts[i].get_y() << std::endl;
        }
    }
    out << "Focal length:" << focal_length << std::endl;
}


void Manhattan_world::get_3D_corner
(
    double z_distance,
    double princ_x,
    double princ_y,
    unsigned int corner_index,
    bool usecorner3,
    kjb::Vector & corner3D_1,
    kjb::Vector & corner3D_2,
    kjb::Vector & corner3D_3,
    kjb::Vector & position
) const
{
    if(usecorner3)
    {
        if(corner_index >= _corners3.size())
        {
            KJB_THROW_2(Illegal_argument,"Corner index out of bounds");
        }

        try
        {
            _corners3[corner_index].get_3D_corner(z_distance, focal_length, princ_x, princ_y,
                    corner3D_1,corner3D_2,corner3D_3, position);
        }
        catch(KJB_error e)
        {
            KJB_THROW_2(KJB_error,"Could not get 3D corner from Manhattan corner");
        }
    }
    else
    {
        if(corner_index >= _corners2.size())
        {
            KJB_THROW_2(Illegal_argument,"Corner index out of bounds");
        }
        try
        {
              _corners2[corner_index].get_3D_corner(z_distance, focal_length, princ_x, princ_y,
                    corner3D_1,corner3D_2,corner3D_3, position);
        }
        catch(KJB_error e)
        {
            KJB_THROW_2(KJB_error,"Could not get 3D corner from Manhattan corner");
        }
    }
}

void Manhattan_world::set_extra_corners_from_vertical_pairs(const std::vector<Segment_pair> & vpairs)
{
    _extra_corners.clear();
    Vector extr1(3, 1.0);
    Vector extr2(3, 1.0);
    Vector tempextr(3, 1.0);
    for(unsigned int i = 0; i < vpairs.size(); i++)
    {
        extr1 = vpairs[i].get_extremity_1();
        extr2 = vpairs[i].get_extremity_2();
        bool towards_vp = true;
        Line_segment temp_ls;
        if(!_vpts[2].is_at_infinity())
        {
            if(_vpts[2].get_y() > extr1[1])
            {
                towards_vp = 2;
            }
        }
        try
        {
            Line_segment temp_ls2(extr1, extr2);
            temp_ls = temp_ls2;
        }
        catch(...)
        {
            //Extr1 and Extr2 coincide
            Manhattan_corner c1(&_vpts);
            bool result1 = create_corner3_from_incomplete(c1, 2, towards_vp, extr1, false);
            if(!result1)
            {
                continue;
            }
            Manhattan_corner c2(c1);
            result1 = create_corner3_from_incomplete(c1, 0, false, extr1, false);
            if(result1)
            {
                Manhattan_corner c1b(c1);
                result1 = create_corner3_from_incomplete(c1, 1, false, extr1, false);
                if(result1)
                {
                    _extra_corners.push_back(c1);
                }
                result1 = create_corner3_from_incomplete(c1b, 1, true, extr1, false);
                if(result1)
                {
                    _extra_corners.push_back(c1b);
                }
            }
            result1 = create_corner3_from_incomplete(c2, 0, true, extr1, false);
            if(result1)
            {
                Manhattan_corner c2b(c2);
                result1 = create_corner3_from_incomplete(c2, 1, false, extr1, false);
                if(result1)
                {
                    _extra_corners.push_back(c2);
                }
                result1 = create_corner3_from_incomplete(c2b, 1, true, extr1, false);
                if(result1)
                {
                    _extra_corners.push_back(c2b);
                }
            }

            continue;
        }
        int vp_index = 0;
        if(Vanishing_point_detector::compute_alpha(_vpts[1], &temp_ls) < Vanishing_point_detector::compute_alpha(_vpts[0], &temp_ls))
        {
            vp_index = 1;
        }
        //We assign only to one of the orthogonal vanishing points

        double diff1 = ((extr1(0) - _vpts[vp_index].get_x()) * (extr1(0) - _vpts[vp_index].get_x())) +
                ((extr1(1) - _vpts[vp_index].get_y()) * (extr1(1) - _vpts[vp_index].get_y()));
        double diff2 = ((extr2(0) - _vpts[vp_index].get_x()) * (extr2(0) - _vpts[vp_index].get_x())) +
                ((extr2(1) - _vpts[vp_index].get_y()) * (extr2(1) - _vpts[vp_index].get_y()));
        if(diff1 < diff2)
        {
            tempextr = extr1;
            extr1 = extr2;
            extr2 = tempextr;
            // Now extr1 contains the extremity closest to the vanishing point, extr2 the farthest
        }
        Manhattan_corner c1(&_vpts);
        c1.set_position(extr1);
        int second_vp_index = 0;
        if(vp_index == 0)
        {
            second_vp_index = 1;
        }
        bool result1 = create_corner3_from_incomplete(c1, vp_index, false, extr1, false);
        if(result1)
        {
            result1 = create_corner3_from_incomplete(c1, 2, towards_vp, extr1, false);
            if(result1)
            {
                Manhattan_corner c1b(c1);
                result1 = create_corner3_from_incomplete(c1, second_vp_index, true, extr1, false);
                if(result1)
                {
                    _extra_corners.push_back(c1);
                }
                result1 = create_corner3_from_incomplete(c1b, second_vp_index, false, extr1, false);
                if(result1)
                {
                    _extra_corners.push_back(c1b);
                }
            }
        }
        Manhattan_corner c2(&_vpts);
        c2.set_position(extr1);
        result1 = create_corner3_from_incomplete(c2, vp_index, true, extr2, false);
        if(result1)
        {
            bool towards_vp = true;
            if(!_vpts[2].is_at_infinity())
            {
                if(_vpts[2].get_y() > extr2[1])
                {
                    towards_vp = 2;
                }
            }
            result1 = create_corner3_from_incomplete(c2, 2, towards_vp, extr2, false);
            if(result1)
            {
                Manhattan_corner c2b(c2);
                result1 = create_corner3_from_incomplete(c2, second_vp_index, true, extr2, false);
                if(result1)
                {
                    _extra_corners.push_back(c2);
                }
                result1 = create_corner3_from_incomplete(c2b, second_vp_index, false, extr2, false);
                if(result1)
                {
                    _extra_corners.push_back(c2b);
                }
            }
        }

    }

}

std::ostream & kjb::operator<<(std::ostream& out, const Manhattan_corner& mc)
{
    std::cout << "Corner position:" << mc.position(0) << " | " << mc.position(1) << std::endl;
    for(unsigned int i = 0; i < 3; i++)
    {
        out << "Segment " << i << ":" << std::endl;
        if(mc.available[i])
        {
            out << "Index:" << mc.indexes[i] << std::endl;
            out << *(mc.segments[i]);
        }
        else
        {
            out << "NOT available" << std::endl;
        }
    }
    return out;
}

std::ostream & kjb::operator<<(std::ostream& out, const Manhattan_corner_segment& mcs)
{
    out << mcs.get_manhattan_segment();
    out << "Distance:" << mcs.distance_to_centre << std::endl;
    out << "Perpendicular distance:" << mcs.perpendicular_distance << std::endl;
    return out;
}

std::ostream & kjb::operator<<(std::ostream& out, const Manhattan_segment& ms)
{
    if(ms._outlier)
    {
        std::cout << "Outlier" << std::endl;
    }
    else
    {
        out << ms._segment;
        out << "Alpha:" << ms.alpha << std::endl;
    }
    return out;
}

Manhattan_world * kjb::create_manhattan_world_from_CMU_file(const std::string & file_name)
{
    std::vector<kjb::Vanishing_point> vpts;
    double focal_length;
    read_CMU_vanishing_points(vpts, focal_length, file_name);
    return new kjb::Manhattan_world(vpts, focal_length);
}

Manhattan_world * kjb::create_mw_from_CMU_file_and_compute_focal_length
(
    const std::string & file_name,
    const kjb::Edge_segment_set & iset,
    unsigned int num_rows,
    unsigned int num_cols
)
{

    kjb::Manhattan_world * mw = kjb::create_manhattan_world_from_CMU_file(file_name);
    kjb::Vanishing_point_detector vpd(iset, num_rows, num_cols);
    double fl = vpd.compute_focal_length(mw->get_vanishing_points());
    mw->set_focal_length(fl);
    return mw;
}

