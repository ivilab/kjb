#ifndef SEMANTIC_DB_H_
#define SEMANTIC_DB_H_

/*!
 * @file Semantic_db.h
 *
 * @author Colin Dawson 
 * $Id: Semantic_db.h 16870 2014-05-22 19:34:15Z cdawson $ 
 */

#include "semantics/Token_map.h"

namespace semantics
{

class Semantic_db : public Token_map
{
public:
    /*! @brief get the string associated with the special code for "null" value
     */
    static const Key_type& null_key();

    static const Key_type& unknown_entity();
public:
    Semantic_db();
    /*! @brief read in semantic features from a file, mapping them to codes
     */
    void initialize_from_file(std::string file);
    /*! @brief get the string associated with the special code for unknown value
     */
    const Key_type& unknown_key() {return unknown_entity();}
};

};

#endif
