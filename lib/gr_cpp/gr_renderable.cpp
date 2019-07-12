/* $Id: gr_renderable.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/* =========================================================================== *
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
 * =========================================================================== */

#include "gr_cpp/gr_renderable.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_exception.h"

#include <iostream>

using std::bad_alloc;
using std::cout;
using std::cerr;
using std::endl;
using kjb::Vector;
using kjb::Illegal_argument;
using kjb::Runtime_error;
using kjb::Not_implemented;
using kjb::Index_out_of_bounds;
using kjb::Generic_renderable;

/**
 * The rendering framework used to render. The only one implemented
 * up to now is OpenGL
 */
unsigned int kjb::Abstract_renderable::_rendering_framework = kjb::Abstract_renderable::RI_OPENGL;

void Generic_renderable::wire_render() const
{
#ifdef KJB_HAVE_OPENGL
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    solid_render();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * Render object edges using our "offset" method.
 *
 * 1. Render solid model
 * 2. Render full wireframe behind solid model with width=2
 *
 * Pros:
 *   * Fast: Uses graphics hardware and only requires one extra render pass
 *   * Clean outlines: single pixel width, no missing pixels
 *   * works better for nurbs surfaces
 * Cons:
 *   * Outlines are 1 pixel larger than object
 *   * Doesn't draw crease edges
 *   * need to set polygon offset right to avoid internal edges from appearing.
 *
 *   @param offset_factor "factor" parameter to glPolygonOffset
 *   @param offset_units "units" parameter to glPolygonOffset
 *
 *   @note Caller must set line color using glColor before calling this
 *
 *   If internal edges show through, try increasing factor or units
 *
 *   Assumes:
 *      <li> GL_DEPTH_TEST is enabled
 *      <li> glColor3f is set to something other than white
 */
void Generic_renderable::_opengl_offset_edge(double offset_factor, double offset_units) const
{
#ifdef KJB_HAVE_OPENGL
    // save state
    glPushAttrib(GL_ENABLE_BIT);
    glPushAttrib(GL_LINE_BIT);
    glPushAttrib(GL_CURRENT_BIT);

    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    if(offset_factor == 0) offset_factor = -1;
    if(offset_units == 0) offset_units = -1;

    // STEP 2: RENDER WHITE SOLID (OCCLUDES INTERNAL WIRE EDGES)
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(offset_factor, offset_units);
    ::kjb::opengl::glColor(background_color);
    solid_render();

    // STEP 1: RENDER WIREFRAME

    // push back in z direction to ensure solid render will overwrite this
    glDisable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_POLYGON_OFFSET_LINE);
    // this may need to be adjustable
    glPolygonOffset(1, 1);
    // must be > 1 so outline is larger than solid
    // this may need to be adjustable
    glLineWidth(2.);
    ::kjb::opengl::glColor(foreground_color);
    wire_render();
    glDisable(GL_POLYGON_OFFSET_LINE);

    glPopAttrib(); // GL_CURRENT_BIT (color)

    // clean up
    // glPopAttrib();
    glPopAttrib();
    glPopAttrib();
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

/**
 * Inexpensive, general purpose silhouette edge rendering using opengl stencils.
 *
 * Benefits:
 *      Fast: Silhouettes computed in graphics hardware
 *      No knowledge of geometry necessary.
 * Drawbacks:
 *      Slow: Requires rendering the model 3 times (4 if hollow)
 *      Doesn't draw crease edges;
 *      Some double-width edges at orthogonal surfaces
 *      Edge pixels
 *      not so good with nurbs
 * @param hollow set to true if both sides of a polygon are visible.  Enabling roughly doubles render time.
 */
void Generic_renderable::_opengl_stencil_edge(bool hollow) const
{
#ifdef KJB_HAVE_OPENGL
    if(!::kjb::opengl::has_stencil_buffer())
    {
        KJB_THROW_2(Runtime_error,
                    "Cannot render silhouette edges -- no stencil buffer exists.");
    }
    // Render silhouette edges using a neat opengl hack:
    // 1. set line width to 2 pixels
    // 2. render wire mesh of back-facing polygons
    // 3. render solid mesh of front-facing polygons
    // The outer half of the outside line will appear from behind the front surface

    glPushAttrib(GL_ENABLE_BIT);
//    glPushAttrib(GL_POLYGON_BIT);
    glPushAttrib(GL_CURRENT_BIT);
//    glPushAttrib(GL_COLOR_BUFFER_BIT);
//    glPushAttrib(GL_DEPTH_BUFFER_BIT);
//    glPushAttrib(GL_LINE_BIT);

        glDisable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glColor3f(1., 1., 1.);
        glDisable(GL_LINE_SMOOTH);

        // 1. Render solid into depth buffer; offset away from camera
        glEnable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_CULL_FACE);
        glPolygonOffset(1, 1);
        glColorMask(true, true, true, true);
//        glColorMask(false, false, false, false);
        // render into depth bufferd
        solid_render();


        // 2. Render edges into stencil buffer;
        //    use invert so double lines aren't drawn;
        //    cull back faces so only silhouette edges remain


        // respect depth buffer, but don't write to it
        glEnable(GL_DEPTH_TEST);
        glDepthMask(false);

        glPopAttrib(); // GL_CURRENT_BIT

        // pull lines toward camera (prevents stitching)
        glDisable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1, -1);


        // set up stencil
        glEnable(GL_STENCIL_TEST);
        glClear(GL_STENCIL_BUFFER_BIT);
        glStencilFunc(GL_ALWAYS, 1, 1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
        glStencilMask(0x1);

        glColorMask(false, false, false, false);

        // render front faces
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        wire_render();


        if(hollow)
        {
            // render back faces
            glCullFace(GL_FRONT);
            glStencilMask(0x2);
            wire_render();
        }

        glStencilMask(~0);
        glDisable(GL_CULL_FACE);

        // 3. render silhouette into depth buffer and color buffer
        //
        // Implementation note:
        //
        // We could just disable depth test, render a full-screen quad here,
        // and let the stencil handle it, but we wouldn't get subtle changes in
        // the depth buffer due to misalignment between the wire frame and solid
        // rendering.  This method increases render time somewhat by re-rendering
        // the geometry a fourth time, but the result doesn't exhibit stitching
        // when interacting with other objects.

        // draw ignore depth buffer, but still write to it
        glDepthFunc(GL_ALWAYS);
        glDepthMask(true);

        glStencilFunc(GL_NOTEQUAL, 0, 0x3);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        glColorMask(true, true, true, true);

        wire_render();

        // clean up
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_STENCIL_TEST);
    glPopAttrib();
//    glPopAttrib();
//    glPopAttrib();
//    glPopAttrib();
//    glPopAttrib();
//    glPopAttrib();
#else
    KJB_THROW_2(Missing_dependency, "Opengl");
#endif
}

Vector Generic_renderable::background_color = Vector().set(0.0,0.0,0.0,0.0);
Vector Generic_renderable::foreground_color = Vector().set(1.0,1.0,1.0,1.0);
