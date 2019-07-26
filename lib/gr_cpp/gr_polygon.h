
/* $Id: gr_polygon.h 18278 2014-11-25 01:42:10Z ksimek $ */

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

#ifndef POLYGON_H_INCLUDED
#define POLYGON_H_INCLUDED

#include <vector>
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include <gr_cpp/gr_renderable.h>

#include "l_cpp/l_readable.h"
#include "l_cpp/l_writeable.h"

namespace kjb {

typedef std::pair<kjb::Vector, kjb::Vector> Line3d;

class Polygon : public Abstract_renderable, public Readable, public Writeable
{
public:

    /** @brief Constructs a polygon with zero vertices */
    Polygon();

    /** @brief Constructs a polygon with N vertices */
    Polygon(unsigned int N);

    Polygon(const Polygon& p);

    /** @brief Constructs a polygon by reading it from a file */
    Polygon(const char* fname) throw (Illegal_argument, IO_error);

    /** @brief Constructs a polygon by reading it from an input stream */
    Polygon(std::istream& in) throw (Illegal_argument, IO_error);

    virtual ~Polygon();

    virtual Polygon& operator=(const Polygon& p);

    virtual Polygon* clone() const;

    virtual void read(std::istream& in) throw (Illegal_argument, IO_error);

    virtual void write(std::ostream& out) const throw (IO_error);

    /** @brief transforms all the vertices by the input matrix */
    virtual void transform(const Matrix& M) throw (Illegal_argument);

    /** @brief Adds a point to this polygon */
    void add_point(const Vector& pt) throw (Illegal_argument);

    /** @brief Adds a point to this polygon */
    void add_point(double x, double y, double z) throw (Illegal_argument);

    /** @brief Resets one of the points in this polygon */
    inline void set_point(double index, double x, double y, double z) throw(Index_out_of_bounds)
    {
        (pts[index])(0) = x;
        (pts[index])(1) = y;
        (pts[index])(2) = z;
    }

    /** @brief Returns the normal of this polygon */
    inline const Vector& get_normal() const
    {
        return normal;
    }

    /** @brief Clears all the vertices */
    inline void clear() {pts.clear();}

    /** @brief returns the centroid of this polygon */
    inline const Vector& get_centroid() const
    {
        return centroid;
    }

    /** @brief returns the number of vertices of this polygon */
    inline unsigned int get_num_points() const
    {
        return pts.size();
    }

    /** @brief returns the ith point of this polygon */
    inline const Vector& get_point(unsigned int i) const throw (Illegal_argument)
    {
        if(i >= pts.size())
        {
            KJB_THROW(Index_out_of_bounds);
        }

        return pts[i];
    }

    /** @brief returns the vector of vertices */
    inline const std::vector<Vector> & get_vertices() const
    {
        return pts;
    }

    /** @brief Flips the normal of this polygon */
    void flip_normal();

    /** @brief This function is useful when we index a polygon by its edges. In a Polygon
     * with n vertices, 0 is the edge between the first and the second point, n-1 is the edge
     * between the last and the first point. Given an edge index, this function returns the first
     * vertex forming this edge using this convention
     */
    inline const Vector& get_edge_first_vertex(unsigned int edge) const throw (Index_out_of_bounds)
    {
        if(edge >= get_num_points())
        {
            throw Index_out_of_bounds("Edge index out of bounds, cannot get first vertex");
        }

        return pts[edge];
    }

    /** @brief This function is useful when we index a polygon by its edges. In a Polygon
    * with n vertices, 0 is the edge between the first and the second point, n-1 is the edge
    * between the last and the first point. Given an edge index, this function returns the second
    * vertex forming this edge using this convention
    */
    inline const Vector& get_edge_second_vertex(unsigned int edge) const throw (Index_out_of_bounds)
    {
        unsigned int n_pts = get_num_points();
        if(edge >= n_pts)
        {
            throw Index_out_of_bounds("Edge index out of bounds, cannot get second vertex");
        }

        return pts[ ( ( edge + 1 ) % n_pts ) ];
    }

    /** Rendering */
    /** @brief Renders this polygon as a wireframe */
    virtual void wire_render() const;
    /** @brief Renders this polygon in the depth buffer */
    virtual void wire_occlude_render() const;
    /** @brief Renders this polygon as a solid */
    virtual void solid_render() const;
    /** @brief Projects all the vertices in this polygon onto the image plane */
    virtual void project();

    virtual void project(const Matrix & M, double width, double height);

    /** @brief Finds the coefficients of the plane that this polygon lies in */
    void fit_plane(Vector & plane_params) const;

    /** @brief  Checks that this polygon is convex */
    bool check_convexity() const;

    /** @brief Computes the area of this polygon if it is convex */
    double compute_area() const;

    /** @brief Stores all of the vertices of this polygon in a vector */
    void get_all_vertices(std::vector<Vector> & vertices) const;

    /** @brief Checks if this polygon is a right triangle */
    bool check_polygon_is_right_triangle(double tolerance) const;

    /** @brief Returns the index of the longest edge in the polygon */
    int get_index_of_longest_edge() const;

    /** @brief Updates the normal and the centroid of the polygon according to the vertices */
    void update()
    {
        update_normal();
        update_centroid();
    }

    void get_lines(std::vector<Line3d> & lines) const;

protected:

    /** @brief Recalculates the normal of this polygon. The first three vertices are used. Notice
     *         that this is very important, because it determines a sign convention */
    void update_normal();

    /** @brief Recalculates the centroid of this polygon */
    void update_centroid();

protected:

    std::vector<Vector> pts;
    Vector normal;
    Vector centroid;

    //Used to determine whether the normal of this polygon is flipped
    bool normal_flipped;

private:

    friend class GL_Polygon_Renderer;
};

}

#endif

