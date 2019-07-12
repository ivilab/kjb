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
|     Ernesto Brau
|
* =========================================================================== */

/**
 * @file gr_rectangle_2d.h
 *
 * @author Ernesto Brau
 *
 * @brief Class representing a rectangle in R^2
 */


#ifndef KJB_RECTANGLE_2D_H
#define KJB_RECTANGLE_2D_H

#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <g_cpp/g_util.h>

namespace kjb {

/**
 * @class Rectangle_2d
 *
 * @brief   Class that represents an axis-aligned 2D rectangle.
 *          It is defined in terms of its (2D) center, its width and its height.
 */
class Rectangle_2d
{
public:
    /** @brief Constructs a Rectangle_2d. */
    Rectangle_2d
    (
        const Vector& center = Vector().set(0.0, 0.0),
        double orientation = 0.0,
        double width = 1.0,
        double height = 1.0
    ) :
        m_center(center),
        m_orientation(orientation),
        m_width(width),
        m_height(height)
    {}

    /** @brief returns the center of this rectangle. */
    const Vector& get_center() const
    {
        return m_center;
    }

    /** @brief  Returns the orientation of this rectangle. */
    double get_orientation() const
    {
        return m_orientation;
    }

    /** @brief returns the width of this rectangle. */
    double get_width() const
    {
        return m_width;
    }

    /** @brief returns the height of this rectangle. */
    double get_height() const
    {
        return m_height;
    }

    /** @brief  Returns the corners of this rectangle */
    std::vector<Vector> get_corners() const;

    /** @brief  Returns the midpoints of the sides of this rectangle */
    std::vector<Vector> get_side_midpoints() const;

    /** @brief sets the center of this rectangle. */
    void set_center(const Vector& center)
    {
        m_center = center;
    }

    /** @brief  Sets the orientation of this rectangle. */
    void set_orientation(double orientation)
    {
        // TODO: keep orientation between -M_PI and M_PI.
        m_orientation = orientation;
    }

    /** @brief sets the width of this rectangle. */
    void set_width(double width)
    {
        m_width = width;
    }

     /** @brief sets the height of this rectangle. */
    void set_height(double height)
    {
        m_height = height;
    }

    /*===================================================================== *
     *                          GEOMETRY                                    *
     *===================================================================== */

    void translate(const Vector& t)
    {
        set_center(get_center() + t);
    }

    void rotate(double angle)
    {
        set_orientation(get_orientation() + angle);
    }

    /*===================================================================== *
     *                          RENDERING                                   *
     *===================================================================== */

    void wire_render() const;

private:
    Vector m_center;
    double m_orientation;
    double m_width;
    double m_height;

};

} // namespace kjb

#endif /*KJB_RECTANGLE_2D_H */

