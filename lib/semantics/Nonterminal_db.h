#ifndef NONTERMINAL_DB_H_
#define NONTERMINAL_DB_H_

/*!
 * @file Nonterminal_db.h
 *
 * @author Colin Dawson 
 * $Id: Nonterminal_db.h 16870 2014-05-22 19:34:15Z cdawson $ 
 */

#include "semantics/Token_map.h"

namespace semantics
{

class Nonterminal_db : public Token_map
{
public:
    static const Key_type& stop_key();
    static const Key_type& low_freq_key();
    static const Key_type& root_key();
    static const Val_type& root_code();
public:
    Nonterminal_db() : Token_map() {}
    Nonterminal_db(std::string file);
    const Key_type& unknown_key() {return low_freq_key();}
};

};

#endif
