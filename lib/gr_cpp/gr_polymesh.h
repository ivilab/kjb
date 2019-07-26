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
 * @author Joseph Schlecht, Luca Del Pero
 *
 * @brief Abstract class of connected polygons (faces) forming a mesh.
 */


#ifndef KJB_POLYMESH_H
#define KJB_POLYMESH_H

#include <inttypes.h>

#include <l_cpp/l_exception.h>
#include <gr_cpp/gr_polygon.h>
#include <gr_cpp/gr_rigid_object.h>
#include <vector>
#include <gr_cpp/gr_renderable.h>

#include "l_cpp/l_readable.h"
#include "l_cpp/l_writeable.h"

namespace kjb{


class Base_gl_interface;
/**
 * @class Polymesh
 *
 * @brief Abstract class of connected polygons (faces) forming a mesh.
 * We assume that each edge is shared between exactly two faces, that is to say the
 * mesh has to be fully connected
 */
class Polymesh: public kjb::Abstract_renderable,
                public kjb::Rigid_object,
                public kjb::Readable,
                public kjb::Writeable
{
    public:

        /** @brief Constructs a triangular mesh. */
        Polymesh()
        : Abstract_renderable(), Rigid_object(), Readable(), Writeable()
        {

        }

        /** @bried Constructs a polymesh with n empty polygons */
        Polymesh(unsigned int n);


        /** @brief Reads a triangular mesh from an input file. */
        Polymesh(const char* fname) throw (kjb::Illegal_argument, kjb::IO_error);


        /** @brief Reads a triangular mesh from an input file. */
        Polymesh(std::istream& in) throw (kjb::Illegal_argument, kjb::IO_error);


        /** @brief Copy constructor. */
        Polymesh(const Polymesh& t);


        /** @brief Copies a triangular mesh into this one. */
        virtual Polymesh& operator= (const Polymesh& t);

        /** @brief Clones this mesh. */
        virtual Polymesh * clone() const;

        /** @brief Deletes this Polymesh. */
        virtual ~Polymesh();


        /** @brief Returns the number of faces in this mesh. */
        unsigned int num_faces() const;

        /** @brief Returns the faces of this mesh. */
        const std::vector<kjb::Polygon>& get_faces() const;
        
        /** 
         * @brief Returns an indexed face.
         *
         * @param  i  Index of the face to get; ranges 0 to N-1. 
         *
         * @return The specified face.
         *
         * @throw  kjb::Illegal_argument  Invalid face index.
         */
         const kjb::Polygon& get_face(unsigned int i) const throw (kjb::Index_out_of_bounds);

        /**
        * @brief Returns an indexed face.
        *
        * @param  i  Index of the face to get; ranges 0 to N-1.
        *
        * @return The specified face.
        *
        * @throw  kjb::Illegal_argument  Invalid face index.
        */
        kjb::Polygon& get_face_ref(unsigned int i) throw (kjb::Index_out_of_bounds);

        //virtual void add_face(const Polygon & face, uint32_t id) throw (kjb::Illegal_argument);

        virtual void add_face(const Polygon & face) throw (kjb::Illegal_argument);

         /** @brief Reads this polymesh from an input stream. */
        virtual void read(std::istream & in)
        throw(kjb::IO_error,kjb::Illegal_argument);


        /** @brief Writes this mesh to an output stream. */
        virtual void write(std::ostream & ost)
        const throw(kjb::IO_error);

        /** @brief Writes this mesh to an output stream. */
        virtual void write(const char *filename) const throw (IO_error);


        /** @brief Applies a linear transformation to this parallelepiped. */
        virtual void transform(const kjb::Matrix &) throw(kjb::Illegal_argument);

        /* @brief Translates this mesh */
        virtual void translate(double x, double y, double z);

        /* @brief translates this mesh around an axis */
        virtual void rotate(double phi, double x, double y, double z);

        virtual void rotate(double dpitch, double dyaw, double droll)
        {
            Rigid_object::rotate(dpitch, dyaw, droll);
        }

        /* @brief scales this mesh */
        virtual void scale(double scale_x, double scale_y, double scale_z);

        /** @brief returns the index of the face adjacent to face f along edge e
         *  This is a very inefficient implementation, class derived from this one
         *  should implement their own version, possibly storing the adjacency information
         *  and not computing it every time.
         *  Not thoroughly tested
         */
        virtual unsigned int adjacent_face(unsigned int f, unsigned int e) const
                       throw (Index_out_of_bounds,KJB_error);

        void get_faces(std::vector<const Polygon *> & ifaces) const;

        /** Rendering */

        /** @brief Renders this mesh as a wire frame */
        virtual void wire_render() const;

        /** @brief Render each edge of this polymesh with a different color.
         * The first edge will be rendered using the input id as a color,
         * the following edges will be rendered with an id sequentially
         * increasing. Returns the id (color) used to render the last
         * edge of the polygon
         */
        unsigned int wire_render_with_sequential_ids(unsigned int start_id = 1) const;

        /** @brief Render each polygon of this polymesh with a different color.
         * The first polygon will be rendered using the input id as a color,
         * the following ones will be rendered with an id sequentially
         * increasing. Returns the id (color) used to render the last
         * polygon
         */
        unsigned int solid_render_with_sequential_ids(unsigned int start_id = 1) const;

        /**
         *  @brief Renders this mesh as a wire frame into the depth buffer.
         *  Use wire_render after this to render this mesh as a wire frame
         *  by not drawing occluded edges
         */
        virtual void wire_occlude_render() const;

        /*
         * @brief Renders this mesh as a solid object
         */
        virtual void solid_render() const;

        /** @ brief Renders the silhouette of this mesh */
        virtual void silhouette_render(const kjb::Base_gl_interface & camera, double iwidth = 1.0) const;

        /*
         * @brief Projects all the polygons in this mesh using
         * onto the image plane
         */
        virtual void project();

         /**
         * @brief Returns the surface area of the mesh
         */
        double compute_surface_area() const;

        /**
         * @brief Stores the vertices of the mesh in a vector
         */
        void get_all_vertices(std::vector<Vector> & vertices) const;
        
        /**
         * @brief Stores all of the unique pairs of points that make up the
         * edges of the polymesh in a vector.
         */
        void get_all_edges(std::vector<std::vector<Vector> >& edges) const;

        void get_lines(std::vector<Line3d> & lines);

        /*
         * @brief  Iterates through all the vertices of the mesh and
         *         stores the smallest and largest x,y,z values.
         */
        void find_bounds();

        /*
         * @brief  Returns the center point of the mesh based on the
         *         midpoint of the smallest and largest bounds.
         */
        kjb::Vector get_center();

        /*
         * @brief  Returns the largest z-value of the points in the 
         *         polymesh.
         */
        double get_largest_z_bound();

        /*
         * @brief  Returns the largest x, y, and z bounds in the polymesh.
         */
        kjb::Vector get_largest_bounds() const;

        /*
         * @brief  Returns the smallest x, y, and z bounds in the polymesh.
         */
        kjb::Vector get_smallest_bounds() const;


    protected:
       /** @brief Polygons defining the mesh. */
       std::vector<kjb::Polygon> _faces;
       Vector smallest_bounds;
       Vector largest_bounds;

       /** @brief Compares two points*/
       inline bool is_same_vertex(const kjb::Vector& p1,const kjb::Vector& p2) const
       {
           return p1 == p2;
       }

       /** @brief Checks whether edge e1 on face f1 is the same as edge e2 on face f2 */
       bool is_shared_edge(const Polygon & f1, unsigned int e1, const Polygon & f2, unsigned int e2) const
       throw (Index_out_of_bounds);

       /*
        * @brief Returns true if face f2 contains contains edge e of face 1, false otherwise.
        * If f2 contains the edge, index will contain the index of the shared edge in face f2.
        */
       bool edge_index_in_polygon(const Polygon &f1, unsigned int e, const Polygon &f2, unsigned int & index)
           throw (Index_out_of_bounds);

       friend class GL_Polymesh_Renderer;

};


}


#endif
