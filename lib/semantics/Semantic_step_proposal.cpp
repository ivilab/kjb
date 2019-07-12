/*!
 * @file Semantic_step_proposal.cpp
 *
 * @author Colin Dawson 
 * $Id: Semantic_step_proposal.cpp 21596 2017-07-30 23:33:36Z kobus $ 
 */

#include "l/l_sys_debug.h"
#include "prob_cpp/prob_weight_array.h"
#include "prob_cpp/prob_util.h"
#include "semantics/Semantic_step_proposal.h"
#include "semantics/SemanticIO.h"
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/random/discrete_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <algorithm>
#include <cmath>
#include <ctime>

namespace semantics{
    template<typename T, size_t D>
    std::ostream& operator<<(
	std::ostream& os,
	const boost::array<T, D>& arr
	) {return kjb::operator<<(os,arr);}
};


namespace semantics
{
    
    /// Import typedefs from Semantic_step_propsal
    typedef Semantic_step_proposal::Elab_ptr             Elab_ptr;
    typedef Semantic_step_proposal::Elab_ptr_const       Elab_ptr_const;
    typedef Semantic_step_proposal::Syn_event_ptr        Syn_event_ptr;
    typedef Semantic_step_proposal::Sem_event_ptr        Sem_event_ptr;
    typedef Semantic_step_proposal::Proposal             Proposal;
    typedef Semantic_step_proposal::Proposal_array       Proposal_array;
    typedef Semantic_step_proposal::Step_result          Step_result;
    typedef Semantic_step_proposal::Proposal_dist        Proposal_dist;

    typedef std::vector<double>                          Probability_array;
    /// Meaning of positions in Proposal tuple
    const int STEP_CODE = 0, ELABORATION = 1, SEM_EVENT = 2, SYN_EVENT = 3;

    Proposal propose_a_step(
	const Elab_ptr_const semantic_parent,
	const Sem_event_ptr  old_sem_event,
	const Syn_event_ptr  old_syn_event,
	const Step_code_t    step
	)
    {
	Sem_event_ptr new_sem_event;
	Syn_event_ptr new_syn_event;
	Step_result semantic_result =
	    Elaboration_tree::take_a_step(semantic_parent, step);
	if(old_sem_event != NULL)
	{
	    new_sem_event = old_sem_event->get_a_copy(false);
	    new_sem_event -> update_step(step);
	}
	if(old_syn_event != NULL)
	{
	    new_syn_event = old_syn_event -> get_a_copy(false);
	    new_syn_event ->
		update_semantics(semantic_result.get<1>());
	}
	return boost::make_tuple(
	    step,
	    semantic_result.get<0>(),
	    new_sem_event,
	    new_syn_event
	    );
    }

    double calculate_proposal_probability(
        const Proposal& proposal,
        const bool&     collins
        )
    {
	Step_code_t tmp1;
	Elab_ptr_const tmp2;
	Sem_event_ptr sem_event;
	Syn_event_ptr syn_event;
	boost::tie(tmp1, tmp2, sem_event, syn_event) = proposal;
	double result =
	    exp((sem_event == NULL ? 0.0
		 : sem_event->log_probability(collins)) +
		(syn_event == NULL ? 0.0
		 : syn_event->log_probability(collins)));
	ASSERT(result > 0);
	return result;
    }

    boost::tuple<Proposal_array, Proposal_dist>
    generate_all_proposals(
	const Elab_ptr_const semantic_parent,
	const Sem_event_ptr  sem_event,
	const Syn_event_ptr  syn_event,
	const Step_code_t&   curr_step,
	const bool&          tree_is_altered,
        const bool&          collins
	)
    {
	Proposal_array proposals;
	Step_code::Weights probabilities;
	// Generate all possible step moves
	std::transform(
	    Step_code::codes.begin(), Step_code::codes.end(),
	    proposals.begin(),
	    boost::bind(
		propose_a_step,
		semantic_parent, sem_event, syn_event, _1
		)
	    );
	// Evaluate their (local) probabilities
	std::transform(
	    proposals.begin(), proposals.end(),
	    probabilities.begin(),
	    boost::bind(calculate_proposal_probability, _1, collins)
	    );
	// Apply a filter based on steps allowed from parent
	probabilities *= semantic_parent->step_filter();
	// If we have not yet modified the tree in this pass, then
	// the probability of staying put is increased
	// Generate a distribution and normalize the probabilities
	Proposal_dist dist(probabilities.begin(), probabilities.end());
	if(!tree_is_altered) 
	{
	    std::vector<Proposal_dist::input_type> probs =
		dist.probabilities();
	    double mutability_prob = Semantic_step_proposal::mutability_prob();
	    const size_t curr_index = static_cast<size_t>(curr_step);
	    // only modify at all with probability mutability_prob().
	    // then, choose a modification in proportion to the calculated
	    // local posterior
	    std::transform(
		probs.begin(), probs.end(), probs.begin(),
		boost::bind(
		    std::multiplies<Proposal_dist::input_type>(),
		    _1, mutability_prob)
		);
	    // there are two ways to stay put: either by choosing to modify
	    // and then selecting the current value, or by choosing not
	    // to modify
	    probs[curr_index] += (1 - mutability_prob);
	    // update the proposal distribution
	    dist = Proposal_dist(probs.begin(), probs.end());
	}
	return boost::make_tuple(proposals, dist);
    }


    /// Member definitions

    bool Semantic_step_proposal::VERBOSE = false;
    boost::mt19937 Semantic_step_proposal::urng = boost::mt19937(std::time(0));
    
    Semantic_step_proposal::Semantic_step_proposal()
	: state_(
	    boost::make_tuple(
		Step_code::IDENTITY,
		null_semantic_terminal(),
		Sem_event_ptr(),
		Syn_event_ptr()
		)
	    ),
	  prob_(1.0)
    {}
    
    void Semantic_step_proposal::sample_a_proposal(
	const Proposal_array&    proposals,
	const Proposal_dist&     dist
	)
    {
	if(VERBOSE)
	{
	    std::vector<double> probs = dist.probabilities();
	    std::cerr << "Probabilities are: "
		      << probs << std::endl;
	    std::cerr << "Sampling a proposal step...";
	}
	int result_index = dist(urng);
	if(VERBOSE)
	{
	    std::vector<double> probs = dist.probabilities();
	    std::cerr << "selected "
		      << enum_to_string<Step_code_t>(
			  static_cast<Step_code_t>(result_index)
			  )
		      << ", with probability "
		      << probs[result_index]
		      << std::endl;
	}
	// Set the state based on the result
	state_ = proposals[result_index];
	// Store the proposal probability
	prob_ = dist.probabilities()[result_index];
    }

    bool Semantic_step_proposal::propose(
	const Elab_ptr_const semantic_parent,
	const Sem_event_ptr  sem_event,
	const Syn_event_ptr  syn_event,
	const Step_code_t&   curr_step,
	const bool&          tree_is_altered,
        const bool&          collins
	)
    {
	Proposal_array proposals;
	Proposal_dist  proposal_dist;
	boost::tie(proposals, proposal_dist) =
	    generate_all_proposals(
		semantic_parent,
		sem_event,
		syn_event,
		curr_step,
		tree_is_altered,
                collins
		);
	sample_a_proposal(proposals, proposal_dist);
	return (tree_is_altered || state_.get<STEP_CODE>() != curr_step);
    }

    double Semantic_step_proposal::evaluate_a_proposal(
	const Elab_ptr_const semantic_parent,
	const Sem_event_ptr  sem_event,
	const Syn_event_ptr  syn_event,
	const Step_code_t    curr_step,
	const bool&          tree_is_altered,
        const bool&          collins
	)
    {
	using namespace ::kjb;
	Proposal_array proposals;
	Proposal_dist  dist;
	boost::tie(proposals, dist) =
	    generate_all_proposals(
		semantic_parent,
		sem_event,
		syn_event,
		curr_step,
		tree_is_altered,
                collins
                );
	if(VERBOSE)
	{
	    std::cerr << "Filter is "
		      << (semantic_parent->step_filter())
		      << std::endl;
	    std::cerr << "Currently in position "
		      << static_cast<int>(curr_step)
		      << std::endl;
	}
	return dist.probabilities()[static_cast<int>(curr_step)];
    }

};
