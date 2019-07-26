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

#ifndef WN_SEMANTIC_TREE
#define WN_SEMANTIC_TREE

#include <stdlib.h>
#include <stdio.h>

#include "m/m_incl.h"
#include "n/n_incl.h" 
#include "i/i_incl.h" 
#include "i/i_float.h"
#include "wrap_wordnet/wn_linkedlist.h"
#include "wrap_wordnet/wn_hash.h"
#include "wrap_wordnet/wn_word_sense.h"
#include "wrap_wordnet/wn_tree.h"
#include "wrap_wordnet/wn_path.h"
#include "wrap_wordnet/wn_array.h"
#include "wrap_wordnet/wn_region_label.h"

#ifdef KJB_HAVE_WN
#include "wn.h"
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int construct_semantic_tree_for_vocabulary
(
    const Array *vocabulary_array_ptr,
    const Array *word_array_ptr,
    Array  **tree_array_ptr_ptr
);
       
int construct_semantic_tree_for_word
(
    Word_sense *word_wp,
    const Hash_table *vocabulary_ht_ptr,
    Tree **tree_ptr_ptr
);

void free_tree_array
(
   Array *tree_array_ptr
);

int write_semantic_tree_result
(
    const Array *tree_array_ptr, 
    const char  *filename
);

int generate_dot_file
(
    const Array *tree_array_ptr,
    const char *filename
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
