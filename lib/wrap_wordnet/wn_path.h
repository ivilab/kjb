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

#ifndef WN_PATH
#define WN_PATH

#include "i/i_type.h" 
#include "wrap_wordnet/wn_word_sense.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

typedef struct Path_node
{
    struct Path_node *prev_node;   /*previous node along the path */
    Word_sense       *word_wp;
} Path_node;

Path_node *construct_path_node
(
void
);

Path_node *construct_path_node_1
(
    Path_node *prev_node,
    Word_sense *word_wp
);

int compare_path_node
(
    const void *path_node_ptr1,
    const void *path_node_ptr2
);

int hash_path_node
(
    const void *path_node_ptr
);

void print_path_node
(
    const void *path_node_ptr
);

void free_path_node
(
    void *path_node_ptr
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif



#endif
