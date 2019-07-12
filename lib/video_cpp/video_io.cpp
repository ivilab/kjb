/* $Id: video_io.cpp 11310 2011-12-17 04:57:52Z ksimek $ */
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

#include <video_cpp/video_io.h>
#include <i_cpp/i_image.h>
#include <gr_cpp/gr_opengl.h>
#include <l_cpp/l_filesystem.h>

namespace kjb
{

namespace opengl
{

/**
 * Generate a set of video frames using opengl.  User provides two 
 * callbacks: one to render the current scene, and one to advance the scene state.
 *
 * To render an offscreen buffer, simply activate/bind the buffer before calling this.
 *
 * @param draw Draw callback.  This should paint the current scene to the current opengl colorbuffer.  This reads from the backbuffer by default, so the draw method should NOT include a call to swap the front and back buffers.
 * @param tick "Tick" callback.  Calling this should advance the scene state by one frame
 * @param num_frames Number of frames to generate
 * @param frame_fmt Printf-formatted string representing filenames to draw frames to (e.g. frame_%03d.jpg).  File extension will determine the image format used
 */
void generate_video_frames(
    const boost::function0<void>& draw,
    const boost::function0<void>& tick,
    const size_t num_frames,
    const std::string& frame_out_fmt)
{
    Image img;
    std::vector<std::string> out_fnames = strings_from_format(frame_out_fmt, num_frames);

    for(size_t i = 0; i < num_frames; i++)
    {
        draw();
        opengl::get_framebuffer_as_image().write(out_fnames[i]);
        tick();
    }
}

} // namespace opengl
} // namespace kjb
