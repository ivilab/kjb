/*!
 * @file SemLexer.cpp
 *
 * @author Colin Dawson 
 * $Id: SemLexer.cpp 16866 2014-05-22 07:03:05Z cdawson $ 
 */

#include <sstream>

#include "semantics/SemLexer.h"

using namespace std;
using namespace semantics;
    

void Sem_lexer::skip_white_spaces()
{
  Char c;
  while((c = input_stream_.get()) != EOF && is_space(c))
  {
    // if(c == W('\n')){
    //   _line_count ++;
  }
  input_stream_.unget();
}

#define IS_STRING_CHAR(c) (			\
    c != W('[') &&				\
    c != W(']') &&				\
    c != W('=') &&				\
    c != W(':') &&				\
    ! is_space(c) &&				\
    ! is_newline(c)				\
)

boost::tuple<Sem_lexer::Token_code, string> Sem_lexer::get_and_classify_token()
{
    // skips white space
    skip_white_spaces();

    // reads in a single character
    Char c = input_stream_.get();
    Token_code token_code;
    string token = "";
    // detects whether it is a bracket, 
    // a content character, or a delimiter
    if(c == EOF) 
    {
	token_code = TOKEN_EOF;
    } else if(is_newline(c))
    {
	token_code = TOKEN_EOL;
    } else if(c == W('[')) 
    {
	token_code = TOKEN_LB;
    } else if(c == W(']')) 
    {
	token_code = TOKEN_RB;
    } else if(c == W('=')) // used to specify attributes
    {
	token_code = TOKEN_EQUALS;
    } else if(c == W(':')) // delimits node types and semantic labels
    {
	token_code = TOKEN_COLON;
    } else if(IS_STRING_CHAR(c)) // content character
    {
	OStringStream buffer;
	buffer << c;
	// if a character is read, keep reading in characters
	// and output to text until either an EOF or a non-character
	// is read
	while((c = input_stream_.get()) != TOKEN_EOF && IS_STRING_CHAR(c))
	{
	    buffer << c;
	}
	token = buffer.str();
	input_stream_.unget();
	token_code = TOKEN_STRING;
    } else
    {
	// should never get here
	token_code = TOKEN_EOF;
    }
    return boost::make_tuple(token_code, token);
}

