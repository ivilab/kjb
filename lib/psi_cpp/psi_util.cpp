/* $Id: psi_util.cpp 18331 2014-12-02 04:30:55Z ksimek $ */
/* {{{=========================================================================== *
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker
#include <psi_cpp/psi_util.h>
#include <camera_cpp/camera_backproject.h>
#include <detector_cpp/d_deva_detection.h>
#include <people_tracking_cpp/pt_util.h>
#include <boost/assign.hpp>

#include <g_cpp/g_quaternion.h>
#include <l_cpp/l_util.h>
#include <l/l_sys_lib.h>
#include <l/l_word_list.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>
 
#ifdef KJB_HAVE_UA_CARTWHEEL
#include <Core/Visualization.h>
#endif

namespace kjb
{
namespace psi
{

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

#ifdef KJB_HAVE_UA_CARTWHEEL
void set_camera(CartWheel::Visualization& vis, const kjb::Perspective_camera& cam, double WIDTH, double HEIGHT)
{
    using kjb::Vector;
    using kjb::Quaternion;

    vis.setWidth(WIDTH);
    vis.setHeight(HEIGHT);

    const Quaternion& cam_to_world = cam.get_orientation().conj();
    Vector up = cam_to_world.rotate(Vector(0.0, 1.0, 0.0));
    Vector center =  kjb::geometry::projective_to_euclidean(cam.get_camera_centre());
    Vector target = center + cam_to_world.rotate(Vector(0.0, 0.0, -10.0));

    vis.setCameraLocation(to_cw_pt_3d(center));
    vis.setCameraUp(to_cw_pt_3d(up));
    vis.setCameraTarget(to_cw_pt_3d(target));

    double f = cam.get_focal_length();
//    double w = vis.getWidth();
    double h = vis.getHeight();

    // use h or w here?
    double factor = 1;
    double fovy = factor * 2.0 * atan2(h/2, f) * 180.0 / M_PI;
    vis.setCameraFovy(fovy);
}
#endif

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::map<std::string, Simulator_type> simulator_name_map = boost::assign::map_list_of
        (static_cast<std::string>("cartwheel"), CARTWHEEL_SIMULATOR)
        ("cylinder",  CYLINDER_SIMULATOR);

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

const std::string& get_name(Simulator_type type)
{
    static std::map<Simulator_type, std::string> type_map;

    if(type_map.size() == 0)
    {
        std::map<std::string, Simulator_type>::const_iterator it = simulator_name_map.begin();

        for(; it != simulator_name_map.end(); it++)
        {
            type_map[it->second] = it->first;
        }
    }

    return type_map[type];
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::istream& operator>>(std::istream& ist, Simulator_type& type)
{
    std::string token;
    ist >> token;
    if(simulator_name_map.count(token) == 0)
    {
        KJB_THROW_2(IO_error, "Uknown simulator_type name");
    }
    type = simulator_name_map[token];

    return ist;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::ostream& operator<<(std::ostream& ost, Simulator_type type)
{
    const std::string& name = get_name(type);

    ost << name;
    return ost;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void standardize(Deva_detection& boxes, double cam_width, double cam_height)
{
    for(size_t i = 0; i < boxes.size(); i++)
    {
        pt::standardize(boxes[i], cam_width, cam_height);
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void prune_by_height
(
    std::vector<Deva_detection>& deva_boxes,
    double screen_width,
    double screen_height,
    const Perspective_camera& camera, 
    double average_height
)
{
    Ground_back_projector back_project(camera, 0.0);
    for(size_t j = 0; j < deva_boxes.size(); j++)
    {
        // Use the pruned deva boxes 
        psi::Bbox sbox(deva_boxes[j].full_body());
        pt::standardize(sbox, screen_width, screen_height);
        double box_height = get_3d_height(sbox.get_bottom_center(), 
                sbox.get_top_center(), camera); 
        double max_height = average_height*2.0;
        double min_height = average_height/2.0;
        if(box_height <= max_height && box_height >= min_height)
        {
            kjb::Vector pt3d = back_project(sbox.get_bottom_center()[0],
                                            sbox.get_bottom_center()[1]);
            if(pt3d.empty())
            {
                std::vector<Deva_detection>::iterator box_j_p = deva_boxes.begin();
                box_j_p += (j--);
                deva_boxes.erase(box_j_p);
            }
        }
        else
        {
            std::vector<Deva_detection>::iterator box_j_p = deva_boxes.begin();
            box_j_p += (j--);
            deva_boxes.erase(box_j_p);
        }
    }
}

} // namespace psi
} // namespace kjb

