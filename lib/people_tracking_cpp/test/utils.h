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
#include <flow_cpp/flow_integral_flow.h>
#include <vector>

extern const double face_sd;
extern const double scale_x;
extern const double scale_y;
extern const double bg_scale_x;
extern const double bg_scale_y;
extern const double gp_scale;
extern const double gp_svar;
extern const double gp_scale_dir;
extern const double gp_svar_dir;
extern const double gp_scale_fdir;
extern const double gp_svar_fdir;

/** @brief  Generates a typical scene. */
void make_typical_scene
(
    size_t num_frames,
    kjb::pt::Scene& scene,
    kjb::pt::Box_data& box_data,
    kjb::pt::Facemark_data& fm_data,
    std::vector<kjb::Integral_flow>& flows_x,
    std::vector<kjb::Integral_flow>& flows_y,
    size_t max_tracks = 0
);

/** @brief  Creates or reads a scene. */
std::vector<std::string> create_or_read_scene
(
    int argc,
    char** argv,
    size_t& num_frames,
    double& img_width,
    double& img_height,
    kjb::pt::Box_data& box_data,
    kjb::pt::Facemark_data& fm_data,
    std::vector<kjb::Integral_flow>& flows_x,
    std::vector<kjb::Integral_flow>& flows_y,
    kjb::pt::Scene& scene,
    size_t max_tracks = 0,
    bool read_frame = false
);

