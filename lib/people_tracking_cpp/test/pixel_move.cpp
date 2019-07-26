/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
| Authors:
|     Ernesto Brau
|
* =========================================================================== */

/* $Id: pixel_move.cpp 15396 2013-09-23 17:29:19Z ernesto $ */

#include <people_tracking_cpp/pt_scene_diff.h>
#include <st_cpp/st_perspective_camera.h>
#include <g_cpp/g_util.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_test.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <vector>

using namespace kjb;
using namespace kjb::pt;
using namespace std;

/** @brief  Returns true if r + (ex, ez) == s in pixel space. */
bool moved_by_epsilon
(
    const Vector& r,
    const Vector& s,
    const Matrix& C,
    double ex,
    double ey
);

/** @brief  Main. */
int main(int argc, char** argv)
{
    // make camera
    Perspective_camera camera; 
    camera.set_camera_centre(Vector(0.0, 2.5, 0.0));
    camera.set_focal_length(3 * 500.0 / 2.0);
    camera.set_pitch(M_PI/10.0);
    Matrix C = camera.build_camera_matrix();

    // make 3D point
    Uniform_distribution U_x(-5.0, 5.0);
    Uniform_distribution U_z(-10.0, -1.0);
    vector<Vector> rs(1000);
    for(size_t i = 0; i < rs.size(); i++)
    {
        rs[i].set(sample(U_x), sample(U_z));
    }

    // pixel-move functor
    Pixel_move move_point(C);
    const double e = 1.0;
    Vector s;

    //// BEGIN TESTS
    for(size_t i = 0; i < rs.size(); i++)
    {
        const Vector& r = rs[i];
        move_point.set_point(r);

        // move point right (in image plane)
        s = move_point.x(e);
        TEST_TRUE(moved_by_epsilon(r, s, C, e, 0.0));

        // move point left (in image plane)
        s = move_point.x(-e);
        TEST_TRUE(moved_by_epsilon(r, s, C, -e, 0.0));

        // move point up (in image plane)
        s = move_point.y(e);
        TEST_TRUE(moved_by_epsilon(r, s, C, 0.0, e));

        // move point down (in image plane)
        s = move_point.y(-e);
        TEST_TRUE(moved_by_epsilon(r, s, C, 0.0, -e));
    }

    RETURN_VICTORIOUSLY();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool moved_by_epsilon
(
    const Vector& r,
    const Vector& s,
    const Matrix& C,
    double ex,
    double ey
)
{
    Vector rr = r;
    Vector ss = s;

    rr.set(r[0], 0.0, r[1], 1.0);
    ss.set(s[0], 0.0, s[1], 1.0);

    Vector pr = geometry::projective_to_euclidean_2d(C*rr);
    Vector ps = geometry::projective_to_euclidean_2d(C*ss);

    return (fabs(pr[0] + ex - ps[0]) <= FLT_EPSILON
                && fabs(pr[1] + ey - ps[1]) <= FLT_EPSILON);
}

