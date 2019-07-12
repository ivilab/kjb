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

/* $Id: cv_features_to_track.cpp 21155 2017-01-29 19:20:33Z jguan1 $ */

#include <wrap_opencv_cpp/cv_features_to_track.h>
#include <wrap_opencv_cpp/cv_util.h>

namespace kjb
{
namespace opencv
{

#ifdef KJB_HAVE_OPENCV
std::vector<Vector> CV_features_to_track_detector::find_good_features
(
    const Image& image
) const
{
    const int MAX_CORNERS = 5000;
    //cv::Ptr<IplImage> cv_img = to_opencv(image);
    // Need to convert RBG images to GRAYSCALE images
    Matrix matrix_gray = image.to_grayscale_matrix();
    cv::Ptr<IplImage> cv_img_gray = to_opencv_gray(matrix_gray);

    cv::Ptr<CvPoint2D32f> corners = new CvPoint2D32f[MAX_CORNERS]; 
    
    CvSize img_sz = cvGetSize(cv_img_gray);
    cv::Ptr<IplImage> eig_image = cvCreateImage(img_sz, IPL_DEPTH_32F, 1);
    cv::Ptr<IplImage> tmp_image = cvCreateImage(img_sz, IPL_DEPTH_32F, 1);

    int corner_count = 0;
    
    if(use_harris == true)
        // Use Harris operator and k  
        cvGoodFeaturesToTrack(cv_img_gray, eig_image, tmp_image, corners, &corner_count, 
                                quality_level, min_distance, NULL, block_size, 1, k);
    else 
        // Use default CornerMinEigenVal
        cvGoodFeaturesToTrack(cv_img_gray, eig_image, tmp_image, corners, &corner_count, 
                                quality_level, min_distance, NULL, block_size);

    std::vector<Vector> features(corner_count, Vector(2, 0.0));
    for(int i = 0; i < corner_count; i++)
    {
        features[i](0) = corners[i].x;
        features[i](1) = corners[i].y;
    }

    return features;
}

std::vector<Vector> CV_features_to_track_detector::find_good_features
(
    const Image& image, 
    int x, 
    int y, 
    int width,
    int height 
) const
{
    if(x > image.get_num_cols() || y > image.get_num_rows() ||
        (width-x) > image.get_num_cols() || (height-y) > image.get_num_rows() )
    {
        KJB_THROW_2(kjb::Illegal_argument, "Bad region of interst of the image");
    }

    CvRect roi = cvRect(x, y, width, height);

    const int MAX_CORNERS = 1000;

    Matrix matrix_gray = image.to_grayscale_matrix();
    cv::Ptr<IplImage> cv_img_gray = to_opencv_gray(matrix_gray);
    //cv::Ptr<IplImage> cv_img_gray = to_opencv_gray(image);

    CvSize img_sz = cvGetSize(cv_img_gray);

    cv::Ptr<CvPoint2D32f> corners = new CvPoint2D32f[MAX_CORNERS]; 
    
    cv::Ptr<IplImage> eig_img = cvCreateImage(img_sz, IPL_DEPTH_32F, 1);
    cv::Ptr<IplImage> tmp_img = cvCreateImage(img_sz, IPL_DEPTH_32F, 1);

    cvSetImageROI(cv_img_gray, roi);
    cvSetImageROI(eig_img, roi);
    cvSetImageROI(tmp_img, roi);

    int corner_count = 0;
   
    if(use_harris == true)
        // Use Harris operator and k  
        cvGoodFeaturesToTrack(cv_img_gray, eig_img, tmp_img, corners, &corner_count, 
                                quality_level, min_distance, NULL, block_size, 1, k);
    else 
        // Use default CornerMinEigenVal
        cvGoodFeaturesToTrack(cv_img_gray, eig_img, tmp_img, corners, &corner_count, 
                                quality_level, min_distance, NULL, block_size);

    std::vector<Vector> features(corner_count, Vector(2, 0.0));
    for(int i = 0; i < corner_count; i++)
    {
        features[i](0) = corners[i].x + x;
        features[i](1) = corners[i].y + y;
    }

    cvResetImageROI(cv_img_gray);
    cvResetImageROI(eig_img);
    cvResetImageROI(tmp_img);

    return features;
}

//#else
//    KJB_THROW_2(Missing_dependency, "opencv");
#endif

} // namespace opencv 
} // namespace kjb
