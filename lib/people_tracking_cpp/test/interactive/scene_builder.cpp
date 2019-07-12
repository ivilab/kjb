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

/* $Id: scene_builder.cpp 18808 2015-04-08 17:04:54Z jguan1 $ */

#include <people_tracking_cpp/pt_association.h>
#include <mcmcda_cpp/mcmcda_proposer.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_util.h>
#include <people_tracking_cpp/pt_visibility.h>
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_complete_trajectory.h>
#include <gr_cpp/gr_opengl.h>
#ifdef KJB_HAVE_GLUT
#include <gr_cpp/gr_glut.h>
#endif
#include <gr_cpp/gr_opengl_headers.h>
#include <l_cpp/l_filesystem.h>
#include <detector_cpp/d_bbox.h>
#include <wrap_opencv_cpp/cv_histogram.h>
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

#define NUM_STR 6
using namespace kjb;
using namespace kjb::pt;
using namespace kjb::opengl;
using namespace kjb::mcmcda;
using namespace std;
using namespace boost;

// types
typedef bimap<multiset_of<double, greater<double> >,
              set_of<const Detection_box*> > Probability_map;

enum Display_mode : size_t
{
    MODE_MCMCDA, MODE_GROWING, MODE_MERGE, MODE_NUM_MODES
};

struct Render_properties
{
    Vector color;
    GLushort stipple;
    double line_width;

    Render_properties() : color(0.0, 0.0, 0.0), stipple(0xFFFF), line_width(2.0)
    {}
};

typedef map<const Detection_box*, Render_properties> Bp_map;
typedef map<const Target*, Render_properties> Tp_map;

// function declarations
void reshape_window(int w, int h);
void display_scene();
void process_key(unsigned char key, int x, int y);

void display_mcmcda();
vector<string> display_growing(Bp_map& box_props, Tp_map& trail_props);
vector<string> display_merge(Bp_map& box_props, Tp_map& trail_props);

void process_key_mcmcda(unsigned char key);
void process_key_growing(unsigned char key);
void process_key_merge(unsigned char key);

void initialize_states();
void draw_box(const Bbox&);
void draw_ellipse(const Vector& c, double rx, double ry, double inc);
Vector get_dbox_center(const Detection_box& dbox);
Detection_box average_box_centers(const vector<const Detection_box*>& boxes);
void process_options(int argc, char** argv);

// options
string movie_dp;
string detector_str;
double img_width;
double img_height;
double v_bar;
int d_bar;
int b_bar;
double noise_variance;
double gamm;

string input_dp;

// Screenshots
string img_out_dp;
string scene_out_dp;

// global pointers (to avoid memory issues of KJB+OPENGL)
#ifdef KJB_HAVE_GLUT
const Glut_window* wnd_p;
#endif
const Box_data* data_p;
const vector<string>* frame_imgs_p;
const vector<Hist_map>* histograms_p; 
const vector<Integral_flow>* flows_x_p;
const vector<Integral_flow>* flows_y_p;
const Proposer<Target>* proposer_p;

const Perspective_camera* cam_p;
Scene* scene_p;
Ascn* w_p;
const Target* empty_tg_p;
string mode_names[MODE_NUM_MODES] = {"MCMCDA", "Grow", "Merge"};

// state variables
size_t cur_frame;
Ascn::iterator cur_tg_p;
Ascn::iterator cur_tg_q;
Box_data::Box_set::const_iterator cur_box_p;

// colors and stipples
Vector deva_color = Vector().set(1.0, 1.0, 0.0);
Vector target_color = Vector().set(0.0, 1.0, 0.0);
Vector grow_color = Vector().set(0.0, 1.0, 1.0);
Vector info_color = Vector().set(0.0, 0.0, 0.0);
GLushort solid_stipple = 0xFFFF;
GLushort corr_stipple = 0x3333;


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
            movie_dp + "/detection_boxes/" + detector_str + "/%05d.txt"));

        // read camera 
        Perspective_camera camera;
        string cam_fp = movie_dp + "/camera/gt_camera1_fix_height.txt";
        camera.read(cam_fp.c_str());

        // create empty association and target
        //Ascn w(data);
        Target empty_tg(1.7, 0.4, 0.3, data.size());

        // initial scene
        Scene scene(Ascn(data), camera, 1.0, 1.0, 1.0);

        // Compute the histogram of each detection box
        const int red_bins = 12;
        const int green_bins = 12;
        vector<Hist_map> histograms(frame_imgs.size());

        for(size_t i = 0; i < frame_imgs.size(); i++)
        {
            Image img(frame_imgs[i]);
            std::cout << "Load image at frame  " << i + 1 << std::endl; 
            BOOST_FOREACH(const Detection_box& dbox, data[i])
            {
                Bbox box(dbox.bbox);
                unstandardize(box, img_width, img_height);
                histograms[i].insert(
                        make_pair<const Detection_box*, Matrix>(&dbox, 
                        opencv::calculate_rg_histogram(img, box, 
                                               red_bins, green_bins)));
            }
        }

        histograms_p = &histograms;

        // flows 
        string feat_dp(movie_dp + "/features/optical_flow/brox/");
        vector<string> x_of_fps = 
            file_names_from_format(feat_dp + "/x_int_s4/%05d.txt");
        vector<string> y_of_fps = 
            file_names_from_format(feat_dp + "/y_int_s4/%05d.txt");
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        for(size_t i = 0; i < x_of_fps.size(); i++)
        {
            std::cout << " read in flow " << x_of_fps[i] << std::endl;
            flows_x.push_back(Integral_flow(x_of_fps[i]));
            flows_y.push_back(Integral_flow(x_of_fps[i]));
        }
        flows_x_p = &flows_x;
        flows_y_p = &flows_y;

        Feature_score feature_score(histograms, flows_x, flows_y);

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
            feature_score,
            noise_variance,
            empty_tg);

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
        frame_imgs_p = &frame_imgs;
        proposer_p = &proposer;
        empty_tg_p = &empty_tg;
        scene_p = &scene;
        w_p = &(scene_p->association);
        cam_p = &camera;

        // initialize state
        cur_frame = 1;
        cur_box_p = data[cur_frame - 1].begin();
        cur_tg_p = w_p->end();
        cur_tg_q = w_p->end();

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

    // Set all boxes to use default properties
    Bp_map box_props;
    BOOST_FOREACH(const Detection_box& box, (*data_p)[cur_frame - 1])
    {
        Render_properties rps;
        rps.color = deva_color * (1.0 - box.prob_noise);
        box_props[&box] = rps;
    }

    // Set all boxes to use default properties
    Tp_map trail_props;
    BOOST_FOREACH(const Target& tg, *w_p)
    {
        Render_properties rps;
        rps.color = target_color;
        trail_props[&tg] = rps;
    }

    // display frame
    Image img((*frame_imgs_p)[cur_frame - 1]);
    set_framebuffer(img);

    // display current association
    vector<string> info_strs = display_growing(box_props, trail_props);

    // display trails
    BOOST_FOREACH(const Tp_map::value_type& pr, trail_props)
    {
        glColor(pr.second.color);
        glLineWidth(pr.second.line_width);
        glLineStipple(1, pr.second.stipple);

        const Target& tg = *pr.first;
        Target::const_iterator pr_p = tg.begin();

        glBegin(GL_LINE_STRIP);
        while(pr_p != tg.end() && pr_p->first <= cur_frame)
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
    }

    // display appropriate boxes
    BOOST_FOREACH(const Bp_map::value_type& pr, box_props)
    {
        glColor(pr.second.color);
        glLineWidth(pr.second.line_width);
        glLineStipple(1, pr.second.stipple);

        draw_box(pr.first->bbox);
    }

    // display info
    double info_x = -img_width/2 + 5;
    double info_y = -img_height/2 + 5;
    double text_height = 12;
    glColor(info_color);

    for(size_t i = 0; i < info_strs.size(); i++)
    {
        bitmap_string(info_strs[i], info_x, info_y);
        info_y += text_height;
    }

    //bitmap_string("Mode: " + mode_names[cur_mode], info_x, info_y);
#ifdef KJB_HAVE_GLUT
    glutSwapBuffers();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

/*pair<set<const Detection_box*>, vector<string> > display_mcmcda()
{
}*/

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

vector<string> display_growing(Bp_map& box_props, Tp_map& trail_props)
{
    // shorten current track and create new association
    Target cur_sh_tg = *empty_tg_p;
    if(cur_tg_p != w_p->end())
    {
        cur_sh_tg.insert(cur_tg_p->begin(), cur_tg_p->lower_bound(cur_frame));
    }

    Ascn w = *w_p;
    Ascn::iterator temp_tg_p = w.begin();
    if(!w_p->empty())
    {
        advance(temp_tg_p, distance(w_p->begin(), cur_tg_p));
        w.erase(temp_tg_p);
        w.insert(cur_sh_tg);

        trail_props[&(*cur_tg_p)].line_width = 4.0;
    }

    // create ignored list and info strings

    //double p_deva = 1.0 - cur_box_p->prob_noise;

    // flow 
    //const vector<Integral_flow>& flows_x = *flows_x_p;
    //const vector<Integral_flow>& flows_y = *flows_y_p;

    // draw free boxes
    int d = cur_frame - (cur_sh_tg.empty() ?  0 : cur_sh_tg.get_end_time());
    if((*data_p)[cur_frame - 1].size() > 0)
    {
        if(cur_tg_p == w_p->end())
        {
            box_props[&(*cur_box_p)].line_width = 4.0;
        }
        else if(cur_sh_tg.empty() || d > d_bar)
        {
            // if first frame draw using deva color or target color
            pair<Target::const_iterator, Target::const_iterator>
                it_pair = cur_tg_p->equal_range(cur_frame);
            if(count(it_pair.first, it_pair.second,
                     Target::value_type(cur_frame, &(*cur_box_p))) == 0)
            {
                box_props[&(*cur_box_p)].color = (1.0 - cur_box_p->prob_noise)
                                                                    * deva_color;
            }
            else
            {
                box_props[&(*cur_box_p)].color = target_color;
            }

            box_props[&(*cur_box_p)].line_width = 4.0;
        }
        else
        {
            set<const Detection_box*> dpt = w.get_dead_points_at_time(cur_frame);
            BOOST_FOREACH(const Detection_box& box, (*data_p)[cur_frame - 1])
            {
                // in target?
                pair<Target::const_iterator, Target::const_iterator>
                    it_pair = cur_tg_p->equal_range(cur_frame);
                if(count(it_pair.first, it_pair.second,
                         Target::value_type(cur_frame, &box)) == 0)
                {
                    box_props[&box].color = target_color * 0.5;

                    if(dpt.count(&box) == 0)
                    {
                        box_props[&box].stipple = corr_stipple;
                    }
                }
                else
                {
                    box_props[&box].color = target_color;
                }

                // cur box?
                if(&box == &(*cur_box_p))
                {
                    box_props[&box].line_width = 4.0;

                }
            }
        }
    }

    vector<string> info_strings(1);
    info_strings[0] = "growing";

    return info_strings;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

vector<string> display_merge(Bp_map& box_props, Tp_map& trail_props)
{
    std::cout << " merge: " << w_p->size() << std::endl;
    // create ignored list and info strings
    stringstream info_sstreams[1];
    info_sstreams[0] << "probability: ";

    if(w_p->size() < 2)
    {
        return vector<string>(1, info_sstreams[0].str());
    }

    // compute merge candidates
    vector<double> ps;
    ps.reserve((w_p->size()*(w_p->size()-1))/2);
    list<Target> M;
    Ascn w_e(w_p->get_data());
    size_t c = 0;
    size_t cur_pair;
    for(Ascn::const_iterator track_p1 = w_p->begin();
                                            track_p1 != w_p->end();
                                            track_p1++)
    {
        Ascn::const_iterator track_p2 = track_p1;
        for(track_p2++; track_p2 != w_p->end(); track_p2++, c++)
        {
            if(&(*track_p1) == &(*cur_tg_p) && &(*track_p2) == &(*cur_tg_q))
            {
                cur_pair = c;
                trail_props[&(*track_p1)].line_width = 5.0;
                trail_props[&(*track_p2)].line_width = 5.0;
                trail_props[&(*track_p1)].color = Vector().set(1.0, 0.0, 0.0);
                trail_props[&(*track_p2)].color = Vector().set(1.0, 0.0, 0,0);
            }

            Target new_track = *track_p1;
            new_track.insert(track_p2->begin(), track_p2->end());

            M.push_back(new_track);
            ps.push_back(proposer_p->p_grow_track_forward(
                                                new_track, w_e,
                                                new_track.get_start_time()));
        }
    }

    // normalize
    list<Target>::const_iterator target_p = M.begin();
    for(size_t i = 0; i < ps.size(); i++, target_p++)
    {
        ps[i] /= target_p->real_size();
        ps[i] *= -1.0;
        ps[i] = sqrt(ps[i]);
        ps[i] *= -1.0;
    }

    double lsum = log_sum(ps.begin(), ps.end());
    transform(ps.begin(), ps.end(), ps.begin(),
              std::bind2nd(minus<double>(), lsum));
    transform(ps.begin(), ps.end(), ps.begin(),
              std::ptr_fun<double, double>(std::exp));

    info_sstreams[0] << ps[cur_pair];

    return vector<string>(1, info_sstreams[0].str());
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

        case 'c':
        {
            Image img = get_framebuffer_as_image();
            boost::format img_out_fmt(img_out_dp + "/%05d.jpg");
            if(img_out_dp == "")
            {

                std::cout << " img_out_dp is empty\n";
            }
            else
            {
                // screen shot counter 
                kjb_c::kjb_mkdir(img_out_dp.c_str());
                img.write((img_out_fmt % cur_frame).str());
            }
            break;
        }

        case 'i':
        {
            if(input_dp != "")
            {
                //w_p->read(input_dp + "/association.txt", *empty_tg_p);
                read_scene(*scene_p, input_dp + "/tracks");
                BOOST_FOREACH(const Target& target, scene_p->association)
                {
                    target.estimate_trajectory(scene_p->camera);
                    //target.smooth_trajectory();
                    //initialize_directions(target.trajectory());
                    //target.changed(false);
                    target.estimate_directions();
                }

                std::for_each(scene_p->association.begin(), 
                              scene_p->association.end(),
                              boost::bind(&Target::update_boxes, _1, 
                              scene_p->camera));

                update_visibilities(*scene_p);

                initialize_states();
            }
            else
            {
                std::cout << "Input directory is empty! \n";
            }
            break;
        }

        case 'o':
        {
            if(scene_out_dp != "")
            {
                kjb_c::kjb_mkdir(scene_out_dp.c_str());
                write_scene(*scene_p, scene_out_dp + "/tracks/");
                cout << "Wrote scene in " << scene_out_dp << endl;
            }
            else
            {
                std::cout << "Input directory is empty! \n";
            }
            break;
        }

        case 'q':
        {
            cout << "Bye..." << endl;
            exit(EXIT_SUCCESS);
        }

        default:
        {
            process_key_growing(key);
            return;
        }
    }
#ifdef KJB_HAVE_GLUT
    wnd_p->redisplay();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_key_mcmcda(unsigned char key)
{
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_key_growing(unsigned char key)
{
    switch(key)
    {
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

        case 'j':
        {
            if(!w_p->empty())
            {
                cur_tg_p++;
                if(cur_tg_p == w_p->end())
                {
                    cur_tg_p = w_p->begin();
                }
            }

            break;
        }

        case 'k':
        {
            if(!w_p->empty())
            {
                if(cur_tg_p == w_p->begin())
                {
                    cur_tg_p = w_p->end();
                }
                cur_tg_p--;
            }

            break;
        }

        case 't':
        {
            set<const Detection_box*>
                dpt = w_p->get_dead_points_at_time(cur_frame);
            if(dpt.count(&(*cur_box_p)) != 0)
            {
                Target tg = *empty_tg_p;
                tg.insert(make_pair(cur_frame, &(*cur_box_p)));
                cur_tg_p = (w_p->insert(tg)).first;
            }

            break;
        }

        case 'd':
        {
            if(!w_p->empty())
            {
                w_p->erase(cur_tg_p);
                cur_tg_p = w_p->begin();
            }

            break;
        }

        case 'a':
        {
            if(!w_p->empty())
            {
                set<const Detection_box*>
                    dpt = w_p->get_dead_points_at_time(cur_frame);
                if(dpt.count(&(*cur_box_p)) != 0)
                {
                    Target tg = *cur_tg_p;
                    tg.insert(make_pair(cur_frame, &(*cur_box_p)));
                    w_p->erase(cur_tg_p);
                    cur_tg_p = (w_p->insert(tg)).first;
                }
            }

            break;
        }

        case 'x':
        {
            if(!w_p->empty())
            {
                Target tg = *cur_tg_p;

                pair<Target::iterator, Target::iterator>
                    dp_pr = tg.equal_range(cur_frame);

                Target::iterator pair_p = find(dp_pr.first, dp_pr.second,
                            Target::value_type(cur_frame, &(*cur_box_p)));
                if(pair_p != dp_pr.second)
                {
                    tg.erase(pair_p);
                    w_p->erase(cur_tg_p);
                    if(!tg.empty())
                    {
                        cur_tg_p = w_p->insert(tg).first;
                    }
                    else
                    {
                        cur_tg_p = w_p->begin();
                    }
                }
            }

            break;
        }

        case 'g':
        {
            if(!w_p->empty())
            {
                Target tg = *cur_tg_p;
                w_p->erase(cur_tg_p);
                proposer_p->grow_track_forward(tg, *w_p);
                cur_tg_p = w_p->insert(tg).first;
                cur_frame = cur_tg_p->get_end_time();
                cur_box_p = (*data_p)[cur_frame - 1].begin();

            }

            break;
        }

        default:
        {
            return;
        }
    }

    BOOST_FOREACH(const Target& target, scene_p->association)
    {
        //target.update_trajectory(*cam_p);
        target.estimate_trajectory(*cam_p);
        //target.smooth_trajectory();
        //initialize_directions(target.trajectory());
        //target.changed(false);
        target.estimate_directions();
    }

    std::for_each(scene_p->association.begin(), 
                  scene_p->association.end(),
                  boost::bind(&Target::update_boxes, _1, 
                  *cam_p));

    update_visibilities(*scene_p);


#ifdef KJB_HAVE_GLUT
    wnd_p->redisplay();
#endif
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void process_key_merge(unsigned char key)
{
    switch(key)
    {
        case 'j':
        {
            if(w_p->size() >= 2)
            {
                cur_tg_q++;
                if(cur_tg_q == w_p->end())
                {
                    cur_tg_p++;

                    Ascn::const_iterator last_tg_p = w_p->end();
                    last_tg_p--;
                    if(cur_tg_p == last_tg_p)
                    {
                        cur_tg_p = w_p->begin();
                    }

                    cur_tg_q = cur_tg_p;
                    cur_tg_q++;
                }
            }

            break;
        }

        case 'm':
        {
            Target new_target = *cur_tg_p;
            new_target.insert(cur_tg_q->begin(), cur_tg_q->end());
            w_p->erase(cur_tg_p);
            w_p->erase(cur_tg_q);
            w_p->insert(new_target);

            if(w_p->size() < 2)
            {
                cur_tg_p = w_p->end();
                cur_tg_q = w_p->end();
            }
            else
            {
                cur_tg_p = w_p->begin();
                cur_tg_q = cur_tg_p;
                cur_tg_q++;
            }

            break;
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

void initialize_states()
{
    cur_box_p = (*data_p)[cur_frame - 1].begin();

    if(!w_p->empty())
    {
        cur_tg_p = w_p->begin();
    }

    if(w_p->size() >= 2)
    {
        cur_tg_q = w_p->begin();
        cur_tg_q++;
    }
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
            ("detector", bpo::value<string>(&detector_str)->required(),
                      "path to the data directory")
            ("scene-out-path", bpo::value<string>(&scene_out_dp),
                      "scene output directory")
            ("img-out-path", bpo::value<string>(&img_out_dp),
                      "screenshots output directory")
            ("input-path", bpo::value<string>(&input_dp),
                      "input directory of existing scene")
            ("v-bar", bpo::value<double>(&v_bar)->default_value(5.0),
                      "maximum velocity of tracks")
            ("d-bar", bpo::value<int>(&d_bar)->default_value(40),
                      "maximum skipped frames of a track")
            ("b-bar", bpo::value<int>(&b_bar)->default_value(15),
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

