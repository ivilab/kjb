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
   |  Author:  Ernesto Brau, Jinyan Guan
 * =========================================================================== */

/* $Id$ */

#include <people_tracking_cpp/pt_scene_viewer.h>
#include <people_tracking_cpp/pt_scene.h>
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_association.h>
#include <gr_cpp/gr_offscreen.h>
#include <l_cpp/l_exception.h>
#include <l_cpp/l_filesystem.h>

#include <vector>
#include <string>
#include <algorithm>

#include <boost/program_options.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace kjb;
using namespace kjb::pt;

string movie_data_dp;
string movie_exp_dp;
string detector_name;
string movie_gt_dp;
string out_dp;

bool render_ground;
bool render_cylinders;
bool render_bodies;
bool render_heads;
bool render_faces;
bool render_bottoms;
bool render_trails;
bool render_full_boxes;
bool render_body_boxes;
bool render_face_boxes;
bool render_face_features;
bool render_model_vectors;
bool render_face_vectors;
bool render_data_boxes;
bool render_facemarks;
bool render_flow_vectors;
bool render_flow_face_vectors;

size_t every_nth;
size_t shorten_size;
string shorten_location;
size_t start_frame;
size_t end_frame;


/** @brief  Process command-line options. */
void process_options(int argc, char** argv);

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

int main(int argc, char** argv)
{
//#ifdef TEST
//    kjb_c::kjb_init();
//    kjb_c::kjb_l_set("heap-checking", "off");
//    kjb_c::kjb_l_set("initialization-checking", "off");
//#endif
    try
    {
        process_options(argc, argv);

        string feat_dp = movie_data_dp + "/features/";
        string data_dp = movie_data_dp + "/detection_boxes/" + detector_name;
        string flow_dp = feat_dp + "/optical_flow/brox/";
        string fm_dp = feat_dp + "/deva_face/";
        // read flow
        vector<string> x_of_fps = 
            file_names_from_format(flow_dp + "/x_int_s4/%05d.txt");
        vector<string> y_of_fps = 
            file_names_from_format(flow_dp + "/y_int_s4/%05d.txt");
        // get frame names 
        vector<string> frame_fps = 
            file_names_from_format(movie_data_dp + "/frames/%05d.jpg");

        size_t num_frames = x_of_fps.size() + 1;
        IFT(num_frames > 1, Runtime_error, "Could not read features.");

        std::cout << "Start reading in flow ...\n";
        vector<Integral_flow> flows_x;
        vector<Integral_flow> flows_y;
        flows_x.reserve(num_frames - 1);
        flows_y.reserve(num_frames - 1);
        copy(x_of_fps.begin(), x_of_fps.end(), back_inserter(flows_x));
        copy(y_of_fps.begin(), y_of_fps.end(), back_inserter(flows_y));
        std::cout << "Read in flow \n";
        double img_width = flows_x.front().img_width();
        double img_height = flows_x.front().img_height();

        // create offscreen buffer
        std::cout << "start creating offscreen buffer ...\n";
        Offscreen_buffer* buffer = NULL;
        try 
        {
            buffer = create_and_initialize_offscreen_buffer(img_width, img_height);
        }
        catch(...)
        {
            std::cerr << "No rendering available!\n";
        }
        std::cout << "finished create buffer\n";

        // create and read box data
        std::cout << "start reading in box data ...\n";
        Box_data box_data(img_width, img_height, 0.99);
        box_data.read(file_names_from_format(data_dp + "/%05d.txt"));
        assert(num_frames == box_data.size());
        std::cout << "Read in box data\n";

        // create and read FM data
        std::cout << "Start reading in facemarks ...\n";
        Facemark_data fm_data = parse_deva_facemarks(
                        file_names_from_format(fm_dp + "/%05d.txt"),
                        img_width,
                        img_height);
        assert(num_frames == fm_data.size());
        std::cout << "Read in facemarks\n";

        // read scene
        std::cout << "Start reading in scene ...\n";
        Scene scene(Ascn(box_data), Perspective_camera(), 0.0, 0.0, 0.0);
        read_scene(scene, movie_exp_dp);
        std::cout << "Read in scene ...\n";

        // create viewer
        std::cout << "Creating the viewer ...\n";
        Scene_viewer viewer(scene, img_width, img_height, false);
        viewer.set_flows(flows_x, flows_y);
        viewer.set_facemarks(fm_data);
        viewer.weigh_data_box_color(true);

        render_body_boxes = true;
        render_face_boxes = true;
        render_face_features = true;
        render_trails = true;
        render_cylinders = true;
        render_full_boxes = false;
        render_bottoms = false;
        viewer.clear_alternative_camera();
        std::cout << "frame_fps: " << frame_fps.size() << std::endl;
        kjb_c::init_real_time();
        viewer.set_frame_images(frame_fps.begin(), frame_fps.end());
        long time = kjb_c::get_real_time();
        std::cout << "Loading images takes: " << time / 1000.0 << "s.\n";
        viewer.draw_images(true);
        viewer.draw_ground(render_ground);
        viewer.draw_cylinders(render_cylinders);
        viewer.draw_bodies(render_bodies);
        viewer.draw_heads(render_heads);
        viewer.draw_faces(render_faces);
        viewer.draw_bottoms(render_bottoms);
        viewer.draw_trails(render_trails);
        viewer.draw_full_boxes(render_full_boxes);
        viewer.draw_body_boxes(render_body_boxes);
        viewer.draw_face_boxes(render_face_boxes);
        viewer.draw_face_features(render_face_features);
        viewer.draw_model_vectors(render_model_vectors);
        viewer.draw_face_vectors(render_face_vectors);
        viewer.draw_data_boxes(render_data_boxes);
        viewer.draw_facemarks(render_facemarks);
        viewer.draw_flow_vectors(render_flow_vectors);
        viewer.draw_flow_face_vectors(render_flow_face_vectors);

        // save the images
        vector<string> out_fps(num_frames);
        kjb_c::kjb_mkdir(out_dp.c_str());
        boost::format out_fmt(out_dp + "/%05d.jpg");
        for(size_t i = 0; i < num_frames; i++)
        {
            out_fps[i] = (out_fmt % (i+1)).str();
        }
        viewer.write_images(out_fps);

        delete buffer;
    }
    catch(const Exception& ex)
    {
        ex.print_details();
        return EXIT_FAILURE;
    }
    catch(const exception& ex)
    {
        cerr<<ex.what()<<endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

namespace bpo = boost::program_options;

void process_options(int argc, char** argv)
{
    bpo::variables_map vm;

    try
    {
        // Hidden options
        bpo::options_description hidden_options("Hidden options");
        hidden_options.add_options()
            ("movie-path",
                bpo::value<string>(&movie_data_dp)->required(),
                "Data movie dir; has frames, detector outputs, etc");

        // General options
        bpo::options_description generic_options("Options");
        generic_options.add_options()
            ("help,h", "Produce help message")
            ("movie-exp-directory", bpo::value<string>(&movie_exp_dp),
                "Movie directory in experiments/runs")
            ("ground-truth-path",
                bpo::value<string>(&movie_gt_dp),
                "Ground truth dir; has annoated 3d tracks and camera")
            ("detector-name",
                bpo::value<string>(&detector_name)->required(), "Detector name")
            ("output-path",
                bpo::value<string>(&out_dp)->required(), "Output directory")
            ("render-ground", 
                bpo::bool_switch(&render_ground)->default_value(false), 
                "Render the ground plane")
            ("render-cylinders", 
                bpo::bool_switch(&render_cylinders)->default_value(false), 
                "Render cylinders")
            ("render-bodies", 
                bpo::bool_switch(&render_bodies)->default_value(false), 
                "Render bodies")
            ("render-heads", 
                bpo::bool_switch(&render_heads)->default_value(false), 
                "Render heads")
            ("render-faces", 
                bpo::bool_switch(&render_faces)->default_value(false), 
                "Render faces")
            ("render-data-boxes", 
                bpo::bool_switch(&render_data_boxes)->default_value(false), 
                "Render data boxes")
            ("render-data-face-marks", 
                bpo::bool_switch(&render_facemarks)->default_value(false), 
                "Render facemarks")
            ("render-face-features", 
                bpo::bool_switch(&render_face_features)->default_value(false), 
                "Show face features.")
            ("render-model-vectors", 
                bpo::bool_switch(&render_model_vectors)->default_value(false), 
                "Show average velocity vector in model boxes.")
            ("render-face-vectors", 
                bpo::bool_switch(&render_face_vectors)->default_value(false), 
                "Show average velocity vector in model boxes.")
            ("render-flow-vectors", 
                bpo::bool_switch(&render_flow_vectors)->default_value(false), 
                "Show average velocity vector in model boxes.")
            ("render-flow-face-vectors", 
                bpo::bool_switch(&render_flow_face_vectors)->default_value(false), 
                "Show average velocity vector in model face boxes.")
            ("every-nth", bpo::value<size_t>(&every_nth)->default_value(1),
                "see tracker help")
            ("shorten-size", bpo::value<size_t>(&shorten_size),
                "see tracker help")
            ("shorten-location", bpo::value<string>(&shorten_location)
                                             ->default_value("middle"),
                "see tracker help")
            ("start-frame", bpo::value<size_t>(&start_frame)
                                             ->default_value(0),
                "start frame ([1..N])")
            ("end-frame", bpo::value<size_t>(&end_frame)
                                             ->default_value(0),
                "end frame ([1..N])");

        // Combine options
        bpo::options_description visible_options;
        visible_options.add(generic_options);

        bpo::options_description cmdline_options;
        cmdline_options.add(visible_options).add(hidden_options);

        bpo::positional_options_description pstnl;
        pstnl.add("movie-path", 1);

        // process options
        bpo::store(bpo::command_line_parser(argc, argv)
                .options(cmdline_options).positional(pstnl).run(), vm);

        if(vm.count("help"))
        {
            cout << "Usage: tracks_to_video OPTIONS MOVIE-PATH\n"
                      << "Visualize everything related to a movie."
                      << visible_options << "\n"
                      << "For questions or complaints please contact "
                      << "ernesto@cs.arizona.edu.\n" << endl;

            exit(EXIT_SUCCESS);
        }

        // notify
        bpo::notify(vm);

        bool shorten = vm.count("shorten-size");
    }
    catch(const bpo::error& err)
    {
        KJB_THROW_2(Exception, err.what());
    }    
    catch(const exception& ex)
    {
        throw ex;
    }    
}

