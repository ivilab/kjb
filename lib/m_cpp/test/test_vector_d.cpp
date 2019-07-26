/* $Id: test_vector_d.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

// test_vector_d is split into two test programs.
// This file is tests the generic functionality
// for arbitrary D.  The other is test_vector_d_precompiled.cpp
// which tests that for vectors of dimension 1 through 10 
// m_vector_d.impl.h does not need to be included.
// 
#include <stdlib.h>
static const size_t D = 100;
#include <m_cpp/m_vector_d.h>
#include <m_cpp/m_vector_d.impl.h>
#include <test_vector_d.impl>

int main()
{
    return main_();
}
