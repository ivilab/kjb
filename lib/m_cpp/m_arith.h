/* $Id: m_arith.h 21596 2017-07-30 23:33:36Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#ifndef KJB_CPP_M_VECTOR_UTIL_H
#define KJB_CPP_M_VECTOR_UTIL_H

#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_vector_d.h"

namespace kjb 
{



/**
 * Sum elements in array by recursive pairwise merging.  This is less prone
 * to precision loss than a sequential sum, a problem that is
 * quite drastic when summing large arrays of small numbers.
 *
 * @tparam Array_type Any type that implements deep-copy semantics with operator=  and implements operator[] (this includes kjb::Matrix, but not c-style arrays)
 */
template <class Assignable_array_type>
double reduce(const Assignable_array_type& array, size_t length)
{
    Assignable_array_type tmp = array;
    return reduce_in_place(tmp, length);
}

/**
 * Sum elements in array by recursive pairwise merging.  This is less prone
 * to precision loss than a sequential sum, a problem that is
 * quite drastic when summing large arrays of small numbers.
 *
 * @tparam Array_type Any type that implements operator[] (this includes kjb::Matrix and c-style arrays)
 *
 * @warning input array will be modified and should not be used after calling
 */
template <class Array_type>
double reduce_in_place(Array_type& array, size_t length)
{
    // sum up distances (use divide-and-conquor to avoid precision loss
    while(length > 1)
    {
        size_t half_length = (length+1) / 2;

        for(size_t i = 0; i < half_length; i++)
        {
            // TODO: can we improve cache locality here?
            if(i + half_length < length)
                array[i] += array[i + half_length];
        }

        length = half_length;
    }

    return array[0];
}

} //namespace kjb
#endif
