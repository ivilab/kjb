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

/* $Id: box_trajectory.cpp 15947 2013-10-30 06:18:23Z ernesto $ */

#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <people_tracking_cpp/pt_body_2d.h>
#include <people_tracking_cpp/pt_box_trajectory.h>
#include <m_cpp/m_vector_d.h>
#include <l_cpp/l_test.h>
#include <detector_cpp/d_bbox.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace kjb;
using namespace pt;
using namespace std;

/**
 * @brief   Helper function that compares two boxes for equality.
 */
inline
bool equal_box(const Bbox& b1, const Bbox& b2)
{
    return b1.get_center() == b2.get_center()
            && fabs(b1.get_height() - b2.get_height()) < DBL_EPSILON
            && fabs(b1.get_width() - b2.get_width()) < DBL_EPSILON; 
}

/**
 * @brief   Helper function that compares the contents of two boost::optional's.
 */
inline
bool equal_optional
(
    const boost::optional<Box_trajectory_element>& ob1, 
    const boost::optional<Box_trajectory_element>& ob2
)
{
    return equal_box(ob1->value, ob2->value);
}

/** @brief  Main =). */
int main(int argc, char** argv)
{
    // create trajectory
    Box_trajectory btraj1(10, 1.0, 0.5, 0.25);
    const double width = 100.0;
    const double height = 200.0;
    for(size_t i = 0; i < btraj1.size(); i++)
    {
        btraj1[i] = Bbox(Vector(i*10.0, i*10.0), width, height);
    }

    // write trajectory to (temporary string-) stream
    string fpath = "output/box_trajectory_cpp.txt";
    btraj1.write(fpath);

    // read it back into another trajectory
    ifstream ifs(fpath.c_str());
    if(!ifs)
    {
        cout << "Test failed! "
             << "(Cannot read file '" << fpath << "')"
             << endl;
        return EXIT_FAILURE;
    }

    Box_trajectory btraj2;
    btraj2.parse(ifs);
    ifs.close();

    // test box equality
    TEST_TRUE(equal(btraj1.begin(), btraj1.end(),
                    btraj2.begin(), equal_optional));


    // create trajectory
    Trajectory traj(10, 1.0, 0.5, 0.25);
    for(size_t i = 0; i < traj.size(); i++)
    {
        double ii = static_cast<double>(i);
        traj[i] = Complete_state(Vector3(0.0, 0.0, ii), ii, Vector2(ii, ii));
    }

    Perspective_camera camera(1.0, 100.0); 
    camera.set_camera_centre(Vector(0.0, 2.5, 0.0));
    camera.set_focal_length(500.0);
    camera.set_pitch(M_PI/10.0);

    // Test get_body_box_trajectory(traj, camera)
    Box_trajectory btraj = get_body_box_trajectory(traj, camera);

    TEST_TRUE(btraj.size() == traj.size());
    size_t length = traj.size();
    for(size_t i = 0; i < length; i++)
    {
        if(traj[i])
        {
            TEST_TRUE(btraj[i]);
            Body_2d b2d = project_cstate(traj[i]->value, camera,
                                         traj.height, traj.width, traj.girth);
            const Bbox& pbox = b2d.full_bbox;

            TEST_TRUE(equal_box(pbox, btraj[i]->value));
        }
    }

    // Test get_face_box_trajectory(traj, camera)
    Box_trajectory ftraj = get_face_box_trajectory(traj, camera);
    TEST_TRUE(ftraj.size() == traj.size());
    for(size_t i = 0; i < length; i++)
    {
        if(traj[i])
        {
            TEST_TRUE(ftraj[i]);
        }
    }

    RETURN_VICTORIOUSLY();
}

