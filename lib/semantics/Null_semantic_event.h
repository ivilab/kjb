#ifndef NULL_SEMANTIC_EVENT_H_
#define NULL_SEMANTIC_EVENT_H_

/*!
 * @file Null_semantic_event.h
 *
 * @author Colin Dawson 
 * $Id: Null_semantic_event.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Semantic_step_event.h"
#include "semantics/Event_traits.h"

namespace semantics
{
    class Null_semantic_event : public Semantic_step_event
    {
    public:
	Null_semantic_event() : Semantic_step_event(false){}

	Semantic_step_event::Event_ptr get_a_copy(bool) const 
	{
	    return boost::make_shared<Null_semantic_event>();
	}

	double log_probability(const bool&) const {return 0.0;}

	void update_event_views() {}

        void resample_table_assignments() {}

	void print(std::ostream&) const {}

	void print_view_counts(std::ostream&) const {}

	void update_step(const Step_code_t&) {};

	const Key_slots::Map& var_map() const
	{
	    return Hsem_traits::variable_map;
	}
    };
};

#endif
