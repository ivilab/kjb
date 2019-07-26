#ifndef EDGELEXER_H_
#define EDGELEXER_H_

/*!
 * @file EdgeLexer.h
 *
 * @author Mihai Surdeanu
 * $Id: EdgeLexer.h 16731 2014-05-02 00:26:52Z cdawson $ 
 */

#include <iostream>
#include <string>

#include "spear/Wide.h"


namespace spear {

class EdgeLexer 
{
public:

    static const int TOKEN_EOF = 0;
    static const int TOKEN_STRING = 1;
    static const int TOKEN_LP = 2;
    static const int TOKEN_RP = 3;
  
    EdgeLexer(IStream &);

    int lexem(String &);

    int getLineCount() const { return _lineCount; };

    void print_stream(std::ostream& os) const {os << _stream.rdbuf();}

private:
  
    /** The stream */
    IStream & _stream;

    /** Line count */
    int _lineCount;
  
    /** Advance over white spaces */
    void skipWhiteSpaces();

    bool isSpace(Char c) const;
};

} // end namespace spear

#endif
