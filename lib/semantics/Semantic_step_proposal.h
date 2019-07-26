#ifndef SEMANTIC_STEP_PROPOSAL_H_
#define SEMANTIC_STEP_PROPOSAL_H_

/*!
 * @file Semantic_step_proposal.h
 *
 * @author Colin Dawson 
 * $Id: Semantic_step_proposal.h 17326 2014-08-19 01:57:36Z cdawson $ 
 */

#include "prob_cpp/prob_weight_array.h"
#include "semantics/Elaboration_tree.h"
#include "semantics/Syntactic_event.h"
#include "semantics/Semantic_step_event.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

namespace
{
    const double minimum = 0.001;
    const double mutability = 0.3;
}

namespace semantics
{
    class Semantic_step_proposal
    {
    public:
	typedef boost::shared_ptr<Semantic_step_proposal> Self_ptr;
	typedef Elaboration_tree::Elab_ptr                Elab_ptr;
	typedef Elaboration_tree::Elab_ptr_const          Elab_ptr_const;
	typedef Syntactic_event::Event_ptr                Syn_event_ptr;
	typedef Semantic_step_event::Event_ptr            Sem_event_ptr;
	typedef boost::tuple<Step_code_t, Elab_ptr_const, Sem_event_ptr, Syn_event_ptr>
	Proposal;
	typedef boost::array<Proposal, Step_code::NUM_STEPS> Proposal_array;
	typedef Elaboration_tree::Step_result             Step_result;
	typedef boost::random::discrete_distribution<>    Proposal_dist;
	static boost::mt19937 urng;
	static bool VERBOSE;
    public:
	Semantic_step_proposal();

	/*! @brief generate a proposal and store the result and the log prob
	 *  @param semantic_parent the semantic node which is the starting point
	 *  @param sem_event ptr to the semantic event object currently in use
	 *  @param syn_event ptr to the syntactic event object currently in use
	 */
	bool propose(
	    const Elab_ptr_const semantic_parent,
	    const Sem_event_ptr  sem_event,
	    const Syn_event_ptr  syn_event,
	    const Step_code_t&   curr_step,
	    const bool&          tree_is_altered,
            const bool&          collins = false
	    );

	/*! @brief given the current step, compute the probability of proposing it
	 *  @param sem_event pointer to the current semantic event
	 *  @param syn_event pointer to the current syntactic event
	 *  @param step the code of the step being evaluated
	 *  @return the log probability of proposing the current step code
	 */
	static double evaluate_a_proposal(
	    const Elab_ptr_const semantic_parent,
	    const Sem_event_ptr  sem_event,
	    const Syn_event_ptr  syn_event,
	    const Step_code_t    step,
	    const bool&          tree_is_altered,
            const bool&          collins = false
	    );

	/*! @brief a smoothing factor
	 */
	static const double& min_prob()
	{
	    static double mp = minimum;
	    return mp;
	}

	/*! @brief probability of initiating a move for a subtree
	 */
	static const double& mutability_prob()
	{
	    static double mp = mutability;
	    return mp;
	}
	
	Proposal state(){return state_;}
	double prob(){return prob_;}
    private:
	/*! @brief sample one proposal from an array of candidates
	 */
	void sample_a_proposal(
	    const Proposal_array&  proposals,
	    const Proposal_dist&   dist
	    );
	
	Proposal state_; 
	double prob_;
    };
}

#endif
