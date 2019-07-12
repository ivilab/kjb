/* $Id: psi_background.h 17393 2014-08-23 20:19:14Z predoehl $ */
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
   |  Author: Jinyan Guan 
 * =========================================================================== */

#ifndef PSI_BACKGROUND_H
#define PSI_BACKGROUND_H
#include <people_tracking_cpp/pt_box_trajectory.h>
#include <video_cpp/video.h>

namespace kjb
{
namespace psi
{

/**
 * @brief   Compute the background using the median value
 */
void compute_median_background
(
    const Video& video, 
    Matrix& s_mat, 
    Matrix& r_mat, 
    Matrix& g_mat,
    size_t rate = 1
);

/**
 * @brief   Compute the background using median value after
 *          excluding the regions where there are tracks
 */
void compute_median_background
(
    const Video& video, 
    const pt::Box_trajectory_map& btrajs,
    Matrix& s_mat, 
    Matrix& s_count,
    Matrix& r_mat, 
    Matrix& r_count,
    Matrix& g_mat,
    Matrix& g_count
);


} //namespace psi
} //namespace kjb

#endif /* PSI_BACKGROUND_H */

