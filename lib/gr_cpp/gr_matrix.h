/* $Id: gr_matrix.h 21599 2017-07-31 00:44:30Z kobus $ */
/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
 * =========================================================================== */

#ifndef KJB_GR_MATRIX_H
#define KJB_GR_MATRIX_H

#include "g_cpp/g_quaternion.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"

/**
 * @file
 * Graphics-specific matrix operations
 */


namespace kjb
{

void rotate(Matrix& m, const Quaternion& r);

/**
 * Apply translation v to homogeneous 3D transformation matrix m.
 * This mimicks opengl's glTranslate() function. 
 * Vector is assumed to be a column vector.
 */
void translate(Matrix& m, const Vector& v_in);

}
#endif
