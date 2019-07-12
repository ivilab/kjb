/* @id */

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
* =========================================================================== */

#ifndef WN_HASH
#define WN_HASH
#include "l/l_incl.h"
#include "wrap_wordnet/wn_linkedlist.h"
#include "wrap_wordnet/wn_array.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* hash */
#define DEFAULT_HASH_SIZE 1000 

typedef struct Hash_table
{
    Linkedlist **lists;
    int (*hash_func)(const void*);
    int (*hash_comp_func)(const void*, const void*);
    int size;
} Hash_table;

Hash_table *create_hash_table(int n);
void set_hash_func(Hash_table *hash_table, int (*hash_func)(const void*));

void set_hash_comp_func
(
    Hash_table *hash_table, 
    int        (*hash_comp_func)(const void*, const void*)
);

int hash(const Hash_table *hash_table, const void *key);
void *hash_find(const Hash_table *hash_table, const void *key);
int hash_insert(Hash_table *hash_table, const void *key, void *contents);
void *hash_delete(Hash_table *hash_table, const void *key);
void clean_hash_table
(
    Hash_table *hash_table, 
    void      (*free_contents_fn)(void*)
);
void print_hash_table(Hash_table *hash_table, void (*print_func)(const void*));
int from_list_to_hashtable(
    const Linkedlist *list_ptr, 
    int (*hash_func)(const void*),
    int (*hash_comp_func)(const void*, const void*),
    int size, 
    Hash_table **ht_ptr_ptr
);

int from_array_to_hashtable(
    const Array *array_ptr, 
    int (*hash_func)(const void*),
    int (*hash_comp_func)(const void*, const void*),
    int size, 
    Hash_table **ht_ptr_ptr
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif


#endif
