/* $Id: gr_opengl_headers.h 18652 2015-03-16 16:27:26Z ksimek $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2014 by Kobus Barnard (author)
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
#ifndef GR_CPP_GR_OPENGL_HEADERS_H
#define GR_CPP_GR_OPENGL_HEADERS_H

/** 
 * Use this file to include the OpenGL headers in a platform-independent way.
 */

#if !defined(__gl_h_) && !defined(__GL_H__)
    #ifdef KJB_HAVE_GLEW
        #include <GL/glew.h>
    #endif
#endif

#ifdef KJB_HAVE_OSMESA
    #include <GL/osmesa.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
#else

#ifdef KJB_HAVE_OPENGL
    #ifdef MAC_OSX
        #include <OpenGL/glu.h>
        #include <OpenGL/gl.h>
        #ifdef KJB_HAVE_GLUT
            #include <GLUT/glut.h>
        #endif
    #else
        #ifdef WIN32
        #include <GL/gl.h>
        #include <GL/glu.h>
        #ifdef KJB_HAVE_GLUT
            #include <glut.h>
        #endif
        #else
            #include <GL/gl.h>
            #include <GL/glu.h>
            #ifdef KJB_HAVE_GLUT
                #include <GL/glut.h>
            #endif
        #endif
    #endif
#else /* !defined(KJB_HAVE_OPENGL)  */
typedef int GLUquadricObj;
typedef int GLuint;
typedef int GLenum;
static const GLenum GL_RED = -1;
static const GLenum GL_DEPTH_COMPONENT = -1;
#endif
#endif

#endif /*GR_CPP_GR_OPENGL_HEADERS_H */
