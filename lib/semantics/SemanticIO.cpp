/*!
 * @file SemanticIO.cpp
 *
 * @author Colin Dawson 
 * $Id: SemanticIO.cpp 17350 2014-08-21 20:30:43Z cdawson $ 
 */

#include "semantics/SemanticIO.h"
#include <boost/bind.hpp>
#include <algorithm>
#include <numeric>

using namespace std;

namespace semantics
{

//     template<>
//     string enum_to_string<Attribute_type>(Attribute_type value)
//     {
// 	switch(value)
// 	{
// 	case COLOR:
// 	    return "COLOR";
// 	case SIZE:
// 	    return "SIZE";
// 	case NUM_ATTRIBUTES:
// 	    return "NUM_ATT";
// 	case NULL_ATTRIBUTE:
// 	    return "NULL";
// 	default:
// 	    return "<Unk>";
// 	}
//     }

// /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

//     template<>
//     Attribute_type string_to_enum<Attribute_type>(string str)
//     {
// 	if(str.compare("COLOR") == 0) return COLOR;
// 	else if(str.compare("SIZE") == 0) return SIZE;
// 	else if(str.compare("NUM_ATT") == 0) return NUM_ATTRIBUTES;
// 	else if(str.compare("NULL") == 0) return NULL_ATTRIBUTE;
// 	else
// 	{
// 	    kjb_c::set_error("Unknown string encountered");
// 	    return NULL_ATTRIBUTE;
// 	}
//     }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    template<>
    string enum_to_string<Step_code_t>(Step_code_t value)
    {
    	switch(value)
    	{
    	case Step_code::NULL_STEP:
    	    return "NULL_STEP";
    	case Step_code::IDENTITY:
    	    return "IDENTITY";
    	case Step_code::HEAD:
    	    return "HEAD";
    	case Step_code::LEFT_ARG:
    	    return "TARGET";
    	case Step_code::RIGHT_ARG:
    	    return "BASE";
    	case Step_code::ATT0:
    	    return "COLOR";
    	case Step_code::ATT1:
    	    return "SIZE";
    	case Step_code::NUM_STEPS:
    	    return "NUM_STEPS";
    	default:
    	    return "<Unk>";
    	}
    }

};
