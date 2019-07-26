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
|     Luca Del Pero, Yuanliu Liu
|
* =========================================================================== */

#include "camera_cpp/camera_frustum.h"
#include "camera_cpp/perspective_camera.h"
#include "edge_cpp/manhattan_world.h"

#include <iostream>
#include <sstream>
#include <typeinfo>

using namespace kjb;

/*
 * @param ix The x coordinate of the centre
 * @param iy The y coordinate of the centre
 * @param iz The z coordinate of the centre
 * @param inw The number of vertices of the polygon for approximating the circle
 * @param iw The width of the bottom surface
 * @param il The length of the top surface
 * @param iratio_top_bottom The size ratio between the top and bottom surface
 * @param il The height of this frustum (along y axis)
 * @param ipitch The pitch of this frustum (rotation angle around its x axis)
 * @param iyaw The yaw of this frustum (rotation angle around its y axis)
 * @param iroll The roll of this frustum (rotation angle around its z axis)
 */
Parametric_frustum::Parametric_frustum(unsigned int inv, double ix, double iy,
double iz, double iw, double il, double iratio_top_bottom, double ih, double ipitch, double iyaw, double iroll) throw(kjb::Illegal_argument)
: Renderable_model(false), Readable(), Writeable(), centre(4, 1.0), rotation_angles(3, 0.0),
  rendering_interface(inv, ix, iy, iz, iw, il, iratio_top_bottom, ih, ipitch, iyaw, iroll)
{
    if( (iw <= 0.0) || (il <= 0.0) || (iratio_top_bottom <= 0.0) || (ih <= 0.0) )
    {
        throw kjb::Illegal_argument("Frustum constructor, dimensions must be positive");
    }

	num_vertices = inv;
    //width = irb;
	//length = irt;
    width = iw;
    length = il;
    ratio_top_bottom = iratio_top_bottom;
    height = ih;
    centre(0) = ix;
    centre(1) = iy;
    centre(2) = iz;
    rotation_angles(CAMERA_FRUSTUM_PITCH) = ipitch;
    rotation_angles(CAMERA_FRUSTUM_YAW) = iyaw;
    rotation_angles(CAMERA_FRUSTUM_ROLL) = iroll;
    set_rendering_representation_dirty();

}

/*
 * @param src the parametric_frustum to copy into this one
 */
Parametric_frustum::Parametric_frustum(const Parametric_frustum & src)
: Renderable_model(src), Readable(), Writeable(), centre(src.centre),
  rotation_angles(src.rotation_angles), rendering_interface(src.rendering_interface)
{
    num_vertices = src.num_vertices;
    width = src.width;
    length = src.length;
    ratio_top_bottom = src.ratio_top_bottom;
	height = src.height;
}

/*
 * @param fname The name of the file to read this parametric_frustum from
 */
Parametric_frustum::Parametric_frustum(const char* fname) throw (kjb::Illegal_argument,
        kjb::IO_error)
:  Renderable_model(false),
   centre(4, 1.0),  rotation_angles(3, 0.0),
   rendering_interface(6, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0)
{
    kjb::Readable::read(fname);
}

/*
 * @param in The input stream to read this parametric_frustum from
 */
Parametric_frustum::Parametric_frustum(std::istream& in) throw (kjb::Illegal_argument,
        kjb::IO_error)
:  Renderable_model(true),
   centre(4, 1.0), rotation_angles(3, 0.0),
   rendering_interface(6, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0)
{
    read(in);
}

/*
 * @param src The parametric_frustum to be assigned to this one
 */
Parametric_frustum & Parametric_frustum::operator = (const Parametric_frustum & src)
{
    Renderable_model::operator=(src);

    num_vertices = src.num_vertices;
    width = src.width;
    length = src.length;
    ratio_top_bottom = src.ratio_top_bottom;
	height = src.height;

    centre = src.centre;

    rotation_angles = src.rotation_angles;

    rendering_interface = src.rendering_interface;

    return (*this);
}

/*
 * Virtual copy constructor
 */
Parametric_frustum * Parametric_frustum::clone() const
{
    return new  Parametric_frustum(*this);
}

/*
 * @param iwidth The new diameter of the bottom of the frustum along x axis
 */
void Parametric_frustum::set_width(double iwidth) throw(kjb::Illegal_argument)
{
	if(iwidth <= 0)
    {
        throw kjb::Illegal_argument("Frustum bottom width must be positive");
    }

	width = iwidth;
    set_rendering_representation_dirty();

}

/*
 * @param iheight The new height of this frustum (along its y axis)
 */
void Parametric_frustum::set_height(double iheight) throw(kjb::Illegal_argument)
{
    if(iheight <= 0)
    {
        throw kjb::Illegal_argument("Cone height must be positive");
    }

    height = iheight;
    set_rendering_representation_dirty();
}

/*
 * @param iheight The new ratio between top and bottom of this frustum
 */
void Parametric_frustum::set_ratio_top_bottom(double iratio_top_bottom) throw(kjb::Illegal_argument)
{
    if(iratio_top_bottom <= 0)
    {
        throw kjb::Illegal_argument("Top bottom ratio must be positive");
    }

    ratio_top_bottom = iratio_top_bottom;
    set_rendering_representation_dirty();
}

/*
 * @param ilength The new diameter of the bottom of the frustum along z axis
 */
void Parametric_frustum::set_length(double ilength)throw(kjb::Illegal_argument)
{
    if(ilength <= 0)
    {
        throw kjb::Illegal_argument("Frustum bottom length must be positive");
    }

	length = ilength;
    set_rendering_representation_dirty();
}

/*
 * @param ipitch The new pitch of this frustum (rotation around its x axis)
 */
void Parametric_frustum::set_pitch(double ip)
{
    rotation_angles(CAMERA_FRUSTUM_PITCH) = ip;
    set_rendering_representation_dirty();
}

/*
 * @param iyaw The new yaw of this frustum (rotation around its y axis)
 */
void Parametric_frustum::set_yaw(double iy)
{
    rotation_angles(CAMERA_FRUSTUM_YAW) = iy;
    set_rendering_representation_dirty();
}

/*
 * @param iroll The new roll of this frustum (rotation around its z axis)
 */
void Parametric_frustum::set_roll(double ir)
{
    rotation_angles(CAMERA_FRUSTUM_ROLL) = ir;
    set_rendering_representation_dirty();
}

/**
 * Given 3 rotation angles, sets the corresponding Euler angles given the current Euler convention
 *
 * @param dpitch The amount of rotation arount the x-axis of this frustum
 * @param dyaw   The amount of rotation arount the y-axis of this frustum
 * @param droll  The amount of rotation arount the z-axis of this frustum
 */
void Parametric_frustum::compute_new_euler_angles_on_rotations(double dpitch, double dyaw, double droll)
{
    update_rendering_representation();
    rendering_interface.compute_new_euler_angles_on_rotations(dpitch, dyaw, droll, rotation_angles);
    set_rendering_representation_dirty();
}

/*
 * @param theta The amount of rotation to be done around the current x
 *              axis of the frustum
 */
void Parametric_frustum::rotate_around_x_axis(double theta)
{
    compute_new_euler_angles_on_rotations(theta, 0.0, 0.0);
}

/*
 * @param theta The amount of rotation to be done around the current y
 *              axis of the frustum
 */
void Parametric_frustum::rotate_around_y_axis(double theta)
{
    compute_new_euler_angles_on_rotations(0.0, theta, 0.0);
}

/*
 * @param theta The amount of rotation to be done around the current z
 *              axis of the frustum
 */
void Parametric_frustum::rotate_around_z_axis(double theta)
{
    compute_new_euler_angles_on_rotations(0.0, 0.0, theta);
}

/*
 * Order of rotation is: thetax, thetay, thetaz
 *
 * @param thetax The amount of rotation to be done around the current x
 *              axis of the truncated one
 * @param thetay The amount of rotation to be done around the current y
 *              axis of the truncated one
 * @param thetaz The amount of rotation to be done around the current z
 *              axis of the truncated one
 */
void Parametric_frustum::rotate_around_frustum_axes(double thetax, double thetay, double thetaz)
{
    compute_new_euler_angles_on_rotations(thetax, thetay, thetaz);
}

/*
 * @param icentre The new centre of this frustum
 */
void Parametric_frustum::set_centre(const kjb::Vector & icentre) throw(kjb::Illegal_argument)
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
            throw kjb::Illegal_argument("Frustum, set centre, input centre vector, homogeneous coordinate = 0.0");
        }
        centre = icentre/icentre(3);
    }
    else
    {
        throw kjb::Illegal_argument("Frustum, set centre, input centre vector has wrong dimensions");
    }
    set_rendering_representation_dirty();
}

/*
 * @param ix The new x coordinate of the centre of this frustum
 */
void Parametric_frustum::set_centre_x(double ix) throw(kjb::Illegal_argument)
{
    centre(0) = ix;
    set_rendering_representation_dirty();
}

/*
 * @param iy The new y coordinate of the centre of this frustum
 */
void Parametric_frustum::set_centre_y(double iy) throw(kjb::Illegal_argument)
{
    centre(1) = iy;
    set_rendering_representation_dirty();
}

/*
 * @param iz The new z coordinate of the centre of this frustum
 */
void Parametric_frustum::set_centre_z(double iz) throw(kjb::Illegal_argument)
{
    centre(2) = iz;
    set_rendering_representation_dirty();
}

/**
 * Sets the rotation angles from an input quaternion
 *
 * @param q the input quaternion
 */
void Parametric_frustum::set_angles_from_quaternion(const kjb::Quaternion & q)
{
    KJB(UNTESTED_CODE());
    rendering_interface.set_orientation(q);
    rotation_angles = rendering_interface.get_euler_angles();
}

/*
 * Updates the rendering representation of this frustum.
 * First it creates a frustum with centre in the origin, aligned
 * with world axes and with correct width, height and length.
 * Then it rotates it according to pitch, yaw and roll, and translates
 * it to the position specified by its centre
 */
void Parametric_frustum::update_rendering_representation() const throw(kjb::KJB_error)
{
    rendering_interface.set_points(num_vertices, 0, 0, 0, width, length,
    ratio_top_bottom, height);
    rendering_interface.set_rotations_and_translate(rotation_angles(CAMERA_FRUSTUM_PITCH), rotation_angles(CAMERA_FRUSTUM_YAW),
            rotation_angles(CAMERA_FRUSTUM_ROLL), centre(0), centre(1), centre(2));
}

/*
 * @return the rendering interface used to render this parametric_frustum
 */
Abstract_renderable & Parametric_frustum::get_rendering_interface() const
{
    update_rendering_representation();
    return rendering_interface;
}

/*
 * @param in The input stream to read this frustum from
 */
void Parametric_frustum::read(std::istream& in) throw (kjb::Illegal_argument,
        kjb::IO_error)
{
    using std::ostringstream;
    using std::istringstream;

    const char* type_name = typeid(*this).name(); // "frustum"
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
	
    if (!(field_value = read_field_value(in, "number_of_vertices")))
    {
        throw Illegal_argument("Missing number of vertices");
    }
    istringstream ist(field_value);
    ist >> num_vertices;
    if (ist.fail() || (num_vertices < 3))
    {
        throw Illegal_argument("Invalid number of vertices");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "width")))
    {
        throw Illegal_argument("Missing width of bottom surface");
    }
    ist.str(field_value);
    ist >> width;
    if (ist.fail() || (width <= 0.0))
    {
        throw Illegal_argument("Invalid width of bottom surface");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "length")))
    {
        throw Illegal_argument("Missing length of bottom surface");
    }
    ist.str(field_value);
    ist >> length;
    if (ist.fail() || (length <= 0.0))
    {
        throw Illegal_argument("Invalid length of bottom surface");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "ratio_top_bottom")))
    {
        throw Illegal_argument("Missing ratio between top and bottom surface");
    }
    ist.str(field_value);
    ist >> ratio_top_bottom;
    if (ist.fail() || (ratio_top_bottom <= 0.0))
    {
        throw Illegal_argument("Invalid ratio between top and bottom surface");
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

    if (!(field_value = read_field_value(in, "pitch")))
    {
        throw Illegal_argument("Missing pitch");
    }
    ist.str(field_value);
    ist >> rotation_angles(CAMERA_FRUSTUM_PITCH);
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
    ist >> rotation_angles(CAMERA_FRUSTUM_YAW);
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
    ist >> rotation_angles(CAMERA_FRUSTUM_ROLL);
    if (ist.fail())
    {
        throw Illegal_argument("Invalid Roll");
    }
    ist.clear(std::ios_base::goodbit);

    // frustum centre
    if (!(field_value = read_field_value(in, "centre")))
    {
        throw Illegal_argument("Missing frustum Centre");
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
 * @param out The output stream to write this frustum to
 */
void Parametric_frustum::write(std::ostream& out) const
   throw (kjb::IO_error)
{

    out << "     Type: " << typeid(*this).name() << '\n'
    << "      num_vertices: " << num_vertices << '\n'
	<< "      width: " << width << '\n'
	<< "      length: " << length << '\n'
    << "      ratio_top_bottom: " << ratio_top_bottom << '\n'
    << "      height: " << height << '\n'
    << "      pitch: " << rotation_angles(CAMERA_FRUSTUM_PITCH) << '\n'
    << "      yaw: " << rotation_angles(CAMERA_FRUSTUM_YAW) << '\n'
    << "      roll: " << rotation_angles(CAMERA_FRUSTUM_ROLL) << '\n'
    << "      centre: " << centre(0) << ' '
                     << centre(1) << ' '
                     << centre(2) << ' '
                     << centre(3) << '\n';
}

void Parametric_frustum::stretch_along_axis(
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

    if(axis == CAMERA_FRUSTUM_LENGTH)
    {
        length += amount;
    }
    else if(axis == CAMERA_FRUSTUM_WIDTH)
    {
        width += amount;
    }
    else if(axis == CAMERA_FRUSTUM_TOP_RADIUS)
    {
        ratio_top_bottom += amount;
    }
    else if (axis == CAMERA_FRUSTUM_HEIGHT)
    {
        height += amount;
    }
	else
	{
		throw Illegal_argument("Wrong stretch axis (<=2)");
	}

    if(direction)
    {
        if(axis == CAMERA_FRUSTUM_HEIGHT)
        {
            centre(axis) += amount/2.0;
        }
    }
    else
    {
        if(axis == CAMERA_FRUSTUM_HEIGHT)
        {
            centre(axis) -= amount/2.0;
        }
    }
    set_rendering_representation_dirty();

}

/*void Parametric_frustum::draw_orientation_map() const
{
    update_rendering_representation();
    ( (const Frustum &) this->get_polymesh() ).draw_orientation_map();
}

void Parametric_frustum::draw_left_right_orientation_map() const
{
    update_rendering_representation();
    ( (const Frustum &) this->get_polymesh()).draw_left_right_orientation_map();
}

void Parametric_frustum::draw_geometric_context_map() const
{
    update_rendering_representation();
    ( (const Frustum &) this->get_polymesh()).draw_geometric_context_map();
}*/

/**
 * Transforms a point in world coordinates to a coordinate
 *  system where the frustum centre is the origin, and the
 *  axes are defined by the frustum axes
 *  */
void Parametric_frustum::get_point_in_parapiped_coordinates
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
void Parametric_frustum::get_point_in_world_coordinates
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

void Parametric_frustum::get_lines(std::vector<Line3d> & lines)
{
    update_rendering_representation();
    rendering_interface.get_lines(lines);
}

void Parametric_frustum::get_vertices(std::vector<Vector> & vertices)
{
    update_rendering_representation();
    rendering_interface.get_all_vertices(vertices);
}
