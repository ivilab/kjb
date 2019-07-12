#include <wrap_opencv_cpp/cv_histogram.h>
#include <wrap_opencv_cpp/cv_util.h>
#include <detector_cpp/d_deva_detection.h>
#include <people_tracking_cpp/pt_detection_box.h>
#include <prob_cpp/prob_histogram.h>

#include <i_cpp/i_image.h>

#include <string>
#include <vector>
#include <fstream>

#ifdef KJB_HAVE_OPENCV
#include <opencv2/opencv.hpp>
#else
#error "Need OpenCV2 library"
#endif

using namespace kjb;
using namespace kjb::pt;
using namespace kjb::opencv;
using namespace std;

int main(int argc, char* argv[])
{
    kjb_c::kjb_init();
    if(argc != 4)
    {
        std::cout << " Usuage: " << argv[0] << " image-fp boxes-dp out-fp\n";
        return EXIT_SUCCESS;
    }

    std::vector<Axis_aligned_rectangle_2d> boxes;

    string img_name = argv[1];
    string boxes_dp = argv[2];
    string out_fp = argv[3];
    Image image(img_name);

    std::vector<Detection_box> dboxes = parse_detection_file(boxes_dp);
   
    for(size_t i = 0; i < dboxes.size(); i++)
    {
        dboxes[i].bbox.draw(image, i * 50.0, 0.0, 0.0);
        boxes.push_back(dboxes[i].bbox);
    }

    image.write(out_fp);

    const int r_bins = 64;
    const int g_bins = 64;

    std::vector<Matrix> histos = 
        calculate_rg_histograms(image, boxes, r_bins, g_bins);

    std::vector<Matrix> histos_hs = 
        calculate_hs_histograms(image, boxes, r_bins, g_bins);

    float sum = 0.0;
    for (size_t r = 0; r < histos[0].get_num_rows(); r++)
    {
        for(size_t c = 0; c < histos[0].get_num_cols(); c++)
        {
            sum += histos[0].at(r, c);
        }
    }
    std::ofstream ofs("hist_0.txt");
    ofs<< histos[0];
    std::cout << " sum: " << sum << std::endl;

    for(size_t i = 0; i < histos.size(); i++)
    {
        for(size_t j = i; j < histos.size(); j++)
        {
            double s_1 = compare_histograms(histos[i], histos[j], CORRELATION); 
            double s_2 = compare_histograms(histos[i], histos[j], CHI_SQUARE); 
            double s_3 = compare_histograms(histos[i], histos[j], INTERSECTION); 
            double s_4 = compare_histograms(histos[i], histos[j], BHATTACHARYYA); 
            double s_5 = chi_square(histos[i], histos[j]);

            std::cout << i << " vs: " << j << " : (corr) " << s_1
                                           << " : (chiq) " << s_2
                                           << " : (inte) " << s_3
                                           << " : (bhat) " << s_4
                                           << " : (chiq) " << s_5
                                           << std::endl;
        }
    }

    std::cout << "hsv histogram\n";
    for(size_t i = 0; i < histos_hs.size(); i++)
    {
        for(size_t j = i; j < histos_hs.size(); j++)
        {
            double s_1 = compare_histograms(histos_hs[i], histos_hs[j], CORRELATION); 
            double s_2 = compare_histograms(histos_hs[i], histos_hs[j], CHI_SQUARE); 
            double s_3 = compare_histograms(histos_hs[i], histos_hs[j], INTERSECTION); 
            double s_4 = compare_histograms(histos_hs[i], histos_hs[j], BHATTACHARYYA); 
            double s_5 = chi_square(histos[i], histos[j]);

            std::cout << i << " vs: " << j << " : (corr) " << s_1
                                           << " : (chiq) " << s_2
                                           << " : (inte) " << s_3
                                           << " : (bhat) " << s_4
                                           << " : (chiq) " << s_5
                                           << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
