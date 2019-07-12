/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

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
| Authors:
|     Luca Del Pero
|
* =========================================================================== */

#include "l/l_sys_debug.h"
#include "st_cpp/st_parapiped.h"
#include "st_cpp/st_parapiped.h"
#include "st_cpp/st_parapiped.h"
#include "st_cpp/st_perspective_camera.h"
#include "edge_cpp/manhattan_world.h"
#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace kjb;

#define MIN_FOCAL_LENGTH_BLOCK_PROPOSAL 250 /*200 */
#define SECOND_ATTEMPT_FOCAL_LENGTH_BLOCK_PROPOSAL 350
#define PITCH_THRESHOLD 0.15
#define MAX_PITCH 0.5
#define PITCH_RANGE (MAX_PITCH - PITCH_THRESHOLD)
#define FOCAL_INCREMENT 500
#define DISTANCE_TO_BOTTOM 40
#define FRAME_SIZE_AS_PP_HEIGHT_RATIO 0.0001
/*
 * @param ix The x coordinate of the centre
 * @param iy The y coordinate of the centre
 * @param iz The z coordinate of the centre
 * @param iw The width of this parapiped (along x axis)
 * @param ih The height of this parapiped (along y axis)
 * @param il The length of this parapiped (along z axis)
 * @param ipitch The pitch of this parapiped (rotation angle around its x axis)
 * @param iyaw The yaw of this parapiped (rotation angle around its y axis)
 * @param iroll The roll of this parapiped (rotation angle around its z axis)
 */
Parametric_parapiped::Parametric_parapiped(double ix, double iy, double iz,
                         double iw, double ih, double il,
                         double ipitch, double iyaw, double iroll) throw(kjb::Illegal_argument)
: Renderable_model(false), Readable(), Writeable(), centre(4, 1.0), rotation_angles(3, 0.0),
  rendering_interface(ix -iw/2,iy-ih/2,iz-il/2, ix+iw/2, iy-ih/2, iz-il/2, ix+iw/2,iy+ih/2, iz-il/2, ix+iw/2, iy+ih/2,iz+il/2)
{
    if( (iw <= 0.0) || (ih <= 0.0) || (il <= 0.0) )
    {
        throw kjb::Illegal_argument("Parapiped constructor, dimensions must be positive");
    }

    width = iw;
    height = ih;
    length = il;
    centre(0) = ix;
    centre(1) = iy;
    centre(2) = iz;
    rotation_angles(ST_PARAPIPED_PITCH) = ipitch;
    rotation_angles(ST_PARAPIPED_YAW) = iyaw;
    rotation_angles(ST_PARAPIPED_ROLL) = iroll;
    set_rendering_representation_dirty();

}

/*
 * @param src the Parametric_parapiped to copy into this one
 */
Parametric_parapiped::Parametric_parapiped(const Parametric_parapiped & src)
: Renderable_model(src), Readable(), Writeable(), centre(src.centre),
  rotation_angles(src.rotation_angles), rendering_interface(src.rendering_interface)
{
    width = src.width;
    height = src.height;
    length = src.length;
}

/*
 * @param fname The name of the file to read this parametric_parapiped from
 */
Parametric_parapiped::Parametric_parapiped(const char* fname) throw (kjb::Illegal_argument,
        kjb::IO_error)
:  Renderable_model(false),
   centre(4, 1.0),  rotation_angles(3, 0.0),
   rendering_interface(0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0)
{
    kjb::Readable::read(fname);
}

/*
 * @param in The input stream to read this parametric_parapiped from
 */
Parametric_parapiped::Parametric_parapiped(std::istream& in) throw (kjb::Illegal_argument,
        kjb::IO_error)
:  Renderable_model(true),
   centre(4, 1.0), rotation_angles(3, 0.0),
   rendering_interface(0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0)
{
    read(in);
}

/*
 * @param src The Parametric_parapiped to be assigned to this one
 */
Parametric_parapiped & Parametric_parapiped::operator = (const Parametric_parapiped & src)
{
    Renderable_model::operator=(src);
    width = src.width;
    height = src.height;
    length = src.length;

    centre = src.centre;

    rotation_angles = src.rotation_angles;

    rendering_interface = src.rendering_interface;

    return (*this);
}

/*
 * Virtual copy constructor
 */
Parametric_parapiped * Parametric_parapiped::clone() const
{
    return new  Parametric_parapiped(*this);
}

/*
 * @param iwidth The new width of this parapiped (along its x axis)
 */
void Parametric_parapiped::set_width(double iwidth) throw(kjb::Illegal_argument)
{
    if(iwidth <= 0)
    {
        throw kjb::Illegal_argument("Parapiped width must be positive");
    }

    width = iwidth;
    set_rendering_representation_dirty();

}

/*
 * @param iheight The new height of this parapiped (along its y axis)
 */
void Parametric_parapiped::set_height(double iheight) throw(kjb::Illegal_argument)
{
    if(iheight <= 0)
    {
        throw kjb::Illegal_argument("Parapiped height must be positive");
    }

    height = iheight;
    set_rendering_representation_dirty();
}

/*
 * @param ilength The new length of this parapiped (along its z axis)
 */
void Parametric_parapiped::set_length(double ilength)throw(kjb::Illegal_argument)
{
    if(ilength <= 0)
    {
        throw kjb::Illegal_argument("Parapiped length must be positive");
    }

    length = ilength;
    set_rendering_representation_dirty();
}

/*
 * @param ipitch The new pitch of this parapiped (rotation around its x axis)
 */
void Parametric_parapiped::set_pitch(double ip)
{
    rotation_angles(ST_PARAPIPED_PITCH) = ip;
    set_rendering_representation_dirty();
}

/*
 * @param iyaw The new yaw of this parapiped (rotation around its y axis)
 */
void Parametric_parapiped::set_yaw(double iy)
{
    rotation_angles(ST_PARAPIPED_YAW) = iy;
    set_rendering_representation_dirty();
}

/*
 * @param iroll The new roll of this parapiped (rotation around its z axis)
 */
void Parametric_parapiped::set_roll(double ir)
{
    rotation_angles(ST_PARAPIPED_ROLL) = ir;
    set_rendering_representation_dirty();
}

/**
 * Given 3 rotation angles, sets the corresponding Euler angles given the current Euler convention
 *
 * @param dpitch The amount of rotation arount the x-axis of this parapiped
 * @param dyaw   The amount of rotation arount the y-axis of this parapiped
 * @param droll  The amount of rotation arount the z-axis of this parapiped
 */
void Parametric_parapiped::compute_new_euler_angles_on_rotations(double dpitch, double dyaw, double droll)
{
    update_rendering_representation();
    rendering_interface.compute_new_euler_angles_on_rotations(dpitch, dyaw, droll, rotation_angles);
    set_rendering_representation_dirty();
}

/*
 * @param theta The amount of rotation to be done around the current x
 *              axis of the parapiped
 */
void Parametric_parapiped::rotate_around_x_axis(double theta)
{
    compute_new_euler_angles_on_rotations(theta, 0.0, 0.0);
}

/*
 * @param theta The amount of rotation to be done around the current y
 *              axis of the parapiped
 */
void Parametric_parapiped::rotate_around_y_axis(double theta)
{
    compute_new_euler_angles_on_rotations(0.0, theta, 0.0);
}

/*
 * @param theta The amount of rotation to be done around the current z
 *              axis of the parapiped
 */
void Parametric_parapiped::rotate_around_z_axis(double theta)
{
    compute_new_euler_angles_on_rotations(0.0, 0.0, theta);
}

/*
 * Order of rotation is: thetax, thetay, thetaz
 *
 * @param thetax The amount of rotation to be done around the current x
 *              axis of the parapiped
 * @param thetay The amount of rotation to be done around the current y
 *              axis of the parapiped
 * @param thetaz The amount of rotation to be done around the current z
 *              axis of the parapiped
 */
void Parametric_parapiped::rotate_around_parapiped_axes(double thetax, double thetay, double thetaz)
{
    compute_new_euler_angles_on_rotations(thetax, thetay, thetaz);
}

/*
 * @param icentre The new centre of this parapiped
 */
void Parametric_parapiped::set_centre(const kjb::Vector & icentre) throw(kjb::Illegal_argument)
{
    if(icentre.size() == 3)
    {
        centre(0) = icentre(0);
        centre(1) = icentre(1);
        centre(2) = icentre(2);
        centre(3) = 1.0;
    }
    else if(icentre.size() == 4)
    {
        if(fabs(icentre(3)) < 1e-126 )
        {
            throw kjb::Illegal_argument("Parapiped, set centre, input centre vector, homogeneous coordinate = 0.0");
        }
        centre = icentre/icentre(3);
    }
    else
    {
        throw kjb::Illegal_argument("Parapiped, set centre, input centre vector has wrong dimensions");
    }
    set_rendering_representation_dirty();
}

/*
 * @param ix The new x coordinate of the centre of this parapiped
 */
void Parametric_parapiped::set_centre_x(double ix) throw(kjb::Illegal_argument)
{
    centre(0) = ix;
    set_rendering_representation_dirty();
}

/*
 * @param iy The new y coordinate of the centre of this parapiped
 */
void Parametric_parapiped::set_centre_y(double iy) throw(kjb::Illegal_argument)
{
    centre(1) = iy;
    set_rendering_representation_dirty();
}

/*
 * @param iz The new z coordinate of the centre of this parapiped
 */
void Parametric_parapiped::set_centre_z(double iz) throw(kjb::Illegal_argument)
{
    centre(2) = iz;
    set_rendering_representation_dirty();
}

/**
 * Sets the rotation angles from an input quaternion
 *
 * @param q the input quaternion
 */
void Parametric_parapiped::set_angles_from_quaternion(const kjb::Quaternion & q)
{
    KJB(UNTESTED_CODE());
    rendering_interface.set_orientation(q);
    rotation_angles = rendering_interface.get_euler_angles();
}

/*
 * Updates the rendering representation of this parapiped.
 * First it creates a parapiped with centre in the origin, aligned
 * with world axes and with correct width, height and length.
 * Then it rotates it according to pitch, yaw and roll, and translates
 * it to the position specified by its centre
 */
void Parametric_parapiped::update_rendering_representation() const throw(kjb::KJB_error)
{
    rendering_interface.set_points(-width/2, -height/2, -length/2,width/2, -height/2, -length/2,
            width/2, height/2, -length/2, width/2, height/2, length/2);
    rendering_interface.set_rotations_and_translate(rotation_angles(ST_PARAPIPED_PITCH), rotation_angles(ST_PARAPIPED_YAW),
            rotation_angles(ST_PARAPIPED_ROLL), centre(0), centre(1), centre(2));
}

/*
 * @return the rendering interface used to render this parametric_parapiped
 */
Abstract_renderable & Parametric_parapiped::get_rendering_interface() const
{
    update_rendering_representation();
    return rendering_interface;
}

/*
 * @param in The input stream to read this parapiped from
 */
void Parametric_parapiped::read(std::istream& in) throw (kjb::Illegal_argument,
        kjb::IO_error)
{
    using std::ostringstream;
    using std::istringstream;

    const char* type_name = typeid(*this).name();
    const char* field_value;

     // Type
    if (!(field_value = read_field_value(in, "Type")))
    {
        throw Illegal_argument("Missing Type field");
    }
    if (strncmp(field_value, type_name, strlen(type_name)) != 0)
    {
        ostringstream ost;
        ost << "Tried to read a '" << field_value << "' as a '"
            << type_name << "'";
        throw Illegal_argument(ost.str());
    }

    if (!(field_value = read_field_value(in, "width")))
    {
        throw Illegal_argument("Missing width");
    }
    istringstream ist(field_value);
    ist >> width;
    if (ist.fail() || (width <= 0.0))
    {
        throw Illegal_argument("Invalid Width");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "height")))
    {
        throw Illegal_argument("Missing height");
    }
    ist.str(field_value);
    ist >> height;
    if (ist.fail() || (height <= 0.0) )
    {
        throw Illegal_argument("Invalid Height");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "length")))
    {
        throw Illegal_argument("Missing length");
    }
    ist.str(field_value);
    ist >> length;
    if (ist.fail() || (length <= 0.0))
    {
        throw Illegal_argument("Invalid Length");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "pitch")))
    {
        throw Illegal_argument("Missing pitch");
    }
    ist.str(field_value);
    ist >> rotation_angles(ST_PARAPIPED_PITCH);
    if (ist.fail())
    {
        throw Illegal_argument("Invalid Pitch");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "yaw")))
    {
        throw Illegal_argument("Missing yaw");
    }
    ist.str(field_value);
    ist >> rotation_angles(ST_PARAPIPED_YAW);
    if (ist.fail())
    {
        throw Illegal_argument("Invalid Yaw");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "roll")))
    {
        throw Illegal_argument("Missing roll");
    }
    ist.str(field_value);
    ist >> rotation_angles(ST_PARAPIPED_ROLL);
    if (ist.fail())
    {
        throw Illegal_argument("Invalid Roll");
    }
    ist.clear(std::ios_base::goodbit);

    // Parapiped centre
    if (!(field_value = read_field_value(in, "centre")))
    {
        throw Illegal_argument("Missing Parapiped Centre");
    }
    ist.str(field_value);
    ist >> centre(0) >> centre(1) >> centre(2) >> centre(3);
    double epsilon = 1e-127;
    if (ist.fail() || (fabs(centre(3)) < epsilon))
    {
        throw Illegal_argument("Invalid Camera centre");
    }
    ist.clear(std::ios_base::goodbit);

    centre = centre / centre(3);

    set_rendering_representation_dirty();

}

/*
 * @param out The output stream to write this parapiped to
 */
void Parametric_parapiped::write(std::ostream& out) const
   throw (kjb::IO_error)
{

    out << "     Type: " << typeid(*this).name() << '\n'
    << "      width: " << width << '\n'
    << "      height: " << height << '\n'
    << "      length: " << length << '\n'
    << "      pitch: " << rotation_angles(ST_PARAPIPED_PITCH) << '\n'
    << "      yaw: " << rotation_angles(ST_PARAPIPED_YAW) << '\n'
    << "      roll: " << rotation_angles(ST_PARAPIPED_ROLL) << '\n'
    << "      centre: " << centre(0) << ' '
                     << centre(1) << ' '
                     << centre(2) << ' '
                     << centre(3) << '\n';
}

void Parametric_parapiped::stretch_along_axis(
    unsigned int axis,
    double amount,
    bool direction
)
{
    if(axis > 2)
    {
        KJB_THROW_2(Illegal_argument, "Stretch along axis, axis index out of bounds");
    }

    Vector translation(2,0.0);

    if(axis == 0)
    {
        width += amount;
    }
    else if(axis == 1)
    {
        height += amount;
    }
    else
    {
        length += amount;
    }

    if(direction)
    {
        if(axis == 1)
        {
            centre(axis) += amount/2.0;
        }
        else
        {
            Matrix rot2D;
            rot2D.convert_to_2d_rotation_matrix(rotation_angles(ST_PARAPIPED_YAW));
            if(axis == 0)
            {
                translation(0) = amount/2.0;
            }
            else if(axis == 2)
            {
                translation(1) = amount/2.0;
            }
            translation = rot2D*translation;
            centre(0) += translation(0);
            centre(2) += translation(1);
        }

    }
    else
    {
        if(axis == 1)
        {
            centre(axis) -= amount/2.0;
        }
        else
        {
            Matrix rot2D;
            rot2D.convert_to_2d_rotation_matrix(rotation_angles(ST_PARAPIPED_YAW));
            if(axis == 0)
            {
                translation(0) = -amount/2.0;
            }
            else if(axis == 2)
            {
                translation(1) = -amount/2.0;
            }
            translation = rot2D*translation;
            centre(0) += translation(0);
            centre(2) += translation(1);
        }
    }
    set_rendering_representation_dirty();

}


void Parametric_parapiped::draw_orientation_map() const
{
    update_rendering_representation();
    ( (const Parapiped &) this->get_polymesh() ).draw_orientation_map();
}

void Parametric_parapiped::draw_left_right_orientation_map() const
{
    update_rendering_representation();
    ( (const Parapiped &) this->get_polymesh()).draw_left_right_orientation_map();
}

void Parametric_parapiped::draw_CMU_orientation_map() const
{
    update_rendering_representation();
    ( (const Parapiped &) this->get_polymesh()).draw_CMU_orientation_map();
}

void Parametric_parapiped::draw_geometric_context_map() const
{
    update_rendering_representation();
    ( (const Parapiped &) this->get_polymesh()).draw_geometric_context_map();
}

/**
 * Transforms a point in world coordinates to a coordinate
 *  system where the parapiped centre is the origin, and the
 *  axes are defined by the parapiped axes
 *  */
void Parametric_parapiped::get_point_in_parapiped_coordinates
(
    const kjb::Vector & point_in_world_coordinates,
    kjb::Vector & point_in_parapiped_coordinates
) const
{
    if(point_in_world_coordinates.size() != 4)
    {
        KJB_THROW_2(Illegal_argument,"Point in world coordinates must be in homogeneous coordinates");
    }
    if( fabs(point_in_world_coordinates(3)) < DBL_EPSILON)
    {
        KJB_THROW_2(Illegal_argument,"Point in world coordinates has homogeneous coordinate = 0");
    }
    point_in_parapiped_coordinates = point_in_world_coordinates/point_in_world_coordinates(3) - centre;
    point_in_parapiped_coordinates(3) = 1.0;
    point_in_parapiped_coordinates = (get_rotations_as_a_quaternion().get_rotation_matrix().transpose() )*point_in_parapiped_coordinates;

}

/** @brief Transforms a point in parapiped coordinates to
 *  world coordinates */
void Parametric_parapiped::get_point_in_world_coordinates
(
    const kjb::Vector & point_in_parapiped_coordinates,
    kjb::Vector & point_in_world_coordinates
) const
{
    if(point_in_parapiped_coordinates.size() != 4)
    {
        KJB_THROW_2(Illegal_argument,"Point in parapiped coordinates must be in homogeneous coordinates");
    }
    if( fabs(point_in_parapiped_coordinates(3)) < DBL_EPSILON)
    {
        KJB_THROW_2(Illegal_argument,"Point in parapiped coordinates has homogeneous coordinate = 0");
    }

    point_in_world_coordinates = point_in_parapiped_coordinates/point_in_parapiped_coordinates(3);
    point_in_world_coordinates = get_rotations_as_a_quaternion().get_rotation_matrix()*point_in_world_coordinates;
    point_in_world_coordinates = point_in_world_coordinates + centre;
    point_in_world_coordinates(3) = 1.0;
}

void Parametric_parapiped::get_lines(std::vector<Line3d> & lines)
{
    update_rendering_representation();
    rendering_interface.get_lines(lines);
}

void Parametric_parapiped::get_vertices(std::vector<Vector> & vertices)
{
    update_rendering_representation();
    rendering_interface.get_all_vertices(vertices);
}

double kjb::propose_parapiped_and_camera_from_orthogonal_corner_good
(
    kjb::Parametric_parapiped &pp,
    kjb::Perspective_camera & camera,
    const kjb::Manhattan_corner & corner,
    double focal_length,
    unsigned int num_rows,
    unsigned int num_cols,
    double corner_3D_z_position,
    double p_width_ratio,
    double p_height_ratio,
    double p_length_ratio
)
{
	//We should take into account the principal point here
    kjb::Vector corner1;
    kjb::Vector corner2;
    kjb::Vector corner3;
    kjb::Vector position_3D;

    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    /** Get the three vectors defining the directions of the orthogonal corner in 3D and the
     *  position of the 3D corner. The three vector will be orthogonal
     */
    corner.get_3D_corner(corner_3D_z_position, focal_length, princ_x, princ_y,
                        corner1,corner2,corner3, position_3D);

    kjb::Vector corner_pos(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_pos(i) = position_3D(i);
    }

    corner1(3) = 0.0;
    corner2(3) = 0.0;
    corner3(3) = 0.0;
    corner1 = corner1.normalize();
    corner2 = corner2.normalize();
    corner3 = corner3.normalize();

    /** We expect to have the most "vertical" vector defining the
     * 3D corner stored in corner2. If this is not the case, we swap
     * the vectors accordingly */
    if( fabs(corner1(1)) > fabs(corner2(1)) )
    {
        Vector temp = corner1;
        corner1 = corner2;
        corner2 = temp;
    }

    if( fabs(corner3(1)) > fabs(corner2(1)) )
    {
        Vector temp = corner1;
        corner1 = corner3;
        corner3 = temp;
    }

    /*** PITCH ***/
    /** We now rotate corner2 around the x axis until
     * its component along the z axis is 0. This amount
     * of rotation actually corresponds to the pitch of the
     * camera in a configuration where the parapiped is aligned
     * with the world axes, up to some roll and some yaw
     */
    kjb::Vector vertical_component(4,0.0);
    vertical_component(2) = 0.0;
    vertical_component(0) = corner2(0);
    if(corner2(1) < 0)
    {
        vertical_component(1) = -sqrt(1 - (corner2(0)*corner2(0)) );
    }
    else
    {
        vertical_component(1) = sqrt(1 - (corner2(0)*corner2(0)) );
    }

    double pitch = acos ( fabs(corner2(1)) / sqrt(corner2(1)*corner2(1) + corner2(2)*corner2(2)) );
    /** We determine if the rotation is clockwise or
     * counterclockwise */
    if( (corner2(2) > 0 ) && corner2(1) > 0)
    {
        pitch *= -1;
    }
    if( (corner2(2) < 0 ) && corner2(1) < 0)
    {
        pitch *= -1;
    }


    /** Now that we have found the pitch, we rotate the 3D directions of
     * the corner and its position by this angle around the x axis */
    Vector rotation_axis(3, 0.0);
    rotation_axis(0)  = 1.0;
    Quaternion q(rotation_axis, pitch);
    Matrix rotation = q.get_rotation_matrix();
    corner1(3) = 1.0;
    corner2(3) = 1.0;
    corner3(3) = 1.0;
    corner1 = rotation*corner1;
    corner2 = rotation*corner2;
    corner3 = rotation*corner3;
    corner_pos = rotation*corner_pos;

    /*** ROLL ***/
    /** Once again, we expect to have the most "vertical" vector defining the
     * 3D corner stored in corner2. If this is not the case, we swap
     * the vectors accordingly. This should happen very rarely at this point*/
    if( fabs(corner1(1)) > fabs(corner2(1)) )
    {
        Vector temp = corner1;
        corner1 = corner2;
        corner2 = temp;
    }

    if( fabs(corner3(1)) > fabs(corner2(1)) )
    {
        Vector temp = corner1;
        corner1 = corner3;
        corner3 = temp;
    }

    /** We now rotate the vertical direction of the corner around the
     * z-axis until its component along the x axis is 0. This angle corresponds
     * to the roll of the camera, in a configuration where the parapiped is
     * aligned with the world axes, up to some yaw
     */
    vertical_component(0) = 0.0;
    if(corner2(1) < 0)
    {
        vertical_component(1) = -1.0;
    }
    else
    {
        vertical_component(1) = 1.0;
    }
    corner2(3) = 0.0;
    Vector normal2 = corner2.normalize();
    double roll = acos(dot(normal2, vertical_component));
    corner2(3) = 1.0;
    /** We determine if the rotation is clockwise or
     * counterclockwise */
    if( (corner2(0) > 0 ) && corner2(1) < 0)
    {
        roll *= -1;
    }
    if( (corner2(0) < 0 ) && corner2(1) > 0)
    {
        roll *= -1;
    }

    /** Now that we have found the roll, we rotate the 3D directions of
     * the corner and its position by this angle around the z axis */
    rotation_axis(0) = 0.0;
    rotation_axis(1) = 0.0;
    rotation_axis(2)  = 1.0;
    try
    {
        q.set_axis_angle(rotation_axis, roll);
    } catch (KJB_error e)
    {
        throw e;
    }
    rotation = q.get_rotation_matrix();

    corner1 = rotation*corner1;
    corner2 = rotation*corner2;
    corner3 = rotation*corner3;
    corner_pos = rotation*corner_pos;

    Vector dimensions(3);

    /** Find the right height */
    double distance_camera_room = fabs(corner_pos(1));
    if(corner2(1) > 0.0)
    {
        /** This is the distance from the floor */
        //dimensions(1) = distance_camera_room*(1.8);
        //std::cout << "dist from the floor" << std::endl;
        //dimensions(1) = distance_camera_room*1.6;
    }
    else
    {
        /** This is the distance from the ceiling */
        //dimensions(1) = distance_camera_room*(2.0);
        //std::cout << "dist from the ceiling" << std::endl;
        //dimensions(1) = distance_camera_room*2.3;
    }
    dimensions(1) = distance_camera_room*p_height_ratio;
    dimensions(0) = dimensions(1)*p_width_ratio;
    dimensions(2) = dimensions(1)*p_length_ratio;

    //TODO find the right dimensions

    /** We now find the position in 3D of the centre of the parapiped */
    Vector centre(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        if(i == 1)
        {
            continue;
        }
        centre(i) = corner_pos(i);
        centre(i) += corner1(i)*(dimensions(0)/2.0);
        centre(i) += corner2(i)*(dimensions(1)/2.0);
        centre(i) += corner3(i)*(dimensions(2)/2.0);
    }
    centre(1) = corner_pos(1) - dimensions(1)/2.0;
    if(corner2(1) > 0.0 ) //TODO check this
    {
        centre(1) = corner_pos(1) + dimensions(1)/2.0;
    }

    /*** YAW ***/
    /* We  now find the yaw, that is to say the angle we have
     * to rotate the parapiped around its y-axis in order to
     * have it completely aligned with the world axes */

    /** This vector contains the direction vector between the
     * parapiped centre and the parapiped corner in the XZ plane */
    Vector centre_to_corner(2);
    centre_to_corner(0) = corner_pos(0) - centre(0);
    centre_to_corner(1) = corner_pos(2) - centre(2);
    centre_to_corner = centre_to_corner.normalize();

    /** This vector is the direction between the parapiped centre
     * and the parapiped corner on the XZ plane, when the parapiped
     * edges are aligned with the x and z axes.*/
    Vector aligned_centre_to_corner(2);
    aligned_centre_to_corner(0) = dimensions(0)/2.0;
    aligned_centre_to_corner(1) = -dimensions(2)/2.0;
    aligned_centre_to_corner = aligned_centre_to_corner.normalize();

    /** The angle between these two vectors corresponds to the yaw. We compute it using
     * atan2 so that the angle found is always clockwise around the y axis*/
    double yaw = atan2(aligned_centre_to_corner(0)*centre_to_corner(1) - aligned_centre_to_corner(1)*centre_to_corner(0),
                       aligned_centre_to_corner(0)*centre_to_corner(0) + aligned_centre_to_corner(1)*centre_to_corner(1));

    /** These two vectors contain the directions of the parapiped edges
     * when they are aligned with the world x and z axes. If we rotate them
     * by the yaw angle around the y axis, they should align with the parapiped
     * corner in its not aligned position. If they do not, it means that this
     * is the wrong corner, and we have to try with the other one (we do not
     * need to try all the four corners, since solutions are equivalent up to
     * 180 degrees rotations */
    Vector aligned_edge_x(2,0.0);
    aligned_edge_x(0) = -1.0;
    Vector aligned_edge_z(2,0.0);
    aligned_edge_z(1) = 1.0;
    Matrix rot2D;
    rot2D.convert_to_2d_rotation_matrix(-yaw);
    aligned_edge_x = rot2D*aligned_edge_x;
    aligned_edge_z = rot2D*aligned_edge_z;


    /** We have to compute the distance in two ways, because we do not know the
     * correspondence between the two aligned edges and the two not aligned edges.
     * The one giving the smalles difference is the right one*/
    double diff = fabs(aligned_edge_x(0) - corner1(0) ) + fabs(aligned_edge_x(1) - corner1(2) )
            +fabs(aligned_edge_z(0) - corner3(0) ) + fabs(aligned_edge_z(1) - corner3(2) );

    double diff2 = fabs(aligned_edge_x(0) - corner3(0) ) + fabs(aligned_edge_x(1) - corner3(2) )
            +fabs(aligned_edge_z(0) - corner1(0) ) + fabs(aligned_edge_z(1) - corner1(2) );
    if(diff2 < diff)
    {
        diff = diff2;
    }

    /** We now try with the other corner */
    aligned_centre_to_corner(0) = dimensions(0)/2.0;
    aligned_centre_to_corner(1) = dimensions(2)/2.0;
    aligned_centre_to_corner = aligned_centre_to_corner.normalize();

    double yawb = atan2(aligned_centre_to_corner(0)*centre_to_corner(1) - aligned_centre_to_corner(1)*centre_to_corner(0),
                        aligned_centre_to_corner(0)*centre_to_corner(0) + aligned_centre_to_corner(1)*centre_to_corner(1));
    Vector fix_axis_xb(2,0.0);
    aligned_edge_x(0) = -1.0;
    aligned_edge_x(1) = 0.0;
    aligned_edge_z(0) = 0.0;
    aligned_edge_z(1) = -1.0;
    rot2D.convert_to_2d_rotation_matrix(-yawb);
    aligned_edge_x = rot2D*aligned_edge_x;
    aligned_edge_z = rot2D*aligned_edge_z;
    double diffb = fabs(aligned_edge_x(0) - corner1(0) ) + fabs(aligned_edge_x(1) - corner1(2) )
                +fabs(aligned_edge_z(0) - corner3(0) ) + fabs(aligned_edge_z(1) - corner3(2) );

    double diffb2 = fabs(aligned_edge_x(0) - corner3(0) ) + fabs(aligned_edge_x(1) - corner3(2) )
            +fabs(aligned_edge_z(0) - corner1(0) ) + fabs(aligned_edge_z(1) - corner1(2) );
    bool down = true;
    if(diffb2 < diffb)
    {
        diffb = diffb2;
        down = false;
    }
    if(diffb < diff)
    {
        /** We keep the right yaw */
        yaw = yawb;
    }

    /** We now find the amount of stretch necessary so that the camera
     * will be inside the room */
    Vector camera_xz_in_pp_coordinates(2);
    camera_xz_in_pp_coordinates(0) = -centre(0);
    camera_xz_in_pp_coordinates(1) = -centre(2);
    rot2D.convert_to_2d_rotation_matrix(yaw);
    camera_xz_in_pp_coordinates = rot2D*camera_xz_in_pp_coordinates;

    /** Now we set the parameters of the parapiped */
    pp.set_roll(0.0);
    pp.set_pitch(0.0);
    pp.set_yaw(yaw);
    pp.set_width(dimensions[0]);
    pp.set_height(dimensions[1]);
    pp.set_length(dimensions[2]);
    pp.set_centre_x(centre(0));
    pp.set_centre_y(centre(1));
    pp.set_centre_z(-centre(2));

    if( fabs(camera_xz_in_pp_coordinates(0)) > (dimensions[0]/2.0) )
    {
        double stretch_x = (fabs(camera_xz_in_pp_coordinates(0)) - (dimensions[0]/2.0) )  + 0.1;
        if( camera_xz_in_pp_coordinates(0) < 0.0 )
        {
            pp.stretch_along_axis(0,stretch_x,false);
        }
        else
        {
            pp.stretch_along_axis(0,stretch_x,true);
        }
    }

    if( fabs(camera_xz_in_pp_coordinates(1)) > (dimensions[2]/2.0) )
    {
        double stretch_z = (fabs(camera_xz_in_pp_coordinates(1)) - (dimensions[2]/2.0) )  + 0.1;
        if(down)
        {
            pp.stretch_along_axis(2,stretch_z,true);
        }
        else
        {
            pp.stretch_along_axis(2,stretch_z,false);
        }

    }

    /** Now we set the parameters of the camera */
    camera.set_camera_centre_x(0.0);
    camera.set_camera_centre_y(0.0);
    camera.set_camera_centre_z(0.0);
    camera.set_pitch(pitch);
    camera.set_roll(-roll);
    camera.set_yaw(0.0);
    camera.set_focal_length(focal_length);
    camera.set_aspect_ratio(1.0);
    camera.set_principal_point_x(0.0);
    camera.set_principal_point_y(0.0);
    camera.set_skew(1.57079633);

    return pitch;

}

bool kjb::propose_parapiped_inside_parapiped_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_room
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(imin_desired_dimensions.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Propose parapiped inside parapiped, desired_dimensions must be of size 3");
    }
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    if(expansion_deltas.size() != 3)
    {
        expansion_deltas.resize(3, 0.0);
    }
    if(expansion_directions.size() != 3)
    {
        expansion_directions.resize(3, true);
    }

    Vector desired_dimensions = imin_desired_dimensions;

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();

    /** These vector represent the corner directions onto the image plane */
    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);
    /** Direction 3 is direction closest to "vertical", using the Manhattan_corner convention */

    /** This tells us if the corner expands towards the ceiling or
     * towards the floor. Signs are inverted with respect to what
     * we would expect, since in the image y=0 is the top, and incresing
     * y goes towards the bottom of the image*/
    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    /** We get the position in 3D of the projection of the corner vertices onto the image
     * plane, with the principal point as the origin. This is in camera coordinates */
    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0)*1;
    vertex2(0) = (corner.get_position() )(0) + direction2(0)*1;
    vertex3(0) = (corner.get_position() )(0) + direction3(0)*1;

    vertex1(1) =  (corner.get_position() )(1) + direction1(1)*1;
    vertex2(1) =  (corner.get_position() )(1) + direction2(1)*1;
    vertex3(1) =  (corner.get_position() )(1) + direction3(1)*1;

    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();

    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    /** We normalize everything */
    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;

    /** We now find where the vector from the camera to the corner
     * projected onto the image plane intersects the pp lateral faces
     * and the top face (or bottom if this vector is going down) */
    Vector x_plane(4, 0.0);
    x_plane(0) = 1.0;
    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;
    Vector z_plane(4, 0.0);
    z_plane(2) = 1.0;

    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width = pp.get_width();
    /*desired_dimensions(0) *= pp_width;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_length;*/
    desired_dimensions(0) *= pp_height;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_height;

    /** We consider the intersection with the faces only
     * in the direction determined by the vector from the
     * camera to the corner. We find the farthest intersection,
     * which is also the farthest possible position of the corner
     * such that the corner is still inside the parapiped */
    if(camera_to_corner(1) >= 0.0)
    {
        if(expand_up)
        {
            /** This will lead to a configuration where the new parapiped
             * will not lie on the base of the parapiped containing it.*/
            //TODO We might want to consider moving the floor up here
            return false;
        }

        y_plane(3) = - pp_height/2.0;

    }
    else
    {
        y_plane(3) = pp_height/2.0;
    }
    if(camera_to_corner(0) >= 0.0)
    {
        x_plane(3) = -pp_width / 2.0;
    }
    else
    {
        x_plane(3) = pp_width/2.0;
    }
    if(camera_to_corner(2) >= 0.0 )
    {
        z_plane(3) = -pp_length/2.0;
    }
    else
    {
        z_plane(3) = pp_length/2.0;
    }

    Vector intersection;
    double t_x = 0.0;
    double max_t = 0.0;
    bool found_t = false;
    if(kjb::intersect_3D_line_with_plane(intersection, t_x, camera_centre_in_pp, camera_to_corner, x_plane) )
    {
        ASSERT(t_x);
        max_t = t_x;
        found_t = true;
    }
    double t_y = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, y_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_y);
            max_t = t_y;
            found_t = true;
        }
        else if(t_y < max_t)
        {
            max_t = t_y;
        }
    }
    double t_z = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_z, camera_centre_in_pp, camera_to_corner, z_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_z);
            max_t = t_z;
            found_t = true;
        }
        else if(t_z < max_t)
        {
            max_t = t_z;
        }
    }

    if(!found_t)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
    }

    /** We now use t to position the corner in space */

    double chosen_t = 0.0;
    /** This will be set to true if the parapiped is too small
    * to contain a parapiped with the desired dimensions */
    bool need_to_expand = false;
    /** These variables will contain the amount of stretch necessary so
     * that the parapiped will be able to contain a parapiped of the
     * desired dimensions */
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    bool expand_pp_right = true;
    bool expand_pp_up = expand_up;
    bool expand_pp_z_up = true;
    if( (t_y > max_t) && (expand_up))
    {
        /** We need to expand the room if we want the inner parapiped to lie
         * on the base of the parapiped that contains it */
        chosen_t = t_y;
        need_to_expand = true;
        if(!expand_room)
        {
            return false;
        }

    }
    else if(expand_up)
    {
        /** By positioning the  bottom corner of the inner parapiped onto the base of the
         *  containing parapiped, we ensure that the inner parapiped will lie
         *  on the base */
        chosen_t = t_y;
    }
    else
    {
        chosen_t = distance_from_camera*max_t;
    }

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + chosen_t*camera_to_corner(i);
    }

    if(corner_3D_in_pp(0) < 0.0)
    {
        expand_pp_right = false;
    }
    if(corner_3D_in_pp(2) < 0.0)
    {
        expand_pp_z_up = false;
    }

    /** The height of the corner position (the actual corner now,
     *  not its projections, also determines the height of the points
     *  that generated the corner vertices onto the image plane */
    double t1 = 0.0;
    if(fabs(vertex1(1)) > DBL_EPSILON)
    {
        t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    }
    else
    {
        if(vertex1(1) < 0.0)
        {
             t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    /** The vector (x1_edge, z1_edge) and (x2_edge,z2_edge) represent the direction of
     *  the corner vertices onto the xz plane. we will make sure that the first one
     *  expands mostly along the x axis, the second along the z axis */
    double x1_edge = camera_centre_in_pp(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_pp(2) + t1*vertex1(2);

    double t2 = 0.0;
    if(fabs(vertex2(1)) > DBL_EPSILON)
    {
        t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex2(1);
    }
    else
    {
        if(vertex3(1) < 0.0)
        {
             t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    double x2_edge = camera_centre_in_pp(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_pp(2) + t2*vertex2(2);

    x1_edge -= corner_3D_in_pp(0);
    x2_edge -= corner_3D_in_pp(0);
    z1_edge -= corner_3D_in_pp(2);
    z2_edge -= corner_3D_in_pp(2);

    double norm_1 = sqrt(x1_edge*x1_edge +z1_edge*z1_edge);
    double norm_2 = sqrt(x2_edge*x2_edge +z2_edge*z2_edge);
    x1_edge /= norm_1;
    z1_edge /= norm_1;
    x2_edge /= norm_2;
    z2_edge /= norm_2;


    /** The first edge expands mostly along the x axis,
         * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }

    /** Given an orthogonal corner, there can be two 3D configurations that
     *  could have generated it. We make sure we pick the one such that the
     *  camera is not contained into the inner parapiped */
    if(camera_to_corner(0) > 0.0)
    {
        if(camera_to_corner(2) > 0.0)
        {
            /** ^
             *  | ->
             */
            if( (x1_edge < 0) && (z2_edge < 0))
            {
                //x1_edge = - x1_edge;
                //z2_edge = - z2_edge;
            }
        }
        else
        {
            /**
             *  | ->
             *  v
             */
            if( (x1_edge < 0) && (z2_edge > 0))
            {
                //x1_edge = - x1_edge;
                //z2_edge = - z2_edge;
            }
        }
    }
    else
    {
        if(camera_to_corner(2) > 0.0)
        {
            /**    ^
             *   <-|
             *
             */
            if( (x1_edge > 0) && (z2_edge < 0))
            {
                //x1_edge = - x1_edge;
                //z2_edge = - z2_edge;
            }
        }
        else
        {
           /**
             *  <-|
             *    v
            */
            if( (x1_edge > 0) && (z2_edge > 0))
            {
                //x1_edge = - x1_edge;
                //z2_edge = - z2_edge;
            }

        }
    }

    /** In case we need to expand the container, we determine in which direction */
    bool expand_right = true;
    if(x1_edge < 0.0)
    {
        expand_right = false;
    }
    bool expand_z_up = true;
    if(z2_edge < 0.0)
    {
        expand_z_up = false;
    }

    if(need_to_expand)
    {
        if(!expand_room)
        {
            return false;
        }
        if( fabs(corner_3D_in_pp(0)) > (pp_width/2.0) )
        {
            expand_x = fabs(corner_3D_in_pp(0)) - (pp_width/2.0) ;
            if(!expand_pp_right)
            {
                corner_3D_in_pp(0) += (expand_x/2.0);
            }
            else
            {
                corner_3D_in_pp(0) -= (expand_x/2.0);
            }
            pp_width += expand_x;
        }
        if( fabs(corner_3D_in_pp(2)) > (pp_length/2.0) )
        {
            expand_z = fabs(corner_3D_in_pp(2)) - (pp_length/2.0) ;
            if(!expand_pp_z_up)
            {
                corner_3D_in_pp(2) += (expand_z/2.0);
            }
            else
            {
                corner_3D_in_pp(2) -= (expand_z/2.0);
            }
            pp_length += expand_z;
        }
    }

    /** Now find the max size allowed for the inner parapiped, given the position
     * of the corner, the directions determined by the corner vertices, and the
     * size of the container */
    double max_size_x = 0.0;
    if(expand_right)
    {
        max_size_x = ( pp_width/2.0 ) - corner_3D_in_pp(0);
    }
    else
    {
        max_size_x = fabs( ( -pp_width/2.0 ) - corner_3D_in_pp(0) );
    }
    double max_size_y = 0.0;
    if(expand_up)
    {
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_pp(1);
    }
    else
    {
        max_size_y = fabs( ( -pp_height/2.0 ) - corner_3D_in_pp(1) );
    }
    double max_size_z = 0.0;
    if(expand_z_up)
    {
        max_size_z = ( pp_length/2.0 ) - corner_3D_in_pp(2);
    }
    else
    {
        max_size_z = fabs( ( -pp_length/2.0 ) - corner_3D_in_pp(2) );
    }

    /** If these sizes are smaller than the minimum desired dimensions,
     *  we determine how much we have to increase the size of the container */
    if(max_size_x < desired_dimensions(0))
    {
        if(expand_room)
        {
            double temp_expand_x = desired_dimensions(0) - max_size_x;
            pp_width += temp_expand_x;
            max_size_x = desired_dimensions(0);
            if(!expand_right)
            {
                corner_3D_in_pp(0) += (temp_expand_x/2.0);
            }
            else
            {
                corner_3D_in_pp(0) -= (temp_expand_x/2.0);
            }

            if(expand_right == expand_pp_right)
            {
                expand_x += temp_expand_x;
            }
            else
            {
                expand_x -= temp_expand_x;
                if(expand_x < 0.0)
                {
                    expand_x = -expand_x;
                    expand_pp_right = !expand_pp_right;
                }
            }
            need_to_expand = true;
        }
        else
        {
            desired_dimensions(0) = max_size_x;
            if(max_size_x < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_y < desired_dimensions(1))
    {
        if(expand_room)
        {
            double temp_expand_y = desired_dimensions(1) - max_size_y;
            max_size_y = desired_dimensions(1);
            pp_height += temp_expand_y;
            if(!expand_up)
            {
                corner_3D_in_pp(1) += (temp_expand_y/2.0);
            }
            else
            {
                corner_3D_in_pp(1) -= (temp_expand_y/2.0);
            }

            if(expand_up == expand_pp_up)
            {
                expand_y += temp_expand_y;
            }
            else
            {
                expand_y -= temp_expand_y;
                if(expand_y < 0.0)
                {
                    expand_y = -expand_y;
                    expand_pp_up = !expand_pp_up;
                }
            }
            need_to_expand = true;
        }
        else
        {
            desired_dimensions(1) = max_size_y;
            if(max_size_y < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_z < desired_dimensions(2))
    {
        if(expand_room)
        {
            double temp_expand_z = desired_dimensions(2) - max_size_z;
            pp_length += temp_expand_z;
            max_size_z = desired_dimensions(2);
            if(!expand_z_up)
            {
                corner_3D_in_pp(2) += (temp_expand_z/2.0);
            }
            else
            {
                corner_3D_in_pp(2) -= (temp_expand_z/2.0);
            }
            if(expand_z_up == expand_pp_z_up)
            {
                expand_z += temp_expand_z;
            }
            else
            {
                expand_z -= temp_expand_z;
                if(expand_z < 0.0)
                {
                    expand_z = -expand_z;
                    expand_pp_z_up = !expand_pp_z_up;
                }
            }
            need_to_expand = true;
        }
        else
        {
            desired_dimensions(2) = max_size_z;
            if(max_size_z < 0.001)
            {
                return false;
            }
        }
    }

    centre(0) = corner_3D_in_pp(0);
    centre(1) = corner_3D_in_pp(1);
    centre(2) = corner_3D_in_pp(2);



    dimensions(0) = desired_dimensions(0) - 0.0001;
    dimensions(1) = desired_dimensions(1) - 0.0001;
    dimensions(2) = desired_dimensions(2) - 0.0001;

    /** We position the corner at the right place given the corner
     * position and the direction of the vertices. This is in a coordinate
     * system defined by the containing parapiped */
    if(expand_right)
    {
        centre(0) +=  dimensions(0)/2.0;
    }
    else
    {
        centre(0) -= dimensions(0)/2.0;
    }
    if(expand_up)
    {
        centre(1) += dimensions(1)/2.0;
    }
    else
    {
        /** We make sure that the inner parapiped will lie on the floor */
        //dimensions(1) = fabs( corner_3D_in_pp(1) + (pp_height/2.0 ));
        centre(1) -= dimensions(1)/2.0;
    }
    if(expand_z_up)
    {
        centre(2) += dimensions(2)/2.0;
    }
    else
    {
        centre(2) -= dimensions(2)/2.0;
    }

    /** We expand the containing parapiped so that it will contain the inner one */
    if(need_to_expand)
    {
        expansion_deltas(0) = expand_x;
        expansion_deltas(1) = expand_y;
        expansion_deltas(2) = expand_z;
        expansion_directions[0] = expand_pp_right;
        expansion_directions[1] = expand_pp_up;
        expansion_directions[2] = expand_pp_z_up;
        if(expand_x > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(0, expand_x, expand_pp_right);
        }
        if(expand_y > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(1, expand_y, expand_pp_up);
        }
        if(expand_z > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(2, expand_z, expand_pp_z_up);
        }
    }

    return true;

}

/** Propose supported parapiped inside parapiped from orthogonal_corner
 * The height of the support planes are chosen from feasible ranges */
bool kjb::propose_parapiped_at_different_depth_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_room // if it is allowed to expand the room box 
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(imin_desired_dimensions.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Propose parapiped inside parapiped, desired_dimensions must be of size 3");
    }
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }

    Vector desired_dimensions = imin_desired_dimensions;

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
   
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();
    std::cout << "Corner position: " << corner_position << std::endl;

    /** These vector represent the corner directions onto the image plane */
    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);
    std::cout << "Direction1: " << direction1 << std::endl;
    std::cout << "Direction2: " << direction2 << std::endl;
    std::cout << "Direction3: " << direction3 << std::endl;
    /** Direction 3 is direction closest to "vertical", using the Manhattan_corner convention */

    /** This tells us if the corner expands towards the ceiling or
     * towards the floor. Signs are inverted with respect to what
     * we would expect, since in the image y=0 is the top, and incresing
     * y goes towards the bottom of the image*/
    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    /** We get the position in 3D of the projection of the corner vertices onto the image
     * plane, with the principal point as the origin. This is in camera coordinates */
    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0)*1;
    vertex2(0) = (corner.get_position() )(0) + direction2(0)*1;
    vertex3(0) = (corner.get_position() )(0) + direction3(0)*1;

    vertex1(1) =  (corner.get_position() )(1) + direction1(1)*1;
    vertex2(1) =  (corner.get_position() )(1) + direction2(1)*1;
    vertex3(1) =  (corner.get_position() )(1) + direction3(1)*1;

    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();

    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    /** We normalize everything */
    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    std::cout << "Vertex1: " << vertex1 << std::endl;
    std::cout << "Vertex2: " << vertex2 << std::endl;
    std::cout << "Vertex3: " << vertex3 << std::endl;
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;
    //std::cout << "Camera_to_corner: " << camera_to_corner << std::endl;

    /** We now find where the vector from the camera to the corner
     * projected onto the image plane intersects the pp lateral faces
     * and the top face (or bottom if this vector is going down) */
    Vector x_plane(4, 0.0);
    x_plane(0) = 1.0;
    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;
    Vector z_plane(4, 0.0);
    z_plane(2) = 1.0;

    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width = pp.get_width();
    
    desired_dimensions(0) *= pp_height;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_height;

    /** We consider the intersection with the faces only
     * in the direction determined by the vector from the
     * camera to the corner. We find the farthest intersection,
     * which is also the farthest possible position of the corner
     * such that the corner is still inside the parapiped */
    double max_base_y;
    double min_base_y;
    if(camera_to_corner(1) >= 0.0)
    {
        if(expand_up)
        {
            max_base_y = pp_height/2.0 - desired_dimensions(1);
            min_base_y = camera_centre_in_pp(1); 
        }
        else
        {
            max_base_y = pp_height/2.0;
            min_base_y = camera_centre_in_pp(1) + desired_dimensions(1); 
        }
    }
    else
    {
        if(expand_up)
        {
            max_base_y = -pp_height/2.0;
            min_base_y = camera_centre_in_pp(1); 
        }
        else
        {
            max_base_y = -pp_height/2.0 + desired_dimensions(1);
            min_base_y = camera_centre_in_pp(1) - desired_dimensions(1); 
        }
    }
    y_plane(3) = -(min_base_y*(1-distance_from_camera) + max_base_y*distance_from_camera);

    if(camera_to_corner(0) >= 0.0)
    {
        x_plane(3) = -pp_width / 2.0;
    }
    else
    {
        x_plane(3) = pp_width/2.0;
    }

    if(camera_to_corner(2) >= 0.0 )
    {
        z_plane(3) = -pp_length/2.0;
    }
    else
    {
        z_plane(3) = pp_length/2.0;
    }

    Vector intersection;
    double t_x = 0.0;
    double max_t = 0.0;
    bool found_t = false;
    if(kjb::intersect_3D_line_with_plane(intersection, t_x, camera_centre_in_pp, camera_to_corner, x_plane) )
    {
        ASSERT(t_x);
        max_t = t_x;
        found_t = true;
    }
    double t_y = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, y_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_y);
            max_t = t_y;
            found_t = true;
        }
        else if(t_y < max_t)
        {
            max_t = t_y;
        }
    }
    double t_z = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_z, camera_centre_in_pp, camera_to_corner, z_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_z);
            max_t = t_z;
            found_t = true;
        }
        else if(t_z < max_t)
        {
            max_t = t_z;
        }
    }

    if(!found_t)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
        return false; 
    }

    /** We now use t to position the corner in space */

    double chosen_t = 0.0;
    /** This will be set to true if the supporting object is too small
    * to support a parapiped with the desired dimensions */
    bool need_to_expand = false;
    /** These variables will contain the amount of stretch necessary so
     * that the supporting object will be able to support a parapiped of the
     * desired dimensions */
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    bool expand_pp_right = true;
    bool expand_pp_up = expand_up;
    bool expand_pp_z_up = true;
    if( (t_y > max_t) && (expand_up))
    {
        /** We need to expand the room if we want the inner parapiped to lie
         * on the base of the parapiped that contains it */
        chosen_t = t_y;
        need_to_expand = true;
        if(!expand_room)
        {
            return false;
        }

    }
    else
    {
        /** By positioning the  bottom corner of the inner parapiped onto the base of the
         *  containing parapiped, we ensure that the inner parapiped will lie
         *  on the base */
        chosen_t = t_y;
    }

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + chosen_t*camera_to_corner(i);
    }

    if(corner_3D_in_pp(0) < 0.0)
    {
        expand_pp_right = false;
    }
    if(corner_3D_in_pp(2) < 0.0)
    {
        expand_pp_z_up = false;
    }

    /** The height of the corner position (the actual corner now,
     *  not its projections, also determines the height of the points
     *  that generated the corner vertices onto the image plane */
    double t1 = 0.0;
    if(fabs(vertex1(1)) > DBL_EPSILON)
    {
        t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    }
    else
    {
        if(vertex1(1) < 0.0)
        {
             t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    /** The vector (x1_edge, z1_edge) and (x2_edge,z2_edge) represent the direction of
     *  the corner vertices onto the xz plane. we will make sure that the first one
     *  expands mostly along the x axis, the second along the z axis */
    double x1_edge = camera_centre_in_pp(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_pp(2) + t1*vertex1(2);

    double t2 = 0.0;
    if(fabs(vertex2(1)) > DBL_EPSILON)
    {
        t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex2(1);
    }
    else
    {
        if(vertex3(1) < 0.0)
        {
             t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    double x2_edge = camera_centre_in_pp(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_pp(2) + t2*vertex2(2);

    x1_edge -= corner_3D_in_pp(0);
    x2_edge -= corner_3D_in_pp(0);
    z1_edge -= corner_3D_in_pp(2);
    z2_edge -= corner_3D_in_pp(2);

    double norm_1 = sqrt(x1_edge*x1_edge +z1_edge*z1_edge);
    double norm_2 = sqrt(x2_edge*x2_edge +z2_edge*z2_edge);
    x1_edge /= norm_1;
    z1_edge /= norm_1;
    x2_edge /= norm_2;
    z2_edge /= norm_2;


    /** The first edge expands mostly along the x axis,
         * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }

    /** In case we need to expand the container, we determine in which direction */
    bool expand_right = true;
    if(x1_edge < 0.0)
    {
        expand_right = false;
    }
    bool expand_z_up = true;
    if(z2_edge < 0.0)
    {
        expand_z_up = false;
    }

    if(need_to_expand)
    {
        if(!expand_room)
        {
            return false;
        }
        if( fabs(corner_3D_in_pp(0)) > (pp_width/2.0) )
        {
            expand_x = fabs(corner_3D_in_pp(0)) - (pp_width/2.0) ;
            if(!expand_pp_right)
            {
                corner_3D_in_pp(0) += (expand_x/2.0);
            }
            else
            {
                corner_3D_in_pp(0) -= (expand_x/2.0);
            }
            pp_width += expand_x;
        }
        if( fabs(corner_3D_in_pp(2)) > (pp_length/2.0) )
        {
            expand_z = fabs(corner_3D_in_pp(2)) - (pp_length/2.0) ;
            if(!expand_pp_z_up)
            {
                corner_3D_in_pp(2) += (expand_z/2.0);
            }
            else
            {
                corner_3D_in_pp(2) -= (expand_z/2.0);
            }
            pp_length += expand_z;
        }
    }

    /** Now find the max size allowed for the inner parapiped, given the position
     * of the corner, the directions determined by the corner vertices, and the
     * size of the container */
    double max_size_x = 0.0;
    if(expand_right)
    {
        max_size_x = ( pp_width/2.0 ) - corner_3D_in_pp(0);
    }
    else
    {
        max_size_x = fabs( ( -pp_width/2.0 ) - corner_3D_in_pp(0) );
    }
    double max_size_y = 0.0;
    if(expand_up)
    {
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_pp(1);
    }
    else
    {
        max_size_y = fabs( ( -pp_height/2.0 ) - corner_3D_in_pp(1) );
    }
    double max_size_z = 0.0;
    if(expand_z_up)
    {
        max_size_z = ( pp_length/2.0 ) - corner_3D_in_pp(2);
    }
    else
    {
        max_size_z = fabs( ( -pp_length/2.0 ) - corner_3D_in_pp(2) );
    }

    /** If these sizes are smaller than the minimum desired dimensions,
     *  we determine how much we have to increase the size of the container */
    if(max_size_x < desired_dimensions(0))
    {
        if(expand_room)
        {
            double temp_expand_x = desired_dimensions(0) - max_size_x;
            pp_width += temp_expand_x;
            max_size_x = desired_dimensions(0);
            if(!expand_right)
            {
                corner_3D_in_pp(0) += (temp_expand_x/2.0);
            }
            else
            {
                corner_3D_in_pp(0) -= (temp_expand_x/2.0);
            }

            if(expand_right == expand_pp_right)
            {
                expand_x += temp_expand_x;
            }
            else
            {
                expand_x -= temp_expand_x;
                if(expand_x < 0.0)
                {
                    expand_x = -expand_x;
                    expand_pp_right = !expand_pp_right;
                }
            }
            need_to_expand = true;
        }
        else
        {
            desired_dimensions(0) = max_size_x;
            if(max_size_x < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_y < desired_dimensions(1))
    {
        if(expand_room)
        {
            double temp_expand_y = desired_dimensions(1) - max_size_y;
            max_size_y = desired_dimensions(1);
            pp_height += temp_expand_y;
            if(!expand_up)
            {
                corner_3D_in_pp(1) += (temp_expand_y/2.0);
            }
            else
            {
                corner_3D_in_pp(1) -= (temp_expand_y/2.0);
            }

            if(expand_up == expand_pp_up)
            {
                expand_y += temp_expand_y;
            }
            else
            {
                expand_y -= temp_expand_y;
                if(expand_y < 0.0)
                {
                    expand_y = -expand_y;
                    expand_pp_up = !expand_pp_up;
                }
            }
            need_to_expand = true;
        }
        else
        {
            desired_dimensions(1) = max_size_y;
            if(max_size_y < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_z < desired_dimensions(2))
    {
        if(expand_room)
        {
            double temp_expand_z = desired_dimensions(2) - max_size_z;
            pp_length += temp_expand_z;
            max_size_z = desired_dimensions(2);
            if(!expand_z_up)
            {
                corner_3D_in_pp(2) += (temp_expand_z/2.0);
            }
            else
            {
                corner_3D_in_pp(2) -= (temp_expand_z/2.0);
            }
            if(expand_z_up == expand_pp_z_up)
            {
                expand_z += temp_expand_z;
            }
            else
            {
                expand_z -= temp_expand_z;
                if(expand_z < 0.0)
                {
                    expand_z = -expand_z;
                    expand_pp_z_up = !expand_pp_z_up;
                }
            }
            need_to_expand = true;
        }
        else
        {
            desired_dimensions(2) = max_size_z;
            if(max_size_z < 0.001)
            {
                return false;
            }
        }
    }

    centre(0) = corner_3D_in_pp(0);
    centre(1) = corner_3D_in_pp(1);
    centre(2) = corner_3D_in_pp(2);



    dimensions(0) = desired_dimensions(0) - 0.0001;
    dimensions(1) = desired_dimensions(1) - 0.0001;
    dimensions(2) = desired_dimensions(2) - 0.0001;

    /** We position the corner at the right place given the corner
     * position and the direction of the vertices. This is in a coordinate
     * system defined by the containing parapiped */
    if(expand_right)
    {
        centre(0) +=  dimensions(0)/2.0;
    }
    else
    {
        centre(0) -= dimensions(0)/2.0;
    }
    if(expand_up)
    {
        centre(1) += dimensions(1)/2.0;
    }
    else
    {
        /** We make sure that the inner parapiped will lie on the floor */
        //dimensions(1) = fabs( corner_3D_in_pp(1) + (pp_height/2.0 ));
        centre(1) -= dimensions(1)/2.0;
    }
    if(expand_z_up)
    {
        centre(2) += dimensions(2)/2.0;
    }
    else
    {
        centre(2) -= dimensions(2)/2.0;
    }

    /** We expand the containing parapiped so that it will contain the inner one */
    if(need_to_expand)
    {
        expansion_deltas(0) = expand_x;
        expansion_deltas(1) = expand_y;
        expansion_deltas(2) = expand_z;
        expansion_directions[0] = expand_pp_right;
        expansion_directions[1] = expand_pp_up;
        expansion_directions[2] = expand_pp_z_up;
        if(expand_x > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(0, expand_x, expand_pp_right);
        }
        if(expand_y > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(1, expand_y, expand_pp_up);
        }
        if(expand_z > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(2, expand_z, expand_pp_z_up);
        }
    }

    return true;
}

bool kjb::propose_parapiped_onside_parapiped_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_sp_obj, // if it is allowed to expand the supporting object
    kjb::Vector & sp_obj_centre, // centre of the supporting object
    kjb::Vector & sp_obj_dimensions // dimensions of the supporting object
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(imin_desired_dimensions.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Propose parapiped onside parapiped, desired_dimensions must be of size 3");
    }
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }

    double max_x = sp_obj_centre(0) + sp_obj_dimensions(0)/2.0;
    double min_x = sp_obj_centre(0) - sp_obj_dimensions(0)/2.0;
    double max_y = - new_pp.get_height()/2.0 + sp_obj_dimensions(1);// ensure the supporting object is standing on the floor
    double min_y = - new_pp.get_height()/2.0;
    //double max_y = sp_obj_centre(1) + sp_obj_dimensions(1)/2.0;
    //double min_y = sp_obj_centre(1) - sp_obj_dimensions(1)/2.0;
    double max_z = sp_obj_centre(2) + sp_obj_dimensions(2)/2.0;
    double min_z = sp_obj_centre(2) - sp_obj_dimensions(2)/2.0;

    Vector desired_dimensions = imin_desired_dimensions;

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
   
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();
    std::cout << "Corner position: " << corner_position << std::endl;

    /** These vector represent the corner directions onto the image plane */
    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);
    std::cout << "Direction1: " << direction1 << std::endl;
    std::cout << "Direction2: " << direction2 << std::endl;
    std::cout << "Direction3: " << direction3 << std::endl;
    /** Direction 3 is direction closest to "vertical", using the Manhattan_corner convention */

    /** This tells us if the corner expands towards the ceiling or
     * towards the floor. Signs are inverted with respect to what
     * we would expect, since in the image y=0 is the top, and incresing
     * y goes towards the bottom of the image*/
    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    /** We get the position in 3D of the projection of the corner vertices onto the image
     * plane, with the principal point as the origin. This is in camera coordinates */
    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0)*1;
    vertex2(0) = (corner.get_position() )(0) + direction2(0)*1;
    vertex3(0) = (corner.get_position() )(0) + direction3(0)*1;

    vertex1(1) =  (corner.get_position() )(1) + direction1(1)*1;
    vertex2(1) =  (corner.get_position() )(1) + direction2(1)*1;
    vertex3(1) =  (corner.get_position() )(1) + direction3(1)*1;

    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();

    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    /** We normalize everything */
    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    std::cout << "Vertex1: " << vertex1 << std::endl;
    std::cout << "Vertex2: " << vertex2 << std::endl;
    std::cout << "Vertex3: " << vertex3 << std::endl;
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;
    //std::cout << "Camera_to_corner: " << camera_to_corner << std::endl;

    /** We now find where the vector from the camera to the corner
     * projected onto the image plane intersects the top face of the 
     * supporting object */
    Vector sp_plane(4, 0.0);
    sp_plane(1) = 1;
    Vector x_near_plane(4, 0.0);
    x_near_plane(0) = 1;
    Vector x_far_plane(4, 0.0);
    x_far_plane(0) = 1;
    Vector z_near_plane(4, 0.0);
    z_near_plane(2) = 1;
    Vector z_far_plane(4, 0.0);
    z_far_plane(2) = 1;
    
    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width  = pp.get_width();
    
    desired_dimensions(0) *= pp_height;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_height;
    std::cout << "Desired dimensions: " << desired_dimensions<< std::endl;

    Vector intersection;
    double t_x_near = 0.0;
    double t_x_far = 0.0;
    double t_y = 0.0;
    double t_z_near = 0.0;
    double t_z_far = 0.0;
    double max_t = 0.0;
    double min_t = 0.0;
    bool found_t_near = false;
    bool found_t_far  = false;
    
    if (camera_to_corner(0) >= 0)
    {
        x_far_plane(3)  = -max_x;
        x_near_plane(3) = -min_x;
    }
    else
    {
        x_far_plane(3)  = -min_x;
        x_near_plane(3) = -max_x;
    }
    
    if (camera_to_corner(2) >= 0)
    {
        z_far_plane(3)  = -max_z;
        z_near_plane(3) = -min_z;
    }
    else
    {
        z_far_plane(3)  = -min_z;
        z_near_plane(3) = -max_z;
    }
   
    if (expand_up)
    {
        sp_plane(3) = -max_y;
    }
    else
    {
        /** We find the intersection of the line from camera to corner and the top face of the supported object */
        sp_plane(3) = -max_y - desired_dimensions(1);
        /** if the top surface of the supported object is higher than the ceiling, we either expand the room box up
         * or lower down the top face of the supporting object or return false */
    }
    if (-sp_plane(3) > pp_height/2.0)
    {
        sp_plane(3) = -pp_height/2.0;// for now we do no want to expand the room box 
    }

    //std::cout << "sp_plane: " << sp_plane << std::endl;
    //std::cout << "Cemara centre in pp: " << camera_centre_in_pp << std::endl;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, sp_plane) )
    {
        ASSERT(t_y);
        if (t_y < 0)
        {
            if (!expand_up)
            {
                sp_plane(3) = -max_y;
                if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, sp_plane) )
                {
                    ASSERT(t_y);
                    if (t_y < 0)
                    {
                        return false;
                    }
                    t_y = distance_from_camera*t_y;
                }
            }
            else
            {
                return false;
            }
        }
        max_t = t_y;
        min_t = t_y;
        found_t_near = true;
        found_t_far = true;
    }

    /** We find the intersection of the line from camera to corner and their near x-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_x_near, camera_centre_in_pp, camera_to_corner, x_near_plane) )
    {
        ASSERT(t_x_near);
        if (!found_t_near)
        {
            min_t = t_x_near;
            found_t_near = true;
        }
        else if (t_x_near > min_t)
        {
            min_t = t_x_near;
        }
    }
    
    /** We find the intersection of the line from camera to corner and thei further x-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_x_far, camera_centre_in_pp, camera_to_corner, x_far_plane) )
    {
        ASSERT(t_x_far);
        if (!found_t_far)
        {
            max_t = t_x_far;
            found_t_far = true;
        }
        else if (t_x_far < max_t)
        {
            max_t = t_x_far;
        }
    }

    /** We find the intersection of the line from camera to corner and their near z-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_z_near, camera_centre_in_pp, camera_to_corner, z_near_plane) )
    {
        ASSERT(t_z_near);
        if (!found_t_near)
        {
            min_t = t_z_near;
            found_t_near = true;
        }
        else if (t_z_near > min_t)
        {
            min_t = t_z_near;
        }
    }
    
    /** We find the intersection of the line from camera to corner and thei further z-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_z_far, camera_centre_in_pp, camera_to_corner, z_far_plane) )
    {
        ASSERT(t_z_far);
        if (!found_t_far)
        {
            max_t = t_z_far;
            found_t_far = true;
        }
        else if (t_z_far < max_t)
        {
            max_t = t_z_far;
        }
    }

    if(!found_t_near && !found_t_far)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
        return false; // TODO We probably need to expand the supporting object, but just leave it for now
    }

    /** We now use t to position the corner in space */

    double chosen_t = 0.0;
    /** This will be set to true if the supporting object is too small
    * to support a parapiped with the desired dimensions */
    bool need_to_expand = false;
    /** These variables will contain the amount of stretch necessary so
     * that the supporting object will be able to support a parapiped of the
     * desired dimensions */
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    if (t_y > max_t)
    {
        if(!expand_sp_obj)
        {
            chosen_t = max_t; // we put the corner of the supported parapiped on the furthest place of the supporting plane
        }
        else
        {
            /** We need to expand the room if we want the inner parapiped to lie
             * on the base of the parapiped that contains it */
            chosen_t = t_y;
            need_to_expand = true;
        }
    }
    else if (t_y < min_t)
    {
        if(!expand_sp_obj)
        {
            chosen_t = min_t; // we put the corner of the supported parapiped on the furthest place of the supporting plane
        }
        else
        {
            /** We need to expand the room if we want the inner parapiped to lie
             * on the base of the parapiped that contains it */
            chosen_t = t_y;
            need_to_expand = true;
        }
    }
    else
    {
        chosen_t = t_y;
    }

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + chosen_t*camera_to_corner(i);
    }
//    std::cout << "Corner_3D_in_pp: " << corner_3D_in_pp << std::endl;
    /** check if the corner is out of room box */
/*
    if (corner_3D_in_pp(0)<-pp_width/2.0 || corner_3D_in_pp(0)>pp_width/2.0)
    {
        std::cout << "x coordinate out of room box" << std::endl;
        return false;
    }

    if (corner_3D_in_pp(2)<-pp_length/2.0 || corner_3D_in_pp(2)>pp_length/2.0)
    {
        std::cout << "z coordinate out of room box" << std::endl;
        return false;
    }
*/
    /** The height of the corner position (the actual corner now,
     *  not its projections, also determines the height of the points
     *  that generated the corner vertices onto the image plane */
    double t1 = 0.0;
    if(fabs(vertex1(1)) > DBL_EPSILON)
    {
        t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    }
    else
    {
        if(vertex1(1) < 0.0)
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    /** The vector (x1_edge, z1_edge) and (x2_edge,z2_edge) represent the direction of
     *  the corner vertices onto the xz plane. we will make sure that the first one
     *  expands mostly along the x axis, the second along the z axis */
    double x1_edge = camera_centre_in_pp(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_pp(2) + t1*vertex1(2);

    double t2 = 0.0;
    if(fabs(vertex2(1)) > DBL_EPSILON)
    {
        t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex2(1);
    }
    else
    {
        if(vertex3(1) < 0.0)
        {
             t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    double x2_edge = camera_centre_in_pp(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_pp(2) + t2*vertex2(2);

    x1_edge -= corner_3D_in_pp(0);
    x2_edge -= corner_3D_in_pp(0);
    z1_edge -= corner_3D_in_pp(2);
    z2_edge -= corner_3D_in_pp(2);

    double norm_1 = sqrt(x1_edge*x1_edge +z1_edge*z1_edge);
    double norm_2 = sqrt(x2_edge*x2_edge +z2_edge*z2_edge);
    x1_edge /= norm_1;
    z1_edge /= norm_1;
    x2_edge /= norm_2;
    z2_edge /= norm_2;


    /** The first edge expands mostly along the x axis,
     * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }
    
    /** get the expansion direction of proposed object */
    bool expand_right = true;
    if(x1_edge < 0.0)
    {
        expand_right = false;
    }
    bool expand_z_up = true;
    if(z2_edge < 0.0)
    {
        expand_z_up = false;
    }
    if(need_to_expand)
    {
        if(!expand_sp_obj)
        {
            return false;
        }
        if( corner_3D_in_pp(0) > max_x )
        {
            expand_x = corner_3D_in_pp(0) - max_x;
            sp_obj_centre(0) += (expand_x/2.0);
            sp_obj_dimensions(0) += expand_x;
            max_x = sp_obj_centre(0) + sp_obj_dimensions(0)/2.0;
        }
        else if ( corner_3D_in_pp(0) < min_x )
        {
            expand_x = min_x - corner_3D_in_pp(0);
            sp_obj_centre(0) -= (expand_x/2.0);
            sp_obj_dimensions(0) += expand_x;
            min_x = sp_obj_centre(0) - sp_obj_dimensions(0)/2.0;
        }
        if( corner_3D_in_pp(2) > max_z )
        {
            expand_z = corner_3D_in_pp(2) - max_z;
            sp_obj_centre(2) += (expand_z/2.0);
            sp_obj_dimensions(2) += expand_z;
            max_z = sp_obj_centre(2) + sp_obj_dimensions(2)/2.0;
        }
        else if ( corner_3D_in_pp(2) < min_z )
        {
            expand_z = min_z - corner_3D_in_pp(2);
            sp_obj_centre(2) -= (expand_z/2.0);
            sp_obj_dimensions(2) += expand_z;
            min_z = sp_obj_centre(2) - sp_obj_dimensions(2)/2.0;
        }
    }    
   // std::cout << "sp_obj_centre: " << sp_obj_centre << std::endl; 
    //std::cout << "sp_obj_dimensions: " << sp_obj_dimensions << std::endl; 

    /** Now find the max size allowed for the supported parapiped, given the position
     * of the corner, the directions determined by the corner vertices, and the
     * size of the supporter */  
    double max_size_x = 0.0;
    double max_size_x_in_room = 0.0;
    if(expand_right)
    {
        max_size_x =  max_x - corner_3D_in_pp(0);
        max_size_x_in_room = pp_width/2.0 - corner_3D_in_pp(0);
    }
    else
    {
        max_size_x = fabs( min_x - corner_3D_in_pp(0) );
        max_size_x_in_room = fabs(-pp_width/2.0 - corner_3D_in_pp(0));
    }
    double max_size_y = 0.0;
    double max_size_y_in_room = 0.0;
    if(expand_up)
    {
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_pp(1);
        max_size_y_in_room = ( pp_height/2.0 ) - corner_3D_in_pp(1);
    }
    else
    {
        max_size_y = fabs( max_y  - corner_3D_in_pp(1) );
        max_size_y_in_room = fabs( (- pp_height/2.0 ) + (new_pp.get_height()*ST_PARAPIPED_DISTANCE_FROM_THE_FLOOR) + 0.01 - corner_3D_in_pp(1) );//ensure the supporting object's height is no less than the distance from the floor plus a minimum height of 0.01
    }
    double max_size_z = 0.0;
    double max_size_z_in_room = 0.0;
    if(expand_z_up)
    {
        max_size_z = max_z - corner_3D_in_pp(2);
        max_size_z_in_room = pp_length/2.0 - corner_3D_in_pp(2);
    }
    else
    {
        max_size_z = fabs( min_z - corner_3D_in_pp(2) );
        max_size_z_in_room = fabs( -pp_length/2.0 - corner_3D_in_pp(2) );
    }
    
    /** If these sizes are smaller than the minimum desired dimensions,
     *  we determine how much we have to increase the size of the container */
    if(max_size_x < desired_dimensions(0))
    {
        if(expand_sp_obj)
        {
            double temp_expand_x = desired_dimensions(0) - max_size_x;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_x_in_room < desired_dimensions(0))
            {
                temp_expand_x = max_size_x_in_room - max_size_x;
                desired_dimensions(0) = max_size_x_in_room;
            }*/
            sp_obj_dimensions(0) += temp_expand_x;
            if(expand_right)
            {
                sp_obj_centre(0) += (temp_expand_x/2.0);
            }
            else
            {
                sp_obj_centre(0) -= (temp_expand_x/2.0);
            }
        }
        else
        {
            desired_dimensions(0) = max_size_x;
            if(max_size_x < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_y < desired_dimensions(1))
    {
        if(expand_sp_obj)
        {
            double temp_expand_y = desired_dimensions(1) - max_size_y;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_y_in_room < desired_dimensions(1))
            {
                temp_expand_y = max_size_y_in_room - max_size_y;
                desired_dimensions(1) = max_size_y_in_room;
            }*/
            if(!expand_up)
            {
                sp_obj_centre(1) -= (temp_expand_y/2.0); // we lower down the supporting surface instead of raising the ceiling
                sp_obj_dimensions(1) -= temp_expand_y;
            }
            else
            {
                //sp_obj_centre(1) -= (temp_expand_x/2.0);
            }
        }
        else
        {
            desired_dimensions(1) = max_size_y;
            if(max_size_y < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_z < desired_dimensions(2))
    {
        if(expand_sp_obj)
        {
            double temp_expand_z = desired_dimensions(2) - max_size_z;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_z_in_room < desired_dimensions(2))
            {
                temp_expand_z = max_size_z_in_room - max_size_z;
                desired_dimensions(2) = max_size_z_in_room;
            }*/
            sp_obj_dimensions(2) += temp_expand_z;
            if(expand_z_up)
            {
                sp_obj_centre(2) += (temp_expand_z/2.0);
            }
            else
            {
                sp_obj_centre(2) -= (temp_expand_z/2.0);
            }
        }
        else
        {
            desired_dimensions(2) = max_size_z;
            if(max_size_z < 0.001)
            {
                return false;
            }
        }
    }
    std::cout << "sp_obj_centre: " << sp_obj_centre << std::endl; 
    std::cout << "sp_obj_dimensions: " << sp_obj_dimensions << std::endl; 

    centre(0) = corner_3D_in_pp(0);
    centre(1) = corner_3D_in_pp(1);
    centre(2) = corner_3D_in_pp(2);

    dimensions(0) = desired_dimensions(0) - 0.0001;
    dimensions(1) = desired_dimensions(1) - 0.0001;
    dimensions(2) = desired_dimensions(2) - 0.0001;


    /** We position the corner at the right place given the corner
     * position and the direction of the vertices. This is in a coordinate
     * system defined by the containing parapiped */
    if(expand_right)
    {
        centre(0) +=  dimensions(0)/2.0;
    }
    else
    {
        centre(0) -= dimensions(0)/2.0;
    }
    if(expand_up)
    {
        centre(1) += dimensions(1)/2.0;
    }
    else
    {
        /** We make sure that the inner parapiped will lie on the floor */
        //dimensions(1) = fabs( corner_3D_in_pp(1) + (pp_height/2.0 ));
        centre(1) -= dimensions(1)/2.0;
    }
    if(expand_z_up)
    {
        centre(2) += dimensions(2)/2.0;
    }
    else
    {
        centre(2) -= dimensions(2)/2.0;
    }
    
    return true;
}

bool kjb::propose_parapiped_onside_parapiped_from_one_corner_in_the_center
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_sp_obj, // if it is allowed to expand the supporting object
    kjb::Vector & sp_obj_centre, // centre of the supporting object
    kjb::Vector & sp_obj_dimensions // dimensions of the supporting object
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(imin_desired_dimensions.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Propose parapiped onside parapiped, desired_dimensions must be of size 3");
    }
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }

    double max_x = sp_obj_centre(0) + sp_obj_dimensions(0)/2.0;
    double min_x = sp_obj_centre(0) - sp_obj_dimensions(0)/2.0;
    double max_y = - new_pp.get_height()/2.0 + sp_obj_dimensions(1);// ensure the supporting object is standing on the floor
    double min_y = - new_pp.get_height()/2.0;
    //double max_y = sp_obj_centre(1) + sp_obj_dimensions(1)/2.0;
    //double min_y = sp_obj_centre(1) - sp_obj_dimensions(1)/2.0;
    double max_z = sp_obj_centre(2) + sp_obj_dimensions(2)/2.0;
    double min_z = sp_obj_centre(2) - sp_obj_dimensions(2)/2.0;

    Vector desired_dimensions = imin_desired_dimensions;

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
   
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();
    std::cout << "Corner position: " << corner_position << std::endl;

    /** These vector represent the corner directions onto the image plane */
    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);
    std::cout << "Direction1: " << direction1 << std::endl;
    std::cout << "Direction2: " << direction2 << std::endl;
    std::cout << "Direction3: " << direction3 << std::endl;
    /** Direction 3 is direction closest to "vertical", using the Manhattan_corner convention */

    /** This tells us if the corner expands towards the ceiling or
     * towards the floor. Signs are inverted with respect to what
     * we would expect, since in the image y=0 is the top, and incresing
     * y goes towards the bottom of the image*/
    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    /** We get the position in 3D of the projection of the corner vertices onto the image
     * plane, with the principal point as the origin. This is in camera coordinates */
    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0)*1;
    vertex2(0) = (corner.get_position() )(0) + direction2(0)*1;
    vertex3(0) = (corner.get_position() )(0) + direction3(0)*1;

    vertex1(1) =  (corner.get_position() )(1) + direction1(1)*1;
    vertex2(1) =  (corner.get_position() )(1) + direction2(1)*1;
    vertex3(1) =  (corner.get_position() )(1) + direction3(1)*1;

    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();

    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    /** We normalize everything */
    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    std::cout << "Vertex1: " << vertex1 << std::endl;
    std::cout << "Vertex2: " << vertex2 << std::endl;
    std::cout << "Vertex3: " << vertex3 << std::endl;
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;
    //std::cout << "Camera_to_corner: " << camera_to_corner << std::endl;

    /** We now find where the vector from the camera to the corner
     * projected onto the image plane intersects the top face of the 
     * supporting object */
    Vector sp_plane(4, 0.0);
    sp_plane(1) = 1;
    Vector x_near_plane(4, 0.0);
    x_near_plane(0) = 1;
    Vector x_far_plane(4, 0.0);
    x_far_plane(0) = 1;
    Vector z_near_plane(4, 0.0);
    z_near_plane(2) = 1;
    Vector z_far_plane(4, 0.0);
    z_far_plane(2) = 1;
    
    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width  = pp.get_width();
    
    desired_dimensions(0) *= pp_height;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_height;
    std::cout << "Desired dimensions: " << desired_dimensions<< std::endl;

    Vector intersection;
    double t_x_near = 0.0;
    double t_x_far = 0.0;
    double t_y = 0.0;
    double t_z_near = 0.0;
    double t_z_far = 0.0;
    double max_t = 0.0;
    double min_t = 0.0;
    bool found_t_near = false;
    bool found_t_far  = false;
    
    if (camera_to_corner(0) >= 0)
    {
        x_far_plane(3)  = -max_x;
        x_near_plane(3) = -min_x;
    }
    else
    {
        x_far_plane(3)  = -min_x;
        x_near_plane(3) = -max_x;
    }
    
    if (camera_to_corner(2) >= 0)
    {
        z_far_plane(3)  = -max_z;
        z_near_plane(3) = -min_z;
    }
    else
    {
        z_far_plane(3)  = -min_z;
        z_near_plane(3) = -max_z;
    }
   
    if (expand_up)
    {
        sp_plane(3) = -max_y;
    }
    else
    {
        /** We find the intersection of the line from camera to corner and the top face of the supported object */
        sp_plane(3) = -max_y - desired_dimensions(1);
        /** if the top surface of the supported object is higher than the ceiling, we either expand the room box up
         * or lower down the top face of the supporting object or return false */
    }
    if (-sp_plane(3) > pp_height/2.0)
    {
        sp_plane(3) = -pp_height/2.0;// for now we do no want to expand the room box 
    }

    //std::cout << "sp_plane: " << sp_plane << std::endl;
    //std::cout << "Cemara centre in pp: " << camera_centre_in_pp << std::endl;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, sp_plane) )
    {
        ASSERT(t_y);
        if (t_y < 0)
        {
            if (!expand_up)
            {
                sp_plane(3) = -max_y;
                if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, sp_plane) )
                {
                    ASSERT(t_y);
                    if (t_y < 0)
                    {
                        return false;
                    }
                    t_y = distance_from_camera*t_y;
                }
            }
            else
            {
                return false;
            }
        }
        max_t = t_y;
        min_t = t_y;
        found_t_near = true;
        found_t_far = true;
    }

    /** We find the intersection of the line from camera to corner and their near x-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_x_near, camera_centre_in_pp, camera_to_corner, x_near_plane) )
    {
        ASSERT(t_x_near);
        if (!found_t_near)
        {
            min_t = t_x_near;
            found_t_near = true;
        }
        else if (t_x_near > min_t)
        {
            min_t = t_x_near;
        }
    }
    
    /** We find the intersection of the line from camera to corner and thei further x-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_x_far, camera_centre_in_pp, camera_to_corner, x_far_plane) )
    {
        ASSERT(t_x_far);
        if (!found_t_far)
        {
            max_t = t_x_far;
            found_t_far = true;
        }
        else if (t_x_far < max_t)
        {
            max_t = t_x_far;
        }
    }

    /** We find the intersection of the line from camera to corner and their near z-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_z_near, camera_centre_in_pp, camera_to_corner, z_near_plane) )
    {
        ASSERT(t_z_near);
        if (!found_t_near)
        {
            min_t = t_z_near;
            found_t_near = true;
        }
        else if (t_z_near > min_t)
        {
            min_t = t_z_near;
        }
    }
    
    /** We find the intersection of the line from camera to corner and thei further z-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_z_far, camera_centre_in_pp, camera_to_corner, z_far_plane) )
    {
        ASSERT(t_z_far);
        if (!found_t_far)
        {
            max_t = t_z_far;
            found_t_far = true;
        }
        else if (t_z_far < max_t)
        {
            max_t = t_z_far;
        }
    }

    if(!found_t_near && !found_t_far)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
        return false; // TODO We probably need to expand the supporting object, but just leave it for now
    }

    /** We now use t to position the corner in space */

    double chosen_t = 0.0;
    /** This will be set to true if the supporting object is too small
    * to support a parapiped with the desired dimensions */
    bool need_to_expand = false;
    /** These variables will contain the amount of stretch necessary so
     * that the supporting object will be able to support a parapiped of the
     * desired dimensions */
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    if (t_y > max_t)
    {
        if(!expand_sp_obj)
        {
            chosen_t = (max_t+min_t)/2.0; // we put the corner of the supported parapiped in the middle of the supporting plane
        }
        else
        {
            /** We need to expand the room if we want the inner parapiped to lie
             * on the base of the parapiped that contains it */
            chosen_t = t_y;
            need_to_expand = true;
        }
    }
    else if (t_y < min_t)
    {
        if(!expand_sp_obj)
        {
            chosen_t = (max_t+min_t)/2.0; // we put the corner of the supported parapiped in the middle of the supporting plane
        }
        else
        {
            /** We need to expand the room if we want the inner parapiped to lie
             * on the base of the parapiped that contains it */
            chosen_t = t_y;
            need_to_expand = true;
        }
    }
    else
    {
        chosen_t = t_y;
    }

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + chosen_t*camera_to_corner(i);
    }
//    std::cout << "Corner_3D_in_pp: " << corner_3D_in_pp << std::endl;
    /** check if the corner is out of room box */
    if (corner_3D_in_pp(0)<-pp_width/2.0 || corner_3D_in_pp(0)>pp_width/2.0)
    {
        std::cout << "x coordinate out of room box" << std::endl;
        return false;
    }

    if (corner_3D_in_pp(2)<-pp_length/2.0 || corner_3D_in_pp(2)>pp_length/2.0)
    {
        std::cout << "z coordinate out of room box" << std::endl;
        return false;
    }
    
    /** The height of the corner position (the actual corner now,
     *  not its projections, also determines the height of the points
     *  that generated the corner vertices onto the image plane */
    double t1 = 0.0;
    if(fabs(vertex1(1)) > DBL_EPSILON)
    {
        t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    }
    else
    {
        if(vertex1(1) < 0.0)
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    if(need_to_expand)
    {
        if(!expand_sp_obj)
        {
            return false;
        }
        if( corner_3D_in_pp(0) > max_x )
        {
            expand_x = corner_3D_in_pp(0) + desired_dimensions[0]/2.0 - max_x;
            sp_obj_centre(0) += (expand_x/2.0);
            sp_obj_dimensions(0) += expand_x;
            max_x = sp_obj_centre(0) + sp_obj_dimensions(0)/2.0;
        }
        else if ( corner_3D_in_pp(0) < min_x )
        {
            expand_x = min_x - corner_3D_in_pp(0) + desired_dimensions[0]/2.0;
            sp_obj_centre(0) -= (expand_x/2.0);
            sp_obj_dimensions(0) += expand_x;
            min_x = sp_obj_centre(0) - sp_obj_dimensions(0)/2.0;
        }
        if( corner_3D_in_pp(2) > max_z )
        {
            expand_z = corner_3D_in_pp(2) + desired_dimensions[2]/2.0 - max_z;
            sp_obj_centre(2) += (expand_z/2.0);
            sp_obj_dimensions(2) += expand_z;
            max_z = sp_obj_centre(2) + sp_obj_dimensions(2)/2.0;
        }
        else if ( corner_3D_in_pp(2) < min_z )
        {
            expand_z = min_z - corner_3D_in_pp(2) + desired_dimensions[2]/2.0;
            sp_obj_centre(2) -= (expand_z/2.0);
            sp_obj_dimensions(2) += expand_z;
            min_z = sp_obj_centre(2) - sp_obj_dimensions(2)/2.0;
        }
    }    
   // std::cout << "sp_obj_centre: " << sp_obj_centre << std::endl; 
    //std::cout << "sp_obj_dimensions: " << sp_obj_dimensions << std::endl; 

    /** Now find the max size allowed for the supported parapiped, given the position
     * of the corner, the directions determined by the corner vertices, and the
     * size of the supporter */  
    double max_size_x = 0.0;
    if(max_x-corner_3D_in_pp(0)>corner_3D_in_pp(0)-min_x)
    {
        max_size_x = corner_3D_in_pp(0)-min_x;
    }
    else
    {
        max_size_x = max_x-corner_3D_in_pp(0);
    }

    double max_size_y = 0.0;
    if(expand_up)
    {
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_pp(1);
    }
    else
    {
        max_size_y = fabs( max_y  - corner_3D_in_pp(1) );
    }
    
    double max_size_z = 0.0;
    if(max_z-corner_3D_in_pp(2)>corner_3D_in_pp(2)-min_z)
    {
        max_size_z = corner_3D_in_pp(2)-min_z;
    }
    else
    {
        max_size_z = max_z-corner_3D_in_pp(2);
    }

    /** If these sizes are smaller than the minimum desired dimensions,
     *  we determine how much we have to increase the size of the container */
    if(max_size_x < desired_dimensions(0))
    {
        if(expand_sp_obj)
        {
            double temp_expand_x = desired_dimensions(0) - max_size_x;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_x_in_room < desired_dimensions(0))
            {
                temp_expand_x = max_size_x_in_room - max_size_x;
                desired_dimensions(0) = max_size_x_in_room;
            }*/
            sp_obj_dimensions(0) += temp_expand_x;
            if(max_x-corner_3D_in_pp(0)>corner_3D_in_pp(0)-min_x)
            {
                sp_obj_centre(0) += (temp_expand_x/2.0);
            }
            else
            {
                sp_obj_centre(0) -= (temp_expand_x/2.0);
            }
        }
        else
        {
            desired_dimensions(0) = max_size_x;
            if(max_size_x < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_y < desired_dimensions(1))
    {
        if(expand_sp_obj)
        {
            double temp_expand_y = desired_dimensions(1) - max_size_y;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_y_in_room < desired_dimensions(1))
            {
                temp_expand_y = max_size_y_in_room - max_size_y;
                desired_dimensions(1) = max_size_y_in_room;
            }*/
            if(!expand_up)
            {
                sp_obj_centre(1) -= (temp_expand_y/2.0); // we lower down the supporting surface instead of raising the ceiling
                sp_obj_dimensions(1) -= temp_expand_y;
            }
            else
            {
                //sp_obj_centre(1) -= (temp_expand_x/2.0);
            }
        }
        else
        {
            desired_dimensions(1) = max_size_y;
            if(max_size_y < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_z < desired_dimensions(2))
    {
        if(expand_sp_obj)
        {
            double temp_expand_z = desired_dimensions(2) - max_size_z;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_z_in_room < desired_dimensions(2))
            {
                temp_expand_z = max_size_z_in_room - max_size_z;
                desired_dimensions(2) = max_size_z_in_room;
            }*/
            sp_obj_dimensions(2) += temp_expand_z;
            if(max_z-corner_3D_in_pp(2)>corner_3D_in_pp(2)-min_z)
            {
                sp_obj_centre(2) += (temp_expand_z/2.0);
            }
            else
            {
                sp_obj_centre(2) -= (temp_expand_z/2.0);
            }
        }
        else
        {
            desired_dimensions(2) = max_size_z;
            if(max_size_z < 0.001)
            {
                return false;
            }
        }
    }
    std::cout << "sp_obj_centre: " << sp_obj_centre << std::endl; 
    std::cout << "sp_obj_dimensions: " << sp_obj_dimensions << std::endl; 

    centre(0) = corner_3D_in_pp(0);
    centre(1) = corner_3D_in_pp(1);
    centre(2) = corner_3D_in_pp(2);

    dimensions(0) = desired_dimensions(0) - 0.0001;
    dimensions(1) = desired_dimensions(1) - 0.0001;
    dimensions(2) = desired_dimensions(2) - 0.0001;


    /** We position the corner at the right place given the corner
     * position and the direction of the vertices. This is in a coordinate
     * system defined by the containing parapiped */
    if(expand_up)
    {
        centre(1) += dimensions(1)/2.0;
    }
    else
    {
        /** We make sure that the inner parapiped will lie on the floor */
        //dimensions(1) = fabs( corner_3D_in_pp(1) + (pp_height/2.0 ));
        centre(1) -= dimensions(1)/2.0;
    }
    
    return true;
}

bool kjb::propose_supported_parapiped_inside_parapiped_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_sp_obj, // if it is allowed to expand the supporting object
    kjb::Vector & sp_obj_centre, // centre of the supporting object
    kjb::Vector & sp_obj_dimensions // dimensions of the supporting object
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(imin_desired_dimensions.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Propose parapiped inside parapiped, desired_dimensions must be of size 3");
    }
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }

    double max_x = sp_obj_centre(0) + sp_obj_dimensions(0)/2.0;
    double min_x = sp_obj_centre(0) - sp_obj_dimensions(0)/2.0;
    double max_y = sp_obj_centre(1) + sp_obj_dimensions(1)/2.0;
    double min_y = sp_obj_centre(1) - sp_obj_dimensions(1)/2.0;
    double max_z = sp_obj_centre(2) + sp_obj_dimensions(2)/2.0;
    double min_z = sp_obj_centre(2) - sp_obj_dimensions(2)/2.0;

    Vector desired_dimensions = imin_desired_dimensions;

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
   
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();
    std::cout << "Corner position: " << corner_position << std::endl;

    /** These vector represent the corner directions onto the image plane */
    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);
    std::cout << "Direction1: " << direction1 << std::endl;
    std::cout << "Direction2: " << direction2 << std::endl;
    std::cout << "Direction3: " << direction3 << std::endl;
    /** Direction 3 is direction closest to "vertical", using the Manhattan_corner convention */

    /** This tells us if the corner expands towards the ceiling or
     * towards the floor. Signs are inverted with respect to what
     * we would expect, since in the image y=0 is the top, and incresing
     * y goes towards the bottom of the image*/
    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    /** We get the position in 3D of the projection of the corner vertices onto the image
     * plane, with the principal point as the origin. This is in camera coordinates */
    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0)*1;
    vertex2(0) = (corner.get_position() )(0) + direction2(0)*1;
    vertex3(0) = (corner.get_position() )(0) + direction3(0)*1;

    vertex1(1) =  (corner.get_position() )(1) + direction1(1)*1;
    vertex2(1) =  (corner.get_position() )(1) + direction2(1)*1;
    vertex3(1) =  (corner.get_position() )(1) + direction3(1)*1;

    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();

    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    /** We normalize everything */
    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    std::cout << "Vertex1: " << vertex1 << std::endl;
    std::cout << "Vertex2: " << vertex2 << std::endl;
    std::cout << "Vertex3: " << vertex3 << std::endl;
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;
    //std::cout << "Camera_to_corner: " << camera_to_corner << std::endl;

    /** We now find where the vector from the camera to the corner
     * projected onto the image plane intersects the top face of the 
     * supporting object */
    Vector sp_plane(4, 0.0);
    sp_plane(1) = 1;
    Vector x_near_plane(4, 0.0);
    x_near_plane(0) = 1;
    Vector x_far_plane(4, 0.0);
    x_far_plane(0) = 1;
    Vector z_near_plane(4, 0.0);
    z_near_plane(2) = 1;
    Vector z_far_plane(4, 0.0);
    z_far_plane(2) = 1;
    
    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width  = pp.get_width();
    
    desired_dimensions(0) *= pp_height;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_height;
    std::cout << "Desired dimensions: " << desired_dimensions<< std::endl;

    Vector intersection;
    double t_x_near = 0.0;
    double t_x_far = 0.0;
    double t_y = 0.0;
    double t_z_near = 0.0;
    double t_z_far = 0.0;
    double max_t = 0.0;
    double min_t = 0.0;
    bool found_t_near = false;
    bool found_t_far  = false;
    
    if (camera_to_corner(0) >= 0)
    {
        x_far_plane(3)  = -max_x;
        x_near_plane(3) = -min_x;
    }
    else
    {
        x_far_plane(3)  = -min_x;
        x_near_plane(3) = -max_x;
    }
    
    if (camera_to_corner(2) >= 0)
    {
        z_far_plane(3)  = -max_z;
        z_near_plane(3) = -min_z;
    }
    else
    {
        z_far_plane(3)  = -min_z;
        z_near_plane(3) = -max_z;
    }
   
    if (expand_up)
    {
        sp_plane(3) = -max_y;
    }
    else
    {
        /** We find the intersection of the line from camera to corner and the top face of the supported object */
        sp_plane(3) = -max_y - desired_dimensions(1);
        /** if the top surface of the supported object is higher than the ceiling, we either expand the room box up
         * or lower down the top face of the supporting object or return false */
    }
    if (-sp_plane(3) > pp_height/2.0)
    {
        return false; // for now we do no want to expand the room box
    }

    //std::cout << "sp_plane: " << sp_plane << std::endl;
    //std::cout << "Cemara centre in pp: " << camera_centre_in_pp << std::endl;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, sp_plane) )
    {
        ASSERT(t_y);
        if (t_y < 0)
        {
            if (!expand_up)
            {
                sp_plane(3) = -max_y;
                if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, sp_plane) )
                {
                    ASSERT(t_y);
                    if (t_y < 0)
                    {
                        return false;
                    }
                    t_y = distance_from_camera*t_y;
                }
            }
            else
            {
                return false;
            }
        }
        max_t = t_y;
        min_t = t_y;
        found_t_near = true;
        found_t_far = true;
    }

    /** We find the intersection of the line from camera to corner and thei near x-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_x_near, camera_centre_in_pp, camera_to_corner, x_near_plane) )
    {
        ASSERT(t_x_near);
        if (!found_t_near)
        {
            min_t = t_x_near;
            found_t_near = true;
        }
        else if (t_x_near > min_t)
        {
            min_t = t_x_near;
        }
    }
    
    /** We find the intersection of the line from camera to corner and thei further x-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_x_far, camera_centre_in_pp, camera_to_corner, x_far_plane) )
    {
        ASSERT(t_x_far);
        if (!found_t_far)
        {
            max_t = t_x_far;
            found_t_far = true;
        }
        else if (t_x_far < max_t)
        {
            max_t = t_x_far;
        }
    }

    /** We find the intersection of the line from camera to corner and thei near z-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_z_near, camera_centre_in_pp, camera_to_corner, z_near_plane) )
    {
        ASSERT(t_z_near);
        if (!found_t_near)
        {
            min_t = t_z_near;
            found_t_near = true;
        }
        else if (t_z_near > min_t)
        {
            min_t = t_z_near;
        }
    }
    
    /** We find the intersection of the line from camera to corner and thei further z-aligned face of the supported object */
    if(kjb::intersect_3D_line_with_plane(intersection, t_z_far, camera_centre_in_pp, camera_to_corner, z_far_plane) )
    {
        ASSERT(t_z_far);
        if (!found_t_far)
        {
            max_t = t_z_far;
            found_t_far = true;
        }
        else if (t_z_far < max_t)
        {
            max_t = t_z_far;
        }
    }

    if(!found_t_near && !found_t_far)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
        return false; // TODO We probably need to expand the supporting object, but just leave it for now
    }

    /** We now use t to position the corner in space */

    double chosen_t = 0.0;
    /** This will be set to true if the supporting object is too small
    * to support a parapiped with the desired dimensions */
    bool need_to_expand = false;
    /** These variables will contain the amount of stretch necessary so
     * that the supporting object will be able to support a parapiped of the
     * desired dimensions */
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    if (t_y > max_t)
    {
        /** We need to expand the room if we want the inner parapiped to lie
         * on the base of the parapiped that contains it */
        chosen_t = t_y;
        need_to_expand = true;
        if(!expand_sp_obj)
        {
            return false;
        }
    }
    else if (t_y < min_t)
    {
        /** We need to expand the room if we want the inner parapiped to lie
         * on the base of the parapiped that contains it */
        chosen_t = t_y;
        need_to_expand = true;
        if(!expand_sp_obj)
        {
            return false;
        }
    }
    else
    {
        chosen_t = t_y;
    }

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + chosen_t*camera_to_corner(i);
    }
//    std::cout << "Corner_3D_in_pp: " << corner_3D_in_pp << std::endl;
    /** check if the corner is out of room box */
/*
    if (corner_3D_in_pp(0)<-pp_width/2.0 || corner_3D_in_pp(0)>pp_width/2.0)
    {
        std::cout << "x coordinate out of room box" << std::endl;
        return false;
    }

    if (corner_3D_in_pp(2)<-pp_length/2.0 || corner_3D_in_pp(2)>pp_length/2.0)
    {
        std::cout << "z coordinate out of room box" << std::endl;
        return false;
    }
*/
    /** The height of the corner position (the actual corner now,
     *  not its projections, also determines the height of the points
     *  that generated the corner vertices onto the image plane */
    double t1 = 0.0;
    if(fabs(vertex1(1)) > DBL_EPSILON)
    {
        t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    }
    else
    {
        if(vertex1(1) < 0.0)
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    /** The vector (x1_edge, z1_edge) and (x2_edge,z2_edge) represent the direction of
     *  the corner vertices onto the xz plane. we will make sure that the first one
     *  expands mostly along the x axis, the second along the z axis */
    double x1_edge = camera_centre_in_pp(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_pp(2) + t1*vertex1(2);

    double t2 = 0.0;
    if(fabs(vertex2(1)) > DBL_EPSILON)
    {
        t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex2(1);
    }
    else
    {
        if(vertex3(1) < 0.0)
        {
             t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    double x2_edge = camera_centre_in_pp(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_pp(2) + t2*vertex2(2);

    x1_edge -= corner_3D_in_pp(0);
    x2_edge -= corner_3D_in_pp(0);
    z1_edge -= corner_3D_in_pp(2);
    z2_edge -= corner_3D_in_pp(2);

    double norm_1 = sqrt(x1_edge*x1_edge +z1_edge*z1_edge);
    double norm_2 = sqrt(x2_edge*x2_edge +z2_edge*z2_edge);
    x1_edge /= norm_1;
    z1_edge /= norm_1;
    x2_edge /= norm_2;
    z2_edge /= norm_2;


    /** The first edge expands mostly along the x axis,
     * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }
    
    /** get the expansion direction of proposed object */
    bool expand_right = true;
    if(x1_edge < 0.0)
    {
        expand_right = false;
    }
    bool expand_z_up = true;
    if(z2_edge < 0.0)
    {
        expand_z_up = false;
    }
    if(need_to_expand)
    {
        if(!expand_sp_obj)
        {
            return false;
        }
        if( corner_3D_in_pp(0) > max_x )
        {
            expand_x = corner_3D_in_pp(0) - max_x;
            sp_obj_centre(0) += (expand_x/2.0);
            sp_obj_dimensions(0) += expand_x;
            max_x = sp_obj_centre(0) + sp_obj_dimensions(0)/2.0;
        }
        else if ( corner_3D_in_pp(0) < min_x )
        {
            expand_x = min_x - corner_3D_in_pp(0);
            sp_obj_centre(0) -= (expand_x/2.0);
            sp_obj_dimensions(0) += expand_x;
            min_x = sp_obj_centre(0) - sp_obj_dimensions(0)/2.0;
        }
        if( corner_3D_in_pp(2) > max_z )
        {
            expand_z = corner_3D_in_pp(2) - max_z;
            sp_obj_centre(2) += (expand_z/2.0);
            sp_obj_dimensions(2) += expand_z;
            max_z = sp_obj_centre(2) + sp_obj_dimensions(2)/2.0;
        }
        else if ( corner_3D_in_pp(2) < min_z )
        {
            expand_z = min_z - corner_3D_in_pp(2);
            sp_obj_centre(2) -= (expand_z/2.0);
            sp_obj_dimensions(2) += expand_z;
            min_z = sp_obj_centre(2) - sp_obj_dimensions(2)/2.0;
        }
    }    
   // std::cout << "sp_obj_centre: " << sp_obj_centre << std::endl; 
    //std::cout << "sp_obj_dimensions: " << sp_obj_dimensions << std::endl; 

    /** Now find the max size allowed for the supported parapiped, given the position
     * of the corner, the directions determined by the corner vertices, and the
     * size of the supporter */  
    double max_size_x = 0.0;
    double max_size_x_in_room = 0.0;
    if(expand_right)
    {
        max_size_x =  max_x - corner_3D_in_pp(0);
        max_size_x_in_room = pp_width/2.0 - corner_3D_in_pp(0);
    }
    else
    {
        max_size_x = fabs( min_x - corner_3D_in_pp(0) );
        max_size_x_in_room = fabs(-pp_width/2.0 - corner_3D_in_pp(0));
    }
    double max_size_y = 0.0;
    double max_size_y_in_room = 0.0;
    if(expand_up)
    {
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_pp(1);
        max_size_y_in_room = ( pp_height/2.0 ) - corner_3D_in_pp(1);
    }
    else
    {
        max_size_y = fabs( max_y  - corner_3D_in_pp(1) );
        max_size_y_in_room = fabs( (- pp_height/2.0 ) + (new_pp.get_height()*ST_PARAPIPED_DISTANCE_FROM_THE_FLOOR) + 0.01 - corner_3D_in_pp(1) );//ensure the supporting object's height is no less than the distance from the floor plus a minimum height of 0.01
    }
    double max_size_z = 0.0;
    double max_size_z_in_room = 0.0;
    if(expand_z_up)
    {
        max_size_z = max_z - corner_3D_in_pp(2);
        max_size_z_in_room = pp_length/2.0 - corner_3D_in_pp(2);
    }
    else
    {
        max_size_z = fabs( min_z - corner_3D_in_pp(2) );
        max_size_z_in_room = fabs( -pp_length/2.0 - corner_3D_in_pp(2) );
    }
    
    /** If these sizes are smaller than the minimum desired dimensions,
     *  we determine how much we have to increase the size of the container */
    if(max_size_x < desired_dimensions(0))
    {
        if(expand_sp_obj)
        {
            double temp_expand_x = desired_dimensions(0) - max_size_x;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_x_in_room < desired_dimensions(0))
            {
                temp_expand_x = max_size_x_in_room - max_size_x;
                desired_dimensions(0) = max_size_x_in_room;
            }*/
            sp_obj_dimensions(0) += temp_expand_x;
            if(expand_right)
            {
                sp_obj_centre(0) += (temp_expand_x/2.0);
            }
            else
            {
                sp_obj_centre(0) -= (temp_expand_x/2.0);
            }
        }
        else
        {
            desired_dimensions(0) = max_size_x;
            if(max_size_x < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_y < desired_dimensions(1))
    {
        if(expand_sp_obj)
        {
            double temp_expand_y = desired_dimensions(1) - max_size_y;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_y_in_room < desired_dimensions(1))
            {
                temp_expand_y = max_size_y_in_room - max_size_y;
                desired_dimensions(1) = max_size_y_in_room;
            }*/
            if(!expand_up)
            {
                sp_obj_centre(1) -= (temp_expand_y/2.0); // we lower down the supporting surface instead of raising the ceiling
                sp_obj_dimensions(1) -= temp_expand_y;
            }
            else
            {
                //sp_obj_centre(1) -= (temp_expand_x/2.0);
            }
        }
        else
        {
            desired_dimensions(1) = max_size_y;
            if(max_size_y < 0.001)
            {
                return false;
            }
        }
    }
    if(max_size_z < desired_dimensions(2))
    {
        if(expand_sp_obj)
        {
            double temp_expand_z = desired_dimensions(2) - max_size_z;
            /* ensure the supporting object is still inside the room box */
            /*if (max_size_z_in_room < desired_dimensions(2))
            {
                temp_expand_z = max_size_z_in_room - max_size_z;
                desired_dimensions(2) = max_size_z_in_room;
            }*/
            sp_obj_dimensions(2) += temp_expand_z;
            if(expand_z_up)
            {
                sp_obj_centre(2) += (temp_expand_z/2.0);
            }
            else
            {
                sp_obj_centre(2) -= (temp_expand_z/2.0);
            }
        }
        else
        {
            desired_dimensions(2) = max_size_z;
            if(max_size_z < 0.001)
            {
                return false;
            }
        }
    }
    //std::cout << "sp_obj_centre: " << sp_obj_centre << std::endl; 
    //std::cout << "sp_obj_dimensions: " << sp_obj_dimensions << std::endl; 

    centre(0) = corner_3D_in_pp(0);
    centre(1) = corner_3D_in_pp(1);
    centre(2) = corner_3D_in_pp(2);

    dimensions(0) = desired_dimensions(0) - 0.0001;
    dimensions(1) = desired_dimensions(1) - 0.0001;
    dimensions(2) = desired_dimensions(2) - 0.0001;


    /** We position the corner at the right place given the corner
     * position and the direction of the vertices. This is in a coordinate
     * system defined by the containing parapiped */
    if(expand_right)
    {
        centre(0) +=  dimensions(0)/2.0;
    }
    else
    {
        centre(0) -= dimensions(0)/2.0;
    }
    if(expand_up)
    {
        centre(1) += dimensions(1)/2.0;
    }
    else
    {
        /** We make sure that the inner parapiped will lie on the floor */
        //dimensions(1) = fabs( corner_3D_in_pp(1) + (pp_height/2.0 ));
        centre(1) -= dimensions(1)/2.0;
    }
    if(expand_z_up)
    {
        centre(2) += dimensions(2)/2.0;
    }
    else
    {
        centre(2) -= dimensions(2)/2.0;
    }

    //TODO check if we need to expand the room box
    
    
    return true;
}

bool kjb::propose_parapiped_inside_parapiped_from_orthogonal_corner_against_wall
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & imin_desired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    double distance_from_camera,
    bool expand_room,
    bool expand_right,
    bool expand_z_up,
    bool expand_towards_camera
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(imin_desired_dimensions.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Propose parapiped inside parapiped, desired_dimensions must be of size 3");
    }
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    if(expansion_deltas.size() != 3)
    {
        expansion_deltas.resize(3, 0.0);
    }
    if(expansion_directions.size() != 3)
    {
        expansion_directions.resize(3, true);
    }

    Vector desired_dimensions = imin_desired_dimensions;

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();

    /** These vector represent the corner directions onto the image plane */
    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);
    /** Direction 3 is direction closest to "vertical", using the Manhattan_corner convention */

    /** This tells us if the corner expands towards the ceiling or
     * towards the floor. Signs are inverted with respect to what
     * we would expect, since in the image y=0 is the top, and incresing
     * y goes towards the bottom of the image*/
    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    /** We get the position in 3D of the projection of the corner vertices onto the image
     * plane, with the principal point as the origin. This is in camera coordinates */
    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0)*1;
    vertex2(0) = (corner.get_position() )(0) + direction2(0)*1;
    vertex3(0) = (corner.get_position() )(0) + direction3(0)*1;

    vertex1(1) =  (corner.get_position() )(1) + direction1(1)*1;
    vertex2(1) =  (corner.get_position() )(1) + direction2(1)*1;
    vertex3(1) =  (corner.get_position() )(1) + direction3(1)*1;

    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();

    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    /** We normalize everything */
    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;

    /** We now find where the vector from the camera to the corner
     * projected onto the image plane intersects the pp lateral faces
     * and the top face (or bottom if this vector is going down) */
    Vector x_plane(4, 0.0);
    x_plane(0) = 1.0;
    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;
    Vector z_plane(4, 0.0);
    z_plane(2) = 1.0;

    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width = pp.get_width();
    /*desired_dimensions(0) *= pp_width;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_length;*/
    desired_dimensions(0) *= pp_height;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_height;

    /** We consider the intersection with the faces only
     * in the direction determined by the vector from the
     * camera to the corner. We find the farthest intersection,
     * which is also the farthest possible position of the corner
     * such that the corner is still inside the parapiped */
    if(camera_to_corner(1) >= 0.0)
    {
        if(expand_up)
        {
            /** This will lead to a configuration where the new parapiped
             * will not lie on the base of the parapiped containing it.*/
            //TODO We might want to consider moving the floor up here
            return false;
        }

        y_plane(3) = - pp_height/2.0;

    }
    else
    {
        y_plane(3) = pp_height/2.0;
    }
    if(camera_to_corner(0) >= 0.0)
    {
        x_plane(3) = -pp_width / 2.0;
    }
    else
    {
        x_plane(3) = pp_width/2.0;
    }
    if(camera_to_corner(2) >= 0.0 )
    {
        z_plane(3) = -pp_length/2.0;
    }
    else
    {
        z_plane(3) = pp_length/2.0;
    }

    Vector intersection;
    double t_x = 0.0;
    double max_t = 0.0;
    bool found_t = false;
    if(kjb::intersect_3D_line_with_plane(intersection, t_x, camera_centre_in_pp, camera_to_corner, x_plane) )
    {
        ASSERT(t_x);
        max_t = t_x;
        found_t = true;
    }
    double t_y = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, y_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_y);
            max_t = t_y;
            found_t = true;
        }
        else if(t_y < max_t)
        {
            max_t = t_y;
        }
    }
    double t_z = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_z, camera_centre_in_pp, camera_to_corner, z_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_z);
            max_t = t_z;
            found_t = true;
        }
        else if(t_z < max_t)
        {
            max_t = t_z;
        }
    }

    if(!found_t)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
    }

    //Let us find the logic here

    /** We now use t to position the corner in space */

    double chosen_t = 0.0;
    /** This will be set to true if the parapiped is too small
    * to contain a parapiped with the desired dimensions */
    bool need_to_expand = false;
    bool need_to_move_obj_x = false;
    bool need_to_move_obj_z = false;
    /** These variables will contain the amount of stretch necessary so
     * that the parapiped will be able to contain a parapiped of the
     * desired dimensions */
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    bool expand_pp_right = true;
    bool expand_pp_up = expand_up;
    bool expand_pp_z_up = true;

    // If we are going towards the camera:
    // If the corner goes down
    // If we hit the floor first, we want it to expand it, finding
    // the closest wall

    double x_displacement = 0.0;
    double z_displacement = 0.0;
    if(expand_towards_camera)
    {
        // We put is as far as possible against the wall
        // We find the closest wall
        bool closest_x = true;
        chosen_t = t_x;
        if(t_z < t_x)
        {
            closest_x = false;
            chosen_t = t_z;
        }
        if(!closest_x)
        {
            if(expand_up)
            {
                if(t_z < t_y)
                {
                    //Better move the wall! -> this is the key if we manage to readjust
                    //We find the z of the point at t_y and expand accordingly
                    need_to_expand = true;
                    double new_z = camera_centre_in_pp(2) + t_y*camera_to_corner(2);
                    if(expand_z_up)
                    {
                        expand_pp_z_up = false;
                        expand_z =  (pp_length/2.0) + new_z;
                    }
                    else
                    {
                        expand_pp_z_up = true;
                        expand_z = new_z - (pp_length/2.0);
                    }
                    chosen_t = t_y;
                    desired_dimensions(2) += 0.02;
                    if(t_x < t_y)
                    {
                        //Better move the wall! -> this is the key, but this case should not happen that often
                        double new_x = camera_centre_in_pp(0) + t_y*camera_to_corner(0);
                        if(expand_right)
                        {
                            expand_pp_right = false;
                            expand_x = new_x + (pp_width/2.0);
                        }
                        else
                        {
                            expand_pp_right = true;
                            expand_x = new_x - (pp_width/2.0);
                        }
                        desired_dimensions(0) += 0.02;
                    }
                }
                else
                {
                    /** SEEMS OK */
                    //We check if the thing is against the wall, otherwise we stretch it or move it
                    need_to_move_obj_z = true;
                    double new_z = camera_centre_in_pp(2) + t_y*camera_to_corner(2);
                    z_displacement = -(pp_length/2.0) - new_z;
                    //The sign of the displacement is the opposite of what it would seem
                    if(!expand_z_up)
                    {
                        z_displacement = (pp_length/2.0) - new_z;
                    }
                    chosen_t = t_y;
                }
            }
            else
            {
                if(t_z < t_y)
                {
                    //This is just fine, should not do anything
                    //Possibly if this is not the desired height,
                    // we should move the wall back (only if too tall)
                }
                else
                {
                    //This means it hits the celing before the wall:
                    //We can do two things here:
                    //1) move it back against the wall (probably best)
                    //2) expand the room ceiling (this happens so rarely though)
                    //Let us return false for now
                    return false;
                }
            }
        }
        else
        {
            if(expand_up)
            {
                if(t_x < t_y)
                {
                    /**SEEMS_OK */
                    need_to_expand = true;
                    //Better move the wall! -> this is the key, but this case should not happen that often
                    double new_x = camera_centre_in_pp(0) + t_y*camera_to_corner(0);
                    if(expand_right)
                    {
                        expand_pp_right = false;
                        expand_x = new_x + (pp_width/2.0);
                    }
                    else
                    {
                        expand_pp_right = true;
                        expand_x = new_x - (pp_width/2.0);
                    }
                    chosen_t = t_y;
                    desired_dimensions(0) += 0.02;

                    if(t_z < t_y)
                    {
                        double new_z = camera_centre_in_pp(2) + t_y*camera_to_corner(2);
                        if(expand_z_up)
                        {
                            expand_pp_z_up = false;
                            expand_z =  (pp_length/2.0) + new_z;
                        }
                        else
                        {
                            expand_pp_z_up = true;
                            expand_z = new_z - (pp_length/2.0);
                        }
                        desired_dimensions(2) += 0.02;
                    }
                    //chosen_t = t_y;
                }
                else
                {
                    /** SEEMS OK */
                    //We check if the thing is against the wall, otherwise we stretch it or move it
                    need_to_move_obj_x = true;
                    double new_x = camera_centre_in_pp(0) + t_y*camera_to_corner(0);
                    x_displacement = -(pp_width/2.0) - new_x;
                    //The sign of the displacement is the opposite of what it would seem
                    if(!expand_right)
                    {
                        x_displacement = (pp_width/2.0) - new_x;
                    }
                    chosen_t = t_y;
                }
            }
            else
            {
                if(t_x < t_y)
                {
                    //This is just fine, should not do anything
                    //Possibly if this is not the desired height,
                    // we should move the wall back (only if too tall)
                }
                else
                {
                    //This means it hits the celing before the wall:
                    //We can do two things here:
                    //1) move it back against the wall (probably best)
                    //2) expand the room ceiling (this happens so rarely though)
                    //Let us return false for now
                    return false;
                }
            }
        }
    }
    else
    {
        // We put is as far as possible against the wall
        // We find the closest wall
        if(!expand_up)
        {
            double desired_t_x;
            double desired_t_z;
            Vector intersection_plane(4, 0.0);
            bool expansion_x_agree = false;
            if( ((camera_to_corner(0) > 0.0) && (expand_right)) ||
                ((camera_to_corner(0) < 0.0) && (!expand_right))    )
            {
                expansion_x_agree = true;
            }
            bool expansion_z_agree = false;
            if( ((camera_to_corner(2) > 0.0) && (expand_z_up)) ||
                ((camera_to_corner(2) < 0.0) && (!expand_z_up)) )
            {
                expansion_z_agree = true;
            }
            if(expansion_x_agree)
            {
                if(expand_right)
                {
                    double desired_x = (pp_width)/2.0 - desired_dimensions(0);
                    intersection_plane(0) = 1.0;
                    intersection_plane(3) = -desired_x;
                }
                else
                {
                    double desired_x = -(pp_width)/2.0 + desired_dimensions(0);
                    intersection_plane(0) = 1.0;
                    intersection_plane(3) = -desired_x;
                }
                if(kjb::intersect_3D_line_with_plane(intersection, desired_t_x, camera_centre_in_pp, camera_to_corner, intersection_plane) )
                {
                    if(desired_t_x < 0)
                    {
                        //return false;
                        //For now we return false, but this might be a sign that we have to expand the room
                    }
                }
                else
                {
                    return false;
                    //For now we return false, but this might be a sign that we have to expand the room
                }
                //Find the new height
            }

            if(expansion_z_agree)
            {
                if(expand_z_up)
                {
                    double desired_z = (pp_length)/2.0 - desired_dimensions(2);
                    //desired_z is pp_length/2.0 - desired_dimensions(2)
                    intersection_plane(0) = 0.0;
                    intersection_plane(2) = 1.0;
                    intersection_plane(3) = -desired_z;
                }
                else
                {
                    double desired_z = -(pp_length)/2.0 + desired_dimensions(2);
                    //desired_z is -pp_length/2.0 + desired_dimensions(2)*pp_length add it as a function of the room height
                    intersection_plane(0) = 0.0;
                    intersection_plane(2) = 1.0;
                    intersection_plane(3) = -desired_z;
                }
                if(kjb::intersect_3D_line_with_plane(intersection, desired_t_z, camera_centre_in_pp, camera_to_corner, intersection_plane) )
                {
                    if(desired_t_z < 0)
                    {
                        //return false;
                        //For now we return false, but this might be a sign that we have to expand the room
                    }
                }
                else
                {
                    return false;
                    //For now we return false, but this might be a sign that we have to expand the room
                }
            }

            double min_desired_z = true;
            double desired_t = desired_t_z;
            if(!expansion_z_agree)
            {
                if(!expansion_x_agree)
                {
                    //One has to agree!!!
                    return false;
                }
                min_desired_z = false;
                desired_t = desired_t_x;
            }
            else if(expansion_x_agree)
            {
                if( (desired_t_z < 0) && (desired_t_x < 0))
                {
                    desired_t = desired_t_x;
                    min_desired_z = false;
                }
                if(desired_t_z < 0)
                {
                    desired_t = desired_t_x;
                    min_desired_z = false;
                }
                else if(desired_t_x < 0)
                {
                    desired_t = desired_t_z;
                    min_desired_z = true;
                }
                else if(desired_t_z > desired_t_x)
                {
                    desired_t = desired_t_x;
                    min_desired_z = false;
                    //Pick the closest now, in the future, the one that aligns against the wall the dimension we want
                }
            }
            if(t_y < desired_t)
            {
                //Is this ever happening?
                if(expand_up)
                {
                    //We should not deal with this one, just return false
                    return false;
                }
                //Now we are in trouble we have to bring the foor down, but we will worry about this another day
                return false;
            }
            else
            {
                //We are good, we set chosen_t to desired_t and off we go
                chosen_t = desired_t;
                if(chosen_t < 0)
                {
                    return false;
                }
            }
        }
        else
        {
            double start_x = camera_centre_in_pp(0) + t_y*camera_to_corner(0);
            double start_z = camera_centre_in_pp(2) + t_y*camera_to_corner(2);
            double end_x = 0.0;
            double end_z = 0.0;
            if(expand_right)
            {
                end_x = start_x + desired_dimensions(0);
            }
            else
            {
                end_x = start_x - desired_dimensions(0);
            }
            if(expand_z_up)
            {
                end_z = start_z + desired_dimensions(2);
            }
            else
            {
                end_z = start_z - desired_dimensions(2);
            }
            double diff_x = fabs(end_x) - fabs(pp_width/2.0);
            double diff_z = fabs(end_z) - fabs(pp_length/2.0);
            if(diff_x > DBL_EPSILON)
            {
                need_to_expand = true;
                expand_pp_right = expand_right;
                expand_x = diff_x;
            }
            if(diff_z > DBL_EPSILON)
            {
                need_to_expand = true;
                expand_pp_z_up = expand_z_up;
                expand_z = diff_z;
            }

            if(!need_to_expand)
            {
                double move_x;
                double move_z;
                if(expand_right)
                {
                    move_x = (pp_width/2.0) - end_x;
                }
                else
                {
                    move_x = (-pp_width/2.0) - end_x;
                }
                if(expand_z_up)
                {
                    move_z = (pp_length/2.0) - end_z;
                }
                else
                {
                    move_z = (-pp_length/2.0) - end_z;
                }
                if(fabs(move_x) < fabs(move_z))
                {
                    //Translate
                    need_to_move_obj_x = true;
                    x_displacement = move_x;
                }
                if(fabs(move_z) < fabs(move_x))
                {
                    //Translate
                    need_to_move_obj_z = true;
                    z_displacement = move_z;
                }
            }
            chosen_t = t_y;
        }
    }

    /** Find new height */
    if(!expand_up)
    {
        //double top_y =  camera_centre_in_pp(1) + chosen_t*camera_to_corner(1);
        //desired_dimensions(1) = top_y + (pp_height/2.0);
    }

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + chosen_t*camera_to_corner(i);
    }


    /** The height of the corner position (the actual corner now,
     *  not its projections, also determines the height of the points
     *  that generated the corner vertices onto the image plane */
    double t1 = 0.0;
    if(fabs(vertex1(1)) > DBL_EPSILON)
    {
        t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    }
    else
    {
        if(vertex1(1) < 0.0)
        {
             t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    /** The vector (x1_edge, z1_edge) and (x2_edge,z2_edge) represent the direction of
     *  the corner vertices onto the xz plane. we will make sure that the first one
     *  expands mostly along the x axis, the second along the z axis */
    double x1_edge = camera_centre_in_pp(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_pp(2) + t1*vertex1(2);

    double t2 = 0.0;
    if(fabs(vertex2(1)) > DBL_EPSILON)
    {
        t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex2(1);
    }
    else
    {
        if(vertex3(1) < 0.0)
        {
             t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    double x2_edge = camera_centre_in_pp(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_pp(2) + t2*vertex2(2);

    x1_edge -= corner_3D_in_pp(0);
    x2_edge -= corner_3D_in_pp(0);
    z1_edge -= corner_3D_in_pp(2);
    z2_edge -= corner_3D_in_pp(2);

    double norm_1 = sqrt(x1_edge*x1_edge +z1_edge*z1_edge);
    double norm_2 = sqrt(x2_edge*x2_edge +z2_edge*z2_edge);
    x1_edge /= norm_1;
    z1_edge /= norm_1;
    x2_edge /= norm_2;
    z2_edge /= norm_2;


    /** The first edge expands mostly along the x axis,
         * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }

    /** Now find the max size allowed for the inner parapiped, given the position
     * of the corner, the directions determined by the corner vertices, and the
     * size of the container */
    double max_size_x = 0.0;
    if(expand_right)
    {
        max_size_x = ( pp_width/2.0 ) - corner_3D_in_pp(0);
    }
    else
    {
        max_size_x = fabs( ( -pp_width/2.0 ) - corner_3D_in_pp(0) );
    }
    double max_size_y = 0.0;
    if(expand_up)
    {
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_pp(1);
    }
    else
    {
        max_size_y = fabs( ( -pp_height/2.0 ) - corner_3D_in_pp(1) );
    }
    double max_size_z = 0.0;
    if(expand_z_up)
    {
        max_size_z = ( pp_length/2.0 ) - corner_3D_in_pp(2);
    }
    else
    {
        max_size_z = fabs( ( -pp_length/2.0 ) - corner_3D_in_pp(2) );
    }

    /** If these sizes are smaller than the minimum desired dimensions,
     *  we determine how much we have to increase the size of the container */

    centre(0) = corner_3D_in_pp(0);
    centre(1) = corner_3D_in_pp(1);
    centre(2) = corner_3D_in_pp(2);

    dimensions(0) = desired_dimensions(0) - 0.0001;
    dimensions(1) = desired_dimensions(1) - 0.0001;
    dimensions(2) = desired_dimensions(2) - 0.0001;


    /** We position the corner at the right place given the corner
     * position and the direction of the vertices. This is in a coordinate
     * system defined by the containing parapiped */
    if(expand_right)
    {
        centre(0) +=  dimensions(0)/2.0;
    }
    else
    {
        centre(0) -= dimensions(0)/2.0;
    }
    if(expand_up)
    {
        centre(1) += dimensions(1)/2.0;
    }
    else
    {
        /** We make sure that the inner parapiped will lie on the floor */
        //dimensions(1) = fabs( corner_3D_in_pp(1) + (pp_height/2.0 ));
        centre(1) -= dimensions(1)/2.0;
    }
    if(expand_z_up)
    {
        centre(2) += dimensions(2)/2.0;
    }
    else
    {
        centre(2) -= dimensions(2)/2.0;
    }

    /** We expand the containing parapiped so that it will contain the inner one */
    if(need_to_expand)
    {
        expansion_deltas(0) = expand_x;
        expansion_deltas(1) = expand_y;
        expansion_deltas(2) = expand_z;
        expansion_directions[0] = expand_pp_right;
        expansion_directions[1] = expand_pp_up;
        expansion_directions[2] = expand_pp_z_up;
        if(expand_x > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(0, expand_x, expand_pp_right);
            if(expand_pp_right)
            {
                centre(0) -= expand_x/2.0;
            }
            else
            {
                centre(0) += expand_x/2.0;
            }
        }
        if(expand_y > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(1, expand_y, expand_pp_up);
        }
        if(expand_z > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(2, expand_z, expand_pp_z_up);
            if(expand_pp_z_up)
            {
                centre(2) -= expand_z/2.0;
            }
            else
            {
                centre(2) += expand_z/2.0;
            }
        }
    }

    if(need_to_move_obj_x)
    {
        centre(0) += x_displacement;
    }
    if(need_to_move_obj_z)
    {
        centre(2) += z_displacement;
    }
    return true;

}

void kjb::expand_towards_camera
(
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    unsigned int num_rows,
    unsigned int num_cols,
    bool & expand_tw_camera,
    bool & expand_right,
    bool & expand_z_up
)
{
    kjb::Vector centre(4, 1.0);
    kjb::Vector dimensions(3, 0.0);
    Parametric_parapiped new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();

    /** These vector represent the corner directions onto the image plane */
    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);
    /** Direction 3 is direction closest to "vertical", using the Manhattan_corner convention */

    /** This tells us if the corner expands towards the ceiling or
     * towards the floor. Signs are inverted with respect to what
     * we would expect, since in the image y=0 is the top, and incresing
     * y goes towards the bottom of the image*/
    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    /** We get the position in 3D of the projection of the corner vertices onto the image
     * plane, with the principal point as the origin. This is in camera coordinates */
    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0)*1;
    vertex2(0) = (corner.get_position() )(0) + direction2(0)*1;
    vertex3(0) = (corner.get_position() )(0) + direction3(0)*1;

    vertex1(1) =  (corner.get_position() )(1) + direction1(1)*1;
    vertex2(1) =  (corner.get_position() )(1) + direction2(1)*1;
    vertex3(1) =  (corner.get_position() )(1) + direction3(1)*1;

    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();

    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    /** We normalize everything */

    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + 0.5*camera_to_corner(i);
    }

    /** The height of the corner position (the actual corner now,
     *  not its projections, also determines the height of the points
     *  that generated the corner vertices onto the image plane */
    double t1 = 0.0;
    if(fabs(vertex1(1)) > DBL_EPSILON)
    {
        t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    }
    else
    {
        if(vertex1(1) < 0.0)
        {
             t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    /** The vector (x1_edge, z1_edge) and (x2_edge,z2_edge) represent the direction of
     *  the corner vertices onto the xz plane. we will make sure that the first one
     *  expands mostly along the x axis, the second along the z axis */
    double x1_edge = camera_centre_in_pp(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_pp(2) + t1*vertex1(2);

    double t2 = 0.0;
    if(fabs(vertex2(1)) > DBL_EPSILON)
    {
        t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex2(1);
    }
    else
    {
        if(vertex3(1) < 0.0)
        {
             t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (-DBL_EPSILON);
        }
        else
        {
            t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) / (DBL_EPSILON);
        }
    }

    double x2_edge = camera_centre_in_pp(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_pp(2) + t2*vertex2(2);

    x1_edge -= corner_3D_in_pp(0);
    x2_edge -= corner_3D_in_pp(0);
    z1_edge -= corner_3D_in_pp(2);
    z2_edge -= corner_3D_in_pp(2);

    double norm_1 = sqrt(x1_edge*x1_edge +z1_edge*z1_edge);
    double norm_2 = sqrt(x2_edge*x2_edge +z2_edge*z2_edge);
    x1_edge /= norm_1;
    z1_edge /= norm_1;
    x2_edge /= norm_2;
    z2_edge /= norm_2;


    /** The first edge expands mostly along the x axis,
         * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }

    /** In case we need to expand the container, we determine in which direction */
    expand_right = true;
    if(x1_edge < 0.0)
    {
        expand_right = false;
    }
    expand_z_up = true;
    if(z2_edge < 0.0)
    {
        expand_z_up = false;
    }

    Vector average1(2, 0.0);
    Vector average2(2, 0.0);
    if(expand_right)
    {
        average1(0) = 0.707106781 + corner_3D_in_pp(0);
        average2(0) = -0.707106781 + corner_3D_in_pp(0);
    }
    else
    {
        average1(0) = -0.707106781 + corner_3D_in_pp(0);
        average2(0) = 0.707106781 + corner_3D_in_pp(0);
    }
    if(expand_z_up)
    {
        average1(1) = 0.707106781 + corner_3D_in_pp(2);
        average2(1) = -0.707106781 + corner_3D_in_pp(2);
    }
    else
    {
        average1(1) = -0.707106781 + corner_3D_in_pp(2);
        average2(1) = 0.707106781 + corner_3D_in_pp(2);

    }


    double dist1 = (average1(0) - camera_centre_in_pp(0))*(average1(0) - camera_centre_in_pp(0)) +
                   (average1(1) - camera_centre_in_pp(2))*(average1(1) - camera_centre_in_pp(2));
    double dist2 = (average2(0) - camera_centre_in_pp(0))*(average2(0) - camera_centre_in_pp(0)) +
                   (average2(1) - camera_centre_in_pp(2))*(average2(1) - camera_centre_in_pp(2));
    if(dist1 < dist2)
    {
        expand_tw_camera = true;
    }
    else
    {
        expand_tw_camera = false;
    }
}


bool kjb::propose_frame_inside_parapiped_from_orthogonal_corner_good
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    unsigned int & face_number,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & idesired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols,
    bool wall_x,
    bool expand_to_the_ground
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();
    if(idesired_dimensions.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Propose parapiped inside parapiped, desired_dimensions must be of size 3");
    }
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    if(expansion_deltas.size() != 3)
    {
        expansion_deltas.resize(3, 0.0);
    }
    if(expansion_directions.size() != 3)
    {
        expansion_directions.resize(3, true);
    }

    Vector desired_dimensions = idesired_dimensions;
    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width = pp.get_width();
    desired_dimensions(0) *= pp_height;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_height;

    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();

    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);


    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0);
    vertex2(0) = (corner.get_position() )(0) + direction2(0);
    vertex3(0) = (corner.get_position() )(0) + direction3(0);
    vertex1(1) =  (corner.get_position() )(1) + direction1(1);
    vertex2(1) =  (corner.get_position() )(1) + direction2(1);
    vertex3(1) =  (corner.get_position() )(1) + direction3(1);
    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();
    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);
    Vector camera_centre = camera.get_camera_centre();

    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;

    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;

    Vector x_plane(4, 0.0);
    x_plane(0) = 1.0;
    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;
    Vector z_plane(4, 0.0);
    z_plane(2) = 1.0;

    if(camera_to_corner(1) >= 0.0)
    {
        y_plane(3) = - pp_height/2.0;

    }
    else
    {
        if(!expand_up)
        {
            //TODO Expand the floor here
            return false;
        }
        y_plane(3) = pp_height/2.0;
    }
    if(camera_to_corner(0) >= 0.0)
    {
        x_plane(3) = -pp_width / 2.0;
    }
    else
    {
        x_plane(3) = pp_width/2.0;
    }
    if(camera_to_corner(2) >= 0.0 )
    {
        z_plane(3) = -pp_length/2.0;
    }
    else
    {
        z_plane(3) = pp_length/2.0;
    }

    Vector intersection;
    double t_x = 0.0;
    double max_t;
    bool found_t = false;
    if(kjb::intersect_3D_line_with_plane(intersection, t_x, camera_centre_in_pp, camera_to_corner, x_plane) )
    {
        ASSERT(t_x);
        max_t = t_x;
        found_t = true;
    }
    double t_y = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, y_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_y);
            max_t = t_y;
            found_t = true;
        }
        else if(t_y < max_t)
        {
            max_t = t_y;
        }
    }
    double t_z = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_z, camera_centre_in_pp, camera_to_corner, z_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_z);
            max_t = t_z;
            found_t = true;
        }
        else if(t_z < max_t)
        {
            max_t = t_z;
        }
    }

    if(!found_t)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
    }

    double chosen_t = 0.0;
    bool need_to_expand = false;
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    bool expand_room_right = true;
    bool expand_room_up = true;
    bool expand_room_z_up = true;
    bool frame_x = true;
    if(wall_x)
    {
        chosen_t = t_z;
    }
    else
    {
        frame_x = false;
        chosen_t = t_x;
    }
    if(chosen_t > t_y)
    {
        need_to_expand = true;
    }

    /** Now find the face number the frame is attached to
     *        0
     *    ---------
     *    |       |
     *  3 |       | 1
     *    |_______|
     *        2
     */
    if(wall_x)
    {
        if(camera_to_corner(2) >= 0.0)
        {
            face_number = 2;
        }
        else
        {
            face_number = 0;
        }
    }
    else
    {
        if(camera_to_corner(0) >= 0.0)
        {
            face_number = 1;
        }
        else
        {
            face_number = 3;
        }
    }

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + chosen_t*camera_to_corner(i);
    }

    if(corner_3D_in_pp(0) < 0.0)
    {
        expand_room_right = false;
    }
    if(corner_3D_in_pp(1) < 0.0)
    {
        expand_room_up = false;
    }
    if(corner_3D_in_pp(2) < 0.0)
    {
        expand_room_z_up = false;
    }

    double t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    /** The first edge expands mostly along the x axis,
     * the second along the z axis */
    double x1_edge = camera_centre_in_pp(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_pp(2) + t1*vertex1(2);

    double t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex2(1);
    double x2_edge = camera_centre_in_pp(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_pp(2) + t2*vertex2(2);

    x1_edge -= corner_3D_in_pp(0);
    x2_edge -= corner_3D_in_pp(0);

    z1_edge -= corner_3D_in_pp(2);
    z2_edge -= corner_3D_in_pp(2);

    double norm1 = sqrt((x1_edge*x1_edge) + (z1_edge*z1_edge));
    double norm2 = sqrt((x2_edge*x2_edge) + (z2_edge*z2_edge));
    x1_edge /= norm1;
    z1_edge /= norm1;
    x2_edge /= norm2;
    z2_edge /= norm2;

    /** The first edge expands mostly along the x axis,
         * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }

    bool expand_right = true;
    if(x1_edge < 0.0)
    {
        expand_right = false;
    }
    bool expand_z_up = true;
    if(z2_edge < 0.0)
    {
        expand_z_up = false;
    }

    if(need_to_expand)
    {
        if( fabs(corner_3D_in_pp(1)) > (pp_height/2.0) )
        {
            expand_y = fabs(corner_3D_in_pp(1)) - (pp_height/2.0) ;
            if(!expand_room_up)
            {
                corner_3D_in_pp(1) += (expand_y/2.0);
            }
            else
            {
                corner_3D_in_pp(1) -= (expand_y/2.0);
            }
            pp_height += expand_y;
        }
    }


    /** Now find the max size allowed */
    double max_size_x = 0.0;
    if(frame_x)
    {
        if(expand_right)
        {
            max_size_x = ( pp_width/2.0 ) - corner_3D_in_pp(0);
        }
        else
        {
            max_size_x = fabs( ( -pp_width/2.0 ) - corner_3D_in_pp(0) );
         }
    }
    double max_size_y = 0.0;
    if(expand_up)
    {
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_pp(1);
    }
    else
    {
        max_size_y = fabs( ( -pp_height/2.0 ) - corner_3D_in_pp(1) );
    }
    double max_size_z = 0.0;
    if(!frame_x)
    {
        if(expand_z_up)
        {
            max_size_z = ( pp_length/2.0 ) - corner_3D_in_pp(2);
        }
        else
        {
            max_size_z = fabs( ( -pp_length/2.0 ) - corner_3D_in_pp(2) );
        }
    }

    if(frame_x)
    {
        if(max_size_x < desired_dimensions(0))
        {
            double temp_expand_x = desired_dimensions(0) - max_size_x;
            pp_width += temp_expand_x;
            max_size_x = desired_dimensions(0);
            if(!expand_right)
            {
                corner_3D_in_pp(0) += (temp_expand_x/2.0);
            }
            else
            {
                corner_3D_in_pp(0) -= (temp_expand_x/2.0);
            }

            if(expand_right == expand_room_right)
            {
                expand_x += temp_expand_x;
            }
            else
            {
                expand_x -= temp_expand_x;
                if(expand_x < 0.0)
                {
                    expand_x = -expand_x;
                    expand_room_right = !expand_room_right;
                }
            }
            need_to_expand = true;
        }
    }
    if(max_size_y < desired_dimensions(1))
    {
        double temp_expand_y = desired_dimensions(1) - max_size_y;
        max_size_y = desired_dimensions(1);
        pp_height += temp_expand_y;
        if(!expand_up)
        {
            corner_3D_in_pp(1) += (temp_expand_y/2.0);
        }
        else
        {
            corner_3D_in_pp(1) -= (temp_expand_y/2.0);
        }

        if(expand_up == expand_room_up)
        {
            expand_y += temp_expand_y;
        }
        else
        {
            expand_y -= temp_expand_y;
            if(expand_y < 0.0)
            {
                expand_y = -expand_y;
                expand_room_up = !expand_room_up;
            }
        }
        need_to_expand = true;
    }
    if(!frame_x)
    {
        if(max_size_z < desired_dimensions(2))
        {
            double temp_expand_z = desired_dimensions(2) - max_size_z;
            pp_length += temp_expand_z;
            max_size_z = desired_dimensions(2);
            if(!expand_z_up)
            {
                corner_3D_in_pp(2) += (temp_expand_z/2.0);
            }
            else
            {
                corner_3D_in_pp(2) -= (temp_expand_z/2.0);
            }
            if(expand_z_up == expand_room_z_up)
            {
                expand_z += temp_expand_z;
            }
            else
            {
                expand_z -= temp_expand_z;
                if(expand_z < 0.0)
                {
                    expand_z = -expand_z;
                    expand_room_z_up = !expand_room_z_up;
                }
            }
            need_to_expand = true;
        }
    }

    centre(0) = corner_3D_in_pp(0);
    centre(1) = corner_3D_in_pp(1);
    centre(2) = corner_3D_in_pp(2);

    dimensions(0) = desired_dimensions(0);
    dimensions(1) = desired_dimensions(1);
    dimensions(2) = desired_dimensions(2);

    if( (!expand_up) && (expand_to_the_ground))
    {
        double needed_stretch = fabs( (centre(1) - (dimensions(1)/2.0)) + (pp_height/2.0));
        if(needed_stretch > DBL_EPSILON)
        {
            dimensions(1) += needed_stretch;
        }
    }
    if(frame_x)
    {
        dimensions(2)  = FRAME_SIZE_AS_PP_HEIGHT_RATIO*pp.get_height();
        if(centre(2) > 0.0)
        {
            centre(2) += 0.05;
        }
        else
        {
            centre(2) -= 0.05;
        }
    }
    else
    {
        dimensions(0)  = FRAME_SIZE_AS_PP_HEIGHT_RATIO*pp.get_height();;
        if(centre(0) > 0.0)
        {
            centre(0) += 0.05;
        }
        else
        {
            centre(0) -= 0.05;
        }
    }
    if(expand_right)
    {
        centre(0) +=  dimensions(0) /2.0;
    }
    else
    {
        centre(0) -= dimensions(0) /2.0;
    }
    if(expand_up)
    {
        centre(1) += dimensions(1)/2.0;
    }
    else
    {
        centre(1) -= dimensions(1) /2.0;
    }
    if(expand_z_up)
    {
        centre(2) += dimensions(2) /2.0;
    }
    else
    {
        centre(2) -= dimensions(2) /2.0;
    }

    if(need_to_expand)
    {
        expansion_deltas(0) = expand_x;
        expansion_deltas(1) = expand_y;
        expansion_deltas(2) = expand_z;
        expansion_directions[0] = expand_room_right;
        expansion_directions[1] = expand_room_up;
        expansion_directions[2] = expand_room_z_up;
        if( (expand_x > DBL_EPSILON) && frame_x)
        {
            new_pp.stretch_along_axis(0, expand_x, expand_room_right);
        }
        if(expand_y > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(1, expand_y, expand_room_up);
        }
        if((expand_z > DBL_EPSILON) && !frame_x)
        {
            new_pp.stretch_along_axis(2, expand_z, expand_room_z_up);
        }
    }

    return true;
}


bool kjb::propose_frame_inside_parapiped_from_orthogonal_corner
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    unsigned int & face_number,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    const kjb::Vector & idesired_dimensions,
    unsigned int num_rows,
    unsigned int num_cols
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();
    if(idesired_dimensions.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Propose parapiped inside parapiped, desired_dimensions must be of size 3");
    }
    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    if(expansion_deltas.size() != 3)
    {
        expansion_deltas.resize(3, 0.0);
    }
    if(expansion_directions.size() != 3)
    {
        expansion_directions.resize(3, true);
    }

    Vector desired_dimensions = idesired_dimensions;

    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length();

    Vector direction1;
    Vector direction2;
    Vector direction3;
    corner.get_direction(0, direction1);
    corner.get_direction(1, direction2);
    corner.get_direction(2, direction3);


    bool expand_up = true;
    if(direction3(1) > 0.0)
    {
        expand_up = false;
    }

    Vector vertex1(4,1.0);
    Vector vertex2(4,1.0);
    Vector vertex3(4,1.0);

    vertex1(0) = (corner.get_position() )(0) + direction1(0);
    vertex2(0) = (corner.get_position() )(0) + direction2(0);
    vertex3(0) = (corner.get_position() )(0) + direction3(0);
    vertex1(1) =  (corner.get_position() )(1) + direction1(1);
    vertex2(1) =  (corner.get_position() )(1) + direction2(1);
    vertex3(1) =  (corner.get_position() )(1) + direction3(1);
    vertex1(2) = - camera.get_focal_length();
    vertex2(2) = - camera.get_focal_length();
    vertex3(2) = - camera.get_focal_length();
    vertex1(0) -= princ_x;
    vertex2(0) -= princ_x;
    vertex3(0) -= princ_x;
    vertex1(1) = - (vertex1(1) - princ_y);
    vertex2(1) = - (vertex2(1) - princ_y);
    vertex3(1) = - (vertex3(1) - princ_y);
    Vector camera_centre = camera.get_camera_centre();

    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    camera.get_point_in_world_coordinates(vertex1, vertex1);
    camera.get_point_in_world_coordinates(vertex2, vertex2);
    camera.get_point_in_world_coordinates(vertex3, vertex3);

    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;

    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(vertex1, vertex1);
    new_pp.get_point_in_parapiped_coordinates(vertex2, vertex2);
    new_pp.get_point_in_parapiped_coordinates(vertex3, vertex3);

    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
        vertex1(i) -=  camera_centre_in_pp(i);
        vertex2(i) -=  camera_centre_in_pp(i);
        vertex3(i) -=  camera_centre_in_pp(i);
    }

    vertex1(3) = 0.0;
    vertex2(3) = 0.0;
    vertex3(3) = 0.0;
    vertex1 = vertex1.normalize();
    vertex2 = vertex2.normalize();
    vertex3 = vertex3.normalize();
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;

    Vector x_plane(4, 0.0);
    x_plane(0) = 1.0;
    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;
    Vector z_plane(4, 0.0);
    z_plane(2) = 1.0;
    double pp_height = pp.get_height();
    double pp_length = pp.get_length();
    double pp_width = pp.get_width();

    desired_dimensions(0) *= pp_width;
    desired_dimensions(1) *= pp_height;
    desired_dimensions(2) *= pp_length;

    if(camera_to_corner(1) >= 0.0)
    {
        y_plane(3) = - pp_height/2.0;

    }
    else
    {
        if(!expand_up)
        {
            //TODO Expand the floor here
            return false;
        }
        y_plane(3) = pp_height/2.0;
    }
    if(camera_to_corner(0) >= 0.0)
    {
        x_plane(3) = -pp_width / 2.0;
    }
    else
    {
        x_plane(3) = pp_width/2.0;
    }
    if(camera_to_corner(2) >= 0.0 )
    {
        z_plane(3) = -pp_length/2.0;
    }
    else
    {
        z_plane(3) = pp_length/2.0;
    }

    Vector intersection;
    double t_x = 0.0;
    double max_t;
    bool found_t = false;
    if(kjb::intersect_3D_line_with_plane(intersection, t_x, camera_centre_in_pp, camera_to_corner, x_plane) )
    {
        ASSERT(t_x);
        max_t = t_x;
        found_t = true;
    }
    double t_y = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, y_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_y);
            max_t = t_y;
            found_t = true;
        }
        else if(t_y < max_t)
        {
            max_t = t_y;
        }
    }
    double t_z = 0.0;
    if(kjb::intersect_3D_line_with_plane(intersection, t_z, camera_centre_in_pp, camera_to_corner, z_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_z);
            max_t = t_z;
            found_t = true;
        }
        else if(t_z < max_t)
        {
            max_t = t_z;
        }
    }

    if(!found_t)
    {
        KJB_THROW_2(KJB_error,"Could not find intersection of vector with room planes");
    }

    double chosen_t = 0.0;
    bool need_to_expand = false;
    double expand_x = 0.0;
    double expand_y = 0.0;
    double expand_z = 0.0;
    bool expand_room_right = true;
    bool expand_room_up = true;
    bool expand_room_z_up = true;
    bool frame_x = true;
    if( (t_x > t_y) && (t_z > t_y))
    {
        chosen_t = t_x;
        if(t_z < t_x)
        {
            chosen_t = t_z;
        }
        else
        {
            frame_x = false;
        }
        need_to_expand = true;
    }
    else
    {
        if(t_z < t_x)
        {
            chosen_t = t_z;
        }
        else
        {
            chosen_t = t_x;
            frame_x = false;
        }
    }

    /** Now find the face number the frame is attached to
     *        0
     *    ---------
     *    |       |
     *  3 |       | 1
     *    |_______|
     *        2
     */
    if(t_z < t_x)
    {
        if(camera_to_corner(2) >= 0.0)
        {
            face_number = 2;
        }
        else
        {
            face_number = 0;
        }
    }
    else
    {
        if(camera_to_corner(0) >= 0.0)
        {
            face_number = 1;
        }
        else
        {
            face_number = 3;
        }
    }

    Vector corner_3D_in_pp(4, 1.0);
    for(unsigned int i = 0; i < 3; i++)
    {
        corner_3D_in_pp(i) = camera_centre_in_pp(i) + chosen_t*camera_to_corner(i);
    }

    if(corner_3D_in_pp(0) < 0.0)
    {
        expand_room_right = false;
    }
    if(corner_3D_in_pp(1) < 0.0)
    {
        expand_room_up = false;
    }
    if(corner_3D_in_pp(2) < 0.0)
    {
        expand_room_z_up = false;
    }

    double t1 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex1(1);
    /** The first edge expands mostly along the x axis,
     * the second along the z axis */
    double x1_edge = camera_centre_in_pp(0) + t1*vertex1(0);
    double z1_edge = camera_centre_in_pp(2) + t1*vertex1(2);

    double t2 = ( corner_3D_in_pp(1) - camera_centre_in_pp(1) ) /vertex2(1);
    double x2_edge = camera_centre_in_pp(0) + t2*vertex2(0);
    double z2_edge = camera_centre_in_pp(2) + t2*vertex2(2);

    x1_edge -= corner_3D_in_pp(0);
    x2_edge -= corner_3D_in_pp(0);

    z1_edge -= corner_3D_in_pp(2);
    z2_edge -= corner_3D_in_pp(2);

    double norm1 = sqrt((x1_edge*x1_edge) + (z1_edge*z1_edge));
    double norm2 = sqrt((x2_edge*x2_edge) + (z2_edge*z2_edge));
    x1_edge /= norm1;
    z1_edge /= norm1;
    x2_edge /= norm2;
    z2_edge /= norm2;

    /** The first edge expands mostly along the x axis,
         * the second along the z axis */
    if(fabs(x2_edge) > fabs(x1_edge))
    {
        double temp = x1_edge;
        x1_edge = x2_edge;
        x2_edge = temp;
        temp = z1_edge;
        z1_edge = z2_edge;
        z2_edge = temp;
    }

    bool expand_right = true;
    if(x1_edge < 0.0)
    {
        expand_right = false;
    }
    bool expand_z_up = true;
    if(z2_edge < 0.0)
    {
        expand_z_up = false;
    }

    if(need_to_expand)
    {
        if( fabs(corner_3D_in_pp(1)) > (pp_height/2.0) )
        {
            expand_y = fabs(corner_3D_in_pp(1)) - (pp_height/2.0) ;
            if(!expand_room_up)
            {
                corner_3D_in_pp(1) += (expand_y/2.0);
            }
            else
            {
                corner_3D_in_pp(1) -= (expand_y/2.0);
            }
            pp_height += expand_y;
        }
    }



    /** Now find the max size allowed */
    double max_size_x = 0.0;
    if(frame_x)
    {
        if(expand_right)
        {
            max_size_x = ( pp_width/2.0 ) - corner_3D_in_pp(0);
        }
        else
        {
            max_size_x = fabs( ( -pp_width/2.0 ) - corner_3D_in_pp(0) );
         }
    }
    double max_size_y = 0.0;
    if(expand_up)
    {
        max_size_y = ( pp_height/2.0 ) - corner_3D_in_pp(1);
    }
    else
    {
        max_size_y = fabs( ( -pp_height/2.0 ) - corner_3D_in_pp(1) );
    }
    double max_size_z = 0.0;
    if(!frame_x)
    {
        if(expand_z_up)
        {
            max_size_z = ( pp_length/2.0 ) - corner_3D_in_pp(2);
        }
        else
        {
            max_size_z = fabs( ( -pp_length/2.0 ) - corner_3D_in_pp(2) );
        }
    }

    if(frame_x)
    {
    if(max_size_x < desired_dimensions(0))
    {
           double temp_expand_x = desired_dimensions(0) - max_size_x;
           pp_width += temp_expand_x;
           max_size_x = desired_dimensions(0);
           if(!expand_right)
           {
               corner_3D_in_pp(0) += (temp_expand_x/2.0);
           }
           else
           {
               corner_3D_in_pp(0) -= (temp_expand_x/2.0);
           }

           if(expand_right == expand_room_right)
           {
               expand_x += temp_expand_x;
            }
            else
            {
                expand_x -= temp_expand_x;
                if(expand_x < 0.0)
                {
                    expand_x = -expand_x;
                    expand_room_right = !expand_room_right;
                }
            }
            need_to_expand = true;
        }
    }
    if(max_size_y < desired_dimensions(1))
    {
        double temp_expand_y = desired_dimensions(1) - max_size_y;
        max_size_y = desired_dimensions(1);
        pp_height += temp_expand_y;
        if(!expand_up)
        {
            corner_3D_in_pp(1) += (temp_expand_y/2.0);
        }
        else
        {
            corner_3D_in_pp(1) -= (temp_expand_y/2.0);
        }

        if(expand_up == expand_room_up)
        {
            expand_y += temp_expand_y;
        }
        else
        {
            expand_y -= temp_expand_y;
            if(expand_y < 0.0)
            {
                expand_y = -expand_y;
                expand_room_up = !expand_room_up;
            }
        }
        need_to_expand = true;
    }
    if(!frame_x)
    {
        if(max_size_z < desired_dimensions(2))
        {
            double temp_expand_z = desired_dimensions(2) - max_size_z;
            pp_length += temp_expand_z;
            max_size_z = desired_dimensions(2);
            if(!expand_z_up)
            {
                corner_3D_in_pp(2) += (temp_expand_z/2.0);
            }
            else
            {
                corner_3D_in_pp(2) -= (temp_expand_z/2.0);
            }
            if(expand_z_up == expand_room_z_up)
            {
                expand_z += temp_expand_z;
            }
            else
            {
                expand_z -= temp_expand_z;
                if(expand_z < 0.0)
                {
                    expand_z = -expand_z;
                    expand_room_z_up = !expand_room_z_up;
                }
            }
            need_to_expand = true;
        }
    }

    centre(0) = corner_3D_in_pp(0);
    centre(1) = corner_3D_in_pp(1);
    centre(2) = corner_3D_in_pp(2);

    dimensions(0) = desired_dimensions(0);
    dimensions(1) = desired_dimensions(1);
    dimensions(2) = desired_dimensions(2);
    if(frame_x)
    {
        dimensions(2)  = 0.5;
        if(centre(2) > 0.0)
        {
            centre(2) += 0.1;
        }
        else
        {
            centre(2) -= 0.1;
        }
    }
    else
    {
        dimensions(0)  = 0.5;
        if(centre(0) > 0.0)
        {
            centre(0) += 0.1;
        }
        else
        {
            centre(0) -= 0.1;
        }
    }
    if(expand_right)
    {
        centre(0) +=  dimensions(0) /2.0;
    }
    else
    {
        centre(0) -= dimensions(0) /2.0;
    }
    if(expand_up)
    {
        centre(1) += dimensions(1)/2.0;
    }
    else
    {
        centre(1) -= dimensions(1) /2.0;
    }
    if(expand_z_up)
    {
        centre(2) += dimensions(2) /2.0;
    }
    else
    {
        centre(2) -= dimensions(2) /2.0;
    }

    if(need_to_expand)
    {
        expansion_deltas(0) = expand_x;
        expansion_deltas(1) = expand_y;
        expansion_deltas(2) = expand_z;
        expansion_directions[0] = expand_room_right;
        expansion_directions[1] = expand_room_up;
        expansion_directions[2] = expand_room_z_up;
        if( (expand_x > DBL_EPSILON) && frame_x)
        {
            new_pp.stretch_along_axis(0, expand_x, expand_room_right);
        }
        if(expand_y > DBL_EPSILON)
        {
            new_pp.stretch_along_axis(1, expand_y, expand_room_up);
        }
        if((expand_z > DBL_EPSILON) && !frame_x)
        {
            new_pp.stretch_along_axis(2, expand_z, expand_room_z_up);
        }
    }

    return true;
}


bool kjb::propose_supported_parapiped_inside_parapiped_from_three_corners
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner1,
    const Manhattan_corner & corner2,
    const Manhattan_corner & corner3,
    double desired_height,
    unsigned int num_rows,
    unsigned int num_cols,
    double base_height
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    if(expansion_deltas.size() != 3)
    {
        expansion_deltas.resize(3, 0.0);
    }
    if(expansion_directions.size() != 3)
    {
        expansion_directions.resize(3, true);
    }

    Vector desired_dimensions(3, 0.0);

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector corner_position1(4,1.0);
    corner_position1(0) = (corner1.get_position() )(0) - princ_x;
    corner_position1(1) = - ((corner1.get_position() )(1) - princ_y);
    corner_position1(2) = - camera.get_focal_length();

    Vector corner_position2(4,1.0);
    corner_position2(0) = (corner2.get_position() )(0) - princ_x;
    corner_position2(1) = - ((corner2.get_position() )(1) - princ_y);
    corner_position2(2) = - camera.get_focal_length();

    Vector corner_position3(4,1.0);
    corner_position3(0) = (corner3.get_position() )(0) - princ_x;
    corner_position3(1) = - ((corner3.get_position() )(1) - princ_y);
    corner_position3(2) = - camera.get_focal_length();

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position1, corner_position1);
    camera.get_point_in_world_coordinates(corner_position2, corner_position2);
    camera.get_point_in_world_coordinates(corner_position3, corner_position3);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp1;
    Vector corner_position_in_pp2;
    Vector corner_position_in_pp3;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position1, corner_position_in_pp1);
    new_pp.get_point_in_parapiped_coordinates(corner_position2, corner_position_in_pp2);
    new_pp.get_point_in_parapiped_coordinates(corner_position3, corner_position_in_pp3);

    //std::cout << "I got evtg in pp coordinates" << std::endl;

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner1(4,0.0);
    Vector camera_to_corner2(4,0.0);
    Vector camera_to_corner3(4,0.0);
    double norm1 = 0.0;
    double norm2 = 0.0;
    double norm3 = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner1(i) = corner_position_in_pp1(i) - camera_centre_in_pp(i);
        camera_to_corner2(i) = corner_position_in_pp2(i) - camera_centre_in_pp(i);
        camera_to_corner3(i) = corner_position_in_pp3(i) - camera_centre_in_pp(i);
        norm1 += camera_to_corner1(i)*camera_to_corner1(i);
        norm2 += camera_to_corner2(i)*camera_to_corner2(i);
        norm3 += camera_to_corner3(i)*camera_to_corner3(i);
    }

    /** We normalize everything */
    norm1 = sqrt(norm1);
    norm2 = sqrt(norm2);
    norm3 = sqrt(norm3);
    camera_to_corner1 /= norm1;
    camera_to_corner2 /= norm2;
    camera_to_corner3 /= norm3;
    camera_to_corner1(3) = 1.0;
    camera_to_corner2(3) = 1.0;
    camera_to_corner3(3) = 1.0;

    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;

    desired_height *= pp.get_height();

    //std::cout << "I normalized evtg" << std::endl;
    /** We consider the intersection with the faces only
     * in the direction determined by the vector from the
     * camera to the corner. We find the farthest intersection,
     * which is also the farthest possible position of the corner
     * such that the corner is still inside the parapiped */
    if( (camera_to_corner1(1) >= 0.0) || (camera_to_corner2(1) >= 0.0) || (camera_to_corner3(1) >= 0.0))
    {
        return false;
    }
    y_plane(3) = -base_height;
    //We should worry about intercepting the ceiling

    Vector floor_intersection1;
    Vector floor_intersection2;
    Vector floor_intersection3;
    double t_y1, t_y2, t_y3;
    //std::cout << "Check intersections" << std::endl;
    if(!kjb::intersect_3D_line_with_plane(floor_intersection1, t_y1, camera_centre_in_pp, camera_to_corner1, y_plane) )
    {
    	return false;
    }
    if(!kjb::intersect_3D_line_with_plane(floor_intersection2, t_y2, camera_centre_in_pp, camera_to_corner2, y_plane) )
    {
    	return false;
    }
    if(!kjb::intersect_3D_line_with_plane(floor_intersection3, t_y3, camera_centre_in_pp, camera_to_corner3, y_plane) )
    {
    	return false;
    }

    //std::cout << "Checked intersections" << std::endl;
    Line_segment ls12;
    Line_segment ls13;
    Line_segment ls23;
    ls12.init_from_end_points(floor_intersection1(0),floor_intersection1(2),floor_intersection2(0),floor_intersection2(2));
    ls13.init_from_end_points(floor_intersection1(0),floor_intersection1(2),floor_intersection3(0),floor_intersection3(2));
    ls23.init_from_end_points(floor_intersection2(0),floor_intersection2(2),floor_intersection3(0),floor_intersection3(2));
    Vector dir12, dir13, dir23;
    ls12.get_direction(dir12);
    ls13.get_direction(dir13);
    ls23.get_direction(dir23);
    double dp12_13 = kjb::dot(dir12, dir13);
    double dp12_23 = kjb::dot(dir12, dir23);
    double dp13_23 = kjb::dot(dir13, dir23);
    int middle_position = 1;
    /*std::cout << "Floor ints 1 :" << floor_intersection1(0) << "  " << floor_intersection1(2) << std::endl;
    std::cout << "Floor ints 2 :" << floor_intersection2(0) << "  " << floor_intersection2(2) << std::endl;
    std::cout << "Floor ints 3 :" << floor_intersection3(0) << "  " << floor_intersection3(2) << std::endl;
    std::cout << "LS12:  " << ls12.get_dx() << "  " << ls12.get_dy() << std::endl;
    std::cout << "LS13:  " << ls13.get_dx() << "  " << ls13.get_dy() << std::endl;*/

    if(dp12_23 < dp12_13)
    {
        if(dp12_23 < dp13_23)
        {
    	    middle_position = 2;
        }
        else
        {
        	middle_position = 3;
        }
    }
    else if(dp13_23 < dp12_13 )
    {
    	middle_position = 3;
    }

	Line_segment lsx;
	Line_segment lsz;
	Vector fints2_2D(2, 0.0);
	Vector fints3_2D(2, 0.0);
	Vector projx, projz;
    bool x_segment_12 = false;
    bool x_segment_13 = false;
    //std::cout << "MIDDLE POSITION:" << middle_position << std::endl;
    Vector middle_positionv;
    if(middle_position == 1)
    {
    	lsx.init_from_end_points(floor_intersection1(0), floor_intersection1(2),floor_intersection1(0)+10.0,floor_intersection1(2));
    	lsz.init_from_end_points(floor_intersection1(0), floor_intersection1(2),floor_intersection1(0),floor_intersection1(2)+10.0);
    	fints2_2D(0) = floor_intersection2(0);
    	fints2_2D(1) = floor_intersection2(2);
    	fints3_2D(0) = floor_intersection3(0);
    	fints3_2D(1) = floor_intersection3(2);
        if(fabs(ls12.get_dx()) > fabs(ls12.get_dy()))
        {
        	 x_segment_12 = true;
        }
        if(fabs(ls13.get_dx()) > fabs(ls13.get_dy()))
        {
        	 x_segment_13 = true;
        }
        middle_positionv = floor_intersection1;
    }
    else if(middle_position == 2)
    {
    	lsx.init_from_end_points(floor_intersection2(0), floor_intersection2(2),floor_intersection2(0)+10.0,floor_intersection2(2));
    	lsz.init_from_end_points(floor_intersection2(0), floor_intersection2(2),floor_intersection2(0),floor_intersection2(2)+10.0);
    	fints2_2D(0) = floor_intersection1(0);
    	fints2_2D(1) = floor_intersection1(2);
    	fints3_2D(0) = floor_intersection3(0);
    	fints3_2D(1) = floor_intersection3(2);
        if(fabs(ls12.get_dx()) > fabs(ls12.get_dy()))
        {
        	 x_segment_12 = true;
        }
        if(fabs(ls23.get_dx()) > fabs(ls23.get_dy()))
        {
        	 x_segment_13 = true;
        }
        middle_positionv = floor_intersection2;
    }
    else
    {
    	lsx.init_from_end_points(floor_intersection3(0), floor_intersection3(2),floor_intersection3(0)+10.0,floor_intersection3(2));
    	lsz.init_from_end_points(floor_intersection3(0), floor_intersection3(2),floor_intersection3(0),floor_intersection3(2)+10.0);
    	fints2_2D(0) = floor_intersection1(0);
    	fints2_2D(1) = floor_intersection1(2);
    	fints3_2D(0) = floor_intersection2(0);
    	fints3_2D(1) = floor_intersection2(2);
        if(fabs(ls13.get_dx()) > fabs(ls13.get_dy()))
        {
        	 x_segment_12 = true;
        }
        if(fabs(ls23.get_dx()) > fabs(ls23.get_dy()))
        {
        	 x_segment_13 = true;
        }
        middle_positionv = floor_intersection3;
    }

    if(x_segment_13 == x_segment_12)
    {
    	return false;
    }

    if(x_segment_12)
    {
    	//std::cout << "One" << std::endl;
    	projx = lsx.get_line().project_point_onto_line(fints2_2D);
    	projz = lsz.get_line().project_point_onto_line(fints3_2D);

    }
    else
    {
    	//std::cout << "Two" << std::endl;
    	projx = lsx.get_line().project_point_onto_line(fints3_2D);
    	projz = lsz.get_line().project_point_onto_line(fints2_2D);
    }
    //std::cout << projx << std::endl;
    //std::cout << projz << std::endl;
	centre(0) = (middle_positionv(0) + projx(0))/2.0;
	centre(1) = (-pp.get_height()/2.0) + (desired_height/2.0);
	centre(2) = (middle_positionv(2) + projz(1))/2.0;
	dimensions(0) = fabs(middle_positionv(0) - projx(0));
	dimensions(1) = desired_height;
	dimensions(2) = fabs(middle_positionv(2) - projz(1));
	//Now we have the centre and the dimensions, we'll let the room be taken care of outside

    //std::cout << "Done!!" << std::endl;
    return true;

}

bool kjb::propose_parapiped_inside_parapiped_from_three_corners_on_the_floor
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner1,
    const Manhattan_corner & corner2,
    const Manhattan_corner & corner3,
    double desired_height,
    unsigned int num_rows,
    unsigned int num_cols
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    if(expansion_deltas.size() != 3)
    {
        expansion_deltas.resize(3, 0.0);
    }
    if(expansion_directions.size() != 3)
    {
        expansion_directions.resize(3, true);
    }

    Vector desired_dimensions(3, 0.0);

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector corner_position1(4,1.0);
    corner_position1(0) = (corner1.get_position() )(0) - princ_x;
    corner_position1(1) = - ((corner1.get_position() )(1) - princ_y);
    corner_position1(2) = - camera.get_focal_length();

    Vector corner_position2(4,1.0);
    corner_position2(0) = (corner2.get_position() )(0) - princ_x;
    corner_position2(1) = - ((corner2.get_position() )(1) - princ_y);
    corner_position2(2) = - camera.get_focal_length();

    Vector corner_position3(4,1.0);
    corner_position3(0) = (corner3.get_position() )(0) - princ_x;
    corner_position3(1) = - ((corner3.get_position() )(1) - princ_y);
    corner_position3(2) = - camera.get_focal_length();

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position1, corner_position1);
    camera.get_point_in_world_coordinates(corner_position2, corner_position2);
    camera.get_point_in_world_coordinates(corner_position3, corner_position3);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp1;
    Vector corner_position_in_pp2;
    Vector corner_position_in_pp3;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position1, corner_position_in_pp1);
    new_pp.get_point_in_parapiped_coordinates(corner_position2, corner_position_in_pp2);
    new_pp.get_point_in_parapiped_coordinates(corner_position3, corner_position_in_pp3);

    //std::cout << "I got evtg in pp coordinates" << std::endl;

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner1(4,0.0);
    Vector camera_to_corner2(4,0.0);
    Vector camera_to_corner3(4,0.0);
    double norm1 = 0.0;
    double norm2 = 0.0;
    double norm3 = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner1(i) = corner_position_in_pp1(i) - camera_centre_in_pp(i);
        camera_to_corner2(i) = corner_position_in_pp2(i) - camera_centre_in_pp(i);
        camera_to_corner3(i) = corner_position_in_pp3(i) - camera_centre_in_pp(i);
        norm1 += camera_to_corner1(i)*camera_to_corner1(i);
        norm2 += camera_to_corner2(i)*camera_to_corner2(i);
        norm3 += camera_to_corner3(i)*camera_to_corner3(i);
    }

    /** We normalize everything */
    norm1 = sqrt(norm1);
    norm2 = sqrt(norm2);
    norm3 = sqrt(norm3);
    camera_to_corner1 /= norm1;
    camera_to_corner2 /= norm2;
    camera_to_corner3 /= norm3;
    camera_to_corner1(3) = 1.0;
    camera_to_corner2(3) = 1.0;
    camera_to_corner3(3) = 1.0;

    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;

    desired_height *= pp.get_height();

    //std::cout << "I normalized evtg" << std::endl;
    /** We consider the intersection with the faces only
     * in the direction determined by the vector from the
     * camera to the corner. We find the farthest intersection,
     * which is also the farthest possible position of the corner
     * such that the corner is still inside the parapiped */
    if( (camera_to_corner1(1) >= 0.0) || (camera_to_corner2(1) >= 0.0) || (camera_to_corner3(1) >= 0.0))
    {
        return false;
    }
    y_plane(3) = pp.get_height()/2.0;
    //We should worry about intercepting the ceiling

    Vector floor_intersection1;
    Vector floor_intersection2;
    Vector floor_intersection3;
    double t_y1, t_y2, t_y3;
    //std::cout << "Check intersections" << std::endl;
    if(!kjb::intersect_3D_line_with_plane(floor_intersection1, t_y1, camera_centre_in_pp, camera_to_corner1, y_plane) )
    {
    	return false;
    }
    if(!kjb::intersect_3D_line_with_plane(floor_intersection2, t_y2, camera_centre_in_pp, camera_to_corner2, y_plane) )
    {
    	return false;
    }
    if(!kjb::intersect_3D_line_with_plane(floor_intersection3, t_y3, camera_centre_in_pp, camera_to_corner3, y_plane) )
    {
    	return false;
    }

    //std::cout << "Checked intersections" << std::endl;
    Line_segment ls12;
    Line_segment ls13;
    Line_segment ls23;
    ls12.init_from_end_points(floor_intersection1(0),floor_intersection1(2),floor_intersection2(0),floor_intersection2(2));
    ls13.init_from_end_points(floor_intersection1(0),floor_intersection1(2),floor_intersection3(0),floor_intersection3(2));
    ls23.init_from_end_points(floor_intersection2(0),floor_intersection2(2),floor_intersection3(0),floor_intersection3(2));
    Vector dir12, dir13, dir23;
    ls12.get_direction(dir12);
    ls13.get_direction(dir13);
    ls23.get_direction(dir23);
    double dp12_13 = kjb::dot(dir12, dir13);
    double dp12_23 = kjb::dot(dir12, dir23);
    double dp13_23 = kjb::dot(dir13, dir23);
    int middle_position = 1;
    /*std::cout << "Floor ints 1 :" << floor_intersection1(0) << "  " << floor_intersection1(2) << std::endl;
    std::cout << "Floor ints 2 :" << floor_intersection2(0) << "  " << floor_intersection2(2) << std::endl;
    std::cout << "Floor ints 3 :" << floor_intersection3(0) << "  " << floor_intersection3(2) << std::endl;
    std::cout << "LS12:  " << ls12.get_dx() << "  " << ls12.get_dy() << std::endl;
    std::cout << "LS13:  " << ls13.get_dx() << "  " << ls13.get_dy() << std::endl;*/

    if(dp12_23 < dp12_13)
    {
        if(dp12_23 < dp13_23)
        {
    	    middle_position = 2;
        }
        else
        {
        	middle_position = 3;
        }
    }
    else if(dp13_23 < dp12_13 )
    {
    	middle_position = 3;
    }

	Line_segment lsx;
	Line_segment lsz;
	Vector fints2_2D(2, 0.0);
	Vector fints3_2D(2, 0.0);
	Vector projx, projz;
    bool x_segment_12 = false;
    bool x_segment_13 = false;
    //std::cout << "MIDDLE POSITION:" << middle_position << std::endl;
    Vector middle_positionv;
    if(middle_position == 1)
    {
    	lsx.init_from_end_points(floor_intersection1(0), floor_intersection1(2),floor_intersection1(0)+10.0,floor_intersection1(2));
    	lsz.init_from_end_points(floor_intersection1(0), floor_intersection1(2),floor_intersection1(0),floor_intersection1(2)+10.0);
    	fints2_2D(0) = floor_intersection2(0);
    	fints2_2D(1) = floor_intersection2(2);
    	fints3_2D(0) = floor_intersection3(0);
    	fints3_2D(1) = floor_intersection3(2);
        if(fabs(ls12.get_dx()) > fabs(ls12.get_dy()))
        {
        	 x_segment_12 = true;
        }
        if(fabs(ls13.get_dx()) > fabs(ls13.get_dy()))
        {
        	 x_segment_13 = true;
        }
        middle_positionv = floor_intersection1;
    }
    else if(middle_position == 2)
    {
    	lsx.init_from_end_points(floor_intersection2(0), floor_intersection2(2),floor_intersection2(0)+10.0,floor_intersection2(2));
    	lsz.init_from_end_points(floor_intersection2(0), floor_intersection2(2),floor_intersection2(0),floor_intersection2(2)+10.0);
    	fints2_2D(0) = floor_intersection1(0);
    	fints2_2D(1) = floor_intersection1(2);
    	fints3_2D(0) = floor_intersection3(0);
    	fints3_2D(1) = floor_intersection3(2);
        if(fabs(ls12.get_dx()) > fabs(ls12.get_dy()))
        {
        	 x_segment_12 = true;
        }
        if(fabs(ls23.get_dx()) > fabs(ls23.get_dy()))
        {
        	 x_segment_13 = true;
        }
        middle_positionv = floor_intersection2;
    }
    else
    {
    	lsx.init_from_end_points(floor_intersection3(0), floor_intersection3(2),floor_intersection3(0)+10.0,floor_intersection3(2));
    	lsz.init_from_end_points(floor_intersection3(0), floor_intersection3(2),floor_intersection3(0),floor_intersection3(2)+10.0);
    	fints2_2D(0) = floor_intersection1(0);
    	fints2_2D(1) = floor_intersection1(2);
    	fints3_2D(0) = floor_intersection2(0);
    	fints3_2D(1) = floor_intersection2(2);
        if(fabs(ls13.get_dx()) > fabs(ls13.get_dy()))
        {
        	 x_segment_12 = true;
        }
        if(fabs(ls23.get_dx()) > fabs(ls23.get_dy()))
        {
        	 x_segment_13 = true;
        }
        middle_positionv = floor_intersection3;
    }

    if(x_segment_13 == x_segment_12)
    {
    	return false;
    }

    if(x_segment_12)
    {
    	//std::cout << "One" << std::endl;
    	projx = lsx.get_line().project_point_onto_line(fints2_2D);
    	projz = lsz.get_line().project_point_onto_line(fints3_2D);

    }
    else
    {
    	//std::cout << "Two" << std::endl;
    	projx = lsx.get_line().project_point_onto_line(fints3_2D);
    	projz = lsz.get_line().project_point_onto_line(fints2_2D);
    }
    //std::cout << projx << std::endl;
    //std::cout << projz << std::endl;
	centre(0) = (middle_positionv(0) + projx(0))/2.0;
	centre(1) = (-pp.get_height()/2.0) + (desired_height/2.0);
	centre(2) = (middle_positionv(2) + projz(1))/2.0;
	dimensions(0) = fabs(middle_positionv(0) - projx(0));
	dimensions(1) = desired_height;
	dimensions(2) = fabs(middle_positionv(2) - projz(1));
	//Now we have the centre and the dimensions, we'll let the room be taken care of outside

    //std::cout << "Done!!" << std::endl;
    return true;

}

bool kjb::propose_parapiped_inside_parapiped_from_two_corners_on_the_floor
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner1,
    const Manhattan_corner & corner2,
    double desired_height,
    double desired_other_dimension_ratio,
    bool direction,
    bool & is_diagonal,
    bool & is_dx,
    unsigned int num_rows,
    unsigned int num_cols
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    if(expansion_deltas.size() != 3)
    {
        expansion_deltas.resize(3, 0.0);
    }
    if(expansion_directions.size() != 3)
    {
        expansion_directions.resize(3, true);
    }

    Vector desired_dimensions(3, 0.0);

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector corner_position1(4,1.0);
    corner_position1(0) = (corner1.get_position() )(0) - princ_x;
    corner_position1(1) = - ((corner1.get_position() )(1) - princ_y);
    corner_position1(2) = - camera.get_focal_length();

    Vector corner_position2(4,1.0);
    corner_position2(0) = (corner2.get_position() )(0) - princ_x;
    corner_position2(1) = - ((corner2.get_position() )(1) - princ_y);
    corner_position2(2) = - camera.get_focal_length();

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position1, corner_position1);
    camera.get_point_in_world_coordinates(corner_position2, corner_position2);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp1;
    Vector corner_position_in_pp2;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position1, corner_position_in_pp1);
    new_pp.get_point_in_parapiped_coordinates(corner_position2, corner_position_in_pp2);

    //std::cout << "I got evtg in pp coordinates" << std::endl;

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner1(4,0.0);
    Vector camera_to_corner2(4,0.0);
    double norm1 = 0.0;
    double norm2 = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner1(i) = corner_position_in_pp1(i) - camera_centre_in_pp(i);
        camera_to_corner2(i) = corner_position_in_pp2(i) - camera_centre_in_pp(i);
        norm1 += camera_to_corner1(i)*camera_to_corner1(i);
        norm2 += camera_to_corner2(i)*camera_to_corner2(i);
    }

    /** We normalize everything */
    norm1 = sqrt(norm1);
    norm2 = sqrt(norm2);
    camera_to_corner1 /= norm1;
    camera_to_corner2 /= norm2;
    camera_to_corner1(3) = 1.0;
    camera_to_corner2(3) = 1.0;

    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;

    desired_height *= pp.get_height();

    //std::cout << "I normalized evtg" << std::endl;
    /** We consider the intersection with the faces only
     * in the direction determined by the vector from the
     * camera to the corner. We find the farthest intersection,
     * which is also the farthest possible position of the corner
     * such that the corner is still inside the parapiped */
    if( (camera_to_corner1(1) >= 0.0) || (camera_to_corner2(1) >= 0.0))
    {
        return false;
    }
    y_plane(3) = pp.get_height()/2.0;
    //We should worry about intercepting the ceiling

    Vector floor_intersection1;
    Vector floor_intersection2;
    double t_y1, t_y2;
    //std::cout << "Check intersections" << std::endl;
    if(!kjb::intersect_3D_line_with_plane(floor_intersection1, t_y1, camera_centre_in_pp, camera_to_corner1, y_plane) )
    {
    	return false;
    }
    if(!kjb::intersect_3D_line_with_plane(floor_intersection2, t_y2, camera_centre_in_pp, camera_to_corner2, y_plane) )
    {
    	return false;
    }

    //std::cout << "Checked intersections" << std::endl;
    Line_segment ls12;
    ls12.init_from_end_points(floor_intersection1(0),floor_intersection1(2),floor_intersection2(0),floor_intersection2(2));

    Vector fints2_2D(2, 0.0);
    fints2_2D(0) =  floor_intersection2(0);
    fints2_2D(1) =  floor_intersection2(2);
	Line_segment lsx;
	Line_segment lsz;
  	lsx.init_from_end_points(floor_intersection1(0), floor_intersection1(2),floor_intersection1(0)+10.0,floor_intersection1(2));
    lsz.init_from_end_points(floor_intersection1(0), floor_intersection1(2),floor_intersection1(0),floor_intersection1(2)+10.0);
    double seg_length = 0.0; //Z
    double seg_width = 0.0;  //X

    double diagonal_angle = ls12.get_angle_between_line(lsx);
    is_diagonal = false;
    is_dx = true;
    /*std::cout << "DIAG ANGLE: " <<  diagonal_angle << std::endl;
    std::cout << "UNO:" << floor_intersection1 << std::endl;
    std::cout << "DUE:" << floor_intersection2 << std::endl;*/
    if( (fabs(diagonal_angle) > 0.349065) && (fabs(diagonal_angle) < 1.22173 ))
    {
    	//If the angle of the segment is between 30 and 60 degees
    	is_diagonal = true;
    	centre(0) = (floor_intersection1(0) + floor_intersection2(0))/2.0;
    	centre(2) = (floor_intersection1(2) + floor_intersection2(2))/2.0;
    	seg_width = fabs(floor_intersection1(0) - floor_intersection2(0));
    	seg_length = fabs(floor_intersection1(2) - floor_intersection2(2));
    }
    else if(fabs(ls12.get_dx()) > fabs(ls12.get_dy()))
    {
		is_dx = true;
    	Vector proj2 = lsx.get_line().project_point_onto_line(fints2_2D);
    	seg_width = fabs(proj2(0) - floor_intersection1(0));
    	seg_length = seg_width*desired_other_dimension_ratio;
    	centre(0) = (floor_intersection1(0) + proj2(0))/2.0;
    	if(direction)
    	{
    	    centre(2) = floor_intersection1(2) + (seg_length/2.0);
    	}
    	else
    	{
    	    centre(2) = floor_intersection1(2) - (seg_length/2.0);
    	}
    }
    else
    {
    	is_dx = false;
		Vector proj2 = lsz.get_line().project_point_onto_line(fints2_2D);
		seg_length = fabs(proj2(1) - floor_intersection1(2));
    	seg_width = seg_length*desired_other_dimension_ratio;
       	centre(2) = (floor_intersection1(2) + proj2(1))/2.0;
    	if(direction)
    	{
    	    centre(0) = floor_intersection1(0) + (seg_width/2.0);
    	}
    	else
    	{
    	    centre(0) = floor_intersection1(0) - (seg_width/2.0);
    	}
	}

	centre(1) = (-pp.get_height()/2.0) + (desired_height/2.0);
	dimensions(0) = seg_width;
	dimensions(1) = desired_height;
	dimensions(2) = seg_length;
	//Now we have the centre and the dimensions, we'll let the room be taken care of outside

    //std::cout << "Done!!" << std::endl;
    return true;

}

bool kjb::propose_parapiped_inside_parapiped_from_one_corner_in_the_centre_on_the_floor
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    kjb::Vector & expansion_deltas,
    std::vector<bool> & expansion_directions,
    Parametric_parapiped & new_pp,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & corner,
    double desired_height,
    double desired_width,
	double desired_length,
	bool & is_dx,
    unsigned int num_rows,
    unsigned int num_cols
)
{
    new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = num_cols/2.0;
    double princ_y = num_rows/2.0;

    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    if(expansion_deltas.size() != 3)
    {
        expansion_deltas.resize(3, 0.0);
    }
    if(expansion_directions.size() != 3)
    {
        expansion_directions.resize(3, true);
    }

    Vector desired_dimensions(3, 0.0);

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector corner_position(4,1.0);
    corner_position(0) = (corner.get_position() )(0) - princ_x;
    corner_position(1) = - ((corner.get_position() )(1) - princ_y);
    corner_position(2) = - camera.get_focal_length(); // -z axis

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position, corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp); // room coordinates, rotated world coordinate
    new_pp.get_point_in_parapiped_coordinates(corner_position, corner_position_in_pp);

    //std::cout << "I got evtg in pp coordinates" << std::endl;

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner(4,0.0);
    double norm = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm += camera_to_corner(i)*camera_to_corner(i);
    }

    /** We normalize everything */
    norm = sqrt(norm);
    camera_to_corner /= norm;
    camera_to_corner(3) = 1.0;

    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;

    desired_height *= pp.get_height();
    desired_width *= pp.get_height();
    desired_length *= pp.get_height();
		
    //std::cout << "I normalized evtg" << std::endl;
    /** We consider the intersection with the faces only
     * in the direction determined by the vector from the
     * camera to the corner. We find the farthest intersection,
     * which is also the farthest possible position of the corner
     * such that the corner is still inside the parapiped */
    if (camera_to_corner(1) >= 0.0)
    {
        return false;
    }
    y_plane(3) = pp.get_height()/2.0;
    //We should worry about intercepting the ceiling

    Vector floor_intersection;
    double t_y;
    //std::cout << "Check intersections" << std::endl;
    if(!kjb::intersect_3D_line_with_plane(floor_intersection, t_y, camera_centre_in_pp, camera_to_corner, y_plane) )
    {
    	return false;
    }

	centre(1) = (-pp.get_height()/2.0) + (desired_height/2.0);
	centre(0) = floor_intersection(0);
    centre(2) = floor_intersection(2);
	if (is_dx)
	{
		dimensions(0) = desired_width;
		dimensions(2) = desired_length;
	}
	else
	{
		dimensions(0) = desired_length;
		dimensions(2) = desired_width;
	}

	dimensions(1) = desired_height;
    return true;
}

double kjb::propose_parapiped_and_camera_from_vanishing_points
(
    kjb::Parametric_parapiped & pp,
    kjb::Perspective_camera & camera,
    const std::vector<Vanishing_point> & vpts,
    double focal_length,
    unsigned int num_rows,
    unsigned int num_cols
)
{
    kjb::Vector corner1(4, 0.0);
    kjb::Vector corner2(4, 0.0);
    kjb::Vector corner3(4, 0.0);
    kjb::Vector position_3D;

    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(vpts[2].is_at_infinity())
    {
    	corner2(1) = 1.0;
    }
    else
    {
    	corner2(0) = vpts[2].get_x() - princ_x;
    	corner2(1) = -(vpts[2].get_y() - princ_y);
    	corner2(2) = focal_length;
    	corner2 = corner2.normalize();
    }

    if(vpts[0].is_at_infinity())
    {
    	if(vpts[1].is_at_infinity())
    	{
    		KJB_THROW_2(KJB_error, "Cannot have too horizontal vanishing points at infinity");
    	}
    	if(vpts[1].get_x() > 0)
    	{
    		corner1(0) = -1.0;
    	}
    	else
    	{
    		corner1(0) = 1.0;
    	}
    }
    else
    {
    	corner1(0) = vpts[0].get_x() - princ_x;
    	corner1(1) = -(vpts[0].get_y() - princ_y);
    	corner1(2) = focal_length;
    	corner1 = corner1.normalize();
    }

    if(vpts[1].is_at_infinity())
    {
    	if(vpts[0].is_at_infinity())
    	{
    		KJB_THROW_2(KJB_error, "Cannot have too horizontal vanishing points at infinity");
    	}
    	if(vpts[0].get_x() > 0)
    	{
    		corner3(0) = -1.0;
    	}
    	else
    	{
    		corner3(0) = 1.0;
    	}
    }
    else
    {
    	corner3(0) = vpts[1].get_x() - princ_x;
    	corner3(1) = -(vpts[1].get_y() - princ_y);
    	corner3(2) = focal_length;
    	corner3 = corner3.normalize();
    }

    if(vpts[2].is_at_infinity())
    {
    	Vector temp1(3, 0.0);
    	Vector temp2(3, 0.0);
    	for(int kk = 0; kk < 3; kk++)
    	{
    		temp1(kk) = corner1(kk);
    		temp2(kk) = corner3(kk);
    	}
    	Vector temp3 = cross(temp1, temp2).normalize();
    	for(int kk = 0; kk < 3; kk++)
    	{
    		corner2(kk) = temp3(kk);
    	}
    }

    if(vpts[0].is_at_infinity())
    {
    	Vector temp1(3, 0.0);
    	Vector temp2(3, 0.0);
    	for(int kk = 0; kk < 3; kk++)
    	{
    		temp1(kk) = corner3(kk);
    		temp2(kk) = corner2(kk);
    	}
    	Vector temp3 = cross(temp1, temp2).normalize();
    	for(int kk = 0; kk < 3; kk++)
    	{
    		corner1(kk) = temp3(kk);
    	}
    }
    if(vpts[1].is_at_infinity())
    {
    	Vector temp1(3, 0.0);
    	Vector temp2(3, 0.0);
    	for(int kk = 0; kk < 3; kk++)
    	{
    		temp1(kk) = corner1(kk);
    		temp2(kk) = corner2(kk);
    	}
    	Vector temp3 = cross(temp1, temp2).normalize();
    	for(int kk = 0; kk < 3; kk++)
    	{
    		corner3(kk) = temp3(kk);
    	}
    }

    if(corner2(1) < 0.0)
    {
    	corner2 *= -1.0;
    }


    /*** PITCH ***/
    /** We now rotate corner2 around the x axis until
     * its component along the z axis is 0. This amount
     * of rotation actually corresponds to the pitch of the
     * camera in a configuration where the parapiped is aligned
     * with the world axes, up to some roll and some yaw
     */

    double pitch = acos ( fabs(corner2(1)) / sqrt(corner2(1)*corner2(1) + corner2(2)*corner2(2)) );
    /** We determine if the rotation is clockwise or
     * counterclockwise */
    if( (corner2(2) > 0 ) && corner2(1) > 0)
    {
        pitch *= -1;
    }
    if( (corner2(2) < 0 ) && corner2(1) < 0)
    {
        pitch *= -1;
    }


    /** Now that we have found the pitch, we rotate the 3D directions of
     * the corner and its position by this angle around the x axis */
    Vector rotation_axis(3, 0.0);
    rotation_axis(0)  = 1.0;
    Quaternion q(rotation_axis, pitch);
    Matrix rotation = q.get_rotation_matrix();
    corner1(3) = 1.0;
    corner2(3) = 1.0;
    corner3(3) = 1.0;
    corner1 = rotation*corner1;
    corner2 = rotation*corner2;
    corner3 = rotation*corner3;

    /*** ROLL ***/
    /** Once again, we expect to have the most "vertical" vector defining the
     * 3D corner stored in corner2. If this is not the case, we swap
     * the vectors accordingly. This should happen very rarely at this point*/
    if( fabs(corner1(1)) > fabs(corner2(1)) )
    {
        Vector temp = corner1;
        corner1 = corner2;
        corner2 = temp;
    }

    if( fabs(corner3(1)) > fabs(corner2(1)) )
    {
        Vector temp = corner1;
        corner1 = corner3;
        corner3 = temp;
    }

    /** We now rotate the vertical direction of the corner around the
     * z-axis until its component along the x axis is 0. This angle corresponds
     * to the roll of the camera, in a configuration where the parapiped is
     * aligned with the world axes, up to some yaw
     */
    corner2(3) = 0.0;
    Vector normal2 = corner2.normalize();
    //check this
    double roll = acos(normal2(1));
    if(corner2(1) < 0.0)
    {
    	roll = acos(-normal2(1));
    }
    corner2(3) = 1.0;
    /** We determine if the rotation is clockwise or
     * counterclockwise */
    if( (corner2(0) > 0 ) && corner2(1) < 0)
    {
        roll *= -1;
    }
    if( (corner2(0) < 0 ) && corner2(1) > 0)
    {
        roll *= -1;
    }

    /** Now that we have found the roll, we rotate the 3D directions of
     * the corner and its position by this angle around the z axis */
    rotation_axis(0) = 0.0;
    rotation_axis(1) = 0.0;
    rotation_axis(2)  = 1.0;
    try
    {
        q.set_axis_angle(rotation_axis, roll);
    } catch (KJB_error e)
    {
        throw e;
    }
    rotation = q.get_rotation_matrix();

    corner1 = rotation*corner1;
    corner2 = rotation*corner2;
    corner3 = rotation*corner3;

    /** We now find the position in 3D of the centre of the parapiped */
    Vector centre(4, 1.0);
    centre(0) = camera.get_camera_centre_x();
    centre(1) = camera.get_camera_centre_y();
    centre(2) = camera.get_camera_centre_z();
    Vector dimensions(3, 0.0);
    dimensions(0) = 5.16;
    dimensions(1) = 5.23;
    dimensions(2) = 5.0;

    /*** YAW ***/
    /* We  now find the yaw, that is to say the angle we have
     * to rotate the parapiped around its y-axis in order to
     * have it completely aligned with the world axes */

    /** The yaw is the angle between corner1 and the world's x-axis */
    double yaw = 0.0;
    if(fabs(corner1(0)) > fabs(corner3(0)))
    {
    	yaw = acos(corner1(0));
    }
    else
    {
    	yaw = acos(corner3(0));
    }

    //yaw = -0.0874;
    /** Now we set the parameters of the parapiped */
    pp.set_roll(0.0);
    pp.set_pitch(0.0);
    pp.set_yaw(yaw);
    pp.set_width(dimensions[0]);
    pp.set_height(dimensions[1]);
    pp.set_length(dimensions[2]);
    pp.set_centre_x(centre(0));
    pp.set_centre_y(centre(1));
    pp.set_centre_z(centre(2));

    //focal_length = 900;
     /** Now we set the parameters of the camera */
    camera.set_camera_centre_x(0.0);
    camera.set_camera_centre_y(0.0);
    camera.set_camera_centre_z(0.0);
    camera.set_pitch(pitch);
    camera.set_roll(-roll);
    camera.set_yaw(0.0);
    camera.set_focal_length(focal_length);
    camera.set_aspect_ratio(1.0);
    camera.set_principal_point_x(0.0); //386.71
    camera.set_principal_point_y(0.0); //168.156
    camera.set_skew(1.57079633);
    camera.set_near(0.1);

    return pitch;

}


bool kjb::find_height_from_two_corners
(
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    const Manhattan_corner & floor_corner,
    const Manhattan_corner & top_corner,
    double & desired_height,
    unsigned int num_rows,
    unsigned int num_cols
)
{
    Parametric_parapiped new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector floor_corner_position(4,1.0);
    floor_corner_position(0) = (floor_corner.get_position() )(0) - princ_x;
    floor_corner_position(1) = - ((floor_corner.get_position() )(1) - princ_y);
    floor_corner_position(2) = - camera.get_focal_length();

    Vector top_corner_position(4,1.0);
    top_corner_position(0) = (top_corner.get_position() )(0) - princ_x;
    top_corner_position(1) = - ((top_corner.get_position() )(1) - princ_y);
    top_corner_position(2) = - camera.get_focal_length();

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(floor_corner_position, floor_corner_position);
    camera.get_point_in_world_coordinates(top_corner_position, top_corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector floor_corner_position_in_pp;
    Vector top_corner_position_in_pp;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(floor_corner_position, floor_corner_position_in_pp);
    new_pp.get_point_in_parapiped_coordinates(top_corner_position, top_corner_position_in_pp);

    //std::cout << "I got evtg in pp coordinates" << std::endl;

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_floor_corner(4,0.0);
    Vector camera_to_top_corner(4,0.0);
    double norm1 = 0.0;
    double norm2 = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
    	camera_to_floor_corner(i) = floor_corner_position_in_pp(i) - camera_centre_in_pp(i);
    	camera_to_top_corner(i) = top_corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm1 += camera_to_floor_corner(i)*camera_to_floor_corner(i);
        norm2 += camera_to_top_corner(i)*camera_to_top_corner(i);
    }

    /** We normalize everything */
    norm1 = sqrt(norm1);
    norm2 = sqrt(norm2);
    camera_to_floor_corner /= norm1;
    camera_to_top_corner /= norm2;
    camera_to_floor_corner(3) = 1.0;
    camera_to_top_corner(3) = 1.0;

    //std::cout << "I normalized evtg" << std::endl;
    /** We consider the intersection with the faces only
     * in the direction determined by the vector from the
     * camera to the corner. We find the farthest intersection,
     * which is also the farthest possible position of the corner
     * such that the corner is still inside the parapiped */
    if(camera_to_floor_corner(1) >= 0.0)
    {
        return false;
    }

    Vector y_plane(4, 0.0);
    y_plane(1) = 1.0;
    y_plane(3) = new_pp.get_height()/2.0;
    Vector floor_intersection;
    double t_y;
    if(!kjb::intersect_3D_line_with_plane(floor_intersection, t_y, camera_centre_in_pp, camera_to_floor_corner, y_plane) )
    {
        return false;
    }

    Vector z_plane_through_point(4, 0.0);
    z_plane_through_point(2) = 1.0;
    z_plane_through_point(3) = -floor_intersection(2);
    Vector top_intersection;
    double t_z;
    if(!kjb::intersect_3D_line_with_plane(top_intersection, t_z, camera_centre_in_pp, camera_to_top_corner, z_plane_through_point) )
    {
        return false;
    }

    desired_height = top_intersection(1) - floor_intersection(1);
    if(desired_height < DBL_EPSILON)
    {
    	return false;
    }
    desired_height /= new_pp.get_height();

    return true;
}

bool kjb::shift_parapiped_to_match_corner
(
    kjb::Parametric_parapiped & pp,
    kjb::Perspective_camera & camera,
    unsigned int num_rows,
    unsigned int num_cols,
    const kjb::Vector & corner
)
{
	double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
	double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

	/** We get the position in 3D of the corner projection onto the image plane, with
	 *  the principal point as the origin. This is in camera coordinates */
	Vector floor_corner_position(4,1.0);
	floor_corner_position(0) = corner(0) - princ_x;
	floor_corner_position(1) = - (corner(1) - princ_y);
	floor_corner_position(2) = - camera.get_focal_length();

    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(floor_corner_position, floor_corner_position);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);

    Vector camera_centre_in_pp;
    Vector floor_corner_position_in_pp;
    pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    pp.get_point_in_parapiped_coordinates(floor_corner_position, floor_corner_position_in_pp);

    Vector camera_to_floor_corner(4,0.0);
    Vector camera_to_top_corner(4,0.0);
    double norm1 = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
    	camera_to_floor_corner(i) = floor_corner_position_in_pp(i) - camera_centre_in_pp(i);
        norm1 += camera_to_floor_corner(i)*camera_to_floor_corner(i);
    }

    /** We normalize everything */
    norm1 = sqrt(norm1);
    camera_to_floor_corner /= norm1;
    camera_to_floor_corner(3) = 1.0;

    if(camera_to_floor_corner(1) >= 0.0)
    {
        return false;
    }
    camera_to_floor_corner *= 8.0;
    camera_to_floor_corner(3) = 1.0;

    pp.set_height(fabs(camera_to_floor_corner(1))*2.0);
    pp.set_centre_y(0.0);
    pp.set_width(fabs(camera_to_floor_corner(0))*2.0);
    pp.set_centre_x(0.0);
    pp.set_length(fabs(camera_to_floor_corner(2))*2.0);
    pp.set_centre_z(0.0);

    return true;
}


bool kjb::propose_frame_inside_parapiped_from_three_corners_on_the_wall
(
    kjb::Vector & centre,
    kjb::Vector & dimensions,
    const Parametric_parapiped &pp,
    const Perspective_camera & camera,
    unsigned int & face_number,
    const Manhattan_corner & corner1,
    const Manhattan_corner & corner2,
    const Manhattan_corner & corner3,
    unsigned int num_rows,
    unsigned int num_cols
)
{
    Parametric_parapiped new_pp = pp;
    new_pp.update_if_needed();
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

    if(centre.size() != 4)
    {
        centre.resize(4, 1.0);
    }
    if(dimensions.size() < 3)
    {
        dimensions.resize(3,0.0);
    }
    Vector desired_dimensions(3, 0.0);

    /** We get the position in 3D of the corner projection onto the image plane, with
     *  the principal point as the origin. This is in camera coordinates */
    Vector corner_position1(4,1.0);
    corner_position1(0) = (corner1.get_position() )(0) - princ_x;
    corner_position1(1) = - ((corner1.get_position() )(1) - princ_y);
    corner_position1(2) = - camera.get_focal_length();

    Vector corner_position2(4,1.0);
    corner_position2(0) = (corner2.get_position() )(0) - princ_x;
    corner_position2(1) = - ((corner2.get_position() )(1) - princ_y);
    corner_position2(2) = - camera.get_focal_length();

    Vector corner_position3(4,1.0);
    corner_position3(0) = (corner3.get_position() )(0) - princ_x;
    corner_position3(1) = - ((corner3.get_position() )(1) - princ_y);
    corner_position3(2) = - camera.get_focal_length();

    /** We now translate everything into world coordinates */
    Vector camera_centre = camera.get_camera_centre();
    camera.get_point_in_world_coordinates(corner_position1, corner_position1);
    camera.get_point_in_world_coordinates(corner_position2, corner_position2);
    camera.get_point_in_world_coordinates(corner_position3, corner_position3);
    camera.get_point_in_world_coordinates(camera_centre, camera_centre);

    /** We now translate everything into a coordinate system where the centre
     * of the input parapiped is the origin, and the world axes are aligned
     * with the parapiped axes */
    Vector camera_centre_in_pp;
    Vector corner_position_in_pp1;
    Vector corner_position_in_pp2;
    Vector corner_position_in_pp3;
    new_pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp);
    new_pp.get_point_in_parapiped_coordinates(corner_position1, corner_position_in_pp1);
    new_pp.get_point_in_parapiped_coordinates(corner_position2, corner_position_in_pp2);
    new_pp.get_point_in_parapiped_coordinates(corner_position3, corner_position_in_pp3);

    /** Find whether it expands towards the camera or on the opposite direction */

    /** We now get the 3D vectors from the camera centre to each projection of a
     * corner vertex. All of this is in parapiped coordinates */
    Vector camera_to_corner1(4,0.0);
    Vector camera_to_corner2(4,0.0);
    Vector camera_to_corner3(4,0.0);
    double norm1 = 0.0;
    double norm2 = 0.0;
    double norm3 = 0.0;
    for(unsigned int i = 0; i < 3; i ++)
    {
        camera_to_corner1(i) = corner_position_in_pp1(i) - camera_centre_in_pp(i);
        camera_to_corner2(i) = corner_position_in_pp2(i) - camera_centre_in_pp(i);
        camera_to_corner3(i) = corner_position_in_pp3(i) - camera_centre_in_pp(i);
        norm1 += camera_to_corner1(i)*camera_to_corner1(i);
        norm2 += camera_to_corner2(i)*camera_to_corner2(i);
        norm3 += camera_to_corner3(i)*camera_to_corner3(i);
    }

    /** We normalize everything */
    norm1 = sqrt(norm1);
    norm2 = sqrt(norm2);
    norm3 = sqrt(norm3);
    camera_to_corner1 /= norm1;
    camera_to_corner2 /= norm2;
    camera_to_corner3 /= norm3;
    camera_to_corner1(3) = 1.0;
    camera_to_corner2(3) = 1.0;
    camera_to_corner3(3) = 1.0;

    //We now find the wall
    Vector x_plane(4, 0.0);
    x_plane(0) = 1.0;
    Vector z_plane(4, 0.0);
    z_plane(2) = 1.0;

    if(camera_to_corner1(0) >= 0.0)
    {
        x_plane(3) = -new_pp.get_width() / 2.0;
    }
    else
    {
        x_plane(3) = new_pp.get_width()/2.0;
    }
    if(camera_to_corner1(2) >= 0.0 )
    {
        z_plane(3) = -new_pp.get_length()/2.0;
    }
    else
    {
        z_plane(3) = new_pp.get_length()/2.0;
    }

    Vector intersection1;
    Vector temp_intersection;
    double t_x = 0.0;
    double max_t;
    bool found_t = false;
    bool frame_x = true;
    if(kjb::intersect_3D_line_with_plane(intersection1, t_x, camera_centre_in_pp, camera_to_corner1, x_plane) )
    {
        ASSERT(t_x);
        max_t = t_x;
        found_t = true;
        frame_x = false;
    }
    double t_z = 0.0;
    if(kjb::intersect_3D_line_with_plane(temp_intersection, t_z, camera_centre_in_pp, camera_to_corner1, z_plane) )
    {
        if(!found_t)
        {
            ASSERT(t_z);
            max_t = t_z;
            found_t = true;
            frame_x = true;
            intersection1 = temp_intersection;
        }
        else if(t_z < max_t)
        {
            max_t = t_z;
            frame_x = true;
            intersection1 = temp_intersection;
        }
    }

    if(frame_x)
    {
        if(camera_to_corner1(2) >= 0.0)
        {
            face_number = 2;
        }
        else
        {
            face_number = 0;
        }
    }
    else
    {
        if(camera_to_corner1(0) >= 0.0)
        {
            face_number = 1;
        }
        else
        {
            face_number = 3;
        }
    }

    Vector intersection2;
    Vector intersectiontop;
    double temp_t;
    if(!found_t)
    {
    	return false;
    }
    if(frame_x)
    {
    	 if(!kjb::intersect_3D_line_with_plane(intersection2, temp_t, camera_centre_in_pp, camera_to_corner2, z_plane) )
    	 {
    		 return false;
    	 }
       	 if(!kjb::intersect_3D_line_with_plane(intersectiontop, temp_t, camera_centre_in_pp, camera_to_corner3, z_plane) )
		 {
			 return false;
		 }
    }
    else
    {
   	     if(!kjb::intersect_3D_line_with_plane(intersection2, temp_t, camera_centre_in_pp, camera_to_corner2, x_plane) )
   	     {
   		     return false;
   	     }
      	 if(!kjb::intersect_3D_line_with_plane(intersectiontop, temp_t, camera_centre_in_pp, camera_to_corner3, x_plane) )
		 {
			 return false;
		 }
    }

    double width = 0.00001;
    double length = 0.00001;
    double height = 0.00001;
    double centre_x = 0.0;
    double centre_y = 0.0;
    double centre_z = 0.0;
    if(frame_x)
    {
        dimensions(0) = fabs(intersection1(0) - intersection2(0));
        dimensions(2) = 0.0001;
        centre(0) = (intersection1(0) + intersection2(0))/2.0;
        centre(2) = 0.1;
    }
    else
    {
    	dimensions(2) = fabs(intersection1(2) - intersection2(2));
    	dimensions(0) = 0.0001;
        centre(2) = (intersection1(2) + intersection2(2))/2.0;
        centre(0) = 0.1;
    }
    dimensions(1) = fabs(intersectiontop(1) - intersection2(1));
    centre(1) = (intersection2(1) + intersectiontop(1))/2.0;

    return true;

}


bool kjb::backproject_point_to_plane // find out the intersection between the view line and a plane in room coordinate
(
	kjb::Vector & intersection,
	kjb::Vector point2D,
	kjb::Vector plane,
    const Parametric_parapiped & pp,
    const Perspective_camera & camera,
	int num_rows,
	int num_cols
)
{
	// backproject the points and intersects with the table top plane
	//double princ_x = num_cols/2.0;
	//double princ_y = num_rows/2.0;
    double princ_x = (num_cols/2.0) + camera.get_principal_point_x();
    double princ_y = (num_rows/2.0) - camera.get_principal_point_y();

	//Back_projector back_project;
	// get the position of the camera in the room coordinate

	// get the position of the bottom center of the bounding box in the room coordinate
	/** We get the position in 3D of the corner projection onto the image plane, with
		*  the principal point as the origin. This is in camera coordinates */
	Vector point_c(4,1.0);
	point_c(0) = point2D(0) - princ_x;
	point_c(1) = - (point2D(1) - princ_y);
	point_c(2) = - camera.get_focal_length(); // -z axis
    
    std::cout << "point_c: " << point_c << std::endl;
    
    /** We now translate everything into world coordinates */
	Vector camera_centre = camera.get_camera_centre();
	camera.get_point_in_world_coordinates(point_c, point_c);
    std::cout << "point_c: " << point_c << std::endl;
	camera.get_point_in_world_coordinates(camera_centre, camera_centre);
    std::cout << "point_c: " << point_c << "camera_centre: " << camera_centre << std::endl;
	
    /** We now translate everything into a coordinate system where the centre
		* of the input parapiped is the origin, and the world axes are aligned
		* with the parapiped axes */
	Vector camera_centre_in_pp;
	Vector corner_position_in_pp;
	pp.get_point_in_parapiped_coordinates(camera_centre, camera_centre_in_pp); // room coordinates, rotated world coordinate
	pp.get_point_in_parapiped_coordinates(point_c, corner_position_in_pp);

    std::cout << "camera_centre in pp: " << camera_centre_in_pp << "corner position in pp: " << corner_position_in_pp << std::endl;

	/** We now get the 3D vectors from the camera centre to each projection of a
		* corner vertex. All of this is in parapiped coordinates */
	Vector camera_to_corner(4,0.0);
	double norm = 0.0;
	for(unsigned int i = 0; i < 3; i ++)
	{
		camera_to_corner(i) = corner_position_in_pp(i) - camera_centre_in_pp(i);
		norm += camera_to_corner(i)*camera_to_corner(i);
	}

	/** We normalize everything */
	norm = sqrt(norm);
	camera_to_corner /= norm;
	camera_to_corner(3) = 1.0;
    std::cout << "camera_to_corner: " << camera_to_corner << std::endl;

	//std::cout << "I normalized evtg" << std::endl;
	/** We consider the intersection with the faces only
		* in the direction determined by the vector from the
		* camera to the corner. We find the farthest intersection,
		* which is also the farthest possible position of the corner
		* such that the corner is still inside the parapiped */
/*	if (camera_to_corner(1) >= 0.0)
	{
		return false;
	}*/
	//We should worry about intercepting the ceiling

	double t_y;
	//std::cout << "Check intersections" << std::endl;
	if(!kjb::intersect_3D_line_with_plane(intersection, t_y, camera_centre_in_pp, camera_to_corner, plane) )
	{
    	return false;
	}

	return true;	
}


void update_focal_with_position(Perspective_camera& cam, double ifocal, Parametric_parapiped & pp)
{
    double focal_ratio = ifocal/cam.get_focal_length();
    kjb::Vector position = pp.get_centre();
    position(3) = 0.0;

    cam.set_focal_length(ifocal);
    double world_scale = (1.0/focal_ratio)*cam.get_world_scale();
    cam.set_world_scale(world_scale);

    double norm = position(0)*position(0) + position(1)*position(1) + position(2)*position(2);

    position = position.normalize();
    position = position*(norm/focal_ratio);
    position(3) = 1.0;

    pp.set_centre(position);

}
