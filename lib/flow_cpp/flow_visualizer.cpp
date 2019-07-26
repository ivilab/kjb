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

/* $Id: flow_visualizer.cpp 14083 2013-03-12 18:41:07Z jguan1 $ */

#include <flow_cpp/flow_visualizer.h>
#include <i_cpp/i_image.h>
#include <m_cpp/m_vector.h>

#include <vector>

using namespace kjb;

void kjb::draw_features
(
    Image& img, 
    const std::vector<Feature_pair>& features, 
    const Vector& average_flow,
    Image::Pixel_type feature_pixel,
    Image::Pixel_type ave_flow_pixel
)
{
    if(features.size() > 0)
    {
        Vector cur_feature_center(2, 0.0);
        for(size_t i = 0; i < features.size(); i++)
        {
            const Vector& cur_feature = features[i].first;
            const Vector& next_feature = features[i].second; 
            cur_feature_center += cur_feature;
            img.draw_line_segment(cur_feature(1), cur_feature(0),
                                  next_feature(1), next_feature(0), 2.0, feature_pixel);
        }

        // Draw the average flow features 
        cur_feature_center /= features.size();
        Vector end_point = cur_feature_center + average_flow;
        img.draw_line_segment(cur_feature_center(1), cur_feature_center(0),
                              end_point(1), end_point(0), 3.0, ave_flow_pixel);
    }
}

