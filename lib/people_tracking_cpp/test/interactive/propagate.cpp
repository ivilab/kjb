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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: propagate.cpp 18562 2015-02-16 22:50:01Z jguan1 $ */
#include <people_tracking_cpp/pt_data.h>
#include <people_tracking_cpp/pt_util.h>
#include <people_tracking_cpp/pt_file_util.h>
#include <people_tracking_cpp/pt_target.h>
#include <people_tracking_cpp/pt_box_trajectory.h>
#include <people_tracking_cpp/pt_scene.h>
#include <flow_cpp/flow_integral_flow.h>
#include <st_cpp/st_backproject.h>
#include <m_cpp/m_vector.h>
#include <i_cpp/i_image.h>
#include <l_cpp/l_serialization.h>

#include <string> 
#include <ostream>
#include <fstream>
#include <algorithm>

#include <boost/array.hpp>
#include <boost/assign.hpp>
using namespace kjb;

using namespace std;
using namespace kjb::pt;
bool smooth = false;
const boost::array<Vector3, 7> COLOR_ORDER_ = boost::assign::list_of
    (Vector3(0.0, 0.0, 1.0))
    (Vector3(0.0, 0.5, 0.0))
    (Vector3(1.0, 0.0, 0.0))
    (Vector3(0.0, 0.75, 0.75))
    (Vector3(0.75, 0.0, 0.75))
    (Vector3(0.75, 0.75, 0.0))
    (Vector3(0.25, 0.25, 0.25));

int main(int argc, char** argv)
{
    kjb_c::kjb_init();
    
    if(argc != 5 && argc != 6 && argc != 7) 
    {
        cout << " Usuage: " << argv[0]
             << " movie-dir box-dir k scene-dp gt-dp out-dp\n";
        return EXIT_SUCCESS;
    }

    string movie_dir_str = argv[1];
    string data_dp = argv[2];
    size_t frame_rate = boost::lexical_cast<size_t>(argv[3]);
    string scene_dp = argv[4];
    string gt_dp = "";
    string output_dir = "";
    if(argc >= 6) 
    {
        gt_dp = argv[5];
    }
    if(argc == 7)
    {
        output_dir = argv[6];
    }
    else
    {
        cout << " Usuage: " << argv[0]
             << " movie-dir box-dir k scene-dp gt-dp out-dp\n";
        return EXIT_SUCCESS;
    }

    kjb_c::kjb_mkdir(output_dir.c_str());

    Input_directory input_dir(movie_dir_str);

    string feat_dp = movie_dir_str + "/features/";
    vector<string> x_of_fps = 
        file_names_from_format(feat_dp + "/optical_flow/brox/x_int_s4/%05d.txt");
    vector<string> y_of_fps = 
        file_names_from_format(feat_dp + "/optical_flow/brox/y_int_s4/%05d.txt");

    vector<string> back_x_of_fps = file_names_from_format(
            feat_dp + "/optical_flow/brox/back_x_int_s4/%05d.txt", 2);
    vector<string> back_y_of_fps = file_names_from_format(
            feat_dp + "/optical_flow/brox/back_y_int_s4/%05d.txt", 2);

    // Read in the flows 
    vector<Integral_flow> flows_x;
    vector<Integral_flow> flows_y;

    vector<Integral_flow> back_flows_x;
    vector<Integral_flow> back_flows_y;

    KJB(ASSERT(x_of_fps.size() == y_of_fps.size()));
    KJB(ASSERT(back_x_of_fps.size() == back_y_of_fps.size()));
    size_t num_frames = x_of_fps.size();
    //size_t num_frames = 200;

    for(size_t i = 0; i < num_frames; i++)
    {
        std::cout << "loading " << x_of_fps[i] << std::endl;
        flows_x.push_back(Integral_flow(x_of_fps[i]));
        flows_y.push_back(Integral_flow(y_of_fps[i]));
        back_flows_x.push_back(Integral_flow(back_x_of_fps[i]));
        back_flows_y.push_back(Integral_flow(back_y_of_fps[i]));
    }

    // Read in data
    vector<string> frames_fps = input_dir.get_frames_fps(); 
    Image tmp_img(frames_fps[0]);
    double img_width = tmp_img.get_num_cols();
    double img_height = tmp_img.get_num_rows();

    Box_data data(img_width, img_height, 0.99);
    std::cout << "Start to load detection boxes \n";
    data.read(file_names_from_format(data_dp + "/%05d.txt"));
    std::cout << "Loaded detection boxes \n";

    // Create association
    Scene scene(Ascn(data), Perspective_camera(), 0.0, 0.0, 0.0);

    if(gt_dp == "")
    {
        std::cout << "start to compute association from experiments\n";
        read_scene(scene, scene_dp);
        std::cout << "Computed association from experiments\n";
    }
    else
    {
        std::cout << "start to compute association from ground truth\n";
        // read camera
        string cam_fp = gt_dp + "/camera.txt";
        load(scene.camera, cam_fp);

        // create ground truth association
        Box_trajectory_map gt_box_trajs;
        gt_box_trajs.parse(gt_dp + "/2d_boxes", "person");

        Target empty_target(1.85, 0.4, 0.3, data.size());
        scene.association = make_gt_association(
                                            data,
                                            gt_box_trajs,
                                            img_width,
                                            img_height,
                                            boost::none);
        std::cout << "Computed association from ground truth\n";
    }

    if(scene.association.empty())
    {
        std::cout << "Scene association is empty\n";
    }
    IFT(!scene.association.empty(), Runtime_error,
        "Association is empty. Check association file or GT trajectories.");

    // Only keep the first one for dubugging purposes 
    //Ascn::iterator tg_p = scene.association.begin();
    //scene.association.erase(++tg_p, scene.association.end());

    // propogate the boxes at each frame by k frames forward for each target
    BOOST_FOREACH(const Target& target, scene.association)
    {
        size_t sf = target.get_start_time();
        size_t ef = target.get_end_time();
        std::cout << " start time: " << sf << " end time: " << ef << std::endl;
        target.estimate_trajectory(scene.camera, 
                                flows_x, flows_y, 
                                back_flows_x, back_flows_y, 
                                frame_rate);
    }
    
    std::string tracks_dp(output_dir + "/tracks/");

    write_scene(scene, tracks_dp);
}

