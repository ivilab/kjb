/* $Id: gr_opengl_object.h 21599 2017-07-31 00:44:30Z kobus $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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

#ifndef KJB_GR_OPENGL_OBJECT
#define KJB_GR_OPENGL_OBJECT

#ifdef KJB_HAVE_OPENGL
#include "gr_cpp/gr_opengl_headers.h"

#include <l_cpp/l_exception.h>

#include <gr_cpp/gr_opengl_texture.h>

#ifdef DEBUGGING
#include <gr_cpp/gr_glut.h>
#endif

#include <map>

/**
 * @file 
 * A set of object-oriented wrappers for opengl objects (fbo's vbo's, textures, etc.)
 * Most of these objects are opengl extensions, so they require GLEW to compile.  
 * If Glew is not installed, most of these classes will be missing at compile time.
 * Users of these classes should wrap their usage inside KJB_HAVE_OPENGL and KJB_HAVE_GLEW guard statemenets.
 *
 * @author Kyle Simek
 *
 *
 * */


namespace kjb
{
namespace opengl
{

/**
 * Thin wrapper for Opengl renderbuffer objects.
 *
 * The benefit of this class:
 *    Automatic garbage collection.
 *    Adds type safety to opengl objects.
 *
 * @note Renderbuffer objects are an opengl extension, so this class will not exist if GLEW is not installed.
 */
#ifdef KJB_HAVE_GLEW
class Renderbuffer
{
public:
    /**
     * Claim ownership of a renderbuffer id.  This class
     * is now responsible for deleting the renderbuffer on 
     * destruction.
     */
    explicit Renderbuffer(GLuint renderbuffer_id);

    Renderbuffer();

    void allocate(GLenum internal_format, GLsizei width, GLsizei height);

    ~Renderbuffer();

    /**
     * Set this as the active renderbufferobject
     */
    void bind() const;
    
    /**
     * revert to no renderbuffer being active
     *
     * @warning If another renderbuffer is active, this will deactivate it.
     */
    void unbind() const;


    /**
     * Implicit conversion to GLUint.
     * This allows the object to be passed to native OpenGL routines.
     */
    operator GLuint() const;

    GLuint get() const;
private:
    // no copy
    Renderbuffer(const Renderbuffer&){}
    Renderbuffer& operator=(const Renderbuffer&){return *this;}

    GLuint handle_;
};
#endif /* HAVE_GLEW */

/**
 * A wrapper class for Opengl framebuffer objects (FBO's).
 *
 * Benefits of this clas:
 *     Automatic garbage collection.
 *     Simpler OO interface (no GLenums to remember!)
 *     Bind-scope management (all calls are bookeneded with calls to bind/unbind the object; no chance of dangling binds)
 *
 * @note Framebuffer objects are an opengl extension, so this class will not exist if GLEW is not installed.
 *
 * @author Kyle Simek
 *
 */
#ifdef KJB_HAVE_GLEW
class Framebuffer_object
{
public:
    Framebuffer_object();
    
    /**
     * Claim ownership of fbo;  This class is now
     * responsible for deleting the object on destruction.
     */
    explicit Framebuffer_object(GLuint fbo_id);
    
    /**
     * This only destroys the framebuffers, not the attached surfaces
     */
    ~Framebuffer_object();

    void attach(const Renderbuffer& renderbuffer, GLenum target);

    void attach(const Texture& texture, GLenum target);

    void attach_color(const Renderbuffer& renderbuffer, int i = 0);

    void attach_depth(const Renderbuffer& renderbuffer);

    void attach_stencil(const Renderbuffer& renderbuffer);


    void attach_color(const Texture& texture, int i = 0);

    void attach_depth(const Texture& texture);

    void attach_stencil(const Texture& texture);

    /**
     * Check that framebuffer status is valid.  If invalid, an exception is
     * thrown, otherwise nothing happens.
     */
    void check() const;

    /**
     * Set this as the active framebuffer object
     */
    void bind() const;

    /**
     * Revert to no framebuffer object being active.
     *
     * @warning If another framebuffer is active, this will deactivate it.
     */
    void unbind() const;

    /**
     * Implicit conversion to GLUint.
     * This allows the object to be passed to native OpenGL routines.
     */
    operator GLuint() const;

    GLuint get() const;
private:
    GLuint handle_;
};
#endif /* KJB_HAVE_GLEW */

/**
 * A thin wrapper for opengl buffer object (for vertex buffers (vbo's) and pixel buffers (pbo's)).
 *
 * 
 * @warning This class is not to be confused with Renderbuffers, which are used with Framebuffers.  This class has nothing to do with framebuffers.
 *
 * @note Buffer objects are an opengl extension, so this class will not exist if GLEW is not installed.
 */
#ifdef KJB_HAVE_GLEW
class Buffer
{
    // copying not allowed,  shared pointers are a possible alternative
    Buffer(const Buffer& ) {}; // no copying; will cause double-free
    Buffer& operator=(const Buffer& ) {return *this;}; // no copying; will cause double-free
public:
    /**
     * Claim ownership of an existing c-style buffer handle.  This object is now responsible
     * for deleting the buffer on destruction.
     */
    Buffer(GLuint buffer_id);

    Buffer();

    /**
     * allocate memory buffer and initialize with data from _data_.  
     * This is a thin wrapper for glBufferData.
     *
     * @param size The size in bytes of the buffer
     * @param data The data to initialize the buffer with.  Must be at least size bytes.
     * @param usage A hint to opengl as to how the buffer will be used.  See glBufferData for valid values.
     *
     * @see glBufferData
     */
    void allocate(GLenum type, int size, GLvoid* data, GLenum usage_hint);

    /**
     * allocate memory buffer but don't initialize.  Useful for buffers that
     * will store buffer data.
     *
     * This is a thin wrapper for glBufferData.
     *
     * @param type Type of buffer.  Usually GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_PACK_BUFFER, or GL_PIXEL_UNPACK_BUFFER See glBufferData for choices.
     * @param size The size in bytes of the buffer
     * @param usage A hint to opengl as to how the buffer will be used.  See glBufferData for valid values.
     *
     * @see glBufferData
     */
    void allocate(GLenum type, int size, GLenum usage);

    void copy(size_t offset, size_t size, GLvoid* data);

    ~Buffer();

    void bind() const;

    /**
     * Bind to another type than this buffer's type.
     * This is useful when you want to read pixel data from
     * the framebuffer, and then use those values as a vertex list.
     *
     * @param type  One of the target types listed in glBindBuffer
     */
    void bind(GLenum type) const;

    void unbind() const;

    /**
     * complement to bind(type);
     */
    void unbind(GLenum type) const;

    /**
     * Implicit conversion to GLUint.
     * This allows the object to be passed to native OpenGL routines.
     */
    operator GLuint() const;

    GLuint get() const;
public:
    GLuint handle_;
    GLenum type_;
};


void allocate_grayscale_color_buffer(::kjb::opengl::Renderbuffer& color, int width, int height);

#endif /* HAVE_GLEW */



} // namespace opengl
} // namespace kjb

#endif /* HAVE_OPENGL */
#endif
