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


#ifndef KJB_TRIANGULAR_MESH_H
#define KJB_TRIANGULAR_MESH_H

#include <iosfwd>
#include <vector>
#include <inttypes.h>

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <l_cpp/l_int_matrix.h>

#include <gr_cpp/gr_polymesh.h>


namespace kjb {


/**
 * @class Triangular_mesh
 *
 * @brief Triangular_mesh: a polygonal mesh of which each face is a triangle.
 */
class Triangular_mesh : public kjb::Polymesh
{

public:
    /** @brief Constructs a triangular mesh. */
    Triangular_mesh()
    : Polymesh()
    {
        _is_adjacency_consistent = true;
    };


    /** @brief Reads a triangular mesh from an input file. */
    Triangular_mesh(const char* fname) throw (kjb::Illegal_argument, kjb::IO_error);


    /** @brief Reads a triangular mesh from an input file. */
    Triangular_mesh(std::istream& in) throw (kjb::Illegal_argument, kjb::IO_error);


    /** @brief Copy constructor. */
    Triangular_mesh(const Triangular_mesh& t);


    /** @brief Copies a triangular mesh into this one. */
    virtual Triangular_mesh& operator= (const Triangular_mesh& t);


    /** @brief Destructor of a triangular mesh. */
    virtual ~Triangular_mesh() {};

    /** @brief Returns the face adjacent to another on a shared edge. */
    virtual unsigned int adjacent_face(uint32_t i,uint32_t p) const 
           throw (Index_out_of_bounds);


    /** @brief Reads this triangular mesh from an input stream. */
    void read(std::istream & in)
    throw(kjb::IO_error,kjb::Illegal_argument);


    /** @brief Writes this mesh to an output stream. */
    void write(std::ostream & ost)
    const throw(kjb::IO_error);

    /** @brief Writes this mesh to an output stream. */
    void write(const char *filename) const throw (IO_error);


     /** @brief Clones this mesh. */
    virtual Triangular_mesh * clone() const;

    /** @brief Finds the adjacency relationships between faces */
    void create_adjacency_matrix() throw(KJB_error);

    /** @brief Gets the adjacency matrix from a file and saves it in the class parameter. */
    void set_adjacency_matrix(const char* fname) throw(KJB_error);

    //virtual void add_face(const Polygon & face, uint32_t id) throw (kjb::Illegal_argument);

    virtual void add_face(const Polygon & face) throw (kjb::Illegal_argument);

protected:

    /**
     * @brief Face adjacency matrix.
     *
     * Major index is the face, minor is the index of the first point (in
     * increasing order) on the face forming the adjacency edge.
     */
    kjb::Int_matrix _adjacency;

    /**
     * This determines whether the adjacency matrix is up to date or not
     */
    bool _is_adjacency_consistent;

};


}


#endif
