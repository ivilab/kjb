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

/* $Id: cv_optical_flow.h 21062 2017-01-13 18:54:05Z kobus $ */

#ifndef CV_OPTICAL_FLOW_H_
#define CV_OPTICAL_FLOW_H_

#include <vector>
#include <m_cpp/m_vector.h>
#include <m_cpp/m_matrix.h>
#include <i_cpp/i_image.h>

#ifdef KJB_HAVE_OPENCV

#include <wrap_opencv_cpp/cv_util.h>

/* Kobus: Added Jan 13, 2017. */
#include <opencv2/video.hpp>

#endif
namespace kjb
{
namespace opencv
{
/**
 * Wrapper for CvTermCritera.
 */

#define  CV_LKFLOW_PYR_A_READY       1
#define  CV_LKFLOW_PYR_B_READY       2
#define  CV_LKFLOW_INITIAL_GUESSES   4

#define DEFAULT_WINDOW_HEIGHT 15
#define DEFAULT_WINDOW_WIDTH 15
#define DEFAULT_MAX_LEVEL 3


/**
 *
 * @brief A wrapper class for calcOpticalFlowPyrLK() function 
 */
#ifdef KJB_HAVE_OPENCV

struct CV_optical_flow_feature
{
    size_t id;
    Vector value; 
    CV_optical_flow_feature() {}

    CV_optical_flow_feature(size_t id_, const Vector& value_)
        : id(id_), value(value_)
    {}
};

class CV_optical_flow_pyr_lk
{
public:
    // Constructor 
    CV_optical_flow_pyr_lk() 
        : window_height(DEFAULT_WINDOW_HEIGHT), 
          window_width(DEFAULT_WINDOW_WIDTH),
          max_level(DEFAULT_MAX_LEVEL),
          criteria(CV_term_criteria()),
          flags(0)
    {}

    CV_optical_flow_pyr_lk
    (
        int window_height_,
        int window_width_,
        int max_level_,
        const CV_term_criteria& criteria_,
        int flags_
    ) : 
        window_height(window_height_),
        window_width(window_width_),
        max_level(max_level_),
        criteria(criteria_),
        flags(flags_)
    {}

    std::vector<Vector> get_next_features
    (
        const Image& prev_image, 
        const Image& next_image,
        const std::vector<Vector>& prev_features
    ) const;

    /**
     * @brief Return a set of features on the next frame 
     */
    std::vector<CV_optical_flow_feature> get_next_features
    (
        const Image& prev_image, 
        const Image& next_image,
        const std::vector<CV_optical_flow_feature>& prev_features
    ) const;

    std::vector<CV_optical_flow_feature> convert_to_optical_flow_feature
    (
        const std::vector<Vector> & features
    );
    
    /**
     * @brief Write piecewise optical flow features 
     */
    void write_features
    (
        const std::string& out_fname,
        const std::vector<Vector>& cur_features,
        const std::vector<Vector>& next_features
    ) const;

    /** 
     * @brief Write corresponding optical flow features based on the feature id 
     */
    void write_features
    (
        const std::string& out_fname,
        const std::vector<CV_optical_flow_feature>& cur_features,
        const std::vector<CV_optical_flow_feature>& next_features
    ) const;

private: 

    /**
     * Size of the search window of each pyramid level
     */
    int window_height;
    int window_width;

    /**
     * Maximal pyramid level number. 
     * If 0 , pyramids are not used (single level), if 1 , two levels are used, etc
     */
    int max_level;

    /**
     * Specifies when the iteration process of finding the flow for each point on each pyramid 
     * level should be stopped
     */
    CV_term_criteria criteria;

    /**
     * 1: CV_LKFLOWPyr_A_READY - pyramid for the first frame is precalculated before the call
     * 2: CV_LKFLOWPyr_B_READY - pyramid for the second frame is precalculated before the call
     * 4: CV_LKFLOW_INITIAL_GUESSES - array B contains initial coordinates of features before the function call
     *
     */
    int flags;
};
#endif


} // namespace opencv
} // namespace kjb

#endif /* OPTICAL_FLOW_H_ */
