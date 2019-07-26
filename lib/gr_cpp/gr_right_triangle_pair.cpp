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


#include "gr_cpp/gr_right_triangle_pair.h"

using namespace kjb;

/**
 * @param p  the polymesh
 */
Right_Triangle_Pair::Right_Triangle_Pair(const Polymesh& p)
{
    m_p = &p;
}

/* 
 * @param p  the polymesh
 * @param triangle1  the index of one of the right triangles making up a 
 *                   rectangle
 * @param triangle2  the index of the other right triangle making up a 
 *                   rectangle
 * @param hypotenuse1  the index of the longest edge of triangle1
 * @param hypotenuse2  the index of the longest edge of treangle2
 * @param edges1  a Vector containing the index of an edge in triangle1 and 
 *                the index of the edge in triangle2 that is parallel to it
 * @param edges2  a Vector containing the index of the other edge in triangle1 
 *                and the index of the edge in triangle2 that is parallel to it
 */
Right_Triangle_Pair::Right_Triangle_Pair(const Polymesh& p, int triangle1, int triangle2, int hypotenuse1, int hypotenuse2, Vector edges1, Vector edges2)
{
    m_p = &p;
    index_triangle1 = triangle1;
    index_triangle2 = triangle2;
    m_hypotenuse1 = hypotenuse1;
    m_hypotenuse2 = hypotenuse2;
    parallel_edges1 = edges1;
    parallel_edges2 = edges2;
}

/*
 * @param rtp  the Right_Triangle_Pair to copy into this one.
 */
Right_Triangle_Pair::Right_Triangle_Pair(const Right_Triangle_Pair& rtp)
{
    this->m_p = rtp.get_polymesh();
    this->index_triangle1 = rtp.get_triangle1();
    this->index_triangle2 = rtp.get_triangle2();
    this->m_hypotenuse1 = rtp.get_hypotenuse1();
    this->m_hypotenuse2 = rtp.get_hypotenuse2();
    this->parallel_edges1 = rtp.get_parallel_edges1();
    this->parallel_edges2 = rtp.get_parallel_edges2();
}

/*
 * Frees all space allocated by this Right_Triangle_Pair.
 */
Right_Triangle_Pair::~Right_Triangle_Pair()
{
}

/*
 * Performs a deep copy of the triangle and edge indices.
 *
 * @param rtp  Right_Triangle_Pair to copy into this one.
 *
 * @return  A reference to this Right_Triangle_Pair.
 */
Right_Triangle_Pair& Right_Triangle_Pair::operator=(const Right_Triangle_Pair& rtp)
{
    if(this == &rtp) return *this;

    this->m_p = rtp.m_p;
    this->index_triangle1 = rtp.index_triangle1;
    this->index_triangle2 = rtp.index_triangle2;
    this->m_hypotenuse1 = rtp.m_hypotenuse1;
    this->m_hypotenuse2 = rtp.m_hypotenuse2;
    this->parallel_edges1 = rtp.parallel_edges1;
    this->parallel_edges2 = rtp.parallel_edges2;

    return *this;
}
