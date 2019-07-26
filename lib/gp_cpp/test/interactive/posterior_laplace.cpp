/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id: posterior_laplace.cpp 20472 2016-03-03 16:57:16Z ernesto $ */

#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_primitive.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_normal.h>
#include <gp_cpp/gp_sample.h>
#include <gp_cpp/gp_posterior.h>
#include <gp_cpp/gp_pdf.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_functors.h>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <functional>
#include <cmath>
#include <boost/bind.hpp>
#include <boost/format.hpp>

using namespace kjb;
using namespace std;
using boost::bind;

/** @class   A Laplace likelihoood */
class Linear_laplace
{
public:
    Linear_laplace(double scale, const Vector& outputs) :
        dist_(0.0, scale), outputs_(outputs)
    {}

    template<class Iter>
    void set_outputs(Iter ft, Iter lt) { outputs_.assign(ft, lt); }

    double operator()(const Vector& f) const
    {
        IFT(
            f.size() == outputs_.size(),
            Illegal_argument, 
            "Input dimennsion is different from the output dimennsion");

        std::vector<double> lls(f.size());

        transform(
            outputs_.begin(),
            outputs_.end(),
            f.begin(), 
            lls.begin(),
            std::minus<double>());

        transform(
            lls.begin(),
            lls.end(),
            lls.begin(), 
            bind(log_pdf<Laplace_distribution>, dist_, _1));

        return std::accumulate(lls.begin(), lls.end(), 0.0);
    }

private:
    Laplace_distribution dist_;
    Vector outputs_;
};

// types
typedef gp::Posterior<gp::Zero, gp::Squared_exponential, Linear_laplace> Gpp;
const int SCALE = 0;
const int FSIGMA = 1;
const int LSCALE = 2;
//const int ZERO = 0;
//const int IDENTITY = 1;
//const int SIN = 2;
const size_t NUM_ITERS = 10;

// declarations
//// glut functions
void reshape_window(int, int);
void display();
void process_key(unsigned char, int, int);

//// other crap
void update_points();
void draw_gp();
void draw_sidebar();

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

// variables that need to be global (accessed from several places)
Gpp* post_p;
gp::Inputs X;
Vector points;
Vector data;
Vector gtcurve;
double scale = 1;
double fsigma = 1;
double lscale = 1e-1;
double min_x;
double max_x;
double min_y;
double max_y;

const int info_vp_width = 320;
const int main_vp_initial_width = 800;
int win_width = main_vp_initial_width + info_vp_width;
int win_height = main_vp_initial_width;
const double param_val_inc = 0.1;
const double param_val_inc_big = 1.0;
const double grid_sz = 1.0;

// state variables
string param_names[] = { "scale", "signal variance", "laplace noise scale" };
double* params[] = { &scale, &fsigma, &lscale };
int cur_param = SCALE;

bool points_up_to_date = false;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

int main(int argc, char** argv)
{
    try
    {
        // create posterior and resize outputs
        X = gp::make_inputs(0, 10, 0.1);
        size_t num_inputs = X.size();

        data.resize(num_inputs);
        gtcurve.resize(num_inputs);
        for(size_t i = 0; i < num_inputs; i++)
        {
            double x = X[i][0];
            Laplace_distribution ln(0.0, 0.2);
            gtcurve[i] = sqrt(x) - sin(2*x);
            data[i] = gtcurve[i] + sample(ln);
        }

        Gpp post(
            gp::Zero(), 
            gp::Sqex(scale, fsigma),
            Linear_laplace(lscale, data),
            X.begin(),
            X.end(),
            data.begin(),
            data.end());

        post_p = &post;
        points.resize(num_inputs);

        // initialize posterior and realize GP
        min_x = X.front().front() - 0.1;
        max_x = X.back().front() + 0.1;
        min_y = *min_element(data.begin(), data.end()) - 0.1;
        max_y = *max_element(data.begin(), data.end()) + 0.1;
        update_points();

        // set up glut window
        opengl::Glut_window wnd(win_width, win_height, "Play with GP posterior");
        wnd.set_display_callback(display);
        wnd.set_reshape_callback(reshape_window);
        wnd.set_keyboard_callback(process_key);

        glutMainLoop();
    }
    catch(const Exception& kex)
    {
        kex.print_details();
        cerr << endl;
        return EXIT_FAILURE;
    }
    catch(const exception& sex)
    {
        cerr << sex.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void reshape_window(int w, int h)
{
    win_width = w;
    win_height = h;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    draw_gp();
    draw_sidebar();

    glutSwapBuffers();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_key(unsigned char key, int, int)
{
    switch(key)
    {
        case 'x':
        {
            *params[cur_param] -= param_val_inc;
            points_up_to_date = false;
            break;
        }

        case 'X':
        {
            *params[cur_param] += param_val_inc;
            points_up_to_date = false;
            break;
        }

        case 'z':
        {
            *params[cur_param] -= param_val_inc_big;
            points_up_to_date = false;
            break;
        }

        case 'Z':
        {
            *params[cur_param] += param_val_inc_big;
            points_up_to_date = false;
            break;
        }

        case 'k':
        {
            --cur_param;
            cur_param = (cur_param + 3) % 3;
            break;
        }

        case 'j':
        {
            ++cur_param;
            cur_param = cur_param % 3;
            break;
        }

        case 'r':
        {
            update_points();
            break;
        }

        case 'q':
        {
            exit(EXIT_SUCCESS);
        }

        default:
            break;
    }

    //wnd.redisplay();
    glutPostRedisplay();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void update_points()
{
    if(!points_up_to_date)
    {
        post_p->set_covariance_function(gp::Sqex(scale, fsigma));
        post_p->set_likelihood(Linear_laplace(lscale, data));
    }

    // sample to get the MAP
    double max_lp = points_up_to_date ? gp::log_pdf(*post_p, points) : -DBL_MAX;
    for(size_t i = 0; i < NUM_ITERS; i++)
    {
        Vector f = gp::sample(*post_p);
        double lp = gp::log_pdf(*post_p, f);
        if(lp > max_lp)
        {
            max_lp = lp;
            points = f;
        }
    }
    // compute min and max for each dimension
    //min_y = *min_element(points.begin(), points.end()) - 0.1;
    //max_y = *max_element(points.begin(), points.end()) + 0.1;

    points_up_to_date = true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void draw_gp()
{
    // set up main viewport and stuff
    int main_vp_width = max(0, win_width - info_vp_width);
    glViewport(0, 0, (GLsizei)main_vp_width, (GLsizei)win_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(min_x, max_x, min_y, max_y);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // draw axes
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(2.0);
    glBegin(GL_LINES);
        // x-axis
        glVertex2d(min_x, 0.0);
        glVertex2d(max_x, 0.0);

        // y-axis
        glVertex2d(0.0, min_y);
        glVertex2d(0.0, max_y);
    glEnd();

    // draw grid
    glColor3f(0.1, 0.1, 0.1);
    glLineWidth(1.0);
    double mnx = floor(min_x / grid_sz) * grid_sz;;
    double mxx = ceil(max_x / grid_sz) * grid_sz;;
    double mny = floor(min_y / grid_sz) * grid_sz;;
    double mxy = ceil(max_y / grid_sz) * grid_sz;;
    opengl::render_xy_plane_grid(mnx, mxx, mny, mxy, grid_sz);

    // draw data
    opengl::Ellipse ellipse((max_x - min_x)/200.0, (max_y - min_y)/200.0);
    glColor3f(1.0, 1.0, 0.0);
    glLineWidth(1.0);
    for(size_t i = 0; i < data.size(); i++)
    {
        glPushMatrix();
            //opengl::glTranslate(data[i]);
            opengl::glTranslate(Vector(X[i][0], data[i]));
            ellipse.solid_render();
        glPopMatrix();
    }

    // draw gt curve
    glColor3f(0.3, 0.3, 0.3);
    glLineWidth(1.0);
    glBegin(GL_LINE_STRIP);
    for(size_t i = 0; i < gtcurve.size(); ++i)
    {
        opengl::glVertex(Vector(X[i][0], gtcurve[i]));
    }
    glEnd();

    // draw gp
    glColor3f(0.0, 1.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINE_STRIP);
    for(size_t i = 0; i < points.size(); ++i)
    {
        opengl::glVertex(Vector(X[i][0], points[i]));
    }
    glEnd();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void draw_sidebar()
{
    // set up sidebar viewport
    int main_vp_width = max(0, win_width - info_vp_width);
    glViewport(main_vp_width, 0, (GLsizei)info_vp_width, (GLsizei)win_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, info_vp_width, win_height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    // draw text
    using opengl::bitmap_string;
    boost::format fmt("%0.1f");

    glLoadIdentity();
    glColor3f(0.75, 0.75, 0.75);
    glBegin(GL_QUADS);
    glVertex2d(0.0, 0.0);
    glVertex2d(0.0, win_height);
    glVertex2d(info_vp_width, win_height);
    glVertex2d(info_vp_width, 0.0);
    glEnd();

    glColor3f(0.2, 0.2, 0.7);

    const int margin_size = 12;
    const int line_width = 20;
    const int str_pos_x = margin_size;
    int str_pos_y = line_width;
    string temp_str;

    // draw text for current parameter information
    if(cur_param == SCALE) glColor3f(1.0, 0.0, 0.0);
    else glColor3f(0.2, 0.2, 0.7);
    temp_str = param_names[SCALE] + ": " + boost::str(fmt % scale);
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    if(cur_param == FSIGMA) glColor3f(1.0, 0.0, 0.0);
    else glColor3f(0.2, 0.2, 0.7);
    temp_str = param_names[FSIGMA] + ": " + boost::str(fmt % fsigma);
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    if(cur_param == LSCALE) glColor3f(1.0, 0.0, 0.0);
    else glColor3f(0.2, 0.2, 0.7);
    temp_str = param_names[LSCALE] + ": " + boost::str(fmt % lscale);
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    // draw separating line
    glColor3f(0.2, 0.2, 0.7);
    glBegin(GL_LINES);
    glVertex2i(0.0, str_pos_y);
    glVertex2i(info_vp_width, str_pos_y);
    glEnd();
    str_pos_y += line_width;

    // draw text for control information
    bitmap_string("HELP", str_pos_x, str_pos_y);
    str_pos_y += line_width;

    temp_str = "Change parameter: j/k";
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    temp_str = "Modify current parameter by "
                    + boost::str(fmt % param_val_inc) + ": x/X";
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    temp_str = "Modify current parameter by "
                    + boost::str(fmt % param_val_inc_big) + ": z/Z";
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    if(!points_up_to_date)
    {
        glColor3f(1.0, 0.0, 0.0);
    }
    bitmap_string("Re-draw: r", str_pos_x, str_pos_y);
    str_pos_y += line_width;

    glColor3f(0.2, 0.2, 0.7);
    bitmap_string("Quit: q", str_pos_x, str_pos_y);
    str_pos_y += line_width;
}

