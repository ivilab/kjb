/*!
 * @file Root_event.cpp
 *
 * @author Colin Dawson 
 * $Id: Root_event.cpp 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Root_event.h"

namespace semantics
{
    Root_event::Root_event(
	const Node_data&                     node_data,
#ifdef USE_SEMANTICS
	const Semantic_data_base::Hash_pair& sem_data,
#endif
	int                                  id,
	bool                                 learn
	) : Syntactic_event(id, learn)
    {
#ifdef USE_SEMANTICS
	boost::array<Value_type, S2_traits::size> data_array =
	    {{node_data.get<WORD>(),     node_data.get<TAG>(),
	      node_data.get<LABEL>(),    sem_data.get<HEAD>(),
	      Nonterminal_db::root_code(), sem_data.get<ARGS>()}};
#else
	boost::array<Value_type, S2_traits::size> data_array =
	    {{node_data.get<WORD>(),   node_data.get<TAG>(),
	      node_data.get<LABEL>(),  Nonterminal_db::root_code()}};
#endif
	data_ = Data_type(data_array.begin(), data_array.end());
	update_event_views();
    }

    double Root_event::log_probability(const bool& collins) const
    {
	return s1_view->smoothed_probability(collins) +
	    s2_view->smoothed_probability(collins);
    }

    void Root_event::print(std::ostream& os) const
    {
	os << "**";
	Syntactic_event::print(os);
    }
    
    void Root_event::print_with_links(std::ostream& os) const
    {
	os << "S: ";
	os << Nonterminal_db::root_key();
#ifdef USE_SEMANTICS
	os << " {" << data_as_semantic_head("SEM")
	   << data_as_semantic_args("SEMARGS") << "} ";
#endif
	os << " --> ";
	print(os);
    }

    void Root_event::print_view_counts(std::ostream& os) const
    {
	os << "----------" << std::endl;
	print_with_links(os);
	os << std::endl;
	os << "Key: " << data_ << std::endl;
	os << "S1 view: ";
	s1_view->display_conditioning_expression(os);
	os << "Numerators: " << s1_view->get_numerators() << std::endl;
	os << "Denominators: " << s1_view->get_denominators() << std::endl;
	os << "Diversities: " << s1_view->get_diversities() << std::endl;
	os << "S1 Probability: " << s1_view->smoothed_probability() << std::endl;
	os << std::endl;
	os << "S2 view: ";
	s2_view->display_conditioning_expression(os);
	os << "Numerators: " << s2_view->get_numerators() << std::endl;
	os << "Denominators: " << s2_view->get_denominators() << std::endl;
	os << "Diversities: " << s2_view->get_diversities() << std::endl;
	os << "S2 Probability: " << s2_view->smoothed_probability()
	   << std::endl;
    }
    
};

