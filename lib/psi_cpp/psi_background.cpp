/* $Id: psi_background.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author: Jinyan Guan 
 * =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "psi_cpp/psi_background.h"
#include "people_tracking_cpp/pt_box_trajectory.h"

#include <boost/shared_array.hpp>

namespace kjb
{
namespace psi
{
void compute_median_background
(
    const Video& video, 
    Matrix& s_mat, 
    Matrix& r_mat, 
    Matrix& g_mat,
    size_t rate
)
{
    using namespace std;
    size_t num_rows = video.get_height();
    size_t num_cols = video.get_width();
    size_t num_frames = video.size();

    s_mat.resize(num_rows, num_cols, 0.0);
    r_mat.resize(num_rows, num_cols, 0.0);
    g_mat.resize(num_rows, num_cols, 0.0);

    //boost::shared_array<float> back_ground(new float[3 * num_rows * num_cols]);

    // Compute the medium for each pixel location 
    size_t median_frame = std::floor(num_frames*0.5);
    for(size_t row = 0; row < num_rows; row++)
    {
        for (size_t col = 0; col < 3*num_cols; col += 3)
        {
            vector<float> S_vec(num_frames); 
            vector<float> r_vec(num_frames); 
            vector<float> g_vec(num_frames); 
            for (size_t frame = 0; frame < num_frames; frame += rate)
            {
                float R = (float)video.get_buffer(frame)[col + 0 + row*3*num_cols];
                float G = (float)video.get_buffer(frame)[col + 1 + row*3*num_cols];
                float B = (float)video.get_buffer(frame)[col + 2 + row*3*num_cols];
                S_vec[frame] = R + G + B;
                if(S_vec[frame] < FLT_EPSILON)
                {
                    r_vec[frame] = 0.0; 
                    g_vec[frame] = 0.0;
                }
                else
                {
                    r_vec[frame] = R/S_vec[frame];
                    g_vec[frame] = G/S_vec[frame];
                }
            }
            sort(S_vec.begin(), S_vec.end());
            sort(r_vec.begin(), r_vec.end());
            sort(g_vec.begin(), g_vec.end());
            float S_median = S_vec[median_frame];
            float r_median = r_vec[median_frame];
            float g_median = g_vec[median_frame];
            //std::cout << S_median << " " << r_median << " " << g_median << std::endl;

            size_t row_out = num_rows - row - 1; 
            s_mat(row_out, col/3) = S_median;
            r_mat(row_out, col/3) = r_median;
            g_mat(row_out, col/3) = g_median;
            
            /*back_ground[col + 0 + row*3*num_cols] = S_median;
            back_ground[col + 1 + row*3*num_cols] = r_median;
            back_ground[col + 2 + row*3*num_cols] = g_median;
            */
        }
    }

    /*for(size_t row = 0; row < num_rows; row++)
    {
        for(size_t col = 0; col < 3*num_cols; col += 3)
        {
            size_t r = num_rows - row - 1; 
            s(r, col/3) = back_ground[col + 0 + row*3*num_cols];
            r(r, col/3) = back_ground[col + 1 + row*3*num_cols];
            g(r, col/3) = back_ground[col + 2 + row*3*num_cols];
        }
    }
    */
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void compute_median_background
(
    const Video& video, 
    const pt::Box_trajectory_map& btrajs,
    Matrix& s_mat, 
    Matrix& s_count,
    Matrix& r_mat, 
    Matrix& r_count,
    Matrix& g_mat,
    Matrix& g_count
)
{
    using namespace std;
    size_t num_rows = video.get_height();
    size_t num_cols = video.get_width();
    size_t num_frames = video.size();


    s_mat.resize(num_rows, num_cols);
    r_mat.resize(num_rows, num_cols);
    g_mat.resize(num_rows, num_cols);
    s_count.resize(num_rows, num_cols, 0);
    r_count.resize(num_rows, num_cols, 0);
    g_count.resize(num_rows, num_cols, 0);

    //boost::shared_array<float> back_ground(new float[3 * num_rows * num_cols]);

    for(size_t row = 0; row < num_rows; row++)
    {
        for (size_t col = 0; col < 3*num_cols; col += 3)
        {
            vector<float> S_vec; 
            vector<float> r_vec; 
            vector<float> g_vec; 

            size_t row_out = num_rows - row - 1; 

            for (size_t frame = 0; frame < num_frames; frame += 30)
            {
                bool foreground = false;
                BOOST_FOREACH(const pt::Box_trajectory_map::value_type& pr, btrajs)
                {
                    const pt::Box_trajectory btraj = pr.second;
                    if(btraj[frame])
                    {
                        const Axis_aligned_rectangle_2d& box = (*btraj[frame]).value;
                        const Vector& center = box.get_center();
                        double width = box.get_width();
                        double height = box.get_height();
                        size_t min_x = std::max(0, (int)std::floor(center[0] - width/2.0));
                        size_t max_x = std::min((int)num_cols-1, (int)std::ceil(center[0] + width/2.0));
                        size_t min_y = std::max(0, (int)std::floor(center[1] - height/2.0));
                        size_t max_y = std::min((int)num_rows-1, (int)std::ceil(center[1] + height/2.0));
                        if(col/3 > min_x && col/3 < max_x && row_out > min_y && row_out < max_y)
                        {
                            foreground = true;
                            break;
                        }
                    }
                }
                if(!foreground)
                {
                    float R = (float)video.get_buffer(frame)[col + 0 + row*3*num_cols];
                    float G = (float)video.get_buffer(frame)[col + 1 + row*3*num_cols];
                    float B = (float)video.get_buffer(frame)[col + 2 + row*3*num_cols];
                    float cur_S = R + G + B; 
                    S_vec.push_back(cur_S);
                    if(cur_S > 0.0)
                    {
                        r_vec.push_back(R/cur_S);
                        g_vec.push_back(G/cur_S);
                    }
                    else
                    {
                        r_vec.push_back(0.0);
                        g_vec.push_back(0.0);
                    }
                    s_count(row_out, col/3)++;
                    r_count(row_out, col/3)++;
                    g_count(row_out, col/3)++;
                }
            }

            sort(S_vec.begin(), S_vec.end());
            sort(r_vec.begin(), r_vec.end());
            sort(g_vec.begin(), g_vec.end());

            if(S_vec.size() > 1)
            {
                size_t s_median_frame = std::floor(S_vec.size() * 0.5);
                float S_median = S_vec[s_median_frame];
                //back_ground[col + 0 + row*3*num_cols] = S_median;
                s_mat(row_out, col/3) = S_median;
            }
            else
            {
                ASSERT(s_count(row_out, col/3) <= 1);
                //back_ground[col + 0 + row*3*num_cols] = 0.0; // S
                s_mat(row_out, col/3) = 0.0;
            }
            
            if(r_vec.size() > 1)
            {
                size_t r_median_frame = std::floor(r_vec.size() * 0.5);
                float r_median = r_vec[r_median_frame]; 
                //back_ground[col + 1 + row*3*num_cols] = r_median;
                r_mat(row_out, col/3) = r_median;
            }
            else
            {
                ASSERT(r_count(row_out, col/3) <= 1);
                //back_ground[col + 1 + row*3*num_cols] = 0.0; // r
                r_mat(row_out, col/3) = 0.0;
            }

            if(g_vec.size() > 1)
            {
                size_t g_median_frame = std::floor(g_vec.size() * 0.5);
                float g_median = g_vec[g_median_frame];
                //back_ground[col + 2 + row*3*num_cols] = g_median;
                g_mat(row_out, col/3) = g_median;
            }
            else
            {
                ASSERT(g_count(row_out, col/3) <= 1);
                //back_ground[col + 2 + row*3*num_cols] = g_median; // g
                g_mat(row_out, col/3) = 0.0; 
            }
        }
    }
}

} // namespace psi
} // namespace kjb
