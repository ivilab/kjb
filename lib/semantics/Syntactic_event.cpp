/*!
 * @file Syntactic_event.cpp
 *
 * @author Colin Dawson 
 * $Id: Syntactic_event.cpp 16947 2014-06-03 05:13:51Z cdawson $ 
 */

#include "semantics/Syntactic_event.h"
#include <cmath>
#include <iostream>

namespace semantics
{

/// Static initialization

bool Syntactic_event::VERBOSE = false;

/// Member function implementation

Syntactic_event::Syntactic_event(int id, bool learn)
    : Tree_event(learn), id_(id), head_pos_(0)
{}
    
Syntactic_event::Syntactic_event(
    const Data_type& data,
    int              id,
    bool             learn
    ) : Tree_event(data, learn),
	id_(id),
	head_pos_(0)
{}

double Syntactic_event::subtree_log_probability() const
{
    double return_val = log_probability();
    if(VERBOSE)
    {
	std::cerr << *this << " : " << return_val << std::endl;
    }
    for(Event_ptr_container::const_iterator it = children_.begin();
	it != children_.end();
	it++)
    {
	return_val += (*it) -> subtree_log_probability();
    }
    return return_val;
}

void Syntactic_event::update_semantics(const Sem_hash_pair& vals)
{
    boost::tie(data_in_slot("SEM"), data_in_slot("SEMARGS")) = vals;
    update_event_views();
}
    
    
void Syntactic_event::print(std::ostream& os) const
{
    os << data_as_nonterminal("LABEL")
       << "("
       << data_as_nonterminal("TAG")
       << ","
       << data_as_word("WORD")
       << ")";
}

void Syntactic_event::print_semantics(std::ostream& os) const
{
    os << "{" << data_as_semantic_head("SEM")
       << data_as_semantic_args("SEMARGS")
       << "}";
}

void Syntactic_event::print_subtree(std::ostream& os, int indent_level) const
{
    if(data_as_code("WORD") != Lexicon_db::stop_code())
    {
	os << std::string(4 * indent_level, ' ');
	print(os);
	os << std::endl;
    }
    print_child_trees(os, indent_level + 1);
}

void Syntactic_event::print_constituency_tree_with_head(std::ostream& os) const
{
    if(data_as_code("WORD") != Lexicon_db::stop_code())
    {
	os << "(" << data_as_nonterminal("LABEL") << " ";
    } else {
	return;
    }
    if(children_.empty())
    {
	os << data_as_word("WORD");
    } else {
	print_child_constituency_trees(os);
	os << head_pos_ - 1; // STOP is first, but we don't want to count that
    }
    os << ") ";
}

void Syntactic_event::print_child_constituency_trees(std::ostream& os) const
{
    for(Event_ptr_container::const_iterator it = children_.begin();
	it != children_.end(); ++it)
    {
	if(*it != NULL) (*it) -> print_constituency_tree_with_head(os);
    }
}

void Syntactic_event::print_subtree_view_counts(
    std::ostream& os,
    int indent_level
    ) const
{
    std::string lead(indent_level*4, ' ');
    os << lead;
    this -> print_view_counts(os);
    for(Event_ptr_container::const_iterator it = children_.begin();
	it != children_.end();
	it++)
    {
	if(*it) (*it) -> print_subtree_view_counts(os, indent_level + 1);
    }
}

void Syntactic_event::print_child_trees(std::ostream& os, int indent_level) const
{
    for(Event_ptr_container::const_iterator it = children_.begin();
	it != children_.end();
	it++)
    {
	if(*it) (*it) -> print_subtree(os, indent_level);
    }
}


};

