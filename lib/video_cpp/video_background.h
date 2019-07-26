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

/* $Id$ */

#ifndef KJB_VIDEO_BACKGROUND_H_
#define KJB_VIDEO_BACKGROUND_H_

#include <m_cpp/m_matrix.h>
#include <string>
#include <vector>

namespace kjb {

/** 
 * @brief   Compute the background value by a median filter
 */
void compute_median_background
(
    const std::vector<std::string>& frames_fps,
    Matrix& r_mat,
    Matrix& g_mat,
    Matrix& s_mat,
    size_t sub_sampling_sz = 1,
    size_t rate = 1
);

} //namespace kjb

#endif /*KJB_VIDEO_BACKGROIUND_H */

