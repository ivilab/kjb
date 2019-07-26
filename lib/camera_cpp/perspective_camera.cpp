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

// this should be first:
#ifdef KJB_HAVE_BST_SERIAL
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/export.hpp>
#endif

#include "l_cpp/l_util.h"
#include "gr_cpp/gr_opengl.h"
#include "camera_cpp/perspective_camera.h"

#include <sstream>
#include <typeinfo>
#include <iostream>

#define KJB_DEG_TO_RAD 0.01745329251994
#define SKEW_90 1.57079633
#define INIT_FOCAL_LENGTH_VALUE 1000

namespace kjb
{
/**
 * @param inear The distance between the camera centre and the near clipping plane
 * @param ifar The distance between the camera centre and the far clipping plane
 */
Perspective_camera::Perspective_camera(double inear, double ifar)
:
    Cloneable(),
    Readable(),
    Writeable(),
    camera_centre(4, 0.0),
    rotation_angles(3, 0.0),
    focal_length(INIT_FOCAL_LENGTH_VALUE),
    principal_point(2,0.0),
    skew(SKEW_90),
    aspect_ratio(1.0),
    world_scale(1.0),
    rendering_interface(inear, ifar),
    intrinsic_dirty(true),
    extrinsic_dirty(true),
    cam_matrix_dirty(true),
    m_rd_k1(0.0),
    m_rd_k2(0.0),
    m_rd_k3(0.0),
    m_rd_p1(0.0),
    m_rd_p2(0.0)
{
    //Homogeneous coordinate
    camera_centre(3) = 1.0;

    update_rendering_interface();
}

/**
 * @param icentre_x The x coordinate of the camera centre
 * @param icentre_y The y coordinate of the camera centre
 * @param icentre_z The z coordinate of the camera centre
 * @param ipitch The rotation around the camera's x axis
 * @param iyaw The rotation around the camera's y axis
 * @param iroll The rotation around the camera's x axis
 * @param ifocal_length The focal length
 * @param iprincipal_point_x The x coordinate of the principal point
 * @param iprincipal_point_y The y coordinate of the principal point
 * @param iskew The skew angle in radian
 * @param iaspect_ratio The camera aspect ratio
 * @param iscale The scale of the world
 * @param inear The distance between the camera centre and the near clipping plane
 * @param ifar The distance between the camera centre and the far clipping plane
 */
Perspective_camera::Perspective_camera(
       double icentre_x,
       double icentre_y,
       double icentre_z,
       double ipitch,
       double iyaw,
       double iroll,
       double ifocal_length,
       double iprincipal_point_x,
       double iprincipal_point_y,
       double iskew,
       double iaspect_ratio,
       double iscale,
       double inear,
       double ifar) :
    Cloneable(),
    Readable(),
    Writeable(),
    camera_centre(4),
    rotation_angles(3, 0.0),
    focal_length(ifocal_length),
    principal_point(2),
    skew(iskew),
    aspect_ratio(iaspect_ratio),
    world_scale(iscale),
    rendering_interface(inear, ifar),
    intrinsic_dirty(true),
    extrinsic_dirty(true),
    cam_matrix_dirty(true),
    m_rd_k1(0.0),
    m_rd_k2(0.0),
    m_rd_k3(0.0),
    m_rd_p1(0.0),
    m_rd_p2(0.0)
{
    if(iaspect_ratio <= 0)
    {
    // This isn't always true.  If a camera was calibrated using non-standard conventions, you could get a negative focal length in either direction.
    // Commenting out, for eventual deletion.  Kyle, Aug 23, 2010
//        KJB_THROW_2(kjb::Illegal_argument, "Aspect ratio for perspective camera must be positive");
    }
    if(iscale <= 0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "World scale for perspective camera must be positive");
    }
    camera_centre(0) = icentre_x;
    camera_centre(1) = icentre_y;
    camera_centre(2) = icentre_z;
    camera_centre(3) = 1.0; //Homegeneous coordinate

    rotation_angles(CAMERA_PITCH_INDEX) = ipitch;
    rotation_angles(CAMERA_YAW_INDEX) = iyaw;
    rotation_angles(CAMERA_ROLL_INDEX) = iroll;

    principal_point(0) = iprincipal_point_x;
    principal_point(1) = iprincipal_point_y;
    update_rendering_interface();
}

/**
 * @param icamera_centre The camera centre position [x,y,z], optionally in homogeneous coordinates
 * @param ipitch The rotation around the camera's x axis
 * @param iyaw The rotation around the camera's y axis
 * @param iroll The rotation around the camera's x axis
 * @param ifocal_length The focal length
 * @param iprincipal_point_x The x coordinate of the principal point
 * @param iprincipal_point_y The y coordinate of the principal point
 * @param iskew The skew angle in radian
 * @param iaspect_ratio The camera aspect ratio
 * @param iscale The scale of the world
 * @param inear The distance between the camera centre and the near clipping plane
 * @param ifar The distance between the camera centre and the far clipping plane
 */
Perspective_camera::Perspective_camera(
        const kjb::Vector & icamera_centre,
        double ipitch,
        double iyaw,
        double iroll,
        double ifocal_length,
        double iprincipal_point_x,
        double iprincipal_point_y,
        double iskew,
        double iaspect_ratio,
        double iscale,
        double inear,
        double ifar) :
    Cloneable(),
    Readable(),
    Writeable(),
    camera_centre(4),
    //rotation_angles(ipitch, iyaw, iroll), <--- got rid of this constructor
    rotation_angles(Vector().set(ipitch, iyaw, iroll)),
    focal_length(ifocal_length),
    principal_point(2),
    skew(iskew),
    aspect_ratio(iaspect_ratio),
    world_scale(iscale),
    rendering_interface(inear, ifar),
    intrinsic_dirty(true),
    extrinsic_dirty(true),
    cam_matrix_dirty(true),
    m_rd_k1(0.0),
    m_rd_k2(0.0),
    m_rd_k3(0.0),
    m_rd_p1(0.0),
    m_rd_p2(0.0)
{
    // This isn't always true.  If a camera was calibrated using non-standard conventions, you could get a negative focal length in either direction.
    // Commenting out, for eventual deletion.  Kyle, Aug 23, 2010
//    if(iaspect_ratio <= 0)
//    {
//        KJB_THROW_2(kjb::Illegal_argument, "Aspect ratio for perspective camera must be positive");
//    }
    if(iscale <= 0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "World scale for perspective camera must be positive");
    }


    if(icamera_centre.size() == 4)
    {
        camera_centre = icamera_centre;
    }
    else if(icamera_centre.size() == 3)
    {
        camera_centre(0) = icamera_centre(0);
        camera_centre(1) = icamera_centre(1);
        camera_centre(2) = icamera_centre(2);
        camera_centre(3) = 1.0; //Homogeneous coordinate
    }

    principal_point(0) = iprincipal_point_x;
    principal_point(1) = iprincipal_point_y;
    update_rendering_interface();
}


/**
 * @fname The name of the file to read this camera from
 */
Perspective_camera::Perspective_camera(const char* fname) :
    Cloneable(),
    Readable(),
    Writeable(),
    camera_centre(4),
    rotation_angles(3),
    focal_length(INIT_FOCAL_LENGTH_VALUE),
    principal_point(2),
    skew(SKEW_90),
    aspect_ratio(1.0),
    world_scale(1.0),
    rendering_interface(),
    intrinsic_dirty(true),
    extrinsic_dirty(true),
    cam_matrix_dirty(true),
    m_rd_k1(0.0),
    m_rd_k2(0.0),
    m_rd_k3(0.0),
    m_rd_p1(0.0),
    m_rd_p2(0.0)
{
    kjb::Readable::read(fname);
}

/**
 * @in The input stream to read this camera from
 */
Perspective_camera::Perspective_camera(std::istream& in) :
    Cloneable(),
    Readable(),
    Writeable(),
    camera_centre(4),
    rotation_angles(3),
    focal_length(INIT_FOCAL_LENGTH_VALUE),
    principal_point(2),
    skew(SKEW_90),
    aspect_ratio(1.0),
    world_scale(1.0),
    rendering_interface(),
    intrinsic_dirty(true),
    extrinsic_dirty(true),
    cam_matrix_dirty(true),
    m_rd_k1(0.0),
    m_rd_k2(0.0),
    m_rd_k3(0.0),
    m_rd_p1(0.0),
    m_rd_p2(0.0)
{
    read(in);
}

/**
 * @pc The camera to copy into this one
 */
Perspective_camera::Perspective_camera(const Perspective_camera & pc) :
    Cloneable(),
    Readable(),
    Writeable(),
    camera_centre(pc.camera_centre),
    rotation_angles(pc.rotation_angles),
    focal_length(pc.focal_length),
    principal_point(pc.principal_point),
    skew(pc.skew),
    aspect_ratio(pc.aspect_ratio),
    world_scale(pc.world_scale),
    rendering_interface(pc.rendering_interface),
    intrinsic_dirty(true),
    extrinsic_dirty(true),
    cam_matrix_dirty(true),
    m_camera_matrix(pc.m_camera_matrix),
    m_rd_k1(pc.m_rd_k1),
    m_rd_k2(pc.m_rd_k2),
    m_rd_k3(pc.m_rd_k3),
    m_rd_p1(pc.m_rd_p1),
    m_rd_p2(pc.m_rd_p1)
{
}

/**
 * @pc The camera to assign to this one
 */
Perspective_camera & Perspective_camera::operator=(const Perspective_camera & pc)
{
    camera_centre = pc.camera_centre;
    rotation_angles = pc.rotation_angles;

    focal_length = pc.focal_length;
    principal_point = pc.principal_point;
    skew = pc.skew;
    aspect_ratio = pc.aspect_ratio;
    world_scale = pc.world_scale;

    rendering_interface = pc.rendering_interface;
    intrinsic_dirty = pc.intrinsic_dirty;
    extrinsic_dirty = pc.extrinsic_dirty;
    cam_matrix_dirty = pc.cam_matrix_dirty;
    m_camera_matrix = pc.m_camera_matrix;

    m_rd_k1 = pc.m_rd_k1;
    m_rd_k2 = pc.m_rd_k2;
    m_rd_k3 = pc.m_rd_k3;
    m_rd_p1 = pc.m_rd_p1;
    m_rd_p2 = pc.m_rd_p1;

    return *this;
}

/**
 * Virtual copy contructor
 */
Perspective_camera * Perspective_camera::clone() const
{
    return new Perspective_camera(*this);
}

void Perspective_camera::swap(Perspective_camera& other)
{
    using std::swap;

    camera_centre.swap(other.camera_centre);
    rotation_angles.swap(other.rotation_angles);
    swap(focal_length, other.focal_length);
    principal_point.swap(other.principal_point);
    swap(skew, other.skew);
    swap(aspect_ratio, other.aspect_ratio);
    swap(world_scale, other.world_scale);
    rendering_interface.swap(other.rendering_interface);
    swap(m_camera_matrix, other.m_camera_matrix);
    swap(intrinsic_dirty, other.intrinsic_dirty);
    swap(extrinsic_dirty, other.extrinsic_dirty);
    swap(cam_matrix_dirty, other.cam_matrix_dirty);
    swap(m_rd_k1, other.m_rd_k1);
    swap(m_rd_k2, other.m_rd_k2);
    swap(m_rd_k3, other.m_rd_k3);
    swap(m_rd_p1, other.m_rd_p1);
    swap(m_rd_p2, other.m_rd_p1);
}

void Perspective_camera::mult_modelview_matrix() const
{
    update_rendering_interface();
#ifdef KJB_HAVE_OPENGL
    kjb::opengl::glMultMatrix(rendering_interface.get_modelview_matrix());
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

Vector Perspective_camera::to_camera_coordinates(const Vector& v) const
{
    Vector result = v;

    if(result.size() == 3)
    {
        result -= Vector(get_camera_centre()).resize(3);
    }
    else if(result.size() == 4)
    {
        result /= result[3];
        result -= get_camera_centre();
    }
    else
    {
        if(result.size() != 4)
        {
            KJB_THROW_2(Illegal_argument, "v must have dimension 3 or 4.");
        }
    }

    result = get_orientation().rotate(result);

    return result;

}

void Perspective_camera::update_camera_matrix() const
{
    if(!cam_matrix_dirty) return;

    Matrix r = get_orientation().get_rotation_matrix();
    r.resize(3,3);

    Matrix k(3,3,0.0);

    // if nearly perpendicular, skip computation and set
    // to default values to avoid numerical problems like
    // 1/inf.
    double skew_factor = 0.0;
    double skew_shortening = 1.0;
    if(fabs(get_skew() - M_PI / 2) > 10 * FLT_EPSILON)
    {
        skew_shortening = sin(get_skew());
        skew_factor = -get_focal_length() / tan(get_skew());
    }

    k(0,0) = get_focal_length() * get_aspect_ratio();
    k(1,1) = get_focal_length() / skew_shortening;
    k(0,1) = skew_factor;

    // Third row of k is negated, to cancel out the fact that
    // the rotation matrix uses opengl's standard for z-axis
    // (camera looks in -z direction).
    //
    // Note: we could have gotten the same affect by not
    // negating k(:,3), but isntead negating the first two rows of the
    // extrinsic parameter matrix.
    k(0,2) = -get_principal_point_x();
    k(1,2) = -get_principal_point_y();
    k(2,2) = -1.0;

    m_camera_matrix = k * r;
    m_camera_matrix.resize(3,4,0.0);

    Vector t = -k * r * Vector(camera_centre).resize(3);
    m_camera_matrix(0,3) = t[0];
    m_camera_matrix(1,3) = t[1];
    m_camera_matrix(2,3) = t[2];

    cam_matrix_dirty = false;
}


/**
 * @param in The input stream to read from
 */
void Perspective_camera::read(std::istream& in)
{
    using std::ostringstream;
    using std::istringstream;

    const char* type_name = typeid(*this).name();
    const char* field_value;

     // Type
    if (!(field_value = read_field_value(in, "Type")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Type field");
    }
    if (strncmp(field_value, type_name, strlen(type_name)) != 0)
    {
        ostringstream ost;
        ost << "Tried to read a '" << field_value << "' as a '"
            << type_name << "'";
        KJB_THROW_2(Illegal_argument, ost.str());
    }

    // Camera centre
    if (!(field_value = read_field_value(in, "camera_centre")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Camera Centre");
    }
    istringstream ist(field_value);
    ist >> camera_centre(0) >> camera_centre(1) >> camera_centre(2) >> camera_centre(3);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Camera ccentre");
    }
    ist.clear(std::ios_base::goodbit);

    camera_centre = camera_centre / camera_centre(3);

    if (!(field_value = read_field_value(in, "pitch")))
    {
        KJB_THROW_2(Illegal_argument, "Missing pitch");
    }
    ist.str(field_value);
    ist >> rotation_angles(CAMERA_PITCH_INDEX);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Pitch");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "yaw")))
    {
        KJB_THROW_2(Illegal_argument, "Missing yaw");
    }
    ist.str(field_value);
    ist >> rotation_angles(CAMERA_YAW_INDEX);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Yaw");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "roll")))
    {
        KJB_THROW_2(Illegal_argument, "Missing roll");
    }
    ist.str(field_value);
    ist >> rotation_angles(CAMERA_ROLL_INDEX);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Roll");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "focal_length")))
    {
        KJB_THROW_2(Illegal_argument, "Missing focal length");
    }
    ist.str(field_value);
    ist >> focal_length;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Focal length");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "principal_point")))
    {
        KJB_THROW_2(Illegal_argument, "Missing Principal_point");
    }
    ist.str(field_value);
    ist >> principal_point(0) >> principal_point(1);
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Principal point");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "skew")))
    {
        KJB_THROW_2(Illegal_argument, "Missing skew");
    }
    ist.str(field_value);
    ist >> skew;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Skew");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "aspect_ratio")))
    {
        KJB_THROW_2(Illegal_argument, "Missing aspect_ratio");
    }
    ist.str(field_value);
    ist >> aspect_ratio;
    if (ist.fail() || aspect_ratio <= 0.0)
    {
        KJB_THROW_2(Illegal_argument, "Invalid Aspect ratio");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "world_scale")))
    {
        KJB_THROW_2(Illegal_argument, "Missing world scale");
    }
    ist.str(field_value);
    ist >> world_scale;
    if (ist.fail() || (world_scale <=0.0))
    {
        KJB_THROW_2(Illegal_argument, "Invalid world scale");
    }
    ist.clear(std::ios_base::goodbit);

    double tempnear, tempfar;


    if (!(field_value = read_field_value(in, "near")))
    {
        KJB_THROW_2(Illegal_argument, "Missing near clipping plane");
    }
    ist.str(field_value);
    ist >> tempnear;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Near clipping plane ");
    }
    ist.clear(std::ios_base::goodbit);

    if (!(field_value = read_field_value(in, "far")))
    {
        KJB_THROW_2(Illegal_argument, "Missing far clipping plane");
    }
    ist.str(field_value);
    ist >> tempfar;
    if (ist.fail())
    {
        KJB_THROW_2(Illegal_argument, "Invalid Far clipping plane ");
    }
    ist.clear(std::ios_base::goodbit);

    rendering_interface.set_clipping_planes(tempnear, tempfar);

    /** We set the status of the rendering interface as not updated,
     * it will be updated the first time we need to render the camera */
    intrinsic_dirty = true;
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param out The output stream to write to
 */
 void Perspective_camera::write(std::ostream& out) const

{

    out << "     Type: " << typeid(*this).name() << '\n'
    << "      camera_centre: " << camera_centre(0) << ' '
                     << camera_centre(1) << ' '
                     << camera_centre(2) << ' '
                     << camera_centre(3) << '\n'
    << "      pitch: " << rotation_angles(CAMERA_PITCH_INDEX) << '\n'
    << "      yaw: " << rotation_angles(CAMERA_YAW_INDEX) << '\n'
    << "      roll: " << rotation_angles(CAMERA_ROLL_INDEX) << '\n'
    << "      focal_length: " << focal_length << '\n'
    << "      principal_point: " << principal_point(0) << ' '
                                 << principal_point(1) << '\n'
    << "      skew: " << skew << '\n'
    << "      aspect_ratio: " << aspect_ratio << '\n'
    << "      world_scale: " << world_scale << '\n'
    << "      near: " << rendering_interface.get_near() << '\n'
    << "      far: " << rendering_interface.get_far() << '\n';

}

/** This makes sure that the rendering interface is
 * consistent with the camera parameters
 */
void Perspective_camera::update_rendering_interface() const
{
    if(extrinsic_dirty)
    {
        Quaternion q(rotation_angles(CAMERA_PITCH_INDEX), rotation_angles(CAMERA_YAW_INDEX), rotation_angles(CAMERA_ROLL_INDEX), Quaternion::XYZR);
        rendering_interface.set_orientation(q);
        rendering_interface.set_camera_center(camera_centre);
        extrinsic_dirty = false;
    }

    if(intrinsic_dirty)
    {
        rendering_interface.set_intrinsic_parameters(focal_length, aspect_ratio, skew, principal_point(0), principal_point(1));
//        rendering_interface.update_intrinsic_row_2();
        intrinsic_dirty = false;
    }
}

/*
 * @param origin The world origin in camera coordinates, [x,y,z]. Optionally in homogeneous coordinates
 */
void Perspective_camera::set_world_origin(const kjb::Vector & iorigin)
{
    Vector origin = iorigin;

    if(origin.size() == 4)
    {
        origin /= origin(3);
        origin.resize(3, 1.0);
    }

    if(origin.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Invalid vector size.  must be 3 or 4.");
    }

    const Quaternion& orientation = get_orientation();
    set_camera_centre(-orientation.conj().rotate(origin));
}

/**
 * returns the world origin in camera coordinates as a 3-vector.
 */
Vector Perspective_camera::get_world_origin() const
{
    const Quaternion& orientation = get_orientation();
    return -orientation.rotate(Vector(camera_centre).resize(3));
}

/*
 * @param icentre The new camera centre position [x,y,z]. Optionally in homogeneous coordinates
 */
void Perspective_camera::set_camera_centre(const kjb::Vector & icentre)
{
    if(icentre.size() == 3)
    {
        for(unsigned int i = 0; i < 3; i++)
        {
            camera_centre(i) = icentre(i);
        }
        camera_centre(3) = 1.0;
    }
    else if(icentre.size() == 4)
    {
        double epsilon = 1e-127;
        if(icentre(3) < epsilon)
        {
            KJB_THROW_2(kjb::Illegal_argument, "Camera centre, the homogeneous coordinate is too close to zero");
        }
        camera_centre = icentre;
        camera_centre /= camera_centre(3);
    }
    else
    {
        KJB_THROW_2(kjb::Illegal_argument, "Camera centre must contain exactly three coordinates, or four if homogeneous");
    }
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param index Specifies which centre coordinate should be changed (0=x, 1=y, 2=z)
 * @param value The new value for the centre coordinate
 */
void Perspective_camera::set_camera_centre(unsigned int index, double ivalue)
{
    if(index > 2)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Camera centre vector index must be between 0 and 2");
    }
    camera_centre(index) = ivalue;
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param ix The new x coordinate of the camera centre
 */
void Perspective_camera::set_camera_centre_x(double ix)
{
    camera_centre(0) = ix;
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param iy The new y coordinate of the camera centre
 */
void Perspective_camera::set_camera_centre_y(double iy)
{
    camera_centre(1) = iy;
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param iz The new z coordinate of the camera centre
 */
void Perspective_camera::set_camera_centre_z(double iz)
{
    camera_centre(2) = iz;
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param ipitch The new pitch angle
 */
void Perspective_camera::set_pitch(double ipitch)
{
    rotation_angles(CAMERA_PITCH_INDEX) = ipitch;
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param iyaw The new yaw angle
 */
void Perspective_camera::set_yaw(double iyaw)
{
    rotation_angles(CAMERA_YAW_INDEX) = iyaw;
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param iroll The new roll angle
 */
void Perspective_camera::set_roll(double iroll)
{
    rotation_angles(CAMERA_ROLL_INDEX) = iroll;
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/**
 * Given 3 rotation angles, sets the corresponding Euler angles given the current Euler convention
 *
 * @param dpitch The amount of rotation arount the x-axis of this camera
 * @param dyaw   The amount of rotation arount the y-axis of this camera
 * @param droll  The amount of rotation arount the z-axis of this camera
 */
void Perspective_camera::compute_new_euler_angles_on_rotations(double dpitch, double dyaw, double droll)
{
    update_rendering_interface();
    rendering_interface.compute_new_euler_angles_on_rotations(dpitch, dyaw, droll, rotation_angles);
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}
/*
 * @param theta The amount of rotation to be done around the current x
 *              axis of the camera
 */
void Perspective_camera::rotate_around_x_axis(double theta)
{
    compute_new_euler_angles_on_rotations(theta, 0.0, 0.0);
}

/*
 * @param theta The amount of rotation to be done around the current y
 *              axis of the camera
 */
void Perspective_camera::rotate_around_y_axis(double theta)
{
    compute_new_euler_angles_on_rotations(0.0, theta, 0.0);
}

/*
 * @param theta The amount of rotation to be done around the current z
 *              axis of the camera
 */
void Perspective_camera::rotate_around_z_axis(double theta)
{
    compute_new_euler_angles_on_rotations(0.0, 0.0, theta);
}

/*
 * Order of rotation is: thetax, thetay, thetaz
 *
 * @param thetax The amount of rotation to be done around the current x
 *              axis of the camera
 * @param thetay The amount of rotation to be done around the current y
 *              axis of the camera
 * @param thetaz The amount of rotation to be done around the current z
 *              axis of the camera
 */
void Perspective_camera::rotate_around_camera_axes(double thetax, double thetay, double thetaz)
{
    rotate_around_x_axis(thetax);
    rotate_around_y_axis(thetay);
    rotate_around_z_axis(thetaz);
}

/*
 * Order of rotation is: pitch, yaw, roll
 * @param ipitch The new pitch angle
 * @param iyaw The new yaw angle
 * @param iroll The new roll angle
 */
void Perspective_camera::set_rotation_angles(double ipitch, double iyaw, double iroll)
{
    set_pitch(ipitch);
    set_yaw(iyaw);
    set_roll(iroll);
}

/*
 * @param ifocal The new focal length
 */
void Perspective_camera::set_focal_length(double ifocal)
{
    // Calibrated cameras _can_ have negative focal lengths, as the camera's x-axis needs to be flipped
    // in some sitatuions.  Commenting this out, for eventual deletion.  Kyle, Aug 18, 2010
//    if(ifocal < 0)
//    {
//        KJB_THROW_2(kjb::Illegal_argument, "Focal length for perspective camera cannot be negative");
//    }
    focal_length = ifocal;
    intrinsic_dirty = true;
    cam_matrix_dirty = true;
}

void Perspective_camera::update_focal_with_scale(double ifocal)
{
    double focal_ratio = ifocal/get_focal_length();
    focal_length = ifocal;
    world_scale = (1.0/focal_ratio)*world_scale;
    intrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param ip The new principal point [x,y]
 */
void Perspective_camera::set_principal_point(const kjb::Vector & ip)
{
    if(ip.size() != 2)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Principal point vector must contain exactly two coordinates");
    }
    principal_point = ip;
    intrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param index Specifies which coordinate of the principal point is to be modified
 * @param The new value for the principal point coordinate
 */
void Perspective_camera::set_principal_point(unsigned int index, double ip)
{
    if(index >= 2)
    {
        KJB_THROW_2(kjb::Illegal_argument, "Principal point vector index must be zero or one");
    }
    principal_point(index) = ip;
    intrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param ix The new x coordinate of the principal point
 */
void Perspective_camera::set_principal_point_x(double ix)
{
    principal_point(0) = ix;
    intrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param iy The new y coordinate of the principal point
 */
void Perspective_camera::set_principal_point_y(double iy)
{
    principal_point(1) = iy;
    intrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param skew The new skew angle in radian
 */
void Perspective_camera::set_skew(double is)
{
    skew = is;
    intrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param iar The new aspect ratio
 */
void Perspective_camera::set_aspect_ratio(double iar)
{
    // calibrated cameras sometimes have negative aspect ratios, representing a flipped y-axis.  Commenting this out for now, for eventual deletion. --Kyle Aug 18, 2010
//    if(iar <= 0)
//    {
//        KJB_THROW_2(kjb::Illegal_argument, "Aspect ratio for perspective camera must be positive");
//    }
    aspect_ratio = iar;
    intrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/*
 * @param iscale The new world scale
 */
void Perspective_camera::set_world_scale(double iscale)
{
    if(iscale <= 0)
    {
        KJB_THROW_2(kjb::Illegal_argument, "World scale for perspective camera must be positive");
    }
    world_scale = iscale;
}

/**
 * Sets the rotation angles from an input quaternion
 *
 * @param q the input quaternion
 */
void Perspective_camera::set_angles_from_quaternion(const kjb::Quaternion & q)
{
    KJB(UNTESTED_CODE());

    // need to convert euler mode so rotation angles are in pitch, yaw, roll
    Quaternion tmp = q;
    rotation_angles = tmp.set_euler_mode(Quaternion::XYZR).get_euler_angles();

    // save the quaternion in it's original euler mode, so Kyle's code works
    rendering_interface.set_orientation(q);
    extrinsic_dirty = true;
    cam_matrix_dirty = true;
}

/**
 * @param deyex Specifies the x position of the eye point.
 * @param deyey Specifies the y position of the eye point.
 * @param deyex Specifies the z position of the eye point.
 * @param dlookx Specifies the x position of the reference point.
 * @param dlooky Specifies the y position of the reference point.
 * @param dlookz Specifies the z position of the reference point.
 * @param dupx Specifies the x direction of the up vector.
 * @param dupy Specifies the y direction of the up vector.
 * @param dupz Specifies the z direction of the up vector.
 */
void Perspective_camera::set_look_at(double deyex, double deyey, double deyez, double dlookx, double dlooky, double dlookz,
        double dupx, double dupy, double dupz)
{
    Vector eye(deyex, deyey, deyez);
    Vector target(dlookx, dlooky, dlookz);
    Vector up(dupx, dupy, dupz);

    set_look_at(eye, target, up);
}

/**
 * @param eye Specifies the position of the eye point.
 * @param look Specifies the position of the reference point.
 * @param up Specifies the direction of the up vector.
 */
void Perspective_camera::set_look_at(const kjb::Vector & p_eye, const kjb::Vector & p_look, const kjb::Vector & p_up)
{
    Vector eye = p_eye;
    Vector target = p_look;
    Vector up = p_up;

    up.normalize();

    Vector look = target - eye;
    look.normalize();

    Vector s = cross(look, up);
    s.normalize();

    Vector u = cross(s, look);

    Matrix rot_mat(3, 3, 0.0);
    rot_mat(0, 0) = s(0);
    rot_mat(0, 1) = s(1);
    rot_mat(0, 2) = s(2);
    rot_mat(1, 0) = u(0);
    rot_mat(1, 1) = u(1);
    rot_mat(1, 2) = u(2);
    rot_mat(2, 0) = -look(0);
    rot_mat(2, 1) = -look(1);
    rot_mat(2, 2) = -look(2);

    Quaternion q;

    try {
        q.set_rotation_matrix(rot_mat);
    } catch (kjb::Illegal_argument& ex) {
        std::cout << ex.get_msg() << std::endl;
        abort();
    }

    q.set_euler_mode(Quaternion::XYZR);

    Vector orientation = q.get_euler_angles();
    double phi = orientation(0);
    double theta = orientation(1);
    double psi = orientation(2);

    set_camera_centre(eye);
    set_rotation_angles(phi, theta, psi);
}

/*
 * @param dx The amount of translation along the x axis
 * @param dy The amount of translation along the y axis
 * @param dz The amount of translation along the z axis
 * @param frame Specifies whether to translate along world or camera axes
 */
void Perspective_camera::translate(double dx, double dy, double dz, unsigned int frame )
{
    if(frame == FRAME_CAMERA_WORLD_COORDINATES)
    {
        Vector translation(4, 1.0);
        translation(0) = dx;
        translation(1) = dy;
        translation(2) = dz;
        rotate_point_to_camera_frame(translation);
        set_camera_centre_x(camera_centre(0) + translation(0));
        set_camera_centre_y(camera_centre(1) + translation(1));
        set_camera_centre_z(camera_centre(2) + translation(2));
    }
    else
    {
        set_camera_centre_x(camera_centre(0) + dx);
        set_camera_centre_y(camera_centre(1) + dy);
        set_camera_centre_z(camera_centre(2) + dz);
    }
}


/**
 * @param clean_buffers if true, the gl buffers will be cleared
 */
void Perspective_camera::prepare_for_rendering(bool clean_buffers) const
{
    update_rendering_interface();
    rendering_interface.prepare_for_rendering(clean_buffers);
    rendering_interface.set_world_scale(world_scale, world_scale, world_scale);
}


/**
 * Apply Tsai's radial distortion to point
 */
Vector Perspective_camera::radial_distortion(const Vector& v)
{
    const Matrix& P = get_camera_matrix();
    double fx = get_focal_length() * get_aspect_ratio();
    double fy = get_focal_length() / (sin(get_skew()));
    double cx = get_principal_point_x();
    double cy = get_principal_point_y();
    double dx = (v[0] - cx)/fx;
    double dy = (v[1] - cy)/fy;
    double dx2 = dx*dx;
    double dy2 = dy*dy;
    double dxdy = dx*dy;
    double r2 = dx2 + dy2;

    double x = dx * (1 + r2 * (m_rd_k1 + r2 * (m_rd_k2 + r2 * m_rd_k3)));
    double y = dy * (1 + r2 * (m_rd_k1 + r2 * (m_rd_k2 + r2 * m_rd_k3)));

    x += 2*m_rd_p1 * dxdy + m_rd_p2 * (r2 + 2*dx2); 
    y += 2*m_rd_p2 * dxdy + m_rd_p1 * (r2 + 2*dy2);

    return Vector(cx + fx*x, cy + fy*y);
}

} // namespace kjb


// allow calibrated cameras to be serialized through a pointer to a perspective_camera
#ifdef KJB_HAVE_BST_SERIAL
BOOST_CLASS_EXPORT(kjb::Perspective_camera);
#endif
