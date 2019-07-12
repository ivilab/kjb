#include <wrap_opencv_cpp/cv_features_to_track.h>
#include <wrap_opencv_cpp/cv_util.h>

#include <i_cpp/i_image.h>

#ifdef KJB_HAVE_OPENCV
#include <opencv2/opencv.hpp>
#else
#error "Need OpenCV2 library"
#endif

using namespace kjb;
using namespace kjb::opencv;
int main(int argc, char* argv[])
{
    // Read an image 
    const char* img_name = argv[1];
    Image test_img(img_name);

    // detect the features
    CV_features_to_track_detector feature_detector;
    std::vector<Vector> good_features = feature_detector.find_good_features(test_img);

    // draw the features 
    //Image::Pixel_type pixel(1.0, 0.0, 0.0);
    std::cout<<"Num of features: "<<good_features.size()<<std::endl;
    for(size_t i = 0; i < good_features.size(); i++)
    {
        Image::Pixel_type pix;
        pix.r = 255.0;
        pix.g = 0.0; 
        pix.b = 0.0;
        test_img.draw_point(good_features[i](1), good_features[i](0), 1.0, pix);
    }
   
    test_img.write("features.jpg");
    return EXIT_SUCCESS;
}
