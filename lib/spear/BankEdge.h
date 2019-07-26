#ifndef BANK_EDGE_H
#define BANK_EDGE_H

/**
 * @file BankEdge.h
 * Edge representation used in model training
 * $Id: BankEdge.h 18301 2014-11-26 19:17:13Z ksimek $
 */


#include <list>
#include <string>
#include <vector>

#include "spear/RCIPtr.h"
#include "spear/Wide.h"


namespace spear {

    class BankEdge : public RCObject 
    {
    public:

	BankEdge() {};

	void setWord(const String & w) { _word = w; };

	const String & getWord() const { return _word; };

	void setLabel(const String & l) { _label = l; };

#ifdef USE_UNICODE
	void setLabel(const std::string & l);
#endif

	const String & getLabel() const { return _label; };  

	const String & getLabelAugmentation() const { 
	    return _labelAugmentation; 
	};    

	void setLabelAugmentation(const String & la) { 
	    _labelAugmentation = la; 
	};

	bool isTerminal() const { 
	    return (_children.empty() && !_word.empty()); 
	};

	void addChild(const RCIPtr<BankEdge> & c) { _children.push_back(c); };

	const std::list< RCIPtr<BankEdge> > & getChildren() const { 
	    return _children; 
	};

	std::list< RCIPtr<BankEdge> > & getChildren() { return _children; };

	void display(OStream &, 
		     bool isHead = false, 
		     int offset = 0) const;

	void displayPrettyParens(OStream & os,
				 int offset = 0) const;

	void displayParens(OStream & os,
			   bool showHead = true) const;

	int countTerminals() const;

	void setHead(const RCIPtr<BankEdge> & h) { _head = h; };

	const RCIPtr<BankEdge> & getHead() const { return _head; };

	const std::list< RCIPtr<BankEdge> >::iterator & getHeadIterator() const 
	{ return _headIterator; };

	const std::list< RCIPtr<BankEdge> >::reverse_iterator & getHeadReverseIterator() const 
	{ return _headReverseIterator; };

	void setHead();

	void setSentenceIndex(int i) { _index = i; };

	int getSentenceIndex() const { return _index; };

	const String & getHeadWord() const;

	const String & getHeadTerminalLabel() const;

	void setNe(const String & ne) { _ne = ne; };

	const String & getNe() const { return _ne; };

	// for the bootstrapping experiments. not used in stable code
	// void findSynonyms();
	// const std::vector<String> & getSynonyms() const { return _synonyms; };
	// const std::vector<String> & getHeadSynonyms() const;

	static const RCIPtr<BankEdge> & getStopPhrase();

    private:

	String _word;

	String _label;

	String _labelAugmentation;

	String _ne;

	/**
	 * List of synonyms for this terminal phrase word.
	 * Added for the bootstrapping experiments. Not used in the stable code.
	 */
	// std::vector<String> _synonyms;

	int _index;

	RCIPtr<BankEdge> _head;
	std::list< RCIPtr<BankEdge> >::iterator _headIterator;
	std::list< RCIPtr<BankEdge> >::reverse_iterator _headReverseIterator;
    
	/**
	 * List of children of this edge
	 */
	std::list< RCIPtr<BankEdge> > _children;

    };

    typedef RCIPtr<BankEdge> BankEdgePtr;

    typedef std::list< spear::RCIPtr<BankEdge> > BankEdgeList;

    typedef std::list< spear::RCIPtr<BankEdge> >::const_reverse_iterator
    BankEdgeListConstRevIter;

    typedef std::list< spear::RCIPtr<BankEdge> >::const_iterator
    BankEdgeListConstIter;

} // end namespace spear

OStream& operator<<(OStream& os, const spear::BankEdgePtr&);

#endif /* BANK_EDGE_H */
