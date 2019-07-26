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

#include <assert.h>

#include "collins/hash.h"
#include "collins/SharedMemory.h"

#ifdef USE_SHARED_EVENTS
using namespace spear;
#endif

void hash_make_table(int size,hash_table *hash)
{
  int i;
  hash->table=(hash_node **) malloc(size*sizeof(hash_node*));
  hash->num=0;
  for(i=0;i<size;i++)
    hash->table[i]=NULL;
  hash->size=size;
}

#ifdef USE_SHARED_EVENTS

void hash_make_shared_table(int size, hash_table * hash)
{
  hash->table = (hash_node **) SharedMemory::alloc(size * sizeof(hash_node *));
  hash->num = 0;
  for(int i = 0; i < size; i ++){
    hash->table[i] = NULL;
  }
  hash->size = size;
}

#endif

/*adds a new element if not already there, or increments the old value
otherwise*/
int hash_add_element(key_type *key,hash_table *hash,int count, bool shared)
{
  int pos;
  hash_node *h = NULL,*p = NULL;

/*  printf("KE ");
  for(i=0;i<key->klen;i++)
    printf(" %d",key->key[i]);
  printf("\n");*/

  pos=hashval(key,hash->size);

  if(hash->table[pos]==NULL)
    {

#ifdef USE_SHARED_EVENTS
      if(shared == true){
	h = (hash_node *) SharedMemory::alloc(sizeof(hash_node));
      } else{
	h = (hash_node *) mymalloc(sizeof(hash_node));
      }
#else
      h=(hash_node*) mymalloc(sizeof(hash_node));
#endif

      hash->table[pos]=h;

#ifdef USE_SHARED_EVENTS
      if(shared == true){
	key_shared_copy(& (h->key), key);
      } else{
	key_copy(&(h->key), key);
      }
#else
      key_copy(&(h->key),key);
#endif
      h->count=count;
      h->next=NULL;
      return count;
    }    

  h=hash->table[pos];

  while(h!=NULL&&!key_equal(&h->key,key))
    {
      p=h;
      h=h->next;
    }
  if(h==NULL)
    {

#ifdef USE_SHARED_EVENTS
      if(shared == true){
	h = (hash_node *) SharedMemory::alloc(sizeof(hash_node));
      } else{
	h = (hash_node *) mymalloc(sizeof(hash_node));
      }
#else
      h=(hash_node*) mymalloc(sizeof(hash_node));
#endif

      p->next=h;

#ifdef USE_SHARED_EVENTS
      if(shared == true){
	key_shared_copy(& (h->key), key);
      } else{
	key_copy(&(h->key), key);
      }
#else
      key_copy(&(h->key),key);
#endif

      h->count=count;
      h->next=NULL;
      return count;
    }

  (h->count)+=count;
  return (h->count);
}

int hash_find_element(key_type *key,hash_table *hash)
{
  int pos;
  hash_node *h;

/*  printf("KE2 ");
  for(i=0;i<key->klen;i++)
    printf(" %d",key->key[i]);
  printf("\n");*/

  pos=hashval(key,hash->size);

  if(hash->table[pos]==NULL)
    return 0;

  h=hash->table[pos];

  while(h!=NULL&&!key_equal(&h->key,key))
    h=h->next;

  if(h==NULL)
    return 0;

  return (h->count);
}







