/* $Id: g2_rotation.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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


#include <g2_cpp/g2_rotation.h>
#include <g_cpp/g_quaternion.h>
#include <gr_cpp/gr_matrix.h>

namespace kjb {
Rotation_axis::Rotation_axis(const Vector& pt, const Vector& dir) :
    axis_pt_(pt),
    axis_dir_(dir)
{
}

Vector Rotation_axis::rotate(const Vector& pt, double angle) const
{
    if(pt.size() != 3)
    {
        KJB_THROW_2(Illegal_argument, "Point must have size = 3.");

    }
    Quaternion q(axis_dir_, angle);
    Vector result = pt;
    result -= axis_pt_;
    result = q.rotate(result);
    result += axis_pt_;
    return result;
}

Matrix Rotation_axis::get_transformation_matrix(double a) const
{
    Matrix result = create_identity_matrix(4);
    translate(result, -axis_pt_);
    kjb::rotate(result, Quaternion(axis_dir_, a));
    translate(result, axis_pt_);

    // first translate axis to origin, then rotate and translate back
    return result;

}

} // namespace kjb 
