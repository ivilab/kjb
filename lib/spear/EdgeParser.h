#ifndef EDGE_PARSER_H_
#define EDGE_PARSER_H_

/*!
 * @file EdgeParser.h
 *
 * @author Mihai Surdeanu
 * $Id: EdgeParser.h 16468 2014-02-24 00:30:36Z cdawson $ 
 */

#include <iostream>
#include <list>
#include <string>

#include "spear/Pair.h"
#include "spear/EdgeLexer.h"
#include "spear/Lexicon.h"
#include "spear/CharUtils.h"
#include "spear/Exception.h"

#define EXTRACT_TOKEN(type, msg){ \
  if(lexem(text) != type){ \
    throw spear::Exception(msg, lexer_.getLineCount()); \
  } \
}

namespace spear {

/**
 * Parses an edge of type T. 
 * The type T must have these methods: setLabel(), setWord(), addChild(),
 * and optionally setHead() to enable functionality for reading in head
 * marker indices from the input stream
 */
    template <class T>
    class EdgeParser 
    {
    public:
	/*! @brief construct a parser object with text input 'syn_stream'
	 */
	EdgeParser(IStream& syn_stream);

	/*! @brief parse the next edge in the stream
	 *  @return a pointer to an edge object of type T
	 */
	spear::RCIPtr<T> parseEdge();
	
    private:
  
	spear::EdgeLexer lexer_; /*!< The lexer */

	/*! @brief process the next token in the stream
	 *  @param text a string reference to write contents of token
	 *  @return integer code for the token type
	 *  @sideefect writes token contents to param 'text'
	 */
	int lexem(String& text);

	/*! @brief put (text,lex) pair back on the buffer_ stack
	 */
	void unget(String& text, int lex);
	
	/*! @brief the unget stack */
	std::list< spear::Pair<String, int> > buffer_;

    };

    /// Implementation of member functions
    
    template<class T> EdgeParser<T>::EdgeParser(
	IStream& syn_stream
	) : lexer_(syn_stream)
    {
    };

    template<class T>
    spear::RCIPtr<T> EdgeParser<T>::parseEdge()
    {
	String text;
	int lex;
	    
	// return value (lex) is code for what type of token
	// (paren, EOL or content string);
	// if content encountered, writes to text.
	// lexem() is defined in EdgeLexer.cc
	lex = lexem(text);

	// end of file reached
	if(lex == EdgeLexer::TOKEN_EOF)
	{
	    return spear::RCIPtr<T>();
	}
    
	// should start with left paren
	if(lex != spear::EdgeLexer::TOKEN_LP)
	{
	    throw(
		Exception(
		    "Syntax error: Left parenthesis expected", 
		    lexer_.getLineCount()
		    )
		);
	}

	// if so, create an edge pointer
	spear::RCIPtr<T> edge(new T);

	// read in the next token
	lex = lexem(text);

	// Phrase label
	// (have already found left paren, so next
	// string is a phrase label)
	if(lex == spear::EdgeLexer::TOKEN_STRING)
	{
	    // text contains the phrase label,
	    // so attach this label to the edge object
	    edge -> setLabel(text);
	    // read in the next token
	    lex = lexem(text);
	} else
	{
	    // after a left paren, the only time a string
	    // is not encountered is at the root
	    edge -> setLabel("TOP");
	}

	// This is a non-terminal phrase
	// (if not, should encounter another string,
	// not a left paren)
	if(lex == spear::EdgeLexer::TOKEN_LP)
	{
	    // The head position might be specified immediately
	    // after the children
	    int headPosition = -1;

	    // Parse all children
	    while(lex == spear::EdgeLexer::TOKEN_LP)
	    {
		// put the node on the stack
		unget(text, lex); 
		// recursively parse the nonterminal below
		spear::RCIPtr<T> child = parseEdge();
		// once parsed, add the subtree as a child
		// of this node
		edge -> addChild(child);
		// get the next token
		lex = lexem(text);
	    }

	    // This token might be the head position
	    if(lex == spear::EdgeLexer::TOKEN_STRING &&
	       (headPosition = toInteger(text)) >= 0 &&
	       headPosition < (int) edge->getChildren().size())
	    {
		// Set the head
		for(typename std::list< spear::RCIPtr<T> >::
			const_iterator it = edge->getChildren().begin();
		    it != edge->getChildren().end();
		    it ++, headPosition --)
		{
		    // Found the head child
		    if(headPosition == 0)
		    {
			edge->setHead(*it);
			break;
		    }
		}
	    } else unget(text, lex);

	    // Phrase word for terminal phrases
	} else if(lex == spear::EdgeLexer::TOKEN_STRING) {
	    edge->setWord(text);
	} else {
	    throw(
		spear::Exception(
		    "Syntax error: Left parenthesis or string expected", 
		    lexer_.getLineCount()));
	}

	EXTRACT_TOKEN(
	    spear::EdgeLexer::TOKEN_RP, 
	    "Syntax error: Right parenthesis expected");

	return edge;
    }

    template<class T> 
    int EdgeParser<T>::lexem(String& text) 
    {
	if(buffer_.empty() == true)
	{
	    // call EdgeLexer lexem method
	    // once the buffer is empty
	    return lexer_.lexem(text);
	}

	// buffer stack contains pairs whose first
	// element is a string, and whose second is a
	// code denoting the element type
	
	// last is at the top of the buffer stack
	spear::Pair<String, int> last = buffer_.back();
	buffer_.pop_back();
	
	// text is the actual string
	text = last.getFirst();
	// return value is code for type of string
	return last.getSecond();
    }

    template<class T>
    void EdgeParser<T>::unget(String& text, int lex)
    {
	buffer_.push_back(spear::Pair<String, int>(text, lex));
    }



    
} // end namespace spear

#endif
