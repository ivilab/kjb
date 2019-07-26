/* $Id: gr_polygon_renderer.cpp 21596 2017-07-30 23:33:36Z kobus $ */

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
|  Author: Luca Del Pero
|
* =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <iostream>
#include <vector>
#include "gr_cpp/gr_polygon_renderer.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "l/l_bits.h"


using namespace kjb;


/** Does not set color or material effects.
 * @param  p       The polygon to render
 */
void GL_Polygon_Renderer::wire_render(const Polygon & p)
{
#ifdef KJB_HAVE_OPENGL
    if(p.normal.get_length() != 0)
    {
        ASSERT(fabs(p.normal[3] - 1.0) < 1.0e-8);
        glNormal3d(p.normal[0], p.normal[1], p.normal[2]);
    }

    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i < p.pts.size(); i++)
    {
        glVertex4d((p.pts[i])[0], (p.pts[i])[1], (p.pts[i])[2], (p.pts[i])[3]);
    }
    if(p.pts.size() > 0)
    {
        glVertex4d((p.pts[0])[0], (p.pts[0])[1], (p.pts[0])[2], (p.pts[0])[3]);
    }
    glEnd();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/** Render each edge of this polygon with a different color.
 * The first edge will be rendered using the input id as a color,
 * the following edges will be rendered with an id sequentially
 * increasing
 *
 * @param p The polygon to render
 * @param start_id The color for the first edge of the polygon
 * @return The last id used
*/
unsigned int GL_Polygon_Renderer::wire_render_with_sequential_ids(const Polygon &p, unsigned int start_id)
{
    using namespace kjb_c;
#ifdef KJB_HAVE_OPENGL
    unsigned int pixel_color;

    if(p.normal.get_length() != 0)
    {
        ASSERT(fabs(p.normal[3] - 1.0) < 1.0e-8);
        glNormal3d(p.normal[0], p.normal[1], p.normal[2]);
    }
    glBegin(GL_LINES);
    unsigned int i = 0;

    for (i = 0; i < (p.pts.size() - 1); i++)
    {
        pixel_color = start_id;
        if(! kjb_is_bigendian())
        {
            bswap_u32((uint32_t *) &(pixel_color));
        }
        glColor4ub(pixel_color>>24&0xFF, pixel_color>>16&0xFF, pixel_color>>8&0xFF, pixel_color&0xFF);
        glVertex4d((p.pts[i])[0], (p.pts[i])[1], (p.pts[i])[2], (p.pts[i])[3]);
        glVertex4d((p.pts[i+1])[0], (p.pts[i+1])[1], (p.pts[i+1])[2], (p.pts[i+1])[3]);
        start_id++;
    }
    if(p.pts.size() > 0)
    {
        pixel_color = start_id;
        if(! kjb_is_bigendian())
        {
            bswap_u32((uint32_t *) &(pixel_color));
        }
        glColor4ub(pixel_color>>24&0xFF, pixel_color>>16&0xFF, pixel_color>>8&0xFF, pixel_color&0xFF);
        glVertex4d((p.pts[i])[0], (p.pts[i])[1], (p.pts[i])[2], (p.pts[i])[3]);
        glVertex4d((p.pts[0])[0], (p.pts[0])[1], (p.pts[0])[2], (p.pts[0])[3]);
    }
    glEnd();

    return start_id;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}


/** Render each edge of this polygon with a different color.
 * The first edge will be rendered using the input id as a color,
 * the following edges will be rendered with an id sequentially
 * increasing
 *
 * @param p The polygon to render
 * @param start_id The color for the first edge of the polygon
 * @return The last id used
*/
unsigned int GL_Polygon_Renderer::wire_render_with_sequential_ids_16bits
(
    const Polygon &p,
    unsigned int start_id
)
{
    using namespace kjb_c;

#ifdef KJB_HAVE_OPENGL

    if(p.normal.get_length() != 0)
    {
        ASSERT(fabs(p.normal[3] - 1.0) < 1.0e-8);
        glNormal3d(p.normal[0], p.normal[1], p.normal[2]);
    }
    glBegin(GL_LINES);
    unsigned int i = 0;

    for (i = 0; i < (p.pts.size() - 1); i++)
    {

        if(! kjb_is_bigendian())
        {
            glColor4ub(0, 0, start_id&0xFF, start_id>>8&0xFF);
        }
        else
        {
            glColor4ub(start_id>>8&0xFF, start_id&0xFF, 0, 0);
        }
        glVertex4d((p.pts[i])[0], (p.pts[i])[1], (p.pts[i])[2], (p.pts[i])[3]);
        glVertex4d((p.pts[i+1])[0], (p.pts[i+1])[1], (p.pts[i+1])[2], (p.pts[i+1])[3]);
        start_id++;
    }
    if(p.pts.size() > 0)
    {
        if(! kjb_is_bigendian())
        {
            glColor4ub(0, 0, start_id&0xFF, start_id>>8&0xFF);
        }
        else
        {
            glColor4ub(start_id>>8&0xFF, start_id&0xFF, 0, 0);
        }
        glVertex4d((p.pts[i])[0], (p.pts[i])[1], (p.pts[i])[2], (p.pts[i])[3]);
        glVertex4d((p.pts[0])[0], (p.pts[0])[1], (p.pts[0])[2], (p.pts[0])[3]);
    }
    glEnd();

    return start_id;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void GL_Polygon_Renderer::solid_render_with_bases(const Polygon & p, unsigned int base1, unsigned int base2)
{
    using namespace kjb_c;
#ifdef KJB_HAVE_OPENGL

    if(! kjb_is_bigendian())
    {
        glColor4ub(base1&0xFF, base2&0xFF, 0, 0);
    }
    else
    {
        glColor4ub(0, 0, base2&0xFF, base1&0xFF);
    }
    if(p.normal.get_length() != 0)
    {
        ASSERT(fabs(p.normal[3] - 1.0) < 1.0e-8);
        glNormal3d(p.normal[0], p.normal[1], p.normal[2]);
    }

    glBegin(GL_POLYGON);
    for (unsigned int i = 0; i < p.pts.size(); i++)
    {
        glVertex4d(p.pts[i][0], p.pts[i][1], p.pts[i][2], p.pts[i][3]);
    }
    glEnd();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/**
 *
 * @param  p       The polygon to render
 * Does not render into the color buffer, only the depth buffer.
 *
 * After calling this method, call wire_render() to render the polygon with
 * hidden lines occluded.
 */
void GL_Polygon_Renderer::wire_occlude_render(const Polygon & p)
{
#ifdef KJB_HAVE_OPENGL
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    GLboolean color_mask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i < p.pts.size(); i++)
    {
        glVertex4d(p.pts[i][0], p.pts[i][1], p.pts[i][2], p.pts[i][3]);
    }
    if(p.pts.size() > 0)
    {
        glVertex4d(p.pts[0][0], p.pts[0][1], p.pts[0][2], p.pts[0][3]);
    }
    glEnd();

    glStencilFunc(GL_NOTEQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glEnable(GL_DEPTH_TEST);

    glBegin(GL_POLYGON);
    for (unsigned int i = 0; i < p.pts.size(); i++)
    {
        glVertex4d(p.pts[i][0], p.pts[i][1], p.pts[i][2], p.pts[i][3]);
    }
    glEnd();

    glDisable(GL_STENCIL_TEST);

    glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/**
 * Does not render into the color buffer, only in the depth buffer.
 *
 * @param  p       The polygon to render
 */
void GL_Polygon_Renderer::solid_occlude_render(const Polygon & p)
{
#ifdef KJB_HAVE_OPENGL
    GLboolean color_mask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glEnable(GL_DEPTH_TEST);

    glBegin(GL_POLYGON);
    for (unsigned int i = 0; i < p.pts.size(); i++)
    {
        glVertex4d(p.pts[i][0], p.pts[i][1], p.pts[i][2], p.pts[i][3]);
    }
    glEnd();

    glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/**
 * @param  p       The polygon to render
 *  Does not set color or material effects.
 *  */
void GL_Polygon_Renderer::solid_render(const Polygon & p)
{
#ifdef KJB_HAVE_OPENGL
    if(p.normal.get_length() != 0)
    {
        ASSERT(fabs(p.normal[3] - 1.0) < 1.0e-8);
        glNormal3d(p.normal[0], p.normal[1], p.normal[2]);
    }

    glBegin(GL_POLYGON);
    for (unsigned int i = 0; i < p.pts.size(); i++)
    {
        glVertex4d(p.pts[i][0], p.pts[i][1], p.pts[i][2], p.pts[i][3]);
    }
    glEnd();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/**
 * @param p The polygon to project
 * The polygon points are transformed and projected onto the view plane by the
 * current modelview and projection matrix on the GL stack.
 *
 * The origin of the view plane coordinate system is in the lower left corner
 * and its extents are defined by the current GL viewport size.
 */
void GL_Polygon_Renderer::project(Polygon & p)
{
#ifdef KJB_HAVE_OPENGL
    using namespace kjb_c;

    GLdouble glM[16] = {0};

    glGetDoublev(GL_MODELVIEW_MATRIX, glM);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMultMatrixd(glM);
    glGetDoublev(GL_PROJECTION_MATRIX, glM);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    Matrix M(4,4);
    for (unsigned int row = 0; row < 4; row++)
    {
        for (unsigned int col = 0; col < 4; col++)
        {
            M(row, col) = glM[ col*4 + row  ];
        }
    }

    double width  = Base_gl_interface::get_gl_viewport_width();
    double height = Base_gl_interface::get_gl_viewport_height();

    for (unsigned int i = 0; i < p.pts.size(); i++)
    {
        Vector & pt = p.pts[ i ];

        pt = M*pt;

        double w = 1.0 / pt[ 3 ];
        pt[ 0 ] = 0.5*width* (pt[ 0 ]*w + 1.0);
        pt[ 1 ] = 0.5*height*(pt[ 1 ]*w + 1.0);
        pt[ 2 ] = 0;
        pt[ 3 ] = 1.0;

        /** We need to take into account that in opengl (0,0) is the bottom left
         * corner in the image, whereas we work with (0,0) as the top left corner.
         * We thus need to swap the y-coordinates accordingly
         */
        pt[1] = (height - 1) - pt[1];
    }

#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/**
 * @param p The polygon to project
 * The polygon points are transformed and projected onto the view plane by the
 * current modelview and projection matrix on the GL stack.
 *
 * The origin of the view plane coordinate system is in the lower left corner
 * and its extents are defined by the current GL viewport size.
 */
void GL_Polygon_Renderer::project(Polygon & p, const Matrix & M, double width, double height)
{
#ifdef KJB_HAVE_OPENGL
    using namespace kjb_c;

    for (unsigned int i = 0; i < p.pts.size(); i++)
    {
        Vector & pt = p.pts[ i ];

        pt = M*pt;

        double w = 1.0 / pt[ 3 ];
        pt[ 0 ] = 0.5*width* (pt[ 0 ]*w + 1.0);
        pt[ 1 ] = 0.5*height*(pt[ 1 ]*w + 1.0);
        pt[ 2 ] = 0;
        pt[ 3 ] = 1.0;

        /** We need to take into account that in opengl (0,0) is the bottom left
         * corner in the image, whereas we work with (0,0) as the top left corner.
         * We thus need to swap the y-coordinates accordingly
         */
        pt[1] = (height - 1) - pt[1];
    }

#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}
