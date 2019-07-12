/* $Id: psi_likelihood.cpp 21596 2017-07-30 23:33:36Z kobus $ */
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
#include "psi_cpp/psi_likelihood.h"
#include "people_tracking_cpp/pt_entity.h"

#include <vector>
#include <utility> /* for std::pair */
#include <algorithm>

#ifdef KJB_HAVE_UA_CARTWHEEL
#include <Control/CapsuleState.h>
#include <Control/PosState.h>
#include <Control/BoxState.h>
#endif

#include <l_cpp/l_exception.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_pdf.h>
#include <prob_cpp/prob_util.h>

#include <boost/foreach.hpp>

#include <psi_cpp/psi_bbox.h>
#include <psi_cpp/psi_util.h>
#include <psi_cpp/psi_action.h>

namespace kjb
{
namespace psi
{

/// Compute log(n!)
double lfactorial(int n)
{
    if(n < 0) KJB_THROW_2(kjb::Illegal_argument, "n must be a non-negative integer.");
    if(n == 0 || n == 1) return 0;

    double result;
    for(int i = 2; i <= n; i++)
    {
        result += log(i);
    }

    return result;
}

/**
 * Get the depth of a point from the camera.
 *
 * @param pt Point in world coordinates
 * @param cam Camera representing the viewpoint
 */
inline double get_depth(const Vector& pt, const Perspective_camera& cam)
{
    // point in camera coordinates
    Vector cam_pt;
    
    // rotate into camera axes
    cam_pt = cam.get_orientation().rotate(pt);
    cam_pt += cam.get_world_origin();

    // camera points down -z axis, so decreasing z-values mean increasing depth values;
    const double depth = -cam_pt[2];

    return depth;
}

class pair_compare_first
{
public:
    template <class S, class T>
    bool operator()(const std::pair<S, T>& op1, const std::pair<S, T>& op2)
    {
        return op1.first < op2.first;
    }
};

class get_pair_second
{
public:
    template <class S, class T>
    const T& operator()(const std::pair<S, T>& op1)
    {
        return op1.second;
    }
};

inline std::vector<Bbox> sort_by_depth(std::vector<std::pair<double, Bbox> >& boxes)
{
    // sort pairs by depth
    std::sort(
            boxes.begin(),
            boxes.end(),
            pair_compare_first());

    std::vector<Bbox> result(boxes.size());
    transform(boxes.begin(), boxes.end(), result.begin(), get_pair_second());

    return result;

}



double Bbox_pairwise_likelihood::operator()(const Bbox& data, const Bbox& model) const
{
    using kjb::pdf;

    double log_prob = 0;

    // compare two bounding boxes
    double prob = pdf(xy_prob_, data.get_centre()[0] - model.get_centre()[0]);
    if(prob <= MIN_LOG_ARG)
    {
        log_prob += log(MIN_LOG_ARG);
    }
    else
    {
        log_prob += log(prob);
    }
    prob = pdf(xy_prob_, data.get_centre()[1] - model.get_centre()[1]);
    if(prob <= MIN_LOG_ARG)
    {
        log_prob += log(MIN_LOG_ARG);
    }
    else
    {
        log_prob += log(prob);
    }
    prob = pdf(width_prob_, data.get_width() - model.get_width());
    if(prob <= MIN_LOG_ARG)
    {
        log_prob += log(MIN_LOG_ARG);
    }
    else
    {
        log_prob += log(prob);
    }
    prob = pdf(height_prob_, data.get_height() - model.get_height());
    if(prob <= MIN_LOG_ARG)
    {
        log_prob += log(MIN_LOG_ARG);
    }
    else
    {
        log_prob += log(prob);
    }
    /*log_prob += std::log(pdf(xy_prob_, data.get_centre()[0] - model.get_centre()[0]));
    log_prob += std::log(pdf(xy_prob_, data.get_centre()[1] - model.get_centre()[1]));
    log_prob += std::log(pdf(width_prob_, data.get_width() - model.get_width()));
    log_prob += std::log(pdf(height_prob_, data.get_height() - model.get_height()));*/

    return log_prob;
}


double Bbox_noise_likelihood::operator()(const Bbox& data) const
{
    using kjb::pdf;

    double log_prob = 0;

    // compare two bounding boxes
    log_prob += std::log(pdf(x_prob_, data.get_centre()[0]));
    log_prob += std::log(pdf(y_prob_, data.get_centre()[1]));
    log_prob += std::log(pdf(width_prob_, data.get_width()));
    log_prob += std::log(pdf(height_prob_, data.get_height()));

    return log_prob;
}




#ifdef KJB_HAVE_UA_CARTWHEEL
double Cartwheel_likelihood::operator()(const Model& m) const
{
    using namespace CartWheel;

    double video_likelihood = 0;

    // run the simulation with these actions
    interface_->simulate(to_cw_start_state(m), to_cw_actions(m));

    // get back bounding capsules of the simulated actors.
    std::vector<CartWheel::CapsuleState*> capsule_states = interface_->getCapsules();

    // get blocks from simulator
    const std::vector<PosState*>& pos_states = interface_->getPositions();
    std::vector<std::vector<BoxStatePtr> > block_states(pos_states.size());
    for(size_t i = 0; i < block_states.size(); i++)
    {
        block_states[i] = pos_states[i]->getBoxStates();
    }


    size_t num_frames = data_.size();
    size_t num_capsule_frames = capsule_states.size();
//    ASSERT(num_capsule_frames == num_frames); // this will fail

    // TODO: handle mismatched frame counts better
    // (a) design a reasonable generative model
    // (b) re-parameterize the model to ensure constant frame length
    //     (i.e. sample over transition time)
    video_likelihood += log(kjb::pdf(frame_count_penalty_, fabs((double) num_capsule_frames - (double) num_frames)));


    // had some problems here, before, but hopefully resolved now
    ASSERT(video_likelihood < DBL_MAX);
    ASSERT(video_likelihood > -DBL_MAX);


    num_frames = std::min(num_frames, num_capsule_frames);

    // get likelihoods for individual frames
    double avg_frame_likelihood = 0;
    for(size_t i = 0; i < num_frames; i++)
    {
        //  Convert capsules into bounding boxes, sorted by depth,
        std::vector<std::pair<double, Bbox> > model_boxes = capsules_to_boxes(*capsule_states[i]);
        std::vector<std::pair<double, Bbox> > block_model_boxes = blocks_to_boxes(block_states[i]);

        // combine all boxes into a single vector
        model_boxes.insert(model_boxes.end(), block_model_boxes.begin(), block_model_boxes.end());

        // order boxes by their camera depth
        std::vector<Bbox> sorted_boxes = sort_by_depth(model_boxes);

        // compare sets of boxes
        avg_frame_likelihood += frame_likelihood_(data_[i], sorted_boxes);
    }
    avg_frame_likelihood /= num_frames;
    video_likelihood += avg_frame_likelihood;

    return video_likelihood;
}

std::vector<std::pair<double, Bbox> > Cartwheel_likelihood::blocks_to_boxes(const std::vector<CartWheel::BoxStatePtr>& blocks) const
{
    using std::pair;
    using std::vector;

    std::vector<std::pair<double, Bbox> > tmp_boxes;

    for(size_t i = 0; i < blocks.size(); i++)
    {
        Bbox box = get_bounding_box(to_cuboid(*blocks[i]), cam_);

        // get depth
        double depth = get_depth(to_kjb(blocks[i]->getPosition()), cam_);

        // store
        if(depth > 0)
        {
            tmp_boxes.push_back(std::make_pair(depth, box));
        }
        else
        {
            ; // do nothing, behind camera
        }

    }

    return tmp_boxes;
}

///  Take a set of capsules for N humans and convert them into N into bounding boxes, sorted descended by depth from camera
std::vector<std::pair<double, Bbox> > Cartwheel_likelihood::capsules_to_boxes(const CartWheel::CapsuleState& capsule_state) const
{
    int num_actors = capsule_state.getNumEntities();
    std::vector<std::vector<CartWheel::Math::Capsule*> > actor_capsules(num_actors);

    for(int i = 0; i < num_actors; i++)
    {
        actor_capsules[i] = capsule_state.getCapsules(i);
    }

    return capsules_to_boxes(actor_capsules);
}

std::vector<std::pair<double, Bbox> > Cartwheel_likelihood::capsules_to_boxes(const std::vector<std::vector<CartWheel::Math::Capsule*> >& actors_capsules) const
{
    using std::vector;
    using std::pair;

    std::vector<double> model_depths;

//    int num_actors = capsule_state.getNumEntities();
    int num_actors = actors_capsules.size();

    model_depths.resize(num_actors);

    std::vector<std::pair<double, Bbox> > tmp_boxes;
    for(int actor = 0; actor < num_actors; actor++)
    {
        using CartWheel::Math::Capsule;

        // get actor's body in this frame
        // TODO: would like this to receive by const reference
        const std::vector<Capsule*>& body_capsules = actors_capsules[actor];

        // convert to 2d bounding box
        Bbox bbox = get_bounding_box(body_capsules, cam_);

        // get body centroid
        // TODO get this from PosState
        kjb::Vector centroid(3, 0.0);
        for(size_t i = 0; i < body_capsules.size(); i++)
        {
            kjb::Vector p1(3);
            p1[0] = body_capsules[i]->p1.getX();
            p1[1] = body_capsules[i]->p1.getY();
            p1[2] = body_capsules[i]->p1.getZ();

            kjb::Vector p2(3);
            p2[0] = body_capsules[i]->p2.getX();
            p2[1] = body_capsules[i]->p2.getY();
            p2[2] = body_capsules[i]->p2.getZ();

            centroid += p1;
            centroid += p2;
        }
        centroid /= (body_capsules.size() * 2);
        
        const double depth = get_depth(centroid, cam_);

        if(depth > 0)
        {
            // store depth and bounding box together, so we can sort one by the other
            tmp_boxes.push_back(std::make_pair(depth, bbox));
        }
        else
        {
            ; // do nothing; we ignore boxes behind the camera
        }
    }

    return tmp_boxes;
}
#endif

/// @pre all_boxes is sorted descending by depth
double Frame_likelihood::compute_visibility(const std::vector<Bbox>& all_boxes, size_t box_i) const
{
    // instead of computing the exact size/shape of the occlusion (which is at least O(n^2 log n), I beleive),
    // we take subdivide the box of interest into an 8x8 grid of cells, and use a crude method to test if a cell is occluded.  The result is O(n) where n is the number of occluding boxes.

    const size_t NUM_SUBDIVISIONS = 8; 
    const size_t NUM_CELLS = NUM_SUBDIVISIONS * NUM_SUBDIVISIONS; 

    ASSERT(box_i < all_boxes.size());

    const Bbox& main_box = all_boxes[box_i];
    // subdivide box into cell centers
    double x_delta = main_box.get_width() / NUM_SUBDIVISIONS;
    double y_delta = main_box.get_height() / NUM_SUBDIVISIONS;

    double x_min = main_box.get_left() + x_delta / 2.0;
    double y_min = main_box.get_bottom() + y_delta / 2.0;

    std::vector<kjb::Vector> cell_centers;
    cell_centers.reserve(NUM_CELLS);

    std::vector<bool> occluded;
    cell_centers.reserve(NUM_CELLS);

    size_t num_clipped = 0;

    // make vector of cell centers, which we'll test for occlusion
    for(size_t x_i = 0; x_i < NUM_SUBDIVISIONS; x_i++)
    for(size_t y_i = 0; y_i < NUM_SUBDIVISIONS; y_i++)
    {
        const double x = x_min + x_i * x_delta;
        const double y = y_min + y_i * y_delta;
        const kjb::Vector cell_center(x,y);

        if(frame_bounds_.contains(cell_center))
        {
            cell_centers.push_back(kjb::Vector(x, y));
            occluded.push_back(false);
        }
        else
        {
            // Don't bother representing a cell if it's clipped.
            // This will save use having to evaluate it's occlusion later on.
            num_clipped++;
        }

    }

    // iterate over boxes in front of this one
    for(size_t i = 0; i < box_i; i++)
    {
        const Bbox& cur_box = all_boxes[i];
        // if no intersection, skip it
        if(!main_box.intersects(cur_box)) continue;

        // test each cell center for occlusion
        for(size_t cell = 0; cell < cell_centers.size(); cell++)
        {
            // (we could use a binary search over box's area
            // here to avoid exhaustive search; but that is 
            // probably overkill unless NUM_SUBDIVISIONS gets much larger)
            if(cur_box.contains(cell_centers[cell]))
            {
                occluded[cell] = true;
            }
        }
    }

    size_t num_occluded = count(occluded.begin(), occluded.end(), true);
    return 1.0 - ((double) num_clipped + num_occluded) / NUM_CELLS;
}


/**
 * @param affinities The affinity matrix returned by get_affinities(). 
 * @param correspondences The correpondence map returned by get_correspondences_greedily()
 *
 * @pre model_boxes are sorted descending by depth
 */
double Frame_likelihood::operator()(
    const std::vector<Bbox>& data_boxes,
    const std::vector<Bbox>& model_boxes,
    const kjb::Matrix& affinities,
    const std::vector<size_t>& correspondences,
    const std::vector<double>& m_p_missing) const
{

    // frame likelihood is given by:
    //
    // p(d | m) = sum_{c, n, h} p(d | m, c) p(c | n, h) p(h | m)
    // where d is the set of data boxes for this frame
    // m is the set of model boxes for this frame
    // c is the correspondence between data and model boxes (and noise)
    // h is a set of |m| binary variables indicating whether a model is hidden
    // n is a set of |d| binary variables indicating whether a data point is noise.
    //
    // Since this sum over correspondence is intractible, we approximate it by importance sampling.  Our "proposal function" only ever proposes one correspondence with probability 1.  This correspondence is found by greedy matching, in an attempt to find the best possible correspondence.  The result is:
    //
    // p(d | m) ~ 1/N sum_{c*, n*, h*} p(d | m, c*) p(c* | n*, h*) p(h* | m) / h(c*, n*, h*)
    //
    // where c* n* and h* are samples from the proposal distribution h(c, n, h), and N is the number of samples.  Since h() only generates a single result, and since that result approximates argmax{p(d | m)}, we can rewrite it as:
    //
    // p(d | m) ~ max_{c, n, h} { p(d | m, c) p(c | n, h) p(h | m) }
    //
    // This approximation will be exact under the assumption that all sub-optimal correspondences have zero likelihood.  This obviously isn't true, but it's hopefully not far off from the truth.  An alternative is to sample over correspondences using MCMC, which we will probably try in the near future.
    

    // get_affinity() uses the last index as a noise index
    size_t NOISE_INDEX = model_boxes.size();


// STEP 1: Handle unmatched model boxes by finding the probability that they are hidden.
//
// This is 
//      \prod_{j \in M \\ c} p(h_j = 1 | m_j)
// where M is the set of all model boxes, c is the image of the coorespondence function c().  In other words, a product over all
// unmatched model boxes, the probability that they were undetected.
    std::vector<bool> missing(model_boxes.size(), true);
    size_t num_noise = 0;
    for(size_t i = 0; i < correspondences.size(); i++)
    {
        // don't care about counting noise
        if(correspondences[i] == NOISE_INDEX)
        {
            num_noise++;
        }
        else
        {
            missing.at(correspondences[i]) = false;
        }
    }

    double missing_prob = 0;
    for(size_t i = 0; i < missing.size(); i++)
    {
    // compute prior over missing
    //      using occlusion.  Eventually would like to incorporate size into this, too
    //
    // This is computing
    //     p(h_i = 1 | m_i, p(m_i));
    // where h_i is the "Hidden" indicator variable
    // and p(m_i) are the model boxes in front of m_i.
    //
    // In other words,the probability that the i-th model box
    // was undetected, taking occlusion into account.
        if(missing[i])
        {
            missing_prob += m_p_missing[i];
        }
    }

// STEP 2: Compute p(c | n, h), the prior over correspondences, given n noise boxes and h hidden boxes.  Note that h = |D| - n, so we only pass n here.
    double correspondence_prob = p_correspondence(data_boxes.size(), num_noise);

// STEP 3: Compute p(d | c, m), the data likelihood given the correspondences
    double likelihood = 0;
    for(size_t data_i = 0; data_i < data_boxes.size(); data_i++)
    {
        size_t model_i = correspondences[data_i];

        // this term was already computed as the affinity value.
        likelihood += affinities(data_i, model_i);
    }

//    std::cout << likelihood << "\t" << correspondence_prob << "\t" << missing_prob << std::endl;
    // return p(d_i | m_i, h_i, c_i) p(c_i | h_i, m_i) p(h_i | m_i)
    return likelihood + correspondence_prob + missing_prob;
}



/// @pre the correspondence is already "valid", i.e. one-to-one plus a noies bucket
double Frame_likelihood::p_correspondence(size_t num_data, size_t num_noise) const
{
    // p(c | n, h)
    // where n is the set of binary "noise" variables for each data
    // point, and h is the set of binary "hidden" variable for each model point.
    //
    // Uniform distribution over "valid" correspondences.
    // Valid means: |{n : n = 0}| == |{h : h = 0}|
    //    in other words, the number of non-noise data boxesequals the number of 
    //    non-hidden model boxes.
    //
    //  p(c | n, h) = 1/N!, where N = |D| - |n_1|, and |n_1| is the number of data points modelled as noise (i.e. n = 1).
    //
    size_t N = num_data - num_noise;
    return -lfactorial(N); // log(N!)
}

std::vector<double> Frame_likelihood::get_p_missing( const std::vector<Bbox>& model) const
{
    // compute visibility for every model
    std::vector<double> m_p_missing(model.size());
    for(size_t m_i = 0; m_i < model.size(); m_i++)
    {
        double visibility = compute_visibility(model, m_i);
        m_p_missing[m_i] = p_missing(visibility);
    }

    return m_p_missing;
}

/**
 * @pre model is sorted descending by depth
 *
 * @param data Data boudning boxes
 * @param model Bounding boxes generated by the model
 * @param m_p_missing Probability of each model being undetected (based on occlusion/visibility)
 */
kjb::Matrix Frame_likelihood::get_affinity(
        const std::vector<Bbox>& data,
        const std::vector<Bbox>& model,
        const std::vector<double>& m_p_missing
        ) const
{
    // return: for every data point, return corresponding model
    

    // use the last model index to represent the noise model
    size_t NOISE_INDEX = model.size();


    kjb::Matrix affinities(data.size(), model.size()+1);

    for(size_t d_i = 0; d_i < data.size(); d_i++)
    {
        double noise_affinity = 0;
        // don't forget the affinity for "noise"!:
        noise_affinity += noise_likelihood_(data[d_i]);
        noise_affinity += log(p_noise_);
        // TODO: remove the "noise" column from the affinity matrix, since
        // we're marginalizing it below.
        affinities(d_i, NOISE_INDEX) = noise_affinity;

        for(size_t m_i = 0; m_i < model.size(); m_i++)
        {
            double pair_affinity = 0;
            pair_affinity += pair_likelihood_(data[d_i], model[m_i]);
            pair_affinity += log(1-p_noise_); // not noise
            pair_affinity += log(1-m_p_missing[m_i]); // not missing

            if(marginalize_out_noise_)
            {
                // mixture model: uniform-ish noise model + peaked pairwise model.
                pair_affinity = log_sum(pair_affinity, noise_affinity);
            }
            affinities(d_i, m_i) = pair_affinity;
        }
    }

    return affinities;
}

// given an affinity matrix with rows indexing set A and columns indexing set B, 
// greedilly find a correspondence between A and B that attempts to
// maximize the sum of affinities.  This makes no guarantee of
// optimality, but shouldn't do TOO bad...
std::vector<size_t> Frame_likelihood::get_correspondence_greedily( const kjb::Matrix& affinities) const
{
    using namespace std;

    const size_t num_models = affinities.get_num_cols();
    const size_t num_data = affinities.get_num_rows();

    // the last index of affinities is used for the noise model
    const size_t NOISE_INDEX = num_models-1;
//    ASSERT(num_data == data_.size());

    vector<size_t> result(num_data);

    // keep track of which nodes have been removed from the sets
    vector<bool> d_matched(num_data, false);
    vector<bool> m_matched(num_models, false);

    // Find pair with highest affinity, store, repeat.  
    // This is expensive -- O(m * n^2) -- but we're hacking anyway,
    // so we'll tolerate it.  By the time the number of pairs gets
    // high enough to care about this, we won't be using this approach
    // anyway, instead we'll sample over correspondences.
    for(size_t i = 0; i < num_data; i++)
    {
        double best = -DBL_MAX;
        std::pair<size_t,size_t> best_pair;

        for(size_t d_i = 0; d_i < num_data; d_i++)
        {
            if(d_matched[d_i]) continue;

            for(size_t m_i = 0; m_i < num_models; m_i++)
            {
                if(m_matched[m_i]) continue;

                double cur = affinities(d_i, m_i);

                if(best < cur)
                {
                    best = cur;
                    best_pair = make_pair(d_i, m_i);
                }
            }

            // check against noise model
            double cur = affinities(d_i, NOISE_INDEX);
            if(best < cur)
            {
                best = cur;
                best_pair = make_pair(d_i, NOISE_INDEX);
            }
        }

        size_t best_d = best_pair.first;
        size_t best_m = best_pair.second;

        d_matched[best_d] = true;

        if(best_m != NOISE_INDEX)
            m_matched[best_m] = true;

        result[best_d] = best_m;
    }

    return result;
}




//---------------------------------------------------

double Cylinder_world_likelihood::operator()(const Model& m) const
{
    double video_likelihood = 0;

    // run the simulation with these actions
    simulator_.simulate(to_start_state(m), to_actions(m));

    // get back states of the simulated actors.
    std::vector<std::vector<std::vector<Entity_state> > > entities_frames = simulator_.get_entity_states();
//    std::vector<std::vector<Cuboid> > blocks_frames = simulator_.get_box_states();


    size_t num_entity_frames = 0;
    
    // get number of frames from simulation length of the first simulated object
    for(int type = 0; type < pt::NUM_ENTITY_TYPES; type++)
    {
        if(entities_frames[type].size() > 0)
        {
            num_entity_frames = entities_frames[type][0].size();
            ASSERT(num_entity_frames > 0);
            break;
        }
    }
    // required: at least one object was simulated.
    ASSERT(num_entity_frames > 0);



    // TODO: handle mismatched frame counts better
    // (a) design a reasonable generative model
    // (b) re-parameterize the model to ensure constant frame length
    //     (i.e. sample over transition time)
    video_likelihood += log(kjb::pdf(frame_count_penalty_, fabs((double) num_entity_frames - (double) num_frames_)));

    // had some problems here, before, but hopefully resolved now
    ASSERT(video_likelihood < DBL_MAX);
    ASSERT(video_likelihood > -DBL_MAX);


    // TODO: rework this.  we should always evaluate over all frames
    size_t num_evaluation_frames = std::min(num_frames_, num_entity_frames);

    // get likelihoods for individual frames
    double avg_frame_likelihood = 0;
    for(size_t frame = 0; frame < num_evaluation_frames; frame++)
    {
        // get all entities for this frame
        std::vector<std::vector<Entity_state> > entities(pt::NUM_ENTITY_TYPES);
        for(size_t type = 0; type < pt::NUM_ENTITY_TYPES; type++)
        {
            const size_t num_entities = entities_frames[type].size();
            entities[type].resize(num_entities);
            for(size_t entity = 0; entity < num_entities; entity++)
            {
                entities[type][entity] = entities_frames[type][entity][frame];
            }
        }

        //  Convert entities and blocks into bounding boxes with depths
        std::vector<std::pair<double, Bbox> > model_boxes = entities_to_boxes(entities);

        // sort by camera depth
        std::vector<Bbox> sorted_boxes = sort_by_depth(model_boxes);

        // merge per-type data into one big frame data
        // TODO: handle box-matching individually, but handle occlusion together
        // with the current method, people could get confused with bikes, etc.
        std::vector<Bbox> data_boxes;
        for(size_t type = 0; type < pt::NUM_ENTITY_TYPES; type++)
        {
            // ignore unrepresented object types
            if((*data_)[type].size() == 0) continue;

            data_boxes.insert(data_boxes.end(), (*data_)[type][frame].begin(), (*data_)[type][frame].end());
        }


        // compare sets of boxes
        avg_frame_likelihood += frame_likelihood_(data_boxes, sorted_boxes);
    }
//    avg_frame_likelihood /= num_frames;
    video_likelihood += avg_frame_likelihood;


    return video_likelihood;
}

///  Take a set of entity states for N humans and convert them into N into bounding boxes, sorted descended by depth from camera
std::vector<std::pair<double, Bbox> > Cylinder_world_likelihood::entities_to_boxes(const std::vector<std::vector<Entity_state> >& entities) const
{
    using std::vector;
    using std::pair;

    std::vector<std::pair<double, Bbox> > tmp_boxes;

    for(size_t type = 0; type < entities.size(); type++)
    {
        for(size_t i = 0; i < entities[type].size(); i++)
        {

            // convert to 2d bounding box
            Bbox bbox = get_bounding_box(entities[type][i], cam_);

            // get body centroid
            kjb::Vector centroid(3);
            centroid[0] = entities[type][i].get_position()[0];
            centroid[1] = entities[type][i].get_height()/2.0;
            centroid[2] = entities[type][i].get_position()[1];

            const double depth = get_depth(centroid, cam_);


            if(depth > 0)
            {
                // store depth and bounding box together, so we can sort one by the other
                tmp_boxes.push_back(std::make_pair(depth, bbox));
            }
            else
            {
                ; // do nothing; we ignore boxes behind the camera
            }
        }
    }

    return tmp_boxes;

}

} // namespace psi
} // namespace kjb
