/*!
 * @file Tree_event.cpp
 *
 * @author Colin Dawson 
 * $Id: Tree_event.cpp 17119 2014-07-17 00:46:17Z cdawson $
 */

#include "semantics/Tree_event.h"
#include <cmath>

namespace semantics
{

/*------------------------------------------------------------
 * STATIC INITIALIZATION
 *------------------------------------------------------------*/
    
bool Tree_event::VERBOSE = false;

Lexicon_db& Tree_event::lexicon()
{
    static boost::shared_ptr<Lexicon_db> lex(new Lexicon_db());
    return *lex;
}

Nonterminal_db& Tree_event::nt_lexicon()
{
    static boost::shared_ptr<Nonterminal_db> ntl(new Nonterminal_db());
    return *ntl;
}


/*------------------------------------------------------------
 * MEMBER FUNCTION IMPLEMENTATION
 *------------------------------------------------------------*/
    
Tree_event::Tree_event(bool learn) : learn_(learn) {}
    
Tree_event::Tree_event(const Data_type& data, bool learn)
    : data_(data), learn_(learn)
{}

const Tree_event::Key_type& Tree_event::data_as_word(
    const Key_type& variable
    ) const
{
    return lexicon().decode(data_as_code(variable));
}

const Tree_event::Key_type& Tree_event::data_as_nonterminal(
    const Key_type& variable
    ) const
{
    return nt_lexicon().decode(data_as_code(variable));
}

const Tree_event::Key_type Tree_event::data_as_semantic_step(
    const Key_type& variable
    ) const
{
    return enum_to_string<Step_code_t>(
	static_cast<Step_code_t>(data_as_code(variable))
	);
}

const Tree_event::Key_type& Tree_event::data_as_semantic_head(
    const Key_type& variable
    ) const
{
    try
    {
	return Semantic_data_base::global_head_map()[data_as_code(variable)];
    } catch(std::out_of_range& oor) {
	std::cerr << "Attempted to translate unknown hash "
		  << data_as_code(variable)
		  << "into semantic head: " << oor.what()
		  << std::endl;
	throw;
    }
}

const Tree_event::Key_type& Tree_event::data_as_semantic_args(
    const Key_type& variable
    ) const
{
    try
    {
	return Semantic_data_base::global_arg_map()[data_as_code(variable)];
    } catch(std::out_of_range& oor) {
	std::cerr << "Attempted to translate unknown hash "
		  << data_as_code(variable)
		  << "into semantic arg: " << oor.what()
		  << std::endl;
	throw;
    }
}
    

const Tree_event::Value_type& Tree_event::data_as_code(
    const Key_type& variable
    ) const
{
    try
    {
	const size_t pos = var_map().right.at(variable);
	if(pos > data_.size()) return Token_map::UNKNOWN_TOKEN_VAL;
	const Value_type& result = data_[pos];
	return result;
    } catch (const std::out_of_range& oor) {
	std::cerr << "Attempted to access nonexistent variable "
		  << variable
		  << " in Tree_event "
		  << oor.what()
		  << std::endl;
	throw;
    }
}

Tree_event::Value_type& Tree_event::data_in_slot(
    const Key_type& variable
    )
{
    try
    {
	const size_t pos = var_map().right.at(variable);
	if(pos > data_.size()) throw std::out_of_range("Index out of range");
	Value_type& result = data_[pos];
	return result;
    } catch (const std::out_of_range& oor) {
	std::cerr << "Attempted to assign to nonexistent variable "
		  << variable
		  << " in Tree_event::data_in_slot() " << oor.what()
		  << std::endl;
	throw;
    }
}

};

