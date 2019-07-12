
#include <iostream>

#include "spear/BankEdge.h"
#include "spear/Head.h"

// for synonym detection
//#include "Wn.h"

using namespace std;
using namespace spear;

#ifdef USE_UNICODE
static void setWideString(String & s, const char * v)
{
  s.clear();
  for(int i = 0; v[i] != 0; i ++){
    s.push_back(v[i]);
  }
}

void BankEdge::setLabel(const std::string & l)
{
  setWideString(_label, l.c_str());
}
#endif

void BankEdge::display(OStream & os, 
		       bool isHead, 
		       int offset) const
{
  for(int i = 0; i < offset; i ++) os << " ";
  os << getLabel();
  if(getLabelAugmentation().empty() == false){
    os << "/" << getLabelAugmentation();
  }

  if(isTerminal()){
    os << " " << getWord();
    if(isHead) os << " *";
  } else{
    if(isHead) os << " *";
    for(list<BankEdgePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      os << endl;

      isHead = false;
      if(_head == (* it)) isHead = true;

      (* it)->display(os, isHead, offset + 4);
    }
  }
}

void BankEdge::displayPrettyParens(OStream & os,
				   int offset) const
{
  for(int i = 0; i < offset; i ++) os << W(" ");
  os << W("(") << getLabel();
  
  if(isTerminal()){
    os << W(" ") << getWord();
  } else{
    os << endl;
    for(list<BankEdgePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      (* it)->displayPrettyParens(os, offset + 4);
      os << endl;
    }
    for(int i = 0; i < offset; i ++) os << W(" ");
  }
  
  os << W(")");
}

void BankEdge::displayParens(OStream & os,
			     bool showHead) const
{
  os << W("(") << getLabel();

  if(isTerminal()){
    os << W(" ") << getWord();
  } else{
    for(list<BankEdgePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      os << W(" ");
      (* it)->displayParens(os, showHead);
    }

    // display the head position if set
    if(showHead == true && getHead() != (const BankEdge *) NULL){
      int position = 0;
      for(list<BankEdgePtr>::const_iterator it = _children.begin();
	  it != _children.end(); it ++, position ++){
	if((* it) == getHead()){
	  os << W(" ") << position;
	  break;
	}
      }
    }
  }

  os << W(")");
}

int BankEdge::countTerminals() const
{
    if(isTerminal()) return 1;

    int count = 0;
    for(list<BankEdgePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++)
    {
	count += (* it)->countTerminals();
    }

    return count;
}

void BankEdge::setHead()
{
    for(list<BankEdgePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++)
    {
	(* it)->setHead();
    }

    if(_children.empty() == false)
    {
	setHead(findHead<BankEdge>(getLabel(), getChildren()));
	
	// find the direct iterator
	_headIterator = _children.end();
	for(_headIterator = _children.begin();
	    _headIterator != _children.end();
	    _headIterator ++)
	{
	    if((* _headIterator).operator->() == _head.operator->())
	    {
		break;
	    }
	}
	LASSERT(_headIterator != _children.end(), "missing head iterator");

	// find the reverse iterator
	_headReverseIterator = _children.rend();
	for(_headReverseIterator = _children.rbegin();
	    _headReverseIterator != _children.rend();
	    _headReverseIterator ++)
	{
	    if((* _headReverseIterator).operator->() == _head.operator->())
	    {
		break;
	    }
	}
	LASSERT(_headReverseIterator != _children.rend(), "missing head iterator");

    }
}

const String & BankEdge::getHeadWord() const
{
    if(isTerminal()) return getWord();

    BankEdgePtr tmp = getHead();
    while(tmp != (const BankEdge *) NULL && !tmp->isTerminal())
    {
	tmp = tmp->getHead();
    }
    LASSERT(tmp != (const BankEdge *) NULL, "nil pointer");
    return tmp->getWord();
}

const String & BankEdge::getHeadTerminalLabel() const
{
  if(isTerminal()){
    return getLabel();
  }

  BankEdgePtr tmp;
  for(tmp = getHead(); 
      tmp != (const BankEdge *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead())
  {}
  LASSERT(tmp != (const BankEdge *) NULL, "nil pointer");
  return tmp->getLabel();
}

const RCIPtr<BankEdge> & BankEdge::getStopPhrase()
{
  static RCIPtr<BankEdge> stop(new BankEdge);

  if(stop->getLabel().empty() == true){
    stop->setLabel(W("#STOP#"));
    RCIPtr<BankEdge> child(new BankEdge);
    child->setLabel(W("#STOP#"));
    child->setWord(W("#STOP#"));
    stop->addChild(child);
    stop->setHead(stop->getChildren().front());
  }

  return stop;
}

/*
void BankEdge::findSynonyms()
{
  if(isTerminal()){
    if(getLabel() == W("NN") ||
       getLabel() == W("VB") ||
       getLabel() == W("RB") ||
       getLabel() == W("JJ")){
      WordNetPtr wn;
      try{
	wn = WordNetPtr(new WordNet);
      } catch(...){
	CERR << "Failed to initialize WordNet" << endl;
	exit(1);
      }
      wn->synonyms(getWord(), getLabel(), _synonyms);
    }
  }

  for(list<BankEdgePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->findSynonyms();
  }
}

const std::vector<String> & BankEdge::getHeadSynonyms() const
{
  if(isTerminal()){
    return getSynonyms();
  }

  BankEdgePtr tmp;
  for(tmp = getHead(); 
      tmp != (const BankEdge *) NULL && ! tmp->isTerminal();
      tmp = tmp->getHead());
  LASSERT(tmp != (const BankEdge *) NULL, "nil pointer");
  return tmp->getSynonyms();
}
*/

OStream & operator << (OStream & os, const spear::BankEdgePtr & edge)
{
  edge->display(os);
  return os;
}
