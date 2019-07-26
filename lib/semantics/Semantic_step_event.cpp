/*!
 * @file Semantic_step_event.cpp
 *
 * @author Colin Dawson 
 * $Id: Semantic_step_event.cpp 16887 2014-05-24 04:29:00Z cdawson $ 
 */

#include "semantics/Semantic_step_event.h"
#include <iostream>

namespace semantics
{
    void Semantic_step_event::print(std::ostream& os) const
    {
	os << "{" << data_as_semantic_step("STEPCODE") << "}";
    }

    void Semantic_step_event::update_step(const Step_code_t& val)
    {
	data_in_slot("STEPCODE") = val;
	update_event_views();
    }


};

