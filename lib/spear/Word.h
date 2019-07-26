#ifndef WORD_H_
#define WORD_H_

/*!
 * @file Word.h
 *
 * @author Mihai Surdeanu
 * $Id: Word.h 16330 2014-02-04 18:29:14Z cdawson $ 
 */

#include <iostream>
#include <string>
#include <vector>

#include "spear/RCIPtr.h"
#include "spear/Wide.h"

namespace spear {

class Word {

 public:

  Word(const String & w,
       const String & t) : _word(w), _tag(t) {};

  Word(const spear::Word & word) 
    : _word(word.getWord()), _tag(word.getTag()) {};

  spear::Word & operator = (const spear::Word & word) {
    _word = word.getWord();
    _tag = word.getTag();
    return (* this);
  }

  const String & getWord() const { return _word; };

  const String & getTag() const { return _tag; };

  void setTag(const String & tag) { _tag = tag; };

 private:
  
  String _word;

  String _tag;

};

} // end namespace spear

#endif
