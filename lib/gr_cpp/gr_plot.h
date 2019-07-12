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
|  Author: Ernesto Brau
|
* =========================================================================== */

/* $Id: gr_plot.h 18278 2014-11-25 01:42:10Z ksimek $ */

#ifndef KJB_CPP_GR_PLOT_H
#define KJB_CPP_GR_PLOT_H

#include <string>
#include <iterator>
#include <vector>
#include <m_cpp/m_vector.h>

namespace kjb {
namespace opengl {


#ifdef KJB_HAVE_GLUT
#warning "[Code police] Put KJB_HAVE_* guards inside functions"
class Glut_window;

class Glut_plot_window
{
public:
    /** @brief  Construct a plot window. */
    Glut_plot_window(size_t width, size_t height, const std::string& title);

    virtual ~Glut_plot_window();

    /** @brief  Add a graph to plot. */
    Glut_plot_window& add_plot(const Vector& x, const Vector& y);

    /** @brief  Add a graph to plot. */
    Glut_plot_window& add_plot(const Vector* x, const Vector* y);

    /** @brief  Add a graph to plot. */
    template<class OutIt>
    Glut_plot_window& add_plot(OutIt first_x, OutIt last_x, OutIt first_y)
    {
        OutIt last_y = first_y;
        std::advance(last_y, std::distance(first_x, last_x));

        Vector x(first_x, last_x);
        Vector y(first_y, last_y);

        return add_plot(x, y);
    }

    /** @brief  Update min and max for x and y. */
    void update_bounds();

    /** @brief  Call the GLUT redisplay event on this window. */
    void redisplay() const;

private:
    /** @brief  Render the plot. */
    void render() const;

    /** @brief  Render the plot. */
    void reshape(int w, int h);

    /** @brief  Update min and max for x and y. */
    void update_bounds(const Vector& x, const Vector& y);

    Glut_window* wnd_;
    std::vector<Vector> x_;
    std::vector<Vector> y_;
    std::vector<const Vector*> xp_;
    std::vector<const Vector*> yp_;
    double xmin_;
    double xmax_;
    double ymin_;
    double ymax_;
    static const size_t xmargin_ = 74;
    static const size_t ymargin_ = 20;
};
#endif    

}} //namespace kjb::opengl

#endif /*KJB_CPP_GR_PLOT_H */

