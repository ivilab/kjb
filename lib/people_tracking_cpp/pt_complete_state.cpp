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


#include "people_tracking_cpp/pt_complete_state.h"
#include "people_tracking_cpp/pt_util.h"
#include "detector_cpp/d_bbox.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_vector_d.h"
#include "g_cpp/g_util.h"
#include "g_cpp/g_line.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/tuple/tuple.hpp>

using namespace kjb;
using namespace kjb::pt;

// statics
const double Head::HEIGHT_FRACTION = 1 / 7.0;
const double Head::WIDTH_FRACTION = 1 / 3.0;
const double Head::GIRTH_FRACTION = 0.7;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector_vec kjb::pt::body_ellipse
(
    const Complete_state& cs,
    double h,
    double w,
    double g,
    size_t n
)
{
    const double arc_size = 2 * M_PI / n;
    Vector_vec circ(n);

    for(size_t i = 0; i < n; i++)
    {
        circ[i] = (ellipse_point(cs, h, w, g, i*arc_size));
    }

    return circ;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::cylinder_points
(
    const Complete_state& cs,
    double height,
    double width,
    double girth,
    Vector_vec& fpts,
    Vector_vec& bpts,
    Vector& botc,
    Vector& topc,
    Vector& bodc
)
{
    using namespace boost;
    using namespace std;

    // compute dimensions and rotations
    double hheight = height * Head::HEIGHT_FRACTION;
    double bheight = height - hheight;

    botc.set(cs.position[0], cs.position[1], cs.position[2]);
    topc.set(cs.position[0], height, cs.position[2]);
    bodc.set(cs.position[0], bheight, cs.position[2]);

    if(fabs(botc[1] - 0.0) > FLT_EPSILON)
    {
        cerr << "WARNING: trajectory point is not on ground." << endl;
    }

    // find 3D points along rims of cylinder (for sides of box)
    // of both full and body boxes
    Vector_vec bot_pts = body_ellipse(cs, 0.0, width, girth);

    fpts.resize(2*bot_pts.size());
    copy(bot_pts.begin(), bot_pts.end(), fpts.begin());
    transform(
        bot_pts.begin(),
        bot_pts.end(),
        fpts.begin() + bot_pts.size(),
        bind(plus<Vector>(), Vector(0, height, 0), _1));

    bpts.resize(2*bot_pts.size());
    copy(bot_pts.begin(), bot_pts.end(), bpts.begin());
    transform(
        bot_pts.begin(),
        bot_pts.end(),
        bpts.begin() + bot_pts.size(),
        bind(plus<Vector>(), Vector(0, bheight, 0), _1));
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector_vec kjb::pt::head_points
(
    const Complete_state& cs,
    double height,
    double width,
    double girth
)
{
    // points on circle
    Complete_state temp_cs = cs;
    temp_cs.position = Vector3(0.0, 0.0, 0.0);
    temp_cs.body_dir = 0.0;
    Vector_vec epts = body_ellipse(temp_cs, 0.0, 2.0, 2.0);

    // number of points
    const size_t nep = epts.size();
    const size_t nes = 3;

    Head head(cs, height, width, girth);
    Vector_vec pts(nep*nes + 2);

    // top and bottom of ellipsoid
    pts[0] = head_to_world(Vector(0, 1, 0), head, true);
    pts[1] = head_to_world(Vector(0, -1, 0), head, true);

    const double edy = 2.0/(nes + 1);
    for(size_t e = 0; e < nes; e++)
    {
        double cur_y = -1 + (e+1)*edy;
        double r = sqrt(1 - cur_y*cur_y);

        Vector ept;
        for(size_t i = 0; i < nep; i++)
        {
            ept = r * epts[i];
            ept[1] = cur_y;
            pts[2 + e*nep + i] = head_to_world(ept, head, true);
        }
    }

    return pts;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector_vec kjb::pt::face_features
(
    const Complete_state& cs,
    double height,
    double width,
    double girth
)
{
    //// FIRST FEATURES ON UNIT SPHERE AT ORIGIN
    Vector_vec fts(5);

    // eyes
    double x = -1/2.0;
    double y = 0.0;
    double z = sqrt(1 - x*x - y*y);
    fts[0].set(x, y, z);

    x = 1/2.0;
    y = 0.0;
    z = sqrt(1 - x*x - y*y);
    fts[1].set(x, y, z);

    // nose
    x = 0.0;
    y = -3/8.0;
    z = sqrt(1 - x*x - y*y);
    fts[2].set(x, y, z);

    // mouth
    x = -1/3.0;
    y = -5/8.0;
    z = sqrt(1 - x*x - y*y);
    fts[3].set(x, y, z);

    x = 1/3.0;
    y = -5/8.0;
    z = sqrt(1 - x*x - y*y);
    fts[4].set(x, y, z);

    //// THEN PUT IN WORLD COORDINATES
    Head head(cs, height, width, girth);
    std::transform(fts.begin(), fts.end(), fts.begin(),
                   boost::bind(head_to_world, _1, boost::cref(head), true));

    return fts;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::pt::visible_head_point
(
    const Complete_state& cs,
    const Vector& cam_center,
    double height,
    double width,
    double girth
)
{
    Head head(cs, height, width, girth);

    // vector from head to camera
    Vector vc = cam_center - head.center;
    vc.normalize();

    return matrix_transpose(head.orientation) * vc;

    //vc = head.orientation.conj().rotate(vc);
    //return vc;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

bool kjb::pt::looking
(
    const Complete_state& obs,
    double height,
    double width,
    double girth,
    const Vector& obj,
    double thresh,
    bool flt_obj
)
{
    const double ang = 0.08*M_PI;

    // head
    Head head(obs, height, width, girth);

    // gaze line
    const Vector& c = head.center;
    Vector nose = head_to_world(Vector(0, 0, 1), head);

    // object line
    Vector bot = obj;
    bot[1] = flt_obj ? bot[1] - 0.01 : 0.0;

    // find point of "inersection" between lines
    double u1, u2;
    boost::tie(u1, u2) = skew_lines_intersection(c, nose, bot, obj);

    if(u1 <= 0) return false;

    Vector x1 = c + u1*(nose - c);
    Vector x2 = bot + u2*(obj - bot);

    double th = std::max(tan(ang)*vector_distance(c, obj), thresh);
    if(u2 >= 0 && u2 <= 1)
    {
        return vector_distance(x1, x2) <= th;
    }
    else
    {
        return vector_distance(x1, bot) <= th || vector_distance(x1, obj) <= th;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::pt::gaze_intersection
(
    const Complete_state& cs1,
    double height1,
    double width1,
    double girth1,
    const Complete_state& cs2,
    double height2,
    double width2,
    double girth2,
    double thresh
)
{
    // heads
    Head head1(cs1, height1, width1, girth1);
    Head head2(cs2, height2, width2, girth2);

    // gaze lines
    const Vector& c1 = head1.center;
    const Vector& c2 = head2.center;
    Vector nose1 = head_to_world(Vector(0, 0, 1), head1);
    Vector nose2 = head_to_world(Vector(0, 0, 1), head2);

    // find point of "inersection" between lines
    double u1, u2;
    boost::tie(u1, u2) = skew_lines_intersection(c1, nose1, c2, nose2);

    // behind heads?
    if(u1 == DBL_MAX || u2 == DBL_MAX) return Vector();
    if(u1 <= 0 || u2 <= 0) return Vector();

    // actual points
    Vector x1 = c1 + u1*(nose1 - c1);
    Vector x2 = c2 + u2*(nose2 - c2);

    // too far apart?
    if(vector_distance(x1, x2) > thresh) return Vector();

    return (x1 + x2)/2;
}

