/*!
 * @file EdgeLexer.cc
 *
 * @author Mihai Surdeanu
 * $Id$ 
 */

#include <sstream>

#include "spear/EdgeLexer.h"

using namespace std;
using namespace spear;

EdgeLexer::EdgeLexer(IStream & stream)
  : _stream(stream), _lineCount(1)
{
}

bool EdgeLexer::isSpace(Char c) const
{
  if(c != W(' ') &&
     c != W('\t') &&
     c != W('\n') &&
     c != W('\r')){
    return false;
  }

  return true;
}

void EdgeLexer::skipWhiteSpaces()
{
  Char c;
  while((c = _stream.get()) != EOF && isSpace(c)){
    if(c == W('\n')){
      _lineCount ++;
    }
  }
  _stream.unget();
}

#define STRING_CHAR(c) ( \
  c != W('(') &&  \
  c != W(')') &&  \
  ! isSpace(c)    \
)

int EdgeLexer::lexem(String& text)
{   // skips white space
    skipWhiteSpaces();

    // reads in a single character
    Char c = _stream.get();

    // detects whether it is a paren, 
    // another character, or an EOL.
    if(c == EOF) return TOKEN_EOF;
    else if(c == W('(')) return TOKEN_LP;
    else if(c == W(')')) return TOKEN_RP;
    else if(STRING_CHAR(c))
    {
	OStringStream buffer;
	buffer << c;
	// if a character is read, keep reading in characters
	// and output to text
	while((c = _stream.get()) != TOKEN_EOF && STRING_CHAR(c))
	{
	    buffer << c;
	}
	text = buffer.str();
	_stream.unget();
	return TOKEN_STRING;
    }

    // should never get here
    return TOKEN_EOF;
}

/*
#include <fstream>

int main()
{
  wifstream is("/home/mihai/tmp/b.txt");
  EdgeLexer lex(is);
  wstring text;
  int l;

  while((l = lex.lexem(text)) != EdgeLexer::TOKEN_EOF){
    wcout << l;
    if(l == EdgeLexer::TOKEN_STRING){
      // wcout << " " << text;
    }
    wcout << endl;
  }
}
*/
