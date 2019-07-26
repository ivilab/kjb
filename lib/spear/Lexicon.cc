/*!
 * @file Lexicon.cc
 *
 * @author Mihai Surdeanu
 * $Id$ 
 */

#include <iostream>
#include <sstream>

#include "spear/Lexicon.h"
#include "spear/Assert.h"

using namespace std;
using namespace spear;

size_t spear::LF_WORD_THRESHOLD = 6;

Lexicon::Lexicon()
{
}

void Lexicon::sortLexems(std::vector<String>& sortedLex, size_t lf_thresh) const
{
    // generate all lexicon entries in the Collins format: "word tag known"
    for(unsigned int i = 0; i < _lexems.size(); i ++){
	// is this a known/unknown word?
	String known = W(" 1");
	if(_lexems[i]->getCount() < lf_thresh){
	    known = W(" 0");
	}

	// browse all the lexem tags
	for(std::list< spear::Pair<int, int> >::const_iterator it = 
		_lexems[i]->getLabels().begin();
	    it != _lexems[i]->getLabels().end(); it ++){
	    // build the entry: "word tag known"
	    OStringStream os;
	    os << _lexems[i]->getWord() << " "
	       << _labels[(* it).getFirst()] << known;
	    sortedLex.push_back(os.str());
	}
    }

    // sort the lexicon entries
    stable_sort(sortedLex.begin(), sortedLex.end());  
}

void Lexicon::sortLabels(std::vector<String> & sortedLabels) const
{
  for(unsigned int i = 0; i < _labels.size(); i ++){
    sortedLabels.push_back(_labels[i]);
  }
  
  stable_sort(sortedLabels.begin(), sortedLabels.end());
}

LexemPtr Lexicon::getLexem(const Char * word)
{
  int index;
  if(_lexemIndeces.get(word, index) == true){
    return _lexems[index];
  }

  return LexemPtr();
}

int Lexicon::addLabel(const String & label)
{
  int labelIndex = -1;

  // make sure we know this label
  if(_labelIndeces.get(label.c_str(), labelIndex) == false){
    labelIndex = _labels.size();
    _labels.push_back(label);
    _labelIndeces.set(_labels.back().c_str(), labelIndex);
  }

  return labelIndex;
}

void Lexem::add(int tag)
{
  // increment the overall number of instances
  _count ++;

  for(std::list< spear::Pair<int, int> >::iterator it = _labels.begin();
      it != _labels.end(); it ++){

    // we have seen this tag before
    if((* it).getFirst() == tag){
      (* it).getSecond() ++;
      return;
    }
  }

  // first time we see this tag
  _labels.push_back(Pair<int, int>(tag, 1));
}

void Lexicon::addLexem(const String & word,
		       const String & tag)
{
  int tagIndex = addLabel(tag);
  LASSERT(tagIndex >= 0, "negative tag index");

  LexemPtr lex;

  // we have seen this word before
  if((lex = getLexem(word.c_str())) != (const Lexem *) NULL){
    lex->add(tagIndex);
    return;
  }

  // new word
  lex = LexemPtr(new Lexem(word));
  lex->add(tagIndex);

  int lexIndex = _lexems.size();
  _lexems.push_back(lex);
  _lexemIndeces.set(lex->getWord().c_str(), lexIndex);
}
