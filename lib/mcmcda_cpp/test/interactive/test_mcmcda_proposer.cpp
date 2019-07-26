/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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

#include <mcmcda_cpp/mcmcda_data.h>
#include <mcmcda_cpp/mcmcda_association.h>
#include <mcmcda_cpp/mcmcda_proposer.h>
#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_glut.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_sample.h>
#include <l_cpp/l_functors.h>
#include <algorithm>
#include <vector>
#include <string>
#include <exception>
#include <limits>
#include <boost/program_options.hpp>
#include <boost/bind.hpp>

using namespace kjb;
using namespace kjb::opengl;
using namespace kjb::mcmcda;
using namespace std;
using namespace boost;

typedef Generic_track<Vector> Track;

// pointers to stuff
Association<Track>* w_ptr;
Proposer<Track>* proposer_ptr;
Glut_window* wnd_ptr;

// declarations
void reshape_window(int w, int h);
void display_scene();
void process_key(unsigned char key, int x, int y);
void process_options(int argc, char** argv);
set<Vector> random_frame_data(size_t frame);
void gl_circle(int x, int y, int r);
void gl_disk(int x, int y, int r);

Vector average_vector_ptrs(const vector<const Vector*>& vecs);

std::vector<double> feature_prob
(
    const Track*, 
    int t1, 
    const Vector* candidate,
    int t2, 
    size_t wsz
);

set<Vector> line_frame_data
(
    size_t frame, 
    const vector<Vector>& vels,
    const vector<Vector>& data_0
);

// global variables
const int win_width = 1000;
const int win_height = 600;

size_t num_frames;
size_t pts_per_frame;
int d_bar;
int b_bar;
double v_bar;
double noise_var;
double gamm;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

int main(int argc, char** argv)
{
    try
    {
        process_options(argc, argv);

        cout << "Generating random data..." << endl;
        Data<Vector> data;
        data.resize(num_frames);

        // create data from lines
        data[0] = random_frame_data(0);
        data[num_frames - 1] = random_frame_data(num_frames - 1);

        vector<Vector> data_0(data[0].begin(), data[0].end());
        vector<Vector> data_N(data[num_frames - 1].begin(),
                              data[num_frames - 1].end());
        random_shuffle(data_0.begin(), data_0.end());
        vector<Vector> vels(pts_per_frame);
        for(size_t i = 0; i < pts_per_frame; i++)
        {
            vels[i] = (data_N[i] - data_0[i]) / (num_frames - 1);
        }

        for(size_t f = 1; f < num_frames; f++)
        {
            data[f] = line_frame_data(f, vels, data_0);
        }

        // create data UAR
        //for(size_t f = 0; f < num_frames; f++)
        //{
        //    data[f] = random_frame_data(f);
        //}

        cout << "Creating MCMDA proposer..." << endl;
        vector<double> ps(MCMCDA_NUM_MOVES, 0.0);
        ps[MCMCDA_BIRTH] = 0.2;
        ps[MCMCDA_DEATH] = 0.1;
        ps[MCMCDA_EXTENSION] = 0.4;
        ps[MCMCDA_REDUCTION] = 0.1;
        ps[MCMCDA_SWITCH] = 0.05;
        ps[MCMCDA_SECRETION] = 0.1;
        ps[MCMCDA_ABSORPTION] = 0.05;

        Proposer<Track> proposer(
            Categorical_distribution<size_t>(ps, 0),
            v_bar,
            d_bar,
            b_bar,
            gamm,
            Identity<Vector>(),
            average_vector_ptrs,
            feature_prob,
            noise_var,
            Track());
        cout << endl;

        cout << "Starting MCMCDA proposer tester..." << endl;
        Glut_window wnd(win_width, win_height, "MCMCDA proposer");
        wnd.set_reshape_callback(reshape_window);
        wnd.set_display_callback(display_scene);
        wnd.set_keyboard_callback(process_key);
        cout << endl;

        // initial association global pointers
        Association<Track> w(data);
        w_ptr = &w;
        proposer_ptr = &proposer;
        wnd_ptr = &wnd;

        glutMainLoop();
    }
    catch(const std::exception& ex)
    {
        cout << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch(const Exception& kex)
    {
        kex.print_details();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void reshape_window(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)w, 0.0, (GLdouble)h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display_scene()
{
    const Association<Track>& w = *w_ptr;
    const int pt_size = 3;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int vpi[4];
    glGetIntegerv(GL_VIEWPORT, vpi);
    size_t cur_width = vpi[2];
    size_t cur_height = vpi[3];

    BOOST_FOREACH(const set<Vector>& data_t, w.get_data())
    {
        BOOST_FOREACH(const Vector& v, data_t)
        {
            glColor3d(1.0, 0.0, 0.0);
            gl_disk(cur_width * v[0], cur_height * v[1], pt_size);

            glColor3d(0.1, 0.1, 0.1);
            glBegin(GL_LINES);
            glVertex2d(cur_width * (v[0] + 0.5/num_frames), 0);
            glVertex2d(cur_width * (v[0] + 0.5/num_frames), cur_height);
            glEnd();
        }
    }

    glColor3d(0.0, 1.0, 0.0);
    glLineWidth(2.0);
    BOOST_FOREACH(const Track& track, w)
    {
        Track::const_iterator pair_p = track.begin();
        gl_disk(win_width * (*pair_p->second)[0],
                win_height * (*pair_p->second)[1], pt_size);

        Track::const_iterator pair_q = pair_p;
        for(pair_q++; pair_q != track.end(); pair_p++, pair_q++)
        {
            Vector vp, vq;
            vp.set(win_width * (*pair_p->second)[0],
                   win_height * (*pair_p->second)[1]);
            vq.set(win_width * (*pair_q->second)[0],
                   win_height * (*pair_q->second)[1]);

            gl_disk(vq[0], vq[1], pt_size);

            glBegin(GL_LINES);
            glVertex(vp);
            glVertex(vq);
            glEnd();
        }
    }

    glutSwapBuffers();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_key(unsigned char key, int /*x*/, int /*y*/)
{
    Association<Track>& w = *w_ptr;
    Association<Track> w_p;
    const Proposer<Track>& proposer = *proposer_ptr;
    double fwd, rev;

    switch(key)
    {
        case 'b':
        {
            fwd = proposer.propose_birth(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            rev = proposer.p_death(w_p, w);
            if(rev == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_BIRTH, w);
            rev += proposer.move_log_pdf(MCMCDA_DEATH, w_p);

            break;
        }

        case 'd':
        {
            fwd = proposer.propose_death(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            rev = proposer.p_birth(w_p, w);
            if(rev == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_DEATH, w);
            rev += proposer.move_log_pdf(MCMCDA_BIRTH, w_p);

            break;
        }

        /*case 's':
        {
            fwd = proposer.propose_split(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            rev = proposer.p_merge(w_p, w);
            if(rev == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_SPLIT, w);
            rev += proposer.move_log_pdf(MCMCDA_MERGE, w_p);

            break;
        }

        case 'm':
        {
            fwd = proposer.propose_merge(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            rev = proposer.p_split(w_p, w);
            if(rev == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_MERGE, w);
            rev += proposer.move_log_pdf(MCMCDA_SPLIT, w_p);

            break;
        }*/

        case 'e':
        {
            fwd = proposer.propose_extension(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            rev = proposer.p_reduction(w_p, w);
            if(rev == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_EXTENSION, w);
            rev += proposer.move_log_pdf(MCMCDA_REDUCTION, w_p);

            break;
        }

        case 'r':
        {
            fwd = proposer.propose_reduction(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            rev = proposer.p_extension(w_p, w);
            if(rev == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_REDUCTION, w);
            rev += proposer.move_log_pdf(MCMCDA_EXTENSION, w_p);

            break;
        }

        case 'w':
        {
            fwd = proposer.propose_switch(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_SWITCH, w);
            rev = fwd;

            break;
        }

        case 'k':
        {
            fwd = proposer.propose_secretion(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            rev = proposer.p_absorption(w_p, w);
            if(rev == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_SECRETION, w);
            rev += proposer.move_log_pdf(MCMCDA_ABSORPTION, w_p);

            break;
        }

        case 'p':
        {
            fwd = proposer.propose_absorption(w, w_p);
            if(fwd == -numeric_limits<double>::max())
            {
                break;
            }

            rev = proposer.p_secretion(w_p, w);
            if(rev == -numeric_limits<double>::max())
            {
                break;
            }

            fwd += proposer.move_log_pdf(MCMCDA_ABSORPTION, w);
            rev += proposer.move_log_pdf(MCMCDA_SECRETION, w_p);

            break;
        }

        case 'o':
        {
            w.write("w.txt");
            cout << "Wrote association to 'w.txt'." << endl;
            return;
        }

        case 'i':
        {
            w.read("w.txt");
            cout << "Read association from 'w.txt'." << endl;
            wnd_ptr->redisplay();
            return;
        }

        case 'q':
        {
            cout << "Bye..." << endl;
            exit(EXIT_SUCCESS);
        }

        default:
        {
            return;
        }
    }

    if(fwd == -numeric_limits<double>::max()
            || rev == -numeric_limits<double>::max())
    {
        cout << "    proposal failed; probability is 0.\n\n";
    }
    else if(isinf(fwd) || isinf(rev))
    {
        cout << "    (fwd, rev) = (" << fwd << ", " << rev << ").\n";
        cout << "    proposal failed; you pressed the wrong key.\n\n";
    }
    else
    {
        cout << "    success!\n";
        cout << "    fwd prob is " << fwd << "\n";
        cout << "    rev prob is " << rev << "\n\n";

        using std::swap;
        swap(w, w_p);
    }

    wnd_ptr->redisplay();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_options(int argc, char** argv)
{
    namespace bpo = program_options;

    bpo::options_description cmdline_options("General options");
    bpo::variables_map vm;

    try
    {
        // All options
        cmdline_options.add_options()
            ("help,h", "produce help message")
            ("num-frames", bpo::value<size_t>(&num_frames)->default_value(20),
                           "number of frames")
            ("points-per-frame",
                bpo::value<size_t>(&pts_per_frame)->default_value(5),
                "data points per frame to be created")
            ("d-bar", bpo::value<int>(&d_bar)->default_value(5),
                      "maximum skipped frames")
            ("b-bar", bpo::value<int>(&b_bar)->default_value(5),
                      "maximum skipped frames when growing")
            ("gamma", bpo::value<double>(&gamm)->default_value(0.1),
                      "probability of halting growing a proposed track");

        bpo::store(bpo::command_line_parser(argc, argv)
                   .options(cmdline_options).run(), vm);
        bpo::notify(vm);
    }
    catch(const bpo::error& err)
    {
        throw Exception(err.what());
    }    
    catch(const std::exception& ex)
    {
        throw ex;
    }    

    if(vm.count("help"))
    {
        cout << "Usage: mcmcda_proposing_tool OPTIONS\n"
             << "Manually run MCMCDA moves."
             << cmdline_options << "\n"
             << "For questions or complaints see Ernesto." << endl;

        exit(EXIT_SUCCESS);
    }

    v_bar = 1.0 / (2*num_frames);
    noise_var = (0.5e-2)*(0.5e-2);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

set<Vector> random_frame_data(size_t frame)
{
    set<Vector> data_t;
    Uniform_distribution U;
    for(size_t i = 0; i < pts_per_frame; i++)
    {
        double x = (frame + 0.5) / num_frames;
        double y = sample(U);
        data_t.insert(Vector().set(x, y));
    }

    return data_t;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

set<Vector> line_frame_data
(
    size_t frame, 
    const vector<Vector>& vels,
    const vector<Vector>& data_0
)
{
    set<Vector> data_t;
    assert(vels.size() == data_0.size());
    for(size_t i = 0; i < pts_per_frame; i++)
    {
        data_t.insert(data_0[i] + frame*vels[i]);
    }

    return data_t;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void gl_circle(int x, int y, int r)
{
    const size_t num_cir_pts = 36;

    glBegin(GL_LINE_LOOP);
    for(size_t i = 0; i < num_cir_pts; i++)
    {
        double t = i * (2*M_PI / num_cir_pts);
        glVertex2d(x + r * std::cos(t), y + r * std::sin(t));
    }
    glEnd();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void gl_disk(int x, int y, int r)
{
    const size_t num_cir_pts = 36;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(x, y);
    for(size_t i = 0; i <= num_cir_pts; i++)
    {
        double t = i * (2*M_PI / num_cir_pts);
        glVertex2d(x + r * std::cos(t), y + r * std::sin(t));
    }
    glEnd();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector average_vector_ptrs(const vector<const Vector*>& vecs)
{
    Vector res(0.0, 0.0);
    for(size_t i = 0; i < vecs.size(); i++)
    {
        res += *vecs[i];
    }

    if(vecs.size() > 0)
    {
        res /= vecs.size();
    }

    return res;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

std::vector<double> feature_prob
(
    const Track*, 
    int t1, 
    const Vector* candidate,
    int t2, 
    size_t wsz
)
{
    std::vector<double> f_p;
    return f_p;
}

