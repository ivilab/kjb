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

/* $Id: tracking_metrics.cpp 21596 2017-07-30 23:33:36Z kobus $ */

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include "graph/hungarian.h"
#include "m_cpp/m_vector_d.h"
#include "l_cpp/l_exception.h"
#include "tracking_cpp/tracking_metrics.h"
#include "tracking_cpp/tracking_entity.h"

#include <numeric>

using namespace kjb;
using namespace kjb::tracking;

std::vector<Correspondence> kjb::tracking::get_correspondence
(
    const Canonical_trajectory_map& gt_trajs,
    const Canonical_trajectory_map& hypo_trajs,
    double threshold
)
{
    typedef Canonical_trajectory_map::Trajectory_element Tr_element;

    std::vector<Correspondence> corrs;
    if(gt_trajs.empty() || hypo_trajs.empty())
    {
        // no correspondences
        return corrs;
    }

    IFT(gt_trajs.duration() == hypo_trajs.duration(), 
            Illegal_argument, 
            "Ground truth and test tracks must have same duration.");

    double sq_threshold = threshold * threshold;
    corrs.resize(gt_trajs.duration());


    // We need to be able to refer to tracks by integer index.  
    // Below will let us map from index back to Entity_id.
    
    std::vector<const kjb::Vector*> gt_points, hypo_points;

    std::vector<Entity_id> gt_ids, hypo_ids;
    kjb::Matrix pw_distance;

    for(size_t cur_frame = 0; cur_frame < gt_trajs.duration(); cur_frame++)
    {
        if(cur_frame > 0)
        {
            // get correspondences from previous frame
            // if valid, keep in this frame
            typedef Correspondence::left_const_reference Corr_map_elem;
            BOOST_FOREACH(Corr_map_elem pair, corrs[cur_frame-1].left)
            {
                const Entity_id& gt_id = pair.first;
                const Entity_id& hypo_id = pair.second;
            
                if(gt_id.type != hypo_id.type) continue;
                // get current time slice for these tracks
                typedef boost::optional<Tr_element> Op_tr_element;
                const Op_tr_element& gt_elem = gt_trajs.at(gt_id)[cur_frame];
                const Op_tr_element& hypo_elem = hypo_trajs.at(hypo_id)[cur_frame];

                // check if still valid in both tracks
                if(gt_elem && hypo_elem)
                {
                    // check if still within threshold
                    const kjb::Vector& gt_pt = gt_elem->value;
                    const kjb::Vector& hp_pt = hypo_elem->value;
                    if(kjb::vector_distance_squared(gt_pt, hp_pt) < sq_threshold)
                    {
                        corrs[cur_frame].left.insert(pair);
                    }
                }
            }
        }

        // extract unclaimed points for this time slice 
        // (ignores anything currently in correspondence)
        init_ids_and_points(gt_ids, gt_points, 
                            gt_trajs,
                            cur_frame, 
                            corrs[cur_frame], true);
        init_ids_and_points(hypo_ids, hypo_points, 
                            hypo_trajs,
                            cur_frame, 
                            corrs[cur_frame], false);

        // construct pairwise distance map
        get_pw_distance(gt_points, hypo_points, pw_distance);

        // find optimal matching
        std::vector<std::pair<int, int> > new_corr;
        get_best_matching(pw_distance, threshold, new_corr);

        // convert index correspondences to Entity_id correspondences
        for(size_t i = 0; i < new_corr.size(); i++)
        {
            int gt_i = new_corr[i].first;
            int hypo_i = new_corr[i].second;
            
            ASSERT(pw_distance(gt_i, hypo_i) <= threshold);

            Entity_id gt_obj = gt_ids[gt_i];
            Entity_id hypo_obj = hypo_ids[hypo_i];

            typedef Correspondence::value_type Corr_pair;
            Corr_pair pair(gt_obj, hypo_obj);
            corrs[cur_frame].insert(pair);
        }
    }

    return corrs;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */



void kjb::tracking::get_mt_ml_fragment_and_id_switch
(
    const std::vector<Correspondence>& corrs,
    const Canonical_trajectory_map& gt_trajs,
    const Canonical_trajectory_map& hyp_trajs,
    double& mt,
    double& ml,
    size_t& frags,
    size_t& id_switch
)
{
    if(corrs.empty()) return;

    size_t num_frames = corrs.size();
    Correspondence::right_const_iterator t_cur_iter;
    Correspondence::right_const_iterator t_next_iter;

    // Compute the covered gt_track percentage
    size_t mt_counts = 0;
    size_t ml_counts = 0;
    mt = 0.0;
    ml = 0.0; 
    frags = 0;
    id_switch = 0;

    typedef Canonical_trajectory_map::value_type  Traj_pair;
    BOOST_FOREACH(const Traj_pair& pair, gt_trajs)
    {
        size_t gt_trajs_len = 0; 
        size_t gt_trajs_covered_len = 0;
        const Entity_id& gt_id = pair.first;
        const Canonical_trajectory& gt_traj = pair.second;

        for(size_t frame = 0; frame < num_frames; frame++)
        {
            if(!gt_traj[frame]) continue;
            gt_trajs_len++;

            if(corrs[frame].left.count(gt_id) == 1)
            {
                gt_trajs_covered_len++;
            }

            // Check the fragments 
            if(frame == num_frames - 1) continue; 

            t_cur_iter = corrs[frame].right.find(gt_id); 
            t_next_iter = corrs[frame+1].right.find(gt_id);

            if(t_cur_iter != corrs[frame].right.end() &&
               t_next_iter != corrs[frame+1].right.end() &&
               t_cur_iter->second != t_next_iter->second)
            {
               frags++;
            }
        }
        double p = gt_trajs_covered_len/ static_cast<double>(gt_trajs_len);
             
        if(p > 0.8)
        {
            mt_counts++;
        }
        if(p < 0.2) 
        {
            ml_counts++;
        }
    }

    size_t num_gt_tracks = gt_trajs.size();
    mt = mt_counts/static_cast<double>(num_gt_tracks);
    ml = ml_counts/static_cast<double>(num_gt_tracks);

    // Compute the id switching
    Correspondence::left_const_iterator g_cur_iter;
    Correspondence::left_const_iterator g_next_iter;

    BOOST_FOREACH(const Traj_pair& pair, hyp_trajs)
    {
        const Entity_id& hyp_id = pair.first;
        const Canonical_trajectory& hyp_traj = pair.second;
        for(size_t frame = 0; frame < num_frames - 1; frame++)
        {
            if(!hyp_traj[frame]) continue;
            g_cur_iter = corrs[frame].left.find(hyp_id); 
            g_next_iter = corrs[frame+1].left.find(hyp_id);
            if(g_cur_iter != corrs[frame].left.end() &&
               g_next_iter != corrs[frame+1].left.end() &&
               g_cur_iter->second != g_next_iter->second)
            {
               id_switch++;
            }
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::tracking::get_mota_and_motp
(
    const std::vector<size_t>& mme_ct,
    const std::vector<size_t>& fp_ct,
    const std::vector<size_t>& miss_ct,
    const std::vector<size_t>& match_ct,
    const std::vector<size_t>& obj_ct,
    const std::vector<double>& dists,
    double& mota,
    double& motp
)
{
    size_t mme = std::accumulate(mme_ct.begin(), mme_ct.end(), 0);
    size_t fp = std::accumulate(fp_ct.begin(), fp_ct.end(), 0);
    size_t miss = std::accumulate(miss_ct.begin(), miss_ct.end(), 0);
    size_t match = std::accumulate(match_ct.begin(), match_ct.end(), 0);
    size_t obj = std::accumulate(obj_ct.begin(), obj_ct.end(), 0);
    double D = std::accumulate(dists.begin(), dists.end(), 0.0);
    motp = D/match;
    mota = 1.0 - static_cast<double>(fp + miss + mme)/obj;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::tracking::get_recall_precision
(
    const std::vector<size_t>& fp_ct,
    const std::vector<size_t>& miss_ct,
    const std::vector<size_t>& match_ct,
    const std::vector<size_t>& obj_ct,
    double& recall,
    double& precision
)
{
    size_t fp = std::accumulate(fp_ct.begin(), fp_ct.end(), 0);
    size_t miss = std::accumulate(miss_ct.begin(), miss_ct.end(), 0);
    size_t match = std::accumulate(match_ct.begin(), match_ct.end(), 0);
    size_t obj = std::accumulate(obj_ct.begin(), obj_ct.end(), 0);
    recall = match * 1.0 / (miss + match);
    precision = match * 1.0 / (match + fp); 
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::tracking::init_ids_and_points
(
    std::vector<Entity_id>& ids,
    std::vector<const kjb::Vector*>& points,
    const Canonical_trajectory_map& trajs,
    size_t cur_frame,
    const Correspondence& existing_corr,
    bool ground_truth
)
{
    ids.clear();
    points.clear();

    typedef Canonical_trajectory_map::value_type  Traj_pair;
    BOOST_FOREACH(const Traj_pair& traj_pair, trajs)
    {
        const Entity_id& track_id = traj_pair.first;
        const Canonical_trajectory& traj = traj_pair.second;

        // if correspondence carried-over from last frame, don't include
        if(ground_truth)
        {
            // check against ground truth id's
            if(existing_corr.left.count(track_id) > 0 )
                continue;
        }
        else
        {
            // check against test track id's
            if(existing_corr.right.count(track_id) > 0 )
                continue;
        }

        if(traj[cur_frame]) // has valid point at this frame?
        {
            ids.push_back(track_id);
            const Vector& pt = traj[cur_frame]->value;
            points.push_back(&pt);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::tracking::get_pw_distance
(
    const std::vector<const Vector*>& pts1,
    const std::vector<const Vector*>& pts2,
    Matrix& distance
)
{
    if(pts1.empty() || pts2.empty())
    {
        distance.resize(0, 0);
        return;
    }
    size_t sz1 = pts1.size();
    size_t sz2 = pts2.size();

    distance.resize(sz1, sz2);

    for(size_t i1 = 0; i1 < sz1; ++i1)
    {
        for(size_t i2 = 0; i2 < sz2; ++i2)
        {
            distance(i1, i2) = vector_distance(*pts1[i1], *pts2[i2]);
        }
    }
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::tracking::get_best_matching
(
    const kjb::Matrix& pw_distance, 
    double threshold, 
    std::vector<std::pair<int, int> >& matching
)
{
    kjb_c::Int_vector* row_vp = NULL;
    double cost = 0.0;
    ETX(kjb_c::hungarian(pw_distance.get_c_matrix(), &row_vp, &cost));
    matching.clear();
    for(int i = 0; i < row_vp->length; i++)
    {
        if(row_vp->elements[i] != -1)
        {
            // check the threshold
            if(pw_distance(i, row_vp->elements[i]) < threshold)
            {
                matching.push_back(std::make_pair(i, row_vp->elements[i]));
            }
        }
    }

    kjb_c::free_int_vector(row_vp);
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::tracking::get_counts_and_distances
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
)
{
    typedef Canonical_trajectory_map Traj_map;
    typedef Traj_map::value_type  Traj_pair;
    typedef Canonical_trajectory Traj;

    if(corrs.empty()) return;
    size_t num_frames = corrs.size();
    mme_ct.resize(num_frames);
    std::fill(mme_ct.begin(), mme_ct.end(), 0);
    fp_ct = mme_ct;
    miss_ct = mme_ct;
    match_ct = mme_ct;
    obj_ct = mme_ct;
    dists.resize(num_frames);
    std::fill(dists.begin(), dists.end(), 0.0);

    for(size_t frame = 0; frame < num_frames; frame++)
    {
        // MISSES
        // (All ground truth points without matches in hyp track)
        BOOST_FOREACH(const Traj_pair& pair, gt_trajs)
        {
            const Entity_id& gt_id = pair.first;
            const Traj& gt_traj = pair.second;

            if(!gt_traj[frame]) continue;
            obj_ct[frame]++;

            if(corrs[frame].left.count(gt_id) == 0)
            {
                // miss (false negative) 
                miss_ct[frame]++;
            }
        }

        // FALSE POSITIVES
        // (All test track points without matches in ground truth)
        BOOST_FOREACH(const Traj_pair& pair, hyp_trajs)
        {
            const Entity_id& hyp_id = pair.first;
            const Traj& hyp_traj = pair.second;

            if(!hyp_traj[frame]) continue;
            if(corrs[frame].right.count(hyp_id) == 0)
            {
                // false positive
                fp_ct[frame]++;
            }
        }

        // MISMATCH && DISTANCE
        typedef Correspondence::left_const_reference Corr_map_elem;
        BOOST_FOREACH(Corr_map_elem pair, corrs[frame].left)
        {
            Entity_id gt_id = pair.first;
            Entity_id hyp_id = pair.second;

            // get current time slice for these tracks
            typedef Canonical_trajectory::value_type Traj_elem;

            const Traj_elem& gt_elem = gt_trajs.at(gt_id)[frame];
            const Traj_elem& hyp_elem = hyp_trajs.at(hyp_id)[frame];

            ASSERT(gt_elem);
            ASSERT(hyp_elem);

            double dist = kjb::vector_distance(gt_elem->value, hyp_elem->value);

            //ASSERT(distance <= threshold);
            dists[frame] += dist;

            // find next correspondence for this g.t. track
            if(frame < num_frames - 1)
            {
                if(corrs[frame + 1].left.count(gt_id) != 0)
                {
                    // if correspondence changed, count as mismatch
                    if(corrs[frame + 1].left.at(gt_id) != hyp_id)
                    {
                        mme_ct[frame + 1]++;
                    }
                }
            }
        }

        // MATCH 
        match_ct[frame] += corrs[frame].size();
    }
}


