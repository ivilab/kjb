/* $Id: gr_polymesh_renderer.cpp 21596 2017-07-30 23:33:36Z kobus $ */

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

#include "gr_cpp/gr_polymesh_renderer.h"
#include "gr_cpp/gr_polygon_renderer.h"
#include "gr_cpp/gr_camera.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "l/l_bits.h"

using namespace kjb;

/** Does not set color or material effects. */
void GL_Polymesh_Renderer::wire_render(const Polymesh &p)
{
    for (size_t i = 0; i < p.num_faces(); i++)
    {
        GL_Polygon_Renderer::wire_render(p._faces[ i ]);
    }
}


/**
 * Does not render into the color buffer, only the depth buffer.
 *
 * After calling this method, call wire_render() to render the parapiped with
 * hidden lines occluded.
 */
void GL_Polymesh_Renderer::wire_occlude_render(const Polymesh &p)
{
    for (size_t i = 0; i < p.num_faces(); i++)
    {
        GL_Polygon_Renderer::wire_occlude_render(p._faces[ i ]);
    }
}

/**
 * Does not render into the color buffer, only the depth buffer.
 *
 * After calling this method, call wire_render() to render the parapiped with
 * hidden lines occluded.
 */
void GL_Polymesh_Renderer::solid_occlude_render(const Polymesh &p)
{
    for (size_t i = 0; i < p.num_faces(); i++)
    {
        GL_Polygon_Renderer::solid_occlude_render(p._faces[ i ]);
    }
}



/** Does not set color or material effects. */
void GL_Polymesh_Renderer::solid_render(const Polymesh &p)
{
    for (size_t i = 0; i < p.num_faces(); i++)
    {
        GL_Polygon_Renderer::solid_render(p._faces[ i ]);
    }
}

/** Render each edge of this polygon with a different color.
 * The first edge will be rendered using the input id as a color,
 * the following edges will be rendered with an id sequentially
 * increasing
 *
 * @param p The polymesh to render
 * @param start_id The color for the first edge of the polymesh
 * @return The last id used
*/
unsigned int GL_Polymesh_Renderer::wire_render_with_sequential_ids(const Polymesh &p, unsigned int start_id)
{
    if(p.num_faces() == 0)
    {
        return start_id;
    }

    for (unsigned int i = 0; i < p.num_faces(); i++)
    {
        start_id = GL_Polygon_Renderer::wire_render_with_sequential_ids(p._faces[ i ], start_id);
        start_id++;
    }

    return (start_id - 1);
}

/** Render each edge of this polygon with a different color.
 * The first edge will be rendered using the input id as a color,
 * the following edges will be rendered with an id sequentially
 * increasing.
 * This works if and only if the polymesh has 8 faces or less!!!!
 *
 * @param p The polymesh to render
 * @param start_id The color for the first edge of the polymesh
 * @return The last id used
*/
unsigned int GL_Polymesh_Renderer::wire_render_with_sequential_ids_16bits
(
    const Polymesh &p,
    unsigned int start_id
)
{
    using namespace kjb_c;
    if(p.num_faces() == 0)
    {
        return start_id;
    }

    unsigned int full_base2 = 0;
    for (unsigned int i = 0; i < p.num_faces(); i++)
    {
        start_id = GL_Polygon_Renderer::wire_render_with_sequential_ids_16bits(p._faces[ i ], start_id);
        start_id++;
    }

    return (start_id - 1);

}

/** Render each edge of this polygon with a different color.
 * The first edge will be rendered using the input id as a color,
 * the following edges will be rendered with an id sequentially
 * increasing.
 * This works if and only if the polymesh has 8 faces or less!!!!
 *
 * @param p The polymesh to render
 * @param start_id The color for the first edge of the polymesh
 * @return The last id used
*/
void GL_Polymesh_Renderer::solid_render_orientations_with_bases
(
    const Polymesh &p,
    unsigned int base1
)
{
    //I assume base 1 to be left unchanged
    using namespace kjb_c;
    if(p.num_faces() == 0)
    {
        return;
    }

    /*enum Geometric_context_label {
          CENTRAL  = 0, // Green 0
          LEFT, //Yellow 1
          RIGHT, //Blue 2
          FLOOR, //Red 3
          CEILING, // Light blue 4
          OBJECT_1,
          OBJECT_2
      };*/

    unsigned int colour = 0;
#ifdef KJB_HAVE_OPENGL
    double threshold = M_PI/4.0; //45 degrees
    //Yellow is left wall,  green is back wall, blue is right wall
    double flipped_norm_z = -(p._faces[0].get_normal())(2);
    double flipped_norm_x = -(p._faces[0].get_normal())(0);
    double angle = acos(flipped_norm_z);
    //  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        //Central -> green
        colour = 0;
    }
    else if(flipped_norm_x > 0.0)
    {
        //Left -> yellow
        colour = 1;
    }
    else
    {
        //Right -> blue
        colour = 2;
    }
    colour = colour<<3;
    //Index is zero, we do not need to do an or with zero
    GL_Polygon_Renderer::solid_render_with_bases(p._faces[ 0 ], base1, colour);

    flipped_norm_z = -(p._faces[1].get_normal())(2);
    flipped_norm_x = -(p._faces[1].get_normal())(0);
    angle = acos(flipped_norm_z);
    //  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        //Central -> green
        colour = 0;
    }
    else if(flipped_norm_x > 0.0)
    {
        //Left -> yellow
        colour = 1;
    }
    else
    {
        //Right -> blue
        colour = 2;
    }
    colour = (colour<<3) | 1;
    GL_Polygon_Renderer::solid_render_with_bases(p._faces[ 1 ], base1, colour);


    //Ceiling is light blue
    colour = 4;
    colour = (colour<<3) | 2;
    GL_Polygon_Renderer::solid_render_with_bases(p._faces[ 2 ], base1, colour);

    //Floor is red
    colour = 3;
    colour = (colour<<3) | 3;
    GL_Polygon_Renderer::solid_render_with_bases(p._faces[ 3 ], base1, colour);

    flipped_norm_z = -(p._faces[4].get_normal())(2);
    flipped_norm_x = -(p._faces[4].get_normal())(0);
    angle = acos(flipped_norm_z);
    //  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        //Central -> green
        colour = 0;
    }
    else if(flipped_norm_x > 0.0)
    {
        //Left -> yellow
        colour = 1;
    }
    else
    {
        //Right -> blue
        colour = 2;
    }
    colour = (colour<<3) | 4;
    GL_Polygon_Renderer::solid_render_with_bases(p._faces[ 4 ], base1, colour);

    flipped_norm_z = -(p._faces[5].get_normal())(2);
    flipped_norm_x = -(p._faces[5].get_normal())(0);
    angle = acos(flipped_norm_z);
    //  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        //Central -> green
        colour = 0;
    }
    else if(flipped_norm_x > 0.0)
    {
        //Left -> yellow
        colour = 1;
    }
    else
    {
        //Right -> blue
        colour = 2;
    }
    colour = (colour<<3) | 5;
    GL_Polygon_Renderer::solid_render_with_bases(p._faces[ 5 ], base1, colour);

#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif

}

/** Render each edge of this polygon with a different color.
 * The first edge will be rendered using the input id as a color,
 * the following edges will be rendered with an id sequentially
 * increasing.
 * This works if and only if the polymesh has 8 faces or less!!!!
 *
 * @param p The polymesh to render
 * @param start_id The color for the first edge of the polymesh
 * @return The last id used
*/
void GL_Polymesh_Renderer::solid_render_with_bases
(
    const Polymesh &p,
    unsigned int base1,
    unsigned int base2
)
{
    using namespace kjb_c;
    //I assume base 1 to be left unchanged
    // We will change base2
    base2 = (base2<<3);

#ifdef KJB_HAVE_OPENGL
    unsigned int full_base2 = 0;
    for(unsigned int i = 0; i < p.num_faces(); i++)
    {
        full_base2 = (base2) | i;
        GL_Polygon_Renderer::solid_render_with_bases(p.get_face(i), base1, full_base2);
    }
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

unsigned int GL_Polymesh_Renderer::solid_render_with_sequential_ids(const Polymesh & p, unsigned int start_id)
{
#ifdef KJB_HAVE_OPENGL
    for(unsigned int i = 0; i < p.num_faces(); i++)
    {
        unsigned int pixel_color = start_id;
        glColor4ub(pixel_color>>24&0xFF, pixel_color>>16&0xFF, pixel_color>>8&0xFF, pixel_color&0xFF);
        p.get_face(i).solid_render();
        start_id ++;
    }
    return (start_id - 1);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/**
 * The polymesh faces are transformed and projected onto the view plane by the
 * current modelview and projection matrix on the GL stack.
 *
 * The origin of the view plane coordinate system is in the lower left corner
 * and its extents are defined by the current GL viewport size.
 */
void GL_Polymesh_Renderer::project(Polymesh  & p)
{
    for (size_t i = 0; i < p.num_faces(); i++)
    {
        GL_Polygon_Renderer::project(p._faces[ i ]);
    }
}

/**
 * Draws an orientation map given a parapiped. This function
 * should be moved somewhere else
 */
void GL_Polymesh_Renderer::draw_orientation_map(const Parapiped &p)
{
#ifdef KJB_HAVE_OPENGL
    glColor3f(0.0, 255.0, 0.0);
    GL_Polygon_Renderer::solid_render(p._faces[2]);
    GL_Polygon_Renderer::solid_render(p._faces[3]);
    glColor3f(255.0, 0.0, 0.0);
    GL_Polygon_Renderer::solid_render(p._faces[0]);
    GL_Polygon_Renderer::solid_render(p._faces[1]);
    glColor3f(0.0, 0.0, 255.0);
    GL_Polygon_Renderer::solid_render(p._faces[4]);
    GL_Polygon_Renderer::solid_render(p._faces[5]);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

/**
 * Draws the orientation map given a parapiped. This function should
 * be moved somewhere else. This function assumes that the parapiped is
 * lying on a plane parallel to the x-z plane
 */
void GL_Polymesh_Renderer::draw_left_right_orientation_map(const kjb::Parapiped & p)
{
#ifdef KJB_HAVE_OPENGL
    glColor3f(0.0, 255.0, 0.0);
    //Ceiling is green
    GL_Polygon_Renderer::solid_render(p._faces[2]);
    //Floor is white
    glColor3f(255.0, 255.0, 255.0);
    GL_Polygon_Renderer::solid_render(p._faces[3]);

    double threshold = M_PI/4.0; //45 degrees

    //Red is left wall,  yellow is back wall, blue is right wall
    double flipped_norm_z = -(p._faces[0].get_normal())(2);
    double flipped_norm_x = -(p._faces[0].get_normal())(0);
    double angle = acos(flipped_norm_z);
//  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        glColor3f(255.0, 255.0, 0.0);
    }
    else if(flipped_norm_x > 0.0)
    {
        glColor3f(255.0, 0.0, 0.0);
    }
    else
    {
        glColor3f(0.0, 0.0, 255.0);
    }
    GL_Polygon_Renderer::solid_render(p._faces[0]);

    flipped_norm_z = -(p._faces[1].get_normal())(2);
    flipped_norm_x = -(p._faces[1].get_normal())(0);
    angle = acos(flipped_norm_z);

    if(fabs(angle) < (threshold))
    {
        glColor3f(255.0, 255.0, 0.0);
    }
    else if(flipped_norm_x > 0.0)
    {
        glColor3f(255.0, 0.0, 0.0);
    }
    else
    {
        glColor3f(0.0, 0.0, 255.0);
    }
    GL_Polygon_Renderer::solid_render(p._faces[1]);

    flipped_norm_z = -(p._faces[4].get_normal())(2);
    flipped_norm_x = -(p._faces[4].get_normal())(0);
    angle = acos(flipped_norm_z);

    if(fabs(angle) < (threshold))
    {
        glColor3f(255.0, 255.0, 0.0);
    }
    else if(flipped_norm_x > 0.0)
    {
        glColor3f(255.0, 0.0, 0.0);
    }
    else
    {
        glColor3f(0.0, 0.0, 255.0);
    }
    GL_Polygon_Renderer::solid_render(p._faces[4]);

    flipped_norm_z = -(p._faces[5].get_normal())(2);
    flipped_norm_x = -(p._faces[5].get_normal())(0);
    angle = acos(flipped_norm_z);

    if(fabs(angle) < (threshold))
    {
        glColor3f(255.0, 255.0, 0.0);
    }
    else if(flipped_norm_x > 0.0)
    {
        glColor3f(255.0, 0.0, 0.0);
    }
    else
    {
        glColor3f(0.0, 0.0, 255.0);
    }
    GL_Polygon_Renderer::solid_render(p._faces[5]);
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}


/**
 * This method renders the silhouette (contour) of this polygonal mesh. This method
 * works only for convex meshes, and gives reasonable results for concave ones.
 * A simple algorithm is used: first, all polygons are rendered in the z buffer,
 * then we render the contour of all polygon that are not visible.
 *
 * @param mesh The mesh to be rendered
 * @param iwidth The line width used to render the silhouette.
 */
void GL_Polymesh_Renderer::silhouette_render
(
    const kjb::Base_gl_interface &   camera,
    const Polymesh & mesh,
    double iwidth
)
{
#ifdef KJB_HAVE_OPENGL
    using namespace kjb_c;
    using kjb::Polygon;

    GL_Polymesh_Renderer::solid_occlude_render(mesh);
    const std::vector<kjb::Polygon>&  ifaces = mesh.get_faces();
    GLdouble lw;
    glGetDoublev(GL_LINE_WIDTH, &lw);

    glLineWidth(iwidth);

    for (unsigned int i = 0; i < ifaces.size(); i++)
    {
        const Polygon & face_i = mesh.get_face(i);

        bool visible = camera.Polygon_visibility_test(face_i);
        if(!visible)
        {
            glColor3f(1.0, 0.0, 0.0);
            glLineWidth(3.0);
            GL_Polygon_Renderer::wire_render(face_i);
            glLineWidth(1.0);
        }

    }

    /** We reset the original line width */
    glLineWidth(lw);

#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void GL_Polymesh_Renderer::draw_CMU_orientation_map(const kjb::Parapiped & p)
{
#ifdef KJB_HAVE_OPENGL
    //Ceiling and floor are red
    glColor3f(255.0, 0.0, 0.0);
    GL_Polygon_Renderer::solid_render(p._faces[2]);
    GL_Polygon_Renderer::solid_render(p._faces[3]);

    //One direction is green
    glColor3f(0.0, 255.0, 0.0);
    GL_Polygon_Renderer::solid_render(p._faces[0]);
    GL_Polygon_Renderer::solid_render(p._faces[1]);


    //And the other is blue
    glColor3f(0.0, 0.0, 255.0);
    GL_Polygon_Renderer::solid_render(p._faces[4]);
    GL_Polygon_Renderer::solid_render(p._faces[5]);

#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

void GL_Polymesh_Renderer::draw_geometric_context_map(const kjb::Parapiped & p)
{
#ifdef KJB_HAVE_OPENGL
    glColor3f(0.0, 255.0, 255.0);
    //Ceiling is light blue
    GL_Polygon_Renderer::solid_render(p._faces[2]);
    //Floor is red
    glColor3f(255.0, 0.0, 0.0);
    GL_Polygon_Renderer::solid_render(p._faces[3]);

    double threshold = M_PI/4.0; //45 degrees

    //Yellow is left wall,  green is back wall, blue is right wall
    double flipped_norm_z = -(p._faces[0].get_normal())(2);
    double flipped_norm_x = -(p._faces[0].get_normal())(0);
    double angle = acos(flipped_norm_z);
    //  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        glColor3f(0.0, 255.0, 0.0);
    }
    else if(flipped_norm_x > 0.0)
    {
        glColor3f(255.0, 255.0, 0.0);
    }
    else
    {
        glColor3f(0.0, 0.0, 255.0);
    }
    GL_Polygon_Renderer::solid_render(p._faces[0]);

    flipped_norm_z = -(p._faces[1].get_normal())(2);
    flipped_norm_x = -(p._faces[1].get_normal())(0);
    angle = acos(flipped_norm_z);
    //  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        glColor3f(0.0, 255.0, 0.0);
    }
    else if(flipped_norm_x > 0.0)
    {
        glColor3f(255.0, 255.0, 0.0);
    }
    else
    {
        glColor3f(0.0, 0.0, 255.0);
    }
    GL_Polygon_Renderer::solid_render(p._faces[1]);

    flipped_norm_z = -(p._faces[4].get_normal())(2);
    flipped_norm_x = -(p._faces[4].get_normal())(0);
    angle = acos(flipped_norm_z);
    //  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        glColor3f(0.0, 255.0, 0.0);
    }
    else if(flipped_norm_x > 0.0)
    {
        glColor3f(255.0, 255.0, 0.0);
    }
    else
    {
        glColor3f(0.0, 0.0, 255.0);
    }
    GL_Polygon_Renderer::solid_render(p._faces[4]);

    flipped_norm_z = -(p._faces[5].get_normal())(2);
    flipped_norm_x = -(p._faces[5].get_normal())(0);
    angle = acos(flipped_norm_z);
    //  double degrees45 = M_PI/4.0;

    if(fabs(angle) < (threshold))
    {
        glColor3f(0.0, 255.0, 0.0);
    }
    else if(flipped_norm_x > 0.0)
    {
        glColor3f(255.0, 255.0, 0.0);
    }
    else
    {
        glColor3f(0.0, 0.0, 255.0);
    }
    GL_Polygon_Renderer::solid_render(p._faces[5]);

#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif
}

