/* $Id: st_draw_shape.cpp 18283 2014-11-25 05:05:59Z ksimek $ */
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

#include <st_cpp/st_draw_shape.h>
#include <gr_cpp/gr_primitive.h>
#include <gr_cpp/gr_find_shapes.h>
#include <iostream>
//#include <gr_cpp/gr_camera.h>

using namespace kjb;
using namespace kjb::opengl;


/**
 * @param p1  The center point of one of the bases of the cylinder.
 * @param p2  The center point of the other base of the cylinder.
 * @param top  The larger center point (i.e. MAX(p1, p2) ).
 * @param bottom  The smaller center point (i.e. MIN(p1, p2) ).
 * @param magnitude  The computed height of the cylinder.
 * @param angle_y  The angle to rotate the cylinder vector (given by top - bottom)
 *                 around the y-axis to move it into the yz-plane.
 * @param angle_x  The angle to rotate the cylinder vector (given by top - bottom)
 *                 around the x-axis to move it from the yz-plane onto the z-axis.
 */
void kjb::compute_cylinder_rotation_angles(const Vector& p1, const Vector& p2, Vector& top, Vector& bottom, double& magnitude, double& angle_y, double& angle_x)
{
    if(p1 <= p2)
    {
        top = p2;
        bottom = p1;
    }
    else
    {
        top = p1;
        bottom = p2;
    }

    Vector a(3);
    for(int i = 0; i < 3; i++)
    {
        a(i) = top(i) - bottom(i);
    }
    magnitude = a.magnitude();
    if(magnitude < DBL_EPSILON) // magnitude == 0
    {
        throw KJB_error("Error: height of cylinder is 0\n");
    }
    a = a / magnitude;

    // Rotate the vector if it is not already in the yz-plane
    if(a(0) != 0)
    {
        angle_y = acos(a(2) / sqrt(a(0)*a(0) + a(2)*a(2)));
    }
    // If the vector is already in the yz-plane, rotate it by 0 degrees
    else
    {
        angle_y = 0;
    }

    if(a(0) >  0.0)
    {
        angle_y = -1.0 * angle_y;
    }

    Matrix rot_y(3,3);
    rot_y(0,0) = cos(angle_y);
    rot_y(0,1) = 0;
    rot_y(0,2) = -1.0 * sin(angle_y);
    rot_y(1,0) = 0;
    rot_y(1,1) = 1;
    rot_y(1,2) = 0;
    rot_y(2,0) = sin(angle_y);
    rot_y(2,1) = 0;
    rot_y(2,2) = cos(angle_y);

    Vector a_yz = rot_y * a;

    angle_x = acos(a_yz(2) / a_yz.magnitude());

    if(a_yz(1) > 0.0)
    {
        angle_x = -1.0 * angle_x;
    }
}

/**
 * @param myQuadric  A GLUquadricObj needed for the gluCylinder() method.
 * @param bottom  The smaller center point (i.e. MIN(p1, p2) ).
 * @param angle_y  The angle to rotate the cylinder vector (given by top - bottom)
 *                 around the y-axis to move it into the yz-plane.
 * @param angle_x  The angle to rotate the cylinder vector (given by top - bottom)
 *                 around the x-axis to move it from the yz-plane onto the z-axis.
 * @param radius  The radius of the cylinder to be drawn.
 * @param magnitude  The height of the cylinder to be drawn.
 */
void kjb::draw_translated_and_rotated_cylinder(GLUquadricObj* myQuadric, const Vector& bottom, const double angle_y, const double angle_x, const double radius, const double magnitude)
{
#ifdef KJB_HAVE_OPENGL
//    int slices = 5;     // Number of subdivisions around the z-axis.
//    int stacks = 64;    // Number of subdivisions along the z-axis.
    int slices = 16;     // Number of subdivisions around the z-axis.
    int stacks = 64;    // Number of subdivisions along the z-axis.

    // Check size of Vector.
    if(bottom.size() < 3)
    {
        std::cout << "ERROR: size of Vector is less than 3\n";
        return;
    }

    // Draw cylinder.
    glTranslatef(bottom(0), bottom(1), bottom(2));
    glRotatef(angle_y * rad_to_deg, 0, 1, 0);
    glRotatef(angle_x * rad_to_deg, 1, 0, 0);   
    gluCylinder( myQuadric, radius, radius, magnitude, slices, stacks );
    glRotatef(-1*angle_x * rad_to_deg, 1, 0, 0);
    glRotatef(-1*angle_y * rad_to_deg, 0, 1, 0);
    glTranslatef(-bottom(0), -bottom(1), -bottom(2));
#else // ! KJB_HAVE_OPENGL
    KJB_THROW_2(kjb::Missing_dependency, "opengl");
#endif // KJB_HAVE_OPENGL
}


/**
 * @param  p1  A vector representing the center point of the top of the cylinder
 * @param  p2  A vector representing the center point of the bottom of the cylinder
 * @param  radius  A double representing the radius of the cylinder to be drawn
 */
int kjb::draw_cylinder(GLUquadricObj* myQuadric, const Vector& p1, const Vector& p2, double radius)
{
#ifdef KJB_HAVE_OPENGL
    Vector top;
    Vector bottom;
    
    // The height of the cylinder.
    double magnitude;
    // The angle to rotate the cylinder about the y-axis.
    double angle_y;
    // The angle to rotate the cylinder about the x-axis.
    double angle_x;
    

    compute_cylinder_rotation_angles(p1, p2, top, bottom, magnitude, angle_y, angle_x);

    // Draw cylinder.
    draw_translated_and_rotated_cylinder(myQuadric, bottom, angle_y, angle_x, radius, magnitude);

    return 0;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}


/**
 * @param  c  A Perspective_camera
 * @param  p1  A vector representing the center point of the top of the cylinder
 * @param  p2  A vector representing the center point of the bottom of the cylinder
 * @param  radius  A double representing the radius of the cylinder to be drawn
 */
int kjb::draw_cylinder(Perspective_camera& c, GLUquadricObj* myQuadric, const Vector& p1, const Vector& p2, double radius)
{
#ifdef KJB_HAVE_OPENGL
    c.prepare_for_rendering(true);

    draw_cylinder(myQuadric, p1, p2, radius);

    return 0;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}


int kjb::draw_cylinder(GLUquadricObj* myQuadric, const Cylinder cyl)
{
#ifdef KJB_HAVE_OPENGL
    Vector p1 = cyl.get_p1();
    Vector p2 = cyl.get_p2();
    double radius = cyl.get_radius();

    draw_cylinder(myQuadric, p1, p2, radius);

    return 0;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}


int kjb::draw_cylinder(Perspective_camera& c, GLUquadricObj* myQuadric, const std::vector<Cylinder> cylinders)
{
#ifdef KJB_HAVE_OPENGL
    c.prepare_for_rendering(true);

    glColor3f(0,255,0);

    for(unsigned int i = 0; i < cylinders.size(); i++)
    {
        draw_cylinder(myQuadric, cylinders[i]);
    }
    return 0;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}


/**
 * @param  p1  A vector representing the center point of the top of the cylinder
 * @param  p2  A vector representing the center point of the bottom of the cylinder
 * @param  radius  A double representing the radius of the cylinder to be drawn
 * @param  angle
 * @param  angleStartpt
 * @param  angleEndpt
 * @param  MY_CLIP_PLANE
 * @param  MY_CLIP_PLANE1
 */
int kjb::draw_cylinder_section(GLUquadricObj* myQuadric, const Vector& p1, const Vector& p2, double radius, double angle, const Vector& angleStartpt, const Vector& angleEndpt, GLuint& MY_CLIP_PLANE, GLuint& MY_CLIP_PLANE1)
{
#ifdef KJB_HAVE_OPENGL
//    GLuint MY_CLIP_PLANE = GL_CLIP_PLANE0;
//    GLuint MY_CLIP_PLANE1 = GL_CLIP_PLANE1;

    Vector top;
    Vector bottom;

    // The height of the cylinder.
    double magnitude;
    // The angle to rotate the cylinder about the y-axis.
    double angle_y;
    // The angle to rotate the cylinder about the x-axis.
    double angle_x;

    compute_cylinder_rotation_angles(p1, p2, top, bottom, magnitude, angle_y, angle_x);

    glColor3f(0,255,0);
        
    int numEq;
    if(fabs(2*M_PI - fabs(angle)) < 0.00001)
    {
        numEq = 0;
    }
    else if(fabs(M_PI - fabs(angle)) < DBL_EPSILON)
    {
        numEq = 1;
    }
    else
    {
        numEq = 2;
    }
    // Setup clip planes
    std::vector< std::vector<Vector> > verts(numEq);
    Vector v1(4, 0.0);
    Vector v6(4, 0.0);
    // center bottom point
    v1(0) = p2(0);
    v1(1) = p2(1);
    v1(2) = p2(2);
        
    if(numEq >= 1)
    {
        std::vector<Vector> vec1(4);
        Vector v2(4, 0.0);
        Vector v3(4, 0.0);
        
        if(angle < 0)
        {
            // reflection of center top point (want reflection so that this cylinder section goes clockwise)
            v2(0) = (2*p2(0)) - p1(0);
            v2(1) = (2*p2(1)) - p1(1);
            v2(2) = (2*p2(2)) - p1(2);
        }
        else
        {
            // center top point
            v2(0) = p1(0);
            v2(1) = p1(1);
            v2(2) = p1(2);
        }
        // stored edge point
        v3(0) = angleStartpt(0);
        v3(1) = angleStartpt(1);
        v3(2) = angleStartpt(2);
/*
// draw purple sphere at start EdgePoint
glColor3f(64,0,64);
glTranslatef(v3(0), v3(1), v3(2));
GLUquadricObj* quadric = 0;
quadric = gluNewQuadric();
gluSphere(quadric, 0.5, 32, 32);
glTranslatef(-v3(0), -v3(1), -v3(2));
glColor3f(0,255,0);
*/
        vec1[0] = v1;
        vec1[1] = v2;
        vec1[2] = v3; 
        vec1[3] = v6;
        verts[0] = vec1;
    }
    if(numEq >= 2)
    {
        std::vector<Vector> vec2(4);
        Vector v4(4, 0.0);
        Vector v5(4, 0.0);

        if(angle < 0)
        {
            // center top point
            v4(0) = p1(0);
            v4(1) = p1(1);
            v4(2) = p1(2);
        }
        else
        {
            // reflection of center top point (want reflection so that this cylinder section goes clockwise)
            v4(0) = (2*p2(0)) - p1(0);
            v4(1) = (2*p2(1)) - p1(1);
            v4(2) = (2*p2(2)) - p1(2);
        }
        
        // other edge point
        v5(0) = angleEndpt(0);
        v5(1) = angleEndpt(1);
        v5(2) = angleEndpt(2);
/*
// draw blue sphere at otherEdgePoint
glColor3f(0,0,255);
glTranslatef(v5(0), v5(1), v5(2));
GLUquadricObj* quadric = 0;
quadric = gluNewQuadric();
gluSphere(quadric, 0.5, 32, 32);
glTranslatef(-v5(0), -v5(1), -v5(2));
glColor3f(0,255,0);
*/
        vec2[0] = v1;
        vec2[1] = v4;
        vec2[2] = v5;
        vec2[3] = v6;
        verts[1] = vec2;
    }
    
    std::vector<Vector> equation(numEq);
    for(int k = 0; k < numEq; k++)
    {
        get_plane_parameters(verts[k][0], verts[k][1], verts[k][2], equation[k]);
    }

    GLdouble eq[numEq][4];
    for(int k = 0; k < numEq; k++)
    {
        for(int j=0; j<4; j++)
        {
            eq[k][j] = equation[k](j);
        }
    }

    if(numEq > 0)
    {
        glEnable(MY_CLIP_PLANE);
        if(fabs(angle) < (M_PI - DBL_EPSILON))
        {
            glEnable(MY_CLIP_PLANE1);
        }

        for(int k = 0; k < numEq; k++)
        {
            glClipPlane(MY_CLIP_PLANE, eq[k]);
            if(fabs(angle) < (M_PI - DBL_EPSILON))
            {
                glClipPlane(MY_CLIP_PLANE1, eq[k+1]);
            }
    
            //**** Rendering the mesh's clip edge ****//
            glEnable(GL_STENCIL_TEST);
            glClear(GL_STENCIL_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            for(int pass=2; pass<3; pass++)
            {
                if(pass == 0)
                {
                    // first pass: increment stencil buffer value on back faces
                    glStencilFunc(GL_ALWAYS, 0, 0);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
                    glCullFace(GL_FRONT);  // render back faces only
                }
                else if(pass == 1)
                {
                    // second pass: decrement stencil buffer value on front faces
                    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
                    glCullFace(GL_BACK);  // render front faces only
                }
                else
                {
                    glEnable(GL_STENCIL_TEST);
                    glClear(GL_STENCIL_BUFFER_BIT);
                    glDisable(GL_DEPTH_TEST);
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

                    // drawing clip planes masked by stencil buffer content
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glEnable(GL_DEPTH_TEST);
                    glDisable(MY_CLIP_PLANE);
                    if(fabs(angle) < (M_PI - DBL_EPSILON))
                    {
                        glDisable(MY_CLIP_PLANE1);
                    }
                    glStencilFunc(GL_NOTEQUAL, 0, ~0);

                    // stencil test will pass only when stencil buffer value = 0;
                    // // (~0 = 0x11...11)
                    glBegin(GL_QUADS); // rendering the plane quad.  Note, it should be big enough to cover all clip edge area.
                    for(int j = 3; j >= 0; j--)
                    {
                        GLfloat v[3] = { (float) verts[k][j](0), (float) verts[k][j](1), (float) verts[k][j](2) };
                        glVertex3fv(&v[0]);
                        if(fabs(angle) < (M_PI - DBL_EPSILON))
                        {
                            GLfloat v1[3] = { (float) verts[k+1][j](0), (float) verts[k+1][j](1), (float) verts[k+1][j](2) };
                            glVertex3fv(&v1[0]);
                        }
                    }
                    glEnd();
                    //**** End rendering mesh's clip edge ****///
                 
                    //**** Rendering mesh ****//
                    glDisable(GL_STENCIL_TEST);
                    glEnable(MY_CLIP_PLANE);
                    if(fabs(angle) < (M_PI - DBL_EPSILON))
                    {
                        glEnable(MY_CLIP_PLANE1);
                    }
                }

                // Draw cylinder.
                draw_translated_and_rotated_cylinder(myQuadric, bottom, angle_y, angle_x, radius, magnitude);
            }

            if(fabs(angle) < (M_PI - DBL_EPSILON))
            {
                k++;
            }
        }
        glDisable(MY_CLIP_PLANE);
        if(fabs(angle) < (M_PI - DBL_EPSILON))
        {
            glDisable(MY_CLIP_PLANE1);
        }
    }
    else    // angle == 2*M_PI
    {
        // Draw cylinder.
        draw_translated_and_rotated_cylinder(myQuadric, bottom, angle_y, angle_x, radius, magnitude);
    }

    return 0;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}


int kjb::draw_cylinder_section(Perspective_camera& c, GLUquadricObj* myQuadric, const std::vector<Cylinder_section> cylinders)
{
#ifdef KJB_HAVE_OPENGL
    c.prepare_for_rendering(true);

    GLuint MY_CLIP_PLANE = GL_CLIP_PLANE0;
    GLuint MY_CLIP_PLANE1 = GL_CLIP_PLANE1;

    for(unsigned int i = 0; i < cylinders.size(); i++)
    {
        Vector p1 = cylinders[i].get_p1();
        Vector p2 = cylinders[i].get_p2();
        double radius = cylinders[i].get_radius();
        double angle = cylinders[i].get_angle();
        Vector angleStartpt = cylinders[i].get_startpoint_of_angle();
        Vector angleEndpt = cylinders[i].get_endpoint_of_angle();

        draw_cylinder_section(myQuadric, p1, p2, radius, angle, angleStartpt, angleEndpt, MY_CLIP_PLANE, MY_CLIP_PLANE1);
    }
    return 0;
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Generate the vertex and normal data for a cylinder on the +z-axis with a base centered
 * at the origin.
 *
 * @param vlist  The list of coordinates for the vertices that make up the cylinder.
 * @param nlist  The list of normal vectors for the vertices of the cylinder.
 * @param facets  The number of subdivisions around the z-axis.
 * @param ribs  The number of subdivisions along the z-axis.
 * @param radius  The radius of the cylinder.
 * @param length  The height of the cylinder.
 */
void kjb::build_cylinder(std::vector<Vector>& vlist, std::vector<Vector>& nlist, int facets, int ribs, float radius, float length)
{
    double angle;
    int facet, rib;

    // Malloc the arrays
    vlist.clear();
    vlist.reserve(facets * (ribs+1));

    nlist.clear();
    nlist.reserve(facets * (ribs+1));

    // Fill the vertex and normal arrays.
    for(rib = 0; rib <= ribs; rib++)
    {
        for(facet = 0; facet < facets; facet++)
        {
            angle = facet * (2.0 * M_PI / facets);

            Vector temp(3);
            temp(0) = cos(angle) * radius;
            temp(1) = sin(angle) * radius;
            temp(2) = rib * (length / ribs);
            vlist.push_back(temp);

            temp(0) = cos(angle);
            temp(1) = sin(angle);
            temp(2) = 0;
            nlist.push_back(temp);
        }
    }
}

/*
 * Generate the vertex and normal data for a frustum on the +z-axis with a base centered
 * at the origin.
 *
 * @param vlist  The list of coordinates for the vertices that make up the frustum.
 * @param nlist  The list of normal vectors for the vertices of the frustum.
 * @param facets  The number of subdivisions around the z-axis.
 * @param ribs  The number of subdivisions along the z-axis.
 * @param radius_bottom  The radius of the bottom surface of the frustum.
 * @param radius_top  The radius of the top surface of the frustum.
 * @param length  The height of the frustum.
 */
void kjb::build_frustum(std::vector<Vector>& vlist, std::vector<Vector>& nlist, int facets, int ribs, float radius_bottom, float radius_top, float length)
{
    double angle;
    int facet, rib;

	double delta_radius = radius_bottom-radius_top;
	double radius;
	double theta_z = atan(delta_radius/length);

    // Malloc the arrays
    vlist.clear();
    vlist.reserve(facets * (ribs+1));

    nlist.clear();
    nlist.reserve(facets * (ribs+1));

    // Fill the vertex and normal arrays.
    for(rib = 0; rib <= ribs; rib++)
    {
		radius = radius_top + delta_radius*rib/ribs;

        for(facet = 0; facet < facets; facet++)
        {
            angle = facet * (2.0 * M_PI / facets);

			// vertices of the mesh
            Vector temp(3);
            temp(0) = cos(angle) * radius;
            temp(1) = sin(angle) * radius;
            temp(2) = rib * (length / ribs);
            vlist.push_back(temp);

			// norm direction of each polygon
            temp(0) = cos(angle);
            temp(1) = sin(angle);
            temp(2) = theta_z;
            nlist.push_back(temp);
        }
    }
}

/*
 * Call the compute_cylinder_rotation_angles() method before calling this method
 * in order to get the values for angle_y and angle_x.  Call build_cylinder() to
 * get the values for vlist.
 *
 * @param vlist  The list of coordinates for the points that make up the cylinder.
 * @param bottom  The smaller center point (i.e. MIN(p1, p2) ).
 * @param angle_y  The angle to rotate the cylinder vector (given by top - bottom)
 *                 around the y-axis to move it into the yz-plane.
 * @param angle_x  The angle to rotate the cylinder vector (given by top - bottom)
 *                 around the x-axis to move it from the yz-plane onto the z-axis.
 * @param vlist_tr  The list of transformed points.
 */
void kjb::untransform_points(std::vector<Vector>& vlist, const Vector& bottom, const double angle_y, const double angle_x, std::vector<Vector>& vlist_tr)
{
    vlist_tr.clear();

    double ang_x = -1*angle_x;
    double ang_y = -1*angle_y;

    Matrix rot_x(4,4);
    rot_x(0,0) = 1;
    rot_x(0,1) = 0;
    rot_x(0,2) = 0;
    rot_x(0,3) = 0;
    rot_x(1,0) = 0;
    rot_x(1,1) = cos(ang_x);
    rot_x(1,2) = sin(ang_x);
    rot_x(1,3) = 0;
    rot_x(2,0) = 0;
    rot_x(2,1) = -1.0 * sin(ang_x);
    rot_x(2,2) = cos(ang_x);
    rot_x(2,3) = 0;
    rot_x(3,0) = 0;
    rot_x(3,1) = 0;
    rot_x(3,2) = 0;
    rot_x(3,3) = 1;

    Matrix rot_y(4,4);
    rot_y(0,0) = cos(ang_y);
    rot_y(0,1) = 0;
    rot_y(0,2) = -1.0 * sin(ang_y);
    rot_y(0,3) = 0;
    rot_y(1,0) = 0;
    rot_y(1,1) = 1;
    rot_y(1,2) = 0;
    rot_y(1,3) = 0;
    rot_y(2,0) = sin(ang_y);
    rot_y(2,1) = 0;
    rot_y(2,2) = cos(ang_y);
    rot_y(2,3) = 0;
    rot_y(3,0) = 0;
    rot_y(3,1) = 0;
    rot_y(3,2) = 0;
    rot_y(3,3) = 1;

    Matrix trans(4,4);
    trans(0,0) = 1;
    trans(0,1) = 0;
    trans(0,2) = 0;
    trans(0,3) = 0;
    trans(1,0) = 0;
    trans(1,1) = 1;
    trans(1,2) = 0;
    trans(1,3) = 0;
    trans(2,0) = 0;
    trans(2,1) = 0;
    trans(2,2) = 1;
    trans(2,3) = 0;
    trans(3,0) = -bottom(0);
    trans(3,1) = -bottom(1);
    trans(3,2) = -bottom(2);
    trans(3,3) = 1;

    Matrix rot(4,4);
    rot = rot_x * rot_y;
    rot = rot * trans;

    Vector tmp(4, 1.0);
    for(int i = 0; i < vlist.size(); i++)
    {
        tmp[0] = vlist[i][0];
        tmp[1] = vlist[i][1];
        tmp[2] = vlist[i][2];

        Matrix tmp_m = create_row_matrix(tmp);

        Matrix tmpMatrix = tmp_m * rot;
        Vector tmp_tr = tmpMatrix.get_row(0);

        if(tmp_tr(3) != 1)
        {
            tmp_tr = tmp_tr / tmp_tr(3);
        }

        vlist_tr.push_back(Vector(tmp_tr(0), tmp_tr(1), tmp_tr(2)));
    }
}

/*
 * Draw only the true edges of the cylinder (the circles at the top and
 * the bottom and the outermost edges of the body w.r.t the camera position).
 *
 * @param vlist  The list of coordinates for the points that make up the cylinder.
 * @param facets  The number of subdivisions around the z-axis.
 * @param ribs  The number of subdivisions along the z-axis.
 * @param bottom  The smaller center point (i.e. MIN(p1, p2) ).
 * @param angle_y  The angle to rotate the cylinder vector (given by top - bottom)
 *                 around the y-axis to move it into the yz-plane.
 * @param angle_x  The angle to rotate the cylinder vector (given by top - bottom)
 *                 around the x-axis to move it from the yz-plane onto the z-axis.
 * @param camera  Pointer to a Perspective_camera.
 */
void kjb::draw_cylinder_edges(std::vector<Vector>& vlist, int facets, int ribs, const Vector& bottom, const double angle_y, const double angle_x, const Perspective_camera* camera)
{
    int facet, rib;
    int i, j, next;
    int p;
#ifdef KJB_HAVE_OPENGL
    // Draws top and bottom circles of cylinder.
    glBegin(GL_LINES);
    i = 0;
    j = ribs * facets;
    for(facet = 0; facet < facets; facet++, i++, j++)
    {
        next = (facet == (facets-1)) ? 1-facets : 1;
        glVertex3f(vlist[i][0], vlist[i][1], vlist[i][2]);
        glVertex3f(vlist[i+next][0], vlist[i+next][1], vlist[i+next][2]);
        glVertex3f(vlist[j][0], vlist[j][1], vlist[j][2]);
        glVertex3f(vlist[j+next][0], vlist[j+next][1], vlist[j+next][2]);
    }
    glEnd();

    // Get the real position of the points.
    std::vector<Vector> vlist_tr;
    untransform_points(vlist, bottom, angle_y, angle_x, vlist_tr);

    // Constructs polygons from facets.
    std::vector<Polygon> polygon_vec;
    for(rib = 0; rib < ribs; rib++)
    {
        i = rib * facets;
        j = i + facets;

        for(facet = 0; facet < facets; facet++, i++, j++)
        {
            next = (facet == (facets-1)) ? 1-facets : 1;    // means if(facet == (facets-1)) then (next = 1-facets) else (next = 1)
            Polygon poly(4);
            poly.add_point(vlist_tr[i][0], vlist_tr[i][1], vlist_tr[i][2]);
            poly.add_point(vlist_tr[i+next][0], vlist_tr[i+next][1], vlist_tr[i+next][2]);
            poly.add_point(vlist_tr[j+next][0], vlist_tr[j+next][1], vlist_tr[j+next][2]);
            poly.add_point(vlist_tr[j][0], vlist_tr[j][1], vlist_tr[j][2]);
            
            polygon_vec.push_back(poly);
        }
    }

    // Checks which polygons are visible to the camera.
    std::vector<int> isVisible(ribs*facets);
    if(camera != NULL)
    {
        Parametric_camera_gl_interface camera_gl = (*camera).get_rendering_interface();
        for(p = 0; p < polygon_vec.size(); p++)
        {
            if(camera_gl.Polygon_visibility_test(polygon_vec[p], 0))
            {
                isVisible[p] = 1;
            }
            else
            {
                isVisible[p] = 0;
            }
        }
    }
    else
    {
        std::cout << "ERROR: camera not passed to draw_cylinder_edges() so can't determine which polygons are visible\n";
        return;
    }
    
    if(isVisible.size() < ribs*facets)
    {
        std::cout << "ERROR: the vector isVisible is the wrong size\n";
        return;
    }

    // For every pair of adjacent faces, if one is visible and the other is not then draw the edge between them.
    glBegin(GL_LINES);
    for(rib = 0; rib < ribs; rib++)
    {
        i = rib * facets;
        j = i + facets;
        for(facet = 0; facet < facets; facet++, i++, j++)
        {
            next = (facet == (facets-1)) ? 1-facets : 1;    // means if(facet == (facets-1)) then (next = 1-facets) else (next = 1)
            if(isVisible[i] + isVisible[i+next] == 1)
            {
                glVertex3f(vlist[i+next][0], vlist[i+next][1], vlist[i+next][2]);
                glVertex3f(vlist[j+next][0], vlist[j+next][1], vlist[j+next][2]);
            }
        }
    }
    glEnd();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}


/*
 * Draws all the facets of a cylinder.
 *
 * @param vlist  The list of coordinates for the points that make up the cylinder.
 * @param facets  The number of subdivisions around the z-axis.
 * @param ribs  The number of subdivisions along the z-axis.
 */
void kjb::draw_cylinder_facets(std::vector<Vector>& vlist, int facets, int ribs)
{
    int rib, facet;
    int i, j, next;
    float height;
#ifdef KJB_HAVE_OPENGL
    for(rib = 0; rib < ribs; rib++)
    {
        i = rib * facets;
        j = i + facets;
        // Draws the sides of the cylinder as rectangles.
        glBegin(GL_QUADS);
        for(facet = 0; facet < facets; facet++, i++, j++)
        {
            next = (facet == (facets-1)) ? 1-facets : 1;    // means if(facet == (facets-1)) then (next = 1-facets) else (next = 1)
            glVertex3f(vlist[i][0], vlist[i][1], vlist[i][2]);
            glVertex3f(vlist[i+next][0], vlist[i+next][1], vlist[i+next][2]);
            glVertex3f(vlist[j+next][0] ,vlist[j+next][1], vlist[j+next][2]);
            glVertex3f(vlist[j][0], vlist[j][1], vlist[j][2]);
        }
        glEnd();
    }

    // Draws the bases of the cylinder as triangles around the center of the circle.
    glBegin(GL_TRIANGLES);
    i = 0;
    j = ribs * facets;
    height = vlist[j][2];
    for(facet = 0; facet < facets; facet++, i++, j++)
    {
        next = (facet == (facets-1)) ? 1-facets : 1;
        glVertex3f(0,0,0);
        glVertex3f(vlist[i+next][0], vlist[i+next][1], vlist[i+next][2]);
        glVertex3f(vlist[i][0], vlist[i][1], vlist[i][2]);
        
        glVertex3f(0,0,height);
        glVertex3f(vlist[j][0], vlist[j][1], vlist[j][2]);
        glVertex3f(vlist[j+next][0], vlist[j+next][1], vlist[j+next][2]);
    }
    glEnd();
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}


/**
 * @param polymesh  Pointer to a Parapiped.
 * @param camera  Pointer to a Perspective_camera.
 */
void kjb::render_cylinder_silhouette(const Polymesh* polymesh, const Perspective_camera* camera)
{
    std::vector<Vector> vlist;
    std::vector<Vector> nlist;
    int facets = 64;
    int ribs = 1;
    Vector top;
    Vector bottom;
    double magnitude;
    double angle_y;
    double angle_x;

    // Note: face 2 is the top and face 3 is the bottom of the parapiped.
    Polygon bottom_face = polymesh->get_face(3);

    Vector center_top = (polymesh->get_face(2)).get_centroid();
    Vector center_bottom = bottom_face.get_centroid();

    // Compute radius and height of cylinder.
    Vector p0 = bottom_face.get_point(0);
    Vector p1 = bottom_face.get_point(1);
    Vector p3 = bottom_face.get_point(3);

    Vector edge1(3);
    Vector edge2(3);
    Vector heightVec(3);
    for(int j = 0; j < 3; j++)
    {
        edge1(j) = p1(j) - p0(j);
        edge2(j) = p3(j) - p0(j);

        heightVec(j) = center_top(j) - center_bottom(j);
    }

    // The radius is half the average of the length and the width.
    double face_length = edge1.magnitude();
    double face_width = edge2.magnitude();
    double cyl_radius = ((face_length + face_width) / 2) / 2;

    double cyl_height = heightVec.magnitude();
                
    // Get the points of the cylinder.
    build_cylinder(vlist, nlist, facets, ribs, cyl_radius, cyl_height);

    // Get the rotation angles.
    compute_cylinder_rotation_angles(center_top, center_bottom, top, bottom, magnitude, angle_y, angle_x);

    // Check size of Vector.
    if(bottom.size() < 3)
    {
        std::cout << "ERROR: size of Vector bottom is less than 3\n";
        return;
    }
#ifdef KJB_HAVE_OPENGL
    // Draw cylinder.
    glTranslatef(bottom(0), bottom(1), bottom(2));
    glRotatef(angle_y * rad_to_deg, 0, 1, 0);
    glRotatef(angle_x * rad_to_deg, 1, 0, 0); 

    // Draws the top and bottom circles of the cylinder and the outer visible edges of the body of the cylinder.
    draw_cylinder_edges(vlist, facets, ribs, bottom, angle_y, angle_x, camera);

    glRotatef(-1*angle_x * rad_to_deg, 1, 0, 0);
    glRotatef(-1*angle_y * rad_to_deg, 0, 1, 0);
    glTranslatef(-bottom(0), -bottom(1), -bottom(2));
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/**
 * @param polymesh  Pointer to a Parapiped.
 * @param camera  Pointer to a Perspective_camera.
 */
void kjb::render_occlude_cylinder_silhouette(const Polymesh* polymesh, const Perspective_camera* camera)
{
    std::vector<Vector> vlist;
    std::vector<Vector> nlist;
    int facets = 64;
    int ribs = 1;
    Vector top;
    Vector bottom;
    double magnitude;
    double angle_y;
    double angle_x;

    // Note: face 2 is the top and face 3 is the bottom of the parapiped.
    Polygon bottom_face = polymesh->get_face(3);

    Vector center_top = (polymesh->get_face(2)).get_centroid();
    Vector center_bottom = bottom_face.get_centroid();

    // Compute radius and height of cylinder.
    Vector p0 = bottom_face.get_point(0);
    Vector p1 = bottom_face.get_point(1);
    Vector p3 = bottom_face.get_point(3);

    Vector edge1(3);
    Vector edge2(3);
    Vector heightVec(3);
    for(int j = 0; j < 3; j++)
    {
        edge1(j) = p1(j) - p0(j);
        edge2(j) = p3(j) - p0(j);

        heightVec(j) = center_top(j) - center_bottom(j);
    }

    // The radius is half the average of the length and the width.
    double face_length = edge1.magnitude();
    double face_width = edge2.magnitude();
    double cyl_radius = ((face_length + face_width) / 2) / 2;

    double cyl_height = heightVec.magnitude();
                
    // Get the points of the cylinder.
    build_cylinder(vlist, nlist, facets, ribs, cyl_radius, cyl_height);

    // Get the rotation angles.
    compute_cylinder_rotation_angles(center_top, center_bottom, top, bottom, magnitude, angle_y, angle_x);

    // Check size of Vector.
    if(bottom.size() < 3)
    {
        std::cout << "ERROR: size of Vector 'bottom' is less than 3\n";
        return;
    }
#ifdef KJB_HAVE_OPENGL
    // Draw cylinder.
    glTranslatef(bottom(0), bottom(1), bottom(2));
    glRotatef(angle_y * rad_to_deg, 0, 1, 0);
    glRotatef(angle_x * rad_to_deg, 1, 0, 0); 

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    GLboolean color_mask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    draw_cylinder_edges(vlist, facets, ribs, bottom, angle_y, angle_x, camera);

    glStencilFunc(GL_NOTEQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glEnable(GL_DEPTH_TEST);

    draw_cylinder_facets(vlist, facets, ribs);

    glDisable(GL_STENCIL_TEST);
    glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);

    glRotatef(-1*angle_x * rad_to_deg, 1, 0, 0);
    glRotatef(-1*angle_y * rad_to_deg, 0, 1, 0);
    glTranslatef(-bottom(0), -bottom(1), -bottom(2));
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/**
 * @param polymesh  Pointer to a Frustum.
 * @param camera  Pointer to a Perspective_camera.
 */
void kjb::render_frustum_silhouette(const Polymesh* polymesh, Perspective_camera* camera)
{
    std::vector<Vector> vlist;
    std::vector<Vector> nlist;
    int facets = 64;
    int ribs = 1;
    Vector top;
    Vector bottom;
    double magnitude;
    double angle_y;
    double angle_x;

    // Note: face 0 is the top and face 1 is the bottom of the frustum
    Polygon bottom_face = polymesh->get_face(1);

    //TODO check that top is face 0
    Vector center_top = (polymesh->get_face(0)).get_centroid();
    Vector center_bottom = bottom_face.get_centroid();

    // Compute radius and height of cylinder.
    Vector p0_bottom = bottom_face.get_point(0);
	Vector p0_top = polymesh->get_face(0).get_point(0);
    
    Vector edge_bottom(3);
	Vector edge_top(3);
    Vector heightVec(3);
    for(int j = 0; j < 3; j++)
    {
        edge_bottom(j) = center_bottom(j) - p0_bottom(j);
		edge_top(j) = center_top(j) - p0_top(j);
        heightVec(j) = center_top(j) - center_bottom(j);
    }

    // The radius is half the average of the length and the width.
    double radius_bottom = edge_bottom.magnitude();
    double radius_top = edge_top.magnitude();

    double frustum_height = heightVec.magnitude();
                
    // Get the points of the frustum.
    build_frustum(vlist, nlist, facets, ribs, radius_bottom, radius_top, frustum_height);

    // Get the rotation angles.
    compute_cylinder_rotation_angles(center_top, center_bottom, top, bottom, magnitude, angle_y, angle_x);

    // Check size of Vector.
    if(bottom.size() < 3)
    {
        std::cout << "ERROR: size of Vector 'bottom' is less than 3\n";
        return;
    }
#ifdef KJB_HAVE_OPENGL
    // Draw cylinder.
    glTranslatef(bottom(0), bottom(1), bottom(2));
    glRotatef(angle_y * rad_to_deg, 0, 1, 0);
    glRotatef(angle_x * rad_to_deg, 1, 0, 0); 

    // Draws the top and bottom circles of the cylinder and the outer visible edges of the body of the cylinder.
    draw_cylinder_edges(vlist, facets, ribs, bottom, angle_y, angle_x, camera);

    glRotatef(-1*angle_x * rad_to_deg, 1, 0, 0);
    glRotatef(-1*angle_y * rad_to_deg, 0, 1, 0);
    glTranslatef(-bottom(0), -bottom(1), -bottom(2));
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}

/**
 * @param polymesh  Pointer to a Frustum.
 * @param camera  Pointer to a Perspective_camera.
 */
void kjb::render_occlude_frustum_silhouette(const Polymesh* polymesh, Perspective_camera* camera)
{
    std::vector<Vector> vlist;
    std::vector<Vector> nlist;
    int facets = 64;
    int ribs = 1;
    Vector top;
    Vector bottom;
    double magnitude;
    double angle_y;
    double angle_x;

    // Note: face 0 is the top and face 1 is the bottom of the frustum
    Polygon bottom_face = polymesh->get_face(1);

    Vector center_top = (polymesh->get_face(0)).get_centroid();
    Vector center_bottom = bottom_face.get_centroid();

    // Compute radius and height of cylinder.
    Vector p0_bottom = bottom_face.get_point(0);
	Vector p0_top = (polymesh->get_face(0)).get_point(0);
    
    Vector edge_bottom(3);
	Vector edge_top(3);
    Vector heightVec(3);
    for(int j = 0; j < 3; j++)
    {
        edge_bottom(j) = center_bottom(j) - p0_bottom(j);
		edge_top(j) = center_top(j) - p0_top(j);
        heightVec(j) = center_top(j) - center_bottom(j);
    }

    // The radius is half the average of the length and the width.
    double radius_bottom = edge_bottom.magnitude();
    double radius_top = edge_top.magnitude();

    double frustum_height = heightVec.magnitude();
                
    // Get the points of the frustum.
    build_frustum(vlist, nlist, facets, ribs, radius_bottom, radius_top, frustum_height);

    // Get the rotation angles.
    compute_cylinder_rotation_angles(center_top, center_bottom, top, bottom, magnitude, angle_y, angle_x);

    // Check size of Vector.
    if(bottom.size() < 3)
    {
        std::cout << "ERROR: size of Vector 'bottom' is less than 3\n";
        return;
    }
#ifdef KJB_HAVE_OPENGL
    // Draw cylinder.
    glTranslatef(bottom(0), bottom(1), bottom(2));
    glRotatef(angle_y * rad_to_deg, 0, 1, 0);
    glRotatef(angle_x * rad_to_deg, 1, 0, 0); 

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    GLboolean color_mask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    draw_cylinder_edges(vlist, facets, ribs, bottom, angle_y, angle_x, camera);

    glStencilFunc(GL_NOTEQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glEnable(GL_DEPTH_TEST);

    draw_cylinder_facets(vlist, facets, ribs);

    glDisable(GL_STENCIL_TEST);
    glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);

    glRotatef(-1*angle_x * rad_to_deg, 1, 0, 0);
    glRotatef(-1*angle_y * rad_to_deg, 0, 1, 0);
    glTranslatef(-bottom(0), -bottom(1), -bottom(2));
#else
    KJB_THROW_2(Missing_dependency, "opengl");
#endif /* KJB_HAVE_OPENGL */
}
