#ifndef SEMANTIC_STEP_EVENT_H_
#define SEMANTIC_STEP_EVENT_H_

/*!
 * @file Semantic_step_event.h
 *
 * @author Colin Dawson 
 * $Id: Semantic_step_event.h 16887 2014-05-24 04:29:00Z cdawson $ 
 */

#include "semantics/Tree_event.h"

namespace semantics
{
    class Semantic_step_event : public Tree_event
    {
    public:
	typedef boost::shared_ptr<Semantic_step_event> Event_ptr;
    protected:
	Semantic_step_event(bool learn) : Tree_event(learn) {}
	Semantic_step_event(Data_type data, bool learn) : Tree_event(data, learn) 
	{}
	virtual ~Semantic_step_event() {};
    public:
	virtual Event_ptr get_a_copy(bool learn = false) const = 0;

	void print(std::ostream& os) const;

	virtual void update_step(const Step_code_t& val);
    };
};

#endif
