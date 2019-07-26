/*
 * $Id: generate_lsl_data.cpp 15736 2013-10-19 05:44:01Z predoehl $
 */
#include <l/l_sys_lib.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_iterator.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <edge_cpp/line_segment.h>
#include <likelihood_cpp/line_segment_likelihood.h>
#include <m_cpp/m_vector.h>

#include <list>
#include <vector>
#include <algorithm>
#include <functional>
#include <boost/bind.hpp>

using namespace kjb;

// typedefs
typedef std::list<Line_segment> Segment_list;
typedef Line_segment_likelihood::Segment_vector Segment_vector;

// interface constants
const int win_width = 700;
const int win_height = 700;
const double trans_inc = 2.0;
const double rot_inc = M_PI / 32.0;
const double growth_inc = 4.0;
const size_t d_segs_per_m_seg = 5;

// likelihood parameters
double L = 10;
double p = 2.0;
double q = 5.0;
double r = 3 * M_PI / 2.0;

// medges and iterators...
Segment_list* model_segments;
Segment_vector* data_segments;
circular_iterator<Segment_list::iterator> active_seg_p;

// Is data showing
bool show_data;

// stipple masks
GLushort solid_stipple = 0xFFFF;
const size_t num_stipples = 4;
GLushort stipples[num_stipples] = {0x00FF, 0xF00F, 0xFF00, 0x0FF0};

circular_iterator<GLushort*> cur_stipple(stipples, stipples + num_stipples);

// callbacks
void display_scene();
void reshape_window(int, int);
void process_key(unsigned char, int, int);
void process_idle();

// helpers
void display_segment(const Line_segment&);

int main(int, char**)
{
    // set up window and callbacks
    opengl::Glut_window win(win_width, win_height);
    win.set_display_callback(display_scene);
    win.set_reshape_callback(reshape_window);
    win.set_keyboard_callback(process_key);
    win.set_idle_callback(process_idle);

    // init opengl with stipple and anti-aliasing
    opengl::default_init_opengl(win_width, win_height);
    glEnable(GL_LINE_STIPPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

    // set up initial medges and iterators
    model_segments = new Segment_list;
    data_segments = new Segment_vector;
    active_seg_p = make_circular_iterator(*model_segments);
    show_data = false;

    // start execution
    glutMainLoop();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display_scene()
{
    try
    {
        if(show_data)
        {
        }

        // start the rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glLineWidth(2.0);

        // render all model segments
        glColor3d(0.0, 0.0, 1.0);
        std::for_each(model_segments->begin(), model_segments->end(),
                      std::ptr_fun(display_segment));

        if(show_data)
        {
            // render data chains
            glColor3d(1.0, 1.0, 0.0);
            glLineStipple(1, solid_stipple);
            std::for_each(data_segments->begin(), data_segments->end(),
                          std::mem_fun_ref(&Line_segment::wire_render));
        }
    }
    catch(const Exception& ex)
    {
        ex.print_details();
    }

    glutSwapBuffers();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void reshape_window(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, win_width, win_height, 0.0);
    glMatrixMode(GL_MODELVIEW);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_key(unsigned char key, int, int)
{
    try
    {
        switch(key)
        {
            // add model segment
            case 'm':
            {
                double scx = win_width / 2.0;
                double scy = win_height / 2.0;
                const double medge_length = 250.0;
                Vector stpt = Vector().set(scx - medge_length / 2.0, scy, 1.0);
                Vector enpt = Vector().set(scx + medge_length / 2.0, scy, 1.0);

                Line_segment default_medge(stpt, enpt);
                model_segments->push_back(default_medge);
                active_seg_p = make_circular_iterator(*model_segments);
                active_seg_p--;
                break;
            }

            // remove active
            case 'd':
            {
                if(!model_segments->empty())
                {
                    Segment_list::iterator medge_p = active_seg_p.get_iterator();
                    active_seg_p++;
                    model_segments->erase(medge_p);
                }
                break;
            }

            // translate active
            case 'h':
            {
                if(!model_segments->empty())
                {
                    active_seg_p->translate(Vector().set(-trans_inc, 0.0));
                }
                break;
            }

            case 'l':
            {
                if(!model_segments->empty())
                {
                    active_seg_p->translate(Vector().set(trans_inc, 0.0));
                }
                break;
            }

            case 'j':
            {
                if(!model_segments->empty())
                {
                    active_seg_p->translate(Vector().set(0.0, trans_inc));
                }
                break;
            }

            case 'k':
            {
                if(!model_segments->empty())
                {
                    active_seg_p->translate(Vector().set(0.0, -trans_inc));
                }
                break;
            }

            // rotate active
            case 'r':
            {
                if(!model_segments->empty())
                {
                    active_seg_p->rotate(-rot_inc);
                }
                break;
            }

            case 'R':
            {
                if(!model_segments->empty())
                {
                    active_seg_p->rotate(rot_inc);
                }
                break;
            }

            // change active
            case 'n':
            {
                if(!model_segments->empty())
                {
                    active_seg_p++;
                }
                break;
            }

            // grow if active is medge
            case 'G':
            {
                if(!model_segments->empty())
                {
                    const Vector& ac_center = active_seg_p->get_centre();
                    double ac_orient = active_seg_p->get_orientation();
                    double ac_len = active_seg_p->get_length();
                    active_seg_p->init_from_centre_and_orientation(ac_center[0],
                                  ac_center[1], ac_orient, ac_len + growth_inc);
                }

                break;
            }

            // shrink if active is medge
            case 'g':
            {
                if(!model_segments->empty())
                {
                    const Vector& ac_center = active_seg_p->get_centre();
                    double ac_orient = active_seg_p->get_orientation();
                    double ac_len = active_seg_p->get_length();
                    active_seg_p->init_from_centre_and_orientation(ac_center[0],
                                  ac_center[1], ac_orient, ac_len - growth_inc);
                }

                break;
            }

            // update data
            case 'u':
            {
                // Create likelihood and sample data
                Line_segment_likelihood likelihood(L, p, q, r);
                Segment_vector model_segs(model_segments->begin(),
                                          model_segments->end());

                data_segments->resize(model_segs.size() * d_segs_per_m_seg);
                std::generate(data_segments->begin(), data_segments->end(),
                              boost::bind(&Line_segment_likelihood::sample,
                                          likelihood, model_segs));
                break;
            }

            // toggle show data
            case 's':
            {
                show_data = !show_data;
                break;
            }

            // quit
            case 'q':
            {
                delete model_segments;
                delete data_segments;

                kjb_c::kjb_exit(0);
            }

            default: return;
        }
    }
    catch(const Exception& ex)
    {
        ex.print_details();
    }

    glutPostRedisplay();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_idle()
{
    static size_t idle_count = 0;

    if(++idle_count > 40000)
    {
        cur_stipple++;
        idle_count = 0;

        glutPostRedisplay();
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display_segment
(
    const Line_segment& segment
)
{
    if(!model_segments->empty() && &segment == &(*active_seg_p))
    {
        glLineStipple(1, *cur_stipple);
    }
    else
    {
        glLineStipple(1, solid_stipple);
    }

    segment.wire_render();
}

