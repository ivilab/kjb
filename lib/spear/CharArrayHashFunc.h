#ifndef CHAR_ARRAY_HASH_FUNC_H_
#define CHAR_ARRAY_HASH_FUNC_H_

/*!
 * @file CharArrayHashFunc.h
 *
 * @author Mihai Surdeanu
 * $Id: CharArrayHashFunc.h 16330 2014-02-04 18:29:14Z cdawson $ 
 */


#include <cstdio>

#include "spear/Wide.h"

namespace spear {

typedef struct CharArrayHashFunc
{
  size_t operator()(const Char * s) const;
} CharArrayHashFunc;

} // end namespace spear

#endif
