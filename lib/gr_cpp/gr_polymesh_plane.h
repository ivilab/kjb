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
|     Emily Hartley
|
* =========================================================================== */

/**
 * @file gr_polymesh_plane.h
 *
 * @author Emily Hartley
 *
 * @brief This class contains a Vector of plane parameters, a list of the 
 * face indices that lie in the plane, and the polymesh that the faces are
 * from.
 */


#ifndef KJB_POLYMESH_PLANE_H
#define KJB_POLYMESH_PLANE_H

#include <gr_cpp/gr_polymesh.h>
#include <vector>

namespace kjb{


/**
 * @class Polymesh_Plane
 *
 * @brief This class contains a Vector of plane parameters, a vector of the 
 * face indices that lie in the plane, and the polymesh that the faces are
 * from.  The plane parameters are the coefficients of a plane of the form
 * ax + by + cz + d = 0.
 */
class Polymesh_Plane
{
    public:
    /** @brief Constructs a polymesh_plane from a polymesh */
    Polymesh_Plane(const Polymesh& p);

    /** 
     * @brief Constructs a polymesh_plane with a polymesh, plane
     * parameters, and a vector of face indices.
     */
    Polymesh_Plane(const Polymesh& p, Vector& params, std::vector<int>& indices);
        
    /** @brief Copy constructor. */
    Polymesh_Plane(const Polymesh_Plane& pp);

    /** @brief Deletes this Polymesh_Plane. */
    ~Polymesh_Plane();

    /** @brief Copies a Polymesh_Plane into this one. */
    Polymesh_Plane& operator=(const Polymesh_Plane&);

    /** 
     * @brief Returns the indices of the faces that lie in the defined 
     * plane.
     */
    inline std::vector<int> get_face_indices() const;

    /** @brief Returns the coefficients of a plane. */
    inline Vector get_plane_params() const;

    /** @brief Returns the polymesh. */
    inline const Polymesh* get_polymesh() const;

    /** @brief Sets the vector of indices that lies in the plane. */
    inline void set_face_indices(std::vector<int> indices);

    /** @brief Sets the Vector of plane coefficients. */
    inline void set_plane_params(Vector params);

    private:
    const Polymesh* m_p;

    /** 
     * @brief Coefficients of a plane of the form ax + by + cz + d = 0
     * in which at least one of the faces in the mesh lies.
     */
    Vector plane_params;

    /** 
     * @brief Indices of the faces of the mesh that lie in the plane
     * defined by the plane_params.
     */
    std::vector<int> face_indices;
};

inline std::vector<int> Polymesh_Plane::get_face_indices() const
{
    return face_indices;
}

inline Vector Polymesh_Plane::get_plane_params() const
{
    return plane_params;
}

inline const Polymesh* Polymesh_Plane::get_polymesh() const
{
    return m_p;
}

inline void Polymesh_Plane::set_face_indices(std::vector<int> indices)
{
    face_indices = indices;
}
    

inline void Polymesh_Plane::set_plane_params(Vector params)
{
    plane_params = params;
}

}

#endif
