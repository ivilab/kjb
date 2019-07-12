/*!
 * @file CharArrayHashFunc.cc
 *
 * @author Mihai Surdeanu
 * $Id$ 
 */

#include "spear/CharArrayHashFunc.h"

using namespace spear;

size_t CharArrayHashFunc::operator()(const Char * s) const
{
  int hashTemp = 0;
  
  for(unsigned int i = 0; s[i] != 0; i ++){
    if(0 > hashTemp) hashTemp = (hashTemp << 1) + 1;
    else hashTemp = hashTemp << 1;
    hashTemp ^= s[i];
  }
  
  return (size_t(hashTemp));
}
