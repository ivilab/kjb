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

/* $Id: flow_feature_set.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

#include <flow_cpp/flow_feature_set.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_util.h>
#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <m_cpp/m_vector.h>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <set>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <algorithm>

using namespace kjb;

Flow_feature_set kjb::read_flow_features
(
    const std::string& fname 
)
{
    std::ifstream ifs(fname.c_str());

    std::string line;
    if(ifs.fail())
    {
        KJB_THROW_3(IO_error, "Can't open file: %s", (fname.c_str()));
    }
   
    Flow_feature_set frame_flow;
    const size_t tokens_per_feature = 2;
    const size_t tokens_per_line = 2*tokens_per_feature;
    while(std::getline(ifs, line))
    {
        std::vector<std::string> tokens;
        boost::trim(line);
        boost::split(tokens, line, boost::is_any_of("\t\n "),
                                   boost::token_compress_on);
        assert(tokens.size() == tokens_per_line);

        size_t t = 0;
        Vector p1(2), p2(2);
        p1(0) = boost::lexical_cast<double>(tokens[t++]);
        p1(1) = boost::lexical_cast<double>(tokens[t++]);
        p2(0) = boost::lexical_cast<double>(tokens[t++]);
        p2(1) = boost::lexical_cast<double>(tokens[t++]);
        
        frame_flow.insert(std::make_pair(p1, p2));
    }

    return frame_flow;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Feature_pair> kjb::look_up_features
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& box
)
{
    std::vector<Feature_pair> features_pairs;

    //if(of_set.empty())
    //{
    //    KJB_THROW_2(Runtime_error, "The optical flow is not precomputed.");
    //}

    const Vector& center = box.get_center();

    double left = box.get_left();
    double right = box.get_right();
    double bottom = box.get_bottom();
    double top = box.get_top();

    size_t left_px = left;
    size_t right_px = std::ceil(right);
    size_t bottom_px = bottom;
    size_t top_px = std::ceil(top);

    Vector src = Vector().set(left_px, bottom_px);
    Flow_feature_set::const_iterator fp = of_set.lower_bound(std::make_pair(src, src));

    for(; fp != of_set.end(); fp++)
    {
        const Vector& cur_feature = fp->first;
        if(cur_feature[0] >= right_px)
            break;
        if(cur_feature[1] < bottom_px || cur_feature[1] >= top_px)
            continue;
        if(cur_feature[0] < right_px && cur_feature[1] < top_px)
        {
            const Vector& next_feature = fp->second;
            features_pairs.push_back(std::make_pair(cur_feature, next_feature));
        }
    }

    return features_pairs;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Feature_pair> kjb::look_up_bg_features
(
    const Flow_feature_set& of_set,
    const std::vector<Axis_aligned_rectangle_2d>& model_boxes
)
{
    std::vector<Feature_pair> features_pairs;

    //if(of_set.empty())
    //{
    //    KJB_THROW_2(Runtime_error, "The optical flow is not precomputed.");
    //}

    Flow_feature_set::const_iterator flow_iter;
    for(flow_iter = of_set.begin(); flow_iter != of_set.end(); flow_iter++)
    {
        const Vector& cur_feature = flow_iter->first;
        double fx = cur_feature[0];
        double fy = cur_feature[1]; 
        bool outside = true;
        BOOST_FOREACH(const Axis_aligned_rectangle_2d& box, model_boxes)
        {
            double left = box.get_left();
            double right = box.get_right();
            double top = box.get_top();
            double bottom = box.get_bottom();
            if(fx >= left && fx <= right && fy >= bottom && fy <=top)
            {
                outside = false;
                break;
            }
        }
        if(outside)
        {
            const Vector& next_feature = flow_iter->second;
            features_pairs.push_back(std::make_pair(cur_feature, 
                                                    next_feature));
        }
    }

    return features_pairs; 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::valid_flow
(
    const std::vector<Feature_pair>& feature_pairs,
    double percentile
)
{
    if(feature_pairs.empty())
    {
        return std::vector<Vector>();
    }

    size_t num_features = feature_pairs.size();
    std::vector<Vector> flow_vectors(num_features);
    for(size_t i = 0; i < num_features; i++)
    {
        const Vector& cur_feature = feature_pairs[i].first;
        const Vector& next_feature = feature_pairs[i].second;
        flow_vectors[i] = next_feature - cur_feature;
    }

    std::sort(flow_vectors.begin(), flow_vectors.end(), compare_flow_magnitude);

    size_t cut_off = std::ceil(num_features*percentile);
    std::vector<Vector> valid_flows(cut_off);

    for(size_t i = 0; i < cut_off; i++)
    {
        valid_flows[i] = flow_vectors[i];
    }

    return valid_flows;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> kjb::valid_flow
(
    const std::vector<Feature_pair>& feature_pairs,
    const std::vector<size_t>& angle_hist
)
{
    std::vector<size_t>::const_iterator max_p;
    max_p = std::max_element(angle_hist.begin(), angle_hist.end()); 
    size_t max_index = *max_p;
    const double min_angle = -179.0;
    const double max_angle = 180.0;
    size_t num_bins = angle_hist.size();
    double bin_size = (max_angle - min_angle) / num_bins; 
    double lower_angle = min_angle + bin_size * max_index;
    double upper_angle = lower_angle + bin_size; 
     
    std::vector<Vector> val_flows; 
    val_flows.reserve(feature_pairs.size());
    BOOST_FOREACH(const Feature_pair& fp, feature_pairs)
    {
        const Vector& cur_feature = fp.first;
        const Vector& next_feature = fp.second;
        Vector flow = next_feature - cur_feature;
        double angle = atan2(flow[1], flow[0]) * 180.0 / M_PI;
        if(angle >= lower_angle && angle < upper_angle)
        {
            val_flows.push_back(flow);
        }
    }
    return val_flows; 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<size_t> kjb::angle_histogram
(
    const std::vector<Feature_pair>& feature_pairs,
    size_t num_bins
)
{
    const double min_angle = -180.0; 
    const double max_angle = 180.0;

    double bin_size = (max_angle - min_angle) / num_bins;
    std::vector<size_t> hist(num_bins, 0);
    BOOST_FOREACH(const Feature_pair& fp, feature_pairs)
    {
        const Vector& cur_feature = fp.first;
        const Vector& next_feature = fp.second;
        Vector flow = next_feature - cur_feature;
        double angle = atan2(flow[1], flow[0]) * 180.0 / M_PI;
        size_t index = std::floor((angle - min_angle) / bin_size);
        if(index == num_bins) index = num_bins - 1; 
        KJB(ASSERT(index < num_bins));
        hist[index]++; 
    }

    return hist;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::update_average_velocity
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& old_box,
    const Vector& old_velocity,
    MOVE_DIRECTION dir,
    size_t unit
)
{
    IFT(unit > 0, Illegal_argument,
        "update_average_velocity: must move box by at least 1.");

    double width = old_box.get_width();
    double height = old_box.get_height();
    Vector old_center = old_box.get_center();
    double old_top = old_box.get_top();
    double old_bot = old_box.get_bottom();
    double old_left = old_box.get_left();
    double old_right = old_box.get_right();

    Axis_aligned_rectangle_2d old_part;
    Axis_aligned_rectangle_2d new_part;

    switch(dir)
    {
        case COL_PLUS:
            new_part.set_width(unit);
            new_part.set_height(height);
            new_part.set_center(Vector(old_right + unit/2.0, old_center[1])); 
            old_part.set_width(unit);
            old_part.set_height(height);
            old_part.set_center(Vector(old_left + unit/2.0, old_center[1])); 
            break;

        case COL_MINUS:
            new_part.set_width(unit);
            new_part.set_height(height);
            new_part.set_center(Vector(old_left - unit/2.0, old_center[1])); 
            old_part.set_width(unit);
            old_part.set_height(height);
            old_part.set_center(Vector(old_right - unit/2.0, old_center[1])); 
            break;

        case ROW_PLUS: 
            new_part.set_width(width);
            new_part.set_height(unit);
            new_part.set_center(Vector(old_center[0], old_top + unit/2.0)); 
            old_part.set_width(width);
            old_part.set_height(unit);
            old_part.set_center(Vector(old_center[0], old_bot + unit/2.0)); 
            break;

        case ROW_MINUS:
            new_part.set_width(width);
            new_part.set_height(unit);
            new_part.set_center(Vector(old_center[0], old_bot - unit/2.0)); 
            old_part.set_width(width);
            old_part.set_height(unit);
            old_part.set_center(Vector(old_center[0], old_top - unit/2.0)); 
            break;
                    
        default:
            KJB_THROW_2(Illegal_argument, "Unknown moving direction");
            break;
    }

    Vector old_flow = total_flow(of_set, old_part);
    Vector new_flow = total_flow(of_set, new_part);

    double num_features = width * height;
    Vector flow(old_velocity * num_features);

    flow = (flow - old_flow + new_flow)/num_features;
    return flow;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::update_average_velocity
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& old_box,
    const Axis_aligned_rectangle_2d& new_box,
    const Vector& old_velocity
)
{
    typedef Axis_aligned_rectangle_2d Bbox;
    std::vector<Bbox> old_parts;
    std::vector<Bbox> new_parts;

    Vector o_c = old_box.get_center();
    double o_w = old_box.get_width();
    double o_h = old_box.get_height();
    double o_t = old_box.get_top();
    double o_b = old_box.get_bottom();
    double o_l = old_box.get_left();
    double o_r = old_box.get_right();

    Vector n_c = new_box.get_center();
    double n_w = new_box.get_width();
    double n_h = new_box.get_height();
    double n_t = new_box.get_top();
    double n_b = new_box.get_bottom();
    double n_l = new_box.get_left();
    double n_r = new_box.get_right();

    double d_l = o_l - n_l; 
    double d_r = o_r - n_r; 
    double d_t = o_t - n_t; 
    double d_b = o_b - n_b; 

    // If the new box and the old box not overlapping 
    if(o_r < n_l || o_l > n_r || o_b > n_t || o_t < n_b)
    {
        old_parts.push_back(old_box);
        new_parts.push_back(new_box);
    }
    else
    {
        if(d_l < 0.0)
        {
            double x = o_l + fabs(d_l/2.0);
            if(o_b > n_b)
            {
                if(o_t <= n_t)
                {
                    old_parts.push_back(Bbox(Vector(x, o_c[1]), -d_l, o_h));
                }
                else 
                {
                    if(n_t > o_b)
                    {
                        double dh = o_t - n_t; 
                        assert(dh >= 0.0);
                        double y = (n_t - o_b)/2.0;
                        double h = o_h - dh;
                        old_parts.push_back(Bbox(Vector(x, y), -d_l, h));
                    }
                }
            }
            else
            {
                if(o_t >= n_t)
                {
                    old_parts.push_back(Bbox(Vector(x, n_c[1]), -d_l, n_h));
                }
                else
                {
                    if(o_t > n_b)
                    {
                        double dh = n_b - o_b;
                        assert(dh >= 0.0);
                        double y = (o_t - n_b)/2.0;
                        double h = o_h - dh;
                        old_parts.push_back(Bbox(Vector(x, y), -d_l, h));
                    }
                }
            }
        }
        else if(d_l > 0.0)
        {
            double x = n_l + d_l/2.0;
            if(n_b > o_b)
            {
                if(o_t >= n_t)
                {
                    new_parts.push_back(Bbox(Vector(x, n_c[1]), d_l, n_h));
                }
                else 
                {
                    if(o_t > n_b)
                    {
                        double dh = n_t - o_t; 
                        assert(dh >= 0.0);
                        double y = (o_t - n_b)/2.0;
                        double h = n_h - dh;
                        new_parts.push_back(Bbox(Vector(x, y), d_l, h));
                    }
                }
            }
            else
            {
                if(n_t >= o_t)
                {
                    new_parts.push_back(Bbox(Vector(x, o_c[1]), d_l, o_h));
                }
                else
                {
                    if(n_t > o_b)
                    {
                        double dh = o_b - n_b; 
                        assert(dh >= 0.0);
                        double y = (n_t - o_b)/2.0;
                        double h = n_h - dh; 
                        new_parts.push_back(Bbox(Vector(x, y), d_l, h));
                    }
                }
            }
        }

        if(d_r < 0.0)
        {
            double x = o_r + fabs(d_r/2.0);
            if(n_b > o_b)
            {
                if(o_t >= n_t)
                {
                    new_parts.push_back(Bbox(Vector(x, n_c[1]), -d_r, n_h));
                }
                else 
                {
                    if(o_t > n_b) 
                    {
                        double dh = n_t - o_t;
                        assert(dh >= 0.0);
                        double y = (o_t - n_b)/2.0;
                        double h = n_h - dh;
                        new_parts.push_back(Bbox(Vector(x, y), -d_r, h));
                    }
                }
            }
            else
            {
                if(n_t >= o_t)
                {
                    new_parts.push_back(Bbox(Vector(x, o_c[1]), -d_r, o_h));
                }
                else
                {
                    if(n_t > o_b)
                    {
                        double dh = o_b - n_b;
                        assert(dh >= 0.0);
                        double y = (n_t - o_b)/2.0;
                        double h = n_h - dh;
                        new_parts.push_back(Bbox(Vector(x, y), -d_r, h));
                    }
                }
            }
        }
        else if(d_r > 0.0)
        {
            double x = n_r + d_r/2.0;
            if(o_b > n_b)
            {
                if(n_t >= o_t)
                {
                    old_parts.push_back(Bbox(Vector(x, o_c[1]), d_r, o_h));
                }
                else
                {
                    if(n_t > o_b)
                    {
                        double dh = o_t - n_t;
                        assert(dh >= 0.0);
                        double y = (n_t - o_b)/2.0;
                        double h = o_h - dh;
                        old_parts.push_back(Bbox(Vector(x, y), d_r, h));
                    }
                }
            }
            else
            {
                if(o_t >= n_t)
                {
                    old_parts.push_back(Bbox(Vector(x, n_c[1]), d_r, n_h));
                }
                else
                {
                    if(o_t > n_b)
                    {
                        double dh = n_b - o_b;
                        assert(dh >= 0.0);
                        double y = (o_t - n_b)/2.0;
                        double h = o_h - dh;
                        old_parts.push_back(Bbox(Vector(x, y), d_r, h));
                    }
                }
            }
        }

        if(d_t < 0.0)
        {
            double y = o_t + fabs(d_t/2.0);
            new_parts.push_back(Bbox(Vector(n_c[0], y), n_w, -d_t));
        }
        else if(d_t > 0.0)
        {
            double y = n_t + d_t/2.0;
            old_parts.push_back(Bbox(Vector(o_c[0], y), o_w, d_t));
        }

        if(d_b < 0.0)
        {
            double y = o_b + fabs(d_b/2.0);
            old_parts.push_back(Bbox(Vector(o_c[0], y), o_w, -d_b));
        }
        else if(d_b > 0.0)
        {
            double y = n_b + d_b/2.0;
            new_parts.push_back(Bbox(Vector(n_c[0], y), n_w, d_b)); 
        }
    }

    ///////////////////////////////////////////////////////////

    double area = o_w * o_h; 
    Vector flow(old_velocity * area);
    //std::cout << " old boxes: \n";
    BOOST_FOREACH(const Bbox& box, old_parts)
    {
        //std::cout << box << std::endl;
        flow -= total_flow(of_set, box);
        area -= (box.get_width() * box.get_height());
    }
    //std::cout << " new boxes: \n";
    BOOST_FOREACH(const Bbox& box, new_parts)
    {
        //std::cout << box << std::endl;
        flow += total_flow(of_set, box);
        area += (box.get_width() * box.get_height());
    }

    return flow/area;
    
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector kjb::lookup_feature
(
    const Flow_feature_set& of_set,
    size_t x,
    size_t y,
    size_t subsample_sz
)
{
    size_t dx = x % subsample_sz;
    size_t dy = y % subsample_sz;

    size_t rx = x - dx;
    size_t ry = y - dy;

    Vector src = Vector().set(rx, ry);
    Flow_feature_set::const_iterator fp = of_set.find(std::make_pair(src, src));

    if(fp == of_set.end())
    {
        return Vector().set(0.0, 0.0);
    }

    return fp->second - fp->first;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*Vector kjb::total_flow
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& box,
    size_t subsample_sz
)
{
    IFT(subsample_sz > 0, Illegal_argument,
        "subsample size must be at least 1.");

    Vector total(2, 0.0);

    double left = box.get_left();
    double right = box.get_right();
    double bottom = box.get_bottom();
    double top = box.get_top();

    size_t left_px = std::floor(left);
    size_t right_px = std::ceil(right);
    size_t bottom_px = std::floor(bottom);
    size_t top_px = std::ceil(top);

    //std::ofstream ofs("flow");
    for(size_t x = left_px; x < right_px; x++)
    { 
        double pxl, pxr; 
        // border
        if(x == left_px)
        {
            pxl = (left_px < left ? left : left_px); 
            pxr = x + 1;
        }

        else if(x == right_px - 1)
        {
            pxl = x; 
            pxr = (right_px > right ? right : right_px); 
        }
        else
        {
            pxl = x; 
            pxr = x + 1; 
        }

        for(size_t y = bottom_px; y < top_px; y++)
        {
            double pxb, pxt;
            if(y == bottom_px)
            {
                pxb = (bottom_px < bottom ? bottom : bottom_px);
                pxt = y + 1; 
            }
            else if(y == top_px - 1)
            {
                pxb = y; 
                pxt = (top_px > top ? top : top_px); 
            }
            else
            {
                pxb = y; 
                pxt = y + 1; 
            }
            double pxa = (pxr - pxl) * (pxt - pxb);

            Vector flow = pxa * lookup_feature(of_set, x, y, subsample_sz);
            total += flow;
            //std::cout << x << " " << y << " " << pxa <<  " " << flow << std::endl;
        }
    }
    //std::cout << std::endl;

    return total;
}
*/

Vector kjb::total_flow
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& box
)
{
    Vector tflow(2, 0.0);
    const Vector& center = box.get_center();

    double left = box.get_left();
    double right = box.get_right();
    double bottom = box.get_bottom();
    double top = box.get_top();

    size_t left_px = left;
    size_t right_px = std::ceil(right);
    size_t bottom_px = bottom;
    size_t top_px = std::ceil(top);

    Vector src = Vector().set(left_px, bottom_px);
    Flow_feature_set::const_iterator fp = of_set.find(std::make_pair(src, src));

    //std::cout << " cur features: \n"; 
    for(; fp != of_set.end(); fp++)
    {
        const Vector& cur_feature = fp->first;
        if(cur_feature[0] >= right_px)
            break;
        if(cur_feature[1] < bottom_px || cur_feature[1] >= top_px)
            continue;
        if(cur_feature[0] < right_px && cur_feature[1] < top_px)
        {
            size_t x = cur_feature[0];
            size_t y = cur_feature[1];
            //std::cout << x << " " << y << " ";
            // Border
            double pxl, pxr; 
            if(left_px == right_px -1)
            {
                pxl = left; 
                pxr = right; 
            }
            else
            {
                if(x == left_px)
                {
                    pxl = (left_px < left ? left : left_px); 
                    pxr = x + 1;
                }

                else if(x == right_px - 1)
                {
                    pxl = x; 
                    pxr = (right_px > right ? right : right_px); 
                }
                else
                {
                    pxl = x; 
                    pxr = x + 1; 
                }
            }

            double pxb, pxt;
            if(bottom_px == top_px -1)
            {
                pxb = bottom; 
                pxt = top; 
            }
            else
            {
                if(y == bottom_px)
                {
                    pxb = (bottom_px < bottom ? bottom : bottom_px);
                    pxt = y + 1; 
                }
                else if(y == top_px - 1)
                {
                    pxb = y; 
                    pxt = (top_px > top ? top : top_px); 
                }
                else
                {
                    pxb = y; 
                    pxt = y + 1; 
                }
            }

            double pxa = (pxr - pxl) * (pxt - pxb);
            const Vector& next_feature = fp->second;
            Vector flow = next_feature - cur_feature; 
            tflow += (pxa * flow);

            //std::cout << pxa << " " << flow << std::endl;
        }
    }

    //std::cout << std::endl << std::endl;

    return tflow;
}

