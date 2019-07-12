/* $Id: gr_polymesh_renderer.h 18278 2014-11-25 01:42:10Z ksimek $ */

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

#ifndef POLYMESH_RENDERER_H_INCLUDED
#define POLYMESH_RENDERER_H_INCLUDED

#include <vector>
//#include "gr_cpp/gr_polymesh.h"
#include "gr_cpp/gr_parapiped.h"
#include "gr_cpp/gr_camera.h"

namespace kjb {

class Parapiped;
class GL_Polymesh_Renderer
{
public:

     /** @brief Renders this polymesh as a wireframe using OpenGL */
    static void wire_render(const Polymesh &);
    /** @brief Renders this polymesh into the depth buffer using OpenGL */
    static void wire_occlude_render(const Polymesh &);

    static void solid_occlude_render(const Polymesh &p);

    /** @brief Renders this polymesh as a solid using OpenGL */
    static void solid_render(const Polymesh &);
    /** @brief Projects this polymesh onto the image plane using the current OpenGL transformation */
    static void project(Polymesh &);

    /** @brief Render each edge of this polymesh with a different color.
     * The first edge will be rendered using the input id as a color,
     * the following edges will be rendered with an id sequentially
     * increasing. Returns the id (color) used to render the last
     * edge of the polygon
     */
    static unsigned int wire_render_with_sequential_ids(const Polymesh &, unsigned int start_id = 1);

    /** @brief Render each edge of this polymesh with a different color.
     * The first edge will be rendered using the input id as a color,
     * the following edges will be rendered with an id sequentially
     * increasing. Returns the id (color) used to render the last
     * edge of the polygon
     */
    static unsigned int wire_render_with_sequential_ids_16bits
    (
        const Polymesh &,
        unsigned int start_id = 1
    );

    /** @brief Render each polygon of this polymesh with a different color.
     * The first polygon will be rendered using the input id as a color,
     * the following edges will be rendered with an id sequentially
     * increasing. Returns the id (color) used to render the last
     * polygon
     */
    static unsigned int solid_render_with_sequential_ids(const Polymesh &, unsigned int start_id = 1);

    static void solid_render_with_bases(const Polymesh &p, unsigned int base1 = 0, unsigned int base2 = 0);

    static void solid_render_orientations_with_bases(const Polymesh &p, unsigned int base1);

    /** @brief Renders the silhouette (contour) of this polygonal mesh. This method
     *  works only for convex meshes, and gives reasonable results for concave ones.
     */
    static void silhouette_render(const kjb::Base_gl_interface & camera, const Polymesh & p, double iwidth = 1.0);

    /** @brief Draws the orientation map given a parapiped. This function must be moved somewhere else */
    static void draw_orientation_map(const kjb::Parapiped & p);

    /** @brief Draws the left-right orientation map given a parapiped.
     *  This function assumes that the parapiped is
     * lying on a plane parallel to the x-z plane
     * This function must be moved somewhere else */
    static void draw_left_right_orientation_map(const kjb::Parapiped & p);

    static void draw_CMU_orientation_map(const kjb::Parapiped & p);

    static void draw_geometric_context_map(const kjb::Parapiped & p);
};

}

#endif

