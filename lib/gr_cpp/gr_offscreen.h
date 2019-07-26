/**
 * This work is licensed under a Creative Commons 
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 * 
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 * 
 * You are free:
 * 
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 * 
 * Under the following conditions:
 * 
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 * 
 *    Noncommercial. You may not use this work for commercial purposes.
 * 
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 * 
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 * 
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 * 
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

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
| Authors:
|     Joseph Schlecht, Luca Del Pero
|
* =========================================================================== */

/**
 * @file
 *
 * @author Robert Dionne
 * @author Joseph Schlecht
 * @author Luca Del Pero
 *
 * @brief Offscreen rendering buffer for X11 or Mac OS X.
 *
 * Under X11, pixmap and pbuffer are currently supported. For OS X, CGL pbuffers
 * are supported.
 */

#ifndef KJB_GR_OFFSCREEN_H
#define KJB_GR_OFFSCREEN_H

#include "l_cpp/l_exception.h"
#include "gr_cpp/gr_opengl_headers.h"

#ifdef KJB_HAVE_X11
#ifdef KJB_HAVE_OPENGL
#ifndef MAC_OSX

#ifdef KJB_HAVE_OSMESA
#include <GL/osmesa.h>
#else
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#endif

#else
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>
#endif

#endif
#endif



namespace kjb {


/** @brief Checks what offscreen rendering capabilities are available
 *  on the machine
 */
void retrieve_offscreen_capabilities(bool *_has_gl, bool *_has_osmesa);

/**@brief
 *
 *  DEPRECATED
 *  Returns true if pbuffers are available. ONLY FOR DEBUG PURPOSES
 */
bool supports_pbuffers();

/**@brief
 *
 *  DEPRECATED
 *  Returns true if pixelmaps are available. ONLY FOR DEBUG PURPOSES
 */
bool supports_pixmaps();

/**
 * @class Offscreen_buffer
 *
 * @brief Offscreen rendering buffer.
 */
class Offscreen_buffer
{
    public:

        /** @brief Constructs an offscreen buffer. */
        Offscreen_buffer(int iwidth, int iheight)
        { this->width = iwidth; this->height = iheight; active = false; }

        /** @brief Deletes an offscreen buffer. */
        virtual ~Offscreen_buffer() { }

        /** @brief Activates an offscreen buffer. */
        virtual void activate() throw(kjb::Exception) = 0;

        /** @brief Deactivates an offscreen buffer. */
        virtual void deactivate() throw(kjb::Exception) = 0;

        /** @brief Reactivates an offscreen buffer. */
        virtual void reactivate() throw(kjb::Exception) = 0;

    protected:

        /** @brief The offscreen buffer is active. */
        bool active;

        /** @brief Width of the offscreen buffer. */
        int width; 

        /** @brief height of the offscreen buffer. */
        int height;
};


#if defined KJB_HAVE_X11 && defined KJB_HAVE_OPENGL
#ifndef KJB_HAVE_OSMESA
#ifndef MAC_OSX
/**
 * @class GLX_offscreen_buffer
 *
 * @brief GLX Offscreen rendering buffer.
 */
class GLX_offscreen_buffer : public Offscreen_buffer
{
    public:

        /** @brief Constructs an offscreen buffer for GLX. */
        GLX_offscreen_buffer(int iwidth, int iheight)
            : Offscreen_buffer(iwidth, iheight) { display_open = false; }

        /** @brief Deletes an offscreen buffer for GLX. */
        virtual ~GLX_offscreen_buffer() { }

        /** @brief Activates an offscreen buffer for GLX. */
        virtual void activate() throw(kjb::Exception);

        /** @brief Deactivates an offscreen buffer for GLX. */
        virtual void deactivate() throw(kjb::Exception);

        /** @brief Reactivates an offscreen buffer for GLX. */
        virtual void reactivate() throw(kjb::Exception);

    protected:

        /** 
         * @brief Saves the current GLX drawable object and context for later
         * restoration.
         */
        void save_old_context();

        /** @brief Opens the current X11 display. */
        void open_display(const char* disp_name=0) 
            throw(kjb::Result_error);

        /** @brief Creates the GLX drawable object and context. */
        virtual void create_drawable_and_context(int* attrs)
            throw(kjb::Exception) = 0;

        /** @brief Destroys the GLX drawable object. */
        virtual void destroy_drawable() = 0;

        /** @brief returns true if pbuffers are supported */
        bool has_pbuffer_extension() const;

        /** @brief returns true if pixmap are supported */
        bool has_pixmap_extension() const;

    protected:

        /** @brief The X11 display is open. */
        bool display_open;

        /** @brief The X11 display. */
        Display* display;

        /** @brief GLX drawable object. */
        GLXDrawable  drawable;
        
        /** @brief Previous GLX drawable object. */
        GLXDrawable old_drawable;

        /** @brief GLX context. */
        GLXContext context;

        /** @brief Previous GLX context. */
        GLXContext old_context;
};


/**
 * @class GLX_offscreen_pixmap
 *
 * @brief GLX offscreen rendering pixmap buffer.
 *
 * Uses the GLX pixmap extension for offscreen rendering.
 */
class GLX_offscreen_pixmap : public GLX_offscreen_buffer
{
    public:

        /** @brief Constructs an offscreen pixmap for GLX. */
        GLX_offscreen_pixmap(int width, int height, int* attrs=0)
            throw(kjb::Exception);
        
        /** @brief Deletes an offscreen pixmap for GLX. */
        virtual ~GLX_offscreen_pixmap();

    protected:

        /** @brief Creates the GLX drawable object and context. */
        virtual void create_drawable_and_context(int* attrs)
            throw(kjb::Result_error);

        /** @brief Destroys the GLX drawable object. */
        virtual void destroy_drawable();

    protected:

        /** @brief GLX pixmap to draw in. */
        Pixmap pixmap;
};


/**
 * @class GLX_offscreen_pbuffer
 *
 * @brief GLX offscreen rendering pbuffer buffer.
 *
 * Uses the SGI pbuffer extension for offscreen rendering.
 */
class GLX_offscreen_pbuffer : public GLX_offscreen_buffer
{
    public:

        /** @brief Constructs an offscreen pbuffer for GLX. */
        GLX_offscreen_pbuffer(int width, int height)
            throw(kjb::Exception);

        /** @brief Constructs an offscreen pbuffer for GLX. */
        GLX_offscreen_pbuffer(int width, int heigth, int* attrs)
            throw (kjb::Exception);

        /** @brief Deletes an offscreen pbuffer for GLX. */
        virtual ~GLX_offscreen_pbuffer();

    protected:

        /** @brief Creates the GLX drawable object and context. */
        virtual void create_drawable_and_context(int* attrs)
            throw(kjb::Exception);

        /** @brief Destroys the GLX drawable object. */
        virtual void destroy_drawable();
};


#else
/**
 * @class CGL_offscreen_pbuffer
 *
 * @brief CGL Offscreen rendering buffer.
 */
class CGL_offscreen_pbuffer : public Offscreen_buffer
{
    public:

        /** @brief Constructs an offscreen pbuffer for CGL. */
        CGL_offscreen_pbuffer
        (
            GLsizei width, 
            GLsizei height, 
            GLenum  format=GL_RGBA,
            const CGLPixelFormatAttribute* attrs=0
        )
        throw (kjb::Exception);

        /** @brief Deletes an offscreen pbuffer for CGL. */
        virtual ~CGL_offscreen_pbuffer();

        /** @brief Activates an offscreen buffer for CGL. */
        virtual void activate() throw(kjb::Exception);

        /** @brief Deactivates an offscreen buffer for CGL. */
        virtual void deactivate() throw(kjb::Exception);

        /** @brief Reactivates an offscreen buffer for CGL. */
        virtual void reactivate() throw(kjb::Exception);

    protected:

        /** 
         * @brief Saves the current CGL context for later restoration.
         */
        virtual void save_old_context();

        /** @brief Creates the drawing context. */
        virtual void create_pbuffer_and_context
        (
            GLenum format,
            const CGLPixelFormatAttribute* attrs
        )
        throw(kjb::Exception);

    protected:

        /** @brief CGL context. */
        CGLContextObj context;

        /** @brief Previous CGL context. */
        CGLContextObj old_context;

        /** @brief CGL pbuffer. */
        CGLPBufferObj pbuffer;
};
#endif
#endif
#endif

#ifdef KJB_HAVE_OSMESA
/**
 * @class OSMesa32_offscreen_buffer
 *
 * @brief OSMesa32 Offscreen rendering buffer.
 */
class OSMesa32_offscreen_buffer : public Offscreen_buffer
{
    public:

        /** @brief Constructs an offscreen buffer for OSMesa32. */
        OSMesa32_offscreen_buffer
        (
            GLsizei width, 
            GLsizei height, 
            GLenum  format=OSMESA_RGBA,
            GLint   depth_bits=4,
            GLint   stencil_bits=4,
            GLint   accum_bits=4
        )
        throw (kjb::Exception);

        /** @brief Deletes an offscreen buffer for OSMesa32. */
        virtual ~OSMesa32_offscreen_buffer();

        /** @brief Activates an offscreen buffer for OSMesa32. */
        virtual void activate() throw(kjb::Exception);

        /** @brief Deactivates an offscreen buffer for OSMesa32. */
        virtual void deactivate() throw(kjb::Exception);

        /** @brief Reactivates an offscreen buffer for OSMesa32. */
        virtual void reactivate() throw(kjb::Exception);

    protected:

        /** @brief Creates the drawing context. */
        virtual void create_buffer_and_context
        (
            GLenum format,
            GLint  depth_bits,
            GLint  stencil_bits,
            GLint  accum_bits
        )
        throw(kjb::Exception);

    protected:

        /** @brief OSMesa32 context. */
        OSMesaContext context;

        /** @brief OSMesa32 buffer. */
        void* buffer;
};
#endif

#ifdef KJB_HAVE_OPENGL

/** @brief This function will create an offscreen buffer using the available
 * offscreen rendering capabilities.
 * The capabilities will be checked in the following order:
 * GLX Pbuffers (reguires glx version 1.3 or higher + x11 + SGI)
 * GLX Pixelmaps (reguires glx version 1.3 or higher + x11)
 * osmesa buffers (requires osmesa). We prefer glx offscreen
 * rendering because it uses hardware acceleration, osmesa does
 * everything in software. Further, pbuffers are more efficient
 *
 * @param iwidth the width of the offscreen buffer
 * @param iheight the width of the offscreen buffer
 * @param use_pbuffers_when_available if set to true, pixelmaps will be
 *        preferred to pbuffers
 */
Offscreen_buffer * create_default_offscreen_buffer
(
    GLsizei iwidth,
    GLsizei iheight,
    bool use_pbuffers_when_available = true
);

/**@brief This function will create an offscreen buffer using the available
 * offscreen rendering capabilities, and will activate it and make
 * it ready for rendering (for more details see create_default_offscreen_buffer).
 *
 * @param iwidth the width of the offscreen buffer
 * @param iheight the width of the offscreen buffer
 * @param use_pbuffers_when_available if set to true, pixelmaps will be
 *        preferred to pbuffers
 */
Offscreen_buffer * create_and_initialize_offscreen_buffer
(
    unsigned int width,
    unsigned int height,
    bool use_pbuffers_when_available = true
);

#endif

}


#endif
