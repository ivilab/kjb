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

#include "people_tracking_cpp/pt_body_2d.h"
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

Body_2d kjb::pt::project_cstate
(
    const Complete_state& cs,
    const Perspective_camera& cam,
    double height,
    double width,
    double girth
)
{
    Vector_vec fpts;
    Vector_vec bpts;
    Vector botc;
    Vector topc;
    Vector bodc;
    cylinder_points(cs, height, width, girth, fpts, bpts, botc, topc, bodc);

    // project points
    std::transform(fpts.begin(), fpts.end(), fpts.begin(),
                   boost::bind(project_point, boost::cref(cam), _1));

    std::transform(bpts.begin(), bpts.end(), bpts.begin(),
                   boost::bind(project_point, boost::cref(cam), _1));

    // project top and bottom of cylinder
    botc = project_point(cam, botc);
    topc = project_point(cam, topc);
    bodc = project_point(cam, bodc);

    // get bounding boxes
    Bbox fbox = cylinder_bounding_box(fpts, botc, topc);
    Bbox bbox = cylinder_bounding_box(bpts, botc, bodc);

    // WARNING: still need to get model direction
    return Body_2d(fbox, bbox);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::pt::model_direction
(
    const Complete_state& cs1,
    const Complete_state& cs2,
    const Perspective_camera& cam,
    double height,
    double width,
    double girth
)
{
    // camera center
    Vector cc = cam.get_camera_centre();
    cc.resize(3.0);

    // compute visible point on body
    Vector bc1(cs1.position.begin(), cs1.position.end());
    Vector vc = cc - bc1;

    // compute angle on body
    double ang = body_direction(vc);
    double a1 = ang - cs1.body_dir;

    // project both points
    Vector p1 = ellipse_point(cs1, height/2.0, width, girth, a1);
    Vector p2 = ellipse_point(cs2, height/2.0, width, girth, a1);

    p1 = project_point(cam, p1);
    p2 = project_point(cam, p2);

    return p2 - p1;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Bbox kjb::pt::cylinder_bounding_box
(
    const Vector_vec& pts,
    const Vector& bot,
    const Vector& top
)
{
    // find extremes
    Vector left = *std::min_element(pts.begin(), pts.end(),
                                    Index_less_than<Vector>(0));
    Vector right = *std::max_element(pts.begin(), pts.end(),
                                     Index_less_than<Vector>(0));

    // compute box
    Vector center;
    center.set((left[0] + right[0]) / 2.0, (bot[1] + top[1]) / 2.0);
    return Bbox(center, right[0] - left[0], top[1] - bot[1]);
}

