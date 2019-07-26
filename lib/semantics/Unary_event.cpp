/*!
 * @file Unary_event.cpp
 *
 * @author Colin Dawson U
 * $Id: Unary_event.cpp 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Unary_event.h"

namespace semantics
{
    
    Unary_event::Unary_event(
	const Node_data&                   node_data,
#ifdef USE_SEMANTICS
	const Elaboration_tree::Hash_pair& sem_data,
	const Elaboration_tree::Hash_pair& psem_data,
#endif
	const Label_type&                  parent_label,
	int                                id,
	bool                               learn
	) : Syntactic_event(id, learn)
    {
#ifdef USE_SEMANTICS
	boost::array<Value_type, U_traits::size> data_array =
	    {{node_data.get<LABEL>(), parent_label,
	      sem_data.get<HEAD>(),   psem_data.get<HEAD>(),
	      node_data.get<TAG>(),   sem_data.get<ARGS>(),
	      psem_data.get<ARGS>(),   node_data.get<WORD>()}};
#else
	boost::array<Value_type, U_traits::size> data_array =
	    {{node_data.get<LABEL>(), parent_label,
	      node_data.get<TAG>(),   node_data.get<WORD>()}};
#endif
	data_ = Data_type(data_array.begin(), data_array.end());
	update_event_views();
    }

    double Unary_event::log_probability(const bool& collins) const
    {
	return u_view->smoothed_probability(collins);
    }

    void Unary_event::print(std::ostream& os) const
    {
	os << "*";
	Syntactic_event::print(os);
    }

    void Unary_event::print_with_links(std::ostream& os) const
    {
	os << "U: ";
	os << data_as_nonterminal("PLABEL") << "("
	   << data_as_nonterminal("TAG") << ","
	   << data_as_word("WORD") << ")";
#ifdef USE_SEMANTICS
	os << " {" << data_as_semantic_head("SEM")
	   << data_as_semantic_args("SEMARGS") << "} ";
#endif
	os << " --> ";
	os << data_as_nonterminal("LABEL");
    }

    void Unary_event::print_view_counts(std::ostream& os) const
    {
	os << "------------" << std::endl;
	print_with_links(os);
	os << std::endl;
	os << "Key: " << data_ << std::endl;
	os << "U view: ";
	u_view->display_conditioning_expression(os);
	os << "Numerators: " << u_view->get_numerators() << std::endl;
	os << "Denominators: " << u_view->get_denominators() << std::endl;
	os << "Diversities: " << u_view->get_diversities() << std::endl;
	os << "U Probability: " << u_view->smoothed_probability()
	   << std::endl;
    }


};

