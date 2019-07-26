/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
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
* =========================================================================== */

/* $Id: pt_color_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "people_tracking_cpp/pt_color_likelihood.h"
#include "people_tracking_cpp/pt_scene.h"
#include "people_tracking_cpp/pt_target.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"
#include "people_tracking_cpp/pt_body_2d_trajectory.h"
#include "people_tracking_cpp/pt_util.h"
#include "people_tracking_cpp/pt_visibility.h"
#include "people_tracking_cpp/pt_body_2d.h"
#include "people_tracking_cpp/pt_detection_box.h"
#include "prob_cpp/prob_histogram.h"
#include "i_cpp/i_image.h"
#include "i_cpp/i_filter.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"

#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <boost/shared_array.hpp>

using namespace kjb;
using namespace kjb::pt;

void Color_likelihood::read_frames(const std::vector<std::string>& frame_fps)
{
    size_t num_frames = frame_fps.size();
    ASSERT(num_frames > 0);
    Image temp_img(frame_fps[0]);
    size_t num_rows = temp_img.get_num_rows();
    size_t num_cols = temp_img.get_num_cols();
    size_t new_nrows = num_rows / m_pixel_ssz;
    size_t new_ncols = num_cols / m_pixel_ssz;
    size_t new_sz = new_nrows * new_ncols;

    m_width = num_cols; 
    m_height = num_rows;

    m_r_pixels.resize(num_frames);
    m_g_pixels.resize(num_frames);
    // Load in the image frames 
    for(size_t i = 0; i < num_frames; i = i + m_frame_ssz)
    {
        Image image(frame_fps[i]);
        boost::shared_array<float> r_buffer(new float[new_sz]);
        boost::shared_array<float> g_buffer(new float[new_sz]);

        for(size_t row = 0; row < num_rows; row += m_pixel_ssz)
        {
            size_t srow = row / m_pixel_ssz;
            for(size_t col = 0; col < num_cols; col += m_pixel_ssz)
            {
                double red = float(image(row, col, Image::RED));
                double green = float(image(row, col, Image::GREEN));
                double blue = float(image(row, col, Image::BLUE));
                double s = red + green + blue;
                size_t scol = col / m_pixel_ssz;

                r_buffer[scol + srow*new_ncols] = red / s;
                g_buffer[scol + srow*new_ncols] = green / s;
                scol++;
            }
            srow++;
        }
        m_r_pixels[i] = r_buffer;
        m_g_pixels[i] = g_buffer;
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Color_likelihood::operator()(const Scene& scene)const
{
    IFT(m_bg_r_p != NULL && m_bg_g_p != NULL,
        Runtime_error, "Background matrix not set.");

    double ll = 0.0;
    BOOST_FOREACH(const Target& tg, scene.association)
    {
        ll += at_trajectory(tg);
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Color_likelihood::at_trajectory(const Target& target) const
{
    const Trajectory traj = target.trajectory();
    const Body_2d_trajectory btraj = target.body_trajectory();
    size_t sf = target.get_start_time();
    size_t ef = target.get_end_time();

    double ll = 0.0;
    for(size_t cur_frame = sf; cur_frame <= ef - 1; cur_frame++)
    {
        size_t next_frame = cur_frame + m_frame_ssz;
        if(next_frame > ef) break;

        const Body_2d& cur_b2d = btraj[cur_frame - 1]->value;
        const Body_2d& next_b2d = btraj[next_frame - 1]->value;

        if(cur_b2d.visibility.visible == 0.0) continue;

        ll += at_box(cur_b2d, next_b2d, cur_frame, next_frame);
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

double Color_likelihood::at_box
(
    const Body_2d& cur_b2d,
    const Body_2d& next_b2d,
    size_t cur_frame,
    size_t next_frame
) const
{
    const Bbox& cur_box = cur_b2d.body_bbox;
    const Bbox& next_box = next_b2d.body_bbox;
    const Visibility& bvis = cur_b2d.visibility;
    const Visibility& bvis_next = next_b2d.visibility;

    const std::vector<Vector>& cur_vis_cells = bvis.visible_cells;
    const std::vector<Vector>& next_vis_cells = bvis_next.visible_cells;

    const size_t num_subdivisions = 8; 
    //const size_t num_cells = num_subdivisions * num_subdivisions; 

    // subdivide cur_box into cell centers
    const double x_delta = cur_box.get_width() / num_subdivisions;
    const double y_delta = cur_box.get_height() / num_subdivisions;
    Filter filter = gaussian_filter(x_delta/5.0);

    const double x_min = cur_box.get_left() + x_delta / 2.0;
    const double y_min = cur_box.get_bottom() + y_delta / 2.0;

    // subdivide next_box into cell centers
    const double xn_delta = next_box.get_width() / num_subdivisions;
    const double yn_delta = next_box.get_height() / num_subdivisions;

    const double xn_min = next_box.get_left() + xn_delta / 2.0;
    const double yn_min = next_box.get_bottom() + yn_delta / 2.0;

    const size_t num_bins = 8;
    std::pair<double, double> range_1 = std::make_pair(0.0, 1.0);
    std::pair<double, double> range_2 = std::make_pair(0.0, 1.0);

    std::vector<Vector>::const_iterator cur_vis_it = 
                                            cur_vis_cells.begin();
    std::vector<Vector>::const_iterator next_vis_it = 
                                            next_vis_cells.begin();

    double ll = 0.0;
    for(size_t x_i = 0; x_i < num_subdivisions; x_i++)
    {
        for(size_t y_i = 0; y_i < num_subdivisions; y_i++)
        {
            const double x = x_min + x_i * x_delta;
            const double y = y_min + y_i * y_delta;
            const double xn = xn_min + x_i * xn_delta;
            const double yn = yn_min + y_i * yn_delta;

            Vector cur_loc(x, y);
            Vector next_loc(xn, yn);

            
            if(bvis.visible < 1.0 || bvis_next.visible < 1.0)
            {
                // if the cur cell is not visible advance the cell
                if(vector_distance(cur_loc, *cur_vis_it) > DBL_EPSILON)
                {
                    // if the next cell is visable
                    // advance the visble cell
                    if(vector_distance(next_loc, *next_vis_it) <= DBL_EPSILON)
                    {
                        next_vis_it++;
                    }
                    continue;
                }
                else // if the cur cell is visible 
                {
                    // if the next visble cell is not visible 
                    // advance the current visible cell
                    if(vector_distance(next_loc, *next_vis_it) > DBL_EPSILON)
                    {
                        cur_vis_it++;
                        continue;
                    }
                }
            }
             // if the current cell and the next cell are both visible
            // cell at the current frame
            bool is_foreground = true;
            Matrix H1 = get_norm_hist_as_matrix(cur_loc, 
                                                x_delta, y_delta, 
                                                cur_frame, 
                                                range_1, range_2, 
                                                num_bins, 
                                                filter,
                                                is_foreground); 

            // cell at the next frame
            Matrix H2  = get_norm_hist_as_matrix(next_loc, 
                                                 xn_delta, yn_delta, 
                                                 next_frame, 
                                                 range_1, range_2, 
                                                 num_bins, 
                                                 filter,
                                                 is_foreground); 

            // Compute the chi-squared distance between the two histograms

            double s_fg = chi_square(H1, H2);

            // pixels at the cell location of the background

            is_foreground = false;
            Matrix Hb =  get_norm_hist_as_matrix(cur_loc, 
                                                 x_delta, y_delta, 
                                                 cur_frame, 
                                                 range_1, range_2, 
                                                 num_bins, 
                                                 filter,
                                                 is_foreground); 

            // Compute the chi-squared distance to the background cell
            double s_bg = chi_square(H1, Hb);
            ll += (log(chi_squared_dist_to_prob(s_fg)) -
                   log(chi_squared_dist_to_prob(s_bg)));

            cur_vis_it++;
            next_vis_it++;
        }
    }

    return ll;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<Vector> Color_likelihood::get_foreground
(
    size_t frame, 
    const Bbox& box
) const
{
    IFT(frame <= m_r_pixels.size(), Illegal_argument, "frame out of bound"); 
    std::vector<Vector> values;
    //const Vector& center = box.get_center();

    double left = std::max(0.0, box.get_left());
    double right = std::max(0.0, box.get_right());
    double bottom = std::max(0.0, box.get_bottom());
    double top = std::max(0.0, box.get_top());
    size_t num_elements = box.get_width() * box.get_height();

    size_t left_px = std::max(0.0, left);
    size_t right_px = std::min((double)m_width, std::ceil(right));
    size_t bottom_py = std::max(0.0, bottom);
    size_t top_py = std::min((double)m_height, std::ceil(top));
    
    boost::shared_array<float> r_pixels = m_r_pixels[frame - 1]; 
    boost::shared_array<float> g_pixels = m_g_pixels[frame - 1]; 
    //size_t new_nrows = m_height / m_pixel_ssz;
    size_t new_ncols = m_width / m_pixel_ssz;

    values.reserve(num_elements);
    for(size_t i = left_px; i < right_px; i += m_pixel_ssz )
    {
        for(size_t j = bottom_py; j < top_py; j += m_pixel_ssz)
        {
            size_t srow = j / m_pixel_ssz;
            size_t scol = i / m_pixel_ssz;
            double r = r_pixels[scol + srow*new_ncols];
            double g = g_pixels[scol + srow*new_ncols];
            values.push_back(Vector(r, g));
        }
    }
    return values;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
std::vector<Vector> Color_likelihood::get_background
(
    const Bbox& box
) const
{
    std::vector<Vector> values;
    //const Vector& center = box.get_center();

    double left = std::max(0.0, box.get_left());
    double right = std::max(0.0, box.get_right());
    double bottom = std::max(0.0, box.get_bottom());
    double top = std::max(0.0, box.get_top());
    size_t num_elements = box.get_width() * box.get_height();

    size_t left_px = std::max(0.0, left);
    size_t right_px = std::min((double)m_width, std::ceil(right));
    size_t bottom_py = std::max(0.0, bottom);
    size_t top_py = std::min((double)m_height, std::ceil(top));

    values.reserve(num_elements);
    for(size_t i = left_px; i < right_px; i++)
    {
        for(size_t j = bottom_py; j < top_py; j++)
        {
            values.push_back(Vector((*m_bg_r_p)(j,i), (*m_bg_g_p)(j,i)));
        }
    }

    return values;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Matrix Color_likelihood::get_norm_hist_as_matrix
(
    const Vector& loc,
    double x_delta,
    double y_delta,
    size_t frame,
    const std::pair<double, double>& range_1,
    const std::pair<double, double>& range_2,
    size_t num_bins,
    const Filter& filter,
    bool is_foreground
) const
{
    Bbox vis_cell(loc, x_delta, y_delta);
    unstandardize(vis_cell, m_width, m_height);

    // get the pixels in range
    std::vector<Vector> pixels;
    if(is_foreground)
    {
        pixels = get_foreground(frame, vis_cell);
    }
    else
    {
        pixels = get_background(vis_cell);
    }

    Histogram_2d hist(pixels.begin(), pixels.end(), 
                     num_bins, num_bins, 
                     range_1, range_2);
    Matrix M;
    if(pixels.size() > 1)
         //M = hist.normalized(); 
         M = hist.normalized() * filter; 
    else 
         M = hist.as_matrix();

    return M;

}
