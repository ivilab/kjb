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
   |  Author:  Jinyan Guan, Ernesto Brau
 * =========================================================================== */

/* $Id: flow_integral_flow.h 17393 2014-08-23 20:19:14Z predoehl $ */

#ifndef KJB_FLOW_INTEGRAL_FLOW_H
#define KJB_FLOW_INTEGRAL_FLOW_H

#include <m_cpp/m_matrix.h>
#include <gr_cpp/gr_2D_bounding_box.h>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>

namespace kjb {

/**
 * @class   Integral_flow
 *
 * Class that represents the summed area table for optical flow features.
 */
class Integral_flow
{
public:
    /**
     * @brief   Construct an integral flow object from the given
     *          flow matrix and sub-sampling rate.
     */
    Integral_flow(const Matrix& flow, size_t subsample_rate = 1);

    /**
     * @brief   Construct an integral flow from input file  
     */
    Integral_flow(const std::string& fname);

    /**
     * @brief   Returns the (interpolated) value of the integral flow at
     *          (x, y); i.e., computes the sum of flow in the box with
     *          corners at (0, 0) and (x, y).
     */
    double flow_sum(double x, double y) const;

    /**
     * @brief   Returns the sum of flows inside the given box.
     */
    double flow_sum(const Axis_aligned_rectangle_2d& box) const
    {
        double left = std::max(0.0, box.get_left());
        double right = std::min(box.get_right(), img_width_ - 1.0);
        //double bottom = std::max(0.0, box.get_top());
        double bottom = std::min(box.get_top(), img_height_ - 1.0);
        //double top = std::min(box.get_bottom(), (double)img_height_);
        double top = std::max(0.0, box.get_bottom());

        /*if(left >= img_width_
            || right < 0
            || bottom >= img_height_
            || top < 0)
        {
            return 0.0;
        }
        */
        if(right < 0.0 
            || left >= img_width_
            || bottom < 0.0
            || top >= img_height_)
        {
            return 0.0;
        }

        double fs = flow_sum(right, bottom) + flow_sum(left, top)
                    - flow_sum(left, bottom) - flow_sum(right, top);

        return fs;
    }

    void write(const std::string& fname);

    size_t img_width() const { return  img_width_; }
    size_t img_height() const { return  img_height_; }

private:
    /**
     * @brief   Looks up the integral image value at (x, y). Throws error
     *          if it does not exist.
     */
    double at(size_t row, size_t col) const
    {
        return image_[row*width_ + col];
    }

    /**
     * @brief   Returns the nearest existing flow feature to the left and up.
     */
    std::pair<size_t, size_t> ul_corner(double x, double y) const
    {
        return std::make_pair(x / ss_rate_, y / ss_rate_);
    }

    size_t ss_rate_;
    size_t img_width_;
    size_t img_height_;
    size_t width_;
    size_t height_;
    std::vector<float> image_;
};

} //namespace kjb

#endif /*KJB_FLOW_INTEGRAL_FLOW_H */

