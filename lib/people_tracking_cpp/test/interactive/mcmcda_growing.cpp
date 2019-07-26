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

/* $Id: mcmcda_growing.cpp 18808 2015-04-08 17:04:54Z jguan1 $ */

#include <people_tracking_cpp/pt_association.h>
#include <mcmcda_cpp/mcmcda_proposer.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_data.h>
#include <gr_cpp/gr_opengl.h>
#include <gr_cpp/gr_opengl_headers.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <l_cpp/l_filesystem.h>
#include <i_cpp/i_image.h>
#include <detector_cpp/d_bbox.h>
#include <string>
#include <sstream>
#include <exception>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/lexical_cast.hpp>

using namespace kjb;
using namespace kjb::pt;
using namespace kjb::opengl;
using namespace kjb::mcmcda;
using namespace std;
using namespace boost;

typedef bimap<multiset_of<double, greater<double> >,
              set_of<const Detection_box*> > Score_map;

// pointers to stuff
#ifdef KJB_HAVE_GLUT
const Glut_window* wnd_p;
#endif
const Box_data* data_p;
const Proposer<Target>* proposer_p;
Target* tg_p;
const vector<string>* frame_imgs_p;

// declarations
void reshape_window(int w, int h);
void display_scene();
void process_key(unsigned char key, int x, int y);
void draw_box(const Bbox&);
void draw_ellipse(const Vector& c, double rx, double ry, double inc);
Vector get_dbox_center(const Detection_box& dbox);
Detection_box average_box_centers(const vector<const Detection_box*>& boxes);
void process_options(int argc, char** argv);

// options
string movie_dp;
string img_out_dp;
double img_width;
double img_height;
double v_bar;
int d_bar;
int b_bar;
double noise_variance;
double gamm;
int N;

// global variables
size_t cur_frame;
Box_data::Box_set::const_iterator cur_box_p;

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

int main(int argc, char** argv)
{
    try
    {
        process_options(argc, argv);

        // get image filenames
        vector<string> frame_imgs
            = file_names_from_format(movie_dp + "/frames/%05d.jpg");

        assert(frame_imgs.size() > 0);
        Image temp_img(frame_imgs[0]);

        img_width = temp_img.get_num_cols();
        img_height = temp_img.get_num_rows();

        // read data boxes
        Box_data data(img_width, img_height, 1.0);
        data.read(file_names_from_format(
            movie_dp + "/detection_boxes/person_deva/%05d.txt"));

        // create empty target
        Target tg(1.7, 0.4, 0.3, data.size());

        // create proposer
        Proposer<Target> proposer(
            Categorical_distribution<size_t>(
                MCMCDA_BIRTH, MCMCDA_NUM_MOVES - 1, 1),
            v_bar,
            d_bar,
            b_bar,
            gamm,
            bind(get_dbox_center, _1),
            average_box_centers,
            noise_variance,
            tg);

        // create GL environment
#ifdef KJB_HAVE_GLUT
        Glut_window wnd(img_width, img_height, "MCMCDA growth");
        wnd.set_reshape_callback(reshape_window);
        wnd.set_display_callback(display_scene);
        wnd.set_keyboard_callback(process_key);
#endif
        glEnable(GL_LINE_STIPPLE);

        // global pointers
#ifdef KJB_HAVE_GLUT
        wnd_p = &wnd;
#endif
        data_p = &data;
        proposer_p = &proposer;
        tg_p = &tg;
        frame_imgs_p = &frame_imgs;

        // other stuff
        cur_frame = 1;
        cur_box_p = data[cur_frame - 1].begin();

        // screen shot counter 
        kjb_c::kjb_mkdir(img_out_dp.c_str());
        N = 1; 
#ifdef KJB_HAVE_GLUT
        glutMainLoop();
#endif 
    }
    catch(const Exception& kex)
    {
        kex.print_details();
        return EXIT_FAILURE;
    }
    catch(const std::exception& ex)
    {
        cout << ex.what() << endl;
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
    glOrtho(-img_width / 2.0, img_width / 2.0,
            -img_height / 2.0, img_height / 2.0,
            -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void display_scene()
{
    // clear stuff
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glRasterPos2d(-img_width/2.0, -img_height/2.0);

    // colors and stipples
    Vector data_color = Vector().set(1.0, 0.0, 0.0);
    Vector data_color_2 = Vector().set(0.0, 1.0, 0.0);
    Vector tg_color = Vector().set(0.0, 0.0, 1.0);
    Vector norm_color = Vector().set(1.0, 1.0, 0.0);
    GLushort solid_stipple = 0xFFFF;
    GLushort corr_stipple = 0x3333;

    // create temporary association
    Target temp_tg = *tg_p;
    temp_tg.erase(temp_tg.lower_bound(cur_frame), temp_tg.end());
    Ascn w(*data_p);
    w.insert(temp_tg);
    const Target& tg = *w.begin();

    // display frame
    Image img((*frame_imgs_p)[cur_frame - 1]);
    set_framebuffer(img);

    // display trail
    glLineWidth(2.0);
    glLineStipple(1, solid_stipple);
    glColor(tg_color);
    glBegin(GL_LINE_STRIP);
    Target::const_iterator pr_p = tg.begin();
    while(pr_p != tg.end())
    {
        size_t t = pr_p->first;
        vector<const Detection_box*> dboxes;
        while(pr_p != tg.end() && pr_p->first == t)
        {
            dboxes.push_back(pr_p->second);
            pr_p++;
        }

        Detection_box dbox = average_box_centers(dboxes);
        glVertex(dbox.bbox.get_bottom_center());
    }
    glEnd();
    
    // compute scores and details about them
    Score_map scores;
    double noise_score;
    Vector vel;
    Vector mu;
    double sg_u;
    double sg_v;
    bool ignore_score = false;
    if(tg.real_size() < 2)
    {
        ignore_score = true;
    }
    else
    {
        int prev_t = tg.get_end_time();
        int d = cur_frame - prev_t;
        if(d > b_bar)
        {
            ignore_score = true;
        }
        else
        {
            // compute scores
            pair<Vector, Vector> fut = proposer_p->track_future_ls(tg);
            const Vector& x = fut.first;
            vel = fut.second;
            set<const Detection_box*> dpt = w.get_dead_points_at_time(cur_frame);
            scores = proposer_p->get_growing_probabilities(tg, cur_frame, dpt, x, vel, d);
            //noise_score = proposer_p->get_noise_score(d, vel);

            // compute sigmas of normal
            mu = x;
            if(!vel.empty()) mu += d*vel;
            double vel_sigma = vel.empty() ? d*d*v_bar*v_bar : 0.0;
            sg_u = sqrt(noise_variance + vel_sigma);
            sg_v = sqrt(noise_variance + vel_sigma);

            // display fitted line
            if(!vel.empty())
            {
                glLineWidth(1.0);
                glLineStipple(1, solid_stipple);
                glColor(norm_color);
                glBegin(GL_LINES);
                glVertex(x + 100*vel);
                glVertex(x - 100*vel);
                glEnd();
            }
        }
    }

    // display normals
    if(!ignore_score)
    {
        glLineWidth(1.0);
        glLineStipple(1, solid_stipple);
        glColor(norm_color);
        draw_ellipse(mu, sg_u, sg_v, M_PI/12);
        draw_ellipse(mu, 2*sg_u, 2*sg_v, M_PI/12);
        draw_ellipse(mu, 3*sg_u, 3*sg_v, M_PI/12);
    }

    // display boxes
    double cur_p_det;
    double cur_pn;
    double cur_score;
    glLineWidth(2.0);
    BOOST_FOREACH(const Detection_box& box, (*data_p)[cur_frame - 1])
    {
        double p_det;
        double score;
        if(ignore_score)
        {
            p_det = 1.0 - box.prob_noise;
            score = -1.0;
            noise_score = -1.0;
        }
        else
        {
            Score_map::right_const_iterator pair_p = scores.right.find(&box);
            if(pair_p != scores.right.end())
            {
                score = pair_p->second;
                //p_det = pair_p->second / (pair_p->second + noise_score);
                p_det = pair_p->second;
            }
            else
            {
                cout << "WARNING: this should not happen!" << endl;
            }
        }

        if(&box == &(*cur_box_p))
        {
            glLineStipple(1, corr_stipple);
            cur_p_det = p_det;
            cur_pn = 1.0 - box.prob_noise;
            cur_score = score;
        }
        else
        {
            glLineStipple(1, solid_stipple);
        }

        pair<Target::const_iterator, Target::const_iterator>
            it_pair = tg_p->equal_range(cur_frame);
        if(find(it_pair.first,
                it_pair.second,
                Target::value_type(cur_frame, &box)) != it_pair.second)
        {
            glColor(tg_color);
        }
        else
        {
            glColor(p_det * (ignore_score ? data_color : data_color_2));
        }

        draw_box(box.bbox);
    }

    // display info
    stringstream info_str;
    double info_x = -img_width/2 + 5;
    double info_y = -img_height/2 + 5;
    double text_height = 12;
    glColor3d(1.0, 0.0, 1.0);

    info_str << "velocity: ";
    if(vel.empty()) info_str << "N/A";
    else info_str << vel;
    bitmap_string(info_str.str(), info_x, info_y);
    info_y += text_height;
    info_str.str("");
    info_str.clear();

    info_str << "noise score: " << noise_score;
    bitmap_string(info_str.str(), info_x, info_y);
    info_y += text_height;
    info_str.str("");
    info_str.clear();

    info_str << "box score: " << cur_score;
    bitmap_string(info_str.str(), info_x, info_y);
    info_y += text_height;
    info_str.str("");
    info_str.clear();

    info_str << "p deva: " << cur_pn;
    bitmap_string(info_str.str(), info_x, info_y);
    info_y += text_height;
    info_str.str("");
    info_str.clear();

    info_str << "box prob: " << cur_p_det;
    bitmap_string(info_str.str(), info_x, info_y);
#ifdef KJB_HAVE_GLUT
    glutSwapBuffers();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_key(unsigned char key, int /*x*/, int /*y*/)
{
    switch(key)
    {
        case 'n':
        {
            cur_frame++;
            if(cur_frame > frame_imgs_p->size())
            {
                cur_frame = 1;
            }

            cur_box_p = (*data_p)[cur_frame - 1].begin();
            break;
        }

        case 'p':
        {
            if(cur_frame == 1)
            {
                cur_frame = frame_imgs_p->size() + 1;
            }

            cur_frame--;
            cur_box_p = (*data_p)[cur_frame - 1].begin();
            break;
        }

        case 'l':
        {
            cur_box_p++;
            if(cur_box_p == (*data_p)[cur_frame - 1].end())
            {
                cur_box_p = (*data_p)[cur_frame - 1].begin();
            }
            break;
        }

        case 'h':
        {
            if(cur_box_p == (*data_p)[cur_frame - 1].begin())
            {
                cur_box_p = (*data_p)[cur_frame - 1].end();
            }
            cur_box_p--;
            break;
        }

        case 'a':
        {
            pair<Target::const_iterator, Target::const_iterator>
                dp_pr = tg_p->equal_range(cur_frame);

            if(find(dp_pr.first, dp_pr.second,
                Target::value_type(cur_frame, &(*cur_box_p))) == dp_pr.second)
            {
                tg_p->insert(make_pair(cur_frame, &(*cur_box_p)));
            }

            break;
        }

        case 'x':
        {
            pair<Target::iterator, Target::iterator>
                dp_pr = tg_p->equal_range(cur_frame);

            Target::iterator pair_p = find(dp_pr.first, dp_pr.second,
                        Target::value_type(cur_frame, &(*cur_box_p)));
            if(pair_p != dp_pr.second)
            {
                tg_p->erase(pair_p);
            }

            break;
        }

        case 'g':
        {
            Ascn w(*data_p);
            w.insert(*tg_p);
            proposer_p->grow_track_forward(*tg_p, w);
            cur_frame = tg_p->get_end_time();
            break;
        }

        case 'c':
        {
            Image img = get_framebuffer_as_image();
            boost::format img_out_fmt(img_out_dp + "/%05d.jpg");
            img.write((img_out_fmt % N).str());
            N++;
            break;
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

#ifdef KJB_HAVE_GLUT
    wnd_p->redisplay();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void draw_box(const Bbox& box)
{
    glBegin(GL_LINE_LOOP);
    glVertex2d(box.get_left(), box.get_top());
    glVertex2d(box.get_right(), box.get_top());
    glVertex2d(box.get_right(), box.get_bottom());
    glVertex2d(box.get_left(), box.get_bottom());
    glEnd();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void draw_ellipse(const Vector& c, double rx, double ry, double inc)
{
    glBegin(GL_LINE_LOOP);
    for(double ang = 0.0; ang < 2*M_PI; ang += inc)
    {
        glVertex(c + Vector().set(rx*cos(ang), ry*sin(ang)));
    }
    glEnd();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Vector get_dbox_center(const Detection_box& dbox)
{
    return dbox.bbox.get_bottom_center();
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

Detection_box average_box_centers(const vector<const Detection_box*>& boxes)
{
    Detection_box res = *boxes[0];

    Vector avg(0.0, 0.0);
    size_t num_boxes = boxes.size();
    for(size_t i = 0; i < num_boxes; i++)
    {
        avg += boxes[i]->bbox.get_bottom_center();
    }

    res.bbox.set_center(avg / num_boxes);
    res.bbox.set_centre_y(res.bbox.get_centre_y() + res.bbox.get_height()/2.0);
    return res;
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
            ("movie-path", bpo::value<string>(&movie_dp)->required(),
                      "path to the data directory")
            ("img-out-path", bpo::value<string>(&img_out_dp),
                      "screenshots output directory")
            ("v-bar", bpo::value<double>(&v_bar)->default_value(5.0),
                      "maximum velocity of tracks")
            ("d-bar", bpo::value<int>(&d_bar)->default_value(30),
                      "maximum skipped frames of a track")
            ("b-bar", bpo::value<int>(&b_bar)->default_value(30),
                      "maximum skipped frames of a track (in a birth)")
            ("noise-variance",
                bpo::value<double>(&noise_variance)->default_value(50.0),
                "noise variance for detection boxes")
            ("gamma", bpo::value<double>(&gamm)->default_value(0.1),
                      "probability of halting growing a proposed track");

        bpo::store(bpo::command_line_parser(argc, argv)
                   .options(cmdline_options).run(), vm);

        if(vm.count("help"))
        {
            cout << "Usage: mcmcda_growing OPTIONS\n"
                 << "Visualize how MCMCDA growth occurs."
                 << cmdline_options << "\n"
                 << "For questions or complaints see Ernesto." << endl;

            exit(EXIT_SUCCESS);
        }

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
}

