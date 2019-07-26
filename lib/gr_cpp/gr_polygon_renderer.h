
/* $Id: gr_polygon_renderer.h 18278 2014-11-25 01:42:10Z ksimek $ */

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

#ifndef POLYGON_RENDERER_H_INCLUDED
#define POLYGON_RENDERER_H_INCLUDED

#include <vector>
#include "gr_cpp/gr_polygon.h"
#include "gr_cpp/gr_camera.h"

namespace kjb {

class GL_Polygon_Renderer
{
public:

    /** @brief Renders this polygon as a wireframe using OpenGL */
    static void wire_render(const Polygon &);

    /** @brief Renders this polygon into the depth buffer using OpenGL, excluding the contour */
    static void wire_occlude_render(const Polygon &);

    /** @brief Renders this polygon in the Z buffer */
    static void solid_occlude_render(const Polygon &);

    /** @brief Render each edge of this polygon with a different color.
     * The first edge will be rendered using the input id as a color,
     * the following edges will be rendered with an id sequentially
     * increasing. Returns the id (color) used to render the last
     * edge of the polygon
     */
    static unsigned int wire_render_with_sequential_ids(const Polygon &, unsigned int start_id = 1);

    static unsigned int wire_render_with_sequential_ids_16bits
    (
        const Polygon &p,
        unsigned int start_id = 1
    );

    /** @brief Renders this polygon as a solid using OpenGL */
    static void solid_render(const Polygon &);

    /** @brief Renders this polygon as a solid using OpenGL */
    static void solid_render_with_bases(const Polygon & p, unsigned int base1 = 0, unsigned int base2 = 0);

    /** @brief Projects this polygon onto the image plane using the current OpenGL transformation */
    static void project(Polygon & p);

    static void project(Polygon & p, const Matrix & M, double width, double height);
};

}

#endif

