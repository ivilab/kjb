/* $Id: st_draw_shape.h 18278 2014-11-25 01:42:10Z ksimek $ */
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
|     Emily Hartley
|
* =========================================================================== */

#ifndef KJB_DRAW_SHAPE_H
#define KJB_DRAW_SHAPE_H

#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <gr_cpp/gr_camera.h>
#include <gr_cpp/gr_polymesh.h>
#include <st_cpp/st_perspective_camera.h>
#include <g_cpp/g_cylinder.h>
#include <g_cpp/g_cylinder_section.h>

#define rad_to_deg 57.2957795
//      rad_to_deg = 180.0 / M_PI;

namespace kjb {

/**
 * @brief Uses opengl to draw a cylinder with the given points as the centers 
 * of the top and bottom of the cylinder and with the given radius.
 */
int draw_cylinder(GLUquadricObj* myQuadric, const Vector& p1, const Vector& p2, double r);

int draw_cylinder(Perspective_camera& c, GLUquadricObj* myQuadric, const Vector& p1, const Vector& p2, double r);

int draw_cylinder(GLUquadricObj* myQuadric, const Cylinder cyl);

int draw_cylinder(Perspective_camera& c, GLUquadricObj* myQuadric, const std::vector<Cylinder> cylinders);

/**
 * @brief Uses opengl to draw a cylinder section.
 */
int draw_cylinder_section(GLUquadricObj* myQuadric, const Vector& p1, const Vector& p2, double radius, double angle, const Vector& angle_startpt, const Vector& angle_endpt, GLuint& MY_CLIP_PLANE, GLuint& MY_CLIP_PLANE1);

int draw_cylinder_section(Perspective_camera& c, GLUquadricObj* myQuadric, const std::vector<Cylinder_section> cylinders);


// Perspective_camera_gl_interface


void compute_cylinder_rotation_angles(const Vector& p1, const Vector& p2, Vector& top, Vector& bottom, double& magnitude, double& angle_y, double& angle_x);

void draw_translated_and_rotated_cylinder(GLUquadricObj* myQuadric, const Vector& bottom, const double angle_y, const double angle_x, const double radius, const double magnitude);

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Creates a list of vertices for a cylinder with the given height and radius on the z-axis 
 * with the bottom face centered at the origin. Also creates a corresponding list containing
 * the normal vector for each vertex.
 */
void build_cylinder(std::vector<Vector>& vlist, std::vector<Vector>& nlist, int facets, int ribs, float radius, float length);

/**
 * @brief Creates a list of vertices for a frustum with the given height and radius of bottom and top faces on the z-axis 
 * with the bottom face centered at the origin. Also creates a corresponding list containing
 * the normal vector for each vertex.
 */
void build_frustum(std::vector<Vector>& vlist, std::vector<Vector>& nlist, int facets, int ribs, float radius_bottom, float radius_top, float length);

/*
 * @brief Transforms the vertices of a cylinder on the +z-axis and with a base centered at the 
 * origin back to their real-world position.
 */
void untransform_points(std::vector<Vector>& vlist, const Vector& bottom, const double angle_y, const double angle_x, std::vector<Vector>& vlist_tr);

/*
 * @brief Draws the top and bottom circles and the outermost edges of the body (with respect to
 * the camera position) of the cylinder.
 */
void draw_cylinder_edges(std::vector<Vector>& vlist, int facets, int ribs, const Vector& bottom, const double angle_y, const double angle_x, const Perspective_camera* camera);

/*
 * @brief Draws the edges for all of the facets of the cylinder.
 */
void draw_cylinder_facets(std::vector<Vector>& vlist, int facets, int ribs);

void render_cylinder_silhouette(const Polymesh* polymesh, const Perspective_camera* camera);

void render_occlude_cylinder_silhouette(const Polymesh* polymesh, const Perspective_camera* camera);

void render_frustum_silhouette(const Polymesh* polymesh, Perspective_camera* camera);

void render_occlude_frustum_silhouette(const Polymesh* polymesh, Perspective_camera* camera);

}

#endif
