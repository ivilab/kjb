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

/* $Id$ */

#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <gr_cpp/gr_primitive.h>
#include <gp_cpp/gp_covariance.h>
#include <gp_cpp/gp_mean.h>
#include <gp_cpp/gp_base.h>
#include <gp_cpp/gp_normal.h>
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
#include <boost/lexical_cast.hpp>

using namespace kjb;
using namespace std;
using boost::bind;

// types
typedef gp::Posterior<gp::Zero, gp::Sqex, gp::Linear_gaussian> Gpp;
const int SCALE = 0;
const int FSIGMA = 1;
const int NSIGMA = 2;

// declarations
//// glut functions
void reshape_window(int, int);
void display();
void process_key(unsigned char, int, int);

//// other crap
void update_points();
void generate_data();
void draw_gp();
void draw_sidebar();

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

// variables that need to be global (accessed from several places)
Gpp* post_p;
gp::Inputs ins;
Vector f;
Vector y;
Vector fs;
double gt_scale = 2;
double gt_svar = 20;
double gt_nsigma = 0.5;
double scale = 1;
double svar = 1;
double nsigma = 1e-1;
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
string param_names[] = { "scale", "signal variance", "noise variance" };
double* params[] = { &scale, &svar, &nsigma };
int cur_param = SCALE;

bool points_up_to_date = false;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

int main(int argc, char** argv)
{
    try
    {
        // create data
        const size_t T = 10;
        ins = gp::make_inputs(0, T, 0.1);
        generate_data();

        // create posterior
        gp::Zero mf;
        gp::Sqex cf(scale, svar);
        Gpp post = gp::make_posterior(mf, cf, nsigma, ins, y);
        post_p = &post;
        fs.resize(f.size());

        // initialize posterior and realize GP
        min_x = ins.front().front() - 0.1;
        max_x = ins.back().front() + 0.1;
        min_y = *min_element(y.begin(), y.end()) - 0.1;
        max_y = *max_element(y.begin(), y.end()) + 0.1;

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

        case 'g':
        {
            generate_data();
            post_p->set_outputs(y.begin(), y.end());

            min_y = *min_element(y.begin(), y.end()) - 0.1;
            max_y = *max_element(y.begin(), y.end()) + 0.1;

            points_up_to_date = false;

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
    if(points_up_to_date) return;

    size_t N = std::distance(post_p->in_begin(), post_p->in_end());
    post_p->set_covariance_function(gp::Sqex(scale, svar));
    post_p->set_likelihood(gp::Linear_gaussian(nsigma, N));

    fs = post_p->normal().get_mean();

    points_up_to_date = true;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void generate_data()
{
    const size_t N = ins.size();
#if KJB_CXX11_FLAGS
    auto prior = gp::make_prior(gp::Zero(), gp::Sqex(gt_scale, gt_svar), ins);
#else 
    gp::Prior<gp::Zero, gp::Sqex>  prior = gp::make_prior(gp::Zero(), gp::Sqex(gt_scale, gt_svar), ins);
#endif

    Normal_distribution lhood(0.0, gt_nsigma);
    f = gp::sample(prior);
    y = f;
    for(size_t i = 0; i < N; ++i)
    {
        y[i] += kjb::sample(lhood);
    }
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
    for(size_t i = 0; i < ins.size(); ++i)
    {
        glPushMatrix();
            glTranslated(ins[i].front(), y[i], 0.0);
            ellipse.solid_render();
        glPopMatrix();
    }

    // draw gt curve
    glColor3f(0.3, 0.3, 0.3);
    glLineWidth(1.0);
    glBegin(GL_LINE_STRIP);
    for(size_t i = 0; i < ins.size(); ++i)
    {
        glVertex2d(ins[i].front(), f[i]);
    }
    glEnd();

    // draw gp
    glColor3f(0.0, 1.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINE_STRIP);
    for(size_t i = 0; i < ins.size(); ++i)
    {
        glVertex2d(ins[i].front(), fs[i]);
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
    temp_str = param_names[FSIGMA] + ": " + boost::str(fmt % svar);
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    if(cur_param == NSIGMA) glColor3f(1.0, 0.0, 0.0);
    else glColor3f(0.2, 0.2, 0.7);
    temp_str = param_names[NSIGMA] + ": " + boost::str(fmt % nsigma);
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

    temp_str = "Generate data: g";
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    glColor3f(0.2, 0.2, 0.7);
    bitmap_string("Quit: q", str_pos_x, str_pos_y);
    str_pos_y += line_width;

    // draw separating line
    glColor3f(0.2, 0.2, 0.7);
    glBegin(GL_LINES);
    glVertex2i(0.0, str_pos_y);
    glVertex2i(info_vp_width, str_pos_y);
    glEnd();
    str_pos_y += line_width;

    // draw text for GT info
    //temp_str = "GT scale: " + std::to_string(gt_scale);
    temp_str = "GT scale: " + boost::lexical_cast<std::string>(gt_scale);
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    //temp_str = "GT signal variance: " + std::to_string(gt_svar);
    temp_str = "GT signal variance: " + boost::lexical_cast<std::string>(gt_svar);
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;

    //temp_str = "GT noise stddev: " + std::to_string(gt_nsigma);
    temp_str = "GT noise stddev: " + boost::lexical_cast<std::string>(gt_nsigma);
    bitmap_string(temp_str, str_pos_x, str_pos_y);
    str_pos_y += line_width;
}

