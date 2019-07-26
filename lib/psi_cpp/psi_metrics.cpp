/* $Id: psi_metrics.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include "l/l_sys_debug.h"  /* For ASSERT */
#include "psi_cpp/psi_metrics.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"

namespace kjb
{
namespace psi
{

/** 
* Utility method, not intended for calling directly
* Populate a vector with all points for current time slice.
* Also create a vector to tell use which track each point belongs to.
* Allows caller to specify certain tracks to ignore when populating
* @param left Indicates whether to ignore keys of the left or right side of the bimap.
*/
namespace metrics 
{

typedef pt::Trajectory_map::value_type  Traj_pair;
typedef Correspondence::left_const_reference Corr_pair;

void init_ids_and_points(
        std::vector<pt::Entity_id>& ids,
        std::vector<const Vector3*>& points,
        const pt::Trajectory_map& track_map,
        size_t cur_frame,
        const Track_metrics::Correspondence& ignore,
        bool left)
{
//    typedef std::pair<pt::Entity_id, Trajectory> Traj_pair;
    ids.clear();
    points.clear();
    BOOST_FOREACH(const Traj_pair& traj_pair, track_map)
    {
        const pt::Entity_id& track_id = traj_pair.first;
        const pt::Trajectory& traj = traj_pair.second;

        // if correspondence carried-over from last frame, don't include
        if(left)
        {
            // check against ground truth id's
            if(ignore.left.count(track_id) > 0 )
                continue;
        }
        else
        {
            // check against test track id's
            if(ignore.right.count(track_id) > 0 )
                continue;
        }

        if(traj[cur_frame]) // has valid point at this frame?
        {
            ids.push_back(track_id);
            const Vector3& pt = traj[cur_frame]->value.position;
            points.push_back(&pt);
        }
    }

}

/*
 * Splitting this into a template function lets the compiler optimize the
 * if-statement out of the inner-loops
 */
template <bool transpose>
void get_best_matching_dispatch(const kjb::Matrix& distance, double threshold, std::vector<std::pair<int, int> >& matching)
{
    size_t num_elements, other_num_elements;
    if(transpose)
    {
        num_elements = distance.get_num_cols();
        other_num_elements = distance.get_num_rows();
    }
    else
    {
        num_elements = distance.get_num_rows();
        other_num_elements = distance.get_num_cols();
    }

    if(num_elements > 1000)
    {
        // since we're just trying all permutations, there are N! permutations.
        // More than 6 tracks, this becomes really large
        KJB_THROW_2(Not_implemented, "Matchings for sets larger than 8 not supported.");
    }

    std::vector<int> indices(num_elements);
    // fill with 1, 2, 3, ...
    std::copy(boost::make_counting_iterator<int>(0), boost::make_counting_iterator<int>(num_elements), indices.begin());

#ifdef TEST
    // delete me after trying once
    {
        std::vector<int> test(indices);
        std::vector<int>::iterator begin = test.begin();
        std::vector<int>::iterator end = test.end();

        // did I actually construct this in the minimal lexicographical order?
        ASSERT(!std::prev_permutation(begin, end));
    }
#endif

    std::vector<int>::iterator begin = indices.begin();
    std::vector<int>::iterator end = indices.end();

    std::vector<int> best_matching(num_elements);
    double best_distance = DBL_MAX;
    do
    {
        double total_distance = 0;
        // compute distance of this permutation
        std::vector<int>::iterator it = begin;
        for(size_t other = 0; other < other_num_elements && it != end; ++other, ++it)
        {
            double dist;
            if(transpose)
                dist = distance(other, *it);
            else
                dist = distance(*it, other);
            total_distance += std::min(threshold, dist);
        }

        if(total_distance < best_distance)
        {
            best_distance = total_distance;
            std::copy(begin, end, best_matching.begin());
        }
    } while(std::next_permutation(begin, end));

    matching.clear();
    for(size_t other = 0; other < other_num_elements; other++)
    {
        // elements at end of best_matching have no match
        if(transpose)
        {
            if(distance(other, best_matching[other]) < threshold)
                matching.push_back(std::make_pair(other, best_matching[other]));
        }
        else
        {
            if(distance(best_matching[other], other) < threshold)
                matching.push_back(std::make_pair(best_matching[other], other));
        }
    }
}


void get_best_matching(const kjb::Matrix& pw_distance, double threshold, std::vector<std::pair<int, int> >& matching)
{
    if(pw_distance.get_num_rows() < pw_distance.get_num_cols())
    {
        get_best_matching_dispatch<true>(pw_distance, threshold, matching);
    }
    else
    {
        get_best_matching_dispatch<false>(pw_distance, threshold, matching);
    }
}


void get_pw_distance(const std::vector<const Vector3*>& pts1, const std::vector<const Vector3*>& pts2, kjb::Matrix& distance)
{
    if(pts1.size() == 0 || pts2.size() == 0)
    {
        distance.resize(0, 0);
        return;
    }

    distance.resize(pts1.size(), pts2.size());

    for(size_t i1 = 0; i1 < pts1.size(); ++i1)
    for(size_t i2 = 0; i2 < pts2.size(); ++i2)
    {
        distance(i1, i2) = vector_distance(*pts1[i1], *pts2[i2]);
    }
}

void init_correspondence(
        const pt::Trajectory_map& gt_track_map,
        const pt::Trajectory_map& test_track_map,
        double threshold,
        std::vector<Correspondence>& corrs)
        
{
    if(gt_track_map.size() == 0 || test_track_map.size() == 0)
    {
        // no correspondences
        return;
    }

    if(gt_track_map.duration() != test_track_map.duration())
    {
        KJB_THROW_2(Illegal_argument, "Ground truth and test tracks must have same duration.");
    }

    double sq_threshold = threshold * threshold;

    corrs.resize(gt_track_map.duration());
    typedef Correspondence::value_type Correspondence_pair;
    typedef Correspondence::left_const_reference Correspondence_map_element;

    // need to be able to refer to tracks by integer index.  Below will
    // let us map from index back to pt::Entity_id.
    
    std::vector<const Vector3*> gt_points, test_points;
    std::vector<pt::Entity_id> gt_ids, test_ids;
    kjb::Matrix pw_distance;

    for(size_t cur_frame = 0; cur_frame < gt_track_map.duration(); cur_frame++)
    {
        if(cur_frame > 0)
        {
            // get correspondences from previous frame
            // if valid, keep in this frame
            BOOST_FOREACH(Correspondence_map_element pair, corrs[cur_frame-1].left)
            {
                const pt::Entity_id& gt_id = pair.first;
                const pt::Entity_id& test_id = pair.second;
                
                typedef boost::optional<pt::Trajectory_element> Optional_element;
                // get current time slice for these tracks
                const Optional_element& gt_element = gt_track_map.at(gt_id)[cur_frame];
                const Optional_element& test_element = test_track_map.at(test_id)[cur_frame];

                // check if still valid in both tracks
                if(gt_element && test_element)
                {
                    // check if still within threshold
                    if( vector_distance_squared(gt_element->value.position, test_element->value.position) < sq_threshold)
                    {
                        corrs[cur_frame].left.insert(pair);
                    }
                }
            }
        }


        // extract unclaimed points for this time slice (ignores anything currently in correspondence)
        init_ids_and_points(gt_ids, gt_points, gt_track_map, cur_frame, corrs[cur_frame], true);
        init_ids_and_points(test_ids, test_points, test_track_map, cur_frame, corrs[cur_frame], false);

        // construct pairwise distance map
        get_pw_distance(gt_points, test_points, pw_distance);

        // find optimal matching
        std::vector<std::pair<int, int> > new_correspondences;
        get_best_matching(pw_distance, threshold, new_correspondences);

        // convert index correspondences to pt::Entity_id correspondences
        for(size_t i = 0; i < new_correspondences.size(); i++)
        {
            int gt_i = new_correspondences[i].first;
            int test_i = new_correspondences[i].second;
            
            ASSERT(pw_distance(gt_i, test_i) <= threshold);

            pt::Entity_id gt_obj = gt_ids[gt_i];
            pt::Entity_id test_obj = test_ids[test_i];

            Correspondence_pair pair(gt_obj, test_obj);

            corrs[cur_frame].insert(pair);
        }

    }
}


double get_distance(
        const pt::Trajectory_map& gt_track_map,
        const pt::Trajectory_map& test_track_map,
        double threshold,
        const std::vector<Correspondence>& corrs,
        size_t frame)
{
    typedef Correspondence::left_const_reference Correspondence_map_element;

    double total_distance = 0;

    BOOST_FOREACH(Correspondence_map_element pair, corrs[frame].left)
    {
        const pt::Entity_id& gt_id = pair.first;
        const pt::Entity_id& test_id = pair.second;
        
        typedef boost::optional<pt::Trajectory_element> Optional_element;
        // get current time slice for these tracks
        const Optional_element& gt_element = gt_track_map.at(gt_id)[frame];
        const Optional_element& test_element = test_track_map.at(test_id)[frame];

        ASSERT(gt_element);
        ASSERT(test_element);
        double distance = vector_distance(gt_element->value.position, test_element->value.position);
        ASSERT(distance <= threshold);
        total_distance += distance;
    }

    return total_distance;
}

void get_counts(
        const pt::Trajectory_map& gt_track,
        const pt::Trajectory_map& test_track,
        const std::vector<Correspondence >& corrs,
        size_t frame,
        size_t& mme_ct,
        size_t& fp_ct,
        size_t& miss_ct,
        size_t& match_ct,
        size_t& obj_ct)
{
    mme_ct = 0;
    fp_ct = 0; 
    miss_ct = 0;
    match_ct = 0;
    obj_ct = 0;

    // MISSES
    // (All ground truth points without matches in test track)
    BOOST_FOREACH(const Traj_pair& pair, gt_track)
    {
        const pt::Entity_id& gt_id = pair.first;
        const pt::Trajectory& gt_traj = pair.second;

        if(!gt_traj[frame]) continue;

        ++obj_ct;

        if(corrs[frame].left.count(gt_id) == 0)
            ++miss_ct; // miss (false negative) 
    }

    // FALSE POSITIVE
    // (All test track points without matches in ground truth)
    BOOST_FOREACH(const Traj_pair& pair, test_track)
    {
        const pt::Entity_id& test_id = pair.first;
        const pt::Trajectory& test_traj = pair.second;

        if(!test_traj[frame]) continue;
        if(corrs[frame].right.count(test_id) == 0)
            ++fp_ct; // false positive
    }

    // MISMATCH
    BOOST_FOREACH(Corr_pair pair, corrs[frame].left)
    {
        pt::Entity_id gt_id = pair.first;
        pt::Entity_id hyp_id = pair.second;

        // find next correspondence for this g.t. track
        for(size_t next_frame = frame+1; next_frame < corrs.size(); next_frame++)
        {
            if(corrs[next_frame].left.count(gt_id) > 0)
            {
                // if correspondence changed, count as mismatch
                if(corrs[next_frame].left.at(gt_id) != hyp_id)
                    ++mme_ct;
                break;
            }
        }
    }

    match_ct += corrs[frame].size();

}

} // namespace metrics


using namespace metrics;

Track_frame_metrics::Track_frame_metrics(
        const pt::Trajectory_map& gt_track,
        const pt::Trajectory_map& test_track,
        double threshold) :
    Base(gt_track.duration())
{
    std::vector<Correspondence> corrs;
    metrics::init_correspondence(
        gt_track,
        test_track,
        threshold,
        corrs);

    init_distance_(
        gt_track,
        test_track,
        threshold,
        corrs);

    init_counts_(
        gt_track,
        test_track,
        corrs);
}


void Track_frame_metrics::init_distance_(
        const pt::Trajectory_map& gt_track_map,
        const pt::Trajectory_map& test_track_map,
        double threshold,
        const std::vector<Correspondence>& corrs
)
{
    for(size_t frame = 0; frame < corrs.size(); frame++)
    {
        Track_metrics& current = (*this)[frame];

        current.total_distance_ = metrics::get_distance(gt_track_map, test_track_map, threshold, corrs, frame);
    }
}

void Track_frame_metrics::init_counts_(
        const pt::Trajectory_map& gt_track,
        const pt::Trajectory_map& test_track,
        const std::vector<Correspondence >& corrs)
{
    for(size_t frame = 0; frame < corrs.size(); frame++)
    {
        Track_metrics& current = (*this)[frame];

        get_counts(
                gt_track,
                test_track,
                corrs,
                frame,
                current.mme_ct_,
                current.fp_ct_,
                current.miss_ct_,
                current.match_ct_,
                current.obj_ct_);
    }
}

Track_metrics::Track_metrics(
    const pt::Trajectory_map& gt_track,
    const pt::Trajectory_map& test_track,
    double threshold) :
        threshold_(threshold),
        total_distance_(0),
        mme_ct_(0),
        fp_ct_(0),
        miss_ct_(0),
        match_ct_(0),
        obj_ct_(0),
        mt_(0.0),
        ml_(0.0),
        frag_(0),
        ids_(0),
        total_num_gt_tracks_(gt_track.size())
{
    std::vector<Correspondence> corrs;

    init_correspondence(
        gt_track,
        test_track,
        threshold,
        corrs);

    init_distance_(
        gt_track,
        test_track,
        threshold,
        corrs);

    init_counts_(
        gt_track,
        test_track,
        corrs);
}

Track_metrics::Track_metrics(
    const pt::Trajectory_map& gt_track,
    const pt::Trajectory_map& test_track,
    double threshold,
    const std::vector<Correspondence>& corrs) :
        threshold_(threshold),
        total_distance_(0),
        mme_ct_(0),
        fp_ct_(0),
        miss_ct_(0),
        match_ct_(0),
        obj_ct_(0),
        mt_(0.0),
        ml_(0.0),
        frag_(0),
        ids_(0),
        total_num_gt_tracks_(gt_track.size())
{
    init_distance_(
        gt_track,
        test_track,
        threshold,
        corrs);

    init_counts_(
        gt_track,
        test_track,
        corrs);
}





void Track_metrics::init_distance_(
        const pt::Trajectory_map& gt_track_map,
        const pt::Trajectory_map& test_track_map,
        double threshold,
        const std::vector<Correspondence>& corrs
)
{
    total_distance_ = 0;
    for(size_t cur_frame = 0; cur_frame < corrs.size(); cur_frame++)
    {
        total_distance_ += metrics::get_distance(gt_track_map, test_track_map, threshold, corrs, cur_frame);
    }
}



void Track_metrics::init_counts_(
        const pt::Trajectory_map& gt_track,
        const pt::Trajectory_map& test_track,
        const std::vector<Correspondence >& corrs)
{
    mme_ct_ = 0;
    fp_ct_ = 0; 
    miss_ct_ = 0;
    match_ct_ = 0;
    obj_ct_ = 0;

    size_t cur_mme_ct;
    size_t cur_fp_ct; 
    size_t cur_miss_ct;
    size_t cur_match_ct;
    size_t cur_obj_ct;

    for(size_t frame = 0; frame < corrs.size(); frame++)
    {
        get_counts(
                gt_track,
                test_track,
                corrs,
                frame,
                cur_mme_ct,
                cur_fp_ct,
                cur_miss_ct,
                cur_match_ct,
                cur_obj_ct);

        mme_ct_ += cur_mme_ct;
        fp_ct_ += cur_fp_ct; 
        miss_ct_ += cur_miss_ct;
        match_ct_ += cur_match_ct;
        obj_ct_ += cur_obj_ct;
    }

    Correspondence::right_const_iterator t_cur_iter;
    Correspondence::right_const_iterator t_next_iter;
    // Compute the covered gt_track percentage
    size_t mt_counts = 0;
    size_t ml_counts = 0;
    BOOST_FOREACH(const Traj_pair& pair, gt_track)
    {
        size_t gt_track_len = 0; 
        size_t gt_track_covered_len = 0;
        for(size_t frame = 0; frame < corrs.size(); frame++)
        {
            const pt::Entity_id& gt_id = pair.first;
            const pt::Trajectory& gt_traj = pair.second;

            if(!gt_traj[frame]) continue;
            ++gt_track_len;

            if(corrs[frame].left.count(gt_id) == 1)
                ++gt_track_covered_len;

            // Check the fragments 
            if(frame == corrs.size() - 1) continue; 

            t_cur_iter = corrs[frame].right.find(gt_id); 
            t_next_iter = corrs[frame+1].right.find(gt_id);

            if(t_cur_iter != corrs[frame].right.end() &&
               t_next_iter != corrs[frame+1].right.end() &&
               t_cur_iter->second != t_next_iter->second)
            {
               ++frag_;
            }
        }
        double p = gt_track_covered_len/ static_cast<double>(gt_track_len);
             
        if(p > 0.8)
            ++mt_counts;
        if(p < 0.2) 
            ++ml_counts;
    }
    mt_ = mt_counts/static_cast<double>(gt_track.size());
    ml_ = ml_counts/static_cast<double>(gt_track.size());

    // Compute the id switching
    Correspondence::left_const_iterator g_cur_iter;
    Correspondence::left_const_iterator g_next_iter;

    BOOST_FOREACH(const Traj_pair& pair, test_track)
    {
        for(size_t frame = 0; frame < corrs.size()-1; frame++)
        {
            const pt::Entity_id& track_id = pair.first;
            const pt::Trajectory& track_traj = pair.second;
            if(!track_traj[frame]) continue;
            g_cur_iter = corrs[frame].left.find(track_id); 
            g_next_iter = corrs[frame+1].left.find(track_id);
            if(g_cur_iter != corrs[frame].left.end() &&
               g_next_iter != corrs[frame+1].left.end() &&
               g_cur_iter->second != g_next_iter->second)
            {
               ++ids_;
            }
        }
    }
}

} // namespace psi
} // namespace kjb
