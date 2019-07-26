#include <l/l_sys_lib.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_iterator.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_glut.h>
#include <edge_cpp/line_segment.h>
#include <edge_cpp/collinear_segment_chain.h>
#include <m_cpp/m_vector.h>

#include <list>
#include <algorithm>
#include <functional>
#include <string>
#include <sstream>

using namespace kjb;

// typedefs
typedef std::list<Line_segment> Segment_list;
typedef std::vector<Collinear_segment_chain> Chain_vector;

// likelihood parameters
double distance_threshold = 10.0;
double orientation_threshold = M_PI / 12.0;
double gap_threshold = 100.0;

// other constants
const int win_width = 700;
const int win_height = 700;
const double trans_inc = 5.0;
const double rot_inc = M_PI / 32.0;

// medges and iterators...
Segment_list* segments;
circular_iterator<Segment_list::iterator> active_seg_p;

// Is data showing
bool show_chains;

// stipple masks
GLushort solid_stipple = 0xFFFF;
GLushort corr_stipple = 0x3333;
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
void display_chain(const Collinear_segment_chain&);
void display_GL_bitmap_string(const std::string&, int, int);

int main(int, char**)
{
    // set up window and callbacks
    opengl::Glut_window win(win_width, win_height);
    win.set_display_callback(display_scene);
    win.set_reshape_callback(reshape_window);
    win.set_keyboard_callback(process_key);
    //win.set_idle_callback(process_idle);

    // init opengl with stipple and anti-aliasing
    opengl::default_init_opengl(win_width, win_height);
    glEnable(GL_LINE_STIPPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

    // set up initial medges and iterators
    segments = new Segment_list;
    active_seg_p = make_circular_iterator(*segments);
    show_chains = false;

    // start execution
    glutMainLoop();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display_scene()
{
    try
    {
        // start the rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // render chains if supposed to
        Chain_vector chains;
        if(show_chains)
        {
            chains = find_collinear_segment_chains(segments->begin(),
                                                   segments->end(),
                                                   distance_threshold,
                                                   orientation_threshold,
                                                   gap_threshold);

            glLineStipple(1, solid_stipple);
            std::for_each(chains.begin(), chains.end(),
                          std::ptr_fun(display_chain));
        }

        // render segments
        glColor3d(1.0, 0.0, 0.0);
        glLineWidth(3.0);
        std::for_each(segments->begin(), segments->end(),
                      std::ptr_fun(display_segment));

        std::stringstream info_stream;
        info_stream << segments->size() << " segments";
        display_GL_bitmap_string(info_stream.str(), win_width / 2.0 - 150,
                                                    -win_height / 2.0 + 100);
        info_stream.str("");
        info_stream << chains.size() << " chains: ";
        for(size_t i = 0; i < chains.size(); i++)
        {
            info_stream << chains[i].get_segments().size() << ", ";
        }

        display_GL_bitmap_string(info_stream.str(), win_width / 2.0 - 150,
                                                    -win_height / 2.0 + 80);
    }
    catch(const kjb::Exception& ex)
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
    gluOrtho2D(-win_width / 2.0, win_width / 2.0,
               -win_height / 2.0, win_height / 2.0);
    glMatrixMode(GL_MODELVIEW);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_key(unsigned char key, int, int)
{
    try
    {
        show_chains = false;
        switch(key)
        {
            // add model segment
            case 'm':
            {
                const double seg_length = 250.0;
                Vector stpt = Vector().set(-seg_length / 2.0, 0.0, 1.0);
                Vector enpt = Vector().set(seg_length / 2.0, 0.0, 1.0);

                Line_segment default_seg(stpt, enpt);
                segments->push_back(default_seg);
                active_seg_p = make_circular_iterator(*segments);
                active_seg_p--;
                break;
            }

            // remove active
            case 'd':
            {
                Segment_list::iterator seg_p = active_seg_p.get_iterator();
                active_seg_p++;
                segments->erase(seg_p);
                break;
            }

            // translate active
            case 'h':
            {
                if(!segments->empty())
                {
                    active_seg_p->translate(Vector().set(-trans_inc, 0.0));
                }
                break;
            }

            case 'l':
            {
                if(!segments->empty())
                {
                    active_seg_p->translate(Vector().set(trans_inc, 0.0));
                }
                break;
            }

            case 'j':
            {
                if(!segments->empty())
                {
                    active_seg_p->translate(Vector().set(0.0, -trans_inc));
                }
                break;
            }

            case 'k':
            {
                if(!segments->empty())
                {
                    active_seg_p->translate(Vector().set(0.0, trans_inc));
                }
                break;
            }

            // rotate active
            case 'r':
            {
                if(!segments->empty())
                {
                    active_seg_p->rotate(-rot_inc);
                }
                break;
            }

            case 'R':
            {
                if(!segments->empty())
                {
                    active_seg_p->rotate(rot_inc);
                }
                break;
            }

            // change active
            case 'n':
            {
                if(!segments->empty())
                {
                    active_seg_p++;
                }
                break;
            }

            // grow active
            case 'G':
            {
                if(!segments->empty())
                {
                    const Vector& ac_center = active_seg_p->get_centre();
                    double ac_orient = active_seg_p->get_orientation();
                    double ac_len = active_seg_p->get_length();
                    active_seg_p->init_from_centre_and_orientation(ac_center[0],
                                                                   ac_center[1],
                                                                   ac_orient,
                                                                   ac_len + 5.0);
                }
                break;
            }

            // shrink if active is medge
            case 'g':
            {
                if(!segments->empty())
                {
                    const Vector& ac_center = active_seg_p->get_centre();
                    double ac_orient = active_seg_p->get_orientation();
                    double ac_len = active_seg_p->get_length();
                    active_seg_p->init_from_centre_and_orientation(ac_center[0],
                                                                   ac_center[1],
                                                                   ac_orient,
                                                                   ac_len - 5.0);
                }

                break;
            }

            // show chains
            case 'c':
            {
                show_chains = true;
                break;
            }

            // quit
            case 'q':
            {
                delete segments;

                kjb_c::kjb_exit(0);
            }

            default: return;
        }
    }
    catch(const kjb::Exception& ex)
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
    const Line_segment& seg
)
{
    if(&seg == &(*active_seg_p))
    {
        glLineStipple(1, *cur_stipple);
    }
    else
    {
        glLineStipple(1, solid_stipple);
    }

    seg.wire_render();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display_chain(const Collinear_segment_chain& chain)
{
    /* glLineWidth(5.0);
    glColor3d(0.0, 0.0, 1.0);
    chain.Line_segment::wire_render(); */

    glLineWidth(1.5);
    glColor3d(1.0, 1.0, 0.0);
    chain.wire_render();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display_GL_bitmap_string(const std::string& s, int x, int y)
{
    glRasterPos2i(x, y);
    for(size_t i = 0; i < s.length(); i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, s.c_str()[i]);
    }
}

