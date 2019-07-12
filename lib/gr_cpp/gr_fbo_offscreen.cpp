/* $Id: gr_fbo_offscreen.cpp 18278 2014-11-25 01:42:10Z ksimek $ */
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
#include <gr_cpp/gr_fbo_offscreen.h>
#include <gr_cpp/gr_opengl.h>

#ifdef KJB_HAVE_OPENGL
#ifdef KJB_HAVE_GLEW
namespace kjb
{
namespace opengl
{

Fbo_offscreen_buffer::Fbo_offscreen_buffer(int width_in, int height_in, bool grayscale) :
    width_(width_in),
    height_(height_in),
    fbo_(),
    depth_(),
    color_(),
    grayscale_(grayscale),
    viewport_(),
    viewport_saved_(false)
{
    using namespace kjb::opengl;
    using boost::scoped_ptr;


    {
        scoped_ptr<Framebuffer_object> tmp(new Framebuffer_object());
        fbo_.swap(tmp);
    }

    {
        scoped_ptr<Renderbuffer> tmp(new Renderbuffer());
        depth_.swap(tmp);
    }

    {
        scoped_ptr<Renderbuffer> tmp(new Renderbuffer());
        color_.swap(tmp);
    }

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    if(grayscale)
        init_grayscale_fbo();
    else
        init_color_fbo();
}

void Fbo_offscreen_buffer::bind() const
{
    glGetIntegerv(GL_VIEWPORT, viewport_);
    viewport_saved_ = true;
    glViewport(0, 0, width_, height_);
    GL_ETX();

    fbo_->bind();
}

void Fbo_offscreen_buffer::unbind() const
{
    fbo_->unbind();

    if(viewport_saved_)
    {
        glViewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]);
        viewport_saved_ = false;
        GL_ETX();
    }
}

void Fbo_offscreen_buffer::render(const Renderable& object)
{
    bind();
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0,1.0,1.0);
    object.render();
    unbind();

    GL_ETX();
}

void Fbo_offscreen_buffer::init_color_fbo()
{
    grayscale_ = false;
//    depth_->allocate(GL_DEPTH_COMPONENT, width_, height_);
    depth_->allocate(GL_DEPTH_COMPONENT24, width_, height_);
//    color_->allocate(GL_RGBA, width_, height_);
    color_->allocate(GL_RGBA8, width_, height_);

    GL_ETX();
    fbo_->attach_depth(*depth_);
    fbo_->attach_color(*color_);
    GL_ETX();
    fbo_->check();
}


void Fbo_offscreen_buffer::init_grayscale_fbo()
{
    grayscale_ = true;
    depth_->allocate(GL_DEPTH_COMPONENT, width_, height_);
    ::kjb::opengl::allocate_grayscale_color_buffer(*color_, width_, height_);

    fbo_->attach_depth(*depth_);
    fbo_->attach_color(*color_);
    fbo_->check();
}

 
} // namespace opengl
} // namespace kjb
#endif /* KJB_HAVE_GLEW */
#endif /* KJB_HAVE_OPENGL */
