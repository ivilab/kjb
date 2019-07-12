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

/* $Id: cv_features_to_track.h 21172 2017-01-30 09:21:24Z kobus $ */

#ifndef CV_FEATURES_TO_TRACK_DETECTOR_H_
#define CV_FEATURES_TO_TRACK_DETECTOR_H_

#ifdef KJB_HAVE_OPENCV
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#endif

#include <i_cpp/i_image.h>
#include <m_cpp/m_vector.h>

#define DEFAULT_QUALITY_LEVEL 0.05
#define DEFAULT_MIN_DISTANCE 5.0
#define DEFAULT_BLOCK_SIZE 3 
#define DEFAULT_K_VALUE 0.04
//#define DEFAULT_MAX_CORNERS 500

namespace kjb
{
namespace opencv
{

class CV_features_to_track_detector
{
public: 
    // Constructor
    CV_features_to_track_detector():
        quality_level(DEFAULT_QUALITY_LEVEL),
        min_distance(DEFAULT_MIN_DISTANCE),
        block_size(DEFAULT_BLOCK_SIZE),
        use_harris(false),
        k(DEFAULT_K_VALUE)
    {}

    CV_features_to_track_detector
    (
        double quality_level_,
        double min_distance_, 
        int block_size_,
        bool use_harris_,
        double k_ 
    ) : 
        quality_level(quality_level_),
        min_distance(min_distance_),
        block_size(block_size_),
        use_harris(use_harris_),
        k(k_) 
    {}  

    ~CV_features_to_track_detector(){}
    

public:
    std::vector<Vector> find_good_features
    (
        const Image& image
    ) const;

    std::vector<Vector> find_good_features
    (
        const Image& image, 
        int lower_left_x, 
        int lower_left_y, 
        int higher_right_x,
        int higher_right_y
    ) const;

private:

    /**
     * Multiplier for the max/min eigenvalue; 
     * specifies the minimal accepted quality of image corners
     */
    double quality_level; 

    /**
     * Limit, specifying the minimum possible distance between the returned corners;
     * Euclidian distance is used
     */
    double min_distance;

    /**
     * Size of the averaging block, used in the corner detection algorithm 
     */
    int block_size;

    /**
     * If true, Harris operator is used instead of default CornerMinEigenVal
     */
    bool use_harris;

    /**
     * Free parameter of Harris detector; used only if ( useHaairs = true )
     */
    double k;

};



} //namespace opencv
} // namespace kjb



#endif /* CV_FEATURES_TO_TRACK_DETECTOR_H_ */
