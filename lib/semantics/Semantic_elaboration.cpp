/*!
 * @file Semantic_elaboration.cpp
 *
 * @author Colin Dawson 
 * $Id: Semantic_elaboration.cpp 17322 2014-08-18 20:54:36Z cdawson $ 
 */

#include "semantics/Semantic_elaboration.h"
#include <iostream>

namespace semantics
{
    void initialize_global_semantic_maps()
    {
	std::cerr << "Initializing global semantic head map...";
	Semantic_data_base::global_head_map().insert(
	    Semantic_data_base::Map_entry(0, "NULL"));
	std::cerr << "done." << std::endl;
	std::cerr << "Initializing global semantic args map...";
	Semantic_data_base::global_arg_map().insert(
	    Semantic_data_base::Map_entry(0, "()"));
	std::cerr << "done." << std::endl;
    };
    
    std::ostream& operator<<(
	std::ostream&                  os,
	Semantic_data_base::Hash_pair  hash_pair
	)
    {
	os << "(" << hash_pair.get<0>() << "," << hash_pair.get<1>() << ")";
	return os;
    }

    std::ostream& operator<<(
	std::ostream&                    os,
	Semantic_elaboration::Self_ptr   elab
	)
    {
	elab->print(os);
	return os;
    }

    /*------------------------------------------------------------
     * STATIC INITIALIZATION
     *------------------------------------------------------------*/

    const Semantic_elaboration::Referent_code Semantic_elaboration::NULL_REFERENT = -1;

};











