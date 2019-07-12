#ifndef SEMSPEAR_TREE_PARSER_H_
#define SEMSPEAR_TREE_PARSER_H_

/*!
 * @file Semspear_tree_parser.h
 *
 * @author Colin Dawson 
 * $Id: Semspear_tree_parser.h 17024 2014-06-17 01:47:45Z cdawson $ 
 */

#include "semantics/Lexicon_db.h"
#include "semantics/Nonterminal_db.h"
#include "spear/Pair.h"
#include "spear/EdgeLexer.h"
#include "spear/Lexicon.h"
#include "spear/CharUtils.h"
#include "spear/Exception.h"
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <list>
#include <string>


namespace semantics {

/**
 * Parses an edge of type T. 
 * The type T must have these methods: set_label(), set_word(), add_child(),
 * and optionally set_head() to enable functionality for reading in head
 * marker indices from the input stream.  T must also have a public container
 * of children or child pointers, accessible with a public children() method.
 * T should have a complete_tree() method to signal that tree is built
 */
    template <class T>
    class Semspear_tree_parser
    {
    public:
	typedef std::istream Input_stream;
	typedef std::string Token_contents;
	typedef int Token_code;
	typedef boost::tuple<Token_contents, Token_code> Token_tuple;
	typedef std::list<Token_tuple> Tuple_stack;
	typedef boost::shared_ptr<T> Constituent_ptr;
    public:
	static Lexicon_db& lexicon() {return T::lexicon();}
	static Nonterminal_db& nt_lexicon() {return T::nt_lexicon();}
	static bool VERBOSE;
    public:
	/*! @brief construct a parser object with text input 'istream'
	 *  @param tree_stream an input stream containing trees to be parsed
	 *  @param lexicon_stream an input stream containing words and freq flags
	 *  @param learn governs whether to update model using this tree
	 */
	Semspear_tree_parser(
	    Input_stream& tree_stream,
	    Input_stream& lexicon_stream,
	    bool          learn = true);

	/*! @brief parse the next edge in the stream
	 *  @return a pointer to an edge object of type T
	 */
	Constituent_ptr parse_constituent(bool at_root = true);
	
    private:
	spear::EdgeLexer lexer_; /*!< The lexer */

	/*! @brief read in words from a lexicon file
	 */
	void initialize_lexicon(Input_stream& input);
	
	/*! @brief process the next token in the stream
	 *  @return a tuple containing a type code and the text content
	 */
	Token_tuple lexem();

	/*! @brief put (text,lex) pair back on the buffer_ stack
	 */
	void unget(Token_contents& text, Token_code lex);
	
	Tuple_stack buffer_; /*!< the unget stack */
	bool learn_; /*!< incorporate constituents into running counts? */

    };

    /// Implementation of member functions
    
    template<class T> Semspear_tree_parser<T>::Semspear_tree_parser(
	Semspear_tree_parser<T>::Input_stream&  syn_stream,
	Semspear_tree_parser<T>::Input_stream&  lex_stream,
	bool                                    learn
	) : lexer_(syn_stream), learn_(learn)
    {
	initialize_lexicon(lex_stream);
    };

    template<class T>
    typename Semspear_tree_parser<T>::Constituent_ptr
    Semspear_tree_parser<T>::parse_constituent(bool at_root)
    {
	Token_contents contents;
	Token_code code;
	    
	// return value (lex) is code for what type of token
	// (paren, EOL or content string);
	// if content encountered, writes to text.
	// lexem() is defined in EdgeLexer.cc
	boost::tie(contents, code) = lexem();

	// end of file reached
	if(code == spear::EdgeLexer::TOKEN_EOF)
	{
	    if(VERBOSE)
	    {
		std::cerr << "parse_constituent() encountered EOF"
			  << std::endl;
	    }
	    return Constituent_ptr();
	}

	// should start with left paren
	if(code != spear::EdgeLexer::TOKEN_LP)
	{
	    throw(
		spear::Exception(
		    "Syntax error: Left parenthesis expected", 
		    lexer_.getLineCount()
		    )
		);
	}

	// if so, create an edge pointer
	Constituent_ptr result(new T(at_root ? T::TOP : T::DEPENDENCY, learn_));

	// read in the next token
	boost::tie(contents,code) = lexem();

	// Phrase label
	// (have already found left paren, so next
	// string is a phrase label)

        // 'contents' contains the phrase label,
	// so attach this label to the edge object
	result -> set_label(nt_lexicon().encode(contents, learn_));
	// read in the next token
	boost::tie(contents, code) = lexem();

	// This is a non-terminal phrase
	// (if not, should encounter another string,
	// not a left paren)
	if(code == spear::EdgeLexer::TOKEN_LP)
	{
	    // The head position might be specified immediately
	    // after the children
	    int head_position = -1;

	    // Parse all children
	    while(code == spear::EdgeLexer::TOKEN_LP)
	    {
		// put the node on the stack
		unget(contents, code); 
		// recursively parse the nonterminal below
		Constituent_ptr child = parse_constituent(false);
		// once parsed, add the subtree as a child
		// of this node
		result -> add_child(child);
		// get the next token
		boost::tie(contents, code) = lexem();
	    }

	    // This token might be the head position
	    if(code == spear::EdgeLexer::TOKEN_STRING &&
	       (head_position = spear::toInteger(contents)) >= 0 &&
	       head_position < (int) result -> children().size() )
	    {
		// Set the head
		for(typename T::Child_list::const_iterator it =
			result -> children().begin();
		    it != result -> children().end();
		    it++, head_position--)
		{
		    // Found the head child
		    if(head_position == 0)
		    {
			result -> set_head(it);
			break;
		    }
		}
	    } else unget(contents, code);

	    // Phrase word for terminal phrases
	} else if(code == spear::EdgeLexer::TOKEN_STRING) {
	    result -> set_word(lexicon().encode(contents, learn_));
	} else {
	    throw(
		spear::Exception(
		    "Syntax error: Left parenthesis or string expected", 
		    lexer_.getLineCount()));
	}
	
	boost::tie(contents, code) = lexem();
	if(code != spear::EdgeLexer::TOKEN_RP)
	{							\
	    throw spear::Exception(
		"Syntax error: Right parenthesis expected",
		lexer_.getLineCount()
		);
	}

	if(at_root)
	{
	    if(VERBOSE) std::cerr << "Setting events." << std::endl;
	    result -> preprocess_tree();
	    result -> complete_tree();
	    if(VERBOSE)
	    {
		std::cerr << "Resulting tree is:" << std::endl;
		result -> print_dependency_tree(std::cerr);
		// std::cerr << "Done setting events. Remaining input is ";
		// lexer_.print_stream(std::cerr);
	    }
	}

	return result;
    }

    template<class T> 
    typename Semspear_tree_parser<T>::Token_tuple
    Semspear_tree_parser<T>::lexem() 
    {
	if(buffer_.empty() == true)
	{
	    // call EdgeLexer lexem method
	    // once the buffer is empty
	    // NOTE: contents gets written to by lexer_.lexem
	    Token_contents contents;
	    Token_code code = lexer_.lexem(contents);
	    return boost::tie(contents, code);
	}

	// buffer stack contains pairs whose first
	// element is a string, and whose second is a
	// code denoting the element type
	
	// last is at the top of the buffer stack
	Token_tuple last = buffer_.back();
	buffer_.pop_back();
	
	return last;
    }

    template<class T>
    void Semspear_tree_parser<T>::unget(
	Semspear_tree_parser<T>::Token_contents& contents,
	Semspear_tree_parser<T>::Token_code      code
	)
    {
	buffer_.push_back(Token_tuple(contents, code));
    }

    template<class T>
    void Semspear_tree_parser<T>::initialize_lexicon(Input_stream& input)
    {
	std::string word, tag;
	bool is_hi_freq;
	std::string line_string;

	while(std::getline(input, line_string))
	{
	    std::istringstream line(line_string);
	    line >> word;
	    line >> tag;
	    line >> is_hi_freq;
	    Token_map::Val_type wcode = lexicon().encode(word, true);
	    T::lf_word_map()[wcode] = is_hi_freq;
	}

    }
    

    /// Static initialization

    template<class T>
    bool Semspear_tree_parser<T>::VERBOSE = false;

} // end namespace spear

#endif
