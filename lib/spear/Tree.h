#ifndef TREE_H_
#define TREE_H_

/*!
 * @file Tree.h
 *
 * @author Mihai Surdeanu
 * $Id: Tree.h 16704 2014-04-22 17:05:09Z cdawson $ 
 */

#include <iostream>
#include <string>
#include <list>

#include "spear/RCIPtr.h"
#include "spear/Wide.h"

#ifdef USE_MORPHER
#include "spear/Morpher.h"
#endif

namespace spear {

class Tree;

typedef enum { 
  PATTERN_OBJ, 
  PATTERN_IOBJ, 
  PATTERN_SUBJ, 
  PATTERN_PREP,
  PATTERN_ROBJ,
  PATTERN_RIOBJ,
  PATTERN_RSUBJ
} PatternType;

typedef enum { 
  VERB_ACTIVE, 
  VERB_PASSIVE, 
  VERB_COPULATIVE, 
  VERB_INFINITIVE, 
  VERB_GERUND 
} VerbType;

class Pattern: public RCObject
{

public:

    Pattern(
	const RCIPtr<spear::Tree> & src,
	const RCIPtr<spear::Tree> & dest,
	const PatternType & type,
	Tree * qual = NULL)
	: _source(src.operator->()), 
	  _destination(dest.operator->()), 
	  _type(type), _qualifier(qual)
    {};

    Pattern(
	Tree * src,
	Tree * dest,
	const PatternType & type,
	Tree * qual = NULL):
	_source(src), _destination(dest), 
	_type(type), _qualifier(qual) {};

    spear::Tree * getSource() const { return _source; };

    spear::Tree * getDestination() const { return _destination; };

    const PatternType & getType() const { return _type; };

    const String & getQualifierString() const;

    spear::Tree * getQualifier() const { return _qualifier; };

    void display(OStream & os) const;

    void displayType(OStream & os) const;

#ifdef DISPLAY_PROLOG
    void displayProlog(OStream & os) const;
#endif

private:

    Tree * _source;
  
    Tree * _destination;

    PatternType _type;

    Tree * _qualifier;

    static String NULL_QUALIFIER;
};

typedef RCIPtr<Pattern> PatternPtr;

class Tree : public RCObject
{

public:

    Tree() : _head(NULL), _verbType(VERB_ACTIVE), _position(-1) {};

    Tree(
	const String&  word,
	const String&  label
	): _word(word),
	   _label(label),
	   _head(NULL), 
	   _verbType(VERB_ACTIVE),
	   _position(-1)
    {};

    Tree(const String & label)
    : _label(label),
      _head(NULL), 
      _verbType(VERB_ACTIVE),
      _position(-1)
    {};

    const String& getWord() const { return _word; };

    void setWord(const String& w) { _word = w; };

    const String& getLabel() const { return _label; };  

    void setLabel(const String& label) { _label = label; };

    const RCIPtr<spear::Tree>& getHead() const { return _head; };

    const String& getHeadWord() const;

    short getHeadPosition() const;

    const String& getHeadNe() const;

    /** Returns the POS tag associated with the head word */
    const String& getHeadTag() const;

    void setHead(const RCIPtr<spear::Tree>& h) { _head = h; };

    void setLemma(const String& lemma) { _lemma = lemma; };
  
#ifdef USE_MORPHER
    /** Not const because lemmas are detected on the fly */
    const String & getHeadLemma();

    /** Not const because lemmas are detected on the fly */
    const String & getLemma();
#endif

    bool isTerminal() const
    { 
	return (_children.empty() == true && _word.empty() == false);
    };

    void addChild(
	const RCIPtr<spear::Tree>&  c,
	bool                        atEnd = true)
    { 
	if(atEnd == true)_children.push_back(c); 
	else _children.push_front(c);
    };

    const std::list< RCIPtr<spear::Tree> >& getChildren() const 
    { return _children; };

    /** 
     * Returns non-const children list
     * Note: Use with care! Update _head if children are changed!
     */
    std::list< RCIPtr<spear::Tree> >& children() 
    { return _children; };

    void addPattern(const RCIPtr<spear::Pattern> & p) 
    { _patterns.push_back(p); };

    const std::list< RCIPtr<spear::Pattern> > & getPatterns() const
    { return _patterns; };

    bool hasPatterns() const;

    const spear::VerbType & getVerbType() const { return _verbType; };

    void setVerbType(const spear::VerbType & vt) { _verbType = vt; };

    void setNe(const String & ne) { _ne = ne; };

    const String & getNe() const { return _ne; };

    void clear()
    {
	_word = W("");
	_lemma = W("");
	_label = W("");
	_children.clear();
	_head = RCIPtr<spear::Tree>();
	_patterns.clear();
    };

    void display(
	OStream&  os,
	bool      isHead = false, 
	int       offset = 0
	) const;

    void displayParens(
	OStream& os,
	bool     showHead = true
	) const;

    void displayPrettyParens(
	OStream& os,
	int      offset = 0,
	bool     showHead = false
	) const;

    /** Display the tree as a Prolog predicate */
    void displayProlog(
	OStream& os,
	int index
	) const;

    void displayPatterns(
	OStream& os,
	int offset = 0
	) const;

    /** 
     * Sets the _position fields for all terminal edges in this phrase 
     * The first token gets position 1
     * Returns the number of terminals in this sentence
     */
    short setPositions();

    short getPosition() const { return _position; };

#ifdef USE_MORPHER
    static void registerMorpher(const RCIPtr<spear::Morpher> & m)
    { _morpher = m; };
#endif

    typedef std::list< RCIPtr<spear::Tree> >::iterator iterator;

    typedef std::list< RCIPtr<spear::Tree> >::const_iterator const_iterator;

private:
    /** used by displayProlog */
    void displayPrologPhrase(OStream & os) const;

    /** 
     * used by displayProlog 
     * Positions must be set before this method is called
     */
    void displayPrologPatterns(OStream & os,
			       bool & printComma) const;

    /** used by setPositions */
    void setPosition(short & current);

private:

    Tree(const spear::Tree&);
  
    String _word;

    String _lemma;

    String _label;

    String _ne;

    RCIPtr<spear::Tree> _head;

    std::list< RCIPtr<spear::Tree> > _children;

    std::list< RCIPtr<spear::Pattern> > _patterns;

    spear::VerbType _verbType;

    /**
     * Position in the input token list
     * By default, this value is not set. Call setPositions() to set it
     */
    short _position;

#ifdef USE_MORPHER
    static RCIPtr<spear::Morpher> _morpher;
#endif
};

typedef RCIPtr<spear::Tree> TreePtr;

}

OStream& operator<<(OStream& os, const spear::TreePtr& tree);

OStream& operator<<(OStream& os, const spear::Tree& tree);

#endif
