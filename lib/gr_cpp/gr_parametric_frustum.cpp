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
|     Joseph Schlecht, Luca Del Pero, Yuanliu Liu
|
* =========================================================================== */

/**
 * @file
 *
 * @author Yuanliu Liu
 *
 * @brief Frustum: a polyhedron of which each torso face is a trapezoid and the top and bottom surfaces are polygons.
 */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "gr_cpp/gr_parametric_frustum.h"

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
#include "gr_cpp/gr_frustum.h"
#include "l/l_bits.h"

#define PI 3.1415926

using namespace kjb;

/**
 * The mapping from the input center point together with the height, top radius
 * and bottom radius to the defined frustum
 *
 * Upon initialization, the top face and the bottom face are parallel to the
 * xz-planes. Each of these two faces has nv vertices, indexed by 0:nv-1 and
 * nv:2nv-1, respectively. The vertices are placed in the clockwise order, while
 * the 0-th vertice is placed on the +x axis. 
 *
 * @code
 *         y                    |   nv-2
 *         |                    |    nv-1
 *         +---x           -----+-----0-> x
 *        /                     |    1   
 *       z                      |   2 
 *                              V    
 *                              z
 * @endcode
 *
 * The torso faces are formed by connecting the i-th
 * vertice to the (i+nv)-th vertice, i belongs to 0:nv-1. Each torso face has 4
 * vertices, which are (i, (i+1)%nv, (i+1)%nv+nv, i+nv). 
 *
 * The indexes of the faces: 
 * @code
 *    j=0:      top surface, xz-plane, +y norm 
 *    j=1:      bottom surface, xz-plane, -y norm 
 *    j=2:nv+1: torso faces with vertices (j-2,(j-2+1)%nv,(j-2+1)%nv+nv,j-2+nv)  
 * @endcode
 *
 * @param  nv  number of vertices of the top surface.
 * @param  ix  X-coord of the center point.
 * @param  iy  Y-coord of the center point.
 * @param  iz  Z-coord of the center point.
 *
 * @param  iw  diameter of the bottom face along the x-axis.
 * @param  il  diameter of the bottom face along the z-axis.
 * @param  iratio_top_bottom ratio of the diameters of the top and bottom face
 * @param  ih   height of the frustum.
 *
 * @param  ipitch  pitch angle.
 * @param  iyaw    yaw angle.
 * @param  iroll   roll angle.
 */
Frustum::Frustum
(
    unsigned int inv,
	double ix, double iy, double iz,
	double iw, double il, double iratio_top_bottom, double ih,
	double ipitch, double iyaw, double iroll
)
: Polymesh(inv+2), points(inv*2)
{
    using namespace kjb;

	// set the number of vertices
	nv = inv;

	// set the bottom n vertices
	double bottom_center_y = iy - ih/2;
	double bottom_centre_x = ix;
	double bottom_centre_z = iz;
    double bottom_rx = iw/2;
    double bottom_rz = il/2;

	for (unsigned int i = inv; i<2*inv; i++)
	{
		double theta = (i-inv)*2*PI/inv;
        double sign_x = 1;
        if ((PI/2< theta) && (theta < PI*3/2) )
        {
            sign_x = -1;
        }
		points[i].zero_out(4);
		(points[i])[0] =
        bottom_centre_x+sign_x*1.0/sqrt(1.0/pow(bottom_rx,2)+1.0/pow(bottom_rz,2)*pow(tan(theta),2));
		(points[i])[1] = bottom_center_y;
		(points[i])[2] = bottom_centre_z+(points[i])[0]*tan(theta);
		(points[i])[3] = 1;
	}

	// set the top n vertices
	double top_center_y = iy + ih/2;
	double top_centre_x = ix;
	double top_centre_z = iz;
    double top_rx = bottom_rx*iratio_top_bottom;
    double top_rz = bottom_rz*iratio_top_bottom;

	for (unsigned int i = 0; i<inv; i++)
	{
		double theta = i*2*PI/inv;
        double sign_x = 1;
        if ((PI/2< theta) && (theta < PI*3/2) )
        {
            sign_x = -1;
        }
		points[i].zero_out(4);
		(points[i])[0] =
        top_centre_x+sign_x*1.0/sqrt(1.0/pow(top_rx,2)+1.0/pow(top_rz,2)*pow(tan(theta),2));
        (points[i])[1] = top_center_y;
		(points[i])[2] = top_centre_z+(points[i])[0]*tan(theta);
        (points[i])[3] = 1;
	}

    ////////////////////////// rotate the vertices

	/* * set the vertices of the faces0
	*/
	// the top surface
	for (unsigned int i = 0; i<inv; i++)
	{
		_faces[0].add_point((points[i])[0], (points[i])[1], (points[i])[2]);
	}

	// the bottom surface
	for (unsigned int i = 0; i<inv; i++)
	{
		_faces[1].add_point((points[i+inv])[0], (points[i+inv])[1], (points[i+inv])[2]);
	}

	// the surfaces of the profile
	for (unsigned int i = 0; i<inv-1; i++)
	{
		_faces[i+2].add_point((points[i])[0], (points[i])[1], (points[i])[2]);
		_faces[i+2].add_point((points[i+1])[0], (points[i+1])[1], (points[i+1])[2]);
		_faces[i+2].add_point((points[i+inv+1])[0], (points[i+inv+1])[1], (points[i+inv+1])[2]);
		_faces[i+2].add_point((points[i+inv])[0], (points[i+inv])[1], (points[i+inv])[2]);
	}
	_faces[inv+1].add_point((points[inv-1])[0], (points[inv-1])[1], (points[inv-1])[2]);
	_faces[inv+1].add_point((points[0])[0], (points[0])[1], (points[0])[2]);
	_faces[inv+1].add_point((points[inv])[0], (points[inv])[1], (points[inv])[2]);
	_faces[inv+1].add_point((points[2*inv-1])[0], (points[2*inv-1])[1], (points[2*inv-1])[2]);

    center.zero_out(4);

    for (size_t i = 0; i < inv+2; i++)
    {
        const Vector& centroid = _faces[ i ].get_centroid();

        // Ignore the homogeneous coordinate, since it must be 1.
        center[0] += centroid[0];
        center[1] += centroid[1];
        center[2] += centroid[2];
    }

    center[0] /= inv+2.0;
    center[1] /= inv+2.0;
    center[2] /= inv+2.0;
    center[3]  = 1.0;

    create_adjacency_matrix();
}


/**
* The mapping from 2*nv points to the defined parallelepiped, while those
 * points should come from the vertices of a frustum
 *
 * @param  p  the vector of vertices.
 *
 * @throw  kjb::Illegal_argument  The vectors are not 3D.
 */
Frustum::Frustum
(
    const std::vector<kjb::Vector> & p
)
throw (kjb::Illegal_argument)
: Polymesh(6), points(p.size())
{
    using namespace kjb;

	nv = p.size()/2;
	unsigned int inv = nv;

    points = p;

	/* * set the vertices of the faces
	*/
	// the top surface
	for (unsigned int i = 0; i<inv; i++)
	{
		_faces[0].add_point((points[i])[0], (points[i])[1], (points[i])[2]);
	}

	// the bottom surface
	for (unsigned int i = 0; i<inv; i++)
	{
		_faces[1].add_point((points[i+inv])[0], (points[i+inv])[1], (points[i+inv])[2]);
	}

	// the surfaces of the profile
	for (unsigned int i = 0; i<inv-1; i++)
	{
		_faces[i+2].add_point((points[i])[0], (points[i])[1], (points[i])[2]);
		_faces[i+2].add_point((points[i+1])[0], (points[i+1])[1], (points[i+1])[2]);
		_faces[i+2].add_point((points[i+inv+1])[0], (points[i+inv+1])[1], (points[i+inv+1])[2]);
		_faces[i+2].add_point((points[i+inv])[0], (points[i+inv])[1], (points[i+inv])[2]);
	}
	_faces[inv+1].add_point((points[inv-1])[0], (points[inv-1])[1], (points[inv-1])[2]);
	_faces[inv+1].add_point((points[0])[0], (points[0])[1], (points[0])[2]);
	_faces[inv+1].add_point((points[2*inv-1])[0], (points[2*inv-1])[1], (points[2*inv-1])[2]);
	_faces[inv+1].add_point((points[inv])[0], (points[inv])[1], (points[inv])[2]);

    center.zero_out(4);

    for (size_t i = 0; i < inv+2; i++)
    {
        const Vector& centroid = _faces[ i ].get_centroid();

        // Ignore the homogeneous coordinate, since it must be 1.
        center[0] += centroid[0];
        center[1] += centroid[1];
        center[2] += centroid[2];
    }

    center[0] /= inv+2.0;
    center[1] /= inv+2.0;
    center[2] /= inv+2.0;
    center[3]  = 1.0;

    create_adjacency_matrix();
}


/** @param  p  Frustum to copy into this one. */
Frustum::Frustum(const Frustum& p)
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
Frustum::Frustum(const char* fname, unsigned int inv)
    throw (kjb::Illegal_argument, kjb::IO_error)
    : Polymesh(), points(inv*2)
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
Frustum::Frustum(std::istream& in, unsigned int inv)
    throw (kjb::Illegal_argument, kjb::IO_error)
    : Polymesh(), nv(inv), points(inv*2)
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
Frustum& Frustum::operator= (const Frustum& p)
{
    Polymesh::operator=(p);

    points = p.points;
    center = p.center;
    _adjacency = p._adjacency;

    return *this;
}


/** @return A new copy of this frustum. */
Frustum* Frustum::clone() const
{
    return new Frustum(*this);
}


/** @return The ith point of this frustum using the
 * convention described in the constructor
 *
 * @param i the index of the point to be returned
 */
const kjb::Vector & Frustum::get_point(size_t i) const
    throw (kjb::Illegal_argument)
{
    if (i >= nv*2)
    { 
        throw kjb::Illegal_argument("Invalid point index");
    }

    return points[i];
}


/** 
 * The vector is in homogeneous coordinates x, y, z, w. The coordinate
 * w is always 1.
 */
const kjb::Vector &Frustum::get_center() const
{ 
    return center; 
}


/** 
 * @param  in  Input stream to read the members of this frustum from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid argument to read from file.
 */
void Frustum::read(std::istream& in) throw (kjb::IO_error,
        kjb::Illegal_argument)
{
    using namespace kjb_c;

    Polymesh::read(in);

    ASSERT(sizeof(double) == sizeof(uint64_t));

    double x, y, z, w;

    for (size_t i = 0; i < nv*2; i++)
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
void Frustum::write(std::ostream& out) const throw (kjb::IO_error)
{
    using namespace kjb_c;

    Polymesh::write(out);

    ASSERT(sizeof(double) == sizeof(uint64_t));

    for (size_t i = 0; i < nv*2; i++)
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
 *            frustum by.
 *
 * @throw  kjb::Illegal_argument  The matrix is not in 4x4 homogeneous
 *                                  coordinates.
 */
void Frustum::transform(const kjb::Matrix & M)
    throw (kjb::Illegal_argument)
{
    Polymesh::transform(M);

    for (size_t i = 0; i < 2*nv; i++)
    {
        points[i] = M*points[i];
    }

    // Transform the center.
    center = M*center;
}

/** Resets the points of this frustum
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
void Frustum::set_points(
    unsigned int inv,
	double ix, double iy, double iz,
	double iw, double il, double iratio_top_bottom, double ih)
{
    using namespace kjb;

    // set the number of vertices of top face
    nv = inv;

    // set the bottom n vertices
	double bottom_center_y = iy - ih/2;
	double bottom_centre_x = ix;
	double bottom_centre_z = iz;
    double bottom_rx = iw/2;
    double bottom_rz = il/2;

	for (unsigned int i = inv; i<2*inv; i++)
	{
		double theta = (i-inv)*2*PI/inv;
        double sign_x = 1;
        if ((PI/2< theta) && (theta < PI*3/2) )
        {
            sign_x = -1;
        }
		points[i].zero_out(4);
		(points[i])[0] =
        bottom_centre_x+sign_x*1.0/sqrt(1.0/pow(bottom_rx,2)+1.0/pow(bottom_rz,2)*pow(tan(theta),2));
		(points[i])[1] = bottom_center_y;
		(points[i])[2] = bottom_centre_z+(points[i])[0]*tan(theta);
		(points[i])[3] = 1;
	}

	// set the top n vertices
	double top_center_y = iy + ih/2;
	double top_centre_x = ix;
	double top_centre_z = iz;
    double top_rx = bottom_rx*iratio_top_bottom;
    double top_rz = bottom_rz*iratio_top_bottom;

	for (unsigned int i = 0; i<inv; i++)
	{
		double theta = i*2*PI/inv;
        double sign_x = 1;
        if ((PI/2< theta) && (theta < PI*3/2) )
        {
            sign_x = -1;
        }
		points[i].zero_out(4);
		(points[i])[0] =
        top_centre_x+sign_x*1.0/sqrt(1.0/pow(top_rx,2)+1.0/pow(top_rz,2)*pow(tan(theta),2));
        (points[i])[1] = top_center_y;
		(points[i])[2] = top_centre_z+(points[i])[0]*tan(theta);
        (points[i])[3] = 1;
	}


	////////////////////////// rotate the vertices

	/* * set the vertices of the faces
	*/
	// the top surface
	for (unsigned int i = 0; i<inv; i++)
	{
		_faces[0].set_point(i,(points[i])[0], (points[i])[1], (points[i])[2]);
	}

	// the bottom surface
	for (unsigned int i = 0; i<inv; i++)
	{
		_faces[1].set_point(i,(points[i+inv])[0], (points[i+inv])[1], (points[i+inv])[2]);
	}

	// the surfaces of the profile
	for (unsigned int i = 0; i<inv-1; i++)
	{
		_faces[i+2].set_point(0,(points[i])[0], (points[i])[1], (points[i])[2]);
		_faces[i+2].set_point(1,(points[i+1])[0], (points[i+1])[1], (points[i+1])[2]);
		_faces[i+2].set_point(2,(points[i+inv+1])[0], (points[i+inv+1])[1], (points[i+inv+1])[2]);
		_faces[i+2].set_point(3,(points[i+inv])[0], (points[i+inv])[1], (points[i+inv])[2]);
	}
	_faces[inv+1].set_point(0,(points[inv-1])[0], (points[inv-1])[1], (points[inv-1])[2]);
	_faces[inv+1].set_point(1,(points[0])[0], (points[0])[1], (points[0])[2]);
	_faces[inv+1].set_point(2,(points[inv])[0], (points[inv])[1], (points[inv])[2]);
	_faces[inv+1].set_point(3,(points[2*inv-1])[0], (points[2*inv-1])[1], (points[2*inv-1])[2]);

	for(unsigned int i = 0; i < inv+2; i++)
    {
        _faces[i].update();
    }

    center.zero_out(4);

    for (size_t i = 0; i < inv+2; i++)
    {
        const Vector& centroid = _faces[ i ].get_centroid();

        // Ignore the homogeneous coordinate, since it must be 1.
        center[0] += centroid[0];
        center[1] += centroid[1];
        center[2] += centroid[2];
    }

    center[0] /= inv+2.0;
    center[1] /= inv+2.0;
    center[2] /= inv+2.0;
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
unsigned int Frustum::adjacent_face(unsigned int f, unsigned int e) const
               throw (Index_out_of_bounds,KJB_error)
{
    return _adjacency(f, e);
}

/**
 * It creates the adjacency matrix for this frustum
 */
void Frustum::create_adjacency_matrix()
{
    _adjacency.zero_out(nv+2,nv);

	/* *
		the top and bottom surface each has nv adjacent surfaces
		while the others each has 4 adjacent surfaces
	*/
	for (unsigned int i = 0; i<nv; i++)
	{
	    _adjacency(0,i) = i+2;
		_adjacency(1,i) = i+2;
	}

	for (unsigned int i = 2; i<nv+2; i++)
	{
	    _adjacency(i,0) = 0;
		_adjacency(i,1) = (i+1-2)%nv+2;
		_adjacency(i,2) = 1;
		_adjacency(i,3) = (i-1-2)%nv+2;
		for (unsigned int j = 4; j<nv; j++)
		{
			_adjacency(i,j) = -1;
		}
	}
}

/**
 * Adds a face to this frustum -> Not implemented here,
 * it will throw an exception, because the use of this method
 * will violate the constraints defining this frustum
 *
 * @param face The face to be added
 */
void Frustum::add_face(const Polygon & /* face */) throw (kjb::Illegal_argument)
{
    KJB_THROW(Not_implemented);
}

/*void Frustum::draw_orientation_map() const
{
    GL_Polymesh_Renderer::draw_orientation_map(*this);
}

void Frustum::draw_left_right_orientation_map() const
{
    GL_Polymesh_Renderer::draw_left_right_orientation_map(*this);
}

void Frustum::draw_CMU_orientation_map() const
{
    GL_Polymesh_Renderer::draw_CMU_orientation_map(*this);
}

void Frustum::draw_geometric_context_map() const
{
    GL_Polymesh_Renderer::draw_geometric_context_map(*this);
}*/
