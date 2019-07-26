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
   |  Author:  Kyle Simek, Jinyan Guan
 * =========================================================================== */

/* $Id: cv_histogram.h 15022 2013-07-28 22:31:58Z jguan1 $ */

#ifndef CV_HISTOGRAM_H_
#define CV_HISTOGRAM_H_

#include <m_cpp/m_vector.h>
#include <i_cpp/i_image.h>
#include <gr_cpp/gr_2D_bounding_box.h>

#ifdef KJB_HAVE_OPENCV
#include <wrap_opencv_cpp/cv.h>
#endif

#include <vector>

namespace kjb
{
namespace opencv
{

enum HIST_COMP_METHOD {CORRELATION, CHI_SQUARE, INTERSECTION, BHATTACHARYYA };
   
/**
 * @brief Calculates a histogram of values in src
 * @param src:          Source Image.
 * @param num_bins:     Size of histograms of each R, G, B component (number of bins)
 * @param lower_bound:  Lower bound of the range of histogram 
 * @param upper_bound:  Upper bound of the range of histogram (exclusive)
 *
 */
Matrix calculate_histogram
(
    const Matrix& src,
    int num_bins,
    float lower_bound = 0.0f,
    float upper_bound = 256.0f
);

/**
 * @brief Calculates a histogram of values in src
 * @param img:          Source Image.
 * @param box:          The region of interest
 * @param red_bins:     Size of histograms of each r component
 * @param green_bins:   Size of histograms of each g component
 *
 */
Matrix calculate_rg_histogram
(
     const Image& img, 
     const Axis_aligned_rectangle_2d& box,
     int red_bins,
     int green_bins
);

/**
 * @brief Calculates a histogram of values in src
 * @param img:              Source Image.
 * @param boxes:            Regions of interest
 * @param hue_bins:         Size of histograms of each hue component
 * @param saturation_bins:  Size of histograms of each saturation component
 *
 */
Matrix calculate_hs_histogram
(
     const Image& img, 
     const Axis_aligned_rectangle_2d& box,
     int hue_bins,
     int saturation_bins
);

/**
 * @brief Calculates a histogram of values in src
 * @param img:          Source Image.
 * @param red_bins:     Size of histograms of each r component
 * @param green_bins:   Size of histograms of each g component
 *
 */
std::vector<Matrix> calculate_rg_histograms
(
     const Image& img, 
     const std::vector<Axis_aligned_rectangle_2d> & boxes,
     int red_bins,
     int green_bins
);

/**
 * @brief Calculates a histogram of values in src
 * @param img:              Source Image.
 * @param boxes:            A vector of regions of interest
 * @param hue_bins:         Size of histograms of each hue component
 * @param saturation_bins:  Size of histograms of each saturation component
 *
 */
std::vector<Matrix> calculate_hs_histograms
(
     const Image& img, 
     const std::vector<Axis_aligned_rectangle_2d> & boxes,
     int hue_bins,
     int saturation_bins
);

double compare_histograms(const Matrix& h1, const Matrix& h2, int method);

}
} // namespace kjb::opencv
#endif

