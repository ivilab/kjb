/* $Id: SemanticIO.h 17350 2014-08-21 20:30:43Z cdawson $ */
   
#ifndef SEMANTIC_IO_H_
#define SEMANTIC_IO_H_

/*!
 * @file SemanticIO.h
 *
 * Miscellaneous enum definitions pertaining to Elaboration_tree
 *
 * @author Colin Dawson 
 */

#include "l_cpp/l_exception.h"
#include "prob_cpp/prob_weight_array.h"
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <string>
#include <iostream>
#include <numeric>

namespace semantics
{
    template<typename Enum_type>
    std::string enum_to_string(Enum_type)
    {
	return "<Unknown enum type>";
    }


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    template<typename Enum_type>
    Enum_type string_to_enum(std::string)
    {
	return static_cast<Enum_type>(0);
    }

// /* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

//     enum Attribute_type
//     {
// 	COLOR,           //0
// 	SIZE,            //1
// 	NUM_ATTRIBUTES,  //2
// 	NULL_ATTRIBUTE   //3
//     };


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 
    
    namespace Step_code
    {
	enum Code
	{
	    NULL_STEP,    //0
	    IDENTITY,     //1
	    HEAD,         //2
	    LEFT_ARG,     //3
	    RIGHT_ARG,    //4
	    ATT0,         //5
	    ATT1,         //6
	    NUM_STEPS     //7
	};

	typedef kjb::Weight_array<NUM_STEPS> Weights;
	typedef boost::array<Code, NUM_STEPS> Val_array;
	typedef Weights::Filter Filter_array;

	const Val_array codes =
	{{NULL_STEP, IDENTITY, HEAD, LEFT_ARG, RIGHT_ARG, ATT0, ATT1}};

    };

    typedef Step_code::Code Step_code_t;

    // template<> std::string enum_to_string<Attribute_type>(Attribute_type);
    template<> std::string enum_to_string<Step_code_t>(Step_code_t);
    
    // template<> Attribute_type string_to_enum<Attribute_type>(std::string);
};
    
#endif
