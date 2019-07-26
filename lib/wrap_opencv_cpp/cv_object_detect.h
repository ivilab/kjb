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

/* $Id: cv_object_detect.h 21062 2017-01-13 18:54:05Z kobus $ */

#ifndef CV_OBJECT_DETECT_H_
#define CV_OBJECT_DEFECT_H_

#ifdef KJB_HAVE_OPENCV
#include <wrap_opencv_cpp/cv.h>

/* Kobus: Added Jan 13, 2017, but not sure if this right. */
#include <opencv2/objdetect.hpp>

#endif

#include <gr_cpp/gr_2D_bounding_box.h>
#include <wrap_opencv_cpp/cv_util.h>

#include <vector>

namespace kjb
{
namespace opencv
{
#ifdef KJB_HAVE_OPENCV
/**
 * @class The cascade classifier class for object detection.
 */
class CV_cascade_classifier
{
public:
    // Constructors ... 
    CV_cascade_classifier(){}

    CV_cascade_classifier(const std::string& classifier_fname)
        : cv_classifier(classifier_fname)
    {}

    ~CV_cascade_classifier(){}

public:
    /**
     * @brief Load the classifier from a file
     */
    bool load(const std::string& classifier_fname)
    {
        return cv_classifier.load(classifier_fname);
    }

    /**
     * @brief Detect the object
     */
    void detect_multiscale
    (
        const Image& img,
        std::vector<Axis_aligned_rectangle_2d>& objs, 
        double scale_factor = 1.1,
        int min_neighbors = 3,
        int flags = 0, 
        int width = 0,
        int height = 0
    );

private:
    cv::CascadeClassifier cv_classifier;

}; // class CV_cascade_classifier

#endif

} //namespace cv
} //namespace kjb
#endif /* CV_OBJECT_DETECT_H_ */



