#ifndef SEM_LEXER_H_
#define SEM_LEXER_H_

/*!
 * @file SemLexer.h
 *
 * @author Colin Dawson 
 * $Id: SemLexer.h 16866 2014-05-22 07:03:05Z cdawson $ 
 */

#include <iostream>
#include <string>

#include "semantics/SemanticIO.h"
#include "spear/Wide.h"
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

namespace semantics {

    class Sem_lexer 
    {
    public:

	enum Token_code
	{
	    TOKEN_EOF,
	    TOKEN_EOL,
	    TOKEN_STRING,
	    TOKEN_LB,
	    TOKEN_RB,
	    TOKEN_EQUALS,
	    TOKEN_COLON
	};
  
	// Initialize with an input stream
	inline Sem_lexer(std::istream& stream) :
	     input_stream_(stream)
	{
	}

	// reads a token and returns a token code as well as the contents
	boost::tuple<Sem_lexer::Token_code, String> get_and_classify_token();

    private:

	/** The stream */
	std::istream& input_stream_;

	template<typename T>
	T get_semantic_token_as(const String &input)
	{
	    return enum_to_string<T>(input);
	}

	/** Advance over white spaces */
	void skip_white_spaces();

	inline bool is_space(Char c) const
	{
	    return !(c != W(' ') && c != W('\t'));
	}
		
	inline bool is_newline(Char c) const
	{
	    return !(c != W('\n') && c != W('\r'));
	}
    };

} // end namespace spear

#endif
