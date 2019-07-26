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

#include "wrap_wordnet/wn_tree.h"
#include "wrap_wordnet/wn_word_sense.h"

#ifdef __cplusplus
extern "C" {
#endif

static Tree_node *search_tree_1
(
    Tree_node  *node_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr
);

static Tree_node *search_tree_2
(
    Tree_node  *node_ptr,
    const void       *key_ptr
);

static Tree_node *search_tree_3
(
    Tree_node  *node_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr,
    int              *depth_ptr
);

static void recursive_free_tree
(
    Tree_node *node_ptr,
    void      (*free_element_fn)(void*)
);
    
static void print_tree_node
(
    const Tree_node *node_ptr,
    int              depth,
    void            (*print_fn) (const void *)
);

static void fp_print_tree_node
(
    FILE            *fp,
    const Tree_node *node_ptr,
    int              depth,
    void            (*print_fn) (FILE *fp, const void *)
);

Tree *create_tree( )
{
    Tree *tree_ptr;

    tree_ptr = (Tree*)malloc(sizeof(Tree));
    if(tree_ptr == NULL)
    {
        return NULL;
    }

    tree_ptr->root = NULL;
    tree_ptr->size = 0;

    return tree_ptr;
}

Tree_node *construct_tree_node( ) 
{
    Tree_node *node_ptr = NULL;
    node_ptr = (Tree_node*)malloc(sizeof(Tree_node));
    if(node_ptr == NULL)
    {
        return NULL;
    }

    node_ptr->parent = NULL;
    node_ptr->children = NULL;
    node_ptr->next = NULL;
    node_ptr->contents = NULL;
    node_ptr->visited = 0;

    return node_ptr;
}


int insert_tree_node
(
    Tree      *tree_ptr,
    Tree_node *parent,
    Tree_node *node_ptr
)
{
    Tree_node *child_ptr;

    if(parent == NULL)
    {
        tree_ptr->root = node_ptr;
    }
    else
    {
        child_ptr = parent->children;
        if(child_ptr == NULL)  /* first child */
        {
            parent->children = node_ptr;
        }
        else
        {
            while(child_ptr->next != NULL)
            {
                child_ptr = child_ptr->next;
            }
            child_ptr->next = node_ptr;
        }
    }
    node_ptr->next = NULL;
    node_ptr->parent = parent;

    (tree_ptr->size)++;

    return NO_ERROR;
}

Tree_node *insert_tree_node_with_contents
(
    Tree       *tree_ptr,
    Tree_node  *parent,
    void       *contents
)
{
    Tree_node *node_ptr = NULL;
    int res;

    node_ptr = construct_tree_node();
    if(node_ptr == NULL)
    {
        return NULL;
    }
    node_ptr->contents = contents;

    
    res = insert_tree_node(tree_ptr, parent, node_ptr);
    if(res == ERROR) return NULL;

    return node_ptr;
}

Tree_node *insert_tree_node_with_contents_1
(
    Tree       *tree_ptr,
    const void *parent_contents,
    int              (*key_comp_fn) (const void*, const void*),
    void *contents
)
{
    Tree_node *parent_node_ptr = NULL;
    
    parent_node_ptr = search_tree_1(tree_ptr->root, key_comp_fn, parent_contents);
    if(parent_node_ptr == NULL)
    {
        print_word_sense((Word_sense*)contents);
        print_word_sense((Word_sense*)parent_contents);
        print_word_sense((Word_sense*)tree_ptr->root->contents);
        pso("Parent node is not found!\n");
        return NULL;
    }

    return insert_tree_node_with_contents(tree_ptr, parent_node_ptr, contents);
}


/*void *remove_tree_node
(
    Tree       *tree_ptr,
    const void *contents
)
{
   Tree_node *node_ptr;

   node_ptr = search_tree_2(tree_ptr->root, contents);
   if(node_ptr == NULL) return NULL;


}*/

void *search_tree
(
    const Tree       *tree_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr
)
{
    Tree_node *node_ptr;

    node_ptr = search_tree_1(tree_ptr->root, key_comp_fn, key_ptr);
    if(node_ptr == NULL) return NULL;

    return node_ptr->contents;
}

void free_tree
(
    void *tree_ptr,
    void      (*free_element_fn)(void*)
)
{
    Tree *ptr = (Tree*)tree_ptr;

    if(ptr == NULL) return;

    recursive_free_tree(ptr->root, free_element_fn);
    free(ptr);
    ptr = NULL;
}

void free_tree_nodes_only
(
    void *tree_ptr
)
{
    free_tree(tree_ptr, NULL);
}


void print_tree
(
    const Tree   *tree_ptr, 
    void         (*print_fn) (const void *)
)
{
    if(tree_ptr == NULL) return;

    pso("\n------- Tree (size: %d) ------\n", tree_ptr->size);
    print_tree_node(tree_ptr->root, 0, print_fn);
    pso("\n");
}

void fp_print_tree
(
    FILE         *fp,
    const Tree   *tree_ptr, 
    void         (*print_fn) (FILE *fp, const void *)
)
{
    if(tree_ptr == NULL) return;

    fprintf(fp, "\n------- Tree (size: %d) ------\n", tree_ptr->size);
    fp_print_tree_node(fp, tree_ptr->root, 0, print_fn);
    fprintf(fp, "\n");
}



static Tree_node *search_tree_1
(
    Tree_node  *node_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr
)
{
    Tree_node *ptr = NULL;
    Tree_node *child_ptr = NULL;

    if(node_ptr == NULL) return NULL;

    if(key_comp_fn(node_ptr->contents, key_ptr) == 1)
    {
        return node_ptr;
    }

    child_ptr = node_ptr->children;
    while(child_ptr != NULL)
    {
        ptr = search_tree_1(child_ptr, key_comp_fn, key_ptr);
        if(ptr != NULL) return ptr;

        child_ptr = child_ptr->next;
    }

    return NULL;
}

static Tree_node *search_tree_2
(
    Tree_node  *node_ptr,
    const void       *key_ptr
)
{
    void *ptr = NULL;
    Tree_node *child_ptr = NULL;

    if(node_ptr == NULL) return NULL;

    ptr = node_ptr->contents;
    if(ptr == key_ptr)
    {
        return node_ptr;
    }

    child_ptr = node_ptr->children;
    while(child_ptr != NULL)
    {
        search_tree_2(child_ptr, key_ptr);
        child_ptr = child_ptr->next;
    }

    return NULL;
}

static void recursive_free_tree
(
    Tree_node *node_ptr,
    void      (*free_element_fn)(void*)
)
{
    Tree_node *child_ptr = NULL;

    if(node_ptr == NULL) return;

    child_ptr = node_ptr->children;
    while(child_ptr != NULL)
    {
        recursive_free_tree(child_ptr, free_element_fn);
        child_ptr = child_ptr->next;
    }

    if(free_element_fn != NULL)
    {
        free_element_fn(node_ptr->contents);
    }

    free(node_ptr);
    node_ptr = NULL;
}
    
/*static void print_tree_node
(
    const Tree_node *node_ptr,
    int              depth,
    void            (*print_fn) (const void *)
)
{
    Tree_node *child_ptr = NULL;

    if(node_ptr == NULL) return;

    pso("---- Depth: %d ---- \n", depth);
    

    print_fn(node_ptr->contents);

    child_ptr = node_ptr->children;

    while(child_ptr != NULL)
    {
        print_tree_node(child_ptr, depth + 1, print_fn);
        child_ptr = child_ptr->next;
    }
}*/

static void print_tree_node
(
    const Tree_node *node_ptr,
    int              depth,
    void            (*print_fn) (const void *)
)
{
    const Tree_node *current = NULL;

    if(node_ptr == NULL) return;

    pso("\n          Depth: %d \n", depth);
    current = node_ptr;
    while( current != NULL)
    {
        print_fn(current->contents);
        current = current->next;
    }
    
    current = node_ptr;
    while( current != NULL)
    {
        print_tree_node(current->children, depth + 1, print_fn);
        current = current->next;
    }
}

static void fp_print_tree_node
(
    FILE            *fp,
    const Tree_node *node_ptr,
    int              depth,
    void            (*print_fn) (FILE *fp, const void *)
)
{
    const Tree_node *current = NULL;

    if(node_ptr == NULL) return;

    fprintf(fp, "\n          Depth: %d \n", depth);
    current = node_ptr;
    while( current != NULL)
    {
        print_fn(fp, current->contents);
        current = current->next;
    }
    fprintf(fp, "\n");

    current = node_ptr;
    while( current != NULL)
    {
        fp_print_tree_node(fp, current->children, depth + 1, print_fn);
        current = current->next;
    }
}

void *search_tree_with_depth
(
    const Tree       *tree_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr,
    int              *depth_ptr
)
{
    Tree_node *node_ptr;

    node_ptr = search_tree_3(tree_ptr->root, key_comp_fn, key_ptr, depth_ptr);
    if(node_ptr == NULL) return NULL;

    return node_ptr->contents;
}

static Tree_node *search_tree_3
(
    Tree_node  *node_ptr,
    int              (*key_comp_fn) (const void*, const void*),
    const void       *key_ptr,
    int              *depth_ptr
)
{
    Tree_node *ptr = NULL;
    Tree_node *child_ptr = NULL;

    if(node_ptr == NULL) return NULL;

    if(key_comp_fn(node_ptr->contents, key_ptr) == 1)
    {
        return node_ptr;
    }

    child_ptr = node_ptr->children;
    while(child_ptr != NULL)
    {
        if(depth_ptr != NULL) (*depth_ptr)++;
        ptr = search_tree_3(child_ptr, key_comp_fn, key_ptr, depth_ptr);
        if(ptr != NULL) return ptr;

        child_ptr = child_ptr->next;
    }

    return NULL;
}

#ifdef __cplusplus
}
#endif
