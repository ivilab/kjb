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
|     Joseph Schlecht, Luca Del Pero, Ernesto Brau
|
* =========================================================================== */

/**
 * @file
 *
 * @author Luca Del Pero, Ernesto Brau
 *
 * @brief Class representing an axis-aligned, 2D rectangle.
 */


#ifndef KJB_AA_RECTANGLE_2D_H
#define KJB_AA_RECTANGLE_2D_H

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_vector.h"
#include "l_cpp/l_functors.h"

#include <iosfwd>

#include <algorithm>
#include <vector>

namespace kjb {
class Base_gl_interface;
class Image;

/**
 * @class Axis_aligned_rectangle_2d
 *
 * @brief   Class that represents an axis-aligned 2D rectangle.
 *          It is defined in terms of its (2D) center, its width and its height.
 */
class Axis_aligned_rectangle_2d
{
    typedef Axis_aligned_rectangle_2d Self;
public:

    /** @brief Constructs a Axis_aligned_rectangle_2d. */
    Axis_aligned_rectangle_2d(const Vector& center = Vector(2, 0.0), double width = 1.0, double height = 1.0) :
        m_center(center),
        m_width(width),
        m_height(height)
    {}

    /** @brief Constructs a Axis_aligned_rectangle_2d. */
    Axis_aligned_rectangle_2d(const Vector& p1, const Vector& p2) :
        m_center(0.5 * (p1 + p2)),
        m_width(fabs(p1[0] - p2[0])),
        m_height(fabs(p1[1] - p2[1]))
    {}

    /** @brief Clones this Axis_aligned_rectangle_2d. */
    Axis_aligned_rectangle_2d* clone() const
    {
        return new Axis_aligned_rectangle_2d(*this);
    }

    /** @brief Deletes this Axis_aligned_rectangle_2d. */
    ~Axis_aligned_rectangle_2d(){}

    /** @brief returns the center of this Axis_aligned_rectangle_2d */
    const Vector& get_center() const
    {
        return m_center;
    }

    /** @brief returns the width of this bounding box */
    double get_width() const
    {
        return m_width;
    }

    /** @brief returns the height of this bounding box */
    double get_height() const
    {
        return m_height;
    }

    double get_left() const { return m_center[0] - m_width / 2.0; }
    double get_right() const { return m_center[0] + m_width / 2.0; }
    double get_bottom() const { return m_center[1] - m_height / 2.0; }
    double get_top() const { return m_center[1] + m_height/ 2.0; }

    Vector get_top_left() const { return m_center + Vector(-m_width / 2.0, m_height / 2.0); }
    Vector get_bottom_right() const { return m_center + Vector(m_width / 2.0, -m_height / 2.0); }
    Vector get_top_center() const { return m_center + Vector(0.0, m_height / 2.0); }
    Vector get_bottom_center() const { return m_center + Vector(0.0, -m_height / 2.0); }


    /** @brief sets the center of this bounding box */
    void set_center(const Vector& center)
    {
        m_center = center;
    }

    /** @brief sets the width of this bounding box */
    void set_width(double width)
    {
        m_width = width;
    }

     /** @brief sets the height of this bounding box */
    void set_height(double height)
    {
        m_height = height;
    }

    /*===================================================================== *
     *                          RENDERING                                   *
     *===================================================================== */

    void wire_render() const;

    /*===================================================================== *
     *                          TESTS                                       *
     *===================================================================== */


    /** Returns true if pt lies inside this box */
    bool contains(const kjb::Vector& pt) const
    {
        ASSERT(pt.size() == 2);

        return pt[0] >= get_left() && pt[0] <= get_right() &&
               pt[1] >= get_bottom() && pt[1] <= get_top();
    }

    bool intersects(const Self& other) const
    {
        // probably could rewrite this for quicker short-circuiting...
        return !(other.get_left() >= get_right() ||
                 other.get_right() <= get_left() ||
                 other.get_bottom() >= get_top() ||
                 other.get_top() <= get_bottom());
    }

    /*===================================================================== *
     *                              OTHER                                   *
     *===================================================================== */

    void draw(kjb::Image & img, double ir = 255, double ig = 0, double ib = 0, double iwidth = 1.0) const;

    void write_corners_on(std::ostream& ofs);

    /**
     * @brief   Swaps this rectangle with another.
     */
    void swap(Axis_aligned_rectangle_2d& r)
    {
        using std::swap;
        swap(m_center, r.m_center);
        swap(m_width, r.m_width);
        swap(m_height, r.m_height);
    }


    /* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
    /* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
    /*===================================================================== *
     *              SUPERFLUOUS METHODS -- MIGHT DISAPPEAR                  *
     *===================================================================== */
    /* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */
    /* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */


    /** @brief Constructs a Axis_aligned_rectangle_2d. */
    // Superflous...should disappear.
    Axis_aligned_rectangle_2d(double center_x, double center_y, double iwidth, double iheight) :
        m_center(Vector().set(center_x, center_y)),
        m_width(iwidth),
        m_height(iheight)
    {}

    /** @brief returns the center of this Axis_aligned_rectangle_2d */
    const Vector& get_centre() const
    {
        return get_center();
    }

    /** @brief returns the x-coordinate of the center of this Axis_aligned_rectangle_2d */
    double get_centre_x() const
    {
        return m_center(0);
    }

    /** @brief returns the y-coordinate of the center of this Axis_aligned_rectangle_2d */
    double get_centre_y() const
    {
        return m_center(1);
    }

    /** @brief returns the area of the box */
    double get_area() const
    {
        return m_width*m_height;
    }

    /** @brief sets the center of this bounding box */
    void set_centre(const Vector & icenter)
    {
        set_center(icenter);
    }

    /** @brief sets the x-coordinate of the center of this bounding box */
    void set_centre_x(double center_x)
    {
        m_center(0) = center_x;
    }

    /** @brief sets the y-coordinate of the center of this bounding box */
    void set_centre_y(double center_y)
    {
        m_center(1) = center_y;
    }

friend std::ostream& operator<<(std::ostream& ost, const Axis_aligned_rectangle_2d& box);
friend std::istream& operator>>(std::istream& ist, Axis_aligned_rectangle_2d& box);

private:
    Vector m_center;
    double m_width;
    double m_height;

};

std::ostream& operator<<(std::ostream& ost, const Axis_aligned_rectangle_2d& box);

std::istream& operator>>(std::istream& ist, Axis_aligned_rectangle_2d& box);

typedef Axis_aligned_rectangle_2d Bounding_Box2D;

/**
 * find the intersection between two bounding boxes
 */
Bounding_Box2D intersect(const Bounding_Box2D& b1, const Bounding_Box2D& b2);

/**
 * Translate a bounding box by a 2d offset vector
 */
inline void translate(kjb::Axis_aligned_rectangle_2d& box, const kjb::Vector& t)
{
    kjb::Vector c = box.get_center();
    box.set_center(c + t);
}

/**
 * Scale a bounding box by a 2d scale vector.  
 *
 * The effect is that the four corners are effectively multiplied by a scalar;
 * i.e. boxes not centered at the origin will change position (similar to
 * opengl). If this is undesirable, scale first, then translate.
 */
void scale(kjb::Axis_aligned_rectangle_2d& box, const kjb::Vector& s);

/**
 * @brief Projects a set of 3D points onto the image plane,
 * and finds a bounding box (aligned with the image axes),
 * such that it contains all the projected points
 */
void get_projected_bbox_from_3Dpoints
(
    Axis_aligned_rectangle_2d& bb,
    const std::vector<Vector>& points,
    const Base_gl_interface& camera,
    double img_width,
    double img_height
);

/**
 * @brief   Computes the 2D bounding box of a range of 2D points.
 *
 * This is implemented in the most straight-forward way possible; it's
 * linear in the number of points.
 *
 * @param   first   Points to the first kjb::Vector in the range.
 * @param   first   Past-the-end iterator in the range.
 */
template<class Iterator>
inline
Axis_aligned_rectangle_2d compute_bounding_box(Iterator first, Iterator last)
{
    Vector left = *std::min_element(first, last, Index_less_than<Vector>(0));
    Vector right = *std::max_element(first, last, Index_less_than<Vector>(0));
    Vector bottom = *std::min_element(first, last, Index_less_than<Vector>(1));
    Vector top = *std::max_element(first, last, Index_less_than<Vector>(1));

    return Axis_aligned_rectangle_2d(Vector().set((left[0] + right[0]) / 2.0, (bottom[1] + top[1]) / 2.0), right[0] - left[0], top[1] - bottom[1]);
}

/**
 * @brief   Compute area of intersection of two rectangles.
 */
double get_rectangle_intersection
(
    const kjb::Bounding_Box2D& b1,
    const kjb::Bounding_Box2D& b2
);

/**
 * @brief   Swaps two rectangles.
 */
inline
void swap(Axis_aligned_rectangle_2d& r1, Axis_aligned_rectangle_2d& r2)
{
    r1.swap(r2);
}

} // namespace kjb

#endif

