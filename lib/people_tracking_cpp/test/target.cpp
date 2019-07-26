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

/* $Id: target.cpp 18677 2015-03-19 19:15:14Z jguan1 $ */

#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <detector_cpp/d_bbox.h>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_vector_d.h>
#include <l_cpp/l_test.h>
#include <st_cpp/st_backproject.h>
#include <st_cpp/st_perspective_camera.h>
#include <vector>
#include <utility>

using namespace kjb;
using namespace pt;
using namespace std;
const double noise_sigma = 1e-3;

/** @brief  Helper function that makes sure trajectory is OK. */
bool confirm_trajectory(const Trajectory& trj1, const vector<Vector>& trj2)
{
    if(trj1.size() != trj2.size())
    {
        return false;
    }

    for(size_t i = 0; i < trj1.size(); i++)
    {
        const Vector3& pos = trj1[i]->value.position;
        if(!trj1[i] || vector_distance(pos, Vector3(trj2[i])) > noise_sigma * 10.0)
        {
            return false;
        }
    }

    return true;
}

/** @brief  Main, baby! */
int main(int argc, char** argv)
{
    const size_t data_size = 5;
    const double x_inc = 0.1;

    // create data
    Bbox base_box(Vector(0.0, 0.0), 1/6.0, 1.0);
    vector<Detection_box> data;
    for(size_t i = 0; i < data_size; i++)
    {
        Bbox box(base_box.get_center() + Vector().set(i*x_inc, 0.0),
                 base_box.get_width(), base_box.get_height());

        /*if(i == 3)
        {
            box.set_center(box.get_center() + Vector().set(0.0, 0.2));
        }*/

        data.push_back(Detection_box(box, 0.0, "ground_truth"));
    }

    // create track
    Target target(1.75, 0.4, 0.3, data_size);
    target.insert(make_pair(1, &data[0]));
    target.insert(make_pair(2, &data[1]));
    target.insert(make_pair(5, &data[4]));

    // easy camera
    Perspective_camera cam;
    cam.set_focal_length(1.0);
    cam.set_camera_centre_y(1.0);

    // automatically back-project and fill in
    target.set_changed_all();
    target.estimate_trajectory(cam, noise_sigma, noise_sigma);

    // manually back-project and fill in
    Ground_back_projector backproject(cam, 0.0);
    vector<Vector> tr(data_size);

    Target::const_iterator pair_p = target.begin();
    tr[0] = backproject(pair_p->second->bbox.get_bottom_center()[0],
                        pair_p->second->bbox.get_bottom_center()[1]);

    pair_p++;
    tr[1] = backproject(pair_p->second->bbox.get_bottom_center()[0],
                        pair_p->second->bbox.get_bottom_center()[1]);

    tr[2].set(0.4, 0.0, -2.0);
    tr[3].set(0.6, 0.0, -2.0);

    pair_p++;
    tr[4] = backproject(pair_p->second->bbox.get_bottom_center()[0],
                        pair_p->second->bbox.get_bottom_center()[1]);

    // compare
    TEST_TRUE(confirm_trajectory(target.trajectory(), tr));

    // add two more boxes
    target.insert(make_pair(3, &data[2]));
    target.insert(make_pair(4, &data[3]));

    target.set_changed_start(3);
    target.set_changed_end(4);

    // automatically back-project
    target.estimate_trajectory(cam, noise_sigma, noise_sigma);

    // compare
    TEST_TRUE(confirm_trajectory(target.trajectory(), tr));

    RETURN_VICTORIOUSLY();
}

