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

/* $Id: cv_util.h 21172 2017-01-30 09:21:24Z kobus $ */

#ifndef CV_UTIL_H_
#define CV_UTIL_H_

#include <i_cpp/i_image.h>
#include <m_cpp/m_matrix.h>

#ifdef KJB_HAVE_OPENCV

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#endif

#define CV_TERMCRIT_ITER    1
#define CV_TERMCRIT_NUMBER  CV_TERMCRIT_ITER
#define CV_TERMCRIT_EPS     2

#define DEFAULT_MAX_ITER 30 
#define DEFAULT_MAX_EPSILON 0.1

namespace kjb
{
namespace opencv
{

struct CV_term_criteria
{
    // A combination of CV_ITERMCRIT_ITER and CV_TERMCRIT_EPS
    int type;

    // Maximum number of iterations
    int max_iter;

    // Required accuracy 
    double epsilon;

    // Constructor
    CV_term_criteria():
        type(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS),
        max_iter(DEFAULT_MAX_ITER),
        epsilon(DEFAULT_MAX_EPSILON)
    {}

    inline CV_term_criteria
    (
        int type_,
        int max_iter_,
        double epsilon_
    ) : 
        type(type_),
        max_iter(max_iter_),
        epsilon(epsilon_)
    {}

};

#ifdef KJB_HAVE_OPENCV
/**
 * @brief   Convert a CV_term_criteria to cv::CvTermCriteria
 */
inline CvTermCriteria to_opencv(CV_term_criteria criteria)
{
    CvTermCriteria cv_criteria;
    cv_criteria.type = criteria.type; 
    cv_criteria.max_iter = criteria.max_iter; 
    cv_criteria.epsilon = criteria.epsilon;
     
    return cv_criteria;
}

/**
 * @brief   Convert a kjb::Image to a IplImage
 */
//cv::Ptr<IplImage> to_opencv(const Image& image);

/**
 * @brief Convert a kjb::Image to a IplImage 
 */
cv::Ptr<IplImage> to_opencv_gray(const Image& image);

/**
 * @brief   Convert a kjb::Matrix to a IplImage 
 */
cv::Ptr<IplImage> to_opencv_gray(const Matrix& matrix);

/**
 * @brief   Convert a kjb::Image to a cv::Mat
 */
cv::Mat to_opencv(const Image& img);

/**
 * @brief   Convert a kjb::Matrix to a cv::Mat
 * @param   matrix is a kjb::Matrix with doubles
 * @return  a cv::Mat with type float
 */
cv::Mat to_opencv(const Matrix& matrix);

/**
 * @brief   Cnovert a cv::Mat to kjb::Matrix 
 * @param   cv_mat is a cv::Mat with 32F1C 
 * @return  a kjb::Matrix of doubles
 */
Matrix opencv_to_kjb(const cv::Mat& cv_mat);

#endif
} // namespace opencv
} // namespace kjb

#endif
