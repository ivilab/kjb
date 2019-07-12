#ifndef CHAR_ARRAY_EQUAL_FUNC_H_
#define CHAR_ARRAY_EQUAL_FUNC_H_

/*!
 * @file CharArrayEqualFunc.h
 *
 * @author Mihai Surdeanu
 * $Id: CharArrayEqualFunc.h 16330 2014-02-04 18:29:14Z cdawson $ 
 */

#include "spear/Wide.h"

namespace spear {

typedef struct CharArrayEqualFunc
{
  bool operator()(const Char * s1, const Char * s2) const;
} CharArrayEqualFunc;

} // end namespace spear

#endif
