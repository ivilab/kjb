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

/* $Id: flow_feature_set.h 18278 2014-11-25 01:42:10Z ksimek $ */

#ifndef KJB_FLOW_FEATURE_SET_H
#define KJB_FLOW_FEATURE_SET_H

#include <m_cpp/m_vector.h>
#include <gr_cpp/gr_2D_bounding_box.h>

#include <set>
#include <vector>
#include <string>
#include <utility> 
#include <numeric> 


namespace kjb {

/** @brief   Move direction of the 2D bounding box. */
enum MOVE_DIRECTION { COL_PLUS, COL_MINUS, ROW_PLUS, ROW_MINUS }; 

typedef std::pair<float, float> Feature;
typedef std::pair<Vector, Vector> Feature_pair;

/** @brief  Functor to compare two Feature_pair */
struct Compare_flow_features
{
    /** @brief  Apply this functor. */
    bool operator()
    (
        const Feature_pair& feature_pair_1, 
        const Feature_pair& feature_pair_2
    ) const
    {
        return feature_pair_1.first < feature_pair_2.first;
    }
};

typedef std::set<Feature_pair, Compare_flow_features> Flow_feature_set;

/**
 * @brief   Read in the procomputed optical flow from files 
 */
Flow_feature_set read_flow_features
(
    const std::string& fname 
);

/**
 * @brief   Given the features and a bounding box at the same frame,
 *          find and return the detected features inside the bounding box
 */
std::vector<Feature_pair> look_up_features
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& box
);

/**
 * @brief   Given the features and all the model boxes at cur_frame,
 *          find all the background features
 */
std::vector<Feature_pair> look_up_bg_features
(
    const Flow_feature_set& of_set,
    const std::vector<Axis_aligned_rectangle_2d>& model_boxes
);

/**
 * @brief   Calculate the average optical flow for a vector of flow Vectors
 *
 */
inline
Vector average_flow
(
    const std::vector<Vector>& flows
)
{
    Vector ave_flow(2, 0.0);
    Vector total_flow = std::accumulate(flows.begin(), flows.end(), ave_flow);
    if(flows.size() > 0)
    {
        ave_flow = total_flow/flows.size();
    }
    return ave_flow;
}

/** 
 * @brief   Compare the optical flow magnitude 
 */
inline
bool compare_flow_magnitude
(
    const Vector& f_p,
    const Vector& f_n
)
{
    return f_p.magnitude() > f_n.magnitude();
}

/**  @brief   Get valid features  */
std::vector<Vector> valid_flow
(
    const std::vector<Feature_pair>& feature_pairs,
    double percentile = 1.0
);

/**  
 * @brief   Get valid flow features that with angle 
 *          alpha in the rangle (min_angle, max_angle)
 * 
 **/
std::vector<Vector> valid_flow
(
    const std::vector<Feature_pair>& feature_pairs,
    const std::vector<size_t>& angle_hist
);

/**  @brief   Compute the histogram of the flow vector angles */
std::vector<size_t> angle_histogram
(
    const std::vector<Feature_pair>& feature_pairs,
    size_t num_bins = 36 
); 

/** 
 * @brief  Update the average flow velocity 
 *         based on the moving direction
 */
Vector update_average_velocity
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& old_model_box,
    const Vector& old_velocity,
    MOVE_DIRECTION dir,
    size_t unit = 1
);

/**
 * @brief   Update the average model velocity 
 *
 */
Vector update_average_velocity
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& old_box,
    const Axis_aligned_rectangle_2d& new_box,
    const Vector& old_velocity
);

/** @brief  Looks up a feature in a feature set; deals with subsampling. */
Vector lookup_feature
(
    const Flow_feature_set& of_set,
    size_t x,
    size_t y,
    size_t subsample_sz = 1
);

/** 
 * @brief   Compute the average flow vector inside the bounding box
 */
Vector total_flow
(
    const Flow_feature_set& of_set,
    const Axis_aligned_rectangle_2d& box
    //size_t subsample_sz = 1
);

} //namespace kjb

#endif /* KJB_PSI_OPTICAL_FLOW_H */

