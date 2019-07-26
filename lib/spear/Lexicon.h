#ifndef LEXICON_H_
#define LEXICON_H_

/*!
 * @file Lexicon.h
 *
 * Lexicon learned from training data
 *
 * @author Mihai Surdeanu
 * $Id: Lexicon.h 16664 2014-04-08 22:52:41Z cdawson $ 
 */

#include <list>
#include <vector>
#include <string>

#include "spear/RCIPtr.h"
#include "spear/HashMap.h"
#include "spear/CharArrayEqualFunc.h"
#include "spear/CharArrayHashFunc.h"
#include "spear/Pair.h"
#include "spear/StringMap.h"
#include "spear/Wide.h"


namespace spear {

    extern size_t LF_WORD_THRESHOLD;

class Lexem : public RCObject
{
 public:

  Lexem() : _count(0) {};

  Lexem(const String & w) : _word(w), _count(0) {};

  void add(int tag);

  const String & getWord() const { return _word; };

  const std::list< spear::Pair<int, int> > & getLabels() const { return _labels; };

  int getCount() const { return _count; };
  
 private:
  /** The lexem */
  String _word;

  /** How many times have we seen this word */
  int _count;

  /** 
   * The set of valid part of speech tags and their corresponding counts,
   * in the form <tag, count>
   */
  std::list< spear::Pair<int, int> > _labels;
};

typedef RCIPtr<spear::Lexem> LexemPtr;

class Lexicon : public RCObject
{
 public:
  Lexicon();

    void addLexem(
	const String & word,
	const String & tag);

    int addLabel(const String & label);

    void sortLexems(std::vector<String> & sortedLex, size_t lf_thresh) const;
    void sortLabels(std::vector<String> & sortedLabels) const;  

private:

    LexemPtr getLexem(const Char * word);

    /** The set of words in the lexicon */
    std::vector<LexemPtr> _lexems;

    /** Hash from words to corresponding indeces */
    StringMap<int> _lexemIndeces;

    /** Set of seen labels */
    std::vector<String> _labels;

    /** Hash from labels to corresponding indeces */
    StringMap<int> _labelIndeces;
};

typedef RCIPtr<spear::Lexicon> LexiconPtr;

} // end namespace spear

#endif
