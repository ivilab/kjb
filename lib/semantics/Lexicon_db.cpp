/*!
 * @file Lexicon_db.cpp
 *
 * @author Colin Dawson 
 * $Id: Lexicon_db.cpp 16870 2014-05-22 19:34:15Z cdawson $ 
 */

#include "semantics/Lexicon_db.h"
#include <boost/shared_ptr.hpp>

namespace semantics
{
    const Lexicon_db::Key_type stop_key = "#STOP#";
    const Lexicon_db::Key_type lf_key = "+UNK+";
    const Lexicon_db::Val_type stop_code = 0;
    
    const Lexicon_db::Key_type& Lexicon_db::stop_key()
    {
	static Key_type sk = semantics::stop_key;
	return sk;
    }
    const Lexicon_db::Key_type& Lexicon_db::low_freq_key()
    {
	static Key_type lfk = semantics::lf_key;
	return lfk;
    }
    const Lexicon_db::Val_type& Lexicon_db::stop_code()
    {
	static Val_type sc = semantics::stop_code;
	return sc;
    }
    
    Lexicon_db::Lexicon_db(std::string file)
	: Token_map()
    {
	map_.insert(Key_val_pair(stop_key(), stop_code()));
	map_.insert(Key_val_pair(low_freq_key(), UNKNOWN_TOKEN_VAL));
	std::ifstream data(file.c_str());
	std::string line;
	while(getline(data, line))
	{
	    Key_type key = line.substr(0, line.find(' '));
	    encode(key, true);
	}
    }

};

