/*!
 * @file CharArrayEqualFunc.cc
 *
 * @author Mihai Surdeanu
 * $Id$ 
 */

#include "spear/CharArrayEqualFunc.h"
#include "spear/CharUtils.h"

using namespace spear;

bool CharArrayEqualFunc::operator()(const Char * s1, 
				    const Char * s2) const
{
  return compareWideCharToWideChar(s1, s2);
}
