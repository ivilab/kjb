/*!
 * @file Nonterminal_db.cpp
 *
 * @author Colin Dawson 
 * $Id: Nonterminal_db.cpp 16870 2014-05-22 19:34:15Z cdawson $ 
 */

#include "semantics/Nonterminal_db.h"
#include <fstream>

namespace semantics
{
    const Nonterminal_db::Key_type stop_key = "#STOP#";
    const Nonterminal_db::Key_type low_freq_key = "+UNK+";
    const Nonterminal_db::Key_type root_key = "TOP";
    const Nonterminal_db::Val_type root_code = 2;

    const Nonterminal_db::Key_type& Nonterminal_db::stop_key()
    {
	static Nonterminal_db::Key_type sk = semantics::stop_key;
	return sk;
    }
    const Nonterminal_db::Key_type& Nonterminal_db::low_freq_key()
    {
	static Nonterminal_db::Key_type lfk = semantics::low_freq_key;
	return lfk;
    }
    const Nonterminal_db::Key_type& Nonterminal_db::root_key()
    {
	static Nonterminal_db::Key_type rk = semantics::root_key;
	return rk;
    }
    const Nonterminal_db::Val_type& Nonterminal_db::root_code()
    {
	static Nonterminal_db::Val_type rc = semantics::root_code;
	return rc;
    }
    
    Nonterminal_db::Nonterminal_db(std::string file)
	: Token_map()
    {
	// #STOP# has key 0
	map_.insert(Key_val_pair(stop_key(), 0));
	map_.insert(Key_val_pair(low_freq_key(), UNKNOWN_TOKEN_VAL));
	map_.insert(Key_val_pair(root_key(), root_code()));
	next_val_ = root_code() + 1;
	std::ifstream data(file.c_str());
	Key_type token;
	while(data >> token)
	{
	    encode(token, true);
	}
    }
};

