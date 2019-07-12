/*!
 * @file Mod_semantic_event.cpp
 *
 * @author Colin Dawson 
 * $Id: Mod_semantic_event.cpp 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Mod_semantic_event.h"

namespace semantics
{
    Mod_semantic_event::Mod_semantic_event(
	const Step_code_t&   step_code,
	const size_t&        type_code,
	const Node_data&     parent_data,
	const Node_data&     head_data,
	const Sem_hash_pair& parent_sem_data,
	const Sem_hash_pair& head_sem_data,
	const Distance_type& dist,
	bool                 learn
	) : Semantic_step_event(learn)
    {
	boost::array<Value_type, Msem_traits::size> data_array =
	    {{static_cast<Tree_event::Value_type>(step_code),
	      type_code,
	      parent_sem_data.get<HEAD>(), parent_data.get<LABEL>(),
	      dist,                        head_sem_data.get<HEAD>(),
	      head_data.get<LABEL>(),      head_data.get<TAG>(),
	      parent_sem_data.get<ARGS>(), head_sem_data.get<ARGS>(),
	      parent_data.get<WORD>()}};
	data_ = Data_type(data_array.begin(), data_array.end());
	update_event_views();
    }

    double Mod_semantic_event::log_probability(const bool& collins) const
    {
	return msem_view->smoothed_probability(collins);
    }

    void Mod_semantic_event::print_with_links(std::ostream& os) const
    {
	os << "MS: ";
	os << data_as_nonterminal("PLABEL") << "("
	   << data_as_nonterminal("HTAG") << ","
	   << data_as_word("HWORD") << ") "
	   <<  "{" << data_as_semantic_head("PSEM")
	   << data_as_semantic_args("PSEMARGS") << "} "
	   << "[" << data_as_nonterminal("HLABEL")
	   << " {" << data_as_semantic_head("HSEM")
	   << data_as_semantic_args("HSEMARGS") << "} ]"
	   << " --> ";
	print(os);
    }

    void Mod_semantic_event::print_view_counts(std::ostream& os) const
    {
	os << "------------" << std::endl;
	print_with_links(os);
	os << std::endl;
	os << "Key: " << data_ << std::endl;
	os << "Msem view: ";
	msem_view->display_conditioning_expression(os);
	os << "Numerators: " << msem_view->get_numerators() << std::endl;
	os << "Denominators: " << msem_view->get_denominators() << std::endl;
	os << "Diversities: " << msem_view->get_diversities() << std::endl;
	os << "Msem Probability: " << msem_view->smoothed_probability()
	   << std::endl;
    }

};

