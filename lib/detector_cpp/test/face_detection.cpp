/* =========================================================================== *
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id: face_detection.cpp 15844 2013-10-23 01:33:14Z jguan1 $ */

#include <gr_cpp/gr_2D_bounding_box.h>
#include <detector_cpp/d_facecom.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>
#include <st_cpp/st_backproject.h>
#include <st_cpp/st_perspective_camera.h>

using namespace kjb;
using namespace std;

int main(int argc, char** argv)
{
    Axis_aligned_rectangle_2d 
        face_box(Vector(0.0, 5.0/32), 1.0/16, 1.0/16);
    Face_detection face(face_box);

    Perspective_camera cam;
    cam.set_focal_length(1.0);
    cam.set_camera_centre_y(1.0);

    // test 3D location
    Axis_aligned_rectangle_2d 
        body_box(Vector(0.0, -1.0/32), 7.0/32, 7/16.0);
    Vector floc = face_location_3d(face, cam, body_box);
    TEST_TRUE(floc == Vector(0.0, 13.0/8, -4.0));

    // test gaze direction
    face.yaw() = M_PI/2.0;
    Vector gdir = gaze_direction(face, cam);
    TEST_TRUE(gdir == Vector(1.0, 0.0, 0.0));

    // test gaze direction 2
    face.yaw() = M_PI/4.0;
    Vector gdir2 = gaze_direction(face, cam);
    TEST_TRUE(gdir2 == Vector(1.0, 0.0, 1.0)/sqrt(2.0));

    RETURN_VICTORIOUSLY();
}

