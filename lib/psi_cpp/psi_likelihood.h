/* $Id: psi_likelihood.h 21596 2017-07-30 23:33:36Z kobus $ */
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

#ifndef PSI_LIKELIHOOD
#define PSI_LIKELIHOOD


#include "l/l_sys_debug.h"  /* For ASSERT */
#include "m_cpp/m_matrix.h"
#include "camera_cpp/perspective_camera.h"
#include "prob_cpp/prob_distribution.h"
#include "prob_cpp/prob_pdf.h"

#include "psi_cpp/psi_cylinder_world.h"
#include "psi_cpp/psi_model.h"
#include "psi_cpp/psi_bbox.h"
#include "psi_cpp/psi_util.h"

#ifdef KJB_HAVE_UA_CARTWHEEL
#include <Control/CapsuleState.h>
#include <Control/SimulationInterface.h>
#include <Control/ExtendedAction.h>
#include <Control/WrapperAction.h>
#include <Control/StartState.h>
#endif

#include <boost/function.hpp>

#include <vector>
#include <cmath>

namespace kjb
{
namespace psi
{

class Bbox_pairwise_likelihood
{
public:
    Bbox_pairwise_likelihood(
            double xy_sigma,
            double width_sigma,
            double height_sigma) :
        xy_prob_(0.0, xy_sigma),
        width_prob_(0.0, width_sigma),
        height_prob_(0.0, height_sigma)
    { }

    double operator()(const Bbox& data, const Bbox& model) const;

private:
    kjb::Normal_distribution xy_prob_;
    kjb::Normal_distribution width_prob_;
    kjb::Normal_distribution height_prob_;
};

class Bbox_noise_likelihood
{
public:
    Bbox_noise_likelihood(
            double x_min,
            double x_max,
            double y_min,
            double y_max,
            double width_lambda,
            double height_lambda) :
        x_prob_(x_min, x_max),
        y_prob_(y_min, y_max),
        width_prob_(width_lambda),
        height_prob_(height_lambda)
    { }


    double operator()(const Bbox& data) const;
private:

    kjb::Uniform_distribution x_prob_;
    kjb::Uniform_distribution y_prob_;

    kjb::Exponential_distribution width_prob_;
    kjb::Exponential_distribution height_prob_;
};

class Frame_likelihood
{
public:
    Frame_likelihood(
            size_t width,
            size_t height,
            const boost::function2<double, const Bbox&, const Bbox&>& pairwise_box_likelihood,
            const boost::function1<double, const Bbox&>& noise_box_likelihood,
            double p_noise /// bernoulli noise process
            ) :
        P_MISSING_SWITCH_POINT(0.3), // hard-coded; may need tweaking or fitting
        P_MISSING_BASE_PROB(0.1), // hard-coded; may need tweaking
        P_MISSING_SMOOTHNESS(0.1), // hard-coded; may need tweaking
        p_noise_(p_noise),
        pair_likelihood_(pairwise_box_likelihood),
        noise_likelihood_(noise_box_likelihood),
        frame_bounds_(kjb::Vector(0.0, 0.0), (double) width, (double) height),
        marginalize_out_noise_(true)
    {}

    /**
     * Compare two sets of bounding boxes.
     *
     * This will find a correspodence between data and model by 
     * greedilly matching data boxes to model boxes or noise.
     *
     * @pre model_boxes are sorted in order of depth, with nearest first
     */
    double operator()(const std::vector<Bbox>& data_boxes, const std::vector<Bbox>& model_boxes) const
    {
    // TODO Somehow cache this and only update values that change with each iteration
        const std::vector<double> m_p_missing = get_p_missing(model_boxes);

        kjb::Matrix affinities = get_affinity(data_boxes, model_boxes, m_p_missing);


        std::vector<size_t> correspondences = get_correspondence_greedily(affinities);
        return this->operator()(data_boxes, model_boxes, affinities, correspondences, m_p_missing);
    }

    /**
    // Find the amount of a bounding box that is visible, from zero to 1.  Reduction in visibility
    //  can be the result of occlusion from other boxes in front of it, or from
    //  the box being parially (or completely) outside the view of the camera.
    //
    //  Currently, this occlusion approximated by discretizing the box into 64 cells, and doing a binary
    //  test on each cell to see if it's occluded.  As a result, the returned values will have
    //  a resolution of 1/64, but it gives a running time of O(n) instead of O(n^2 log n) for an
    //  exact solution (and less error-prone!!).
    //
    // @param all_boxes The set of all bounding boxes, sorted in order of depth, with nearest first.
    // @param i the index of the box to find the occlusion of
    // **/
    double compute_visibility(const std::vector<Bbox>& all_boxes, size_t box_i) const;

    /// @param visibility the amount of a box's area that is visible by the camera
    double p_missing(double visibility) const
    {
        if(visibility < 0 || visibility > 1.0)
        {
            KJB_THROW_2(kjb::Illegal_argument, "Visibility must be in [0,1].");
        }

        using kjb::cdf;

        // this is all hacked, and hand-tuned.  Maybe we can fit a function from data at some point?
       
        // using normal cdf as a soft step function.  sigma doesn't have any real meaning here, except the smoothness of the plot "looked okay" compared to the data.  Need to investigate this more...
        
        const double mean = P_MISSING_SWITCH_POINT;
        const double sigma = P_MISSING_SMOOTHNESS;

        const kjb::Normal_distribution normal_dist(mean, sigma);

        const double min = P_MISSING_BASE_PROB; // when no occlusion, miss detection 1% of the time, due to detector failure, or random occlusion from unmodelled objects

        const double obscured = 1-visibility;

        double result = log(min + (1-min) * cdf(normal_dist, obscured));

        return result;
    }

    /**
     * @param num_data the total number of data boxes
     * @param num_noise the number of data boxes assigned to noise */
    double p_correspondence(size_t num_data, size_t num_noise) const;

    /**
     * Returns a vector containing the probability of each model being undetected, based on how much it is occluded by the others and whether or not it is on the screen
     */
    std::vector<double> get_p_missing( const std::vector<Bbox>& model) const;

    kjb::Matrix get_affinity(
        const std::vector<Bbox>& data,
        const std::vector<Bbox>& model) const
    {
        return get_affinity(data, model, get_p_missing(model));
    }

    std::vector<size_t> get_correspondence_greedily( const kjb::Matrix& affinities) const;

    /**
     * You'll almost never call this.  This sets  a little flag that lets
     * the likelihood demo be more illustrative by allowing the 
     * correspondence matching to switch from "match" to "noise"
     * if the match is too bad.
     */
    Frame_likelihood& dont_marginalize_noise()
    {
        marginalize_out_noise_ = false;

        return *this;
    }

protected:
    double operator()(
            const std::vector<Bbox>& data_boxes,
            const std::vector<Bbox>& models,
            const kjb::Matrix& affinities,
            const std::vector<size_t>& correspondences,
            const std::vector<double>& m_p_missing) const;

    kjb::Matrix get_affinity(
            const std::vector<Bbox>& data,
            const std::vector<Bbox>& model,
            const std::vector<double>& m_p_missing
            ) const;

private:
    // p_missing_center parameter, base parameter
    const double P_MISSING_SWITCH_POINT; // this says how occluded a box can be before the detector misses it 50% of the time
    const double P_MISSING_BASE_PROB; // probability of a miss when fully visible
    const double P_MISSING_SMOOTHNESS; // how quickly the transition from zero to 1 occurs.  setting this to zero gives a step function

    // p_noise bernoilli parameter
    double p_noise_;

    // pair likelihood
    boost::function2<double, const Bbox&, const Bbox&> pair_likelihood_;
    // noise likelihood
    boost::function1<double, const Bbox&> noise_likelihood_;

    Bbox frame_bounds_;

    // this describes how the transition between pairwise matches
    // an noise models occurs.  If true, the transition is smooth,
    // because the noise model is "rolled into" the pairwise probability.
    // If false, the correspondence can switch from being pairwise to being
    // noise, which makes the likelihood demo more interesting, but offers
    // little other benefit.
    bool marginalize_out_noise_;

};

class Model_evaluator
{
public:
    virtual double operator()(const Model& m) const = 0;
};

#ifdef KJB_HAVE_UA_CARTWHEEL
class Cartwheel_likelihood : public kjb::psi::Model_evaluator
{
public:
    /**
     * @param data Bounding box set in format: data[frame][actor].  These should be pre-thinned, so the time delta between data boxes equals the time step between simulator outputs (TODO need a better way to explain this).
     * @param cam  A "standardize" perspective camera (screen origin at center of image, y-axis pointing upward).
     * @param width Width of the video frame
     * @param height height of the video frame
     *
     * @note The camera provided MUST be defined using "standard" screen coordinates; i.e. the origin of screen coordinates is located in the center of the frame, the x-axis points right, y-axis points up, and the z-axis points out of the screen.  
     */
    Cartwheel_likelihood(
            const std::vector<std::vector<Bbox> >& data,
            const kjb::Perspective_camera& cam,
            const Frame_likelihood& frame_likelihood,
            InterfacePtr interface,
//            size_t width,
//            size_t height,
//            const boost::function2<double, const Bbox&, const Bbox&>& pairwise_box_likelihood,
//            const boost::function1<double, const Bbox&>& noise_box_likelihood,
//            double p_noise, // bernoulli noise process
            double p_frame_count /// probability of each extra frame
            ) :
        data_(data),
        cam_(cam),
        interface_(interface),
        frame_likelihood_(frame_likelihood),
        frame_count_penalty_(p_frame_count)
//        P_MISSING_SWITCH_POINT(0.6), // hard-coded; may need tweaking or fitting
//        P_MISSING_BASE_PROB(0.01), // hard-coded; may need tweaking
//        p_noise_(p_noise),
//        noise_prob_(data_.size(), p_noise),
//        pair_likelihood_(pairwise_box_likelihood),
//        noise_likelihood_(noise_box_likelihood),
//        frame_bounds_(kjb::Vector(0.0, 0.0), (double) width, (double) height)
//
    { }

    virtual double operator()(const Model& m) const;

    std::vector<std::pair<double, Bbox> > blocks_to_boxes(const std::vector<CartWheel::BoxStatePtr>& box_state) const;

    std::vector<std::pair<double, Bbox> > capsules_to_boxes(const CartWheel::CapsuleState& capsule_state) const;

    std::vector<std::pair<double, Bbox> > capsules_to_boxes(const std::vector<std::vector<CartWheel::Math::Capsule*> >& actors_capsules) const;

    /**
    // Find the amount of a bounding box that is visible, from zero to 1.  Reduction in visibility
    //  can be the result of occlusion from other boxes in front of it, or from
    //  the box being parially (or completely) outside the view of the camera.
    //
    //  Currently, this occlusion approximated by discretizing the box into 64 cells, and doing a binary
    //  test on each cell to see if it's occluded.  As a result, the returned values will have
    //  a resolution of 1/64, but it gives a running time of O(n) instead of O(n^2 log n) for an
    //  exact solution (and less error-prone!!).
    //
    // @param all_boxes The set of all bounding boxes, sorted in order of depth, with nearest first.
    // @param i the index of the box to find the occlusion of
    // **/
//    double compute_visibility(const std::vector<Bbox>& all_boxes, size_t box_i) const;


private:
    const std::vector<std::vector<Bbox> >& data_;
    kjb::Perspective_camera cam_;
    mutable InterfacePtr interface_;

    Frame_likelihood frame_likelihood_;

    // penalty for requesting too many or too few frames
    kjb::Geometric_distribution frame_count_penalty_;
};
#endif

class Cylinder_world_likelihood : public kjb::psi::Model_evaluator
{
public:
    /**
     * @param data Bounding box set in format: data[frame][actor].  These should be pre-thinned, so the time delta between data boxes equals the time step between simulator outputs (TODO need a better way to explain this).
     * @param cam  A "standardize" perspective camera (screen origin at center of image, y-axis pointing upward).
     * @param width Width of the video frame
     * @param height height of the video frame
     *
     * @note The camera provided MUST be defined using "standard" screen coordinates; i.e. the origin of screen coordinates is located in the center of the frame, the x-axis points right, y-axis points up, and the z-axis points out of the screen.  
     */
    Cylinder_world_likelihood(
            boost::shared_ptr<std::vector<std::vector<std::vector<Bbox> > > > data,
            const kjb::Perspective_camera& cam,
            const Frame_likelihood& frame_likelihood,
            const Simple_simulator& sim,
            double p_frame_count /// probability of each extra frame
            ) :
        data_(data),
        cam_(cam),
        simulator_(sim),
        frame_likelihood_(frame_likelihood),
        frame_count_penalty_(p_frame_count),
        num_frames_(0)
    {

        for(size_t i = 0; i < data_->size(); i++)
        {
            if((*data_)[i].size() == 0) continue;

            if(num_frames_ == 0)
                num_frames_ = (*data_).size();
            else
            {
                if(num_frames_ != (*data_).size())
                    KJB_THROW_2(Illegal_argument, "Number of data frames must be equal for all object types.");
            }
        }

        ASSERT(num_frames_ > 0);
    }

    virtual double operator()(const Model& m) const;

    /// @brief convert 3d blocks into 2d bounding boxes, with depths from camera
    std::vector<std::pair<double, Bbox> > blocks_to_boxes(const std::vector<Cuboid>& blocks) const;

    /// @brief convert 3d entities into 2d bounding boxes, with depths from camera
    std::vector<std::pair<double, Bbox> > entities_to_boxes(const std::vector<std::vector<Entity_state> >& entities) const;

private:
    boost::shared_ptr<std::vector<std::vector<std::vector<Bbox> > > > data_;
    kjb::Perspective_camera cam_;
    mutable Simple_simulator simulator_;

    Frame_likelihood frame_likelihood_;

    // penalty for requesting too many or too few frames
    kjb::Geometric_distribution frame_count_penalty_;
    size_t num_frames_;
};


inline std::vector<Bbox> sort_by_depth(std::vector<std::pair<double, Bbox> >& boxes);

} // namespace psi
} // namespace kjb
#endif
