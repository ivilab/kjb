/*!
 * @file Event_parser.cpp
 *
 * @author Colin Dawson 
 * $Id: Event_parser.cpp 21596 2017-07-30 23:33:36Z kobus $ 
 */

#include "l/l_sys_debug.h"
#include "semantics/Event_parser.h"
#include "semantics/Unary_event.h"
#include "semantics/TOP_event.h"
#include "semantics/Root_event.h"
#include "semantics/Dependency_event.h"
#include "semantics/Punctuation_event.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <sstream>
#ifdef USE_SEMANTICS
#include "semantics/Elaboration_tree.h"
#endif

namespace spear
{
    extern size_t LF_WORD_THRESHOLD;
}

namespace semantics
{
    bool Event_db::VERBOSE = false;

    void Event_db::read_events(
	const std::string&  event_file,
	const int&          num_lines
	)
    {
	std::string event;
	std::ifstream input_stream(event_file.c_str());
	for(int lines_left = num_lines;
	    getline(input_stream, event) && lines_left > 0;
	    lines_left--)
	{
	    std::istringstream event_stream(event);
	    read_event(event_stream);
	}
    }

    bool Event_db::read_event(std::istringstream& event_stream)
    {
	int event_code;
	event_stream >> event_code;
	switch(event_code)
	{
	case(6):
	    if(num_trees() % 1 == 0)
		std::cerr << "Sentence # " << num_trees() << std::endl;
	    return read_event_f(event_stream);
	case(2):
	    return read_event_d(event_stream);
	case(3):
	    return read_event_u(event_stream);
	default:
	    KJB_THROW_2(
		kjb::IO_error,
		"Unknown event code encountered in parser");
	}
    }

    bool Event_db::read_event_u(std::istringstream& input)
    {
	Token_map::Val_type event_id, parent_num, word, tag;
	Token_map::Val_type label, parent_label;
	Event_ptr parent;
	std::string tmp;
	input >> event_id;
	input >> parent_num;
	input >> tmp;
	word = lexicon().encode(tmp, learn_);
	input >> tmp;
	tag = nt_lexicon().encode(tmp, learn_);
	input >> tmp;
	parent_label = nt_lexicon().encode(tmp, learn_);
	input >> tmp;
	label = nt_lexicon().encode(tmp, learn_);
	// skip subcat frame
	if(parent_num == 0)
	{
	    if(lf_word_map()[word] == false && word != 0)
	    {
		word = Lexicon_db::UNKNOWN_TOKEN_VAL;
	    }
	    Event_ptr top = boost::make_shared<TOP_event>(0);
	    Syntactic_event::Node_data node_data(word, tag, label);
#ifdef USE_SEMANTICS
	    Event_ptr root = boost::make_shared<Root_event>(
		node_data, Elaboration_tree::Hash_pair(0,0), event_id, learn_
		);
#else
	    Event_ptr root = boost::make_shared<Root_event>(
		node_data, event_id, learn_
		);
#endif
	    top -> add_head_child(root);
	    tree_list_.push_back(top);
	    event_list_.push_back(root);
	    if(VERBOSE)
	    {
		std::cout << "Created Root event " << *event_list_.back()
			  << " at position " << event_id
			  << std::endl;
	    }
	} else {
	    Syntactic_event::Node_data node_data(word, tag, label);
	    parent = event_list_[parent_num - num_events_];
#ifdef USE_SEMANTICS
	    Event_ptr unary_event =
		boost::make_shared<Unary_event>(
		    node_data,
		    Elaboration_tree::Hash_pair(0,0),
		    Elaboration_tree::Hash_pair(0,0),
		    parent_label,
		    event_id, learn_);
#else
	    Event_ptr unary_event =
		boost::make_shared<Unary_event>(
		    node_data, parent_label, event_id, learn_);
#endif
	    parent -> add_head_child(unary_event);
	    event_list_.push_back(unary_event);
	    if(VERBOSE)
	    {
		// std::cout << "Created Unary event " << *event_list_.back()
		// 	  << " at position " << event_id
		// 	  << std::endl;
	    }
	}
	return true;
    }


    bool Event_db::read_event_d(std::istringstream& input)
    {
	int event_id, parent_num, head_num;
	Token_map::Val_type word, tag, label;
	Token_map::Val_type head_word, head_tag, head_label;
	Token_map::Val_type parent_label;
	Token_map::Val_type distance;
	bool coord_flag, punc_flag;
	Token_map::Val_type punc_tag = 0, punc_word = 0;
	Token_map::Val_type coord_tag = 0, coord_word = 0;
	Event_ptr parent, head;
	std::string tmp;
	input >> event_id;
	input >> parent_num;
	input >> head_num;
	head = event_list_[head_num - num_events_];
	input >> tmp;
	word = lexicon().encode(tmp, learn_);
	input >> tmp;
	tag = nt_lexicon().encode(tmp, learn_);
	input >> tmp;
	head_word = lexicon().encode(tmp, learn_);
	// assert(head_word == head -> word());
	input >> tmp;
	head_tag = nt_lexicon().encode(tmp, learn_);
	input >> tmp;
	label = nt_lexicon().encode(tmp, learn_);
	input >> tmp;
	parent_label = nt_lexicon().encode(tmp, learn_);
	input >> tmp;
	head_label = nt_lexicon().encode(tmp, learn_);
	// assert(head_label == head -> label());
	input >> tmp;
	// skip subcat frame
	input >> distance;
	input >> coord_flag;
	if(coord_flag == true)
	{
	    input >> tmp;
	    coord_word = lexicon().encode(tmp, learn_);
	    input >> tmp;
	    coord_tag = nt_lexicon().encode(tmp, learn_);
	}
	input >> punc_flag;
	if(punc_flag == true)
	{
	    input >> tmp;
	    punc_word = lexicon().encode(tmp, learn_);
	    input >> tmp;
	    punc_tag = nt_lexicon().encode(tmp, learn_);
	}

	if(lf_word_map()[word] == false && word != 0)
	{
	    word = Lexicon_db::UNKNOWN_TOKEN_VAL;
	}

	Syntactic_event::Node_data node_data(word, tag, label);
	Syntactic_event::Node_data parent_data(0, 0, parent_label);
	Syntactic_event::Node_data head_data(head_word, head_tag, head_label);
	Syntactic_event::Node_data punc_data(punc_word, punc_tag, 0);
	Syntactic_event::Node_data coord_data(coord_word, coord_tag, 0);
	bool on_left = distance >= 100;
	if(parent_num == 0)
	{
	    assert(parent_label == Nonterminal_db::root_code());
	    if(learn_ == true)
	    {
		parent = tree_list_.back();
	    }
	    else
	    {
		Event_ptr e(new Punctuation_event(event_id));
		event_list_.push_back(e);
		return true;
	    }
	} else {
	    parent = event_list_[parent_num - num_events_];
	}
#ifdef USE_SEMANTICS
	Event_ptr dependency(
	    new Dependency_event(
		node_data, parent_data, head_data,
		Elaboration_tree::Hash_pair(0, 0),
		Elaboration_tree::Hash_pair(0, 0),
		Elaboration_tree::Hash_pair(0, 0),
		distance,
		punc_flag, punc_data,
		coord_flag, coord_data,
		event_id, learn_
		)
	    );
#else
	Event_ptr dependency(
	    new Dependency_event(
		node_data, parent_data, head_data,
		distance,
		punc_flag, punc_data,
		coord_flag, coord_data,
		event_id, learn_
		)
	    );
#endif
	parent -> add_dependency_child(dependency, on_left);
	event_list_.push_back(dependency);
	if(VERBOSE)
	{
	    // std::cout << "Created Dependency event " << *event_list_.back()
	    // 	      << " at position " << event_id
	    // 	      << std::endl;
	}
	return true;
    }

    void Event_db::read_lexicon(const std::string& lexicon_file)
    {
	std::string word, tag;
	bool hi_freq_flag;
	std::string line_string;
	std::ifstream lex_stream(lexicon_file.c_str());
	while(std::getline(lex_stream, line_string))
	{
	    std::istringstream line(line_string);
	    line >> word;
	    line >> tag;
	    line >> hi_freq_flag;
	    Token_map::Val_type wcode = lexicon().encode(word, true);
	    lf_word_map()[wcode] = hi_freq_flag;
	}
    }

    /// Free_function definitions
    
    Event_db process_event_file(
	std::string train_path,
	std::string lexicon_file,
	size_t      max_events,
	std::string test_path
	)
    {
	std::string train_events = train_path + "/events.unzipped";
	std::string test_events = test_path + "/events.unzipped";
    
	if(test_path.empty())
	{
	    std::cerr << "LEXICON:" << std::endl;
    
	    for(Lexicon_db::const_iterator it = Event_db::lexicon().begin();
	        it != Event_db::lexicon().end(); it++)
	    {
	        std::cerr << it->left << ": " << it->right
			  << std::endl;
	    }
	    std::cerr << std::endl;
	    std::cerr << "TRAINING:" << std::endl;
	    return Event_db(train_events, lexicon_file, true, max_events);
	} else {
	    std::cerr << std::endl;
	    std::cerr << "TESTING:" << std::endl;
	
	    return Event_db(test_events, lexicon_file, false, max_events);
	}
    }

};

