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
   |  Author:  Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id: pt_visibility.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_visibility.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_data.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_detection_box.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_association.h"
#include "m_cpp/m_vector_d.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_util.h"

#include <vector>
#include <list>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

namespace kjb {
namespace pt {

// depth map
using namespace boost::bimaps;
typedef bimap<const Target*, multiset_of<double> > Depth_map;

/** @brief  Computes visible of a box given other boxes. */
Visibility get_box_visibility
(
    const Bbox& box,
    double depth,
    size_t frame,
    const Depth_map& depth_map,
    double img_width,
    double img_height,
    bool infer_head
);

}} //namespace kjb::pt

using namespace kjb;
using namespace kjb::pt;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::update_visibilities(const Scene& scene, bool infer_head)
{
    if(scene.association.empty()) return;

    for(size_t fr = 1; fr <= scene.association.get_data().size(); fr++)
    {
        update_visibilities(scene, fr, infer_head);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::pt::update_visibilities
(
    const Scene& scene,
    size_t frame,
    bool infer_head
)
{
    size_t t = frame - 1;

    // create depth map for this frame
    Depth_map dmap;
    BOOST_FOREACH(const Target& target, scene.association)
    {
        Trajectory& traj = target.trajectory();
        if(traj[t])
        {
            const Vector3& pos = traj[t]->value.position;
            dmap.left.insert(Depth_map::left_value_type(&target, pos[2]));
        }
    }

    // get image dimensions
    const Box_data& box_data = (const Box_data&)scene.association.get_data();
    double iw = box_data.image_width();
    double ih = box_data.image_height();

    // compute visibilities
    BOOST_FOREACH(const Target& target, scene.association)
    {
        const Trajectory& traj = target.trajectory();
        if(!traj[t]) continue;

        // body visibility
        Body_2d_trajectory& btraj = target.body_trajectory();
        ASSERT(btraj[t]);
        double depth = target.trajectory()[t]->value.position[2];

        const Bbox& bbox = btraj[t]->value.body_bbox;
        btraj[t]->value.visibility
            = get_box_visibility(bbox, depth, frame, dmap, iw, ih, infer_head);

        // face visibility
        if(infer_head)
        {
            Face_2d_trajectory& ftraj = target.face_trajectory();
            ASSERT(ftraj[t]);
            const Bbox& fbox = ftraj[t]->value.bbox;
            ftraj[t]->value.visibility
                = get_box_visibility(fbox, depth, frame, dmap, iw, ih, 
                                     infer_head);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Visibility kjb::pt::get_box_visibility
(
    const Bbox& box,
    double depth,
    size_t frame,
    const Depth_map& depth_map,
    double img_width,
    double img_height,
    bool infer_head
)
{
    size_t t = frame - 1;

    // get box corresponding to track
    //const Bbox& box = target.body_trajectory()[t]->value.body_bbox;

    // constants
    const size_t num_subdivisions = 8; 
    const size_t num_cells = num_subdivisions * num_subdivisions; 

    // subdivide box into cell centers
    const double x_delta = box.get_width() / num_subdivisions;
    const double y_delta = box.get_height() / num_subdivisions;

    const double x_min = box.get_left() + x_delta / 2.0;
    const double y_min = box.get_bottom() + y_delta / 2.0;

    std::vector<Vector> cell_centers(num_cells, Vector(2, 0.0));
    std::list<bool> occluded;

    // make vector of cell centers, which we'll test for occlusion
    size_t i = 0;
    Bbox im_box(Vector(0.0, 0.0), img_width, img_height);
    for(size_t x_i = 0; x_i < num_subdivisions; x_i++)
    {
        for(size_t y_i = 0; y_i < num_subdivisions; y_i++, i++)
        {
            const double x = x_min + x_i * x_delta;
            const double y = y_min + y_i * y_delta;
            cell_centers[i].set(x, y);
            occluded.push_back(!im_box.contains(cell_centers[i]));
        }
    }

    //double depth = target.trajectory()[t]->value.position[2];

    // iterate over boxes in front of this one
    Visibility bvis;
    bvis.cell_width = x_delta;
    bvis.cell_height = y_delta;
    for(Depth_map::right_map::const_iterator
                                  pair_p = depth_map.right.lower_bound(depth);
                                  pair_p != depth_map.right.end();
                                  pair_p++)
    {
        const Target& cur_target = *pair_p->second;

        // do not check against self
        //if(&target == &cur_target) continue;

        // check against body boxes 
        const Bbox& cur_box = cur_target.body_trajectory()[t]->value.body_bbox;

        // do not check against self, if no intersection, skip it
        if(&cur_box == &box || !box.intersects(cur_box)) continue;

        // test each cell center for occlusion
        std::list<bool>::iterator bool_p = occluded.begin();
        BOOST_FOREACH(const Vector& cell_center, cell_centers)
        {
            // (we could use a binary search over box's area
            // here to avoid exhaustive search; but that is 
            // probably overkill unless num_divisions gets much larger)
            if(*bool_p == false && cur_box.contains(cell_center))
            {
                *bool_p = true;
            }

            *bool_p++;
        }

        // check against face boxes
        if(infer_head)
        {
            const Bbox& cur_fbox = cur_target.face_trajectory()[t]->value.bbox;

            // do not check against self, if no intersection, skip it
            if(&cur_fbox == &box || !box.intersects(cur_fbox)) continue;

            // test each cell center for occlusion
            std::list<bool>::iterator bool_p = occluded.begin();
            BOOST_FOREACH(const Vector& cell_center, cell_centers)
            {
                // (we could use a binary search over box's area
                // here to avoid exhaustive search; but that is 
                // probably overkill unless num_divisions gets much larger)
                if(*bool_p == false && cur_fbox.contains(cell_center))
                {
                    *bool_p = true;
                }

                *bool_p++;
            }
        }
    }

    bvis.visible_cells.clear();
    bvis.visible_cells.reserve(cell_centers.size());
    std::list<bool>::iterator bool_p = occluded.begin();
    for(i = 0; i < num_cells; i++, bool_p++)
    {
        if(!*bool_p)
        {
            bvis.visible_cells.push_back(cell_centers[i]);
        }
    }

    size_t num_visible = bvis.visible_cells.size();
    bvis.visible = static_cast<double>(num_visible) / num_cells;
    return bvis;
}

