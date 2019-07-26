/*!
 * @file Head_semantic_event.cpp
 *
 * @author Colin Dawson 
 * $Id: Head_semantic_event.cpp 18252 2014-11-20 00:59:17Z ksimek $ 
 */

#include "semantics/Head_semantic_event.h"

namespace semantics
{
    Head_semantic_event::Head_semantic_event(
	const Step_code_t&   step_code,
	const size_t&        type_code,
	const Node_data&     parent_data,
	const Sem_hash_pair& parent_sem_data,
	bool                 learn
	) : Semantic_step_event(learn)
    {
	boost::array<Value_type, Hsem_traits::size> data_array =
	    {{static_cast<unsigned long>(step_code), type_code,
	      parent_sem_data.get<HEAD>(), parent_data.get<LABEL>(),
	      parent_data.get<TAG>(),      parent_sem_data.get<ARGS>(),
	      parent_data.get<WORD>()}};
	data_ = Data_type(data_array.begin(), data_array.end());
	update_event_views();
    }

    double Head_semantic_event::log_probability(const bool& collins) const
    {
	return hsem_view->smoothed_probability(collins);
    }

    void Head_semantic_event::print_with_links(std::ostream& os) const
    {
	os << "HS: ";
	os << data_as_nonterminal("PLABEL") << "("
	   << data_as_nonterminal("PTAG") << ","
	   << data_as_word("PWORD") << ") "
	   <<  "{" << data_as_semantic_head("PSEM")
	   << data_as_semantic_args("PSEMARGS") << "}"
	   << " --> ";
	print(os);
    }

    void Head_semantic_event::print_view_counts(std::ostream& os) const
    {
	os << "------------" << std::endl;
	print_with_links(os);
	os << std::endl;
	os << "Key: " << data_ << std::endl;
	os << "Hsem view: ";
	hsem_view->display_conditioning_expression(os);
	os << "Numerators: " << hsem_view->get_numerators() << std::endl;
	os << "Denominators: " << hsem_view->get_denominators() << std::endl;
	os << "Diversities: " << hsem_view->get_diversities() << std::endl;
	os << "Hsem Probability: " << hsem_view->smoothed_probability()
	   << std::endl;
    }

};

