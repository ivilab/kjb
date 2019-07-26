/* $Id: gr_matrix.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "gr_cpp/gr_matrix.h"

namespace kjb
{
/**
 * Apply translation v to homogeneous 3D transformation matrix m.
 * This is designed to mimick opengl's glRotate() function. 
 * @pre Matrix m is assumed to be homogeneous or non-homogeneous 3D rotation matrix (3x3 or 4x4).
 * @post Matrix m represents a new coordinate system after rotating by r.
 */
void rotate(Matrix& m, const Quaternion& r)
{
    ASSERT(m.get_num_rows() == m.get_num_cols());

    bool resize = false;
    if(m.get_num_rows() == 3)
    {
        resize = true;
    }
    else if(m.get_num_rows() != 4)
    {
        KJB_THROW_2(Illegal_argument, "Number of arguments must be 3 or 4.");
    }

    m *= r.get_rotation_matrix();

    if(resize)
    {
        m.resize(3,3);
    }
}

/**
 * Apply translation v to homogeneous 3D transformation matrix m.
 * This mimicks opengl's glTranslate() function. 
 * Vector is assumed to be a column vector.
 */
void translate(Matrix& m, const Vector& v_in)
{
    ASSERT(m.get_num_rows() == m.get_num_cols());
    ASSERT(m.get_num_rows() == 4);
    ASSERT(v_in.size() == 4 || v_in.size() == 3);

    Vector v = v_in;
    if(v.size() == 4)
        v /= v[3];

    Matrix t_mat = create_identity_matrix(4);
    t_mat(0,3) = v(0);
    t_mat(1,3) = v(1);
    t_mat(2,3) = v(2);

    m *= t_mat;
}
}
