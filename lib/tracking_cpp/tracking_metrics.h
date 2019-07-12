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
   |  Author:  Kyle Simek, Jinyan Guan, Ernesto Brau
 * =========================================================================== */

/* $Id: tracking_metrics.h 21596 2017-07-30 23:33:36Z kobus $ */

#ifndef TRACKING_METRICS_H
#define TRACKING_METRICS_H

#include "m_cpp/m_matrix.h"
#include "tracking_cpp/tracking_entity.h"
#include "tracking_cpp/tracking_trajectory.h"

#include <boost/bimap.hpp>
#include <vector>

namespace kjb {
namespace tracking {

typedef boost::bimap<Entity_id, Entity_id> Correspondence;

/** 
 * @brief   Compute the correspondence between the groud truth track 
 *          and hypothesized tracks
 */
std::vector<Correspondence> get_correspondence 
(
    const Canonical_trajectory_map& gt_trajs,
    const Canonical_trajectory_map& hyp_trajs,
    double threshold = 1.0
);

/** 
 * @brief   Compute most_tracked, most_lost and id_switches  
 *          for a given correspondence
 */
void get_mt_ml_fragment_and_id_switch
(
    const std::vector<Correspondence>& corrs,
    const Canonical_trajectory_map& gt_trajs,
    const Canonical_trajectory_map& hyp_trajs,
    double& mt,
    double& ml,
    size_t& frags,
    size_t& id_switch
);

/**
 * @brief   Compute metrics based on the counts
 */
void get_mota_and_motp
(
    const std::vector<size_t>& mme_ct,
    const std::vector<size_t>& fp_ct,
    const std::vector<size_t>& miss_ct,
    const std::vector<size_t>& match_ct,
    const std::vector<size_t>& obj_ct,
    const std::vector<double>& dists,
    double& mota,
    double& motp
);

/**
 * @brief   Compute the recall and precision.
 */ 
void get_recall_precision
(
    const std::vector<size_t>& fp_ct,
    const std::vector<size_t>& miss_ct,
    const std::vector<size_t>& match_ct,
    const std::vector<size_t>& obj_ct,
    double& recall,
    double& precision
);

/**
 * @brief   Extract unclaimed points for this time slice 
 *          (ignores anything currently in exsiting correspondence)
 */
void init_ids_and_points
(
    std::vector<Entity_id>& ids,
    std::vector<const Vector*>& points,
    const Canonical_trajectory_map& trajs,
    size_t cur_frame,
    const Correspondence& existing_corr,
    bool ground_truth
);

/**
 * @brief   Compute the pairwise distance between pts1 and pts2
 *          and stores the results in Matrix distance
 */
void get_pw_distance
(
    const std::vector<const Vector*>& pts1,
    const std::vector<const Vector*>& pts2,
    Matrix& distance
);

/**
 * @brief   Compute the best matching using hungarian algorithm
 */
void get_best_matching
(
    const kjb::Matrix& pw_distance, 
    double threshold, 
    std::vector<std::pair<int, int> >& matching
);

/**
 * @brief   Compute the counts (match, miss match, false positives, 
 *          grouth truth objects, and the distance between matched
 *          grouth truth and hypothesized tracks
 */
void get_counts_and_distances
(
    const std::vector<Correspondence>& corrs,
    const Canonical_trajectory_map& gt_trajs,
    const Canonical_trajectory_map& hyp_trajs,
    std::vector<size_t>& mme_ct,
    std::vector<size_t>& fp_ct,
    std::vector<size_t>& miss_ct,
    std::vector<size_t>& match_ct,
    std::vector<size_t>& obj_ct,
    std::vector<double>& dists
);

}} // namespace kjb::tracking


#endif /* TRACKING_METRICS_H */

