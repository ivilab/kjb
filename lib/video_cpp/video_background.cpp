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
   |  Author: Jinyan Guan, Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#include <video_cpp/video_background.h>
#include <i_cpp/i_image.h>
#include <boost/shared_array.hpp>
#include <vector>
#include <string>
#include <cmath>

using namespace kjb;

void kjb::compute_median_background
(
    const std::vector<std::string>& frames_fps,
    Matrix& r_mat,
    Matrix& g_mat,
    Matrix& s_mat,
    size_t sub_sampling_sz,
    size_t rate
)
{
    using namespace std;
    Image temp_img(frames_fps[0]);
    size_t num_rows= temp_img.get_num_rows();
    size_t num_cols = temp_img.get_num_cols();
    size_t num_frames = frames_fps.size();

    size_t new_nrows = num_rows / sub_sampling_sz;
    size_t new_ncols = num_cols / sub_sampling_sz;
    size_t new_sz = new_nrows * new_ncols;
    
    vector<boost::shared_array<float> > frames;
    frames.reserve(num_frames);
    for(size_t frame = 0; frame < num_frames; frame = frame + rate)
    {
        Image image(frames_fps[frame]);
        boost::shared_array<float> buffer(new float[3 * new_sz]);

        for(size_t row = 0; row < num_rows; row += sub_sampling_sz)
        {
            for(size_t col = 0; col < 3*num_cols; col += (3*sub_sampling_sz))
            {
                //size_t r = num_rows - row - 1;
                size_t brow = row / sub_sampling_sz;
                size_t bcol = col / sub_sampling_sz;

                buffer[bcol + 0 + brow*3*new_ncols] = 
                    (float) image(row, col/3, Image::RED);
                buffer[bcol + 1 + brow*3*new_ncols] = 
                    (float) image(row, col/3, Image::GREEN);
                buffer[bcol + 2 + brow*3*new_ncols] = 
                    (float) image(row, col/3, Image::BLUE);
            }
        }
        frames.push_back(buffer);
    }

    // Compute the medium for each pixel location 
    r_mat.resize(new_nrows, new_ncols);
    g_mat.resize(new_nrows, new_ncols);
    s_mat.resize(new_nrows, new_ncols);
    size_t nnum_frames = frames.size();
    size_t median_frame = std::floor(nnum_frames*0.5);
    for(size_t row = 0; row < new_nrows; row++)
    {
        for (size_t col = 0; col < 3*new_ncols; col += 3)
        {
            vector<float> R(nnum_frames); 
            vector<float> G(nnum_frames); 
            vector<float> S(nnum_frames); 
            for (size_t frame = 0; frame < nnum_frames; frame++)
            {
                float r = frames[frame][col + 0 + row*3*new_ncols];
                float g = frames[frame][col + 1 + row*3*new_ncols];
                float b = frames[frame][col + 2 + row*3*new_ncols];

                double s = r + g + b;
                if(s > FLT_EPSILON)
                {
                    R[frame] = r/s;
                    G[frame] = g/s;
                }
                else
                {
                    R[frame] = r;
                    G[frame] = g;
                }
                S[frame] = s;
            }
            sort(R.begin(), R.end());
            sort(G.begin(), G.end());
            sort(S.begin(), S.end());
            float R_median = R[median_frame];
            float G_median = G[median_frame];
            float S_median = S[median_frame];
            //int real_row = new_nrows - row - 1;
            r_mat(row, col/3) = R_median;
            g_mat(row, col/3) = G_median;
            s_mat(row, col/3) = S_median;
        }
    }
}

