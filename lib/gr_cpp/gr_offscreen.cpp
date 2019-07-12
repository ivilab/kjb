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

#include <cstdlib>
/*
#include <cassert>
*/
#include <cstring>
#include <sstream>

#include "gr_cpp/gr_opengl.h"

#if defined KJB_HAVE_X11 && defined KJB_HAVE_OPENGL
#ifndef MAC_OSX
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#else
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#endif
#endif


#include "gr_cpp/gr_offscreen.h"

using namespace kjb;


/**
 *  Checks what offscreen rendering capabilities are available
 *  on the machine
 *
 *  @param _has_gl Will be true if open gl rendering is available
 *  @param _has_osmesa Will be true if libosmesa is available
 */
void kjb::retrieve_offscreen_capabilities(bool *_has_gl, bool *_has_osmesa)
{
    *_has_gl = false;
    *_has_osmesa = false;
#if defined KJB_HAVE_X11 && defined KJB_HAVE_OPENGL
    *_has_gl = true;
#endif

#if defined KJB_HAVE_OSMESA
    *_has_osmesa = true;
#endif

}

/**
 *  DEPRECATED
 *  Returns true if pbuffers are available. ONLY FOR DEBUG PURPOSES
 */
bool kjb::supports_pbuffers()
{
#ifdef KJB_HAVE_OSMESA
    return false;
#else
#if defined KJB_HAVE_X11 && defined KJB_HAVE_OPENGL
#ifndef MAC_OSX
    try{
        kjb::GLX_offscreen_pbuffer _temp_buffer(100, 100, NULL);
    }
    catch(kjb::Result_error e)
    {
        return false;
    }

    return true;
#else
    /** We assume here that framework CGL always supports pbuffers on MAC_OSX */
    return true;
#endif
#endif
#endif
    return false;
}

/**
 *  DEPRECATED
 *  Returns true if pixel maps are available. ONLY FOR DEBUG PURPOSES
 */
bool kjb::supports_pixmaps()
{
#ifdef KJB_HAVE_OSMESA
    return false;
#else
#if defined KJB_HAVE_X11 && defined KJB_HAVE_OPENGL
#ifndef MAC_OSX
    try{
        kjb::GLX_offscreen_pixmap _temp_pixmap(100, 100, NULL);
    }
    catch(kjb::Result_error e)
    {
        return false;
    }

    return true;
#else
    /** We assume here that framework CGL always supports pbuffers on MAC_OSX */
    return true;
#endif
#endif
#endif
    return false;
}

#if defined KJB_HAVE_X11 && defined KJB_HAVE_OPENGL
#ifndef MAC_OSX
#ifndef KJB_HAVE_OSMESA
/* -----------------------  GLX_offscreen_buffer  --------------------------- */

void GLX_offscreen_buffer::activate() throw (kjb::Exception)
{
    if (!active)
    {
        if (!(active = glXMakeCurrent(display, drawable, context)))
        {
            throw kjb::Exception(
                    "Could not make drawable context current");
        }
    }
}

void GLX_offscreen_buffer::deactivate() throw (kjb::Exception)
{
    if (active)
    {
        if ((active = !glXMakeCurrent(display, old_drawable, old_context)))
        {
            throw kjb::Exception(
                    "Could not make old drawable context current");
        }
    }
}

void GLX_offscreen_buffer::reactivate() throw (kjb::Exception)
{
    if (!active)
    {
        throw kjb::Exception("No active context to reactivate");
    }
    if (glXMakeCurrent(display, drawable, context))
    {
        throw kjb::Exception("Could not make context current");
    }
}

void GLX_offscreen_buffer::open_display(const char* disp_name)
    throw(kjb::Result_error)
{
    display = glXGetCurrentDisplay();

    if (!display)
    {
        display = XOpenDisplay(disp_name);

        if (!display && !disp_name)
        {
            std::ostringstream ost;
            ost << "Could not open X display '" << getenv("DISPLAY") << '\'';
            throw kjb::Result_error(ost.str());
        }
        else if (!display)
        {
            std::ostringstream ost;
            ost << "Could not open X display '" << disp_name << '\'';
            throw kjb::Result_error(ost.str());
        }

        display_open = true;
    }
}

void GLX_offscreen_buffer::save_old_context()
{
    // Grab the current drawable and context there might be none, in which case
    // these will be 0.
    old_drawable = glXGetCurrentDrawable();
    old_context = glXGetCurrentContext();
}


bool GLX_offscreen_buffer::has_pbuffer_extension() const
{
    const char* version_string = glXQueryServerString(display, 0, GLX_VERSION); 
    if(!version_string)
    {
        return false;
    }
    float version = atof(version_string);
    if(version < 1.29)
    {
        return false;
    }
    
    bool has_pbuffer = false;

    const char* ext = glXQueryServerString(display, 0, GLX_EXTENSIONS);
    char* ext_buf = (char*)malloc(strlen(ext)*sizeof(char));
    char* ext_ptr = ext_buf;
    strcpy(ext_ptr, ext);
    ext_ptr = strtok(ext_ptr, " ");
    do
    {
        if (strcmp(ext_ptr, "GLX_SGIX_pbuffer") == 0)
        {
            has_pbuffer = true;
        }
    } while ((ext_ptr = strtok(0, " ")));
    free(ext_buf);

    return has_pbuffer;
}

bool GLX_offscreen_buffer::has_pixmap_extension() const
{
    const char* version_string = glXQueryServerString(display, 0, GLX_VERSION);
    if(!version_string)
    {
        return false;
    }
    float version = atof(version_string);
    if(version < 1.29)
    {
        return false;
    }

   return true;
}

/* ---------------------------------------------------------------------------*/







/* -------------------------  GLX_offscreen_pixmap  --------------------------*/

GLX_offscreen_pixmap::GLX_offscreen_pixmap
(
    int  iwidth,
    int  iheight,
    int* attrs
)
throw (kjb::Exception) : GLX_offscreen_buffer(iwidth, iheight)
{
    open_display();
    if (!has_pixmap_extension())
    {
        throw kjb::Result_error("Pixel maps not supported");
    }

    save_old_context();
    create_drawable_and_context(attrs);
}

GLX_offscreen_pixmap::~GLX_offscreen_pixmap()
{
    if (active)
    {
        deactivate();
    }

    glXDestroyContext(display, context);
    destroy_drawable();

    if (display_open)
    {
        XCloseDisplay(display);
    }
}

void GLX_offscreen_pixmap::create_drawable_and_context(int* attrs)
    throw (kjb::Result_error)
{
    static int default_attrs[] = { GLX_RGBA,
                                   GLX_RED_SIZE,     8,
                                   GLX_GREEN_SIZE,   8,
                                   GLX_BLUE_SIZE,    8,
                                   GLX_ALPHA_SIZE,   8,
                                   GLX_DEPTH_SIZE,   4,
                                   GLX_STENCIL_SIZE, 4,
                                   None };

    if (!attrs)
    {
        attrs = default_attrs;
    }

    XVisualInfo *visual = glXChooseVisual(display, 
            DefaultScreen(display), attrs);
    if (!visual)
    {
        throw kjb::Result_error("Could not get X visual");
    }

    pixmap = XCreatePixmap(display, DefaultRootWindow(display),
                width, height, visual->depth);
    if (!pixmap)
    {
        throw kjb::Result_error("Could not create X11 pixmap");
    }

    drawable = glXCreateGLXPixmap(display, visual, pixmap);
    if (!drawable)
    {
        throw kjb::Result_error("Could not create GLX pixmap");
    }

    context = glXCreateContext(display, visual, 0, GL_FALSE);
    if (!context)
    {
        throw kjb::Result_error("Could not create GLX context");
    }
}

void GLX_offscreen_pixmap::destroy_drawable()
{
    glXDestroyPixmap(display, drawable);
    XFreePixmap(display, pixmap);
}

/* ---------------------------------------------------------------------------*/






/* -------------------------  GLX_offscreen_pbuffer  -------------------------*/

GLX_offscreen_pbuffer::GLX_offscreen_pbuffer
(
    int  iwidth,
    int  iheight,
    int* attrs
)
throw (kjb::Exception)
    : GLX_offscreen_buffer(iwidth, iheight)
{
    open_display();

    if (!has_pbuffer_extension())
    {
        throw kjb::Result_error("PBuffers not supported");
    }

    save_old_context();
    create_drawable_and_context(attrs);
}

GLX_offscreen_pbuffer::~GLX_offscreen_pbuffer()
{
    if (active)
    {
        deactivate();
    }

    glXDestroyContext(display, context);
    destroy_drawable();

    if (display_open)
    {
        XCloseDisplay(display);
    }
}
    
void GLX_offscreen_pbuffer::create_drawable_and_context(int* attrs)
    throw(kjb::Exception)
{
    static int default_attrs[] = { GLX_RED_SIZE,     8,
                                   GLX_GREEN_SIZE,   8,
                                   GLX_BLUE_SIZE,    8,
                                   GLX_ALPHA_SIZE,   8,
                                   GLX_DEPTH_SIZE,   4,
                                   GLX_STENCIL_SIZE, 1,
                                   None };

    int pbuf_attrs[] = { GLX_PBUFFER_WIDTH, width, 
                         GLX_PBUFFER_HEIGHT, height,
                         GLX_PRESERVED_CONTENTS, GL_TRUE,
                         None };

    if (!attrs)
    {
        attrs = default_attrs;
    }

    int nlist;
    GLXFBConfig *fbconfig = glXChooseFBConfig(display, DefaultScreen(display),
                attrs, &nlist);
    if (!fbconfig)
    {
        throw kjb::Result_error("Could not create pbuffer");
    }

    for (int i = 0; i < nlist; i++)
    {
        drawable = glXCreatePbuffer(display, fbconfig[i], pbuf_attrs);

        if (drawable)
        {
            context = glXCreateNewContext(display, fbconfig[i], GLX_RGBA_TYPE,
                    0, GL_TRUE);

            XFree(fbconfig);

            if(!context)
            {
                throw kjb::Result_error("Could not create context");
            }

            return;
        }
    }

    XFree(fbconfig);

    throw kjb::Result_error("Could not find valid framebuffer config");
}

void GLX_offscreen_pbuffer::destroy_drawable()
{
    glXDestroyPbuffer(display, drawable);
}

/* ---------------------------------------------------------------------------*/
#endif


#else
/* -------------------------  CGL_offscreen_pbuffer  -------------------------*/

/**
 * Choices for format: GL_RGB or GL_RGBA.
 */
CGL_offscreen_pbuffer::CGL_offscreen_pbuffer
(
    GLsizei iwidth,
    GLsizei iheight,
    GLenum  format,
    const CGLPixelFormatAttribute* attrs
)
throw (kjb::Exception) : Offscreen_buffer(iwidth, iheight)
{
    save_old_context();
    create_pbuffer_and_context(format, attrs);
}

CGL_offscreen_pbuffer::~CGL_offscreen_pbuffer()
{
    if (active)
    {
        deactivate();
    }

    CGLDestroyContext(context);
    CGLDestroyPBuffer(pbuffer);
}
    
void CGL_offscreen_pbuffer::activate() throw (kjb::Exception)
{
    if (!active)
    {
        active = !CGLSetCurrentContext(context);
        if (!active)
        {
            throw kjb::Exception("Could not make context current");
        }
    }
}

void CGL_offscreen_pbuffer::deactivate() throw (kjb::Exception)
{
    if (active)
    {
        active = CGLSetCurrentContext(old_context);
        if (active)
        {
            throw kjb::Exception(
                    "Could not make old context current");
        }
    }
}


void CGL_offscreen_pbuffer::reactivate() throw (kjb::Exception)
{
    if (!active)
    {
        throw kjb::Exception("No active context to reactivate");
    }
    if (CGLSetCurrentContext(context))
    {
        throw kjb::Exception("Could not make context current");
    }
}

void CGL_offscreen_pbuffer::create_pbuffer_and_context
(
    GLenum format,
    const CGLPixelFormatAttribute* attrs
)
throw(kjb::Exception)
{
    static const CGLPixelFormatAttribute default_attrs[] = 
                                      { kCGLPFAColorSize,   
                                        (CGLPixelFormatAttribute)24,
                                        kCGLPFAAlphaSize,   
                                        (CGLPixelFormatAttribute)8,
                                        kCGLPFADepthSize,   
                                        (CGLPixelFormatAttribute)32,
                                        kCGLPFAAccelerated,
                                        kCGLPFAPBuffer,
                                        (CGLPixelFormatAttribute)0 };

    if (!attrs)
    {
        attrs = default_attrs;
    }

    CGLPixelFormatObj pxlfmt;
    GLint nlist;
    if (CGLChoosePixelFormat(attrs, &pxlfmt, &nlist))
    {
        throw kjb::Result_error("Could not choose pixel format");
    }

    for (int i = 0; i < nlist; i++)
    {
        CGLError err = CGLCreateContext(pxlfmt, 0, &context);

        if (!err)
        {
            break;
        }
        else if (err != kCGLBadPixelFormat || 
                ((nlist - i) == 1 && err == kCGLBadPixelFormat))
        {
            CGLDestroyPixelFormat(pxlfmt);
            throw kjb::Result_error("Could not create context");
        }
    }

    CGLDestroyPixelFormat(pxlfmt);

    GLint screen;
    if (CGLGetVirtualScreen(context, &screen))
    {
        throw kjb::Result_error("Could not get virtual screen");
    }

    if (CGLCreatePBuffer(width, height, GL_TEXTURE_RECTANGLE_EXT, format, 0, 
                &pbuffer))
    {
        throw kjb::Result_error("Could not create pbuffer");
    }

    if (CGLSetPBuffer(context, pbuffer, 0, 0, screen))
    {
        throw kjb::Result_error("Could not set pbuffer");
    }
}

void CGL_offscreen_pbuffer::save_old_context()
{
    old_context = CGLGetCurrentContext();
}

/* ---------------------------------------------------------------------------*/
#endif
#endif




#if defined KJB_HAVE_OSMESA
/* ----------------------  OSMesa32_offscreen_buffer  ------------------------*/

/** 
 * Choices for format: OSMESA_COLOR_INDEX, OSMESA_RGBA, OSMESA_BGRA,
 * OSMESA_ARGB, OSMESA_RGB, or OSMESA_BGR.
 */
OSMesa32_offscreen_buffer::OSMesa32_offscreen_buffer
(
    GLsizei iwidth,
    GLsizei iheight,
    GLenum  format,
    GLint   depth_bits,
    GLint   stencil_bits,
    GLint   accum_bits
)
throw (kjb::Exception) : Offscreen_buffer(iwidth, iheight)
{
    create_buffer_and_context(format, depth_bits, stencil_bits, accum_bits);
}

OSMesa32_offscreen_buffer::~OSMesa32_offscreen_buffer()
{
    if (active)
    {
        deactivate();
    }

    OSMesaDestroyContext(context);
    free(buffer);
}
    
void OSMesa32_offscreen_buffer::activate() throw (kjb::Exception)
{
    if (!active)
    {
        if (!(active = OSMesaMakeCurrent(context, buffer, GL_UNSIGNED_BYTE, 
                        width, height)))
        {
            throw kjb::Exception("Could not make context current");
        }
    }
}

void OSMesa32_offscreen_buffer::deactivate() throw (kjb::Exception)
{
    if (active)
    {
        active = false;
    }
}

void OSMesa32_offscreen_buffer::reactivate() throw (kjb::Exception)
{
    if (!active)
    {
        throw kjb::Exception("No active context to reactivate");
    }
    // Nothing to do.
}

void OSMesa32_offscreen_buffer::create_buffer_and_context
(
    GLenum format,
    GLint  depth_bits,
    GLint  stencil_bits,
    GLint  accum_bits
)
throw(kjb::Exception)
{
    context = OSMesaCreateContextExt(format, depth_bits, stencil_bits, 
            accum_bits, 0);
    if (!context)
    {
        throw kjb::Result_error("Could not create context");
    }

    buffer = malloc(width * height * 4 * sizeof(GLbyte));
    if (!buffer)
    {
        throw kjb::Result_error("Could not create buffer");
    }
}

/* ---------------------------------------------------------------------------*/
#endif

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
#ifdef KJB_HAVE_OPENGL
Offscreen_buffer * kjb::create_default_offscreen_buffer
(
    GLsizei iwidth,
    GLsizei iheight,
    bool use_pbuffers_when_available
)
{

#ifdef KJB_HAVE_OSMESA
    /** If we have found OsMesa, it means this is the only offscreen
     *  rendering tool available on this machine */
    return new OSMesa32_offscreen_buffer(iwidth, iheight, OSMESA_RGBA, 4, 4, 4);
#else

#if defined KJB_HAVE_X11 && defined KJB_HAVE_OPENGL
#ifndef MAC_OSX
    if(kjb::supports_pbuffers() && use_pbuffers_when_available)
    {
        static int attrs_pbuffer[] =
        {   GLX_RED_SIZE,      8,
            GLX_GREEN_SIZE,    8,
            GLX_BLUE_SIZE,     8,
            GLX_ALPHA_SIZE,    8,
            GLX_DEPTH_SIZE,    4,
            GLX_STENCIL_SIZE,  1,
            None
        };
        try
        {
            return new GLX_offscreen_pbuffer(iwidth, iheight, attrs_pbuffer);
        }
        catch(KJB_error e)
        {
            throw KJB_error("PBUFFERS not supported");
        }
    }
    else if(kjb::supports_pixmaps())
    {
        static int attrs_pixmap[] =
        {   GLX_RGBA,
            GLX_RED_SIZE,      8,
            GLX_GREEN_SIZE,    8,
            GLX_BLUE_SIZE,     8,
            GLX_ALPHA_SIZE,    8,
            GLX_DEPTH_SIZE,    4,
            GLX_STENCIL_SIZE,  4,
            None
        };
        return new GLX_offscreen_pixmap(iwidth, iheight, attrs_pixmap);
    }
#else
    /** On a MAC, we assume pbuffers are always available */
    static const CGLPixelFormatAttribute attrs[] =
    {
        kCGLPFAAccelerated,
        kCGLPFASingleRenderer,
        kCGLPFAWindow,
        kCGLPFAPBuffer,
        kCGLPFAColorSize, (CGLPixelFormatAttribute)32,
        kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
        kCGLPFADepthSize, (CGLPixelFormatAttribute)32,
        kCGLPFAStencilSize, (CGLPixelFormatAttribute)8,
        (CGLPixelFormatAttribute)0
    };
    return new CGL_offscreen_pbuffer(iwidth, iheight, GL_RGBA, attrs);

#endif /* End of ifdef MACOSX */
#endif /* End of ifdef GL and X11 */
#endif /* End of ifdef OSMESA */
    
    throw KJB_error("This machine does not support any kind of offscreen rendering");
}

/** @brief This function will create an offscreen buffer using the available
 * offscreen rendering capabilities, and will activate it and make
 * it ready for rendering.
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
Offscreen_buffer * kjb::create_and_initialize_offscreen_buffer
(
    unsigned int width,
    unsigned int height,
    bool use_pbuffers_when_available
)
{
    using namespace kjb;
    Offscreen_buffer * offbuff = kjb::create_default_offscreen_buffer(width, height, use_pbuffers_when_available);
    offbuff->activate();
    kjb::opengl::default_init_opengl(width, height);
    return offbuff;

}
#endif
