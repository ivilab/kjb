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

/* $Id$ */

#include <gr_cpp/gr_opengl.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <gr_cpp/gr_opengl_headers.h>
#include <gr_cpp/gr_sprite.h>
#include <detector_cpp/d_bbox.h>
#include <flow_cpp/flow_integral_flow.h>
#include <i_cpp/i_image.h>
#include <i_cpp/i_hsv.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_filesystem.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <boost/optional.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::opengl;

// types
const size_t IMAGE = 0;
const size_t FLOW = 1;

// globals
vector<string> imgs_fps ;
vector<Integral_flow> flows_x;
vector<Integral_flow> flows_y;
size_t width;
size_t height;
size_t cur_frame;
size_t mode;
Image flow_image;
Vector box_click;
boost::optional<Bbox> avg_box;
Vector avg_flow;

/** @brief  Handle key input. */
void handle_key(unsigned char, int, int);

/** @brief  Handle mouse click. */
void handle_click(int, int, int, int);

/** @brief  Handle mouse motion. */
void handle_motion(int, int);

/** @brief  Reshape window. */
void reshape(int, int);

/** @brief  Render frame. */
void display();

/** @brief  Update flow image from current flow data. */
void update_image();

/** @brief  Convert flow direction to color. */
PixelHSVA dir_to_color(const Vector&);

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    if(argc != 2)
    {
        cout << "Usage: flow_viewer DATA-PATH" << endl;
        return EXIT_FAILURE;
    }

    try
    {
        string data_dp = argv[1];
        string imgs_dp = data_dp + "/frames";
        string flow_dp = data_dp + "/features/optical_flow/brox";
        string xf_dp = flow_dp + "/x_int_s4";
        string yf_dp = flow_dp + "/y_int_s4";

        // read data
        imgs_fps = file_names_from_format(imgs_dp + "/%05d.jpg");
        vector<string> x_of_fps = file_names_from_format(xf_dp + "/%05d.txt");
        vector<string> y_of_fps = file_names_from_format(yf_dp + "/%05d.txt");

        flows_x.assign(x_of_fps.begin(), x_of_fps.end());
        flows_y.assign(y_of_fps.begin(), y_of_fps.end());

        IFT(
            !flows_x.empty() && flows_x.size() == flows_y.size(),
            Illegal_argument,
            "X and Y flows are either empty or differ in size.");

        // GLUT window
        width = flows_x.front().img_width();
        height = flows_x.front().img_height();
        Glut_window glwin(width, height, "Integral flow viewer");
        glwin.set_display_callback(display);
        glwin.set_reshape_callback(reshape);
        glwin.set_keyboard_callback(handle_key);
        glwin.set_mouse_callback(handle_click);
        glwin.set_motion_callback(handle_motion);

        // initial setup
        cur_frame = 0;
        flow_image = Image(height, width);
        avg_box = boost::none;
        avg_flow.set(0.0, 0.0);
        update_image();

        // start rendering
        glutMainLoop();
    }
    catch(const kjb::Exception& kex)
    {
        kex.print_details();
        cout << endl;
        return EXIT_FAILURE;
    }
    catch(const std::exception& sex)
    {
        cout << sex.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_key(unsigned char k, int, int)
{
    switch(k)
    {
        case 'n':
        {
            if(++cur_frame == flows_x.size()) cur_frame = 0;
            update_image();
            break;
        }

        case 'p':
        {
            if(cur_frame-- == 0) cur_frame = flows_x.size();
            update_image();
            break;
        }

        case 'i':
        {
            mode = mode == IMAGE ? FLOW : IMAGE;
            update_image();
            break;
        }

        case 'x':
        {
            avg_box = boost::none;
            break;
        }

        case 'q':
        case 'Q': exit(EXIT_SUCCESS);
        default: break;
    }

    glutPostRedisplay();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_click(int but, int state, int x, int y)
{
    if(but == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            box_click.set(x, y);
        }
    }
    else if(but == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            avg_box = boost::none;
            glutPostRedisplay();
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void handle_motion(int x, int y)
{
    avg_box = Bbox(box_click, Vector().set(x, y));

    avg_flow.set(
        flows_x[cur_frame].flow_sum(*avg_box),
        flows_y[cur_frame].flow_sum(*avg_box));

    if(avg_box->get_area() != 0) avg_flow /= avg_box->get_area();

    glutPostRedisplay();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // render flow image
    Sprite sprite(flow_image, 0.0);
    sprite.render();

    // draw box and arrow
    if(avg_box)
    {
        glColor(dir_to_color(avg_flow));
        glBegin(GL_QUADS);
            glVertex3d(avg_box->get_left(), avg_box->get_top(), 0.1);
            glVertex3d(avg_box->get_right(), avg_box->get_top(), 0.1);
            glVertex3d(avg_box->get_right(), avg_box->get_bottom(), 0.1);
            glVertex3d(avg_box->get_left(), avg_box->get_bottom(), 0.1);
        glEnd();

        glColor3d(0.0, 0.0, 0.0);
        render_arrow(
            avg_box->get_center(),
            avg_box->get_center() + 10*avg_flow);
    }

    glutSwapBuffers();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void update_image()
{
    if(mode == IMAGE)
    {
        flow_image = Image(imgs_fps[cur_frame]);
        return;
    }

    // else
    for(size_t i = 0; i < height; ++i)
    {
        for(size_t j = 0; j < width; ++j)
        {
            // flow
            double ii = i;
            double jj = j;
            Bbox box(Vector(jj, ii), Vector(jj + 1, ii + 1));

            double fx = flows_x[cur_frame].flow_sum(box);
            double fy = flows_y[cur_frame].flow_sum(box);
            flow_image(i, j) = dir_to_color(Vector(fx, fy));
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

PixelHSVA dir_to_color(const Vector& dir)
{
    const double max_sat = 5.0;

    double s = norm2(dir);
    s = std::min(s / max_sat, 1.0);

    double h = atan2(dir[1], dir[0]);
    if(h < 0) h += 2*M_PI;
    h /= 2*M_PI;

    return PixelHSVA(h, s, 1.0);
}

