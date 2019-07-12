/* $Id$ */
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
 * @author Luca Del Pero
 *
 * @brief Polymesh: a generic polygonal mesh
 */

/*
#include <cassert>
*/
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>

#include <inttypes.h>

#include "l/l_bits.h"
#include "l_cpp/l_exception.h"
#include "gr_cpp/gr_polygon.h"
#include "gr_cpp/gr_polymesh.h"
#include "gr_cpp/gr_polymesh_renderer.h"
#include "gr_cpp/gr_camera.h"

using namespace kjb;

/**
 * @param n the number of polygons
 * This creates a polymesh containing n polygons
 */
Polymesh::Polymesh(unsigned int n)
: Abstract_renderable(),
  Rigid_object(), Readable(), Writeable(),
  _faces(n, Polygon())
{

}

/**
 * The file format is big-endian binary.
 *
 * @param  fname  Input file to read this mesh from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid arguments in file to read from.
 */
Polymesh::Polymesh(const char* fname) throw (Illegal_argument, IO_error)
:  Abstract_renderable(),
   Rigid_object(), Readable(), Writeable()
{
    Readable::read(fname);
}

/**
 * The file format is big-endian binary.
 *
 * @param  in  Input stream to read this mesh from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid arguments in file to read from.
 */
Polymesh::Polymesh(std::istream& in) throw (Illegal_argument, IO_error)
:  Abstract_renderable(),
   Rigid_object(), Readable(), Writeable()
{
    read(in);
}

/** @param  t  Mesh to copy into this one. */
Polymesh::Polymesh(const Polymesh& src)
:  Abstract_renderable(src),
   Rigid_object(), Readable(), Writeable(),
   _faces(src._faces)
{

}

/** 
 * Performs a deep copy of the faces in the parallelepiped.
 *
 * @param  t  Triangular mesh to copy into this one.
 *
 * @return A reference to this mesh
 */
Polymesh& Polymesh::operator= (const Polymesh& src)
{
    Abstract_renderable::operator=(src);
    Rigid_object::operator=(src);
    _faces.clear();
    
    for(unsigned int i = 0; i < src.num_faces(); i++)
    {
        _faces.push_back(src._faces[i]);
    }

    return *this;
}


/** @return A new copy of this polymesh. */
Polymesh* Polymesh::clone() const
{
    return new Polymesh(*this);
}

/** Frees all space allocated by this mesh. */
Polymesh::~Polymesh()
{

}


/** 
 * @param  i  Index of the face to get.
 *
 * @return The specified mesh face.
 *
 * @throw  kjb::Illegal_argument  Invalid face index.
 */
const Polygon& Polymesh::get_face(unsigned int i) const throw (Index_out_of_bounds)
{ 
    if(i >= num_faces())
    { 
        throw Index_out_of_bounds("Invalid face index");
    }

    return _faces[i];
}


/**
 * @param  i  Index of the face to get.
 *
 * @return The specified mesh face.
 *
 * @throw  kjb::Illegal_argument  Invalid face index.
 */
Polygon& Polymesh::get_face_ref(unsigned int i) throw (Index_out_of_bounds)
{
    if(i >= num_faces())
    {
        throw Index_out_of_bounds("Invalid face index");
    }

    return _faces[i];
}

/** @brief Returns the faces of this mesh. */
const std::vector<kjb::Polygon>& Polymesh::get_faces() const
{
    return _faces;
}

/**
 * @param  face The face to be added.
 *
 * @param i The index of the face
 */
/*void Polymesh::add_face(const Polygon & face, uint32_t id) throw (Illegal_argument)
{ 
    _faces.push_back(face);
    _faces.back().set_id(id);
}*/

/**
 * @param  face The face to be added.
 *  The index to the face will be set to the index of the last face + 1
 */
void Polymesh::add_face(const Polygon & face) throw (Illegal_argument)
{ 
   // uint32_t newid = _faces.back().get_id() + 1;
    _faces.push_back(face);
    //_faces.back().set_id(newid);
}



/**
 * This functions finds the index of the face adjacent to face f along edge e.
 * The face point index must be in the range [0,num_faces]. To specify the edge formed
 * by the 0th and nth point, set @em p = n.
 *
 * @param  f  Face index to get the adjacent face of.
 * @param  e  First point index in @em face of the shared edge. If set to num_faces,
 *            the other point forming the edge will be 0.
 *
 * @throw  kjb::Illegal_argument  If @em i or @em p are not valid indices.
 */
/** @brief returns the index of the face adjacent to face f along edge e */
unsigned int Polymesh::adjacent_face(unsigned int f, unsigned int e) const throw (Index_out_of_bounds,KJB_error)
{
    try{
        const Polygon & face = get_face(f);

        for(unsigned int i = 0; i < num_faces(); i++)
        {
            if(i == f)
                continue;

            const Polygon & face_i = _faces[i];

            for(unsigned int e2 = 0; e2 < face_i.get_num_points(); e2++)
            {
                if(is_shared_edge(face, e, face_i, e2))
                {
                    return i;
                }
            }
         }
    }catch(Index_out_of_bounds ex)
    {
        throw ex;
    }

    throw KJB_error("The mesh not fully connected, there is at least one edge with no adjacent face");
}

/**
 * Set the indexes of the faces using their order in the faces vector 
 */
/*void Polymesh::set_indexes()
{
    std::vector<Polygon>::iterator it;
    uint32_t i = 0;
    for(it = _faces.begin(); it != _faces.end(); it++, i++)
    {
        (*it).set_id(i);
    }
}*/


/**
 * @param  M  Homogeneous transformation matrix to transform this
 *            parallelepiped by.
 *
 * @throw  kjb::Illegal_argument  The matrix is not in 4x4 homogeneous
 *                                  coordinates.
 */
void Polymesh::transform(const Matrix& M) throw(Illegal_argument)
{
    for(unsigned int i = 0; i < num_faces(); i++)
    {
        try
        {
            _faces[i].transform(M);
        }
        catch(Illegal_argument& e)
        {
            throw;
        }
    }
}

/*
 * @param x the amount of translation along the x axis
 * @param y the amount of translation along the x axis
 * @param z the amount of translation along the x axis
 *
 * Translates this mesh
 */
void Polymesh::translate(double x, double y, double z)
{
    using namespace kjb;

    Matrix M = Matrix::create_3d_homo_translation_matrix(x, y, z);
    transform(M);
}

/*
 * @param x Defines the rotation axis
 * @param y Defines the rotation axis
 * @param z Defines the rotation axis
 * @param phi The amount of rotation, in radian
 *
 * Rotates this mesh around the provided axis
 */
void Polymesh::rotate(double phi, double x, double y, double z)
{
    using namespace kjb;
    Matrix M = Matrix::create_3d_homo_rotation_matrix(phi, x, y, z);
    transform(M);
}

/*
 * @param x The amount of scale along the x axis
 * @param y The amount of scale along the y axis
 * @param z The amount of scale along the z axis
 * @param phi The amount of rotation, in radian
 *
 * Scales this mesh
 */
void Polymesh::scale(double scale_x, double scale_y, double scale_z)
{
    if( (scale_x < DBL_EPSILON) || (scale_y < DBL_EPSILON) || (scale_z < DBL_EPSILON) )
    {
        KJB_THROW_2(Illegal_argument, "Scale amounts must be positive");
    }
    Matrix M = Matrix::create_3d_homo_scaling_matrix(scale_x, scale_y, scale_z);
    transform(M);

}

/*
 * @returns the number of faces in this mesh
 */
unsigned int Polymesh::num_faces() const
{
    return _faces.size();
}

/*
 * @param ifaces it will return a vector containing a pointer
 * to each face of this mesh
 */
void Polymesh::get_faces(std::vector<const Polygon *> & ifaces) const
{
    for(unsigned int i = 0; i < num_faces(); i++)
    {
        ifaces.push_back(&_faces[i]);
    }
}

/**
 * The file format is big-endian binary.
 *
 * @param  out  Output stream to write the members of this parallelepiped to.
 *
 * @throw  kjb::IO_error  Could not write to @em out.
 */
void Polymesh::write(const char *filename) const throw (IO_error)
{
    std::ofstream os;
    os.open(filename);
    if(os.fail())
    {
        throw IO_error("Could not open output file");
    }
    write(os);
    os.close();
}

/** 
 * The file format is big-endian binary.
 *
 * @param  out  Output stream to write the members of this parallelepiped to.
 *
 * @throw  kjb::IO_error  Could not write to @em out.
 */
void Polymesh::write(std::ostream& out) const throw (IO_error)
{
    using namespace kjb_c;

    uint32_t _num_faces = (uint32_t)_faces.size();
    if(! kjb_is_bigendian() )
    {
        kjb_c::bswap_u32((uint32_t*)&(_num_faces));
    }
    out.write((char*)&_num_faces, sizeof(uint32_t));
    if(out.fail() || out.eof())
    {
        throw IO_error("Could not write mesh");
    }
    
    for(unsigned int i = 0; i < _faces.size(); i++)
    {
        _faces[i].write(out);
    }
}

/** 
 * @param  in  Input stream to read the members of this parallelepiped from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid argument to read from file.
 */
void Polymesh::read(std::istream& in) throw (IO_error, Illegal_argument)
{
    using namespace kjb_c;
    
    uint32_t _num_faces;
    _faces.clear();
    
    in.read((char*)&_num_faces, sizeof(uint32_t));
    if(!kjb_is_bigendian())
    {
        bswap_u32((uint32_t*)&(_num_faces));
    }
   
    for(unsigned int i = 0; i < _num_faces; i++)
    {
        _faces.push_back(Polygon(in));
    }

}


/** @brief Checks whether edge e1 on face f1 is the same as edge e2 on face f2 */
bool Polymesh::is_shared_edge(const Polygon & f1, unsigned int e1, const Polygon & f2, unsigned int e2) const
       throw (Index_out_of_bounds)
{
  using namespace kjb;

  try{
      const Vector & e1_p1 = f1.get_edge_first_vertex(e1);
      const Vector & e1_p2 = f1.get_edge_second_vertex(e1);
      const Vector & e2_p1 = f2.get_edge_first_vertex(e2);
      const Vector & e2_p2 = f2.get_edge_second_vertex(e2);
      if(is_same_vertex(e1_p1, e2_p1))
      {
          return is_same_vertex(e1_p2, e2_p2);
      }
      else if(is_same_vertex(e1_p1, e2_p2))
      {
          return is_same_vertex(e1_p2, e2_p1);
      }

  } catch(Index_out_of_bounds ex)
  {
      throw(ex);
  }

  return false;
}

/*
 * @return true if face f2 contains contains edge e of face 1, false otherwise.
 * If f2 contains the edge, index will contain the index of the shared edge in face f2.
 */
bool Polymesh::edge_index_in_polygon(const Polygon &f1, unsigned int e, const Polygon &f2, unsigned int & index)
    throw (Index_out_of_bounds)
{
    try{
        unsigned int n_pts = f2.get_num_points();
        for(unsigned int i = 0; i < n_pts; i++)
        {
            if( is_shared_edge(f1, e, f2, i) )
            {
                index = i;
                return true;
            }
        }
    }catch(Index_out_of_bounds ex)
    {
        throw ex;
    }

    return false;
}

void Polymesh::wire_render() const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polymesh_Renderer::wire_render(*this);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

void Polymesh::wire_occlude_render() const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polymesh_Renderer::wire_occlude_render(*this);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

void Polymesh::solid_render() const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polymesh_Renderer::solid_render(*this);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

/** Render each edge of this polymesh  with a different color.
 * The first edge will be rendered using the input id as a color,
 * the following edges will be rendered with an id sequentially
 * increasing
 *
 * @param p The polymesh to render
 * @param start_id The color for the first edge of the polymesh
 * @return The last id used
*/
unsigned int Polymesh::wire_render_with_sequential_ids(unsigned int start_id) const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            start_id = GL_Polymesh_Renderer::wire_render_with_sequential_ids(*this, start_id);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
    return start_id;
}

unsigned int Polymesh::solid_render_with_sequential_ids(unsigned int start_id) const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            start_id = GL_Polymesh_Renderer::solid_render_with_sequential_ids(*this, start_id);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
    return start_id;
}

/**
 *  Renders the silhouette (contour) of this mesh. The current implementation works for
 *  convex meshes, and produces reasonable results for concave ones
 *
 *  @param camera The camera used for rendering this mesh
 *  @param iwidht The width (in pixel) to use when drawing the silhouette
 */
void Polymesh::silhouette_render
(
    const kjb::Base_gl_interface &   camera,
    double iwidth
) const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polymesh_Renderer::silhouette_render(camera, *this, iwidth);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

/**
 * Projects this polymesh onto the image plane using the current graphics transformation
 */
void Polymesh::project()
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polymesh_Renderer::project(*this);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}
/**
 * Computes the surface area of the mesh by adding together the 
 * areas of each polygon in the mesh.
 *
 * @return  the surface area of the mesh.
 */
double Polymesh::compute_surface_area() const
{
    double area = 0;
    for(size_t i = 0; i < _faces.size(); i++)
    {
        area += _faces[i].compute_area();
    }
    return area;
}

/**
 * @params vertices  a vector of Vectors to store the vertices of the
 *                    mesh in (points represented as Vectors).
 */
void Polymesh::get_all_vertices(std::vector<Vector> & vertices) const
{
    for(unsigned int i = 0; i < _faces.size(); i++)
    {
        std::vector<Vector> temp_vertices;
        _faces[i].get_all_vertices(temp_vertices);
        unsigned int current_size = vertices.size();
        double diff = 0.0;
        for(unsigned j = 0; j < temp_vertices.size(); j++)
        {
            bool found = false;
            for(unsigned int k = 0; k < current_size; k++)
            {
                diff = vertices[k].get_max_abs_difference(temp_vertices[j]);
                if(diff < 1e-16)
                {
                    found = true;
                    break;
                }

            }
            if(!found)
            {
                vertices.push_back(temp_vertices[j]);
            }
        }
    }
}

/**
 * @params edges  a vector of pairs of Vectors to store the vertices of the
 *                edges of the mesh in (points represented as Vectors).
 */
void Polymesh::get_all_edges(std::vector<std::vector<Vector> >& edges) const
{
    double diffTolerance = 1e-16;
    int num_faces = _faces.size();
    for(int i = 0; i < num_faces; i++)
    {
        int num_edges = _faces[i].get_num_points();

    	int current_size = edges.size();
    	double diff;
        for(int j = 0; j < num_edges; j++)
        {
            Vector edgeVertex1 = _faces[i].get_edge_first_vertex(j);
            Vector edgeVertex2 = _faces[i].get_edge_second_vertex(j);

            bool found = false;
            for(int k = 0; k < current_size; k++)
            {
                if(edgeVertex1.get_max_abs_difference(edges[k][0]) < diffTolerance && edgeVertex2.get_max_abs_difference(edges[k][1]) < diffTolerance)
                {
                    found = true;
                    break;
                }
                else if(edgeVertex2.get_max_abs_difference(edges[k][0]) < diffTolerance && edgeVertex1.get_max_abs_difference(edges[k][1]) < diffTolerance)
                {
                    found = true;
                    break;
                }
                else
                {
                    // Continue to next iteration of loop.
                }
            }
            if(!found)
            {
                std::vector<Vector> tmp_edge;
                tmp_edge.push_back(edgeVertex1);
                tmp_edge.push_back(edgeVertex2);
                edges.push_back(tmp_edge);
            }
        }
    }
}


void Polymesh::get_lines(std::vector<Line3d> & lines)
{
    //unsigned int current_size = 0;
    double pt_diff = 0.0;
    for(unsigned int i = 0; i < _faces.size(); i++)
    {
        std::vector<Line3d> line_temp;
        _faces[i].get_lines(line_temp);
        unsigned int current_size = lines.size();
        for(unsigned j = 0; j < line_temp.size(); j++)
        {
            bool found = false;
            for(unsigned int k = 0; k < current_size; k++)
            {
                pt_diff = lines[k].first.get_max_abs_difference(line_temp[j].first);
                pt_diff += lines[k].second.get_max_abs_difference(line_temp[j].second);
                if(pt_diff < 1e-08)
                {
                    found = true;
                    break;
                }
                pt_diff = lines[k].first.get_max_abs_difference(line_temp[j].second);
                pt_diff += lines[k].second.get_max_abs_difference(line_temp[j].first);
                if(pt_diff < 1e-08)
                {
                    found = true;
                    break;
                }

            }
            if(!found)
            {
                lines.push_back(line_temp[j]);
            }
        }
    }
}


/**
 * Finds the smallest and largest x, y, and z values and stores them
 * in the class members smallest_bounds and largest_bounds.
 */
void Polymesh::find_bounds()
{
    std::vector<Vector> vertices;
    get_all_vertices(vertices);

    smallest_bounds = vertices[0];
    largest_bounds = vertices[0];

    for(unsigned int i = 1; i < vertices.size(); i++)
    {
        Vector& vertex = vertices[i];

        // Check x bounds
        if(vertex(0) < smallest_bounds(0))
        {
            smallest_bounds(0) = vertex(0);
        }

        if(vertex(0) > largest_bounds(0))
        {
            largest_bounds(0) = vertex(0);
        }

        // Check y bounds
        if(vertex(1) < smallest_bounds(1))
        {
            smallest_bounds(1) = vertex(1);
        }

        if(vertex(1) > largest_bounds(1))
        {
            largest_bounds(1) = vertex(1);
        }

        // Check z bounds
        if(vertex(2) < smallest_bounds(2))
        {
            smallest_bounds(2) = vertex(2);
        }

        if(vertex(2) > largest_bounds(2))
        {
            largest_bounds(2) = vertex(2);
        }
    }
}

/**
 * @return  a Vector representing the point in the center of the mesh.
 */
Vector Polymesh::get_center()
{
    find_bounds();

    Vector center(3);

    center(0) = (smallest_bounds(0) + largest_bounds(0)) / 2;   // x
    center(1) = (smallest_bounds(1) + largest_bounds(1)) / 2;   // y
    center(2) = (smallest_bounds(2) + largest_bounds(2)) / 2;   // z

    return center;
}

/** 
 * @return  a double representing the value of the largest z-value in
 *          the polymesh.
 */
double Polymesh::get_largest_z_bound()
{
    return largest_bounds(2);
}

/** 
 * @return  a Vector representing the largest x, y, and z-values in the
 *          polymesh.
 */
Vector Polymesh::get_largest_bounds() const
{
    return largest_bounds;
}

/** 
 * @return  a Vector representing the smallest x, y, and z-values in the
 *          polymesh.
 */
Vector Polymesh::get_smallest_bounds() const
{
    return smallest_bounds;
}

