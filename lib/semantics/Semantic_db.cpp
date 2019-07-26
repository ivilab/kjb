/*!
 * @file Semantic_db.cpp
 *
 * @author Colin Dawson 
 * $Id: Semantic_db.cpp 16870 2014-05-22 19:34:15Z cdawson $ 
 */

#include "semantics/Semantic_db.h"
#include <fstream>

namespace semantics
{
    const Semantic_db::Key_type null_key = "NULL";
    const Semantic_db::Key_type unknown_key = "<UNK>";

    const Semantic_db::Key_type& Semantic_db::null_key()
    {
	static Key_type nk = semantics::null_key;
	return nk;
    }

    const Semantic_db::Key_type& Semantic_db::unknown_entity()
    {
	static Key_type uk = semantics::unknown_key;
	return uk;
    }

    Semantic_db::Semantic_db() : Token_map()
    {
	map_.insert(Key_val_pair(null_key(), 0));
	map_.insert(Key_val_pair(unknown_key(), UNKNOWN_TOKEN_VAL));
    }
    
    void Semantic_db::initialize_from_file(std::string file)
    {
	std::ifstream data(file.c_str());
	std::string line;
	while(getline(data, line))
	{
	    Key_type key = line.substr(0, line.find(' '));
	    encode(key, true);
	}
    }
}
