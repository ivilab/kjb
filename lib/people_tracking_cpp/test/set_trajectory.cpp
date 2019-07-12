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

/* $Id$ */

#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_association.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_complete_state.h>
#include <people_tracking_cpp/pt_body_2d.h>
#include <people_tracking_cpp/pt_face_2d.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <people_tracking_cpp/pt_body_2d_trajectory.h>
#include <people_tracking_cpp/pt_face_2d_trajectory.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <detector_cpp/d_bbox.h>
#include <st_cpp/st_perspective_camera.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>
#include <l_cpp/l_exception.h>
#include "utils.h"
#include <algorithm>
#include <boost/foreach.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::pt;
using namespace boost;

bool VERBOSE = false;
bool VERBOSE2 = false;

/**
 * @brief   Helper function that compares two boxes for equality.
 */
bool equal_box(const Bbox& b1, const Bbox& b2);

/**
 * @brief   Helper function that compares two CS for equality.
 */
bool equal_cs(const Complete_state& c1, const Complete_state& c2);

/**
 * @brief   Helper function that compares two face features for equality.
 */
bool equal_ff(const Vector& ff1, const Vector& ff2);

/**
 * @brief   Helper function that compares two Body_2d for equality.
 */
bool equal_b2d(const Body_2d& b1, const Body_2d& b2);

/**
 * @brief   Helper function that compares two Face_2d for equality.
 */
bool equal_f2d(const Face_2d& f1, const Face_2d& f2);

/**
 * @brief   Helper function that compares two boxes for equality.
 */
bool equal_target(const Target& t1, const Target& t2);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    if(VERBOSE2) VERBOSE = true;

    try
    {
        // CREATE SCENES
        size_t num_frames = 40;
        double img_width = 500;
        double img_height = 500;

        Box_data data(img_width, img_height, 0.99);
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        Facemark_data fm_data(num_frames);

        Scene scene1(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);
        create_or_read_scene(
            argc, argv, num_frames, img_width, img_height,
            data, fm_data, flows_x, flows_y, scene1, 3);

        const Ascn& w1 = scene1.association;

        Scene scene2 = scene1;
        const Ascn& w2 = scene2.association;
        const Perspective_camera& camera2 = scene2.camera;

        if(VERBOSE2)
        {
            cout << "=================================== "
                 << "BEFORE CHANGES"
                 << " ==================================="
                 << endl << endl;
        }

        equal(scene1.association.begin(), scene1.association.end(),
              scene2.association.begin(), equal_target);

        if(VERBOSE)
        {
            cout << "Testing ST...\n";
        }

        // TEST SINGLE CHANGE
        Vector3 dv(0.1, 0.0, 0.1);
        double db = 0.1;
        Vector2 df(0.1, 0.1);
        Ascn::iterator tg_p1 = w1.begin();
        Ascn::iterator tg_p2 = w2.begin();
        for(; tg_p1 != w1.end() && tg_p2 != w2.end(); tg_p1++, tg_p2++)
        {
            int sf1 = tg_p1->get_start_time();
            int ef1 = tg_p1->get_end_time();
            int sf2 = tg_p2->get_start_time();
            int ef2 = tg_p2->get_end_time();
            assert(sf1 != -1 && ef1 != -1 && sf1 == sf2 && ef1 == ef2);

            for(size_t t = sf1; t <= ef1; t++)
            {
                if(VERBOSE)
                {
                    cout << "=================================== "
                         << "FRAME: " << t
                         << " ==================================="
                         << endl << endl;
                }

                //// position
                if(VERBOSE2)
                {
                    cout << "--------------- POSITION: --------------\n\n";
                }

                // do target 1
                move_trajectory_at_frame(scene1, *tg_p1, t, dv, false);

                // do target 2
                tg_p2->trajectory()[t - 1]->value.position += dv;
                tg_p2->update_boxes(camera2);
                tg_p2->update_faces(camera2);
                update_visibilities(scene2);

                TEST_TRUE(equal_target(*tg_p1, *tg_p2));

                //// body_dir
                if(VERBOSE2)
                {
                    cout << "--------------- BODY_DIR: --------------\n\n";
                }

                // do target 1
                move_trajectory_dir_at_frame(scene1, *tg_p1, t, db, false);

                // do target 2
                tg_p2->trajectory()[t - 1]->value.body_dir += db;
                tg_p2->update_boxes(camera2);
                tg_p2->update_faces(camera2);
                update_visibilities(scene2);

                TEST_TRUE(equal_target(*tg_p1, *tg_p2));

                //// face_dir
                if(VERBOSE2)
                {
                    cout << "--------------- FACE_DIR: --------------\n\n";
                }

                // do target 1
                move_trajectory_face_dir_at_frame(scene1, *tg_p1, t, df, false);

                // do target 2
                tg_p2->trajectory()[t - 1]->value.face_dir += df;
                tg_p2->update_boxes(camera2);
                tg_p2->update_faces(camera2);
                update_visibilities(scene2);

                TEST_TRUE(equal_target(*tg_p1, *tg_p2));
            }
        }

        if(VERBOSE)
        {
            cout << "Tested ST!\n";
        }

        // TEST DOUBLE CHANGE
        Vector3 dv1(0.1, 0.0, 0.1);
        Vector3 dv2(0.1, 0.0, 0.1);
        double db1 = 0.1;
        double db2 = 0.1;
        Vector2 df1(0.1, 0.1);
        Vector2 df2(0.1, 0.1);
        Ascn::iterator tg_p11 = w1.begin();
        Ascn::iterator tg_p12 = w2.begin();
        for(; tg_p11 != w1.end(); tg_p11++, tg_p12++)
        {
            int sf11 = tg_p11->get_start_time();
            int ef11 = tg_p11->get_end_time();
            int sf12 = tg_p12->get_start_time();
            int ef12 = tg_p12->get_end_time();
            assert(sf11 != -1 && ef11 != -1 && sf11 == sf12 && ef11 == ef12);

            for(size_t t1 = sf11; t1 <= ef11; t1++)
            {
                Ascn::iterator tg_p21 = w1.begin();
                Ascn::iterator tg_p22 = w2.begin();
                for(; tg_p21 != w1.end(); tg_p21++, tg_p22++)
                {
                    int sf21 = tg_p21->get_start_time();
                    int ef21 = tg_p21->get_end_time();
                    int sf22 = tg_p22->get_start_time();
                    int ef22 = tg_p22->get_end_time();
                    assert(sf21 != -1 && ef21 != -1
                            && sf21 == sf22 && ef21 == ef22);

                    for(size_t t2 = sf21; t2 <= ef21; t2++)
                    {
                        if(tg_p11 == tg_p21 && t1 == t2) continue;

                        if(VERBOSE)
                        {
                            cout << "=================================== "
                                 << "FRAME: " << t1 << ", " << t2
                                 << " ==================================="
                                 << endl << endl;
                        }

                        //// position
                        if(VERBOSE2)
                        {
                            cout << "--------------- POSITION:"
                                 << " --------------\n\n";
                        }

                        // do target 1
                        move_trajectories_at_frames(
                            scene1, *tg_p11, *tg_p21, t1, t2, dv1, dv2, false);

                        // do target 2
                        tg_p12->trajectory()[t1 - 1]->value.position += dv1;
                        tg_p22->trajectory()[t2 - 1]->value.position += dv2;
                        tg_p12->update_boxes(camera2);
                        tg_p22->update_boxes(camera2);
                        tg_p12->update_faces(camera2);
                        tg_p22->update_faces(camera2);
                        update_visibilities(scene2);

                        TEST_TRUE(equal_target(*tg_p11, *tg_p12));
                        TEST_TRUE(equal_target(*tg_p21, *tg_p22));

                        //// body_dir
                        if(VERBOSE2)
                        {
                            cout << "--------------- BODY_DIR:"
                                 << " --------------\n\n";
                        }

                        // do target 1
                        move_trajectory_dirs_at_frames(
                            scene1, *tg_p11, *tg_p21, t1, t2, db1, db2, false);

                        // do target 2
                        tg_p12->trajectory()[t1 - 1]->value.body_dir += db1;
                        tg_p22->trajectory()[t2 - 1]->value.body_dir += db2;
                        tg_p12->update_boxes(camera2);
                        tg_p22->update_boxes(camera2);
                        tg_p12->update_faces(camera2);
                        tg_p22->update_faces(camera2);
                        update_visibilities(scene2);

                        TEST_TRUE(equal_target(*tg_p11, *tg_p12));
                        TEST_TRUE(equal_target(*tg_p21, *tg_p22));

                        //// face_dir
                        if(VERBOSE2)
                        {
                            cout << "--------------- FACE_DIR:"
                                 << " --------------\n\n";
                        }

                        // do target 1
                        move_trajectory_face_dirs_at_frames(
                            scene1, *tg_p11, *tg_p21, t1, t2, df1, df2, false);

                        // do target 2
                        tg_p12->trajectory()[t1 - 1]->value.face_dir += df1;
                        tg_p22->trajectory()[t2 - 1]->value.face_dir += df2;
                        tg_p12->update_boxes(camera2);
                        tg_p22->update_boxes(camera2);
                        tg_p12->update_faces(camera2);
                        tg_p22->update_faces(camera2);
                        update_visibilities(scene2);

                        TEST_TRUE(equal_target(*tg_p11, *tg_p12));
                        TEST_TRUE(equal_target(*tg_p21, *tg_p22));
                    }
                }
            }
        }
    }
    catch(const kjb::Exception& ex)
    {
        ex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool equal_box(const Bbox& b1, const Bbox& b2)
{
    return b1.get_center() == b2.get_center()
            && fabs(b1.get_height() - b2.get_height()) < DBL_EPSILON
            && fabs(b1.get_width() - b2.get_width()) < DBL_EPSILON; 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool equal_cs(const Complete_state& c1, const Complete_state& c2)
{
    if(VERBOSE2)
    {
        cout << "(" << c1.position << "), "
             << c1.body_dir << ", "
             << "(" << c1.face_dir << ")" << endl;
        cout << "(" << c2.position << "), "
             << c2.body_dir << ", "
             << "(" << c2.face_dir << ")" << endl;
        cout << endl;
    }

    return fabs(c1.position[0] - c2.position[0]) < DBL_EPSILON
            && fabs(c1.position[1] - c2.position[1]) < DBL_EPSILON
            && fabs(c1.position[2] - c2.position[2]) < DBL_EPSILON
            && fabs(c1.body_dir - c2.body_dir) < DBL_EPSILON
            && fabs(c1.face_dir[0] - c2.face_dir[0]) < DBL_EPSILON
            && fabs(c1.face_dir[1] - c2.face_dir[1]) < DBL_EPSILON;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool equal_ff(const Vector& ff1, const Vector& ff2)
{
    bool f1e = ff1.empty();
    bool f2e = ff2.empty();

    if(f1e != f2e) return false;

    if(f1e && f2e) return true;

    return max_abs_difference(ff1, ff2) < DBL_EPSILON;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool equal_b2d(const Body_2d& b1, const Body_2d& b2)
{
    if(VERBOSE2)
    {
        cout << "(" << b1.full_bbox.get_center() << ": "
             << b1.full_bbox.get_width() << " x "
             << b1.full_bbox.get_height() << "), "
             << "(" << b1.body_bbox.get_center() << ": "
             << b1.body_bbox.get_width() << " x "
             << b1.body_bbox.get_height() << "),"
             << "(" << b1.visibility.visible_cells.size() << ", "
             << b1.visibility.visible << ", "
             << b1.visibility.cell_width << ", "
             << b1.visibility.cell_height << ")" << endl;
        cout << "(" << b2.full_bbox.get_center() << ": "
             << b2.full_bbox.get_width() << " x "
             << b2.full_bbox.get_height() << "), "
             << "(" << b2.body_bbox.get_center() << ": "
             << b2.body_bbox.get_width() << " x "
             << b2.body_bbox.get_height() << "),"
             << "(" << b2.visibility.visible_cells.size() << ", "
             << b2.visibility.visible << ", "
             << b2.visibility.cell_width << ", "
             << b2.visibility.cell_height << ")" << endl;
        cout << endl;
    }

    return equal_box(b1.full_bbox, b2.full_bbox)
            && equal_box(b1.body_bbox, b2.body_bbox)
            && equal(b1.visibility.visible_cells.begin(),
                     b1.visibility.visible_cells.end(),
                     b2.visibility.visible_cells.begin())
            && fabs(b1.visibility.visible - b2.visibility.visible) < DBL_EPSILON
            && fabs(b1.visibility.cell_width
                    - b2.visibility.cell_width) < DBL_EPSILON
            && fabs(b1.visibility.cell_height
                    - b2.visibility.cell_height) < DBL_EPSILON;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool equal_f2d(const Face_2d& f1, const Face_2d& f2)
{
    if(VERBOSE2)
    {
        cout << "(" << f1.bbox.get_center() << ": "
             << f1.bbox.get_width() << " x "
             << f1.bbox.get_height() << "), "
             << "(" << f1.left_eye << "), "
             << "(" << f1.right_eye << "), "
             << "(" << f1.nose << "), "
             << "(" << f1.left_mouth << "), "
             << "(" << f1.right_mouth << "), "
             << "(" << f1.model_dir << "), "
             << "(" << f1.visibility.visible_cells.size() << ", "
             << f1.visibility.visible << ", "
             << f1.visibility.cell_width << ", "
             << f1.visibility.cell_height << ")" << endl;
        cout << "(" << f2.bbox.get_center() << ": "
             << f2.bbox.get_width() << " x "
             << f2.bbox.get_height() << "), "
             << "(" << f2.left_eye << "), "
             << "(" << f2.right_eye << "), "
             << "(" << f2.nose << "), "
             << "(" << f2.left_mouth << "), "
             << "(" << f2.right_mouth << "), "
             << "(" << f2.model_dir << "), "
             << "(" << f2.visibility.visible_cells.size() << ", "
             << f2.visibility.visible << ", "
             << f2.visibility.cell_width << ", "
             << f2.visibility.cell_height << ")" << endl;
        cout << endl;
    }

    return equal_box(f1.bbox, f2.bbox)
            && equal_ff(f1.left_eye, f2.left_eye)
            && equal_ff(f1.right_eye, f2.right_eye)
            && equal_ff(f1.nose, f2.nose)
            && equal_ff(f1.left_mouth, f2.left_mouth)
            && equal_ff(f1.right_mouth, f2.right_mouth)
            && equal_ff(f1.model_dir, f2.model_dir)
            && equal(f1.visibility.visible_cells.begin(),
                     f1.visibility.visible_cells.end(),
                     f2.visibility.visible_cells.begin())
            && fabs(f1.visibility.visible - f2.visibility.visible) < DBL_EPSILON
            && fabs(f1.visibility.cell_width
                    - f2.visibility.cell_width) < DBL_EPSILON
            && fabs(f1.visibility.cell_height
                    - f2.visibility.cell_height) < DBL_EPSILON;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool equal_target(const Target& t1, const Target& t2)
{
    const Trajectory& traj1 = t1.trajectory();
    const Trajectory& traj2 = t2.trajectory();
    const Body_2d_trajectory& btraj1 = t1.body_trajectory();
    const Body_2d_trajectory& btraj2 = t2.body_trajectory();
    const Face_2d_trajectory& ftraj1 = t1.face_trajectory();
    const Face_2d_trajectory& ftraj2 = t2.face_trajectory();

    int sf1 = t1.get_start_time();
    int ef1 = t1.get_end_time();
    for(size_t s = sf1; s <= ef1; s++)
    {
        assert(btraj1[s - 1]);
        assert(btraj2[s - 1]);
        if(!(equal_cs(traj1[s - 1]->value, traj2[s - 1]->value)
                && equal_b2d(btraj1[s - 1]->value, btraj2[s - 1]->value)
                && equal_f2d(ftraj1[s - 1]->value, ftraj2[s - 1]->value)))
        {
            return false;
        }
    }

    return true;
}

