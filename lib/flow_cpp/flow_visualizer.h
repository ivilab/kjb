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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: flow_visualizer.h 18278 2014-11-25 01:42:10Z ksimek $ */

#ifndef KJB_FLOW_VISUALIZER_H
#define KJB_FLOW_VISUALIZER_H

#include <flow_cpp/flow_feature_set.h>
#include <i_cpp/i_image.h>

namespace kjb
{

const Image::Pixel_type apix = {255.0, 0.0, 0.0};
const Image::Pixel_type fpix = {0.0, 0.0, 255.0};

/**
 * @brief   Draw the optical flow features and the average flow 
 */
void draw_features
(
    Image& img, 
    const std::vector<Feature_pair>& features, 
    const Vector& average_flow,
    Image::Pixel_type feature_pixel = fpix,
    Image::Pixel_type ave_flow_pixel = apix
);

} // namespace kjb

#endif
