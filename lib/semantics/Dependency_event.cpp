#include "semantics/Dependency_event.h"
#include <boost/array.hpp>
#include <stdexcept>

namespace semantics
{

    Dependency_event::Dependency_event(
	const Node_data&                     node_data,
	const Node_data&                     parent_data,
	const Node_data&                     head_data,
#ifdef USE_SEMANTICS
	const Elaboration_tree::Hash_pair&   sem_data,
	const Elaboration_tree::Hash_pair&   parent_sem_data,
	const Elaboration_tree::Hash_pair&   head_sem_data,
#endif
	const Distance_type&                 distance,
	bool                                 punc_flag,
	const Node_data&                     punc_data,
	bool                                 coord_flag,
	const Node_data&                     coord_data,
	int                                  id,
	bool                                 learn
	) : Syntactic_event(id, learn)
    {
#ifdef USE_SEMANTICS
	boost::array<Value_type, 16> data_array =
	    {{node_data.get<WORD>(),      node_data.get<TAG>(),
	      node_data.get<LABEL>(),     punc_flag, coord_flag,
	      parent_data.get<LABEL>(),   sem_data.get<HEAD>(),
	      head_data.get<LABEL>(),     parent_sem_data.get<HEAD>(),
	      head_sem_data.get<HEAD>(),  distance,
	      head_data.get<TAG>(),       sem_data.get<ARGS>(),
	      parent_sem_data.get<ARGS>(),head_sem_data.get<1>(),
	      head_data.get<WORD>()}};
#else
	boost::array<Value_type, 10> data_array =
	    {{node_data.get<WORD>(),    node_data.get<TAG>(),
	      node_data.get<LABEL>(),   punc_flag,
	      coord_flag,               parent_data.get<LABEL>(),
	      head_data.get<LABEL>(),   distance,
	      head_data.get<TAG>(),     head_data.get<WORD>()}};
#endif
	data_ = Data_type(data_array.begin(), data_array.end());
	if(punc_flag == true)
	{
	    boost::array<Value_type, 10> punc_array =
		{{punc_data.get<WORD>(), punc_data.get<TAG>(),
		  0, parent_data.get<LABEL>(), head_data.get<LABEL>(),
		  node_data.get<LABEL>(), head_data.get<TAG>(),
		  node_data.get<TAG>(), head_data.get<WORD>(),
		  node_data.get<WORD>()}};
	    pcc_data_ = Data_type(punc_array.begin(), punc_array.end());
	}
	if(coord_flag == true)
	{
	    boost::array<Value_type, 10> coord_array =
		{{coord_data.get<WORD>(),    coord_data.get<TAG>(),
		  1,                         parent_data.get<LABEL>(),
		  head_data.get<LABEL>(),    node_data.get<LABEL>(),
		  head_data.get<TAG>(),      node_data.get<TAG>(),
		  head_data.get<WORD>(),     node_data.get<WORD>()}};
	    coord_data_ = Data_type(coord_array.begin(), coord_array.end());
	}
	update_event_views();
    }

    double Dependency_event::log_probability(const bool& collins) const
    {
	return d1_view->smoothed_probability(collins) +
	    (data_as_code("WORD") == Lexicon_db::stop_code() ?
	     0.0 :
	     d2_view->smoothed_probability(collins)) +
	    (punc1_view != NULL ?
	     punc1_view->smoothed_probability(collins) +
	     punc2_view->smoothed_probability(collins) :
	     0.0) +
	    (coord1_view != NULL ?
	     coord1_view->smoothed_probability(collins) +
	     coord2_view->smoothed_probability(collins) :
	     0.0);
    }

    void Dependency_event::print_with_links(std::ostream& os) const
    {
	os << "D: ";
	os << data_as_nonterminal("PLABEL") << "("
	   << data_as_nonterminal("HTAG") << ","
	   << data_as_word("HWORD") << ")";
#ifdef USE_SEMANTICS
	os << " {" << data_as_semantic_head("SEM")
	   << data_as_semantic_args("SEMARGS") << "} ";
#endif
	os << " --" << data_as_code("DIST") << "("
	   << data_as_code("PUNC") << ","
	   << data_as_code("CONJ") << ")--> ";
	print(os);
	os << " [";
	os << data_as_nonterminal("HLABEL") << "("
	   << data_as_nonterminal("HTAG") << ","
	   << data_as_word("HWORD") << ")";
	os << "]";
    }

    void Dependency_event::print_view_counts(std::ostream& os) const
    {
	os << "-----------" << std::endl;
	print_with_links(os);
	os << std::endl;
	os << "Key: " << data_ << std::endl;
	os << "D1 view: ";
	d1_view->display_conditioning_expression(os);
	os << "Numerators: " << d1_view->get_numerators() << std::endl;
	os << "Denominators: " << d1_view->get_denominators() << std::endl;
	os << "Diversities: " << d1_view->get_diversities() << std::endl;
	os << "D1 Probability: " << d1_view->smoothed_probability() << std::endl;
	os << std::endl;
	if(d2_view != NULL)
	{
	    os << "D2 view: ";
	    d2_view->display_conditioning_expression(os);
	    os << "Numerators: " << d2_view->get_numerators() << std::endl;
	    os << "Denominators: " << d2_view->get_denominators() << std::endl;
	    os << "Diversities: " << d2_view->get_diversities() << std::endl;
	    os << "D2 Probability: " << d2_view->smoothed_probability()
	       << std::endl;
	} else {
	    os << "At a #STOP#." << std::endl;
	}
    }

    void Dependency_event::update_event_views()
    {
	d1_view =
	    boost::make_shared<D1_view>(data_.begin() + 1, data_.end(), learn_);
	if(data_as_code("WORD") != Lexicon_db::stop_code())
	{
	    d2_view =
		boost::make_shared<D2_view>(data_.begin(), data_.end(), learn_);
	}
	if(data_as_code("PUNC") == 1)
	{
	    punc1_view =
		boost::make_shared<PCC1_view>(
		    pcc_data_.begin() + 1, pcc_data_.end(), learn_);
	    punc2_view =
		boost::make_shared<PCC2_view>(
		    pcc_data_.begin(), pcc_data_.end(), learn_);
	}
	if(data_as_code("CONJ") == 1)
	{
	    coord1_view =
		boost::make_shared<PCC1_view>(
		    coord_data_.begin() + 1, coord_data_.end(), learn_);
	    coord2_view =
		boost::make_shared<PCC2_view>(
		    coord_data_.begin(), coord_data_.end(), learn_);
	}
    }
};

