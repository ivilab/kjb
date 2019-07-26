#ifndef LEXICON_DB_H_
#define LEXICON_DB_H_

/*!
 * @file Lexicon_db.h
 *
 * @author Colin Dawson 
 * $Id: Lexicon_db.h 16870 2014-05-22 19:34:15Z cdawson $ 
 */

#include "semantics/Token_map.h"
#include <string>

namespace semantics
{

class Lexicon_db : public Token_map
{
public:
    static const Key_type& stop_key();
    static const Key_type& low_freq_key();
    static const Val_type& stop_code();
public:
    Lexicon_db() : Token_map() {}
    Lexicon_db(std::string file);
    const Key_type& unknown_key() {return low_freq_key();}
};

};

#endif
