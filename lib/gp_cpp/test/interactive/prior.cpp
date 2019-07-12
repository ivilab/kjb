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

/* $Id: prior.cpp 20582 2016-03-23 02:21:31Z ernesto $ */

#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#include <gr_cpp/gr_glut.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_prior.h>
#include <gp_cpp/gp_sample.h>
#include <m_cpp/m_vector.h>
#include <l_cpp/l_exception.h>
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

// types
typedef gp::Prior<gp::Manual, gp::Squared_exponential> Gpp;
const int SCALE = 0;
const int FSIGMA = 1;
const int ZERO = 0;
const int IDENTITY = 1;
const int SIN = 2;

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
Gpp* prior_p;
vector<Vector> points;
vector<Vector> mean_pts;
double scale = 1;
double fsigma = 1;
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
string param_names[2] = { "scale", "signal variance" };
double* params[2] = { &scale, &fsigma };
int cur_param = SCALE;

string mean_names[3] = { "zero", "identity", "sine" };
int cur_mean = ZERO;

bool points_up_to_date = false;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

int main(int argc, char** argv)
{
    try
    {
        // create prior and resize outputs
        gp::Inputs X = gp::make_inputs(0, 10, 0.1);
        size_t num_inputs = X.size();

        gp::Manual mf(X, Vector((int)num_inputs, 0.0));
        Gpp prior(mf, gp::Sqex(scale, fsigma), X.begin(), X.end());
        prior_p = &prior;
        points.resize(2, Vector(num_inputs));

        // initialize prior and realize GP
        min_x = X.front().front() - 0.1;
        max_x = X.back().front() + 0.1;
        update_points();

        // set up glut window
        opengl::Glut_window wnd(win_width, win_height, "Play with GP prior");
        wnd.set_display_callback(display);
        wnd.set_reshape_callback(reshape_window);
        wnd.set_keyboard_callback(process_key);

        glutMainLoop();
    }
    catch(const Exception& kex)
    {
        kex.print_details();
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

        case 'j':
        {
            --cur_param;
            cur_param = (cur_param + 2) % 2;
            break;
        }

        case 'k':
        {
            ++cur_param;
            cur_param = cur_param % 2;
            break;
        }

        case 'm':
        {
            --cur_mean;
            cur_mean = (cur_mean + 3) % 3;
            break;
        }

        case 'M':
        {
            ++cur_mean;
            cur_mean = cur_mean % 3;
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
    // stripped inputs
    Vector x;
    transform(
        prior_p->in_begin(),
        prior_p->in_end(),
        back_inserter(x),
        bind(
            static_cast<const double&(Vector::*)(int)const>(&Vector::at),
            _1, 0));

    // upate prior
    Vector m(x.size());;
    if(cur_mean == ZERO)
    {
        fill(m.begin(), m.end(), 0.0);
    }
    else if(cur_mean == IDENTITY)
    {
        copy(x.begin(), x.end(), m.begin());
    }
    else
    {
        transform(
            x.begin(),
            x.end(),
            m.begin(),
            boost::bind(
                static_cast<double(*)(double)>(sin),
                boost::bind(multiplies<double>(), 4.0, _1)));
        transform(
            m.begin(),
            m.end(),
            x.begin(),
            m.begin(),
            multiplies<double>());
    }

    gp::Manual mf(gp::Inputs(prior_p->in_begin(), prior_p->in_end()), m);
    prior_p->set_mean_function(mf);
    prior_p->set_covariance_function(gp::Sqex(scale, fsigma));

    // get vector of dimensions
    vector<Vector> points_t(2);
    points_t[0] = x;
    points_t[1] = gp::sample(*prior_p);

    vector<Vector> mean_t(2);
    mean_t[0] = x;
    mean_t[1] = m;

    // compute min and max for each dimension
    min_y = *min_element(points_t[1].begin(), points_t[1].end()) - 0.1;
    max_y = *max_element(points_t[1].begin(), points_t[1].end()) + 0.1;

    // "transpose" to convert into vector of positions
    points = get_transpose(points_t);
    mean_pts = get_transpose(mean_t);

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

    // draw gp
    glColor3f(0.0, 1.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINE_STRIP);
    for_each(
        points.begin(),
        points.end(),
        bind(static_cast<void(*)(const Vector&)>(opengl::glVertex), _1));
    glEnd();

    // draw mean
    glColor3f(0.3, 0.3, 0.3);
    glLineWidth(1.0);
    glBegin(GL_LINE_STRIP);
    for_each(
        mean_pts.begin(),
        mean_pts.end(),
        bind(static_cast<void(*)(const Vector&)>(opengl::glVertex), _1));
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

    glColor3f(0.2, 0.2, 0.7);
    temp_str = "Current mean: " + mean_names[cur_mean];
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

    temp_str = "Change mean function: m/M";
    bitmap_string(temp_str, str_pos_x, str_pos_y);
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

