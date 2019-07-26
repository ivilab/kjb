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

#ifndef WN_TREE
#define WN_TREE

#include "i/i_type.h" 

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

typedef struct Tree_node
{
    struct Tree_node* parent;         /* Pointer to next element */
    struct Tree_node* children;  /* Pointer to children */
    struct Tree_node* next;    /* Pointer to next element in the same layer*/
    void*             contents; /* Pointer to anything */
    int              visited;
}
Tree_node; 

typedef struct Tree
{
    Tree_node *root;
    int       size;
}
Tree; 

Tree *create_tree
(
    void
);

Tree_node *construct_tree_node
(
    void
);

void free_tree
(
    void *tree_ptr,
    void      (*free_element_fn)(void*)
);

void free_tree_nodes_only
(
    void *tree_ptr
);

int insert_tree_node
(
    Tree      *tree_ptr,
    Tree_node *parent,
    Tree_node *node_ptr
);

Tree_node *insert_tree_node_with_contents
(
    Tree       *tree_ptr,
    Tree_node  *parent,
    void *contents
);

Tree_node *insert_tree_node_with_contents_1
(
    Tree       *tree_ptr,
    const void *parent_contents,
    int              (*key_comp_fn) (const void*, const void*),
    void *contents
);


void *remove_tree_node
(
    Tree       *tree_ptr,
    const void *contents
);

void *search_tree
(
    const Tree       *tree_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr
);


void print_tree
(
    const Tree   *tree_ptr, 
    void         (*print_fn) (const void *)
);

void fp_print_tree
(
    FILE         *fp,
    const Tree   *tree_ptr, 
    void         (*print_fn) (FILE *fp, const void *)
);

void *search_tree_with_depth
(
    const Tree       *tree_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr,
    int              *depth_ptr
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif


#endif
