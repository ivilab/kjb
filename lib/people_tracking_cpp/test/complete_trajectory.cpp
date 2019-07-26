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

/* $Id: complete_trajectory.cpp 15940 2013-10-29 04:14:54Z ernesto $ */

#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <m_cpp/m_vector_d.h>
#include <l_cpp/l_test.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace kjb;
using namespace pt;
using namespace std;

/**
 * @brief   Helper function that compares the contents of two boost::optional's.
 */
inline
bool equal_optional
(
    const boost::optional<Trajectory_element>& ocs1, 
    const boost::optional<Trajectory_element>& ocs2
)
{
    return ocs1->value.position == ocs2->value.position
            && ocs1->value.body_dir == ocs2->value.body_dir
            && ocs1->value.face_dir == ocs2->value.face_dir;
}

/** @brief  Main =). */
int main(int argc, char** argv)
{
    // create trajectory
    Trajectory traj(10, 1.0, 0.5, 0.25);
    for(size_t i = 0; i < traj.size(); i++)
    {
        double ii = static_cast<double>(i);
        traj[i] = Complete_state(Vector3(ii, 0.0, ii), ii, Vector2(ii, ii));
    }

    // write trajectory to (temporary string-) stream
    string fpath = "output/complete_trajectory_cpp.txt";
    traj.write(fpath);

    // read it back into another trajectory
    ifstream ifs(fpath.c_str());
    if(!ifs)
    {
        cout << "Test failed! "
             << "(Cannot read file '" << fpath << "')"
             << endl;
        return EXIT_FAILURE;
    }

    Trajectory traj2;
    traj2.parse(ifs);
    ifs.close();

    // test box equality
    TEST_TRUE(equal(traj.begin(), traj.end(), traj2.begin(), equal_optional)
        && traj.height == traj2.height
        && traj.width == traj2.width
        && traj.girth == traj2.girth);

    RETURN_VICTORIOUSLY();
}

