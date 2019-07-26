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


#include "edge_cpp/vanishing_point.h"
#include <sstream>
#include <fstream>
#include <iostream>

using namespace kjb;

/** This is an approximation of the position of the
 * vanishing points located at infinity
 */
#define INFINITY_APPROXIMATION 100000000

/**
 *  Reads this Vanishing point from an input stream.
 *
 * @param in the input stream to read this vanishing point from
 */
void Vanishing_point::read(std::istream& in)
{
    using std::istringstream;

    const char* field_value;

    // Vanishing point type
    if (!(field_value = read_field_value(in, "position")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Vanishing Point position");
    }
    istringstream ist(field_value);
    ist >> _point(0) >> _point(1);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Vanishing Point position");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "type")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Vanishing Point type");
    }
    ist.str(field_value);

    unsigned int temp_type = 0;
    ist >> temp_type;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Vanishing Point type");
    }
    ist.clear(std::ios_base::goodbit);
    _type = (Vanishing_point_type)temp_type;
}

/**
 *  Writes this Vanishing point to an input stream.
 *
 * @param out the output stream to write this vanishing point to
 */
void Vanishing_point::write(std::ostream& out) const
{
    out << " position: " << _point(0) << ' ' << _point(1) << '\n'
            << " type:" << (unsigned int)_type << '\n';
}

bool Vanishing_point::operator==(const Vanishing_point & vp)
{

    if(vp._type != _type)
    {
        return false;
    }
    if(vp.is_at_infinity())
    {
        return true;
    }
    if( fabs(vp.get_x() -get_x() ) > 0.1 )
    {
        std::cout << fabs(vp.get_x() -get_x() )  << std::endl;
        return false;
    }
    if( fabs(vp.get_y() -get_y() ) > 0.1 )
    {
        std::cout << fabs(vp.get_y() -get_y() ) << std::endl;
        return false;
    }
    return true;
}


void kjb::read_CMU_vanishing_points
(
    std::vector<Vanishing_point> & vpts,
    double & focal_length,
    std::string file_name
)
{
    std::string vpts_line;
    std::string focal_line;
    std::ifstream ifs(file_name.c_str());
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "Could not open file for reading vanishing points");
    }
    getline(ifs, vpts_line);
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "The first line of the CMU vanishing points file has an invalid format");
    }
    getline(ifs, focal_line);
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "The second line of the CMU vanishing points file has an invalid format");
    }
    if(focal_line.length() < 3)
    {
        getline(ifs, focal_line);
        if(ifs.fail())
        {
            KJB_THROW_2(IO_error, "The second line of the CMU vanishing points file has an invalid format");
        }
    }

    size_t vpt_start = vpts_line.find("[");
    size_t vpt_end = vpts_line.find("]");
    if( (vpt_start == std::string::npos ) || (vpt_end == std::string::npos))
    {
        KJB_THROW_2(IO_error, "The first line of the CMU vanishing points file has an invalid format");
    }
    std::string vpt_string = vpts_line.substr(vpt_start, vpt_end - vpt_start);
    size_t separator = vpt_string.find(",");
    if( separator == std::string::npos)
    {
        KJB_THROW_2(IO_error, "The first line of the CMU vanishing points file has an invalid format");
    }
    std::string vpt_1 = vpt_string.substr(1, separator -1);
    std::string vpt_2 = vpt_string.substr(separator+1, vpt_string.size()-separator-1);
    Vanishing_point vert_vpt((double)atof(vpt_1.c_str()),(double)atof(vpt_2.c_str()));
    //vpts.push_back(Vanishing_point((double)atof(vpt_1.c_str()),(double)atof(vpt_2.c_str())));

    vpt_start = vpts_line.find("[", vpt_start+1);
    vpt_end = vpts_line.find("]", vpt_end+1);
    if( (vpt_start == std::string::npos ) || (vpt_end == std::string::npos))
    {
        KJB_THROW_2(IO_error, "The first line of the CMU vanishing points file has an invalid format");
    }
    vpt_string = vpts_line.substr(vpt_start, vpt_end - vpt_start);
    separator = vpt_string.find(",");
    if( separator == std::string::npos)
    {
        KJB_THROW_2(IO_error, "The first line of the CMU vanishing points file has an invalid format");
    }
    vpt_1 = vpt_string.substr(1, separator -1);
    vpt_2 = vpt_string.substr(separator+1, vpt_string.size()-separator-1);
    vpts.push_back(Vanishing_point((double)atof(vpt_1.c_str()),(double)atof(vpt_2.c_str())));

    vpt_start = vpts_line.find("[", vpt_start+1);
    vpt_end = vpts_line.find("]", vpt_end+1);
    if( (vpt_start == std::string::npos ) || (vpt_end == std::string::npos))
    {
        KJB_THROW_2(IO_error, "The first line of the CMU vanishing points file has an invalid format");
    }
    vpt_string = vpts_line.substr(vpt_start, vpt_end - vpt_start);
    separator = vpt_string.find(",");
    if( separator == std::string::npos)
    {
        KJB_THROW_2(IO_error, "The first line of the CMU vanishing points file has an invalid format");
    }
    vpt_1 = vpt_string.substr(1, separator-1);
    vpt_2 = vpt_string.substr(separator+1, vpt_string.size()-separator-1);
    vpts.push_back(Vanishing_point((double)atof(vpt_1.c_str()),(double)atof(vpt_2.c_str())));
    vpts.push_back(vert_vpt);

    separator = focal_line.find(":");
    if( separator == std::string::npos)
    {
        KJB_THROW_2(IO_error, "The first line of the CMU vanishing points file has an invalid format");
    }
    std::string focal_string = focal_line.substr(separator+2, focal_line.size() - separator - 2);
    focal_length = (double)atof(focal_string.c_str());
    ifs.close();
}

void kjb::draw_mid_point_to_vanishing_point
(
    kjb::Image & img,
    const Line_segment & segment,
    const Vanishing_point & vpt,
    double ir,
    double ig,
    double ib,
    double width
)
{
    double _x = vpt.get_x();
    double _y = vpt.get_y();
    switch(vpt.get_type())
    {
        case Vanishing_point::REGULAR:
            break;
        case Vanishing_point::INFINITY_DOWN :
            _x = segment.get_centre_x();
            _y = 1000000;
            break;
        case Vanishing_point::INFINITY_UP :
            _x = segment.get_centre_x();
            _y = -1000000;
            break;
        case Vanishing_point::INFINITY_RIGHT :
            _x = 1000000;
            _y = segment.get_centre_y();
            break;
        case Vanishing_point::INFINITY_LEFT :
            _x = -1000000;
            _y = segment.get_centre_y();
            break;
        default:
            break;
    }

    kjb_c::image_draw_segment_2(img.non_const_c_ptr(), segment.get_centre_y(), segment.get_centre_x(),
         _y, _x, width, ir,ig,ib);
}

bool kjb::find_vanishing_point_given_one_and_line
(
    const kjb::Vanishing_point & vp1,
    double focal,
    unsigned int img_cols,
    unsigned int img_rows,
    const Line_segment & ls,
    Vanishing_point & vpt
)
{
    if(vp1.is_at_infinity())
    {
        return false;
    }
    double center_x = ((double)img_cols) / 2.0;
    double center_y = ((double)img_rows) / 2.0;
    double nx = vp1.get_x() - center_x;
    double ny = vp1.get_y() - center_y;

    double ny0 = 0.0;
    double nx0 = 0.0;
    if(fabs(nx) > fabs(ny))
    {
        nx0 = - (focal*focal)/nx;
    }
    else
    {
        ny0 = - (focal*focal)/ny;
    }

    nx0 += center_x;
    ny0 += center_y;

    Line l(nx, ny, -(nx*nx0) - (ny*ny0));

    Vector ints;
    if(! Line::find_line_intersection(l, ls.get_line(), ints))
    {
        return false;
    }
    vpt.set_type(Vanishing_point::REGULAR);
    vpt.set_point(ints(0), ints(1));
    return true;
}

bool kjb::find_third_vanishing_point
(
    const kjb::Vanishing_point & vp1,
    const kjb::Vanishing_point & vp2,
    double focal,
    unsigned int img_rows,
    unsigned int img_cols,
    Vanishing_point & vpt
)
{
    if(vp1.is_at_infinity() || vp2.is_at_infinity())
    {
        return false;
    }

    double center_x = ((double)img_cols) / 2.0;
    double center_y = ((double)img_rows) / 2.0;

    Vector v1(3, focal);
    v1(0) = vp1.get_x() - center_x;
    v1(1) = vp1.get_y() - center_y;
    Vector v2(3, focal);
    v2(0) = vp2.get_x() - center_x;
    v2(1) = vp2.get_y() - center_y;
    v1.normalize();
    v2.normalize();
    Vector v3 = v1.cross_with(v2);
    if( fabs(v3(2)) < DBL_EPSILON  )
    {
        return false;
    }
    v3 /= v3(2);
    v3 *= focal;
    vpt.set_type(Vanishing_point::REGULAR);
    vpt.set_x(v3(0) + center_x);
    vpt.set_y(v3(1) + center_y);
    return true;
}


bool kjb::read_hedau_vanishing_points
(
    std::vector<Vanishing_point> & vpts,
    double & focal,
    const std::string & file_path,
    unsigned int num_cols,
    unsigned int num_rows
)
{
    std::ifstream ifs(file_path.c_str());
    std::string vpts_line;
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "Could not open file for reading Hedau vanishing points");
    }
    getline(ifs, vpts_line);
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "Could not open file for reading Hedau vanishing points");
    }
    getline(ifs, vpts_line);
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "Could not open file for reading Hedau vanishing points");
    }
    getline(ifs, vpts_line);
    if(ifs.fail())
    {
        KJB_THROW_2(IO_error, "Could not open file for reading Hedau vanishing points");
    }

    ifs.close();
    using std::istringstream;
    istringstream ist(vpts_line);

    double vx1, vy1, vx2, vy2, vx3, vy3;
    ist >> vx1 >> vy1 >> vx2 >> vy2 >> vx3 >> vy3;
    vpts.clear();
    vpts.push_back(Vanishing_point(vx1, vy1));
    vpts.push_back(Vanishing_point(vx2, vy2));
    vpts.push_back(Vanishing_point(vx3, vy3));
    std::cout << "Done reading" << std::endl;
    return find_vertical_vanishing_point(vpts[0], vpts[1], vpts[2], num_cols, num_rows);
}

bool kjb::find_vertical_vanishing_point
(
    Vanishing_point & vp1,
    Vanishing_point & vp2,
    Vanishing_point & estimated_vertical,
    unsigned int img_cols,
    unsigned int img_rows
)
{
    double vp1x = 0.0, vp1y = 0.0, vp2x = 0.0, vp2y = 0.0, vp3x = 0.0, vp3y = 0.0;
    int inf_ver = 0;
    int inf_lr = 0;
    switch(vp1.get_type())
    {
        case Vanishing_point::REGULAR:
            vp1x = vp1.get_x();
            vp1y = vp1.get_y();
            break;
        case Vanishing_point::INFINITY_DOWN:
            vp1x = ((double)img_cols) / 2.0;
            vp1y = INFINITY_APPROXIMATION;
            inf_ver++;
            break;
        case Vanishing_point::INFINITY_UP:
            vp1x = ((double)img_cols) / 2.0;
            vp1y = - INFINITY_APPROXIMATION;
            inf_ver++;
            break;
        case Vanishing_point::INFINITY_RIGHT:
            vp1y = ((double)img_rows) / 2.0;
            vp1y = INFINITY_APPROXIMATION;
            inf_lr++;
            break;
        case Vanishing_point::INFINITY_LEFT:
            vp1y = ((double)img_rows) / 2.0;
            vp1y = -INFINITY_APPROXIMATION;
            inf_lr++;
            break;
        default:
            break;
    }
    switch(vp2.get_type())
    {
        case Vanishing_point::REGULAR:
            vp2x = vp2.get_x();
            vp2y = vp2.get_y();
            break;
        case Vanishing_point::INFINITY_DOWN:
            vp2x = ((double)img_cols) / 2.0;
            vp2y = INFINITY_APPROXIMATION;
            inf_ver++;
            break;
        case Vanishing_point::INFINITY_UP:
            vp2x = ((double)img_cols) / 2.0;
            vp2y = - INFINITY_APPROXIMATION;
            inf_ver++;
            break;
        case Vanishing_point::INFINITY_RIGHT:
            vp2y = ((double)img_rows) / 2.0;
            vp2y = INFINITY_APPROXIMATION;
            inf_lr++;
            break;
        case Vanishing_point::INFINITY_LEFT:
            vp2y = ((double)img_rows) / 2.0;
            vp2y = -INFINITY_APPROXIMATION;
            inf_lr++;
            break;
        default:
            break;
    }
    switch(estimated_vertical.get_type())
    {
        case Vanishing_point::REGULAR:
            vp3x = estimated_vertical.get_x();
            vp3y = estimated_vertical.get_y();
            break;
        case Vanishing_point::INFINITY_DOWN:
            vp3x = ((double)img_cols) / 2.0;
            vp3y = INFINITY_APPROXIMATION;
            inf_ver++;
            break;
        case Vanishing_point::INFINITY_UP:
            vp3x = ((double)img_cols) / 2.0;
            vp3y = - INFINITY_APPROXIMATION;
            inf_ver++;
            break;
        case Vanishing_point::INFINITY_RIGHT:
            vp3y = ((double)img_rows) / 2.0;
            vp3y = INFINITY_APPROXIMATION;
            inf_lr++;
            break;
        case Vanishing_point::INFINITY_LEFT:
            vp3y = ((double)img_rows) / 2.0;
            vp3y = -INFINITY_APPROXIMATION;
            inf_lr++;
            break;
        default:
            break;
    }
    if(inf_lr > 1)
    {
        return false;
    }
    if(inf_ver > 1)
    {
        return false;
    }

    /*std::cout << vp1x << " " << vp1y << std::endl;
    std::cout << vp2x << " " << vp2y << std::endl;
    std::cout << vp3x << " " << vp3y << std::endl;*/
     double ver_dist_12 = fabs(vp1y-vp2y);
     double ver_dist_13 = fabs(vp1y-vp3y);
     double ver_dist_23 = fabs(vp2y-vp3y);
     double min_dist = ver_dist_12;
     if(ver_dist_13 < ver_dist_12)
     {
         if(ver_dist_23 < ver_dist_13 )
         {
            // two and three are the horizontal vanishing points
             min_dist = ver_dist_23;
             Vanishing_point temp_vp = vp1;
             vp1 = estimated_vertical;
             estimated_vertical = temp_vp;
         }
         else
         {
            // one and three are the horizontal vanishing points
             min_dist = ver_dist_13;
             Vanishing_point temp_vp = vp2;
             vp2 = estimated_vertical;
             estimated_vertical = temp_vp;
         }
     }
     else if(ver_dist_23 < ver_dist_12)
     {
         // two and three are the horizontal vanishing points
         min_dist = ver_dist_23;
         Vanishing_point temp_vp = vp1;
         vp1 = estimated_vertical;
         estimated_vertical = temp_vp;
     }

     return true;

}

