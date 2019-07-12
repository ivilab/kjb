#include <wrap_opencv_cpp/cv_optical_flow.h>
#include <wrap_opencv_cpp/cv_util.h>
#include <wrap_opencv_cpp/cv_features_to_track.h>

#include <detector_cpp/d_deva_detection.h>
#include <i_cpp/i_image.h>
#include <m_cpp/m_vector.h>

#include <utility>
#include <vector>
#include <ostream>
#include <fstream>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>


using namespace kjb;
using namespace kjb::opencv;
using namespace std;

void update_features(const std::vector<Vector> old_features, std::vector<Vector>& new_features)
{
    new_features.clear();
    new_features.resize(old_features.size());
    if(old_features.size() > 0)
    {
        for(size_t i = 0; i < old_features.size(); i++)
        {
            new_features[i] = old_features[i];
        }
    }
}

int main(int argc, char* argv[])
{
//    const char* img1_name = argv[1];
//    const char* img2_name = argv[2];
//    const char* deva_box_name = argv[3];
    if(argc == 1)
    {
        std::cout<<"Usage: test_optical_flow <image_dir> <deva_boxes_dir> <num_frames>\n";
        return EXIT_FAILURE;
    }
     kjb_c::kjb_l_set("page","off");
    //const char* movie_dir = argv[1];
    string movie_dir(argv[1]);
    size_t num_frames = boost::lexical_cast<size_t>(argv[3]);
    size_t frame_d = 5;

    string output_dir_str("output/%05d.jpg");
    boost::format output_dir_fm(output_dir_str);

    size_t frame = 1;

    string deva_boxes_dir_str(movie_dir + "/person/%05d.txt");
    boost::format deva_boxes_dir_fm(deva_boxes_dir_str);
    string deva_box_name = (deva_boxes_dir_fm % frame).str();

    ifstream ifs(deva_box_name.c_str());
    std::vector<Deva_detection> hboxes = parse_deva_detection(ifs);
    std::vector<Bounding_Box2D> bboxes(hboxes.size());
    for(size_t i = 0; i < hboxes.size(); i++)
    {
        bboxes[i] = hboxes[i].full_body();
    }

    std::cout<<"boxes size: "<<bboxes.size() <<std::endl;

    const Vector& center = bboxes[0].get_center();
    double b_w = bboxes[0].get_width();
    double b_h = bboxes[0].get_height();

    double roi_x = center[0] - b_w/2.0;
    double roi_y = center[1] - b_h/2.0;
    double roi_w = b_w;
    double roi_h = b_h;

    std::cout<<"roi_x: "<<roi_x<<" roi_y: "<<roi_y<<" roi_w: "<<roi_w<<" roi_h: "<<roi_h<<std::endl;
    
    // for feature detection 
    double quality_level = 0.1; 
    double min_distance = 10.0; 
    int block_size = 3.0;
    bool use_harris = false;
    double k = 0.04;
    double optical_flow_max_level = 5;
    double optical_flow_win_h = 50;
    double optical_flow_win_w = 50;
    double optical_flow_flags = 0;

    CV_features_to_track_detector feature_detector(quality_level, min_distance, block_size, use_harris, k);

    string img_name_str_first(movie_dir+"/frames");
    boost::format img_name_str_fm(img_name_str_first + "%05d.jpg");
    string img_first_name = (img_name_str_fm % frame).str();

    Image img_first(img_first_name.c_str());
//    std::vector<Vector> good_features = feature_detector.find_good_features(img_first, roi_x, roi_y, roi_w, roi_h);
    std::vector<Vector> good_features = feature_detector.find_good_features(img_first);

    std::cout<<"Number of good features: "<<good_features.size()<<std::endl;

    CV_term_criteria criteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 20, 0.3);


    CV_optical_flow_pyr_lk optical_flow(optical_flow_win_h, optical_flow_win_w, optical_flow_max_level, criteria, optical_flow_flags);

    for(size_t frame = frame_d; frame <= num_frames; frame+=frame_d)
    {

        if((frame + frame_d) >= num_frames)
            break;

        string img_name_str(movie_dir+"/frames");
        boost::format img_name_str_fm(img_name_str + "%05d.jpg");
        //string img1_name = (img_name_str_fm % frame).str();
        string img2_name = (img_name_str_fm % frame).str();

        //Image img1(img1_name.c_str());
        Image img2(img2_name.c_str());

//        std::vector<std::pair<size_t, Vector> > next_features = optical_flow.get_next_features(img1, img2, good_features);
        std::vector<Vector> next_features = optical_flow.get_next_features(img_first, img2, good_features);
        
        std::cout<<"Number of next featurs: "<<next_features.size()<<std::endl;

        if(next_features.size() > 0)
        {
            for(size_t i = 0 ; i < next_features.size(); i++)
            {
                //size_t prev_index = next_features[i].first;

//                const Vector& prev_loc = good_features[prev_index];
//                const Vector& next_loc = next_features[i].second;
                const Vector& prev_loc = good_features[i];
                const Vector& next_loc = next_features[i];
//                std::cout<<"next_loc_x: "<<next_loc(1)<<" next_loc_y: "<<next_loc(1)<<std::endl;
//               
//                if(vector_distance(prev_loc, next_loc) < 3)
//                    continue;
                kjb_c::Pixel pxl;
                pxl.r = 255.0;
                pxl.g = 0.0;
                pxl.b = 0.0;
                img2.draw_line_segment(prev_loc(1), prev_loc(0), next_loc(1), next_loc(0), 1.0, pxl); 
//                img1.draw_point(good_features[prev_index](1), good_features[prev_index](0), 1.0, 255.0, 0.0, 0.0);
//                img2.draw_point((next_features[i].second)(1), (next_features[i].second)(0), 1.0, 255.0, 0.0, 0.0);
            }

            string output_2 = (output_dir_fm % frame).str();

            img2.write(output_2.c_str());
        }
        if(frame % 20 == 0) // reinitialize the features after every 20 frames 
        {
            string deva_box_name = (deva_boxes_dir_fm % frame).str();

            ifstream ifs(deva_box_name.c_str());
            std::vector<Deva_detection> hboxes = parse_deva_detection(ifs);
            std::vector<Bounding_Box2D> bboxes(hboxes.size());
            for(size_t i = 0; i < hboxes.size(); i++)
            {
                bboxes[i] = hboxes[i].full_body();
            }

            std::cout<<"boxes size: "<<bboxes.size() <<std::endl;

            if(bboxes.size() > 0)
            {
                const Vector& center = bboxes[0].get_center();
                double b_w = bboxes[0].get_width();
                double b_h = bboxes[0].get_height();

                double x = center[0] - b_w/2.0;
                double y = center[1] - b_h/2.0;
                double roi_x = x > 0 ? x : 0;
                double roi_y = y > 0 ? y : 0;
                double roi_w = b_w;
                double roi_h = b_h;

                std::cout<<"roi_x: "<<roi_x<<" roi_y: "<<roi_y<<" roi_w: "<<roi_w<<" roi_h: "<<roi_h<<std::endl;

                good_features = feature_detector.find_good_features(img_first, roi_x, roi_y, roi_w, roi_h);
            }
        }
        else
        update_features(next_features, good_features);

        img_first = img2;
    }

    
    return EXIT_SUCCESS;
}
