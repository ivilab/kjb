/* $Id: g_camera_calibration.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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

#include <g_cpp/g_camera_calibration.h>
#include <l_cpp/l_index.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_mat_util.h>
#include <m_cpp/m_mat_view.h>
#include <boost/assign/list_of.hpp>
#include <n/n_qr.h>

namespace kjb
{

const Calibration_descriptor Calibration_descriptor::STANDARD = {0, 0, CENTER, true, true};

std::ostream& operator<<(std::ostream& ost, const Calibration_descriptor::Coord_convention& mode)
{
    switch(mode)
    {
        case Calibration_descriptor::TOP_LEFT:
            ost << "top-left";
            break;
        case Calibration_descriptor::BOTTOM_LEFT:
            ost << "bottom-left";
            break;
        case Calibration_descriptor::CENTER:
            ost << "center";
            break;
        default:
            abort();
    }
    return ost;
}

std::istream& operator>>(std::istream& ist, Calibration_descriptor::Coord_convention& mode)
{
    std::string word;
    ist >> word;
    if(word.size() == 0)
        ist.setstate(std::ios_base::failbit);

    switch(word[0])
    {
        case 't':
            mode = Calibration_descriptor::TOP_LEFT;
            break;
        case 'b':
            mode = Calibration_descriptor::BOTTOM_LEFT;
            break;
        case 'c':
            mode = Calibration_descriptor::CENTER;
            break;
        default:
            ist.setstate(std::ios_base::failbit);
            break;
    }
    return ist;
}


void standardize_camera_matrices(
        Matrix& intrinsic,
        Matrix& rotation,
        Vector& translation,
        const Calibration_descriptor& cal)
{
    typedef Calibration_descriptor Cal;

    const Index_range& _ = Index_range::ALL;

    // Move principal point to center of image
    switch(cal.convention)
    {
        case Cal::TOP_LEFT:
        case Cal::BOTTOM_LEFT:
            intrinsic(0,2) -= cal.image_width/2.0;
            intrinsic(1,2) -= cal.image_height/2.0;
            break;
        case Cal::CENTER:
        default:
            break;
    }

    // make y-axis point upward
    if(cal.convention == Cal::TOP_LEFT)
    {
        // This is a simple mirror operation
        // because we centered the principal point above
        intrinsic(1,_) *= -1;

        // keep diagonal element positive by pushing it into the extrinsic matrix
        intrinsic(_,1) *= -1;
        rotation(1, _) *= -1;
        translation[1] *= -1;
    }

    /*
     * At this point, diagonal elements of the intrinsic matrix are all positive.  However, we need to maintain a right-handed screen coordinate system, so we'll flip the camera x-axis if necessary.
     */
    if(cal.look_down_positive_z)
    {
        // this will result in an x-axis pointing left, in eye coordinates, 
        // which agrees with Hartley and Zisserman
        intrinsic(_, 0) *= -1;
        rotation(0, _)  *= -1;
        translation[0]  *= -1;
    }

    // Force points in front of camera to have positive w coordinate after projecting.  This is mathematically equivalent, but helps in working with opengl
    if(!cal.look_down_positive_z)
    {
        intrinsic(_, 2) *= -1;
        rotation(2, _) *= -1;
        translation[2] *= -1;
    }

    // now make sure our rotation matrix is pure rotation, not a rotoinversion
    if(det(rotation) < 0)
    {
        rotation *= -1;
        translation *= -1;
    }

    // at this point, everything should be as expected.  
    // Check this by making sure origin projects to the right place
    

    // should z be positive?
    bool target_positive_z = cal.world_origin_in_front && cal.look_down_positive_z;
    // is z positive?
    bool positive_z = translation(2) > 0;

    if(positive_z != target_positive_z)
    {

        abort();  // this shouldn't ever happen
//        std::cout << "wrong z-sign. flipping...\n";
        translation(2)  *= -1;
        rotation(2, _)  *= -1;
//        intrinsic(_, 2) *= -1;

        // have to keep det(rotation) > 0
        // so we arbitrarilly choose x to toggle
//        translation(0)  *= -1;
//        rotation(0, _)  *= -1;
//        intrinsic(_, 0) *= -1;
//
//        intrinsic *= -1; // keep element (2,2) positive
    }

}

///**
// * Some camera conventions have the camera looking
// * down the positive z-axis, while others
// * look down the negative z-axis (e.g. OPenGL).
// * This is admittedly a bit arcane, but a necessary operation, nonetheless.
// */
//void flip_camera_matrices_z_convention()
//{
//
//}



void decompose_camera_matrix(
        const Matrix& camera_matrix, 
        Matrix& intrinsic,
        Matrix& rotation,
        Vector& translation)
{
    const Index_range& _ = Index_range::ALL;

    // in Forsyth's notation, 
    //     intrinsic   = K
    //     rotation    = R
    //     translation = t

    kjb_c::Matrix* R = NULL;
    kjb_c::Matrix* Q = NULL;

    Matrix A(camera_matrix);
    A.resize(3,3);

    rq_decompose(A.get_c_matrix(), &R, &Q);
    intrinsic = Matrix(R);
    rotation = Matrix(Q);

    intrinsic *= 1 / intrinsic(2,2);

    // force positive alpha_x
    if(intrinsic(0,0) < 0)
    {
        intrinsic(_,0) *= -1;
        rotation(0, _) *= -1;
    }

    // force positive alpha_y
    if(intrinsic(1,1) < 0)
    {
        intrinsic(_,1) *= -1;
        rotation(1, _) *= -1;
    }

    //  Find camera center in world frame.
    Vector c = Vector(3);
    Vector p1, p2, p3, p4;
    p1 = camera_matrix.get_col(0);
    p2 = camera_matrix.get_col(1);
    p3 = camera_matrix.get_col(2);
    p4 = camera_matrix.get_col(3);

    using boost::assign::list_of;
    std::vector<Vector> v1 = list_of(p2)(p3)(p4);
    std::vector<Vector> v2 = list_of(p1)(p3)(p4);
    std::vector<Vector> v3 = list_of(p1)(p2)(p4);
    std::vector<Vector> v4 = list_of(p1)(p2)(p3);

    c[0] = det(create_matrix_from_columns(v1));
    c[1] = -det(create_matrix_from_columns(v2));
    c[2] = det(create_matrix_from_columns(v3));
    c /= -det(create_matrix_from_columns(v4));
    
    // convert to world origin in camera frame
    translation = -rotation * c;
}

} // namespace kjb

