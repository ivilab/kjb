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

/* $Id: gr_plot.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#ifdef KJB_HAVE_GLUT
#include "gr_cpp/gr_plot.h"
#include "gr_cpp/gr_opengl.h"
#include "gr_cpp/gr_opengl_headers.h"
#include "gr_cpp/gr_glut.h"
#include "m_cpp/m_vector_d.h"

#include <boost/format.hpp>
#include <boost/array.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <limits>
#include <string>
#include <iostream>

using namespace kjb;
using namespace kjb::opengl;

const boost::array<Vector3, 7> PLOT_COLORS_ = boost::assign::list_of
    (Vector3(0.0, 0.0, 1.0))
    (Vector3(0.0, 0.5, 0.0))
    (Vector3(1.0, 0.0, 0.0))
    (Vector3(0.0, 0.75, 0.75))
    (Vector3(0.75, 0.0, 0.75))
    (Vector3(0.75, 0.75, 0.0))
    (Vector3(0.25, 0.25, 0.25));

Glut_plot_window::Glut_plot_window(size_t width, size_t height, const std::string& title) :
    wnd_(new Glut_window(width, height, title))
{
    wnd_->set_reshape_callback(
        boost::bind(&Glut_plot_window::reshape, this, _1, _2));
    wnd_->set_display_callback(boost::bind(&Glut_plot_window::render, this));
    //wnd_->set_keyboard_callback(boost::bind(&Glut_plot_window::keys, this));

    update_bounds();
}

Glut_plot_window::~Glut_plot_window()
{
    delete wnd_;
}

Glut_plot_window& Glut_plot_window::add_plot(const Vector& x, const Vector& y)
{
    update_bounds(x, y);

    x_.push_back(x);
    y_.push_back(y);

    return *this;
}

Glut_plot_window& Glut_plot_window::add_plot(const Vector* x, const Vector* y)
{
    update_bounds(*x, *y);

    xp_.push_back(x);
    yp_.push_back(y);

    return *this;
}

void Glut_plot_window::update_bounds()
{
    if(x_.empty() && xp_.empty())
    {
        xmin_ = ymin_ = -1.0;
        xmax_ = ymax_ = 1.0;
        return;
    }

    xmin_ = std::numeric_limits<double>::max();
    xmax_ = -std::numeric_limits<double>::max();
    ymin_ = std::numeric_limits<double>::max();
    ymax_ = -std::numeric_limits<double>::max();

    for(size_t i = 0; i < x_.size(); i++)
    {
        update_bounds(x_[i], y_[i]);
    }

    for(size_t i = 0; i < xp_.size(); i++)
    {
        update_bounds(*xp_[i], *yp_[i]);
    }
}

void Glut_plot_window::render() const
{
    const size_t num_cells = 8;
    const double buf_frac = 1/50.0;

    double xrange = std::max(xmax_ - xmin_, 1e-5);
    double yrange = std::max(ymax_ - ymin_, 1e-5);
    double xstep = xrange / num_cells;
    double ystep = yrange / num_cells;
    double xbuf = xrange * buf_frac;
    double ybuf = yrange * buf_frac;

    double uppx = (xrange + 2*xbuf) / (wnd_->get_width() - 2*xmargin_);
    double uppy = (yrange + 2*ybuf) / (wnd_->get_height() - 2*ymargin_);

    double xmargin = uppx*xmargin_;
    double ymargin = uppy*ymargin_;

    std::cout << "width = " << wnd_->get_width() << std::endl;
    std::cout << "height = " << wnd_->get_height() << std::endl;
    std::cout << "xrange = " << xrange << std::endl;
    std::cout << "yrange = " << yrange << std::endl;
    std::cout << "xstep = " << xstep << std::endl;
    std::cout << "ystep = " << ystep << std::endl;
    std::cout << "xbuf = " << xbuf << std::endl;
    std::cout << "ybuf = " << ybuf << std::endl;
    std::cout << "uppx = " << uppx << std::endl;
    std::cout << "uppy = " << uppy << std::endl;
    std::cout << "xmargin = " << xmargin << std::endl;
    std::cout << "ymargin = " << ymargin << std::endl;
    std::cout << std::endl;

    // set up matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(xmin_ - xbuf - xmargin, xmax_ + xbuf + xmargin,
               ymin_ - ybuf - ymargin, ymax_ + ybuf + ymargin);
    glMatrixMode(GL_MODELVIEW);

    // reset everything
    glClearColor(0.95, 0.95, 0.95, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // render border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2d(xmin_ - xbuf, ymin_ - ybuf);
        glVertex2d(xmax_ + xbuf, ymin_ - ybuf);
        glVertex2d(xmax_ + xbuf, ymax_ + ybuf);
        glVertex2d(xmin_ - xbuf, ymax_ + ybuf);
    glEnd();

    // render axes
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
        glVertex2d(xmin_ - xbuf, 0.0);
        glVertex2d(xmax_ + xbuf, 0.0);
        glVertex2d(0.0, ymin_ - ybuf);
        glVertex2d(0.0, ymax_ + ybuf);
    glEnd();

    // render grid
    glColor3d(0.7, 0.7, 0.7);
    glBegin(GL_LINES);
    for(size_t i = 1; i < num_cells; i++)
    {
        glVertex2d(xmin_ - xbuf, ymin_ + i*ystep);
        glVertex2d(xmax_ + xbuf, ymin_ + i*ystep);

        glVertex2d(xmin_ + i*xstep, ymin_ - ybuf);
        glVertex2d(xmin_ + i*xstep, ymax_ + ybuf);
    }
    glEnd();

    // render marks
    glColor3d(0.0, 0.0, 0.0);
    for(size_t i = 1; i < num_cells; i += 2)
    {
        std::string x_str = boost::str(
                    boost::format("%.2g") % (xmin_ + i*xstep));
        bitmap_string(
                    x_str,
                    xmin_ + i*xstep - 5*uppx,
                    ymin_ - ybuf - ymargin + 5*uppy);

        std::string y_str = boost::str(
                    boost::format("%8.2g") % (ymin_ + i*ystep));
        bitmap_string(
                    y_str,
                    xmin_ - xbuf - xmargin + 5*uppx,
                    ymin_ + i*ystep - 6*uppy);
    }

    // render plots
    for(size_t i = 0; i < x_.size(); i++)
    {
        glColor(PLOT_COLORS_[i % 7]);
        glBegin(GL_LINE_STRIP);
        for(size_t j = 0; j < x_[i].size(); j++)
        {
            glVertex2d(x_[i][j], y_[i][j]);
        }
        glEnd();
    }

    // render plots (pointers)
    for(size_t i = 0; i < xp_.size(); i++)
    {
        glColor(PLOT_COLORS_[(x_.size() + i) % 7]);
        glBegin(GL_LINE_STRIP);
        for(size_t j = 0; j < xp_[i]->size(); j++)
        {
            glVertex2d((*xp_[i])[j], (*yp_[i])[j]);
        }
        glEnd();
    }

    glutSwapBuffers();
}

void Glut_plot_window::reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

void Glut_plot_window::update_bounds(const Vector& x, const Vector& y)
{
    double xmn = x.empty() ? std::numeric_limits<double>::max()
                           : *std::min_element(x.begin(), x.end());
    double xmx = x.empty() ? -std::numeric_limits<double>::max()
                           : *std::max_element(x.begin(), x.end());
    double ymn = y.empty() ? std::numeric_limits<double>::max()
                           : *std::min_element(y.begin(), y.end());
    double ymx = y.empty() ? -std::numeric_limits<double>::max()
                           : *std::max_element(y.begin(), y.end());

    if(x_.empty() && xp_.empty())
    {
        xmin_ = xmn;
        xmax_ = xmx;
        ymin_ = ymn;
        ymax_ = ymx;
    }
    else
    {
        xmin_ = std::min(xmn, xmin_);
        xmax_ = std::max(xmx, xmax_);
        ymin_ = std::min(ymn, ymin_);
        ymax_ = std::max(ymx, ymax_);
    }
}

void Glut_plot_window::redisplay() const 
{
    wnd_->redisplay(); 
}
#endif
