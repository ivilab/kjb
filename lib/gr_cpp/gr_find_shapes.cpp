/* $Id$ */
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
| Author: Emily Hartley
|
* =========================================================================== */

#include <g_cpp/g_circle.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <gr_cpp/gr_find_shapes.h>
#include <gr_cpp/gr_polygon_renderer.h>
#include <gr_cpp/gr_triangular_mesh.h>
#include <gr_cpp/gr_right_triangle_pair.h>
#include <g_cpp/g_cylinder.h>
#include <g_cpp/g_cylinder_section.h>
#include <m_cpp/m_matrix.h>
#include <n/n_svd.h>
//#include <l/l_sys_mal.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <math.h>
#include <cmath>

using namespace kjb;


/*
 * Finds the coefficients of the plane of the form ax + by + cz + d = 0
 * that is orthogonal to the given normal vector and contains the given
 * point. The coefficients are stored in the plane_params vector.
 *
 * @params  normal         normal vector of the plane
 * @params  point_on_plane Vector representing a point on the plane.
 * @params  plane_params   Vector to store the plane coefficients in.
 *
 * @throw  KJB_error  The normal vector has fewer than 3 values.
 * @throw  KJB_error  The point provided has fewer than 3 coordinates.
 */
void kjb::get_plane_parameters(const Vector& normal, 
                               const Vector& point_on_plane, 
                               Vector& plane_params)
{
    // Check that the normal vector and the point have at least 3 values.
    // If there is a homogeneous coordinate, it is ignored.
    if(normal.get_length() < 3)
    {
        throw KJB_error("The normal vector has fewer than 3 values");
    }
    else if(point_on_plane.get_length() < 3)
    {
        throw KJB_error("The point provided has fewer than 3 coordinates");
    }
    else
    {
        if(plane_params.get_length() != 4)
        {
            plane_params.resize(4);
        }

        // Calculate the magnitude of the normal vector to normalize the plane 
        // parameters.
        double magnitude = sqrt(normal(0)*normal(0) + 
                                normal(1)*normal(1) + 
                                normal(2)*normal(2));

        plane_params(0) = normal(0)/magnitude;  // a
        plane_params(1) = normal(1)/magnitude;  // b
        plane_params(2) = normal(2)/magnitude;  // c
        plane_params(3) = (0 - normal(0)*point_on_plane(0) 
                             - normal(1)*point_on_plane(1) 
                             - normal(2)*point_on_plane(2))/magnitude;  //d

        // Change '-0' to '0'.
        for(int i = 0; i < 4; i++)
        {
            if(plane_params(i) == -0)
            {
                plane_params(i) = 0;
            }
        }
    }
}


/*
 * Finds the coefficients of the plane of the form ax + by + cz + d = 0
 * that contains the three given points. The coefficients are stored in 
 * the plane_params vector.
 *
 * @params  pt1           Vector representing a point on the plane.
 * @params  pt2           Vector representing a point on the plane.
 * @params  pt3           Vector representing a point on the plane.
 * @params  plane_params  Vector to store the plane coefficients in.
 *
 * @throw  KJB_error  The points provided have fewer than 3 coordinates.
 */
void kjb::get_plane_parameters(const Vector& pt1, 
                               const Vector& pt2, 
                               const Vector& pt3, 
                               Vector& plane_params)
{
    // Check that the points have at least 3 values.
    // If there is a homogeneous coordinate, it is ignored.
    if(pt1.get_length() < 3 || pt2.get_length() < 3 || pt3.get_length() < 3)
    {
        throw KJB_error("One of the points has fewer than 3 coordinates");
    }
    else
    {
        // Compute a normal vector.
        Vector normal(3);
        Vector vec1(3);
        Vector vec2(3);
        for(int i = 0; i < 3; i++)
        {
            vec1(i) = pt2(i) - pt1(i);
            vec2(i) = pt3(i) - pt1(i);
        }
        // Cross product of vec1 and vec2 gives the normal.
        normal(0) = vec1(1)*vec2(2) - vec1(2)*vec2(1);
        normal(1) = vec1(2)*vec2(0) - vec1(0)*vec2(2);
        normal(2) = vec1(0)*vec2(1) - vec1(1)*vec2(0);

        // If the cross product is 0, then the 2 vectors are parallel - which 
        // means that the 3 points are collinear and can't be used to define a 
        // plane since a line can be in an infinite number of planes.
        // In this case, the plane parameters returned will be 'nan' or '-nan'.

        get_plane_parameters(normal, pt1, plane_params);
    }
}



/*
 * @param  plane1_params  the Vector of plane parameters for face 1.
 * @param  plane2_params  the Vector of plane parameters for face 2.
 * @param  tolerance      a double representing a small angle given in radians,
 *                        indicating how close to 0 the angle between two faces
 *                        has to be for them to be considered coplanar.
 * @param  distTolerance  a double representing the allowable distance between 
 *                        two faces for them to be considered coplanar.
 *
 * @return  true if the two faces are coplanar, false otherwise
 */
bool kjb::check_if_faces_are_coplanar(const Vector& plane1_params, 
                                      const Vector& plane2_params, 
                                      double tolerance, 
                                      double distTolerance)
{
    // Note: when finding rectangles, we already know that the two triangles 
    //       are adjacent, so distance doesn't matter.

    // Calculate the angle between the two planes.
    double angle = get_angle_between_two_vectors(plane1_params, plane2_params);

    // Check if the angle is close to 0 or to pi.
    // The angle is in radians and will be between 0 and 3.14.
    if(angle > tolerance && angle < (M_PI - tolerance))
    {
        return false;
    }
    // Check that the distance from the origin is the same.
    else if(fabs(fabs(plane1_params(3))-fabs(plane2_params(3))) > distTolerance)
    {
        return false;
    }
    else
    {
        // This is to rule out faces that are parallel and equal distances from
        // the origin but on opposite sides of the origin: Check if distance 
        // from origin is 0, in which case the signs don't matter.
        if(fabs(plane1_params(3)) < DBL_EPSILON && 
           fabs(plane2_params(3)) < DBL_EPSILON)
        {
            return true;
        }
        // Otherwise, make sure that all of the signs are either the same or 
        // opposite.
        else
        {
            int same = 0;
            int opposite = 0;
            int zero = 0;
            for(int i = 0; i < 4; i++)
            {
                if(plane1_params(i) == 0 && plane2_params(i) == 0)
                {
                    zero++;
                }
                else if((plane1_params(i) * plane2_params(i)) >= 0)
                {
                    same++;
                }
                else
                {
                    opposite++;
                }
            }

            if(same == 0 || opposite == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}


/*
 * @param  p1  the Vector representing point A.
 * @param  p2  the Vector representing point B.
 * @param  p3  the Vector representing point C.
 * @param  p4  the Vector representing point D.
 * @param  tolerance  a double representing a small value indicating how 
 *                    close to 0 the volume of the parallelepiped formed by the
 *                    vectors AB, AC, and AD have to be for them to be 
 *                    considered coplanar.
 *
 * @throw  KJB_error  One of the provided points has fewer than 3 coordinates
 *
 * @return  true if the 4 points are coplanar, false otherwise.
 */
bool kjb::check_if_4_points_are_coplanar(const Vector& p1, 
                                         const Vector& p2, 
                                         const Vector& p3, 
                                         const Vector& p4, 
                                         double tolerance)
{
    // Check size of points.
    if(p1.get_length() < 3 || p2.get_length() < 3 || 
       p3.get_length() < 3 || p4.get_length() < 3)
    {
        throw KJB_error("One of the points has fewer than 3 coordinates");
    }
    else
    {
        Vector ab(3);
        Vector ac(3);
        Vector ad(3);

        for(int i = 0; i < 3; i++)
        {
            ab(i) = p2(i) - p1(i);
            ac(i) = p3(i) - p1(i);
            ad(i) = p4(i) - p1(i);
        }

        // Note: manually calculating the cross and dot products seems to be 
        //       faster (time is 1242250000 vs 1383920000 for airVent)

        // Get the cross product of AC and AD.
        Vector crossProd(3);
//        crossProd = ac.cross_with(ad);
        crossProd(0) = (ac(1)*ad(2)) - (ac(2)*ad(1));
        crossProd(1) = (ac(2)*ad(0)) - (ac(0)*ad(2));
        crossProd(2) = (ac(0)*ad(1)) - (ac(1)*ad(0));

        // Get the dot product of AB and (AC x AD).
//        double volume = dot(ab, crossProd);
        double volume = (ab(0)*crossProd(0)) + 
                        (ab(1)*crossProd(1)) + 
                        (ab(2)*crossProd(2));

        if(fabs(volume) < tolerance)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}


/*
 * @param  plane1_params  the Vector of plane parameters for face 1
 * @param  plane2_params  the Vector of plane parameters for face 2
 *
 * @return  a double representing the angle in radians between the normal 
 *          vectors of the two faces (will be between 0 and PI) 
 */
double kjb::get_angle_between_two_vectors(const Vector& plane1_params, 
                                          const Vector& plane2_params) 
            throw (Illegal_argument, KJB_error)
{
    try
    {
        double mag1 = sqrt(plane1_params(0)*plane1_params(0) + 
                           plane1_params(1)*plane1_params(1) + 
                           plane1_params(2)*plane1_params(2));
        double mag2 = sqrt(plane2_params(0)*plane2_params(0) + 
                           plane2_params(1)*plane2_params(1) + 
                           plane2_params(2)*plane2_params(2));
        double dot = plane1_params(0)*plane2_params(0) + 
                     plane1_params(1)*plane2_params(1) + 
                     plane1_params(2)*plane2_params(2);

        double cos_angle = dot / (mag1 * mag2);

        if(fabs(cos_angle) > 1.0000000000000005)
        {
            throw KJB_error("ERROR: invalid domain for acos");
        }

        if(fabs(cos_angle) > 1.0)
        {
            return acos(1.0);
        }

        // This will return an angle that is greater than 0 and less than 3.14
        return acos(cos_angle);

    }catch(Illegal_argument ex)
    {
        throw ex;
    }
}

/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

/**
 * Finds all of the planes which contain at least one face in the polymesh.
 * Does not include planes for which the total area of the faces lying in it
 * is less than a threshold value.
 *
 * @param p  the polymesh that the faces are from.
 * @param plane  the vector of Polymesh_Planes in which the plane parameters
 *               and the corresponding face indices are stored.
 */
void kjb::find_planes(const Polymesh& p, std::vector<Polymesh_Plane>& plane)
{
    plane.clear();
    double threshold = 0.000001;    // Used to eliminate planes that consist of
                                    // less than some % of the polymesh.
    double tolerance = 0.087266;    // 5 degrees

    std::vector<int> all_indices;

    double surface = p.compute_surface_area();
    std::vector<Polygon> faces = p.get_faces();

    int num_faces = p.num_faces();

    for(int i = 0; i < num_faces; i++)
    {
        // Check if the given face has already been included in a previous
        // plane that was found.
        int found = 0;
        for(unsigned int x = 0; x < all_indices.size(); x++)
        {
            if(all_indices[x] == i)
            {
                found = 1;
                break;
            }
        }

        // If the face has not been looked at yet, find its plane parameters.
        if(found == 0)
        {
            double area = 0;

            std::vector<Polygon> coplanar_faces;
            coplanar_faces.push_back(faces[i]);

            std::vector<int> indices;
            indices.push_back(i);
            all_indices.push_back(i);

            area += faces[i].compute_area();

            Vector plane1_params;
            Vector plane2_params;

            faces[i].fit_plane(plane1_params);

            // Compare each of the remaining faces to the current one and check
            // if they lie in the same plane (i.e. have the same plane params).
            for(int j = i+1; j < num_faces; j++)
            {
                faces[j].fit_plane(plane2_params);

                if(check_if_faces_are_coplanar(plane1_params, 
                                               plane2_params, 
                                               tolerance))
                {
                    coplanar_faces.push_back(faces[j]);

                    indices.push_back(j);
                    all_indices.push_back(j);

                    area += faces[j].compute_area();
                }
            }

            // If the fraction of the area of the plane found over the total 
            // surface area of the object is greater than the threshold value,
            // add the plane to the vector of Polymesh_Planes.
            if((area/surface) > threshold)
            {
                plane.push_back(Polymesh_Plane(p, plane1_params, indices));
            }
        }
    }
}

/**
 * Renders all of the faces lying in the same plane the same color. Renders
 * the faces lying in different planes different colors.  All faces not 
 * included in a plane (because they consist of a minimal part of the mesh)
 * are rendered the same color.
 *
 * @param p  the polymesh to be rendered.
 * @param planes  the vector of Polymesh_Planes containing the lists of indices
 *                which lie in the same planes.
 */
void kjb::render_planes(const Polymesh & p, 
                        const std::vector<Polymesh_Plane> & planes)
{
#ifdef KJB_HAVE_OPENGL
    std::vector<int> indices_rendered;

    float red = 1.0;
    float green = 1.0;
    float blue = 1.0;

    int cube_root = std::pow(planes.size() + 1, 1.0/3.0);

    float increment = 1.0 / cube_root;

    for(unsigned int count = 0; count < planes.size(); count++)
    {
        // Determines the color of the faces in the given plane. 
        // Partitions the color space into more partitions than
        // necessary.  Assumes there are faces not included in the
        // planes and takes them into account as an additional plane.
        if(count != 0)
        {
            // increment red
            int red_counter = std::pow(cube_root + 1, 2.0);
            if(count % red_counter == 0)
            {
                red -= increment;
                green = 1.0;
                blue = 1.0;
            }
            // increment green
            else if(count % (cube_root + 1) == 0 )
            {
                green -= increment;
                blue = 1.0;
            }
            // increment blue
            else
            {
                blue -= increment;
            }
        }
        glColor3f(red, green, blue);

        std::vector<int> indices = planes[count].get_face_indices();
        for(unsigned int j = 0; j < indices.size(); j++)
        {
            GL_Polygon_Renderer::solid_render(p.get_face(indices[j]));
        
            indices_rendered.push_back(indices[j]);
        }
    }
    
    glColor3f(red - increment, green - increment, blue);
    
    // render the faces not in a plane
    for(unsigned int k = 0; k < p.num_faces(); k++)     
    {
        int rendered = 0;
        for(unsigned int r = 0; r < indices_rendered.size(); r++)
        {
            int cur_face = k;
            if(indices_rendered[r] == cur_face)
            {
                rendered = 1;
                break;
            }
        }
        if(rendered == 0)
        {
            GL_Polygon_Renderer::solid_render(p.get_face(k));
        }
        rendered = 0;
    }
#endif
}

// TODO: This method still needs to be tested.
/**
 * For each point, creates a list of the points within a sphere of radius 10% of
 * length of longest bounding-box edge and centered at the point.  Fits a circle
 * to each set of 3 points in a list, then checks ALL points in polymesh to see 
 * if they lie on the circle.
 *
 * @param p  the polymesh that the faces are from.
 * @param circles  the vector of Circle_in_3d objects which contain the
 *                 parameters (circle, radius, normal) needed to define a circle
 *                 in the 3d space of the polymesh.
 * @param points  the vector containing lists of points (one for each circle).
 */
void kjb::find_all_circles_in_polymesh
(
    Polymesh&                          p, 
    std::vector<Circle_in_3d>&         circles, 
    std::vector<std::vector<Vector> >& points
)
{
    circles.clear();
    points.clear();

    double diffTolerance = 1e-8;
    double tolerance = 0.0000001;
    double coplanarPointTolerance = 4e-12;
   
    std::vector<Vector> all_vertices;
    std::vector<std::vector<int> > pointsInSphere;

    // Create list of all distinct vertices.
    p.get_all_vertices(all_vertices);
    int num_vertices = all_vertices.size();

    // Compute radius of spheres such that the radius is 10% of the longest edge
    // of the bounding box around the polymesh.
    p.find_bounds();
    Vector smallest = p.get_smallest_bounds();
    Vector largest = p.get_largest_bounds();
    double radius = largest(0) - smallest(0);
    for(int i = 1; i < 3; i++)
    {
        double diff = largest(i) - smallest(i);
        if(radius < diff)
        {
            radius = diff;
        }
    }
    radius = 0.10 * radius;

    double rad_squared = radius * radius;

    // For each point, create a list of points that lie within a sphere with the
    // computed radius centered at that point.
    for(int i = 0; i < num_vertices; i++)
    {
        // A list of the positions of the vertices in the vector all_vertices 
        // that lie within the sphere.
        std::vector<int> pointList;  
        // Add the center point to the pointList.
        pointList.push_back(i);
        for(int j = 0; j < num_vertices; j++)
        {
            // Check that we are not comparing the center point to itself.
            if(i != j)
            {
                double x_diff = all_vertices[j](0) - all_vertices[i](0);
                double y_diff = all_vertices[j](1) - all_vertices[i](1);
                double z_diff = all_vertices[j](2) - all_vertices[i](2);

                // Check that the distance between the two points is less than 
                // or equal to the radius of the sphere. If it is, add the point
                // to the list.
                if(((x_diff*x_diff) + (y_diff*y_diff) + (z_diff*z_diff)) 
                                                                <= rad_squared)
                {
                    pointList.push_back(j);
                }
            }
        }
        // Save the list before moving on to the next point.
        pointsInSphere.push_back(pointList);
    }

    // For each list of points, fit a circle to every set of 3 points.
    // Keep track of whether or not a circle has already been fit to a set of 
    // points using a linear vector with the following formula to access the 
    // correct index:
    //    index of (i,j,k) = i*num_vertices*num_vertices + j*num_vertices + k
    std::vector<int> circleFound;
    // Create a vector of size num_vertices^3 with all values initialized 
    // to 0.
    for(int i = 0; i < num_vertices-2; i++)
    {
        for(int j = i+1; j < num_vertices-1; j++)
        {
            for(int k = j+1; k < num_vertices; k++)
            {
                circleFound.push_back(0);
            }
        }
    }

    // For each list of points,
    for(int i = 0; i < pointsInSphere.size(); i++)
    {
        std::vector<int> pointList = pointsInSphere[i];
        // Get every combination of 3 points.
        int pointList_size = pointList.size();
        for(int p1 = 0; p1 < pointList_size - 2; p1++)
        {
            long long index1 = 0;
            if(p1 > 0)
            {
                // summation from k = (num-p1) to (num-1) of k choose 2
                //     equals (num choose 3) - ((num - p1) choose 3)
                index1 = ((num_vertices*(num_vertices-1)*(num_vertices-2)) - 
                          ((num_vertices-p1)*(num_vertices-p1-1)
                                            *(num_vertices-p1-2)))/6;
            }
            for(int p2 = p1+1; p2 < pointList_size - 1; p2++)
            {
                long long index2 = 0;
                if(p2 > p1 + 1)
                {
                    double tmp2 = (p2 - (p1 + 1)) * num_vertices - 
                                  (((p2/2.0)*(1+p2)) - (((p1+1)/2.0)*(1+p1+1)));
                    index2 = tmp2;
                }
                for(int p3 = p2+1; p3 < pointList_size; p3++)
                {
                    // Check if the combination has been checked yet by looking 
                    // in circleFound.
                    long long index3 = p3 - (p2 + 1);
                    long long pos = index1 + index2 + index3;
                    if(circleFound[pos] == 0)   
                    {
                        // Change the value at pos in circleFound to 1.
                        circleFound[pos] = 1;

                        Vector side_a = all_vertices[pointList[p2]] - 
                                        all_vertices[pointList[p1]];
                        Vector side_b = all_vertices[pointList[p2]] - 
                                        all_vertices[pointList[p3]];
                        Vector side_c = all_vertices[pointList[p3]] - 
                                        all_vertices[pointList[p1]];

                        // If the magnitude of the cross product is 0, then the
                        // 3 points are collinear and we can't fit a circle to 
                        // them. Get the cross product of side_a and side_c.
                        Vector crossProd(3);
                        crossProd(0) = (side_a(1)*side_c(2)) -
                                       (side_a(2)*side_c(1));
                        crossProd(1) = (side_a(2)*side_c(0)) - 
                                       (side_a(0)*side_c(2));
                        crossProd(2) = (side_a(0)*side_c(1)) - 
                                       (side_a(1)*side_c(0));
                        double magnitude = sqrt(crossProd(0)*crossProd(0) + 
                                                crossProd(1)*crossProd(1) + 
                                                crossProd(2)*crossProd(2));
 
                        if(magnitude > DBL_EPSILON)
                        {
                            // Fit a circle to the points since it has not been 
                            // done yet. A circle in 3d is defined by its 
                            // center, radius, and the normal vector to the 
                            // plane it lies in.

                            double length_a = side_a.magnitude();
                            double length_b = side_b.magnitude();
                            double length_c = side_c.magnitude();

                            // Find the radius of the circle.
                            double circle_radius = (length_a*length_b*length_c)/
                                                   (2 * magnitude);

                            // Find normal vector to the plane the points are 
                            // in. The cross product is the normal vector. 
                            // Normalize it by dividing by the magnitude.
                            Vector normal(3);
                            normal(0) = crossProd(0) / magnitude;
                            normal(1) = crossProd(1) / magnitude;
                            normal(2) = crossProd(2) / magnitude;

                            // Find center of circle (circumcenter of triangle).
                            Vector circle_center(3);
                            Vector tmp_center(3);
                            tmp_center = ((length_a*length_a)*side_c) -
                                         ((length_c*length_c)*side_a);
                            // Cross product of tmp_center and crossProd.
                            circle_center(0) = (tmp_center(1)*crossProd(2)) -
                                              (tmp_center(2)*crossProd(1));
                            circle_center(1) = (tmp_center(2)*crossProd(0)) - 
                                              (tmp_center(0)*crossProd(2));
                            circle_center(2) = (tmp_center(0)*crossProd(1)) - 
                                              (tmp_center(1)*crossProd(0));

                            circle_center = circle_center / 
                                            (2 * magnitude * magnitude);
                            
                            circle_center(0) = circle_center(0) +
                                               all_vertices[pointList[p1]](0);
                            circle_center(1) = circle_center(1) +
                                               all_vertices[pointList[p1]](1);
                            circle_center(2) = circle_center(2) +
                                               all_vertices[pointList[p1]](2);

                            // Store the circle parameters.
                            Circle_in_3d c(circle_center, 
                                           circle_radius, 
                                           normal);
                            circles.push_back(c);
                        }
                    }
                }
            }
        }
    }

    // Find and store all of the points that lie on each circle.
    int num_circles = circles.size();

// std::cout << "num_circles = " << num_circles << std::endl;

    for(int c = 0; c < num_circles; c++)
    {
        std::vector<Vector> pointsOnCircle;
        pointsOnCircle.clear();

        Vector center = circles[c].get_circle_center();
        Vector norm = circles[c].get_circle_normal();
        double radius = circles[c].get_circle_radius();

        for(int v = 0; v < num_vertices; v++)
        {
            // Determine if the point is on the same plane as the circle.
            Vector tmp_vec(3);
            for(int i = 0; i < 3; i++)
            {
                tmp_vec(i) = all_vertices[v](i) - center(i);
            }
            // Compute the dot product with the normal.
            double dotProd = (tmp_vec(0) * norm(0)) + 
                             (tmp_vec(1) * norm(1)) + 
                             (tmp_vec(2) * norm(2));

            // If the dot product equals 0, then the point is on the same plane
            // as the circle.
            if(fabs(dotProd) < tolerance)
            {
                // Check that the distance from the point to the center equals 
                // the radius of the circle.
                if(fabs(tmp_vec.magnitude() - radius) < tolerance)
                {
                    // Store the point since it lies on the circle.
                    pointsOnCircle.push_back(all_vertices[v]);
                }
            }
        }
        
        // Store the list of points for the current circle.
        points.push_back(pointsOnCircle);
    }
}



/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

/**
 * Searches through a vector of Polygons representing the faces in a polymesh 
 * and checks each polygon to see if it is a right triangle.  If it is, the 
 * corresponding element in the Int_vector mask is set to 1. If the polygon is 
 * not a right triangle, the mask element is 0.
 *
 * @param  faces  the vector of polygons
 * @param  mask  the Int_vector mask representing which polygons are right 
 *               triangles
 */
void kjb::find_right_triangles(const std::vector<kjb::Polygon> & faces, 
                               Int_vector & mask)
{
    mask.kjb::Int_vector::resize(faces.size(), 0);      

    // Set all values in mask vector to 0 or 1. 
    // Iterate through polygons and check if they are right triangles.
    for(unsigned int i = 0; i < faces.size(); i++)
    {
        if(faces[i].check_polygon_is_right_triangle(0.0087266))
        {
            mask(i) = 1;  
        }
        else
        {
            mask(i) = 0;
        }
    }
}


/**
 * @param  faces  the vector of polygons
 * @param  mask  the Int_vector mask representing which polygons are right 
 *               triangles
 * @param  p  the polymesh that the faces are from
 * @param  triangles  the vector of triangle pairs that form rectangles
 */
void kjb::find_rectangles(const std::vector<kjb::Polygon> & faces, 
                          const Int_vector & mask, 
                          const Polymesh & p, 
                          std::vector<Right_Triangle_Pair>& triangles)
{
    double coplanarTolerance = 0.0087266; // approx. 0.5 degrees
    double parAngleTolerance = 0.0087266;  // 0.087266 == 5 degrees
    
    // For each right triangle, find all adjacent faces and check if any 
    // are right triangles.
    for(unsigned int f = 0; f < faces.size(); f++)
    {
        if(mask(f) == 1)
        {
            // Find which edge is the hypotenuse and check if adjacent
            // face is a right triangle. If so, check that the shared
            // edge is the hypotenuse of the adjacent face.
            // Also need to check if coplanar and if have same vertices
            // along shared edge.

            // Find hypotenuse
            int hypotenuse = faces[f].get_index_of_longest_edge();

            // Note: Assumes that each edge will only be adjacent to 1 other 
            //       edge.

            // Check if face adjacent to the hypotenuse is a right triangle.
            unsigned int adj_face = p.adjacent_face(f, hypotenuse);
            
            // if adj_face < f, then we've already checked it.
            if( mask(adj_face) == 1 && adj_face > f)    
            {
                // Check that adj_face and f are coplanar.
                Vector plane1_params;
                Vector plane2_params;

                faces[f].fit_plane(plane1_params);
                faces[adj_face].fit_plane(plane2_params);

                const std::vector<Vector> & vertices = faces[f].get_vertices();
                const std::vector<Vector> & vertices2 = 
                                                faces[adj_face].get_vertices();

                if(check_if_faces_are_coplanar(plane1_params, 
                                               plane2_params, 
                                               coplanarTolerance))
                {
                    // Check that adj_face is adjacent to f across hypotenuse.
                    int hypotenuse2 = 
                                    faces[adj_face].get_index_of_longest_edge();
           
                    // Check that f is the face adjacent to hypotenuse2.
                    if(f == p.adjacent_face(adj_face, hypotenuse2))
                    {
                        int same = 1;
                        int rev = 1;

                        // Check that there are parallel edges.
                        if(same == 1 || rev == 1)
                        {
                            // If the two triangles form a rectangle, we 
                            // need to identify which edges are parallel to 
                            // each other.
                            int isParallel = 0;

                            Vector p_edges1(2);
                            Vector p_edges2(2);

                            // Get the vectors for three of the four edges 
                            // (1 from triangle1, 2 from triangle2)
                            Vector edge1_1(3);
                            Vector edge1_2(3);
                            Vector edge2_1(3);
                            Vector edge2_2(3);

                            for(int v = 0; v < 3; v++)
                            {
                                edge1_1(v) = vertices[hypotenuse](v) - 
                                             vertices[(hypotenuse-1+3)%3](v);
                                edge1_2(v) = vertices[(hypotenuse+1)%3](v) - 
                                             vertices[(hypotenuse-1+3)%3](v);

                                edge2_1(v) = vertices2[hypotenuse2](v) - 
                                             vertices2[(hypotenuse2-1+3)%3](v);
                                edge2_2(v) = vertices2[(hypotenuse2+1)%3](v) - 
                                             vertices2[(hypotenuse2-1+3)%3](v);
                            }

                            // Normalize the edge vectors.
                            edge1_1.normalize();
                            edge1_2.normalize();
                            edge2_1.normalize();
                            edge2_2.normalize();

                            // Two edges are parallel if the angle between them
                            // is 0 or pi.
                            double angle1 = get_angle_between_two_vectors
                                                            (edge1_1, edge2_1);
                            double angle2 = get_angle_between_two_vectors
                                                            (edge1_1, edge2_2);

                            double angle3 = get_angle_between_two_vectors
                                                            (edge1_2, edge2_1);
                            double angle4 = get_angle_between_two_vectors
                                                            (edge1_2, edge2_2);

                            if(angle1 < parAngleTolerance || 
                               angle1 > (M_PI - parAngleTolerance))
                            {
                                isParallel = 1;

                                p_edges1(0) = (hypotenuse-1+3)%3;
                                p_edges1(1) = (hypotenuse2-1+3)%3;

                                p_edges2(0) = (hypotenuse+1)%3;
                                p_edges2(1) = (hypotenuse2+1)%3;
                            }
                            else if(angle2 < parAngleTolerance || 
                                    angle2 > (M_PI - parAngleTolerance))
                            {
                                isParallel = 1;

                                p_edges1(0) = (hypotenuse-1+3)%3;
                                p_edges1(1) = (hypotenuse2+1)%3;

                                p_edges2(0) = (hypotenuse+1)%3;
                                p_edges2(1) = (hypotenuse2-1+3)%3;
                            }
                            else if(angle4 < parAngleTolerance || 
                                    angle4 > (M_PI - parAngleTolerance))
                            {
                                isParallel = 1;

                                p_edges1(0) = (hypotenuse-1+3)%3;
                                p_edges1(1) = (hypotenuse2-1+3)%3;

                                p_edges2(0) = (hypotenuse+1)%3;
                                p_edges2(1) = (hypotenuse2+1)%3;
                            }
                            else if(angle3 < parAngleTolerance || 
                                    angle3 > (M_PI - parAngleTolerance))
                            {
                                isParallel = 1;

                                p_edges1(0) = (hypotenuse-1+3)%3;
                                p_edges1(1) = (hypotenuse2+1)%3;

                                p_edges2(0) = (hypotenuse+1)%3;
                                p_edges2(1) = (hypotenuse2-1+3)%3;
                            }
                            else
                            {
                                isParallel = 0;
                            }

                            if(isParallel == 1)
                            {
                                // Save information in Right_Triangle_Pair 
                                // class.
                                triangles.push_back(
                                        Right_Triangle_Pair(p, 
                                                            f, 
                                                            adj_face, 
                                                            hypotenuse, 
                                                            hypotenuse2, 
                                                            p_edges1, 
                                                            p_edges2));
                            }
                        }
                    }
                }
            }
        }
    }
}


/*
 * Creates an Int_vector mask of the polymesh where the value at a given index 
 * represents the position in the rectangles vector of the rectangle that that 
 * face belongs to.  The value is -1 if the face is not part of a rectangle.
 *
 * @param  p  the polymesh containing the faces
 * @param  rectangles  a vector of Right_Triangle_Pairs which contains the 
 *                     indices of the faces that form rectangles
 * @param  mask  the Int_vector mask representing which faces are part of 
 *               rectangles
 */
void kjb::create_rectangle_mask(const Polymesh& p, 
                            const std::vector<Right_Triangle_Pair>& rectangles, 
                            Int_vector& mask)
{
    Int_vector rectMask(p.num_faces(), -1);

    for(size_t i = 0; i < rectangles.size(); i++)
    {
        int index1 = rectangles[i].get_triangle1();
        int index2 = rectangles[i].get_triangle2();

        rectMask(index1) = i;
        rectMask(index2) = i;
    }
    mask = rectMask;
}


/*
 * Recursive helper method for find_cylinders().
 * Determines which rectangles are adjacent to eachother and if a ring of 
 * adjacent rectangles form part of a cylinder (i.e. have approximately the same
 * angle between each pair of rectangles in the ring).  Stores the indices of 
 * the faces found to be in the cylinder in the vector cyl_indices.  Before this
 * method is called, the indices of both triangles in the starting rectangle 
 * must be added to the cyl_indices vector.
 *
 * @param  p  the polymesh
 * @param  cylMask  the Int_vector mask representing which faces are part of 
 *                  cylinders
 * @param  rectMask  the Int_vector mask representing which faces are part of 
 *                   rectangles
 * @param  rectangles  a vector of Right_Triangle_Pairs which contains the 
 *                     indices of the faces that form rectangles
 * @param  startIndex  an integer representing the index of the first face 
 *                     looked at while finding the current cylinder.
 * @param  rectIndex  index of the rectangle where one of its triangles has 
 *                    already been added to the cylinder list and we need to 
 *                    find the face adjacent to the other triangle (as well as 
 *                    add the other triangle index to the cylinder list).
 * @param  prevAdjFace  index of the previously found face that is adjacent to 
 *                      the current rectangle (rectIndex). Used to determine 
 *                      whether to use parEdge1 or parEdge2.
 * @param  smallestAngle  the double representing the smallest angle found 
 *                        between two faces in the cylinder
 * @param  largestAngle  the double representing the largest angle found between
 *                       two faces in the cylinder
 * @param  sumAngles  a double representing the sum of the angles between all of
 *                    the rectangles in the cylinder so far. Equals 0 if the 
 *                    cylinder is not valid (not same angle between each 
 *                    rectangle) and is not unique (cylinder has already been 
 *                    found).
 * @param  cyl_indices  a vector containing the indices of the faces in the 
 *                      cylinder.
 * @param  edge_pt1  a Vector representing one of the vertices of the current 
 *                   edge of the cylinder.
 * @param  edge_pt2  a Vector representing the other vertex of the current edge
 *                   of the cylinder.
 * @param  edge_pt1_adj  a Vector representing a point adjacent to and on the 
 *                       same side of the cylinder as either edge_pt1 or 
 *                       edge_pt2.
 * @param  edge_pt2_adj  a Vector representing a point adjacent to and on the 
 *                       same side of the cylinder as either edge_pt2 or 
 *                       edge_pt1.
 */
void kjb::find_adjacent_rectangles(const Polymesh & p, 
                            const Int_vector& cylMask, 
                            const Int_vector& rectMask, 
                            const std::vector<Right_Triangle_Pair>& rectangles, 
                            int startIndex, 
                            int rectIndex, 
                            int prevAdjFace, 
                            double width, 
                            double lengthTolerance, 
                            double& smallestAngle, 
                            double& largestAngle, 
                            double& sumAngles, 
                            std::vector<int>& cyl_indices, 
                            Vector& edge_pt1, 
                            Vector& edge_pt2, 
                            Vector& edge_pt1_adj, 
                            Vector& edge_pt2_adj)
{
    int otherRectFace;
    int newFace;
    Vector plane1_params;
    Vector plane2_params;
    double newAngle;
    double newWidth;
    double height;
    Vector temp_oldEdge1 = edge_pt1;
    Vector temp_oldEdge2 = edge_pt2;

    double angleTolerance = 0.0087266;   // 0.5 degrees

    int index1 = rectangles[rectIndex].get_triangle1();
    int index2 = rectangles[rectIndex].get_triangle2();

    Vector parEdge1 = rectangles[rectIndex].get_parallel_edges1();
    Vector parEdge2 = rectangles[rectIndex].get_parallel_edges2();

    int adjFace1_1 = p.adjacent_face(index1, parEdge1(0));
    int adjFace1_2 = p.adjacent_face(index1, parEdge2(0));
    int adjFace2_1 = p.adjacent_face(index2, parEdge1(1));
    int adjFace2_2 = p.adjacent_face(index2, parEdge2(1));
    Polygon otherRectFacePoly;

// Find points on outer edge of otherRectFace and store in edge_pt1 and 
// edge_pt2.  Store adjacent points in edge_pt1_adj and edge_pt2_adj.  Will keep
// overwriting these values until reach actual edge of cylinder.  Use 
// find_top_and_bottom_points_of_cylinder() method to determine which point is 
// on the bottom of the cylinder. ************

    if(adjFace1_1 == prevAdjFace)
    {
        // Get the second face of the rectangle.
        otherRectFace = index2;

        // Get the face adjacent to otherRectFace.
        newFace = adjFace2_1;

        // Get height of otherRectFace (which is also the height of newFace).
        otherRectFacePoly = p.get_face(otherRectFace);
        edge_pt1 = otherRectFacePoly.get_edge_first_vertex(parEdge1(1));
        edge_pt2 = otherRectFacePoly.get_edge_second_vertex(parEdge1(1));
    }
    else if(adjFace1_2 == prevAdjFace)
    {
        otherRectFace = index2;
        newFace = adjFace2_2;
        otherRectFacePoly = p.get_face(otherRectFace);
        edge_pt1 = otherRectFacePoly.get_edge_first_vertex(parEdge2(1));
        edge_pt2 = otherRectFacePoly.get_edge_second_vertex(parEdge2(1));
    }
    else if(adjFace2_1 == prevAdjFace)
    {
        otherRectFace = index1;
        newFace = adjFace1_1;
        otherRectFacePoly = p.get_face(otherRectFace);
        edge_pt1 = otherRectFacePoly.get_edge_first_vertex(parEdge1(0));
        edge_pt2 = otherRectFacePoly.get_edge_second_vertex(parEdge1(0));
    }
    else if(adjFace2_2 == prevAdjFace)
    {
        otherRectFace = index1;
        newFace = adjFace1_2;
        otherRectFacePoly = p.get_face(otherRectFace);
        edge_pt1 = otherRectFacePoly.get_edge_first_vertex(parEdge2(0));
        edge_pt2 = otherRectFacePoly.get_edge_second_vertex(parEdge2(0));
    }
    else
    {
        sumAngles = 0.0;
std::cout << "Error in adjacency matrix: one face is adjacent to another but " 
          << "not vice versa\n";
//        throw KJB_error("Error in adjacency matrix!\n");
        return;
    }

    edge_pt1_adj = temp_oldEdge1;
    edge_pt2_adj = temp_oldEdge2;

    height = sqrt((edge_pt1(0)-edge_pt2(0))*(edge_pt1(0)-edge_pt2(0))+
                  (edge_pt1(1)-edge_pt2(1))*(edge_pt1(1)-edge_pt2(1))+
                  (edge_pt1(2)-edge_pt2(2))*(edge_pt1(2)-edge_pt2(2)));

    // Add the second face of the rectangle to the cylinder indices vector.
    cyl_indices.push_back(otherRectFace);

    // Check that the new face is part of a rectangle, that the rect index is 
    // greater than the starting rect index, that we haven't come full circle, 
    // and that the angle is similar to the previous angle.

    // Return sumAngles if the next face is not part of a rectangle.
    int newRect = rectMask(newFace);
    if(newRect < 0)
    {
        // The outer edge of otherRectFace is one edge of the cylinder_section.
        return;
    }

    // Return sumAngles if the next face is already part of a cylinder.
    if(cylMask(newRect) >= 0)
    {
        return;
    }

    // Calculate the angle.
    (p.get_face(otherRectFace)).fit_plane(plane1_params);
    (p.get_face(newFace)).fit_plane(plane2_params);
    newAngle = get_angle_between_two_vectors(plane1_params, plane2_params);

    // If the newAngle is outside of the current range, check that the new range
    // is still within the angleTolerance. If it is, set the new 
    // largest/smallest angle. If not, the end of the cylinder has been reached,
    // so return.
    if(newAngle < smallestAngle)
    {
        if(fabs(largestAngle - newAngle) > angleTolerance)
        {
            return;
        }
        else
        {
            smallestAngle = newAngle;
        }
    }
    if(newAngle > largestAngle)
    {
        if(fabs(newAngle - smallestAngle) > angleTolerance)
        {
            return;
        }
        else
        {
            largestAngle = newAngle;
        }
    }

    // If the next face is the same as the starting one, return 2*Pi (360 
    // degrees) because this means the cylinder has come full circle.
    if(newFace == startIndex)
    {
        sumAngles += newAngle;
        if(fabs(sumAngles - (2*M_PI)) > angleTolerance)
        {
std::cout << "ERROR: sumAngles should equal 6.283 but doesn't (sumAngles = " 
          << sumAngles << ")!\n";
//            throw KJB_error("ERROR: sumAngles doesn't equal 6.283!\n");
        }
        sumAngles = 2*M_PI;
        return;
    }

    // Get the width of the rectangle formed by newFace.
    Polygon newFacePoly = p.get_face(newFace);
    Vector newParEdge1 = rectangles[newRect].get_parallel_edges1();
    Vector newParEdge2 = rectangles[newRect].get_parallel_edges2();
    double len_e1;
    double len_e2;
    Vector p1;
    Vector p2;
    if(newFace == rectangles[newRect].get_triangle1())
    {
        p1 = newFacePoly.get_edge_first_vertex(newParEdge1(0));
        p2 = newFacePoly.get_edge_second_vertex(newParEdge1(0));
        len_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                      (p1(1)-p2(1))*(p1(1)-p2(1))+
                      (p1(2)-p2(2))*(p1(2)-p2(2)));

        p1 = newFacePoly.get_edge_first_vertex(newParEdge2(0));
        p2 = newFacePoly.get_edge_second_vertex(newParEdge2(0));
        len_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                      (p1(1)-p2(1))*(p1(1)-p2(1))+
                      (p1(2)-p2(2))*(p1(2)-p2(2)));
    }
    else if(newFace == rectangles[newRect].get_triangle2())
    {
        p1 = newFacePoly.get_edge_first_vertex(newParEdge1(1));
        p2 = newFacePoly.get_edge_second_vertex(newParEdge1(1));
        len_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                      (p1(1)-p2(1))*(p1(1)-p2(1))+
                      (p1(2)-p2(2))*(p1(2)-p2(2)));

        p1 = newFacePoly.get_edge_first_vertex(newParEdge2(1));
        p2 = newFacePoly.get_edge_second_vertex(newParEdge2(1));
        len_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                      (p1(1)-p2(1))*(p1(1)-p2(1))+
                      (p1(2)-p2(2))*(p1(2)-p2(2)));
    }
    else
    {
        std::cout << "Error: newFace " << newFace 
                  << " is not in rectangle newRect " << newRect << "\n";
        throw KJB_error("Error: newFace is not in rectangle newRect\n");
        return;
    }

    if(fabs(len_e1 - height) < DBL_EPSILON)
    {
        newWidth = len_e2;
    }
    else if(fabs(len_e2 - height) < DBL_EPSILON)
    {
        newWidth = len_e1;
    }
    else
    {
        std::cout << "Error: height of newRect " << newRect 
                  << " doesn't match height of otherRectFace " << otherRectFace 
                  << "\n";
        throw KJB_error("newRect and otherRectFace heights don't match\n");
        return;
    }

    if(fabs(width - newWidth) > lengthTolerance)
    {
        // New rectangle isn't part of same cylinder as previous rectangle.
        return;
    }

    // Perform recursive call if the next face is part of a rectangle whose 
    // index is greater than the starting rectangle's index.
    if(rectMask(newFace) > rectMask(startIndex))
    {
        // If so add the face index to the cylinder vector.
        cyl_indices.push_back(newFace);
        sumAngles += newAngle;

        // Call recursive method.
        find_adjacent_rectangles(p, 
                                 cylMask, 
                                 rectMask, 
                                 rectangles, 
                                 startIndex, 
                                 rectMask(newFace), 
                                 otherRectFace, 
                                 width, 
                                 lengthTolerance, 
                                 smallestAngle, 
                                 largestAngle, 
                                 sumAngles, 
                                 cyl_indices, 
                                 edge_pt1, 
                                 edge_pt2, 
                                 edge_pt1_adj, 
                                 edge_pt2_adj);
        return;
    }

    // If reaches this point, then newFace is part of a rectangle (not the 
    // starting one) with the correct angle, but the rectangle index is smaller 
    // than the starting rectangle index, meaning that this cylinder has already
    // been found.
    sumAngles = 0.0;
    return;
}


/*
 * Determines which faces form cylinders and stores the indices of the faces 
 * that form a cylinder in a vector and then stores all of the cylinder vectors
 * in a vector.
 *
 * @param  p  the polymesh
 * @param  rectangles  a vector of Right_Triangle_Pairs which contains the 
 *                     indices of the faces that form rectangles
 * @param  cyl_indices  a vector containing all of the vectors of the indices of
 *                      the faces in a cylinder
 * @param  cylSumAngles  a vector containing the total angle of each cylinder
 * @param  cylEdgePoints  a vector containing sets of 6 points (stored in a 
 *                        vector) where the first two points are on 1 edge of 
 *                        each cylinder, the following two points are adjacent 
 *                        to the edge points, and the last two points are on the
 *                        other edge of each cylinder
 */
void kjb::find_cylinders(const Polymesh & p, 
                         const std::vector<Right_Triangle_Pair>& rectangles, 
                         std::vector<std::vector<int> >& cyl_indices, 
                         std::vector<double>& cylSumAngles, 
                         std::vector<std::vector<Vector> >& cylEdgePoints)
{
    // This is the minimum angle that the cylinder has to have to be counted 
    // (this will determine how complete the cylinder must be)
    // Value must be between 0 and 2*PI (6.283) radians
    double minCylinderSize = 1.570796327; // 90 degrees

    // This is the allowable difference between angles that should be the same.
    double angleTolerance = 0.0087266;   // 0.087266 = 5 degrees

    // The maximun angle allowed between two adjacent rectangles.
    double maxAngle = 0.785;  // = 45 deg
    
    // Note: two faces are considered coplanar if the angle between them is less
    //       than this specified tolerance.
    double coplanarAngleTolerance = 0.0087266; // 0.0000174533 = 0.001 deg

    // Allowed variance between the lengths of the rectangles in a cylinder.
    double lengthTolerance = 0.1;   

    // This is for debugging. Will probably be unneccessary if require cylinder
    // to be at least 1/4 complete.
    unsigned int minNumFacesPerCyl = 4;  

    // Create Int_vector of entire polymesh where the value at a given index 
    // represents the rectangle that the face belongs to.  -1 if not part of 
    // a rectangle.
    Int_vector rectMask;
    create_rectangle_mask(p, rectangles, rectMask);

    // When rectangle is added to a cylinder, store the cylinder number in this 
    // vector. The number corresponds to position in the vector of cylinders.
    // -1 means the face is not part of a cylinder yet.  
    // Only look at a rectangle if it is not already part of a cylinder 
    // (Note: a rectangle is only allowed to be part of 1 cylinder).
    Int_vector cylMask(rectangles.size(), -1);
    // This vector is used to store the angle between rectangles in each 
    // cylinder. The position in the vector corresponds to the cylinder number.
    std::vector<double> cylAngles;
    // Use this counter to keep track of which cylinder we are on.
    int cylNum = 0;

    for(int i = 0; i < (int)rectangles.size(); i++)
    {
        // Check that the rectangle is not already part of a cylinder.
        // If it is, move on to next rectangle.
        if(cylMask(i) < 0)
        {
            // indices for cylinder formed by parallel edge 1.
            std::vector<int> cyl1;  
            // indices for cylinder formed by parallel edge 2.
            std::vector<int> cyl2;  

            Vector edge1_pt1;
            Vector edge1_pt2;
            Vector edge2_pt1;
            Vector edge2_pt2;

            // The two points adjacent to the edge1 points.
            Vector edge1_pt1_adj;
            Vector edge1_pt2_adj;
            // The two points adjacent to the edge2 points.
            Vector edge2_pt1_adj;
            Vector edge2_pt2_adj;

            Vector otherEdge1_pt1;
            Vector otherEdge1_pt2;
            Vector otherEdge2_pt1;
            Vector otherEdge2_pt2;

            Vector plane1_params;
            Vector plane2_params;
            double angle1 = 0.0;
            double angle2 = 0.0;
            double sumAngles1 = 0.0;   // Cylinder complete if sumAngles == 2*PI
            double sumAngles2 = 0.0;
            double smallestAngle1 = 2*M_PI;
            double largestAngle1 = 0.0;
            double smallestAngle2 = 2*M_PI;
            double largestAngle2 = 0.0;

            int index1 = rectangles[i].get_triangle1();
            int index2 = rectangles[i].get_triangle2();

            Vector parEdge1 = rectangles[i].get_parallel_edges1();
            Vector parEdge2 = rectangles[i].get_parallel_edges2();

            int adjFace1_1 = p.adjacent_face(index1, parEdge1(0));
            int adjFace1_2 = p.adjacent_face(index1, parEdge2(0));
            int adjFace2_1 = p.adjacent_face(index2, parEdge1(1));
            int adjFace2_2 = p.adjacent_face(index2, parEdge2(1));

            Polygon face1 = p.get_face(index1);
            Polygon face2 = p.get_face(index2);

            // Note: length = sqrt((x1-x2)^2 + (y1-y2)^2 + (z1-z2)^2)
            Vector p1 = face1.get_point(parEdge1(0));
            Vector p2 = face1.get_point((int)(parEdge1(0) + 1)%3);
            double len_f1_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                    (p1(1)-p2(1))*(p1(1)-p2(1))+
                                    (p1(2)-p2(2))*(p1(2)-p2(2)));
            p1 = face1.get_point(parEdge2(0));
            p2 = face1.get_point((int)(parEdge2(0) + 1)%3);
            double len_f1_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                    (p1(1)-p2(1))*(p1(1)-p2(1))+
                                    (p1(2)-p2(2))*(p1(2)-p2(2)));
            p1 = face2.get_point(parEdge1(1));
            p2 = face2.get_point((int)(parEdge1(1) + 1)%3);
            double len_f2_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                    (p1(1)-p2(1))*(p1(1)-p2(1))+
                                    (p1(2)-p2(2))*(p1(2)-p2(2)));
            p1 = face2.get_point(parEdge2(1));
            p2 = face2.get_point((int)(parEdge2(1) + 1)%3);
            double len_f2_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                    (p1(1)-p2(1))*(p1(1)-p2(1))+
                                    (p1(2)-p2(2))*(p1(2)-p2(2)));
            
            double len_e1 = 0.0;
            double len_e2 = 0.0;

            // Check lengths of parallel sides are the same.
            if(fabs(len_f1_e1 - len_f2_e1) < lengthTolerance && 
               fabs(len_f1_e2 - len_f2_e2) < lengthTolerance)
            {
                len_e1 = (len_f1_e1 + len_f2_e1)/2.0; 
                len_e2 = (len_f1_e2 + len_f2_e2)/2.0;
            }
            else
            {
                // Skip to next iteration of for loop.
                continue;
            }

            // Add triangle indices of the first rectangle to both cylinders.
            cyl1.push_back(index1);
            cyl1.push_back(index2);

            cyl2.push_back(index1);
            cyl2.push_back(index2);

            bool valid1 = false;
            bool valid2 = false;

            bool complete1 = false;
            bool complete2 = false;

            // The valid bit is set to false if a rectangle with a smaller index
            // and the same angle was encountered.
            // If valid == false, only the original rectangle is in the cylinder
            // and sumAngles == 0.
            // For the second half of the cylinder, if valid == false, compare 
            // the angles.  If same, discard whole cylinder. If different, keep 
            // second half.

            // Check if the face adjacent to triangle1 along parallel edge 1 is
            // part of a rectangle.
            int rectAdj_1_1 = rectMask(adjFace1_1);
            // Only look at face if it is part of a rectangle AND not part of a
            // cylinder.
            if(rectAdj_1_1 > i && cylMask(rectAdj_1_1) < 0) 
            {
                Polygon adjFacePoly = p.get_face(adjFace1_1);

                face1.fit_plane(plane1_params);
                adjFacePoly.fit_plane(plane2_params);
                angle1 = get_angle_between_two_vectors(plane1_params, 
                                                       plane2_params);

                // Check that the rectangles are not coplanar or greater than 
                // maxAngle.
                if(angle1 > coplanarAngleTolerance && angle1 < maxAngle)
                {
                    // Check width of rectangle i is same as width of adjacent 
                    // triangle.
                    Vector adjParEdge1 = 
                                rectangles[rectAdj_1_1].get_parallel_edges1();
                    Vector adjParEdge2 = 
                                rectangles[rectAdj_1_1].get_parallel_edges2();
                    int adjTriIndex;
                    if(adjFace1_1 == rectangles[rectAdj_1_1].get_triangle1())
                    {
                        adjTriIndex = 0;
                    }
                    else
                    {
                        adjTriIndex = 1;
                    }
                    p1 = adjFacePoly.get_point(adjParEdge1(adjTriIndex));
                    p2 = adjFacePoly.get_point((int)
                                                (adjParEdge1(adjTriIndex)+1)%3);
                    double adjLen_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                            (p1(1)-p2(1))*(p1(1)-p2(1))+
                                            (p1(2)-p2(2))*(p1(2)-p2(2)));
                    p1 = adjFacePoly.get_point(adjParEdge2(adjTriIndex));
                    p2 = adjFacePoly.get_point((int)
                                                (adjParEdge2(adjTriIndex)+1)%3);
                    double adjLen_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                            (p1(1)-p2(1))*(p1(1)-p2(1))+
                                            (p1(2)-p2(2))*(p1(2)-p2(2)));

                    if((fabs(len_e1 - adjLen_e1) < lengthTolerance && 
                        fabs(len_e2 - adjLen_e2) < lengthTolerance) 
                        ||  (fabs(len_e1 - adjLen_e2) < lengthTolerance && 
                             fabs(len_e2 - adjLen_e1) < lengthTolerance))
                    {
                        cyl1.push_back(adjFace1_1);
                        sumAngles1 += angle1;

                        smallestAngle1 = angle1;
                        largestAngle1 = angle1;

                        // The outer edge of the starting rectangle.
                        otherEdge1_pt1 = face2.get_point(parEdge1(1));
                        otherEdge1_pt2 = face2.get_point((int)
                                                           (parEdge1(1) + 1)%3);

                        // Call recursive method.
                        find_adjacent_rectangles(p, 
                                                 cylMask, 
                                                 rectMask, 
                                                 rectangles, 
                                                 index2, 
                                                 rectMask(adjFace1_1), 
                                                 index1, 
                                                 len_e2, 
                                                 lengthTolerance, 
                                                 smallestAngle1, 
                                                 largestAngle1, 
                                                 sumAngles1, 
                                                 cyl1, 
                                                 edge1_pt1, 
                                                 edge1_pt2, 
                                                 edge1_pt1_adj, 
                                                 edge1_pt2_adj);
                        
                        if(fabs(sumAngles1 - 0.0) < DBL_EPSILON)
                        {
                            sumAngles1 = 0.0;
                            angle1 = 0.0;
                            valid1 = false; 
                            cyl1.clear();

                            cyl1.push_back(index1);
                            cyl1.push_back(index2);
                        }
                        else
                        {
                            valid1 = true;
                        }

                        if(fabs(sumAngles1 - 2*M_PI) < DBL_EPSILON)
                        {
                            complete1 = true;
                        }
                    }
                }
            }

            // Check if the face adjacent to triangle2 along parallel edge 1 is
            // part of a rectangle.
            if(!complete1)
            {
                int rectAdj_2_1 = rectMask(adjFace2_1);
                if(rectAdj_2_1 > i && cylMask(rectAdj_2_1) < 0)
                {
                    Polygon adjFacePoly = p.get_face(adjFace2_1);

                    face2.fit_plane(plane1_params);
                    adjFacePoly.fit_plane(plane2_params);
                    double angle = get_angle_between_two_vectors(plane1_params,
                                                                 plane2_params);

                    // If valid and the angles are different, then we have two 
                    // separate conjoined cylinders. So save the previous 
                    // cylinder and move on to other edge (cyl2).
                    if( valid1 && (angle1 > coplanarAngleTolerance) && 
                                   angle1 < maxAngle )
                    {
                        if((angle > largestAngle1 && 
                           (fabs(angle - smallestAngle1) >= angleTolerance)) || 
                           (angle < smallestAngle1 && 
                           (fabs(largestAngle1 - angle) >= angleTolerance)))
                        {
                            // cyl1 will be saved at end of loop.  
                        }
                        else    // The angles are the same and cyl1 is valid, so
                                // continue adding faces to cyl1.
                        {
                            // Check width of rectangle i is same as width of 
                            // adjacent triangle.
                            Vector adjParEdge1 = 
                                rectangles[rectAdj_2_1].get_parallel_edges1();
                            Vector adjParEdge2 = 
                                rectangles[rectAdj_2_1].get_parallel_edges2();
                            int adjTriIndex;
                            if(adjFace2_1 == 
                                    rectangles[rectAdj_2_1].get_triangle1())
                            {
                                adjTriIndex = 0;
                            }
                            else
                            {
                                adjTriIndex = 1;
                            }
                            p1 = adjFacePoly.get_point(
                                            adjParEdge1(adjTriIndex));
                            p2 = adjFacePoly.get_point((int)
                                            (adjParEdge1(adjTriIndex)+1)%3);
                            double adjLen_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                                    (p1(1)-p2(1))*(p1(1)-p2(1))+
                                                   (p1(2)-p2(2))*(p1(2)-p2(2)));
                            p1 = adjFacePoly.get_point(
                                            adjParEdge2(adjTriIndex));
                            p2 = adjFacePoly.get_point((int)
                                            (adjParEdge2(adjTriIndex)+1)%3);
                            double adjLen_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                                    (p1(1)-p2(1))*(p1(1)-p2(1))+
                                                   (p1(2)-p2(2))*(p1(2)-p2(2)));

                            if((fabs(len_e1 - adjLen_e1) < lengthTolerance && 
                                fabs(len_e2 - adjLen_e2) < lengthTolerance) || 
                                (fabs(len_e1 - adjLen_e2) < lengthTolerance && 
                                 fabs(len_e2 - adjLen_e1) < lengthTolerance))
                            {
                                angle1 = angle;
                                
                                cyl1.push_back(adjFace2_1);
                                sumAngles1 += angle1;

                                if(angle1 < smallestAngle1)
                                {
                                    smallestAngle1 = angle1;
                                }
                                if(angle1 > largestAngle1)
                                {
                                    largestAngle1 = angle1;
                                }

                                otherEdge1_pt1 = edge1_pt1;
                                otherEdge1_pt2 = edge1_pt2;

                                // Call recursive method.
                                find_adjacent_rectangles(p, 
                                                         cylMask, 
                                                         rectMask, 
                                                         rectangles, 
                                                         index1, 
                                                         rectMask(adjFace2_1), 
                                                         index2, 
                                                         len_e2, 
                                                         lengthTolerance, 
                                                         smallestAngle1, 
                                                         largestAngle1, 
                                                         sumAngles1, 
                                                         cyl1, 
                                                         edge1_pt1, 
                                                         edge1_pt2, 
                                                         edge1_pt1_adj, 
                                                         edge1_pt2_adj);

                                if(fabs(sumAngles1 - 0.0) < DBL_EPSILON)
                                {
                                    valid1 = false;
                                    cyl1.clear();
                                }
                                else
                                {
                                    valid1 = true;
                                }

                                if(fabs(sumAngles1 - 2*M_PI) < DBL_EPSILON)
                                {
                                    complete1 = true;
                                }
                            }
                        }
                    }

                    if( (!valid1) && cyl1.size() == 2 && 
                        angle > coplanarAngleTolerance && 
                        angle < maxAngle) // cyl1 only contains index 1 and 2.
                    {
                        // Check width of rectangle i is same as width of 
                        // adjacent triangle.
                        Vector adjParEdge1 = 
                            rectangles[rectAdj_2_1].get_parallel_edges1();
                        Vector adjParEdge2 = 
                            rectangles[rectAdj_2_1].get_parallel_edges2();
                        int adjTriIndex;
                        if(adjFace2_1==rectangles[rectAdj_2_1].get_triangle1())
                        {
                            adjTriIndex = 0;
                        }
                        else
                        {
                            adjTriIndex = 1;
                        }
                        p1 = adjFacePoly.get_point(adjParEdge1(adjTriIndex));
                        p2 = adjFacePoly.get_point((int)
                                                (adjParEdge1(adjTriIndex)+1)%3);
                        double adjLen_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                                (p1(1)-p2(1))*(p1(1)-p2(1))+
                                                (p1(2)-p2(2))*(p1(2)-p2(2)));
                        p1 = adjFacePoly.get_point(adjParEdge2(adjTriIndex));
                        p2 = adjFacePoly.get_point((int)
                                                (adjParEdge2(adjTriIndex)+1)%3);
                        double adjLen_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                                (p1(1)-p2(1))*(p1(1)-p2(1))+
                                                (p1(2)-p2(2))*(p1(2)-p2(2)));

                        if((fabs(len_e1 - adjLen_e1) < lengthTolerance && 
                            fabs(len_e2 - adjLen_e2) < lengthTolerance) || 
                            (fabs(len_e1 - adjLen_e2) < lengthTolerance && 
                             fabs(len_e2 - adjLen_e1) < lengthTolerance))
                        {
                            angle1 = angle;

                            cyl1.push_back(adjFace2_1);
                            sumAngles1 += angle1;

                            smallestAngle1 = angle1;
                            largestAngle1 = angle1;

                            // The outer edge of the starting rectangle.
                            otherEdge1_pt1 = face1.get_point(parEdge1(1));
                            otherEdge1_pt2 = face1.get_point((int)
                                                        (parEdge1(1) + 1)%3);

                            // Call recursive method.
                            find_adjacent_rectangles(p, 
                                                     cylMask, 
                                                     rectMask, 
                                                     rectangles, 
                                                     index1, 
                                                     rectMask(adjFace2_1), 
                                                     index2, 
                                                     len_e2, 
                                                     lengthTolerance, 
                                                     smallestAngle1, 
                                                     largestAngle1, 
                                                     sumAngles1, 
                                                     cyl1, 
                                                     edge1_pt1, 
                                                     edge1_pt2, 
                                                     edge1_pt1_adj, 
                                                     edge1_pt2_adj);
                            
                            if(fabs(sumAngles1 - 0.0) < DBL_EPSILON)
                            {
                                valid1 = false; 
                                cyl1.clear();
                            }
                            else
                            {
                                valid1 = true;
                            }

                            if(fabs(sumAngles1 - 2*M_PI) < DBL_EPSILON)
                            {
                                complete1 = true;
                            }
                        }
                    }
                }
            }

            // Save the lists of face indices that form cyl1.
            if(valid1 && sumAngles1 >= minCylinderSize && 
                                            cyl1.size() > minNumFacesPerCyl)
            {
                cyl_indices.push_back(cyl1);
                cylSumAngles.push_back(sumAngles1);
                std::vector<Vector> edgePts;
                edgePts.push_back(edge1_pt1);
                edgePts.push_back(edge1_pt2);
                edgePts.push_back(edge1_pt1_adj);
                edgePts.push_back(edge1_pt2_adj);
                edgePts.push_back(otherEdge1_pt1);
                edgePts.push_back(otherEdge1_pt2);
                cylEdgePoints.push_back(edgePts);

                for(unsigned int c = 0; c < cyl1.size(); c++)
                {
                    cylMask(rectMask(cyl1[c])) = cylNum;
                }
                cylAngles.push_back(angle1);
                cylNum++;

                // Clear cyl2 since index1 and index2 are included in cyl1.
                cyl2.clear();
            }

            // Check if the face adjacent to triangle1 along parallel edge 2 is
            // part of a rectangle.
            int rectAdj_1_2 = rectMask(adjFace1_2);
            if(cyl2.size() == 2 && rectAdj_1_2 > i && cylMask(rectAdj_1_2) < 0)
            {
                Polygon adjFacePoly = p.get_face(adjFace1_2);

                face1.fit_plane(plane1_params);
                adjFacePoly.fit_plane(plane2_params);
                angle2 = get_angle_between_two_vectors(plane1_params, 
                                                       plane2_params);

                if(angle2 > coplanarAngleTolerance && angle2 < maxAngle)
                {
                    // Check width of rectangle i is same as width of adjacent 
                    // triangle.
                    Vector adjParEdge1 = 
                                rectangles[rectAdj_1_2].get_parallel_edges1();
                    Vector adjParEdge2 = 
                                rectangles[rectAdj_1_2].get_parallel_edges2();
                    int adjTriIndex;
                    if(adjFace1_2 == rectangles[rectAdj_1_2].get_triangle1())
                    {
                        adjTriIndex = 0;
                    }
                    else
                    {
                        adjTriIndex = 1;
                    }
                    p1 = adjFacePoly.get_point(adjParEdge1(adjTriIndex));
                    p2 = adjFacePoly.get_point((int)
                                            (adjParEdge1(adjTriIndex)+1)%3);
                    double adjLen_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                            (p1(1)-p2(1))*(p1(1)-p2(1))+
                                            (p1(2)-p2(2))*(p1(2)-p2(2)));
                    p1 = adjFacePoly.get_point(adjParEdge2(adjTriIndex));
                    p2 = adjFacePoly.get_point((int)
                                            (adjParEdge2(adjTriIndex)+1)%3);
                    double adjLen_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                            (p1(1)-p2(1))*(p1(1)-p2(1))+
                                            (p1(2)-p2(2))*(p1(2)-p2(2)));

                    if((fabs(len_e1 - adjLen_e1) < lengthTolerance && 
                        fabs(len_e2 - adjLen_e2) < lengthTolerance) || 
                        (fabs(len_e1 - adjLen_e2) < lengthTolerance && 
                         fabs(len_e2 - adjLen_e1) < lengthTolerance))
                    {
                        cyl2.push_back(adjFace1_2);
                        sumAngles2 += angle2;

                        smallestAngle2 = angle2;
                        largestAngle2 = angle2;

                        otherEdge2_pt1 = face2.get_point(parEdge2(1));
                        otherEdge2_pt2 = face2.get_point((int)
                                                        (parEdge2(1) + 1)%3);

                        // Call recursive method.
                        find_adjacent_rectangles(p, 
                                                 cylMask, 
                                                 rectMask, 
                                                 rectangles, 
                                                 index2, 
                                                 rectMask(adjFace1_2), 
                                                 index1, 
                                                 len_e1, 
                                                 lengthTolerance, 
                                                 smallestAngle2, 
                                                 largestAngle2, 
                                                 sumAngles2, 
                                                 cyl2, 
                                                 edge2_pt1, 
                                                 edge2_pt2, 
                                                 edge2_pt1_adj, 
                                                 edge2_pt2_adj);
                        
                        if(fabs(sumAngles2 - 0.0) < DBL_EPSILON)
                        {
                            sumAngles2 = 0.0;
                            angle2 = 0.0;
                            valid2 = false;
                            cyl2.clear();

                            cyl2.push_back(index1);
                            cyl2.push_back(index2);
                        }
                        else
                        {
                            valid2 = true;
                        }

                        if(fabs(sumAngles2 - 2*M_PI) < DBL_EPSILON)
                        {
                            complete2 = true;
                        }
                    }
                }
            }

            // Check if the face adjacent to triangle2 along parallel edge 2 is
            // part of a rectangle.
            if( (!complete2) && cyl2.size() >= 2)
            {
                int rectAdj_2_2 = rectMask(adjFace2_2);
                if(rectAdj_2_2 > i && cylMask(rectAdj_2_2) < 0)
                {
                    Polygon adjFacePoly = p.get_face(adjFace2_2);

                    face2.fit_plane(plane1_params);
                    adjFacePoly.fit_plane(plane2_params);
                    double angle = get_angle_between_two_vectors(plane1_params,
                                                                 plane2_params);

                    // If valid and the angles are different, then we have two 
                    // separate conjoined cylinders. So save the previous 
                    // cylinder and move on to next rectangle.
                    if( valid2 && (angle2 > coplanarAngleTolerance) && 
                                   angle2 < maxAngle ) 
                    {
                        if((angle > largestAngle2 && 
                           (fabs(angle - smallestAngle2) >= angleTolerance)) || 
                           (angle < smallestAngle2 && 
                           (fabs(largestAngle2 - angle) >= angleTolerance)))
                        {
                            // cyl2 will be saved at end of loop.
                        }
                        else    // The angles are the same and cyl2 is valid, so
                                // continue adding faces to cyl2.
                        {
                            // Check width of rectangle i is same as width of 
                            // adjacent triangle.
                            Vector adjParEdge1 = 
                                rectangles[rectAdj_2_2].get_parallel_edges1();
                            Vector adjParEdge2 = 
                                rectangles[rectAdj_2_2].get_parallel_edges2();
                            int adjTriIndex;
                            if(adjFace2_2 == 
                                    rectangles[rectAdj_2_2].get_triangle1())
                            {
                                adjTriIndex = 0;
                            }
                            else
                            {
                                adjTriIndex = 1;
                            }
                            p1 = adjFacePoly.get_point(
                                            adjParEdge1(adjTriIndex));
                            p2 = adjFacePoly.get_point((int)
                                            (adjParEdge1(adjTriIndex)+1)%3);
                            double adjLen_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                                    (p1(1)-p2(1))*(p1(1)-p2(1))+
                                                   (p1(2)-p2(2))*(p1(2)-p2(2)));
                            p1 = adjFacePoly.get_point(
                                            adjParEdge2(adjTriIndex));
                            p2 = adjFacePoly.get_point((int)
                                            (adjParEdge2(adjTriIndex)+1)%3);
                            double adjLen_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                                    (p1(1)-p2(1))*(p1(1)-p2(1))+
                                                   (p1(2)-p2(2))*(p1(2)-p2(2)));

                            if((fabs(len_e1 - adjLen_e1) < lengthTolerance && 
                                fabs(len_e2 - adjLen_e2) < lengthTolerance) || 
                                (fabs(len_e1 - adjLen_e2) < lengthTolerance && 
                                 fabs(len_e2 - adjLen_e1) < lengthTolerance))
                            {
                                angle2 = angle;

                                cyl2.push_back(adjFace2_2);
                                sumAngles2 += angle2;

                                if(angle2 < smallestAngle2)
                                {
                                    smallestAngle2 = angle2;
                                }
                                if(angle2 > largestAngle2)
                                {
                                    largestAngle2 = angle2;
                                }

                                otherEdge2_pt1 = edge2_pt1;
                                otherEdge2_pt2 = edge2_pt2;

                                // Call recursive method.
                                find_adjacent_rectangles(p, 
                                                         cylMask, 
                                                         rectMask, 
                                                         rectangles, 
                                                         index1, 
                                                         rectMask(adjFace2_2), 
                                                         index2, 
                                                         len_e1, 
                                                         lengthTolerance, 
                                                         smallestAngle2, 
                                                         largestAngle2, 
                                                         sumAngles2, 
                                                         cyl2, 
                                                         edge2_pt1, 
                                                         edge2_pt2, 
                                                         edge2_pt1_adj, 
                                                         edge2_pt2_adj);
                                
                                if(fabs(sumAngles2 - 0.0) < DBL_EPSILON)
                                {
                                    valid2 = false;
                                    cyl2.clear();
                                }
                                else
                                {
                                    valid2 = true;
                                }

                                if(fabs(sumAngles2 - 2*M_PI) < DBL_EPSILON)
                                {
                                    complete2 = true;
                                }
                            }
                        }
                    }
                    
                    // cyl2 only contains index1 and index2.
                    if( (!valid2) && cyl2.size() == 2 &&
                            angle > coplanarAngleTolerance && angle < maxAngle)
                    {
                        // Check width of rectangle i is same as width of 
                        // adjacent triangle.
                        Vector adjParEdge1 = 
                                rectangles[rectAdj_2_2].get_parallel_edges1();
                        Vector adjParEdge2 = 
                                rectangles[rectAdj_2_2].get_parallel_edges2();
                        int adjTriIndex;
                        if(adjFace2_2 ==
                                rectangles[rectAdj_2_2].get_triangle1())
                        {
                            adjTriIndex = 0;
                        }
                        else
                        {
                            adjTriIndex = 1;
                        }
                        p1 = adjFacePoly.get_point(adjParEdge1(adjTriIndex));
                        p2 = adjFacePoly.get_point((int)(
                                            adjParEdge1(adjTriIndex)+1)%3);
                        double adjLen_e1 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                                (p1(1)-p2(1))*(p1(1)-p2(1))+
                                                (p1(2)-p2(2))*(p1(2)-p2(2)));
                        p1 = adjFacePoly.get_point(adjParEdge2(adjTriIndex));
                        p2 = adjFacePoly.get_point((int)(
                                            adjParEdge2(adjTriIndex)+1)%3);
                        double adjLen_e2 = sqrt((p1(0)-p2(0))*(p1(0)-p2(0))+
                                                (p1(1)-p2(1))*(p1(1)-p2(1))+
                                                (p1(2)-p2(2))*(p1(2)-p2(2)));

                        if((fabs(len_e1 - adjLen_e1) < lengthTolerance && 
                            fabs(len_e2 - adjLen_e2) < lengthTolerance) || 
                            (fabs(len_e1 - adjLen_e2) < lengthTolerance && 
                             fabs(len_e2 - adjLen_e1) < lengthTolerance))
                        {
                            angle2 = angle;

                            cyl2.push_back(adjFace2_2);
                            sumAngles2 += angle2;

                            smallestAngle2 = angle2;
                            largestAngle2 = angle2;

                            // The outer edge of the starting rectangle.
                            otherEdge2_pt1 = face1.get_point(parEdge2(0));
                            otherEdge2_pt2 = face1.get_point((int)
                                                        (parEdge2(0) + 1)%3);

                            // Call recursive method.
                            find_adjacent_rectangles(p, 
                                                     cylMask, 
                                                     rectMask, 
                                                     rectangles, 
                                                     index1, 
                                                     rectMask(adjFace2_2), 
                                                     index2, 
                                                     len_e1, 
                                                     lengthTolerance, 
                                                     smallestAngle2, 
                                                     largestAngle2, 
                                                     sumAngles2, 
                                                     cyl2, 
                                                     edge2_pt1, 
                                                     edge2_pt2, 
                                                     edge2_pt1_adj, 
                                                     edge2_pt2_adj);
                            
                            if(fabs(sumAngles2 - 0.0) < DBL_EPSILON)
                            {
                                valid2 = false;
                                cyl2.clear();
                            }
                            else
                            {
                                valid2 = true;
                            }

                            if(fabs(sumAngles2 - 2*M_PI) < DBL_EPSILON)
                            {
                                complete2 = true;
                            }
                        }
                    }
                }
            }

            // Save the lists of face indices that form cyl2.
            if(valid2 && sumAngles2 >= minCylinderSize && 
                    cyl2.size() > minNumFacesPerCyl)
            {
                cyl_indices.push_back(cyl2);
                cylSumAngles.push_back(sumAngles2);
                std::vector<Vector> edgePts;
                edgePts.push_back(edge2_pt1);
                edgePts.push_back(edge2_pt2);
                edgePts.push_back(edge2_pt1_adj);
                edgePts.push_back(edge2_pt2_adj);
                edgePts.push_back(otherEdge2_pt1);
                edgePts.push_back(otherEdge2_pt2);
                cylEdgePoints.push_back(edgePts);

                for(unsigned int c = 0; c < cyl2.size(); c++)
                {
                    cylMask(rectMask(cyl2[c])) = cylNum;
                }
                cylAngles.push_back(angle2);
                cylNum++;
            }
        }
    }
}

/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

// Helper methods for fit_cylinder().

/*
 * Determines which points make up the 'top' of the cylinder and which points 
 * make up the 'bottom' of the cylinder.
 *
 * @param  p  the polymesh
 * @param  rectMask  the Int_vector mask representing which faces are part of 
 *                   rectangles
 * @param  rectangles  a vector of Right_Triangle_Pairs which contains the 
 *                     indices of the faces that form rectangles
 * @param  cyl_indices  a vector containing all of the indices of the faces in 
 *                      a cylinder
 * @param  top_points  a vector containing the vertices that make up one end of
 *                     the cylinder
 * @param  bottom_points  a vector containing the vertices that make up the 
 *                        other end of the cylinder
 */
void kjb::find_top_and_bottom_points_of_cylinder(const Polymesh& p, 
                            const Int_vector& rectMask, 
                            const std::vector<Right_Triangle_Pair>& rectangles, 
                            const std::vector<int>& cyl_indices, 
                            std::vector<Vector>& top_points, 
                            std::vector<Vector>& bottom_points)
{
    // Use the first three triangles to determine the top and bottom.
    Polygon tri0 = p.get_face(cyl_indices[0]);
    Polygon tri1 = p.get_face(cyl_indices[1]);
    Polygon tri2 = p.get_face(cyl_indices[2]);

    bool found_adj = false;
    unsigned int adj_face_to_tri2;  // Index of face that tri2 is adjacent to
    unsigned int adj_edge_index;    // Other face's edge index that tri2 is 
                                    // adjacent to
    for(int i = 0; i < 3; i++)
    {
        if((int)(p.adjacent_face(cyl_indices[0], i)) == cyl_indices[2])
        {
            found_adj = true;
            adj_face_to_tri2 = cyl_indices[0];
            adj_edge_index = i;
            break;
        }

        if((int)(p.adjacent_face(cyl_indices[1], i)) == cyl_indices[2])
        {
            found_adj = true;
            adj_face_to_tri2 = cyl_indices[1];
            adj_edge_index = i;
            break;
        }
    }

    if(found_adj == false)
    {
        KJB_error("ERROR: first two triangles in cylinder are not adjacent to third!\n");
    }

    int rectIndex = rectMask(cyl_indices[0]);
    Vector parEdges1 = rectangles[rectIndex].get_parallel_edges1();
    Vector parEdges2 = rectangles[rectIndex].get_parallel_edges2();

    int tri0_index;
    int tri1_index;

    // Determine which face is tri0 and which is tri1 in the rectangle
    if(rectangles[rectIndex].get_triangle1() == cyl_indices[1])
    {
        tri0_index = 1;
        tri1_index = 0;
    }
    else if(rectangles[rectIndex].get_triangle2() == cyl_indices[1])
    {
        tri0_index = 0;
        tri1_index = 1;
    }
    else
    {
        throw KJB_error("ERROR: the first two triangles in the cylinder do not form a rectangle!\n");
    }

    // Determine which set of parallel edges the third face (tri2) is adjacent 
    // to.
    if((int)adj_face_to_tri2 == cyl_indices[0])
    {
        if(parEdges1(tri0_index) == adj_edge_index)
        {
            top_points.push_back(tri0.get_edge_first_vertex(
                                                        parEdges2(tri0_index)));
            top_points.push_back(tri0.get_edge_second_vertex(
                                                        parEdges2(tri0_index)));

            bottom_points.push_back(tri1.get_edge_first_vertex(
                                                        parEdges2(tri1_index)));
            bottom_points.push_back(tri1.get_edge_second_vertex(
                                                        parEdges2(tri1_index)));
        }
        else if(parEdges2(tri0_index) == adj_edge_index)
        {
            top_points.push_back(tri0.get_edge_first_vertex(
                                                        parEdges1(tri0_index)));
            top_points.push_back(tri0.get_edge_second_vertex(
                                                        parEdges1(tri0_index)));

            bottom_points.push_back(tri1.get_edge_first_vertex(
                                                        parEdges1(tri1_index)));
            bottom_points.push_back(tri1.get_edge_second_vertex(
                                                        parEdges1(tri1_index)));
        }
        else
        {
            throw KJB_error("ERROR: adjacent edge doesn't match\n");
        }
    }
    else // adj_face_to_tri2 == cyl_indices[1]
    {
        if(parEdges1(tri1_index) == adj_edge_index)
        {
            top_points.push_back(tri0.get_edge_first_vertex(
                                                        parEdges2(tri0_index)));
            top_points.push_back(tri0.get_edge_second_vertex(
                                                        parEdges2(tri0_index)));

            bottom_points.push_back(tri1.get_edge_first_vertex(
                                                        parEdges2(tri1_index)));
            bottom_points.push_back(tri1.get_edge_second_vertex(
                                                        parEdges2(tri1_index)));
        }
        else if(parEdges2(tri1_index) == adj_edge_index)
        {
            top_points.push_back(tri0.get_edge_first_vertex(
                                                        parEdges1(tri0_index)));
            top_points.push_back(tri0.get_edge_second_vertex(
                                                        parEdges1(tri0_index)));

            bottom_points.push_back(tri1.get_edge_first_vertex(
                                                        parEdges1(tri1_index)));
            bottom_points.push_back(tri1.get_edge_second_vertex(
                                                        parEdges1(tri1_index)));
        }
        else
        {
            throw KJB_error("ERROR: adjacent edge doesn't match\n");
        }
    }

    // The first two points for the top and bottom sets have been found.
    // Now need to get the adjacent rectangles and compare the new points to the
    // existing ones.

    // I'm assuming that the triangles making up a rectangle are next to 
    // eachother in the cyl_indices list.

    for(unsigned int i = 2; i < cyl_indices.size(); i+=2)
    {
        rectIndex = rectMask(cyl_indices[i]);

        // Double check that the next cylinder index is in the same rectangle.
        if(rectangles[rectIndex].get_triangle1() != cyl_indices[i+1] && 
           rectangles[rectIndex].get_triangle2() != cyl_indices[i+1])
        {
            throw KJB_error("ERROR: the two consecutive face indices are not part of the same rectangle\n");
        }

        tri0 = p.get_face(cyl_indices[i]);
        tri1 = p.get_face(cyl_indices[i+1]);

        parEdges1 = rectangles[rectIndex].get_parallel_edges1();
        parEdges2 = rectangles[rectIndex].get_parallel_edges2();

        int tri0_index;
        int tri1_index;

        // Determine which face is first and which is second in the rectangle.
        if(rectangles[rectIndex].get_triangle1() == cyl_indices[i])
        {
            tri0_index = 0;
            tri1_index = 1;
        }
        else if(rectangles[rectIndex].get_triangle2() == cyl_indices[i])
        {
            tri0_index = 1;
            tri1_index = 0;
        }
        else
        {
            throw KJB_error("ERROR: problem with rectMask\n");
        }

        Vector tri0_edge1_vertex1 = 
                            tri0.get_edge_first_vertex(parEdges1(tri0_index));
        Vector tri0_edge1_vertex2 = 
                            tri0.get_edge_second_vertex(parEdges1(tri0_index));
        Vector tri0_edge2_vertex1 = 
                            tri0.get_edge_first_vertex(parEdges2(tri0_index));
        Vector tri0_edge2_vertex2 = 
                            tri0.get_edge_second_vertex(parEdges2(tri0_index));
        Vector tri1_edge1_vertex1 = 
                            tri1.get_edge_first_vertex(parEdges1(tri1_index));
        Vector tri1_edge1_vertex2 = 
                            tri1.get_edge_second_vertex(parEdges1(tri1_index));
        Vector tri1_edge2_vertex1 = 
                            tri1.get_edge_first_vertex(parEdges2(tri1_index));
        Vector tri1_edge2_vertex2 = 
                            tri1.get_edge_second_vertex(parEdges2(tri1_index));

        // Compare points to the ones already in the top and bottom sets to 
        // determine where to add the next points.
        // Check if come full circle and all of the points are already included.
        if( (std::find(top_points.begin(), 
                       top_points.end(), 
                       tri0_edge1_vertex1) != top_points.end() || 
             std::find(bottom_points.begin(), 
                       bottom_points.end(), 
                       tri0_edge1_vertex1) != bottom_points.end()
            ) && 
            (std::find(top_points.begin(), 
                       top_points.end(), 
                       tri0_edge1_vertex2) != top_points.end() || 
             std::find(bottom_points.begin(), 
                       bottom_points.end(), 
                       tri0_edge1_vertex2) != bottom_points.end()
            ) && 
            (std::find(top_points.begin(), 
                       top_points.end(), 
                       tri0_edge2_vertex1) != top_points.end() || 
             std::find(bottom_points.begin(), 
                       bottom_points.end(), 
                       tri0_edge2_vertex1) != bottom_points.end()
            ) && 
            (std::find(top_points.begin(), 
                       top_points.end(), 
                       tri0_edge2_vertex2) != top_points.end() || 
             std::find(bottom_points.begin(), 
                       bottom_points.end(), 
                       tri0_edge2_vertex2) != bottom_points.end()
            ) && 
            (std::find(top_points.begin(), 
                       top_points.end(), 
                       tri1_edge1_vertex1) != top_points.end() || 
             std::find(bottom_points.begin(), 
                       bottom_points.end(), 
                       tri1_edge1_vertex1) != bottom_points.end()
            ) && 
            (std::find(top_points.begin(), 
                       top_points.end(), 
                       tri1_edge1_vertex2) != top_points.end() || 
             std::find(bottom_points.begin(), 
                       bottom_points.end(), 
                       tri1_edge1_vertex2) != bottom_points.end()
            ) && 
            (std::find(top_points.begin(), 
                       top_points.end(), 
                       tri1_edge2_vertex1) != top_points.end() || 
             std::find(bottom_points.begin(), 
                       bottom_points.end(), 
                       tri1_edge2_vertex1) != bottom_points.end()
            ) && 
            (std::find(top_points.begin(), 
                       top_points.end(), 
                       tri1_edge2_vertex2) != top_points.end() || 
             std::find(bottom_points.begin(), 
                       bottom_points.end(), 
                       tri1_edge2_vertex2) != bottom_points.end()) )
        {
            continue;
        }
        // tri0 on top, tri1 on bottom - parallel edges 1
        else if( (std::find(top_points.begin(), 
                            top_points.end(), 
                            tri0_edge1_vertex1) != top_points.end() || 
                  std::find(top_points.begin(), 
                            top_points.end(), 
                            tri0_edge1_vertex2) != top_points.end()
                 ) && 
                 (std::find(bottom_points.begin(), 
                            bottom_points.end(), 
                            tri1_edge1_vertex1) != bottom_points.end() || 
                  std::find(bottom_points.begin(), 
                            bottom_points.end(), 
                            tri1_edge1_vertex2) != bottom_points.end()) )
        {
            if(std::find(top_points.begin(), 
                         top_points.end(), 
                         tri0_edge1_vertex1) == top_points.end())
            {
                top_points.push_back(tri0_edge1_vertex1);
            }
            else
            {
                top_points.push_back(tri0_edge1_vertex2);
            }

            if(std::find(bottom_points.begin(), 
                         bottom_points.end(), 
                         tri1_edge1_vertex1) == bottom_points.end())
            {
                bottom_points.push_back(tri1_edge1_vertex1);
            }
            else
            {
                bottom_points.push_back(tri1_edge1_vertex2);
            }
        }
        // tri0 on top, tri1 on bottom - parallel edges 2
        else if( (std::find(top_points.begin(), 
                            top_points.end(), 
                            tri0_edge2_vertex1) != top_points.end() || 
                  std::find(top_points.begin(), 
                            top_points.end(), 
                            tri0_edge2_vertex2) != top_points.end()
                 ) && 
                 (std::find(bottom_points.begin(), 
                            bottom_points.end(), 
                            tri1_edge2_vertex1) != bottom_points.end() || 
                  std::find(bottom_points.begin(), 
                            bottom_points.end(), 
                            tri1_edge2_vertex2) != bottom_points.end()) )
        {
            if(std::find(top_points.begin(), 
                         top_points.end(), 
                         tri0_edge2_vertex1) == top_points.end())
            {
                top_points.push_back(tri0_edge2_vertex1);
            }
            else
            {
                top_points.push_back(tri0_edge2_vertex2);
            }

            if(std::find(bottom_points.begin(), 
                         bottom_points.end(), 
                         tri1_edge2_vertex1) == bottom_points.end())
            {
                bottom_points.push_back(tri1_edge2_vertex1);
            }
            else
            {
                bottom_points.push_back(tri1_edge2_vertex2);
            }
        }
        // tri0 on bottom, tri1 on top - parallel edges 1
        else if( (std::find(bottom_points.begin(), 
                            bottom_points.end(), 
                            tri0_edge1_vertex1) != bottom_points.end() || 
                  std::find(bottom_points.begin(), 
                            bottom_points.end(), 
                            tri0_edge1_vertex2) != bottom_points.end()
                 ) && 
                 (std::find(top_points.begin(), 
                            top_points.end(), 
                            tri1_edge1_vertex1) != top_points.end() || 
                  std::find(top_points.begin(), 
                            top_points.end(), 
                            tri1_edge1_vertex2) != top_points.end()) )
        {
            if(std::find(bottom_points.begin(), 
                         bottom_points.end(), 
                         tri0_edge1_vertex1) == bottom_points.end())
            {
                bottom_points.push_back(tri0_edge1_vertex1);
            }
            else
            {
                bottom_points.push_back(tri0_edge1_vertex2);
            }

            if(std::find(top_points.begin(), 
                         top_points.end(), 
                         tri1_edge1_vertex1) == top_points.end())
            {
                top_points.push_back(tri1_edge1_vertex1);
            }
            else
            {
                top_points.push_back(tri1_edge1_vertex2);
            }
        }
        // tri0 on bottom, tri1 on top - parallel edges 2
        else if( (std::find(bottom_points.begin(), 
                            bottom_points.end(), 
                            tri0_edge2_vertex1) != bottom_points.end() || 
                  std::find(bottom_points.begin(), 
                            bottom_points.end(), 
                            tri0_edge2_vertex2) != bottom_points.end()
                 ) && 
                 (std::find(top_points.begin(), 
                            top_points.end(), 
                            tri1_edge2_vertex1) != top_points.end() || 
                  std::find(top_points.begin(), 
                            top_points.end(), 
                            tri1_edge2_vertex2) != top_points.end()) )
        {
            if(std::find(bottom_points.begin(), 
                         bottom_points.end(), 
                         tri0_edge2_vertex1) == bottom_points.end())
            {
                bottom_points.push_back(tri0_edge2_vertex1);
            }
            else
            {
                bottom_points.push_back(tri0_edge2_vertex2);
            }

            if(std::find(top_points.begin(), 
                         top_points.end(), 
                         tri1_edge2_vertex1) == top_points.end())
            {
                top_points.push_back(tri1_edge2_vertex1);
            }
            else
            { 
                top_points.push_back(tri1_edge2_vertex2);
            }
        }
        else
        {
            throw KJB_error("Error: problem with logic of find_top_and_bottom_points_of_cylinder()\n");
        }
    }
    
    if(top_points[0] < bottom_points[0])    // Switch top and bottom.
    {
        Vector tmp;
        for(unsigned int i = 0; i < top_points.size(); i++)
        {
            tmp = top_points[i];
            top_points[i] = bottom_points[i];
            bottom_points[i] = tmp;
        }
    }
}


/*
 * Finds the center of a set of points.
 *
 * @param  points  A vector containing a set of 3D points
 * @param  centroid  A Vector representing the center point of the set of points
 */
void kjb::find_centroid_of_3d_points
(
    const std::vector<Vector>& points, 
    Vector& centroid
)
{
    centroid.resize(4);

    int num_points = points.size();

    // Check that there is at least one point.
    if(num_points <= 0)
    {
        throw KJB_error("Error: can't find the centroid - there are no points in the vector\n");
        return;
    }

    double sum_x = 0;
    double sum_y = 0;
    double sum_z = 0;

    for(int i = 0; i < num_points; i++)
    {
        sum_x += points[i](0);
        sum_y += points[i](1);
        sum_z += points[i](2);
    }

    centroid(0) = sum_x / num_points;
    centroid(1) = sum_y / num_points;
    centroid(2) = sum_z / num_points;
    centroid(3) = 1;
}


/*
 * Fits a plane to 3d points using orthogonal distance regression (i.e. 
 * orthogonal least squares).
 *
 * The centroid of the data points is a point on the best-fit plane and a vector
 * normal to the plane is the singular vector of matrix M corresponding to M's 
 * smallest singular value. Performing singular value decomposition (svd) on M 
 * gives M = U*S*V^T where U, S, and V are matrices.  The diagonal entries of S
 * are the singular values of M (in descending order). The columns in V are the
 * singular vectors corresponding to the singular values.
 *
 * @param  points        A vector containing the data points to fit a plane to.
 *                       Points are of the form (x,y,z).
 * @param  centroid      A Vector representing the center point of the data.
 * @param  plane_params  Coefficients of the orthogonal distance regression 
 *                       plane (of the form ax + by + cz + d = 0) fitted to the
 *                       data.
 */
void kjb::fit_plane_to_3d_points
(
    const std::vector<Vector>& points, 
    const Vector& centroid, 
    Vector& plane_params
)
{
    // Create the matrix to perform singular value decomposition (svd) on.
    int num_points = points.size();

    Matrix M(num_points, 3);
    
    for(int i = 0; i < M.get_num_rows(); i++)
    {
        for(int j = 0; j < M.get_num_cols(); j++)
        {
            M(i,j) = points[i](j) - centroid(j);
        }
    }

    // Now perform svd on matrix M to get the singular vector of M
    // corresponding to its smallest singular value.
    kjb_c::Vector* S_c = NULL;
    kjb_c::Matrix* V_trans_c = NULL;

    const kjb_c::Matrix* M_c = M.get_c_matrix();

    kjb_c::do_svd(M_c, NULL, &S_c, &V_trans_c, NULL);

    Vector S(S_c);
    Matrix V_trans(V_trans_c);

    // Get the singular vector corresponding to the smallest singular value.
    // The smallest value will be the last element of S.
    int index_smallest_value = S.size() - 1;

    // The corresponding singular vector will be the row in V_trans at that 
    // index.   
    Vector normal = V_trans.get_row(index_smallest_value);

    // Get the plane parameters.
    get_plane_parameters(normal, centroid, plane_params);
}


/*
 * Projects 3d points onto the best-fit 3d plane.
 *
 * @param  points  A vector containing a set of 3D points of the form (x,y,z)
 * @param  plane_params  Coefficients of the best-fit plane (of the form 
 *                       ax + by + cz + d = 0) fitted to the set of points
 * @param  centroid  A Vector representing the center point of the set of points
 * @param  projected_points  The set of points after they have been projected 
 *                           onto the best-fit plane
 */
void kjb::project_points_onto_plane
(
    std::vector<Vector>& points, 
    const Vector& plane_params, 
    const Vector& centroid, 
    std::vector<Vector>& projected_points
)
{
    projected_points.clear();

    // Check that plane_params has size = 4.
    if(plane_params.get_length() != 4)
    {
        throw KJB_error("The input plane_params does not contain 4 values\n");
    }

    // Create unit normal vector to plane
    Vector normal(plane_params(0), plane_params(1), plane_params(2));

    // Check that the length of the normal vector is 1.
    double norm_magnitude = sqrt(normal(0)*normal(0) + 
                                 normal(1)*normal(1) + 
                                 normal(2)*normal(2));

    if(norm_magnitude != 1)
    {
        // Normalize the normal so that it is a unit vector.
        normal(0) = normal(0)/norm_magnitude;
        normal(1) = normal(1)/norm_magnitude;
        normal(2) = normal(2)/norm_magnitude;
    }

    // Project each point onto the given plane.
    for(unsigned int i = 0; i < points.size(); i++)
    {
        // Get the vector from a point on the plane (centroid) to the point.
        Vector point_vec = points[i] - centroid;

        // Get the distance from the point to the plane by projecting point_vec
        // onto the normal.
        double distance = ((normal(0)*point_vec(0) + 
                            normal(1)*point_vec(1) + 
                            normal(2)*point_vec(2)));

        if(distance < 0)
        {
            normal = -1 * normal;
        }
        distance = fabs(distance);

        // Create a vector of length D by multiplying the unit normal by the 
        // distance.
        Vector Q = distance * normal;

        // Subtract modified normal Q from the point to get the projected point.
        points[i] /= points[i](3);

        Vector new_pt(4, 1.0);
        for(int j = 0; j < Q.size(); j++)
        {
            new_pt(j) = points[i](j) - Q(j);
        }

        // Store the projected point.
        projected_points.push_back(new_pt);
    }
}


/*
 * Maps a 3D plane to the X-Y plane.
 *
 * @param  points  A vector of points on the 3D plane to be mapped to the X-Y 
 *                 plane
 * @param  plane_params  Coefficients of the 3D plane (of the form 
 *                       ax + by + cz + d = 0) 
 * @param  translated_points  The set of points after they have been mapped to 
 *                            the X-Y plane
 * @param  transformMatrices  A vector of the matrices used to map the points 
 *                            from the 3D plane to the X-Y plane
 */
void kjb::translate_3d_plane_to_xy_plane
(
    const std::vector<Vector>& points, 
    const Vector& plane_params, 
    std::vector<Vector>& translated_points, 
    std::vector<Matrix>& transformMatrices
)
{
    translated_points.clear();
    transformMatrices.clear();

    // Check that plane_params has size = 4.
    if(plane_params.get_length() != 4)
    {
        throw KJB_error("The input plane_params does not contain 4 values\n");
    }

    // Create normal vector with homogeneous coordinate.
    Vector normal(plane_params(0), plane_params(1), plane_params(2), 1);

    double norm_magnitude = sqrt(normal(0)*normal(0) + 
                                 normal(1)*normal(1) + 
                                 normal(2)*normal(2));

    if(norm_magnitude != 1)
    {
        // Normalize the normal so that it is a unit vector.
        normal(0) = normal(0)/norm_magnitude;
        normal(1) = normal(1)/norm_magnitude;
        normal(2) = normal(2)/norm_magnitude;
    }

    // Add homogeneous coordinate to all points.
    std::vector<Vector> homo_points;
    for(unsigned int i = 0; i < points.size(); i++)
    {
        Vector tmp(points[i](0), points[i](1), points[i](2), 1);
        homo_points.push_back(tmp);
    }

    // Move a point in the plane to the origin.

    // Create transformation matrix T_p1 that shifts the plane so that the first
    // point is on the origin.
    Matrix T_p1;
    T_p1.convert_to_3d_homo_translation_matrix(-1*points[0](0), 
                                               -1*points[0](1), 
                                               -1*points[0](2));

    // Check if the normal is parallel to the z-axis.
    // If it is, then we just need to move the points so that one of them is on
    // the origin.  If the normal is parallel to z-axis, then it is orthogonal 
    // to the x and y axes.
    if(dot(normal, Vector(1,0,0,0)) == 0 && dot(normal, Vector(0,1,0,0)) == 0)
    {
        for(unsigned int i = 0; i < homo_points.size(); i++)
        {
            try
            {
                Matrix m_vec = T_p1 * homo_points[i];
                Vector pt = m_vec.get_col(0);
                translated_points.push_back(Vector(pt(0), pt(1)));

                // double-check that the z-value is zero.
                if(pt(2) != 0)
                {
                    throw KJB_error("Problem with transformation: z-value didn't become 0\n");
                }
            }
            catch(KJB_error)
            {
                throw KJB_error("Error in matrix multiplication: sizes not compatible");
            }
        }
        transformMatrices.push_back(T_p1); 
    }
    else
    {
        // Angle to rotate around the y-axis to move the vector into the 
        // yz-plane
        double angle_y;
        // Rotate the vector if it is not already in the yz-plane
        if(normal(0) != 0)
        {
            angle_y = acos(normal(2) / sqrt(normal(0)*normal(0) + 
                                            normal(2)*normal(2)));
        }
        else
        {
            angle_y = 0;
        }
        
        Matrix T_yz(4,4,0.0);
        if(normal(0) < 0.0)
        {
            angle_y = -1.0 * angle_y;
        }
        T_yz(0,0) = cos(angle_y);
        T_yz(0,2) = -1 * sin(angle_y);
        T_yz(1,1) = 1.0;
        T_yz(2,0) = sin(angle_y);
        T_yz(2,2) = cos(angle_y);
        T_yz(3,3) = 1.0;

        Vector mod_normal = T_yz * normal;

        // Angle to rotate around the x-axis to move the vector onto the z-axis
        double angle_x = acos(mod_normal(2)/sqrt(mod_normal(0)*mod_normal(0) +
                                                 mod_normal(1)*mod_normal(1) +
                                                 mod_normal(2)*mod_normal(2)));
         
        if(mod_normal(1) > 0.0)
        {
            angle_x = -1.0 * angle_x;
        }
        Matrix T_z(4,4,0.0);
        T_z(0,0) = 1.0;
        T_z(1,1) = cos(angle_x);
        T_z(1,2) = sin(angle_x);
        T_z(2,1) = -1 * sin(angle_x);
        T_z(2,2) = cos(angle_x);
        T_z(3,3) = 1.0;

        // Temporary vector to hold modified points.
        std::vector<Vector> temp_pts;

        // For each point, multiply by T_p1 to get new point.
        for(unsigned int i = 0; i < points.size(); i++)
        {
            try
            {
                Matrix m_vec = T_z * T_yz * T_p1 * homo_points[i];
                Vector pt = m_vec.get_col(0);
                translated_points.push_back(Vector(pt(0), pt(1)));
                
                // double-check that the z-value is zero.
                double tolerance = 1e-8;
                if(fabs(pt(2)) > tolerance)
                {
                    throw KJB_error("Problem with transformation: z-value didn't become 0\n");
                }
            }
            catch(KJB_error)
            {
                throw KJB_error("Error in matrix multiplication: sizes not compatible");
            }
        }
        transformMatrices.push_back(T_z);
        transformMatrices.push_back(T_yz);
        transformMatrices.push_back(T_p1);
    }
}


/*
 * Maps a point on the X-Y plane back onto the original 3D plane using the 
 * transform matrices from translate_3d_plane_to_xy_plane().
 *
 * @param  point  A 2D vector representing the center of one of the bases of the
 *                cylinder.  This point is to be mapped back to the 3D plane.
 * @param  transformMatrices  A vector of the matrices needed to map the point 
 *                            back to the 3D plane.
 * @param  translated_point  A vector representing the point after it has been 
 *                           mapped to the 3D plane.
 */
void kjb::translate_xy_point_to_3d_plane
(
    const Vector& point, 
    const std::vector<Matrix>& transformMatrices, 
    Vector& translated_point
)
{
    std::vector<Matrix> inverseMatrices;
    // Get inverse of transformation matrices.
    for(unsigned int i = 0; i < transformMatrices.size(); i++)
    {
        Matrix T_inverse = matrix_inverse(transformMatrices[i]);
        inverseMatrices.push_back(T_inverse);
    }

    // Add z and homogeneous coordinate to translated_point 2D point.
    Vector point_mod(4, 0.0);
    point_mod(0) = point(0);
    point_mod(1) = point(1);
    point_mod(3) = 1.0;

    Matrix m;
    if(inverseMatrices.size() > 0)
    {
        m = inverseMatrices[0] * point_mod;
    }
    else
    {
        throw KJB_error("Error: no inverse matrices\n");
    }

    if(inverseMatrices.size() == 3)
    {
        m = inverseMatrices[2] * inverseMatrices[1] * m;
    }

    translated_point = m.get_col(0);
}


/*
 * Determines the parameters of the cylinder formed by a set of faces in the 
 * polymesh.  Note: This method only takes in one cylinder at a time, so it 
 * needs to be called for each cylinder in the polymesh.
 *
 * @param  p  the polymesh
 * @param  rectMask  the Int_vector mask representing which faces are part of 
 *                   rectangles
 * @param  rectangles  a vector of Right_Triangle_Pairs which contains the 
 *                     indices of the faces that form rectangles
 * @param  cyl_indices  a vector containing all of the indices of the faces in 
 *                      a cylinder
 * @param  cylAngle  a double representing the interior angle of the cylinder 
 *                   (i.e. shows how much of the cylinder is there)
 * @param  cylEdgePoints  a vector containing 6 points where the first two 
 *                        points are on 1 edge of each cylinder, the following 
 *                        two points are adjacent to the edge points, and the 
 *                        last two points are on the opposite edge from the 
 *                        first two points.
 * @param  cyl  A Cylinder containing the parameters of the cylinder formed by 
 *              the faces in cyl_indices
 */
void kjb::fit_cylinder
(
    const Polymesh& p, 
    const Int_vector& rectMask, 
    const std::vector<Right_Triangle_Pair>& rectangles, 
    const std::vector<int>& cyl_indices, 
    double cylAngle, 
    const std::vector<Vector>& cylEdgePoints, 
    Cylinder_section& cyl
)
{
    // Determine which points belong to the top of the cylinder and which points
    // belong to the bottom.
    std::vector<Vector> top_points;
    std::vector<Vector> bottom_points;

    find_top_and_bottom_points_of_cylinder(p, rectMask, rectangles, cyl_indices,
                                           top_points, bottom_points);

    // Determine which edge point is located on the bottom of the cylinder, 
    // which adjacent point is on the bottom of the cylinder and which otherEdge
    // point is on the bottom of the cylinder.
    Vector bottomEdgePt(4, 1.0);
    Vector bottomEdgePt_adj(4, 1.0);
    Vector bottomOtherEdgePt(4, 1.0);
    int found1 = 0;
    int found2 = 0;
    int found3 = 0;
    for(unsigned int i = 0; i < bottom_points.size(); i++)
    {
        if(found1 == 0)
        {
            // Find bottom edge point.
            if(bottom_points[i](0) == cylEdgePoints[0](0) && 
               bottom_points[i](1) == cylEdgePoints[0](1) && 
               bottom_points[i](2) == cylEdgePoints[0](2))
            {
                bottomEdgePt(0) = cylEdgePoints[0](0);
                bottomEdgePt(1) = cylEdgePoints[0](1);
                bottomEdgePt(2) = cylEdgePoints[0](2);

                found1 = 1;
            }
            else if(bottom_points[i](0) == cylEdgePoints[1](0) && 
                    bottom_points[i](1) == cylEdgePoints[1](1) && 
                    bottom_points[i](2) == cylEdgePoints[1](2))
            {
                bottomEdgePt(0) = cylEdgePoints[1](0);
                bottomEdgePt(1) = cylEdgePoints[1](1);
                bottomEdgePt(2) = cylEdgePoints[1](2);

                found1 = 1;
            }
            else
            {
                // Go to next iteration.
            }
        }

        if(found2 == 0)
        {
            // Find adjacent bottom point.
            if(bottom_points[i](0) == cylEdgePoints[2](0) && 
               bottom_points[i](1) == cylEdgePoints[2](1) && 
               bottom_points[i](2) == cylEdgePoints[2](2))
            {
                bottomEdgePt_adj(0) = cylEdgePoints[2](0);
                bottomEdgePt_adj(1) = cylEdgePoints[2](1);
                bottomEdgePt_adj(2) = cylEdgePoints[2](2);

                found2 = 1;
            }
            else if(bottom_points[i](0) == cylEdgePoints[3](0) && 
                    bottom_points[i](1) == cylEdgePoints[3](1) && 
                    bottom_points[i](2) == cylEdgePoints[3](2))
            {
                bottomEdgePt_adj(0) = cylEdgePoints[3](0);
                bottomEdgePt_adj(1) = cylEdgePoints[3](1);
                bottomEdgePt_adj(2) = cylEdgePoints[3](2);

                found2 = 1;
            }
            else
            {
                // Go to next iteration.
            }
        }
        
        if(found3 == 0)
        {
            // Find bottom otherEdge point.
            if(bottom_points[i](0) == cylEdgePoints[4](0) && 
               bottom_points[i](1) == cylEdgePoints[4](1) && 
               bottom_points[i](2) == cylEdgePoints[4](2))
            {
                bottomOtherEdgePt(0) = cylEdgePoints[4](0);
                bottomOtherEdgePt(1) = cylEdgePoints[4](1);
                bottomOtherEdgePt(2) = cylEdgePoints[4](2);

                found3 = 1;
            }
            else if(bottom_points[i](0) == cylEdgePoints[5](0) && 
                    bottom_points[i](1) == cylEdgePoints[5](1) && 
                    bottom_points[i](2) == cylEdgePoints[5](2))
            {
                bottomOtherEdgePt(0) = cylEdgePoints[5](0);
                bottomOtherEdgePt(1) = cylEdgePoints[5](1);
                bottomOtherEdgePt(2) = cylEdgePoints[5](2);

                found3 = 1;
            }
            else
            {
                // Go to next iteration.
            }
        }

        // if all three points have been found, leave for-loop.
        if(found1 == 1 && found2 == 1 && found3 == 1)
        {
            i = bottom_points.size();
        }
    }

    if(found1 == 0 || found2 == 0 || found3 == 0)
    {
        // Not all points found on bottom.

        for(unsigned int i = 0; i < top_points.size(); i++)
        {
            if(found3 == 0)
            {
                // Find bottom otherEdge point.
                if(top_points[i](0) == cylEdgePoints[4](0) && 
                   top_points[i](1) == cylEdgePoints[4](1) && 
                   top_points[i](2) == cylEdgePoints[4](2))
                {
                    bottomOtherEdgePt(0) = cylEdgePoints[5](0);
                    bottomOtherEdgePt(1) = cylEdgePoints[5](1);
                    bottomOtherEdgePt(2) = cylEdgePoints[5](2);

                    found3 = 1;
                }
                else if(top_points[i](0) == cylEdgePoints[5](0) && 
                        top_points[i](1) == cylEdgePoints[5](1) && 
                        top_points[i](2) == cylEdgePoints[5](2))
                {
                    bottomOtherEdgePt(0) = cylEdgePoints[4](0);
                    bottomOtherEdgePt(1) = cylEdgePoints[4](1);
                    bottomOtherEdgePt(2) = cylEdgePoints[4](2);

                    found3 = 1;
                }
                else
                {
                    // Go to next iteration.
                }
            }
        }

        if(found1 == 0 || found2 == 0 || found3 == 0)
        {
            // Top_point not found either
        }
        else
        {
            // FOUND
        }
    }

    Vector top_centroid;
    Vector bottom_centroid;

    find_centroid_of_3d_points(top_points, top_centroid);
    find_centroid_of_3d_points(bottom_points, bottom_centroid);

    Vector top_plane_params;
    Vector bottom_plane_params;

    fit_plane_to_3d_points(top_points, top_centroid, top_plane_params);
    fit_plane_to_3d_points(bottom_points, bottom_centroid, bottom_plane_params);

    std::vector<Vector> top_projected_points;
    std::vector<Vector> bottom_projected_points;

    project_points_onto_plane(top_points, top_plane_params, top_centroid, 
                              top_projected_points);
    project_points_onto_plane(bottom_points, bottom_plane_params, 
                              bottom_centroid, bottom_projected_points);

    // Project bottomEdgePt onto the bottom plane
    std::vector<Vector> bottom_edgePt_vec;
    bottom_edgePt_vec.push_back(bottomEdgePt);
    std::vector<Vector> bottom_projected_edgePt;
    project_points_onto_plane(bottom_edgePt_vec, bottom_plane_params, 
                              bottom_centroid, bottom_projected_edgePt);

    // Project bottomEdgePt_adj onto the bottom plane
    std::vector<Vector> bottom_edgePt_adj_vec;
    bottom_edgePt_adj_vec.push_back(bottomEdgePt_adj);
    std::vector<Vector> bottom_projected_edgePt_adj;
    project_points_onto_plane(bottom_edgePt_adj_vec, bottom_plane_params, 
                              bottom_centroid, bottom_projected_edgePt_adj);

    // Project bottomOtherEdgePt onto the bottom plane
    std::vector<Vector> bottom_otherEdgePt_vec;
    bottom_otherEdgePt_vec.push_back(bottomOtherEdgePt);
    std::vector<Vector> bottom_projected_otherEdgePt;
    project_points_onto_plane(bottom_otherEdgePt_vec, 
                              bottom_plane_params, bottom_centroid, 
                              bottom_projected_otherEdgePt);

    std::vector<Vector> top_translated_points;
    std::vector<Vector> bottom_translated_points;
    std::vector<Matrix> top_transformMatrices;
    std::vector<Matrix> bottom_transformMatrices;

    translate_3d_plane_to_xy_plane(top_projected_points, 
                                   top_plane_params, 
                                   top_translated_points, 
                                   top_transformMatrices);
    translate_3d_plane_to_xy_plane(bottom_projected_points, 
                                   bottom_plane_params, 
                                   bottom_translated_points, 
                                   bottom_transformMatrices);

    Circle top(top_translated_points);
    Circle bottom(bottom_translated_points);

    Vector center_top = top.GetCenter();
    double radius_top = top.GetRadius();

    Vector center_bottom = bottom.GetCenter();
    double radius_bottom = bottom.GetRadius(); 

    Vector top_translated_center;
    Vector bottom_translated_center;

    translate_xy_point_to_3d_plane(center_top, 
                                   top_transformMatrices, 
                                   top_translated_center);
    translate_xy_point_to_3d_plane(center_bottom, 
                                   bottom_transformMatrices, 
                                   bottom_translated_center);

    // Get the average of the top and bottom radii.
    Vector top_point(3);
    Vector bottom_point(3);
    for(int i = 0; i < 3; i++)
    {
        top_point(i) = top_translated_center(i);
        bottom_point(i) = bottom_translated_center(i);
    }

    double radius = (radius_top + radius_bottom) / 2;

    // Compute the angle of the cylinder.
    Vector edge1(3);
    Vector edge2(3);
    for(int i = 0; i < 3; i++)
    {
        edge1(i) = bottom_projected_edgePt[0](i) - bottom_point(i);
        edge2(i) = bottom_projected_otherEdgePt[0](i) - bottom_point(i);
    }
    double tmp_angle = get_angle_between_two_vectors(edge1, edge2);

    if(cylAngle > M_PI)
    {
        cylAngle = (2 * M_PI) - tmp_angle;
    }
    else
    {
        cylAngle = tmp_angle;
    }

    // If cylinder is clockwise from the bottomEdgePt, make the angle negative.
    Vector a(3);
    Vector b(3);
    for(int i = 0; i < 3; i++)
    {
        a(i) = bottom_projected_edgePt[0](i) - bottom_point(i);
        b(i) = bottom_projected_edgePt_adj[0](i) - bottom_point(i);
    }
    // Compute cross product of a and b to get perpendicular vector that follows
    // right-hand rule.
    Vector crossProd(3);
    crossProd(0) = a(1)*b(2) - a(2)*b(1);
    crossProd(1) = a(2)*b(0) - a(0)*b(2);
    crossProd(2) = a(0)*b(1) - a(1)*b(0);
    
    double cp_magnitude = crossProd.magnitude();

    Vector centerVec(3);

    centerVec = top_point - bottom_point;
    double cv_magnitude = centerVec.magnitude();

    // Compute the dot product: 
    //   if dotProd == product of magnitudes, then same direction
    //   if dotProd == negative product of magnitudes, then opposite directions
    double dotProd = crossProd(0)*centerVec(0) + 
                     crossProd(1)*centerVec(1) + 
                     crossProd(2)*centerVec(2);
    double magProd = cp_magnitude * cv_magnitude;
    double diffTolerance = 0.000001;
    if(fabs(dotProd + magProd) < diffTolerance)
    {
        cylAngle = -1 * cylAngle;
    }
    else if(fabs(dotProd - magProd) < diffTolerance)
    {
        // Do nothing.
    }
    else
    {
        std::cout << "ERROR: problem determining direction of cylinder angle, "
                  << "the dot product and magnitude product are not equal!\n";
    }

    // Create cylinder section.
    Cylinder_section cyl_params(top_point, bottom_point, radius, cylAngle, 
                                bottom_projected_edgePt[0], 
                                bottom_projected_otherEdgePt[0]);
    cyl = cyl_params;
}

/*/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void kjb::find_cylinders_to_render_and_fit
(
    char* faceFile, 
    char* cylFile, 
    const Polymesh& p, 
    std::vector<Polymesh_Plane>& plane, 
    std::vector<Cylinder_section>& cyl
)
{
    plane.clear();
    cyl.clear();
    int num_faces = p.num_faces();
    std::vector<Polygon> faces = p.get_faces();

    Int_vector triMask;
    find_right_triangles(faces, triMask);

    std::vector<Right_Triangle_Pair> rectangles;
    find_rectangles(faces, triMask, p, rectangles);

    Int_vector rectMask;
    create_rectangle_mask(p, rectangles, rectMask);

    std::vector< std::vector<int> > cyl_indices;
    std::vector<double> cylSumAngles;
    std::vector< std::vector<Vector> > cylEdgePoints;
    find_cylinders(p, rectangles, cyl_indices, cylSumAngles, cylEdgePoints);

    std::cout << "# cylinders found: " << cyl_indices.size() << std::endl;

    std::ofstream ofs_cyl(cylFile);
    std::ofstream ofs_face(faceFile);

    if(!ofs_cyl.is_open())
    {
        KJB_THROW_2(IO_error, "Could not open file for writing cylinders");
    }
    if(!ofs_face.is_open())
    {
        KJB_THROW_2(IO_error, "Could not open file for writing cylinders");
    }
    
    ofs_cyl << cyl_indices.size() << ' ';
    ofs_face << cyl_indices.size() << ' ';
    for(unsigned int i = 0; i < cyl_indices.size(); i++)
    {
        Cylinder_section cylinder(Vector(0,0,0), Vector(1,0,0), 1, 0, 
                                  Vector(0,0,0,1), Vector(0,0,0,1));
        fit_cylinder(p, rectMask, rectangles, cyl_indices[i], cylSumAngles[i], 
                     cylEdgePoints[i], cylinder);
        cyl.push_back(cylinder);
        
        ofs_face << cyl_indices[i].size() << ' ';
        for(unsigned int j = 0; j < cyl_indices[i].size(); j++)
        {
            ofs_face << cyl_indices[i][j] << ' ';
        }
        ofs_face << '\n';
        cylinder.write(ofs_cyl);
    }
    ofs_face.close();
    ofs_cyl.close();

    std::vector<int> all_indices(num_faces,0);

    for(unsigned int i = 0; i < cyl_indices.size(); i++)
    {
        for(unsigned int j = 0; j < cyl_indices[i].size(); j++)
        {
            all_indices[cyl_indices[i][j]] += 1;
        }

        Vector plane1_params;
        plane.push_back(Polymesh_Plane(p, plane1_params, cyl_indices[i]));
    }

    std::vector<int> indices;
    for(int i = 0; i < num_faces; i++)
    {
        if(all_indices[i] == 0)
        {
            indices.push_back(i);
        }
        else if(all_indices[i] > 1)
        {
            std::cout << "Face " << i << " is in more than 1 cylinder!\n";
        }
        else
        {
            // Do nothing.
        }
    }

    if(indices.size() > 0)
    {
        Vector plane1_params;
        plane.push_back(Polymesh_Plane(p, plane1_params, indices));
    }

    std::cout << "Total planes found: " << plane.size() << std::endl;
}

// Make each cylinder a different color.
void kjb::find_cylinders_to_render
(
    const Polymesh& p, 
    std::vector<Polymesh_Plane>& plane
)
{
    plane.clear();
    int num_faces = p.num_faces();
    std::vector<Polygon> faces = p.get_faces();

    Int_vector mask(1,0);
    find_right_triangles(faces, mask);

    std::vector<Right_Triangle_Pair> rectangles;
    find_rectangles(faces, mask, p, rectangles);

    std::vector< std::vector<int> > cyl_indices;
    std::vector<double> cylSumAngles;
    std::vector< std::vector<Vector> > cylEdgePoints;
    find_cylinders(p, rectangles, cyl_indices, cylSumAngles, cylEdgePoints);

    std::cout << "# cylinders found: " << cyl_indices.size() << std::endl;

    std::vector<int> all_indices(num_faces,0);

    for(unsigned int i = 0; i < cyl_indices.size(); i++)
    {
        for(unsigned int j = 0; j < cyl_indices[i].size(); j++)
        {
            all_indices[cyl_indices[i][j]] += 1;
        }

        Vector plane1_params;
        plane.push_back(Polymesh_Plane(p, plane1_params, cyl_indices[i]));
    }

    std::vector<int> indices;
    for(int i = 0; i < num_faces; i++)
    {
        if(all_indices[i] == 0)
        {
            indices.push_back(i);
        }
        else if(all_indices[i] > 1)
        {
            std::cout << "Face " << i << " is in more than 1 cylinder!\n";
        }
        else
        {
            // Do nothing.
        }
    }

    if(indices.size() > 0)
    {
        Vector plane1_params;
        plane.push_back(Polymesh_Plane(p, plane1_params, indices));
    }

    std::cout << "Total planes found: " << plane.size() << std::endl;
}


// Make each rectangle a different color.
void kjb::find_rectangles_to_render
(
    const Polymesh& p, 
    std::vector<Polymesh_Plane>& plane
)
{
    plane.clear();

    int num_faces = p.num_faces();

    std::vector<int> all_indices(num_faces,0);

    std::vector<Polygon> faces = p.get_faces();


    Int_vector mask(1,0);
    find_right_triangles(faces, mask);

    std::vector<Right_Triangle_Pair> rtp;

    find_rectangles(faces, mask, p, rtp);

    std::cout << "# rectangles found: " << rtp.size() << std::endl;

    for(unsigned int i = 0; i < rtp.size(); i++)
    {
        Vector plane1_params;
        std::vector<int> indices;
        int index1;
        int index2;

        index1 = rtp[i].get_triangle1();
        index2 = rtp[i].get_triangle2();
        
        indices.push_back(index1);
        indices.push_back(index2);
        all_indices[index1] = 1;
        all_indices[index2] = 1;

        plane.push_back(Polymesh_Plane(p, plane1_params, indices));
    }

    std::vector<int> indices;
    for(int i = 0; i < num_faces; i++)
    {
        if(all_indices[i] == 0)
        {
            indices.push_back(i);
        }
    }

    if(indices.size() > 0)
    {
        Vector plane1_params;
        plane.push_back(Polymesh_Plane(p, plane1_params, indices));
    }

    std::cout << "Total planes found: " << plane.size() << std::endl;
}



// Make each right triangle a different color.
void kjb::find_right_triangles_to_render
(
    const Polymesh& p, 
    std::vector<Polymesh_Plane>& plane
)
{
    plane.clear();

    int num_faces = p.num_faces();

    std::vector<int> all_indices(num_faces,0);

    std::vector<Polygon> faces = p.get_faces();

    Int_vector mask(1,0);
    find_right_triangles(faces, mask);

    std::vector<Right_Triangle_Pair> rtp;

    find_rectangles(faces, mask, p, rtp);

    int count = 0;
    Vector plane1_params;
    std::vector<int> other;
    for(int i = 0; i < mask.size(); i++)
    {
        std::vector<int> triangles;
        if(mask(i) == 1)
        {
            triangles.push_back(i);
            count++;
            plane.push_back(Polymesh_Plane(p, plane1_params, triangles));
        }
        else
        {
            other.push_back(i);
        }
    }

    if(other.size() > 0)
    {
        plane.push_back(Polymesh_Plane(p, plane1_params, other));
    }

    std::cout << "# triangles to render: " << count << std::endl;
    std::cout << "Total planes found: " << plane.size() << std::endl;
}


// Make each adjacent right triangle a different color.
void kjb::find_adjacent_right_triangles_to_render
(
    const Polymesh& p, 
    std::vector<Polymesh_Plane>& plane
)
{
    plane.clear();

    int num_faces = p.num_faces();

    std::vector<int> all_indices(num_faces,0);

    std::vector<Polygon> faces = p.get_faces();

    Int_vector mask(1,0);
    find_right_triangles(faces, mask);

    int countAll = 0;
    int countAdj = 0;
    int countRight = 0;

    Vector plane1_params;
    std::vector<int> other;

    for(unsigned int f = 0; f < faces.size(); f++)
    {
        countAll++;

        if(mask(f) == 1 && all_indices[f] == 0)
        {
            countRight++;

            // Find hypotenuse
            int hypotenuse = faces[f].get_index_of_longest_edge();

            // Check if face adjacent to the hypotenuse is a right triangle.
            unsigned int adj_face = p.adjacent_face(f, hypotenuse);

            // if adj_face < f, then we've already checked it.
            if(mask(adj_face) == 1 && all_indices[adj_face] == 0)    
            {
                int hypot2 = faces[adj_face].get_index_of_longest_edge();

                if(adj_face > f && f == p.adjacent_face(adj_face, hypot2)) 
                {
                    std::vector<int> triangle1;

                    countRight++;
                    countAdj++;

                    triangle1.push_back(f);
                    triangle1.push_back(adj_face);

                    plane.push_back(Polymesh_Plane(p, 
                                                   plane1_params, 
                                                   triangle1));

                    if(all_indices[f] != 0)
                    {
                        std::cout << "Face f = " << f 
                                  << " has already been used!" << std::endl;
                    }
                    if(all_indices[adj_face] != 0)
                    {
                        std::cout << "Face adj_face = " << f 
                                  << " has already been used!" << std::endl;
                    }

                    all_indices[f] = 1;
                    all_indices[adj_face] = 1;
                }
            }
        }
    }

    for(unsigned int f = 0; f < faces.size(); f++)
    {
        if(all_indices[f] == 0)
        {
            other.push_back(f);
        }
    }

    if(other.size() > 0)
    {
        plane.push_back(Polymesh_Plane(p, plane1_params, other));
    }

    std::cout << "# triangles found: " << countAll << std::endl;
    std::cout << "# right triangles found: " << countRight << std::endl;
    std::cout << "# adjacent triangles to render: " << countAdj << std::endl;
    std::cout << "Total planes found: " << plane.size() << std::endl;
}

