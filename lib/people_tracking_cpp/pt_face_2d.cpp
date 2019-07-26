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

#include "people_tracking_cpp/pt_face_2d.h"
#include "people_tracking_cpp/pt_complete_state.h"
#include "people_tracking_cpp/pt_util.h"
#include "detector_cpp/d_bbox.h"
#include "m_cpp/m_vector.h"
#include "camera_cpp/perspective_camera.h"
#include "g_cpp/g_util.h"

#include <vector>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

using namespace kjb;
using namespace kjb::pt;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Face_2d kjb::pt::project_cstate_face
(
    const Complete_state& cs,
    const Perspective_camera& cam,
    double height,
    double width,
    double girth
)
{
    // face box
    Vector_vec hpts = head_points(cs, height, width, girth);
    std::transform(hpts.begin(), hpts.end(), hpts.begin(),
                   boost::bind(project_point, boost::cref(cam), _1));

    Bbox head_box = compute_bounding_box(hpts.begin(), hpts.end());

    // face features
    Vector_vec ffts = face_features(cs, height, width, girth);
    std::transform(ffts.begin(), ffts.end(), ffts.begin(),
                   boost::bind(project_point, boost::cref(cam), _1));

    Vector& leye = ffts[0];
    Vector& reye = ffts[1];
    Vector& nose = ffts[2];
    Vector& lmouth = ffts[3];
    Vector& rmouth = ffts[4];

    // erase occluded features
    IFT(cs.face_dir[0] != std::numeric_limits<double>::max() &&
        cs.body_dir != std::numeric_limits<double>::max(),
        Runtime_error,
        "face direction is not initialized properly");
    double fd = cs.face_dir[0] + cs.body_dir;
    while(fd > M_PI) fd -= M_PI;
    while(fd < -M_PI) fd += M_PI;
    if(fd >= 2*M_PI/3.0 || fd <= -2*M_PI/3.0)
    {
        leye.clear();
        reye.clear();
        nose.clear();
        lmouth.clear();
        rmouth.clear();
    }
    else if(fd >= M_PI/3.0)
    {
        reye.clear();
        rmouth.clear();
    }
    else if(fd <= -M_PI/3.0)
    {
        leye.clear();
        lmouth.clear();
    }

    return Face_2d(head_box, leye, reye, nose, lmouth, rmouth);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::pt::face_model_direction
(
    const Complete_state& cs1,
    const Complete_state& cs2,
    const Perspective_camera& cam,
    double height,
    double width,
    double girth
)
{
    // point on head
    Vector cam_center = cam.get_camera_centre();
    cam_center.resize(3.0);
    Vector hp = visible_head_point(cs1, cam_center, height, width, girth);

    // point in world in two frames
    Head head1(cs1, height, width, girth);
    Head head2(cs2, height, width, girth);

    Vector p1 = head_to_world(hp, head1, true);
    Vector p2 = head_to_world(hp, head2, true);

    p1 = project_point(cam, p1);
    p2 = project_point(cam, p2);

    return p2 - p1;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double kjb::pt::face_distance
(
    const Face_2d& face_model,
    const Deva_facemark facemark
)
{
    const Vector& nose_mark = facemark.nose_mark();
    return vector_distance(face_model.bbox.get_center(), nose_mark);
}

