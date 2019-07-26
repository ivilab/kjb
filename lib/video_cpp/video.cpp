/* $Id: video.cpp 19023 2015-05-08 13:56:44Z ernesto $ */
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

#include <video_cpp/video.h>
#include <i_cpp/i_image.h>

namespace kjb
{
bool Video::ffmpeg_registered_ = false;

Image Video_frame::to_image() const
{
    size_t num_cols = width_;
    size_t num_rows = height_;
    Image img(num_rows, num_cols);
    size_t stride = 3 * num_cols;

    size_t out_col = 0;
    int out_row = num_rows - 1;
    size_t offset = 0;

    for(size_t row = 0; row < num_rows; row++)
    {
        for(size_t col = 0; col < 3*num_cols; col += 3)
        {
            img(out_row, out_col, Image::RED) = (float) data_[offset + col + 0];
            img(out_row, out_col, Image::GREEN) = (float) data_[offset + col + 1];
            img(out_row, out_col, Image::BLUE) = (float) data_[offset + col + 2];
            ++out_col;
        }
        --out_row;
        offset += stride;
    }

    return img;
}

}
