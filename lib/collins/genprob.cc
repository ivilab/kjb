/* This code is the statistical natural language parser described in

   M. Collins. 1999.  Head-Driven
   Statistical Models for Natural Language Parsing. PhD Dissertation,
   University of Pennsylvania.

   Copyright (C) 1999 Michael Collins

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// This file does the smoothing of probabilities.

#include <assert.h>

#include "collins/genprob.h"

#define BONTYPE 0 /*numerators sub-type*/
#define BODTYPE 1 /*denominators*/
#define BOUTYPE 2 /*unique outcomes count*/

#define PROBSMALL 0.0000000000000000001

void add_counts(unsigned char *event,int olen,int *backoffs,char type,hash_table *hash, bool shared)
{
    int i;
    key_type key;
    unsigned char buffer[1000];
    int len; /*total length of the input string*/
    int ns[100]; // store counts per backoff level

    // event starts w/ 3 blank characters,
    // then olen output characters,
    // then context.  Truncate after maximum context.
    len = 3 + olen + backoffs[1];
    //key points to starting point in the buffer
    key.key = buffer;
    
    // Read in event up to last needed context element
    for(i=0; i<len; i++) buffer[i] = event[i];

    /*first add the numerators*/

    assert(backoffs[0]<100);
    
    // type is probability type (dependency, unary, etc.)
    buffer[0] = type;
    // next element denotes whether numerator, denominator or unique count
    buffer[1] = BONTYPE;
    // hash keeps track of counts for each type and context/output combination
    for(i=1;i<=backoffs[0];i++)
    {
	// since key points to buffer, changing buffer[2]
	// changes what key is pointing to (since klen is at least 3)
	buffer[2] = i;
	// key.klen is number of digits of lookup key.
	// includes probability type, count type (num, denom, unique),
	// and which backoff level.  For the numerator counts, we also want
	// to index by output, so the next olen bits are output
	// (what is it the probability of).
	// Finally, include backoffs[i] pieces of conditioning context.
	key.klen = 3 + olen + backoffs[i];
	ns[i] = hash_add_element(&key, hash, 1, shared);
    }

    /*now the unique counts*/
    
    // skip past the output part of the buffer, since here
    // we are only interested in counting contexts.
    key.key = buffer + olen; 

    // Store the probability and count types in the next bit of the buffer
    buffer[olen] = type;
    buffer[olen+1] = BOUTYPE;
    
    for(i=1; i<=backoffs[0]; i++)
    {
	if(ns[i] == 1) 
	{
	    // if this is the first time we have seen this
	    // output/context combination
	    buffer[olen+2] = i; // record which backoff level
	    key.klen = 3 + backoffs[i]; // include the required number of context features
	    hash_add_element(&key, hash, 1, shared);
	}
    }
  
    /*now the denominators*/

    key.key = buffer+olen; // again, skip the output
    buffer[olen] = type; // store prob. and count types in the buffer
    buffer[olen+1] = BODTYPE;
    for(i=1;i<=backoffs[0];i++)
	{
	    buffer[olen+2] = i; // record which backoff level
	    key.klen = 3 + backoffs[i]; // include the required number of context features
	    hash_add_element(&key,hash,1, shared);
	}
}

void add_counts_level(unsigned char *event,int olen,int *backoffs,int level,char type,hash_table *hash, bool shared)
{
  int i;
  key_type key;
  unsigned char buffer[1000];
  int len; /*total length of the input string*/
  int ns[100];

  len = 3+olen+backoffs[1];
  key.key = buffer;

  for(i=0;i<len;i++)
    buffer[i] = event[i];


  /*first add the numerators*/

  assert(backoffs[0]<100);

  buffer[0] = type;
  buffer[1] = BONTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[2] = level;
      key.klen = 3+olen+backoffs[i];
      ns[i] = hash_add_element(&key,hash,1, shared);
    }

  /*now the unique counts*/

  key.key = buffer+olen;
  buffer[olen] = type;
  buffer[olen+1] = BOUTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      if(ns[i] == 1)
	{
	  buffer[olen+2] = level;
	  key.klen = 3+backoffs[i];
	  hash_add_element(&key,hash,1, shared);
	}
    }

  /*now the denominators*/

  key.key = buffer+olen;
  buffer[olen] = type;
  buffer[olen+1] = BODTYPE;
  for(i=1;i<=backoffs[0];i++)
    {
      buffer[olen+2] = level;
      key.klen = 3+backoffs[i];
      hash_add_element(&key,hash,1, shared);
    }
}


double get_prob(unsigned char *event,int olen,int *backoffs,char type,int w1,int w2,hash_table *hash)
{
    // event : a line from the formatted training data
    // olen : number of output features (this is what we're getting the probability of)
    // backoffs : array of how many backoff levels, and how many context features to use at each level
    // type : which type of probability is it?
    // w1,w2 : how much to weight observations and unique contexts, respectively
    // hash_table : the hash table that has the counts in it
    int i;
    key_type key;
    unsigned char buffer[1000];
    int len; /*total length of the input string*/
    int ns[100], us[100], ds[100]; /*counts for numerators, denominators, uniques at
				   each level. Assumes that level 1 is most specific
				 */
    double prob; // prob is small value (roughly interpretable as 1 / size of vocabulary for this feature)
    int bo; // bo is lambda smoothing weight for more specific component

    len = 3 + olen + backoffs[1]; 
    key.key = buffer;

    for(i = 0; i < len; i++) buffer[i] = event[i];

    /*first get the numerators*/

    assert(backoffs[0] < 100);

    buffer[0] = type; // which type of probability
    buffer[1] = BONTYPE; // 0 for numerator
    for(i=1; i<=backoffs[0]; i++)
    {
	// like analogous structure in add_prob, except
	// just look up count instead of adding to it
	buffer[2] = i;
	key.klen = 3 + olen + backoffs[i];
	ns[i] = hash_find_element(&key,hash);
    }

    /*now the unique counts*/

    key.key = buffer + olen;
    buffer[olen] = type;
    buffer[olen+1] = BOUTYPE;
    for(i=1; i<=backoffs[0]; i++)
    {
	buffer[olen + 2] = i;
	key.klen = 3 + backoffs[i];
	us[i] = hash_find_element(&key, hash);
    }

    /*now the denominators*/

    key.key = buffer + olen;
    buffer[olen] = type;
    buffer[olen+1] = BODTYPE;
    for(i=1; i<=backoffs[0]; i++)
    {
	buffer[olen + 2] = i;
	key.klen = 3 + backoffs[i];
	ds[i] = hash_find_element(&key, hash);
    }

    /*  for(i=backoffs[0];i>=1;i--)
	printf("BB %d %d %d %d %d\n",(int) type,i,ns[i],ds[i],us[i]);*/

    /*  if(ds[backoffs[0]] <= 0.1 || ns[backoffs[0]] <= 0.1)
	return 1.0/10000.0;*/

    if(ds[backoffs[0]] <= 0.1)
	return PROBSMALL;

    assert( ds[backoffs[0]] > 0.1);
    /*  assert( ns[backoffs[0]] > 0.1);*/
    assert( us[backoffs[0]] > 0.1);

    prob = PROBSMALL;

    /*  prob = (double) ns[backoffs[0]] / ds[backoffs[0]];*/

    for(i=backoffs[0]; i>=1; i--)
    {
	bo = w1 + w2*us[i]; // total "prior" observations
	if(ds[i] > 0.1) prob = (bo*prob + ns[i]) /( (double) (bo + ds[i]));
    }

    return prob;
}
