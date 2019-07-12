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

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "gr_cpp/gr_triangular_mesh.h"
#include "m_cpp/m_matrix_stream_io.h"
#include <fstream>
#include <iostream>

using namespace kjb;

/**
 * The file format is big-endian binary.
 *
 * @param  fname  Input file to read this mesh from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid arguments in file to read from.
 */
Triangular_mesh::Triangular_mesh(const char* fname) throw (Illegal_argument, IO_error)
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
Triangular_mesh::Triangular_mesh(std::istream& in) throw (Illegal_argument, IO_error)
{
    read(in);
}

/** @param  t  Mesh to copy into this one. */
Triangular_mesh::Triangular_mesh(const Triangular_mesh& t)
: Polymesh(), _adjacency(t._adjacency)
{
    _is_adjacency_consistent =  t._is_adjacency_consistent;
}

/**
 * Performs a deep copy of the faces in the parallelepiped.
 *
 * @param  t  Triangular mesh to copy into this one.
 *
 * @return A reference to this mesh
 */
Triangular_mesh& Triangular_mesh::operator= (const Triangular_mesh& t)
{
    Polymesh::operator=(t);

    _adjacency = t._adjacency;

    _is_adjacency_consistent =  t._is_adjacency_consistent;

    return *this;
}


/** @return A new copy of this parallelepiped. */
Triangular_mesh* Triangular_mesh::clone() const
{
    return new Triangular_mesh(*this);
}

/**
 * The face point index must be in the range [0,num_faces]. To specify the edge formed
 * by the 0th and nth point, set @em p = n.
 *
 * @param  i  Face index to get the adjacent face of.
 * @param  p  First point index in @em face of the shared edge. If set to num_faces,
 *            the other point forming the edge will be 0.
 *
 * @throw  kjb::Illegal_argument  If @em i or @em p are not valid indices.
 */
uint32_t Triangular_mesh::adjacent_face(unsigned int f, unsigned int e) const throw (Index_out_of_bounds)
{
    if(!_is_adjacency_consistent)
    {
        throw KJB_error("Adjacency information not available");
    }
    return _adjacency(f, e);
}


/*void Triangular_mesh::add_face(const Polygon & face, uint32_t id) throw (kjb::Illegal_argument)
{
    Polymesh::add_face(face, id);
    _is_adjacency_consistent = false;
}*/

void Triangular_mesh::add_face(const Polygon & face) throw (kjb::Illegal_argument)
{
    Polymesh::add_face(face);
    _is_adjacency_consistent = false;
}
/**
 * It creates the adjacency matrix for this triangular mesh
 */
void Triangular_mesh::create_adjacency_matrix() throw(KJB_error)
{
    unsigned int i, j, ei, ej;
    int supcount = 0;

    try{
        _adjacency.zero_out(num_faces(), 3);
        for(i = 0; i < num_faces(); i++)
        {
            for(j = 0; j < 3; j++)
            {
                _adjacency(i,j) = -1;
            }
        }

        for(i = 0; i < num_faces(); i++)
        {
            const Polygon& face_i = get_face(i);
            for(ei = 0; ei < 3; ei ++)
            {
                if(_adjacency(i,ei) >= 0)
                {
                    continue;
                }
                int count = 0;
                for( j = i + 1; j < num_faces(); j++)
                {
                    const Polygon& face_j = get_face(j);
                    if(edge_index_in_polygon(face_i, ei, face_j, ej) )
                    {
                        _adjacency(i, ei) = j;
                        _adjacency(j, ej) = i;
                        count++;
                    }
                }
                if(count != 1)
                {
                    supcount++;
                }
                // ASSERT(count > 0);
                if(count == 0)
                { 
                    std::cout << "WARNING: face is not adjacent to anything " << std::endl;
                }
            }
        }
    } catch(KJB_error)
    {
        throw KJB_error("Triangular mesh not fully connected, adjacency could not be created");
    }
    //std::cout << "The last count is " << supcount << std::endl;

    _is_adjacency_consistent = true;
}

/**
 * Creates the adjacency matrix for this triangular mesh if it
 * has not been created yet, otherwise reads adjacency matrix
 * from a file.
 */
void Triangular_mesh::set_adjacency_matrix(const char* fname) throw(KJB_error)
{
    char adjFile[100];
    strcpy(adjFile, fname);
    strcat(adjFile, "_adjMatrix");

    std::ifstream in_adj(adjFile);
    if(in_adj.is_open())
    {
        _adjacency = Int_matrix(adjFile);
        _is_adjacency_consistent = true;
        in_adj.close();
    }
    else
    {
        create_adjacency_matrix();
        if(_is_adjacency_consistent)
        {
            _adjacency.write(adjFile);
        }
    }
}

/**
 * The file format is big-endian binary.
 *
 * @param  out  Output stream to write the members of this parallelepiped to.
 *
 * @throw  kjb::IO_error  Could not write to @em out.
 */
void Triangular_mesh::write(const char *filename) const throw (IO_error)
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
void Triangular_mesh::write(std::ostream& out) const throw (IO_error)
{
    using namespace kjb_c;

    try{
        Polymesh::write(out);
        Matrix_stream_io::write_int_matrix(out, _adjacency);
    } catch(IO_error ex)
    {
        throw ex;
    }
}

/**
 * @param  in  Input stream to read the members of this parallelepiped from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid argument to read from file.
 */

void Triangular_mesh::read(std::istream& in) throw (IO_error, Illegal_argument)
{
    using namespace kjb_c;

    try{
        Polymesh::read(in);
        Matrix_stream_io::read_int_matrix(in, _adjacency);
        _is_adjacency_consistent = true;
    } catch(IO_error ex)
    {
        throw ex;
    }
}
