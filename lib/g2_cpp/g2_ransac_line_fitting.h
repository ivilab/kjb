/* $Id: g2_ransac_line_fitting.h 17393 2014-08-23 20:19:14Z predoehl $ */
/* {{{=========================================================================== *
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
 * =========================================================================== }}}*/

#ifndef KJB_CPP_G_RANSAC_LINE_FITTING_H
#define KJB_CPP_G_RANSAC_LINE_FITTING_H

#include <m_cpp/m_vector.h>
#include <g_cpp/g_line.h>

#include <vector>

namespace kjb
{

/**
 * @file Using Ransac to fit a set of points to a line 
 *
 */
class Ransac_line_fitting
{
public:

    /**
     * Ctor for using ransac to fit line 
     *
     * @param observations_ a set of 2D points
     *        num_inliers_required_ the minimum number of data required to fit the model 
     *        threshold_ a threshold value for determining when a datum fits a model
     */
    Ransac_line_fitting
    (
       const std::vector<Vector>& observations_,
       size_t num_inliers_required_,
       double threshold_
    ) : observations(observations_),
        num_inliers_required(num_inliers_required_),
        threshold(threshold_),
        best_error(DBL_MAX)
    {}

    /**
     * Running the RASAC for the max_num_iter iterations 
     * Return ture if it found a model 
     */
    bool run(size_t max_num_iter);

    Line get_best_line() const { return best_line; }
    double get_best_fitting_error() const { return best_error; }
    const std::vector<Vector>& get_consensus_set() const { return consensus_set; }

private: 
    const std::vector<Vector>& observations;
    std::vector<Vector> consensus_set;
    Line best_line;
    size_t num_inliers_required;
    double threshold;
    double best_error;

};

} //namespace kjb
#endif /*KJB_CPP_G_RANSAC_LINE_FITTING_H */

