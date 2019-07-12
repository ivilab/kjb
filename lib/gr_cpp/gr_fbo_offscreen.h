/* $Id: gr_fbo_offscreen.h 21599 2017-07-31 00:44:30Z kobus $ */
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

#include "gr_cpp/gr_opengl_object.h"
#include "gr_cpp/gr_renderable.h"

#include <boost/scoped_ptr.hpp>

// vim: tabstop=4 shiftwidth=4 foldmethod=marker
#ifndef KJB_GR_FBO_OFFSCREEN_H
#define KJB_GR_FBO_OFFSCREEN_H

#ifdef KJB_HAVE_OPENGL
#ifdef KJB_HAVE_GLEW

namespace kjb
{
namespace opengl
{

/**
 * A wrapper of opengl Framebuffer-objects to use them as a simple offscreen buffer.
 *
 */
class Fbo_offscreen_buffer
{
public:
    Fbo_offscreen_buffer(int width_in, int height_in, bool grayscale=true);

    /**
     * Attach the Framebuffer_object so subsequent opengl calls will render to
     * this offscreen buffer
     * This will also set the viewport to be the entire buffer.  If you want a smaller
     * viewport, call glViewport after calling this.
     */
    void bind() const;


    /**
     * This will unbind the Framebuffer_object so subsequent opengl calls will
     * render to the inscreen buffer.
     *
     * This will also restore the viewport to its status before bind was called.
     */
    void unbind() const;

    /// @brief clear buffers and render the object into fbo.  Afterward, fbo will be unbound.
    void render(const Renderable& object);


    void init_color_fbo();
    void init_grayscale_fbo();

    int get_width() const { return width_; }
    int get_height() const { return height_; }

    ::kjb::opengl::Framebuffer_object& get_fbo() { return *fbo_; }
    ::kjb::opengl::Renderbuffer& get_depth_buffer() { return *depth_; }
    ::kjb::opengl::Renderbuffer& get_color_buffer() { return *color_; }
private:

    int width_;
    int height_;

    boost::scoped_ptr<kjb::opengl::Framebuffer_object> fbo_;
    boost::scoped_ptr<kjb::opengl::Renderbuffer> depth_;
    boost::scoped_ptr<kjb::opengl::Renderbuffer> color_;

    bool grayscale_;

    mutable GLint viewport_[4];
    mutable bool viewport_saved_;
};

} // namespace opengl
} // namespace kjb

#endif /* KJB_HAVE_GLEW */
#endif /* KJB_HAVE_OPENGL */
#endif
