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
   |  Author:  Denis Gorbunov, Jinyan Guan
 * =========================================================================== */

/* $Id: cv_optical_flow.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#ifdef KJB_HAVE_OPENCV

#include <wrap_opencv_cpp/cv_optical_flow.h>
#include <wrap_opencv_cpp/cv_util.h>
#include <fstream>
#include <sstream>

namespace kjb
{
namespace opencv
{

#ifdef KJB_HAVE_OPENCV

std::vector<Vector> CV_optical_flow_pyr_lk::get_next_features
(
    const Image& prev_image, 
    const Image& next_image,
    const std::vector<Vector>& prev_features
) const
{
   
    // Need to convert RBG images to GRAYSCALE images


    //cv::Ptr<IplImage> cv_prev_img_gray = to_opencv_gray(prev_image);
    //cv::Ptr<IplImage> cv_next_img_gray = to_opencv_gray(next_image);
    
    Matrix prev_matrix = prev_image.to_grayscale_matrix();
    cv::Ptr<IplImage> cv_prev_img_gray = to_opencv_gray(prev_matrix);

    Matrix next_matrix = next_image.to_grayscale_matrix();
    cv::Ptr<IplImage> cv_next_img_gray = to_opencv_gray(next_matrix);

    unsigned int feature_count = prev_features.size();

    cv::Ptr<CvPoint2D32f> cv_prev_features = new CvPoint2D32f[feature_count];

    for (unsigned int i = 0; i < feature_count; i++)
    {
        const kjb::Vector& vector = prev_features[i];
        double x = vector[0];
        double y = vector[1];
        if(x < 0.0 || y < 0.0)
        {
            KJB_THROW_2(kjb::Runtime_error, "Optical flow features has negative values. ");
        }
        cv_prev_features[i] = cvPoint2D32f(x, y);
    }

    char features_found[feature_count];
    float feature_errors[feature_count];

    CvSize pyr_sz = cvSize(cv_prev_img_gray->width + 8, cv_next_img_gray->height / 3);

    cv::Ptr<IplImage> prev_pyr = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
    cv::Ptr<IplImage> next_pyr = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);

    // This will contain the points matched to points from featuresA
    cv::Ptr<CvPoint2D32f> cv_next_features = new CvPoint2D32f[feature_count];
    CvSize win_size = cvSize(window_height, window_width);
    
    CvTermCriteria cv_criteria = to_opencv(criteria);
    
    try
    {
        cvCalcOpticalFlowPyrLK(cv_prev_img_gray, cv_next_img_gray, prev_pyr, next_pyr, cv_prev_features, cv_next_features,
                feature_count, cvSize(window_height, window_width), max_level, features_found, feature_errors, cv_criteria, flags);
    } catch (cv::Exception & e)
    {
        std::ostringstream ost;
        ost << "Failed to calculate optical flow. Error: " << e.what();
        throw kjb::Result_error(ost.str());
    }

    std::vector<Vector> next_features;
    for (unsigned int i = 0; i < feature_count; i++)
    {
        // The corresponding features are not found 
        if (features_found[i])
        { 
            //std::cout<<static_cast<int>(features_found[i]);
            double x = cv_next_features[i].x;
            double y = cv_next_features[i].y;
            next_features.push_back(Vector(x, y));
        }
        else
        {
            next_features.push_back(Vector(-1.0, -1.0));
        }
    }

    return next_features;
}


std::vector<CV_optical_flow_feature> CV_optical_flow_pyr_lk::get_next_features
(
    const Image& prev_image, 
    const Image& next_image,
    const std::vector<CV_optical_flow_feature>& prev_features
) const
{
   
    unsigned int feature_count = prev_features.size();
    std::vector<Vector>  cv_prev_features(feature_count);

    for (unsigned int i = 0; i < feature_count; i++)
    {
        const kjb::Vector& vector = prev_features[i].value;
        cv_prev_features[i] = vector;
    }

    std::vector<Vector> cv_next_features = get_next_features(prev_image, next_image, cv_prev_features);
    std::vector<CV_optical_flow_feature> next_features;
    for (unsigned int i = 0; i < feature_count; i++)
    {
        size_t prev_feature_id = prev_features[i].id;
        // The corresponding features are not found 
        if (cv_next_features[i][0] > 0.0 && cv_next_features[i][1] > 0.0)
        { 
            //std::cout<<static_cast<int>(features_found[i]);
            next_features.push_back(CV_optical_flow_feature(prev_feature_id, cv_next_features[i]));
        }
    }

    return next_features;
}

void CV_optical_flow_pyr_lk::write_features
(
    const std::string& out_fname,
    const std::vector<Vector>& cur_features,
    const std::vector<Vector>& next_features
) const
{
    ASSERT(cur_features.size() >= next_features.size());
    std::ofstream out;
    out.open(out_fname.c_str());
    if(out.fail())
    {
        KJB_THROW_3(kjb::IO_error, "Could not open file: %s", (out_fname.c_str()));
    }
    else
    {
        for (size_t i = 0; i < next_features.size(); i++)
        {
            // Check whether the corresponding features has been detected 
            // by checking the feature id in the next_features
            if(next_features[i][0] > 0.0 && next_features[i][1] > 0.0)
                out<<cur_features[i]<<"\t" <<next_features[i]<<"\t" << std::endl;
        }
    }
}

void CV_optical_flow_pyr_lk::write_features
(
    const std::string& out_fname,
    const std::vector<CV_optical_flow_feature>& cur_features,
    const std::vector<CV_optical_flow_feature>& next_features
) const 
{
    ASSERT(cur_features.size() >= next_features.size());
    std::ofstream out;
    out.open(out_fname.c_str());
    if(out.fail())
    {
        KJB_THROW_3(kjb::IO_error, "Could not open file: %s", (out_fname.c_str()));
    }
    else
    {
        size_t cur_index = 0; 
        for (size_t next_index = 0; next_index < next_features.size(); next_index++, cur_index++)
        {
            // Check whether the corresponding features has been detected 
            // by checking the feature id in the next_features
            while(cur_features[cur_index].id < next_features[next_index].id)
            {
                cur_index++;
            }
            out<<cur_features[cur_index].value<<"\t" <<next_features[next_index].value<<"\t" << std::endl;
        }
    }
}

std::vector<CV_optical_flow_feature> CV_optical_flow_pyr_lk::convert_to_optical_flow_feature
(
    const std::vector<Vector> & features
)
{
    std::vector<CV_optical_flow_feature> of_features(features.size());
    for(size_t i = 0; i < features.size(); i++)
    {
        of_features[i] = CV_optical_flow_feature(i, features[i]);
    }
    
    return of_features;
}

#endif

} //namespace opencv 
} //namespace kjb
#endif
