/* $Id: gr_polygon.cpp 21596 2017-07-30 23:33:36Z kobus $ */

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
* =========================================================================== */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <iostream>
#include <vector>
#include <gr_cpp/gr_polygon.h>
#include <l/l_bits.h>
#include <gr_cpp/gr_camera.h>
#include <gr_cpp/gr_polygon_renderer.h>


using namespace kjb;

/** The constructed polygon has no points. */
Polygon::Polygon()
: Abstract_renderable(), Readable(), Writeable(),
  normal_flipped(false)
  //id(generate_id())
{
}


/** 
 * The constructed polygon has no points, but space is reserved for @em N of
 * them. 
 *
 * @param  N  Number of points that the polygon will contain.
 */
Polygon::Polygon(unsigned int N)
: Abstract_renderable(), Readable(), Writeable(),
  normal_flipped(false)
{
    pts.reserve(N);
}


/** @param  p  Polygon to copy into this one. */
Polygon::Polygon(const Polygon& p)
: Abstract_renderable(p), Readable(), Writeable(),
  pts(p.pts),
  normal(p.normal),
  centroid(p.centroid),
  normal_flipped(p.normal_flipped)
{
}


/**
 * The file format is big-endian binary.
 *
 * @param  fname  Input file to read this polygon from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid arguments in file to read from.
 */
Polygon::Polygon(const char* fname) throw (kjb::Illegal_argument, kjb::IO_error)
: Abstract_renderable(), Readable(), Writeable()
{
    kjb::Readable::read(fname);
}


/**
 * The file format is big-endian binary.
 *
 * @param  in  Input stream to read this polygon from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid arguments in file to read from.
 */
Polygon::Polygon(std::istream& in) throw (kjb::Illegal_argument, kjb::IO_error)
: Abstract_renderable(), Readable(), Writeable()
{
    read(in);
}


/** Frees all space allocated by this polygon. */
Polygon::~Polygon()
{
}


/** 
 * Performs a deep copy of the points in @em p into this polygon. 
 *
 * @param  p  Polygon to copy into this one.
 *
 * @return A reference to this polygon.
 */
Polygon& Polygon::operator= (const Polygon& p)
{
    Abstract_renderable::operator=(p);
    unsigned int n = pts.size();
    unsigned int N = p.pts.size();

    if(n > N)
    {
        pts.erase(pts.begin() + N, pts.end());
    }
    else
    {
        pts.reserve(N);
    }

    for(unsigned int i = 0; i < N; i++)
    {
        if(i < n)
        {
            pts[i] = p.pts[i];
        }
        else
        {
            pts.push_back(p.pts[i]);
        }
    }

    normal = p.normal;
    centroid = p.centroid;
    normal_flipped = p.normal_flipped;
    //id = p.id;

    return *this;
}


/** @return A new copy of this polygon. */
Polygon* Polygon::clone() const
{
    return new Polygon(*this);
}


/** 
 * @param  in  Input stream to read the members of this polygon from.
 *
 * @throw  kjb::IO_error   Could not read from @em in.
 * @throw  kjb::Illegal_argument  Invalid arguments to read from the file.
 */
 void Polygon::read(std::istream& in) throw (kjb::IO_error, kjb::Illegal_argument)
{
    using namespace kjb_c;

    ASSERT(sizeof(double) == sizeof(uint64_t));

    double x, y, z, w;

    // Number of points.
    unsigned int N;

    in.read((char*)&N, sizeof(unsigned int));
    if(in.fail() || in.eof())
    {
        throw IO_error("Could not read number of points");
    }

    if(! kjb_is_bigendian() )
    {
        bswap_u32(&(N));
    }

    pts.clear();
    pts.reserve(N);

    // Points.
    Vector pt(4);
    for(unsigned int i = 0; i < N; i++)
    {
        in.read((char*)&x, sizeof(double));
        in.read((char*)&y, sizeof(double));
        in.read((char*)&z, sizeof(double));
        in.read((char*)&w, sizeof(double));
        if(in.fail() || in.eof())
        {
            throw IO_error("Could not read points");
        }

        if(! kjb_is_bigendian() )
        {
            bswap_u64((uint64_t*)&(x));
            bswap_u64((uint64_t*)&(y));
            bswap_u64((uint64_t*)&(z));
            bswap_u64((uint64_t*)&(w));
        }

    pt(0) = x;
    pt(1) = y;
    pt(2) = z;
    pt(3) = w;
    pts.push_back(pt);
    }


    // Normal.
    if(N >= 3)
    {
        in.read((char*)&x, sizeof(double));
        in.read((char*)&y, sizeof(double));
        in.read((char*)&z, sizeof(double));
        in.read((char*)&w, sizeof(double));
        if(in.fail() || in.eof())
        {
            throw IO_error("Could not read normal");
        }

        if(! kjb_is_bigendian() )
        {
            bswap_u64((uint64_t*)&(x));
            bswap_u64((uint64_t*)&(y));
            bswap_u64((uint64_t*)&(z));
            bswap_u64((uint64_t*)&(w));
        }

        pt(0) = x;
        pt(1) = y;
        pt(2) = z;
        pt(3) = w;
        normal = pt;
    }

    // Centroid.
    if(N > 0)
    {
        in.read((char*)&x, sizeof(double));
        in.read((char*)&y, sizeof(double));
        in.read((char*)&z, sizeof(double));
        in.read((char*)&w, sizeof(double));
        if(in.fail() || in.eof())
        {
            throw IO_error("Could not read centroid");
        }

        if(! kjb_is_bigendian() )
        {
            bswap_u64((uint64_t*)&(x));
            bswap_u64((uint64_t*)&(y));
            bswap_u64((uint64_t*)&(z));
            bswap_u64((uint64_t*)&(w));
        }

        pt(0) = x;
        pt(1) = y;
        pt(2) = z;
        pt(3) = w;
        centroid = pt;
    }
    
    update_centroid();
    in.read((char*)&normal_flipped, sizeof(unsigned int));
    if(! kjb_is_bigendian() )
    {
        bswap_u32((unsigned int*)&(normal_flipped));
    }

    if(in.fail() || in.eof())
    {
        throw IO_error("Could not read flipped normal indicator");
    }

}


/** 
 * The file format is big-endian binary.
 *
 * @param  out  Output stream to write the members of this polygon to.
 *
 * @throw  kjb::IO_error  Could not write to @em out.
 */
void Polygon::write(std::ostream& out) const throw (kjb::IO_error)
{
    using namespace kjb_c;

    ASSERT(sizeof(double) == sizeof(uint64_t));

    double x, y, z, w;

    // Number of points.
    unsigned int N = pts.size();

    if(! kjb_is_bigendian() )
    {
        bswap_u32(&(N));
    }

    out.write((char*)&N, sizeof(unsigned int));
    if(out.fail() || out.eof())
    {
        throw IO_error("Could not write number of points");
    }


    // Points.
    for (unsigned int i = 0; i < pts.size(); i++)
    {
        x = pts[i](0);
        y = pts[i](1);
        z = pts[i](2);
        w = pts[i](3);

        if(! kjb_is_bigendian() )
        {
            bswap_u64((uint64_t*)&(x));
            bswap_u64((uint64_t*)&(y));
            bswap_u64((uint64_t*)&(z));
            bswap_u64((uint64_t*)&(w));
        }

        out.write((char*)&x, sizeof(double));
        out.write((char*)&y, sizeof(double));
        out.write((char*)&z, sizeof(double));
        out.write((char*)&w, sizeof(double));
        if(out.fail() || out.eof())
        {
            throw IO_error("Could not write points");
        }
    }


    // Normal.
    if(N >= 3)
    {
        if(normal.get_length() == 0)
        {
            KJB_THROW_2(IO_error, "Invalid normal");
        }

        x = normal(0);
        y = normal(1);
        z = normal(2);
        w = normal(3);

        if(! kjb_is_bigendian() )
        {
            bswap_u64((uint64_t*)&(x));
            bswap_u64((uint64_t*)&(y));
            bswap_u64((uint64_t*)&(z));
            bswap_u64((uint64_t*)&(w));
        }

        out.write((char*)&x, sizeof(double));
        out.write((char*)&y, sizeof(double));
        out.write((char*)&z, sizeof(double));
        out.write((char*)&w, sizeof(double));
        if(out.fail() || out.eof())
        {
            throw IO_error("Could not write normal");
        }
    }


    // Centroid.
    if(N > 0)
    {
        if(centroid.get_length() == 0)
    {
        throw IO_error("Invalid centroid");
    }

        x = centroid(0);
        y = centroid(1);
        z = centroid(2);
        w = centroid(3);

        if(! kjb_is_bigendian() )
        {
            bswap_u64((uint64_t*)&(x));
            bswap_u64((uint64_t*)&(y));
            bswap_u64((uint64_t*)&(z));
            bswap_u64((uint64_t*)&(w));
        }

        out.write((char*)&x, sizeof(double));
        out.write((char*)&y, sizeof(double));
        out.write((char*)&z, sizeof(double));
        out.write((char*)&w, sizeof(double));
        if(out.fail() || out.eof())
        {
            throw IO_error("Could not write centroid");
        }
    }

    unsigned int nf = normal_flipped;
    if(! kjb_is_bigendian() )
    {
        bswap_u32((unsigned int*)&(nf));
    }
    out.write((char*)&nf, sizeof(unsigned int));

}


/**
 * @param  M  Homogeneous transformation matrix to transform this polygon by.
 *
 */
void Polygon::transform(const Matrix& M) throw (Illegal_argument)
{
    if(M.get_num_rows() != 4 && M.get_num_cols() != 4)
    {
        KJB_THROW_2(Illegal_argument, "Transformation matrix not homogeneous");
    }

    unsigned int N = pts.size();

    // Transform the points.
    for (unsigned int i = 0; i < N; i++)
    {
        pts[i] = M * pts[i];
    }

    // Transform the centroid.
    centroid = M * centroid;

    update_normal();
}

/** 
 * The point to add must be in the plane containing the points already in the 
 * polygon.
 *
 * @param  x  X-coord of the point to add.
 * @param  y  Y-coord of the point to add.
 * @param  z  Y-coord of the point to add.
 *
 */
void Polygon::add_point(double x, double y, double z)  throw (Illegal_argument)
{
    Vector pt(4, 1.0);

    pt[0] = x;
    pt[1] = y;
    pt[2] = z;

    add_point(pt);
}


/** 
 * The point to add must be in homogeneous coordinates (x,y,z,w) and be in the
 * plane containing the points already in the polygon.
 *
 * @param  pt  Point to add.
 *
 */
void Polygon::add_point(const Vector& pt) throw (Illegal_argument)
{
    if(pt.get_length() != 4)
    {
        KJB_THROW_2(Illegal_argument, "Point to add not homogeneous");
    }
    else if(fabs(pt[3]) < 1.0e-10)
    {
        KJB_THROW_2(Illegal_argument, "Homogeneous coordinate is zero");
    }

    unsigned int N = pts.size();

    pts.push_back(pt);

    if(N >= 3)
    {
        if(normal.get_length() == 0)
        {
            KJB_THROW_2(IO_error, "Invalid normal");
        }

        double dp1 = dot(normal, pts[0]);
        double dp2 = dot(normal, pt);

        if(fabs(dp1 - dp2) > 1.0e-4)
        {
            KJB_THROW_2(Illegal_argument, "Point to add not in the polygon's plane");
        }
    }
    else if(N == 2)
    {
        update_normal();
    }

    update_centroid();
}

/** 
 * The normal vector is created by crossing a vector made from the 
 * first two points with a vector made with the first and last points. 
 * This function flips ths order of that operation and reverses the 
 * direction of the normal.
 */
void Polygon::flip_normal() 
{ 
    normal_flipped = !normal_flipped; 
    update_normal(); 
}



/** 
 * Does nothing if this polygon has fewer than 3 points. The update is done by
 * crossing the vectors created from the first, second and third points in this
 * polygon.
 */
void Polygon::update_normal()
{
    unsigned int N = pts.size();

    if(N < 3)
    {
        return;
    }

    Vector p1 = create_vector_from_vector_section(pts[0], 0, 3);
    Vector p2 = create_vector_from_vector_section(pts[1], 0, 3);
    Vector pN = create_vector_from_vector_section(pts[2], 0, 3);

    p1 /= pts[0][3];
    p2 /= pts[1][3];
    pN /= pts[N - 1][3];

    Vector v1 = p2 - p1;
    Vector v2 = pN - p1;

    normal = cross(v1, v2);
    if(normal_flipped)
    {
        normal *= -1.0;
    }

    normal.normalize();
    normal.resize(4, 1.0);
}


/** 
 * Does nothing if this polygon has zero points. Otherwise
 * computes the centroid of the polygon
 */
void Polygon::update_centroid()
{
    unsigned int N = pts.size();
    double area = 0;

    centroid.zero_out(4);

    if(N == 0)
    {
        return;
    }
    else if(N == 1)
    {
        centroid = pts[0];
        return;
    }
    else if(N == 2)
    {
        double x = ((pts[0])[0]/(pts[0])[3] +
                    (pts[1])[0]/(pts[1])[3]) * 0.5;
        double y = ((pts[0])[1]/(pts[0])[3] +
                    (pts[1])[1]/(pts[1])[3]) * 0.5;
        double z = ((pts[0])[2]/(pts[0])[3] +
                    (pts[1])[2]/(pts[1])[3]) * 0.5;
        centroid.resize(4);
        centroid[0] = x;
        centroid[1] = y;
        centroid[2] = z;
        centroid[3] = 1.0;
        return;
    }

    double x3 = pts[N-1][0];
    double y3 = pts[N-1][1];
    double z3 = pts[N-1][2];

    for (unsigned int i = 0; i < N - 2; i++)
    {
        double x1 = (pts[i])[0];
        double y1 = (pts[i])[1];
        double z1 = (pts[i])[2];

        double x2 = (pts[i+1])[0];
        double y2 = (pts[i+1])[1];
        double z2 = (pts[i+1])[2];

        double dot = (x2 - x1)*(x3 - x1) + 
                     (y2 - y1)*(y3 - y1) + 
                     (z2 - z1)*(z3 - z1);

        double base = sqrt((x2 - x1)*(x2 - x1) + 
                           (y2 - y1)*(y2 - y1) + 
                           (z2 - z1)*(z2 - z1));

        double height;

        if(base < 1.0e-8)
        {
            height = 0;
        }
        else 
        {
            double alpha = dot / (base*base);

            double a = x3 - x1 - alpha*(x2 - x1);
            double b = y3 - y1 - alpha*(y2 - y1);
            double c = z3 - z1 - alpha*(z2 - z1);

            height = sqrt(a*a + b*b + c*c);
        }

        double tri_area = 0.5 * base * height;

        area += tri_area;
        centroid[0] += tri_area * (x1 + x2 + x3) / 3.0;
        centroid[1] += tri_area * (y1 + y2 + y3) / 3.0;
        centroid[2] += tri_area * (z1 + z2 + z3) / 3.0;
    }

    if(area > 1.0e-8)
    {
        centroid[0] /= area;
        centroid[1] /= area;
        centroid[2] /= area;
        centroid[3]  = 1.0;
    }
    else
    {
        centroid = pts[0];
    }
}

void Polygon::wire_render() const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polygon_Renderer::wire_render(*this);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

void Polygon::wire_occlude_render() const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polygon_Renderer::wire_occlude_render(*this);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

void Polygon::solid_render() const
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polygon_Renderer::solid_render(*this);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

void Polygon::project()
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polygon_Renderer::project(*this);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

void Polygon::project(const Matrix & M, double width, double height)
{
    using namespace kjb;
    switch(_rendering_framework)
    {
        case RI_OPENGL:
            GL_Polygon_Renderer::project(*this, M, width, height);
            break;
        default:
            throw KJB_error("Rendering framework not supported");
            break;
    }
}

/**
 * Finds the coefficients of the plane of the form ax + by + cz + d = 0
 * that the polygon lies in.  The coefficients are stored in the input
 * vector.
 *
 * @params  plane_params  Vector to store the plane coefficients in.
 *
 * @throw  KJB_error  Polygon has fewer than 3 points.
 */
void Polygon::fit_plane(Vector& plane_params) const
{
    if(pts.size() < 3)
    {
        throw KJB_error("Polygon has fewer than 3 points");
    }
    else
    {
        if(plane_params.get_length() != 4) 
        {
            plane_params.resize(4);
        }

        // Calculate the magnitude of the normal vector in order to normalize the normal vector.
        // Normalizing the normal vector makes comparing planes easier.
        double magnitude = sqrt(normal(0)*normal(0) + normal(1)*normal(1) + normal(2)*normal(2));

        plane_params(0) = normal(0)/magnitude;
        plane_params(1) = normal(1)/magnitude;
        plane_params(2) = normal(2)/magnitude;
        plane_params(3) = (0 - normal(0)*pts[0](0) - normal(1)*pts[0](1) - normal(2)*pts[0](2))/magnitude;
    }
}

/** 
 * Checks that the polygon is convex.  A polygon is convex if all of
 * its interior angles are less than 180 degrees.
 *
 * @return  true if convex, false otherwise.
 *
 * @throw  KJB_error  Polygon has fewer than 3 points.
 */
bool Polygon::check_convexity() const
{
    // A polygon is convex if all interior angles are less than 180.
    int N = pts.size();

    if(N < 3)
    {
        throw KJB_error("Polygon has fewer than 3 points");
    }

    Vector c(3);
    Vector a(3);
    Vector b(3);

    // To ensure that the interior angles of the polygon are computed, split each angle in half
    // (using a vector from the vertex to the centroid), compute each angle, and sum them together.
    for(int i = 1; i <= N; i++)
    {
        // Vector from the vertex (point i) to the centroid.
        c(0) = centroid(0) - pts[i%N](0);
        c(1) = centroid(1) - pts[i%N](1);
        c(2) = centroid(2) - pts[i%N](2);

        double lengthC = sqrt(c(0)*c(0) + c(1)*c(1) + c(2)*c(2));

        // Vector from the vertex (i) to the adjacent point (i + 1).
        a(0) = pts[(i+1)%N](0) - pts[i%N](0);
        a(1) = pts[(i+1)%N](1) - pts[i%N](1);
        a(2) = pts[(i+1)%N](2) - pts[i%N](2);

        double lengthA = sqrt(a(0)*a(0) + a(1)*a(1) + a(2)*a(2));

        // Vector from the vertex (i) to the other adjacent point (i - 1).
        b(0) = pts[(i-1)%N](0) - pts[i%N](0);
        b(1) = pts[(i-1)%N](1) - pts[i%N](1);
        b(2) = pts[(i-1)%N](2) - pts[i%N](2);

        double lengthB = sqrt(b(0)*b(0) + b(1)*b(1) + b(2)*b(2));

        double prodAC = (a(0)*c(0) + a(1)*c(1) + a(2)*c(2)) / (lengthA * lengthC);

        double prodCB = (c(0)*b(0) + c(1)*b(1) + c(2)*b(2)) / (lengthC * lengthB);

        // acos() gives the angle in radians.
        double angleA = acos(prodAC);
        double angleB = acos(prodCB);
        double angle = angleA + angleB;

        if(angle >= M_PI)   
        {
            return false;
        }
    }

    return true;
}

/**
 * Computes the area of the polygon if it is convex.
 *
 * @return  the area of the polygon.
 *
 * @throw  KJB_error  Polygon is not convex.
 */
double Polygon::compute_area() const
{
    if(!check_convexity())
    {
        throw KJB_error("Polygon is not convex");
    }

    Vector a(3);
    Vector b(3);

    double magnitude;
    double area = 0;

    int N = pts.size();

    for(int i = 0; i < N; i++)
    {
        a(0) = centroid(0) - pts[i](0);
        a(1) = centroid(1) - pts[i](1);
        a(2) = centroid(2) - pts[i](2);

        b(0) = pts[(i+1)%N](0) - pts[i](0);
        b(1) = pts[(i+1)%N](1) - pts[i](1);
        b(2) = pts[(i+1)%N](2) - pts[i](2);

        Vector c = cross(a,b);    

        magnitude = 0.5 * sqrt(c(0)*c(0) + c(1)*c(1) + c(2)*c(2));

        area += magnitude;
    }

    return area;
}

/** 
 * @params  vertices  A vector of Vectors to store the vertex
 *                    points of the polygon in.
 */
void Polygon::get_all_vertices(std::vector<Vector> & vertices) const 
{
    for(unsigned int i = 0; i < pts.size(); i++)
    {
        vertices.push_back(pts[i]);
    }
}

/**
  * Checks whether or not the polygon is a right triangle.
  * 
  * @params  tolerance  one of the angles must be within the range
  *                     (pi/2 - tolerance, pi/2 + tolerance) to be 
  *                     considered a right angle.
  *
  * @return  true if it is a right triangle, false otherwise.
  */
bool Polygon::check_polygon_is_right_triangle(double tolerance) const
{
    int N = pts.size();

    if(N < 3)
    {
        throw KJB_error("Polygon has fewer than 3 points");
    }
    if(N > 3)
    {
        // Polygon is not a triangle, so return false.
        return false;
    }

    // Polygon has 3 vertices, so it is a triangle.
    // Now check if it is a right triangle.

    // Note: all triangles are convex.
    Vector a(3);
    Vector b(3);

    for(int i = 1; i <= N; i++)
    {
        a(0) = pts[(i+1)%N](0) - pts[i%N](0);
        a(1) = pts[(i+1)%N](1) - pts[i%N](1);
        a(2) = pts[(i+1)%N](2) - pts[i%N](2);

        double lengthA = sqrt(a(0)*a(0) + a(1)*a(1) + a(2)*a(2));

        b(0) = pts[(i-1)%N](0) - pts[i%N](0);
        b(1) = pts[(i-1)%N](1) - pts[i%N](1);
        b(2) = pts[(i-1)%N](2) - pts[i%N](2);

        double lengthB = sqrt(b(0)*b(0) + b(1)*b(1) + b(2)*b(2));

        double prodAB = (a(0)*b(0) + a(1)*b(1) + a(2)*b(2)) / (lengthA * lengthB);

        // acos() gives the angle in radians.
        double angle = acos(prodAB);

        double half_pi = 0.5 * M_PI;

        // If the angle is close to 90 degrees, return true.
        if( (angle >= (half_pi - tolerance)) && (angle <= (half_pi + tolerance)) )
        {
            return true;
        }
    }

    // If no 90 degree angles were found, return false.
    return false;
}


/**
  * @return  an int representing the index of the longest edge in the polygon.
  */
int Polygon::get_index_of_longest_edge() const
{
    double max_length = 0.0;
    int N = pts.size();
    int longest;

    Vector edge(3);
    for(int e = 0; e < N; e++)
    {
        edge(0) = pts[(e+1)%N](0) - pts[e](0);
        edge(1) = pts[(e+1)%N](1) - pts[e](1);
        edge(2) = pts[(e+1)%N](2) - pts[e](2);

        double length = sqrt(edge(0)*edge(0) + edge(1)*edge(1) + edge(2)*edge(2));
    
        if(length > max_length)
        {
            max_length = length;
            longest = e;
        }
    }
    return longest;
}


void Polygon::get_lines(std::vector<Line3d> & lines) const
{
    for(unsigned int i = 0; i < (pts.size()-1); i++)
    {
        lines.push_back(Line3d(pts[i]/pts[i](3),pts[i+1]/pts[i+1](3)));
    }

    if(pts.size() > 2)
    {
        lines.push_back(Line3d(pts[pts.size()-1]/pts[pts.size()-1](3),pts[0]/pts[0](3)));
    }
}
