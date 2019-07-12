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
/* $Id: cv_object_detect.cpp 21074 2017-01-15 02:46:25Z jguan1 $ */

#ifdef KJB_HAVE_OPENCV
#include <wrap_opencv_cpp/cv_object_detect.h>
#endif

namespace kjb
{
namespace opencv
{

#ifdef KJB_HAVE_OPENCV
void CV_cascade_classifier::detect_multiscale
(
    const Image& img,
    std::vector<Axis_aligned_rectangle_2d>& objs, 
    double scale_factor, 
    int min_neighbors, 
    int flags, 
    int width,
    int height
)
{
    using namespace cv;
    Matrix kjb_mat = img.to_grayscale_matrix();
    cv::Ptr<IplImage> cv_img = to_opencv_gray(kjb_mat);
    Mat mat = cvarrToMat(cv_img);
    Size size(width, height);

    std::vector<Rect> obj_rects;
    cv_classifier.detectMultiScale(mat, obj_rects, 
                                    scale_factor, min_neighbors, 
                                    flags, size);

    objs.reserve(obj_rects.size());
    std::vector<Rect>::const_iterator iter;
    for(iter = obj_rects.begin(); iter != obj_rects.end(); iter++)
    {
        Vector p1(2, 0.0);
        p1.set(iter->tl().x, iter->tl().y);
        Vector p2(2, 0.0);
        p2.set(iter->br().x, iter->br().y);
        objs.push_back(Axis_aligned_rectangle_2d(p1, p2));
    }
}

#endif    
} // namespace opencv
} // namespace kjb
