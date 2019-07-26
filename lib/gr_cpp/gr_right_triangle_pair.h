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

#ifndef KJB_RIGHT_TRIANGLE_PAIR_H
#define KJB_RIGHT_TRIANGLE_PAIR_H

#include "gr_cpp/gr_polymesh.h"
namespace kjb{


class Right_Triangle_Pair
{
    public:
    /** @brief Constructs a right_triangle_pair from a polymesh. */
    Right_Triangle_Pair(const Polymesh& p);

    Right_Triangle_Pair(const Polymesh& p, int triangle1, int triangle2, int hypotenuse1, int hypotenuse2, Vector edges1, Vector edges2);

    /** @brief Copy constructor. */
    Right_Triangle_Pair(const Right_Triangle_Pair& rtp);

    /** @brief Deletes this Right_Triangle_Pair. */
    ~Right_Triangle_Pair();

    /** @brief Copies a Right_Triangle_Pair into this one. */
    Right_Triangle_Pair& operator=(const Right_Triangle_Pair& rtp);

    /** @brief Returns the polymesh. */
    inline const Polymesh* get_polymesh() const
    {
        return m_p;
    }

    /** @brief Returns the index of one of the right triangles. */
    inline int get_triangle1() const
    {
        return index_triangle1;
    }

    /** @brief Returns the index of the other right triangles. */
    inline int get_triangle2() const
    {
        return index_triangle2;
    }

    /** @brief Returns the index of the hypotenuse of triangle1. */
    inline int get_hypotenuse1() const
    {
        return m_hypotenuse1;
    }

    /** @brief Returns the index of the hypotenuse of triangle2. */
    inline int get_hypotenuse2() const
    {
        return m_hypotenuse2;
    }

    /** 
     * @brief Returns the Vector containing the indices of the parallel 
     * edges in the rectangle formed by the two right triangles. One edge
     * is from triangle1 and the other edge is from triangle2. 
     */
    inline const Vector & get_parallel_edges1() const
    {
        return parallel_edges1;
    }

    /** 
     * @brief Returns the Vector containing the indices of the parallel 
     * edges in the rectangle formed by the two right triangles. One edge
     * is from triangle1 and the other edge is from triangle2. 
     */
    inline const Vector & get_parallel_edges2() const
    {
        return parallel_edges2;
    }

    private:

    const Polymesh* m_p;

    int index_triangle1;
    int index_triangle2;

    int m_hypotenuse1;
    int m_hypotenuse2;

    Vector parallel_edges1;
    Vector parallel_edges2;
};

}

#endif
