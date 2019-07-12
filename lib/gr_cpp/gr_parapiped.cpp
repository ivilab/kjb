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
|     Joseph Schlecht, Luca Del Pero
|
* =========================================================================== */

/**
 * @file
 *
 * @author Joseph Schlecht, Luca Del Pero
 *
 * @brief Parallelepiped: a hexahedron of which each face is a parallelegram.
 */


#include "l/l_sys_debug.h"  /* For ASSERT. */
/*
#include <cassert>
*/
#include <cmath>
#include <iostream>
#include <vector>

#include <inttypes.h>

#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"

#include "l_cpp/l_exception.h"
#include "gr_cpp/gr_polygon.h"
#include "gr_cpp/gr_parapiped.h"
#include "l/l_bits.h"


using namespace kjb;

/**
 * The mapping from four input points to the defined parallelepiped is as 
 * follows:
 *
 * @code
 *        +------------+                             p3
 *        |\           |\                            |\
 *        |  +------------+                          |  p4
 *        |  |         |  |                          |           y      
 *        |  |         |  |    ---->                 |           |      
 *        |  |         |  |                          |           +--- x 
 *        +--|---------+  |             p1-----------p2         /       
 *         \ |          \ |                                    z       
 *           +------------+
 * @endcode
 *
 * Upon initialization, each face of this parallelepiped is parallel with one of
 * the xy, xz, or yz-planes; the symetric pairs of faces are distinguished by
 * the sign of their outward directed normal vector. The order (mapping) of the
 * face indices is as follows
 *
 * @code
 *    i=0:  yz-plane, +x norm 
 *    i=1:  yz-plane, -x norm 
 *    i=2:  xz-plane, +y norm 
 *    i=3:  xz-plane, -y norm 
 *    i=4:  xy-plane, +z norm 
 *    i=5:  xy-plane, -z norm 
 * @endcode
 *
 * @param  x1  X-coord of the point 1.
 * @param  y1  Y-coord of the point 1.
 * @param  z1  Z-coord of the point 1.
 *
 * @param  x2  X-coord of the point 2.
 * @param  y2  Y-coord of the point 2.
 * @param  z2  Z-coord of the point 2.
 *
 * @param  x3  X-coord of the point 3.
 * @param  y3  Y-coord of the point 3.
 * @param  z3  Z-coord of the point 3.
 *
 * @param  x4  X-coord of the point 4.
 * @param  y4  Y-coord of the point 4.
 * @param  z4  Z-coord of the point 4.
 */
Parapiped::Parapiped
(
    double x1, double y1, double z1,
    double x2, double y2, double z2,
    double x3, double y3, double z3,
    double x4, double y4, double z4
)
: Polymesh(6), points(4)
{
    using namespace kjb;

    _faces[5].add_point(x1, y1, z1);
    _faces[5].add_point(x1 + (x3 - x2), y1 + (y3 - y2), z1 + (z3 - z2));
    _faces[5].add_point(x3, y3, z3);
    _faces[5].add_point(x2, y2, z2);

    _faces[3].add_point(x2, y2, z2);
    _faces[3].add_point(x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));
    _faces[3].add_point(x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[3].add_point(x1, y1, z1);

    _faces[0].add_point(x3, y3, z3);
    _faces[0].add_point(x4, y4, z4);
    _faces[0].add_point(x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));
    _faces[0].add_point(x2, y2, z2);

    _faces[2].add_point(x4, y4, z4);
    _faces[2].add_point(x3, y3, z3);
    _faces[2].add_point(x3 + (x1 - x2), y3 + (y1 - y2), z3 + (z1 - z2));
    _faces[2].add_point(x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));

    _faces[4].add_point(x4, y4, z4);
    _faces[4].add_point(x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));
    _faces[4].add_point(x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[4].add_point(x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));

    _faces[1].add_point(x1, y1, z1);
    _faces[1].add_point(x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[1].add_point(x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));
    _faces[1].add_point(x3 + (x1 - x2), y3 + (y1 - y2), z3 + (z1 - z2));

    points[0].zero_out(4);
    (points[0])[0] = x1;
    (points[0])[1] = y1;
    (points[0])[2] = z1;
    (points[0])[3] = 1;

    points[1].zero_out(4);
    (points[1])[0] = x2;
    (points[1])[1] = y2;
    (points[1])[2] = z2;
    (points[1])[3] = 1;

    points[2].zero_out(4);
    (points[2])[0] = x3;
    (points[2])[1] = y3;
    (points[2])[2] = z3;
    (points[2])[3] = 1;

    points[3].zero_out(4);
    (points[3])[0] = x4;
    (points[3])[1] = y4;
    (points[3])[2] = z4;
    (points[3])[3] = 1.0;

    center.zero_out(4);

    for (size_t i = 0; i < 6; i++)
    {
        const Vector& centroid = _faces[ i ].get_centroid();

        // Ignore the homogeneous coordinate, since it must be 1.
        center[0] += centroid[0];
        center[1] += centroid[1];
        center[2] += centroid[2];
    }

    center[0] /= 6.0;
    center[1] /= 6.0;
    center[2] /= 6.0;
    center[3]  = 1.0;

    create_adjacency_matrix();
}


/**
 * The mapping from four input points to the defined parallelepiped is as 
 * follows:
 *
 * @code
 *        +------------+                             p3
 *        |\           |\                            |\
 *        |  +------------+                          |  p4
 *        |  |         |  |                          |           y      
 *        |  |         |  |    ---->                 |           |      
 *        |  |         |  |                          |           +--- x 
 *        +--|---------+  |             p1-----------p2         /       
 *         \ |          \ |                                    z       
 *           +------------+
 * @endcode
 *
 * Upon initialization, each face of this parallelepiped is parallel with one of
 * the xy, xz, or yz-planes; the symetric pairs of faces are distinguished by
 * the sign of their outward directed normal vector. The order (mapping) of the
 * face indices is as follows
 *
 * @code
 *    i=0:  yz-plane, +x norm 
 *    i=1:  yz-plane, -x norm 
 *    i=2:  xz-plane, +y norm 
 *    i=3:  xz-plane, -y norm 
 *    i=4:  xy-plane, +z norm 
 *    i=5:  xy-plane, -z norm 
 * @endcode
 *
 * @param  p1  Point 1.
 * @param  p2  Point 2.
 * @param  p3  Point 3.
 * @param  p4  Point 4.
 *
 * @throw  kjb::Illegal_argument  The vectors are not 3D.
 */
Parapiped::Parapiped
(
    const kjb::Vector & p1,
    const kjb::Vector & p2,
    const kjb::Vector & p3,
    const kjb::Vector & p4
)
throw (kjb::Illegal_argument)
: Polymesh(6), points(4)
{
    using namespace kjb;

    if (p1.get_length() != 3 || p2.get_length() != 3 ||
        p3.get_length() != 3 || p4.get_length() != 3)
    {
        throw Illegal_argument("Vectors for parapiped not 3D");
    }

    double x1 = p1[0]; double y1 = p1[1]; double z1 = p1[2];
    double x2 = p2[0]; double y2 = p2[1]; double z2 = p2[2];
    double x3 = p3[0]; double y3 = p3[1]; double z3 = p3[2];
    double x4 = p4[0]; double y4 = p4[1]; double z4 = p4[2];

    _faces[5].add_point(x1, y1, z1);
    _faces[5].add_point(x1 + (x3 - x2), y1 + (y3 - y2), z1 + (z3 - z2));
    _faces[5].add_point(x3, y3, z3);
    _faces[5].add_point(x2, y2, z2);

    _faces[3].add_point(x2, y2, z2);
    _faces[3].add_point(x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));
    _faces[3].add_point(x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[3].add_point(x1, y1, z1);

    _faces[0].add_point(x3, y3, z3);
    _faces[0].add_point(x4, y4, z4);
    _faces[0].add_point(x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));
    _faces[0].add_point(x2, y2, z2);

    _faces[2].add_point(x4, y4, z4);
    _faces[2].add_point(x3, y3, z3);
    _faces[2].add_point(x3 + (x1 - x2), y3 + (y1 - y2), z3 + (z1 - z2));
    _faces[2].add_point(x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));

    _faces[4].add_point(x4, y4, z4);
    _faces[4].add_point(x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));
    _faces[4].add_point(x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[4].add_point(x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));

    _faces[1].add_point(x1, y1, z1);
    _faces[1].add_point(x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[1].add_point(x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));
    _faces[1].add_point(x3 + (x1 - x2), y3 + (y1 - y2), z3 + (z1 - z2));


    points[0].zero_out(4);
    (points[0])[0] = x1;
    (points[0])[1] = y1;
    (points[0])[2] = z1;
    (points[0])[3] = 1;

    points[1].zero_out(4);
    (points[1])[0] = x2;
    (points[1])[1] = y2;
    (points[1])[2] = z2;
    (points[1])[3] = 1;

    points[2].zero_out(4);
    (points[2])[0] = x3;
    (points[2])[1] = y3;
    (points[2])[2] = z3;
    (points[2])[3] = 1;

    points[3].zero_out(4);
    (points[3])[0] = x4;
    (points[3])[1] = y4;
    (points[3])[2] = z4;
    (points[3])[3] = 1;

    center.zero_out(4);

    for (size_t i = 0; i < 6; i++)
    {
        const Vector & centroid = _faces[ i ].get_centroid();

        // Ignore the homogeneous coordinate, since it must be 1.
        center[0] += centroid[0];
        center[1] += centroid[1];
        center[2] += centroid[2];
    }

    center[0] /= 6.0;
    center[1] /= 6.0;
    center[2] /= 6.0;
    center[3]  = 1.0;

    create_adjacency_matrix();
}


/** @param  p  Parallelepiped to copy into this one. */
Parapiped::Parapiped(const Parapiped& p)
: Polymesh(p), points(p.points), center(p.center), _adjacency(p._adjacency)
{

}


/**
 * The file format is big-endian binary.
 *
 * @param  fname  Input file to read this parallelepiped from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid arguments in file to read from.
 */
Parapiped::Parapiped(const char* fname)
    throw (kjb::Illegal_argument, kjb::IO_error)
    : Polymesh(), points(4)
{
    kjb::Readable::read(fname);
}


/**
 * The file format is big-endian binary.
 *
 * @param  in  Input stream to read this parallelepiped from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid arguments in file to read from.
 */
Parapiped::Parapiped(std::istream& in)
    throw (kjb::Illegal_argument, kjb::IO_error)
    : Polymesh(), points(4)
{
    read(in);
}


/** 
 * Performs a deep copy of the faces in the parallelepiped.
 *
 * @param  p  Parallelepiped to copy into this one.
 *
 * @return A reference to this parallelepiped.
 */
Parapiped& Parapiped::operator= (const Parapiped& p)
{
    Polymesh::operator=(p);

    points = p.points;
    center = p.center;
    _adjacency = p._adjacency;

    return *this;
}


/** @return A new copy of this parallelepiped. */
Parapiped* Parapiped::clone() const
{
    return new Parapiped(*this);
}


/** @return The ith point of this parapiped using the
 * convention described in the constructor
 *
 * @param i the index of the point to be returned
 */
const kjb::Vector & Parapiped::get_point(size_t i) const
    throw (kjb::Illegal_argument)
{
    if (i >= 4)
    { 
        throw kjb::Illegal_argument("Invalid point index");
    }

    return points[i];
}


/** 
 * The vector is in homogeneous coordinates x, y, z, w. The coordinate
 * w is always 1.
 */
const kjb::Vector &Parapiped::get_center() const
{ 
    return center; 
}


/** 
 * @param  in  Input stream to read the members of this parallelepiped from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid argument to read from file.
 */
void Parapiped::read(std::istream& in) throw (kjb::IO_error,
        kjb::Illegal_argument)
{
    using namespace kjb_c;

    Polymesh::read(in);

    ASSERT(sizeof(double) == sizeof(uint64_t));

    double x, y, z, w;

    for (size_t i = 0; i < 4; i++)
    {
        in.read((char*)&x, sizeof(double));
        in.read((char*)&y, sizeof(double));
        in.read((char*)&z, sizeof(double));
        in.read((char*)&w, sizeof(double));
        if (in.fail() || in.eof())
        {
            throw IO_error("Could not read point");
        }

        if(!kjb_is_bigendian())
        {
            bswap_u64((uint64_t*)&(x));
            bswap_u64((uint64_t*)&(y));
            bswap_u64((uint64_t*)&(z));
            bswap_u64((uint64_t*)&(w));
        }

        points[i].zero_out(4);
        (points[ i ])[0] = x;
        (points[ i ])[1] = y;
        (points[ i ])[2] = z;
        (points[ i ])[3] = w;
    }

    in.read((char*)&x, sizeof(double));
    in.read((char*)&y, sizeof(double));
    in.read((char*)&z, sizeof(double));
    in.read((char*)&w, sizeof(double));
    if (in.fail() || in.eof())
    {
        throw IO_error("Could not read center");
    }

    if(!kjb_is_bigendian())
    {
        bswap_u64((uint64_t*)&(x));
        bswap_u64((uint64_t*)&(y));
        bswap_u64((uint64_t*)&(z));
        bswap_u64((uint64_t*)&(w));
    }

    center.zero_out(4);
    center[0] = x;
    center[1] = y;
    center[2] = z;
    center[3] = w;

    create_adjacency_matrix();
}


/** 
 * The file format is big-endian binary.
 *
 * @param  out  Output stream to write the members of this parallelepiped to.
 *
 * @throw  kjb::IO_error  Could not write to @em out.
 */
void Parapiped::write(std::ostream& out) const throw (kjb::IO_error)
{
    using namespace kjb_c;

    Polymesh::write(out);

    ASSERT(sizeof(double) == sizeof(uint64_t));

    for (size_t i = 0; i < 4; i++)
    {
        double x = (points[ i ])[0];
        double y = (points[ i ])[1];
        double z = (points[ i ])[2];
        double w = (points[ i ])[3];

        if(!kjb_is_bigendian())
        {
            bswap_u64((uint64_t*)&(x));
            bswap_u64((uint64_t*)&(y));
            bswap_u64((uint64_t*)&(z));
            bswap_u64((uint64_t*)&(w));
        }

        out.write((char*)&x, sizeof(double));
        out.write((char*)&y, sizeof(double));
        out.write((char*)&z, sizeof(double));
        out.write((char*)&w, sizeof(double));
        if (out.fail() || out.eof())
        {
            throw IO_error("Could not write point");
        }
    }

    double x = center[0];
    double y = center[1];
    double z = center[2];
    double w = center[3];

    if(!kjb_is_bigendian())
    {
        bswap_u64((uint64_t*)&(x));
        bswap_u64((uint64_t*)&(y));
        bswap_u64((uint64_t*)&(z));
        bswap_u64((uint64_t*)&(w));
    }

    out.write((char*)&x, sizeof(double));
    out.write((char*)&y, sizeof(double));
    out.write((char*)&z, sizeof(double));
    out.write((char*)&w, sizeof(double));
    if (out.fail() || out.eof())
    {
        throw IO_error("Could not write center");
    }
}


/**
 * @param  M  Homogeneous transformation matrix to transform this
 *            parallelepiped by.
 *
 * @throw  kjb::Illegal_argument  The matrix is not in 4x4 homogeneous
 *                                  coordinates.
 */
void Parapiped::transform(const kjb::Matrix & M)
    throw (kjb::Illegal_argument)
{
    Polymesh::transform(M);

    for (size_t i = 0; i < 4; i++)
    {
        points[i] = M*points[i];
    }

    // Transform the center.
    center = M*center;
}

/** Resets the points of this parapiped
 *
 * @param  x1  X-coord of the point 1.
 * @param  y1  Y-coord of the point 1.
 * @param  z1  Z-coord of the point 1.
 *
 * @param  x2  X-coord of the point 2.
 * @param  y2  Y-coord of the point 2.
 * @param  z2  Z-coord of the point 2.
 *
 * @param  x3  X-coord of the point 3.
 * @param  y3  Y-coord of the point 3.
 * @param  z3  Z-coord of the point 3.
 *
 * @param  x4  X-coord of the point 4.
 * @param  y4  Y-coord of the point 4.
 * @param  z4  Z-coord of the point 4.
 */
void Parapiped::set_points(
        double x1, double y1, double z1,
        double x2, double y2, double z2,
        double x3, double y3, double z3,
        double x4, double y4, double z4)
{
    using namespace kjb;

    _faces[5].set_point(0, x1, y1, z1);
    _faces[5].set_point(1, x1 + (x3 - x2), y1 + (y3 - y2), z1 + (z3 - z2));
    _faces[5].set_point(2, x3, y3, z3);
    _faces[5].set_point(3, x2, y2, z2);

    _faces[3].set_point(0, x2, y2, z2);
    _faces[3].set_point(1, x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));
    _faces[3].set_point(2, x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[3].set_point(3, x1, y1, z1);

    _faces[0].set_point(0, x3, y3, z3);
    _faces[0].set_point(1, x4, y4, z4);
    _faces[0].set_point(2, x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));
    _faces[0].set_point(3, x2, y2, z2);

    _faces[2].set_point(0, x4, y4, z4);
    _faces[2].set_point(1, x3, y3, z3);
    _faces[2].set_point(2, x3 + (x1 - x2), y3 + (y1 - y2), z3 + (z1 - z2));
    _faces[2].set_point(3, x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));

    _faces[4].set_point(0, x4, y4, z4);
    _faces[4].set_point(1, x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));
    _faces[4].set_point(2, x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[4].set_point(3, x2 + (x4 - x3), y2 + (y4 - y3), z2 + (z4 - z3));

    _faces[1].set_point(0, x1, y1, z1);
    _faces[1].set_point(1, x1 + (x4 - x3), y1 + (y4 - y3), z1 + (z4 - z3));
    _faces[1].set_point(2, x4 + (x1 - x2), y4 + (y1 - y2), z4 + (z1 - z2));
    _faces[1].set_point(3, x3 + (x1 - x2), y3 + (y1 - y2), z3 + (z1 - z2));

    for(unsigned int i = 0; i < 6; i++)
    {
        _faces[i].update();
    }

    (points[0])[0] = x1;
    (points[0])[1] = y1;
    (points[0])[2]=  z1;

    (points[1])[0] = x2;
    (points[1])[1] = y2;
    (points[1])[2] = z2;

    (points[2])[0] = x3;
    (points[2])[1] = y3;
    (points[2])[2] = z3;

    (points[3])[0] = x4;
    (points[3])[1] = y4;
    (points[3])[2] = z4;


    center[0] = 0;
    center[1] = 0;
    center[2] = 0;

    for (size_t i = 0; i < 6; i++)
    {
        const Vector & centroid = _faces[ i ].get_centroid();

        // Ignore the homogeneous coordinate, since it must be 1.
        center[0] += centroid[0];
        center[1] += centroid[1];
        center[2] += centroid[2];
    }

    center[0] /= 6.0;
    center[1] /= 6.0;
    center[2] /= 6.0;
    center[3]  = 1.0;
}

/**
 * @param f we want to find the face adjacent to this face f along edge e
 * @param e the edge along which to look for the adjacent face
 * @return the index of the face adjacent to face f along edge e
 *
 *  This is an efficient implementation to be used only in the context
 *  of the parallelepiped
 */
unsigned int Parapiped::adjacent_face(unsigned int f, unsigned int e) const
               throw (Index_out_of_bounds,KJB_error)
{
    return _adjacency(f, e);
}

/**
 * It creates the adjacency matrix for this parapiped
 */
void Parapiped::create_adjacency_matrix()
{
    _adjacency.zero_out(6,4);
    _adjacency(5,0) = 1;
    _adjacency(5,1) = 2;
    _adjacency(5,2) = 0;
    _adjacency(5,3) = 3;

    _adjacency(3,0) = 0;
    _adjacency(3,1) = 4;
    _adjacency(3,2) = 1;
    _adjacency(3,3) = 5;

    _adjacency(0,0) = 2;
    _adjacency(0,1) = 4;
    _adjacency(0,2) = 3;
    _adjacency(0,3) = 5;

    _adjacency(2,0) = 0;
    _adjacency(2,1) = 5;
    _adjacency(2,2) = 1;
    _adjacency(2,3) = 4;

    _adjacency(4,0) = 2;
    _adjacency(4,1) = 1;
    _adjacency(4,2) = 3;
    _adjacency(4,3) = 0;

    _adjacency(1,0) = 3;
    _adjacency(1,1) = 4;
    _adjacency(1,2) = 2;
    _adjacency(1,3) = 5;
}

/**
 * Adds a face to this parapiped -> Not implemented here,
 * it will throw an exception, because the use of this method
 * will violate the constraints defining this parapiped
 *
 * @param face The face to be added
 */
void Parapiped::add_face(const Polygon & /* face */) throw (kjb::Illegal_argument)
{
    KJB_THROW(Not_implemented);
}

void Parapiped::draw_orientation_map() const
{
    GL_Polymesh_Renderer::draw_orientation_map(*this);
}

void Parapiped::draw_left_right_orientation_map() const
{
    GL_Polymesh_Renderer::draw_left_right_orientation_map(*this);
}

void Parapiped::draw_CMU_orientation_map() const
{
    GL_Polymesh_Renderer::draw_CMU_orientation_map(*this);
}

void Parapiped::draw_geometric_context_map() const
{
    GL_Polymesh_Renderer::draw_geometric_context_map(*this);
}
