/*!
 * @file Tree.cc
 *
 * @author Mihai Surdeanu
 * $Id$ 
 */

#include "spear/Tree.h"
#include "spear/Assert.h"

using namespace std;
using namespace spear;

#ifdef USE_MORPHER
//
// The morpher object (typically WordNet) is used to generate the lemmas
//
RCIPtr<spear::Morpher> Tree::_morpher;
#endif

// Value assigned to NULL qualifiers
String Pattern::NULL_QUALIFIER = "";

void Tree::display(
    OStream & os,
    bool isHead, 
    int offset
    ) const
{
    for(int i = 0; i < offset; i ++) os << W(" ");
    os << getLabel();

    if(isTerminal())
    {
	os << W(" ") << getWord();
	if(isHead) os << W(" *");
    } else {
	if(isHead) os << W(" *");
	for(list<TreePtr>::const_iterator it = _children.begin();
	    it != _children.end(); it ++)
	{
	    os << endl;
	    isHead = false;
	    if(_head == (* it)) isHead = true;
	    (* it)->display(os, isHead, offset + 4);
	}
    }
}

OStream& operator<<(OStream& os, const spear::TreePtr& tree)
{
    tree->display(os);
    os << endl;
    tree->displayPatterns(os);
    return os;
}

OStream& operator<<(OStream& os, const spear::Tree& tree)
{
    tree.display(os);
    os << endl;
    tree.displayPatterns(os);
    return os;
}

void Tree::displayParens(
    OStream&   os,
    bool       showHead
    ) const
{
    os << W("(") << getLabel();

    if(isTerminal())
    {
	os << W(" ") << getWord();
    } else {
	for(list<TreePtr>::const_iterator it = _children.begin();
	    it != _children.end();
	    it ++)
	{
	    os << W(" ");
	    (* it) -> displayParens(os, showHead);
	}

	// display the head position if set
	if(showHead == true && getHead() != (const Tree *) NULL)
	{
	    int position = 0;
	    for(list<TreePtr>::const_iterator it = _children.begin();
		it != _children.end(); it ++, position ++)
	    {
		if((* it) == getHead())
		{
		    os << W(" ") << position;
		    break;
		}
	    }
	}
    }
    os << W(")");
}

void Tree::displayPrettyParens(
    OStream& os,
    int      offset,
    bool     showHead
    ) const
{
    for(int i = 0; i < offset; i ++) os << W(" ");
    os << W("(") << getLabel();
  
    if(isTerminal()){
	os << W(" ") << getWord();
    } else {
	// display the head position if set
	if(showHead == true && getHead() != (const Tree *) NULL)
	{
	    int position = 0;
	    for(list<TreePtr>::const_iterator it = _children.begin();
		it != _children.end(); it ++, position ++)
	    {
		if((* it) == getHead())
		{
		    os << "_" << position;
		    break;
		}
	    }
	}

	os << endl;
	for(list<TreePtr>::const_iterator it = _children.begin();
	    it != _children.end(); it ++)
	{
	    (* it)->displayPrettyParens(os, offset + 4, showHead);
	    os << endl;
	}

	for(int i = 0; i < offset; i ++) os << W(" ");
    }
  
    os << W(")");
}

#ifdef DISPLAY_PROLOG

void Tree::displayProlog(
    OStream& os,
    int      index
    ) const
{
    // display tree
    os << "tacat(" << index << ", [";
    displayPrologPhrase(os);
    os << "]";
  
    // display relations
    os << ", [";
    bool comma = false;
    displayPrologPatterns(os, comma);
    os << "]).";
}

/**
 * Normalizes strings for a prolog representation
 * Everything is lower case because upper case is used for don't know what in P
 * "-" is changed to "_" because "-" is the MINUS operator in Prolog
 * we add a \ before double quotes
 */

static String normalizePrologString(const String & s)
{
    String lower = toLower(s);
    ostringstream os;

    for(int i = 0; i < (int) lower.size(); i ++)
    {
	if(lower[i] == '-')
	{
	    os << '_';
	} else if(lower[i] == '"') {
	    os << '\\' << lower[i];
	} else {
	    os << lower[i];
	}
    }
    return os.str();
}

static String normalizePrologLabel(const String & s)
{
    String lower = toLower(s);

    ostringstream os;

    for(int i = 0; i < (int) lower.size(); i ++)
    {
	if(lower[i] == '-')
	{
	    os << '_';
	} else if(lower[i] == '"') {
	    os << '\\' << lower[i];
	} else {
	    os << lower[i];
	}
    }
  
    lower = os.str();

    // avoid some special chars in labels ($ and #)
    if(lower == "prp$")
    {
	lower = "prp1";
    } else if(lower == "wp$") {
	lower = "wp1";
    } else if(lower == "$") {
	lower = "dollar";
    } else if(lower == "#") {
	lower = "pound";
    }

    return lower;
}

void Tree::displayPrologPhrase(OStream & os) const
{
  if(isTerminal()){
    os << "cm(\"" << normalizePrologString(getWord()) << "\", " 
       << normalizePrologLabel(getLabel()) << ")";
  } else{
    os << "cs(" << normalizePrologLabel(getLabel());
    int position = 0;
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++, position ++){
      if((* it) == getHead()){
	break;
      }
    }
    os << "_" << position << "), op";
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      os << ", ";
      (* it)->displayPrologPhrase(os);
    }
    os << ", tp";
  }
}

void Tree::displayPrologPatterns(OStream & os,
				 bool & printomma) const
{
  for(list<PatternPtr>::const_iterator it = _patterns.begin();
      it != _patterns.end(); it ++){
    if(printComma == false){
      printComma = true;
    } else{
      os << ", ";
    }

    (* it)->displayProlog(os);
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->displayPrologPatterns(os, printComma);
  }
}
#endif

void Pattern::displayType(OStream & os) const  
{
  switch(_type){
  case PATTERN_OBJ: os << W("obj"); break;
  case PATTERN_IOBJ: os << W("iobj"); break;    
  case PATTERN_SUBJ: os << W("subj"); break;
  case PATTERN_PREP: os << W("prep"); break;
  case PATTERN_ROBJ: os << W("robj"); break;
  case PATTERN_RIOBJ: os << W("riobj"); break;    
  case PATTERN_RSUBJ: os << W("rsubj"); break;
  default: os << W("unknown"); break;
  }
}

void Pattern::display(OStream & os) const  
{
  os << _source->getHeadWord() << W(" ") ;

  displayType(os);
  if(_qualifier != NULL){
    os << W("(") << _qualifier->getWord() << W(")");
  }

  os << W(" ") << _destination->getHeadWord();
}

#ifdef DISPLAY_PROLOG

void Pattern::displayProlog(OStream & os) const  
{
  displayType(os);
  os << "(";
  os << getSource()->getHeadPosition();
  os << ", ";
  os << getDestination()->getHeadPosition();
  if(getQualifier() != NULL){
    os << ", ";
    os << getQualifier()->getHeadPosition();
  }
  os << ")";
}

#endif

const String & Pattern::getQualifierString() const
{ 
  if(_qualifier != NULL){
    return _qualifier->getWord(); 
  }

  return NULL_QUALIFIER;
};


void Tree::displayPatterns(OStream & os,
			   int offset) const
{
  for(list<PatternPtr>::const_iterator it = _patterns.begin();
      it != _patterns.end(); it ++){
    for(int i = 0; i < offset; i ++) os << W(" ");
    (* it)->display(os);
    os << endl;
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->displayPatterns(os, offset);
  }
}

bool Tree::hasPatterns() const
{
  if(_patterns.empty() == false){
    return true;
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    if((* it)->hasPatterns()){
      return true;
    }
  }

  return false;
}

const String & Tree::getHeadWord() const
{
  if(isTerminal()){
    return getWord();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getWord();
}

const String & Tree::getHeadTag() const
{
  if(isTerminal()){
    return getLabel();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getLabel();
}

short Tree::getHeadPosition() const
{
  if(isTerminal()){
    return getPosition();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getPosition();
}

#ifdef USE_MORPHER
const String & Tree::getHeadLemma() 
{
  if(isTerminal()){
    return getLemma();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getLemma();
}
#endif

const String & Tree::getHeadNe() const
{
  if(isTerminal()){
    return getNe();
  }

  TreePtr tmp;
  for(tmp = getHead(); 
      tmp != (const Tree *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LASSERT(tmp != (const Tree *) NULL, "nil pointer");
  return tmp->getNe();
}

#ifdef USE_MORPHER
const String & Tree::getLemma()
{
  RASSERT(_morpher != (const Morpher *) NULL, "undefined morpher");
  
  // was not computed before
  if(_lemma.empty() == true){
    setLemma(_morpher->morph(getWord(), getLabel()));
    // CERR << getWord() << " ==> " << _lemma << endl;
  }

  return _lemma;
}
#endif

short Tree::setPositions()
{
  short current = 1;
  setPosition(current);
  return (current - 1);
}

void Tree::setPosition(short & current)
{
  if(isTerminal()){
    _position = current;
    current ++;
  } else{
    for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); it ++){
      (* it)->setPosition(current);
    }
  }
}
