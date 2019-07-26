#ifndef TOKENIZER_H_
#define TOKENIZER_H_

/*!
 * @file Tokenizer.h
 *
 * @author Mihai Surdeanu
 * $Id: Tokenizer.h 16330 2014-02-04 18:29:14Z cdawson $ 
 */

#include <vector>
#include <string>

#include "spear/Wide.h"
#include "spear/RCIPtr.h"
#include "spear/StringMap.h"
#include "spear/Word.h"

namespace spear {

class Tokenizer : public RCObject {

 public:

  Tokenizer(const std::string & dataPath);
  
  void split(const String & buffer, std::vector<spear::Word> & tokens);

  static String normalize(const String & s);

 private:

  static bool initialize(const std::string & dataPath);

  static bool initialized;

  static spear::StringMap<bool> multiWords;

  String concatenate(const std::vector<String> & tokens,
		     unsigned int start,
		     unsigned int end) const;
};

typedef RCIPtr<Tokenizer> TokenizerPtr;

}

#endif
