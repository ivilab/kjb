/**
 * This work is licensed under a Creative Commons
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 *
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 *
 * You are free:
 *
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 *
 * Under the following conditions:
 *
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 *
 *    Noncommercial. You may not use this work for commercial purposes.
 *
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 *
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 *
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 *
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
| Authors:
|     Luca Del Pero
|
* =========================================================================== */

#include "wrap_wordnet/wn_hash.h"

#ifdef __cplusplus
extern "C" {
#endif


static void hash_init(Hash_table *hash_table);

Hash_table *create_hash_table(int n)
{
    Hash_table *hash_table;

    ASSERT(n > 0);

    
    hash_table = (Hash_table*)malloc(sizeof(Hash_table));
    if(hash_table == NULL)
    {
        return NULL;
    }

    hash_table->lists = (Linkedlist**)malloc(sizeof(Linkedlist*)*n);
    if(hash_table->lists == NULL)
    {
        free(hash_table);
        return NULL;
    }

    hash_table->size = n;
    hash_init(hash_table);

    return hash_table;
}

static void hash_init(Hash_table *hash_table)
{
    int i;
    int n;

    n = hash_table->size;
    for(i=0; i<n; i++ ) {
        hash_table->lists[i] = NULL;
    }

    hash_table->hash_func = NULL;
    hash_table->hash_comp_func = NULL;
}

int hash(const Hash_table *hash_table, const void *key)
{
    int result;
    int n;

    if(hash_table == NULL) return -1;
    
    n = hash_table->size;
    result = hash_table->hash_func(key);

    return result % n;
}

void set_hash_func(Hash_table *hash_table, int (*hash_func)(const void*))
{
    hash_table->hash_func = hash_func;
}

void set_hash_comp_func
(
    Hash_table *hash_table, 
    int        (*hash_comp_func)(const void*, const void*)
)
{
    hash_table->hash_comp_func = hash_comp_func;
}

/*void set_hash_free_func
(
    Hash_table *hash_table, 
    void        (*hash_free_func)(void*)
)
{
    hash_table->hash_free_func = hash_free_func;
}*/

/*int hash(const char *word, int n)
{
    const int shift = 6;
    const int mask = ~0 >> (32-shift); 
    int result = 0;
    int len;

    if(word == NULL)
    {
        return -1;
    }

    len = strlen(word);
    for (int i = 0; i < len; i++)
    {
        result = (result << shift) | (word[i] & mask);
    }

    result = (result >= 0) ? result: (-result);
    return result % n;
}*/


void *hash_find(const Hash_table *hash_table, const void *key)
{
  int entry;
  Linkedlist *list;
  int n;
  void *ptr = NULL;

  if(hash_table->hash_comp_func == NULL)
  {
      add_error("No comparison function is defined!\n");
      return NULL;
  }

  n = hash_table->size;

  /* caluculate hash function value */
  entry = hash(hash_table, key);
  if(entry < 0)
  {
      return NULL;
  }

  list = hash_table->lists[entry];
  if( list == NULL )
    return NULL;

  /* check the hash list of the same key */
  ptr = search_linkedlist (list, hash_table->hash_comp_func, key);

  /* otherwise, return NULL */
  return ptr;
}

int hash_insert(Hash_table *hash_table, const void *key, void *contents)
{
    int entry;
    int n;
    void *ptr = NULL;
    Linkedlist *list = NULL;

    if(contents == NULL || key == NULL)
    {
        return ERROR;
    }
    
    ptr = hash_find(hash_table, key);
    if(ptr != NULL)
    {
        add_error("Hash table: entry exists!");
        return ERROR;
    }

    n = hash_table->size;
    entry = hash(hash_table, key);

    if(entry < 0)
    {
        add_error("Invalid entry!\n");
        return ERROR;
    }
    
    list = hash_table->lists[entry];
    if(list == NULL)
    {
        list = create_linkedlist();
        if(list == NULL)
        {
            return ERROR;
        }
    }
    hash_table->lists[entry] = list;

    if(append_element_with_contents(list, contents) == NO_ERROR)
    {
        return NO_ERROR;
    }

    return ERROR;
}


void *hash_delete(Hash_table *hash_table, const void *key)
{
    int entry;
    int n;
    Linkedlist *list = NULL;
    void *contents = NULL;

    contents  = hash_find(hash_table, key);
    if(contents == NULL)
    {
        return NULL;
    }

    n = hash_table->size;
    entry = hash(hash_table, key);
    if(entry < 0)
    {
        return NULL;
    }

    list = hash_table->lists[entry];
    contents = remove_list_element(list, contents);

    return contents;
}

void clean_hash_table
(
    Hash_table *hash_table, 
    void      (*free_contents_fn)(void*)
)
{
    int n;
    int entry;

    if(hash_table == NULL) return;

    n = hash_table->size;
    for(entry = 0; entry < n; entry++)
    {
        free_linkedlist(hash_table->lists[entry], free_contents_fn);
    }

    free(hash_table->lists);
    free(hash_table);
    hash_table = NULL;
}

void print_hash_table(Hash_table *hash_table, void (*print_func)(const void*))
{
    int i;
    int n;

    if(hash_table == NULL) return;

    n = hash_table->size;

    for(i=0; i<n; i++)
    {
        print_linkedlist(hash_table->lists[i], print_func);
    }
}

/* Put the nodes in a linkedlist into a hash table. It requires two functions:
       1) hash_func ---- generate an entry in the hash table for a node in the list.
       2) hash_comp_func --- compare if two nodes are the same
   Note that the content of a node in the linked list is put into
   the table without copying.  When freeing the hash table, do NOT 
   free the content of the node. A way to do this is 
   'clean_hash_table(..., NULL)'.
*/
int from_list_to_hashtable(
    const Linkedlist *list_ptr, 
    int (*hash_func)(const void*),
    int (*hash_comp_func)(const void*, const void*),
    int size, 
    Hash_table **ht_ptr_ptr
)
{
    Hash_table *ht_ptr = NULL;
    List_element *element = NULL;

    if(ht_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }

    if((ht_ptr = create_hash_table(size)) == NULL)
    {
        return ERROR;
    }
    *ht_ptr_ptr = ht_ptr;

    set_hash_func(ht_ptr, hash_func);
    set_hash_comp_func(ht_ptr, hash_comp_func);
    
    element = list_ptr->head;
    while(element != NULL)
    {
        ERE(hash_insert(ht_ptr, element->contents, element->contents));
        element = element->next;
    }

    return NO_ERROR;
}

int from_array_to_hashtable(
    const Array *array_ptr, 
    int (*hash_func)(const void*),
    int (*hash_comp_func)(const void*, const void*),
    int size, 
    Hash_table **ht_ptr_ptr
)
{
    Hash_table *ht_ptr = NULL;
    int i;
    int length;

    if(ht_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }

    if((ht_ptr = create_hash_table(size)) == NULL)
    {
        return ERROR;
    }
    *ht_ptr_ptr = ht_ptr;

    set_hash_func(ht_ptr, hash_func);
    set_hash_comp_func(ht_ptr, hash_comp_func);
    
    length = array_ptr->length;
    for(i=0; i<length; i++)
    {
        ERE(hash_insert(ht_ptr, array_ptr->elements[i], array_ptr->elements[i]));
    }

    return NO_ERROR;
}

#ifdef __cplusplus
}
#endif

