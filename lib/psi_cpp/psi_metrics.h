/* $Id: psi_metrics.h 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector_d.h"
#include "people_tracking_cpp/pt_entity.h"
#include "people_tracking_cpp/pt_complete_trajectory.h"

#include <boost/bimap.hpp>
#include <boost/foreach.hpp>
#include <boost/iterator/counting_iterator.hpp>

#include <utility>
#include <vector>
#include <algorithm>

#ifndef PSI_TRACKING_METRICS_H
#define PSI_TRACKING_METRICS_H
namespace kjb
{
namespace psi
{

namespace metrics
{
    typedef boost::bimap<pt::Entity_id, pt::Entity_id> Correspondence;
    void get_best_matching(
            const kjb::Matrix& pw_distance,
            double threshold,
            std::vector<std::pair<int, int> >& matching);

    void get_pw_distance(
            const std::vector<const Vector3*>& pts1,
            const std::vector<const Vector3*>& pts2,
            kjb::Matrix& distance);

    void init_correspondence(
            const pt::Trajectory_map& gt_track_map,
            const pt::Trajectory_map& test_track_map,
            double threshold,
            std::vector<Correspondence>& corrs);
}


class Track_metrics; // forward declaration

class Track_frame_metrics : public std::vector<Track_metrics>
{
typedef std::vector<Track_metrics> Base;
public:
typedef boost::bimap<pt::Entity_id, pt::Entity_id> Correspondence;
    Track_frame_metrics(
            const pt::Trajectory_map& gt_track,
            const pt::Trajectory_map& test_track,
            double threshold);

protected:
    void init_distance_(
        const pt::Trajectory_map& gt_track_map,
        const pt::Trajectory_map& test_track_map,
        double threshold,
        const std::vector<Correspondence>& corrs
    );

    void init_counts_(
            const pt::Trajectory_map& gt_track,
            const pt::Trajectory_map& test_track,
            const std::vector<Correspondence >& corrs);
};

class Track_metrics
{
public:
typedef boost::bimap<pt::Entity_id, pt::Entity_id> Correspondence;

public:

    Track_metrics(double threshold = 1.0) :
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
        total_num_gt_tracks_(0)
    {}

    Track_metrics(
        const pt::Trajectory_map& gt_track,
        const pt::Trajectory_map& test_track,
        double threshold);

    Track_metrics(
        const pt::Trajectory_map& gt_track,
        const pt::Trajectory_map& test_track,
        double threshold,
        const std::vector<Correspondence>& corrs);

    double motp() const
    {
        if(match_ct_ == 0)
        {
            // can't have distance without matches
            ASSERT(total_distance_ == 0);

            if(obj_ct_ == 0)
            {
                // no ground truth tracks, so it's correct to have no matches
                return 0.0;
            }
            else
            {
                // Utter failure.
                // Threshold is the ceiling for this metric, so return that
                return threshold_;
            }
        }

        return total_distance_ / match_ct_;
    }

    double mota() const
    {
        double error_ratio;
        if(obj_count() == 0 && error_count() == 0)
        {
            // This happens if there are no ground truth tracks.
            error_ratio = 0.0;
        }
        else
        {
            error_ratio = (error_count()) / static_cast<double>(obj_count());
        }
        return 1.0 - error_ratio;
    }

    // Mostly tracked 
    double mt() const
    {
        return mt_;
    }

    // Mostly lost
    double ml() const 
    {
        return ml_;
    }

    // Partially tracked
    double pt() const 
    {
        return 1.0 - mt_ - ml_; 
    }

    // Fragments: the total number of times that a ground truth
    // trajectory is interrupted
    size_t frag() const
    {
        return frag_;
    }

    // ID switchs: the total number of times that a tracked trajectory 
    // changes its matched GT identity
    size_t ids() const
    {
        return ids_;
    }

    // Return the total number of gt tracks
    size_t total_gt_tracks() const
    {
        return total_num_gt_tracks_;
    }

//
//    const std::vector<Correspondence>& get_correspondence() const
//    {
//        return corrs_;
//    }

    size_t obj_count() const
    {
        return obj_ct_;
    }

    size_t error_count() const
    {
        return fp_ct_ + miss_ct_ + mme_ct_;
    }

    double total_distance() const
    {
        return total_distance_;
    }

    size_t match_count() const
    {
        return match_ct_;
    }

protected:
    void init_distance_(
            const pt::Trajectory_map& gt_track_map,
            const pt::Trajectory_map& test_track_map,
            double threshold,
            const std::vector<Correspondence >& corrs);

    void init_counts_(
            const pt::Trajectory_map& gt_track,
            const pt::Trajectory_map& test_track,
            const std::vector<Correspondence >& corrs);

private:
    double threshold_;
    double total_distance_;
    size_t mme_ct_;
    size_t fp_ct_;
    size_t miss_ct_;
    size_t match_ct_;
    size_t obj_ct_;
    double mt_;
    double ml_;
    size_t frag_;
    size_t ids_;
    size_t total_num_gt_tracks_;

friend class Track_frame_metrics;
};

} // namespace psi
} // namespace kjb 

#endif
