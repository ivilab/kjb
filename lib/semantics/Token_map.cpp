/*!
 * @file Token_map.cpp
 *
 * @author Colin Dawson 
 * $Id: Token_map.cpp 16870 2014-05-22 19:34:15Z cdawson $ 
 */

#include "semantics/Token_map.h"
#include <map>

namespace semantics
{
    Token_map::Val_type Token_map::get_code(const Token_map::Key_type& key)
    {
	Map::left_iterator it = map_.left.find(key);
	if(it == map_.left.end()) return UNKNOWN_TOKEN_VAL;
	else return it -> second;
    }
    
    Token_map::Val_type Token_map::encode(
	const Token_map::Key_type& key,
	bool                       learn)
    {
	Val_type code = get_code(key);
	if(learn && code == UNKNOWN_TOKEN_VAL)
	{
	    code = next_val_;
	    map_.insert(Key_val_pair(key, next_val_++));
	}
	return code;
    }

    const Token_map::Key_type& Token_map::decode(const Token_map::Val_type& val)
    {
	Map::right_iterator it = map_.right.find(val);
	if(it == map_.right.end()) return unknown_key();
	else return map_.right.at(val);
    }

    std::ostream& operator<<(std::ostream& os, Token_map& map)
    {
	os << "Keys:\tValues:" << std::endl;
	for(Token_map::Map::iterator it = map.map_.begin();
	    it != map.map_.end();
	    it++)
	{
	    os << it->left << "\t" << it->right << std::endl;
	}
	return os;
    }

    const Token_map::Val_type Token_map::UNKNOWN_TOKEN_VAL = 1;
};
