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

#include "wrap_wordnet/wn_path.h"

#ifdef __cplusplus
extern "C" {
#endif


Path_node *construct_path_node()
{
    return construct_path_node_1(NULL, NULL); 
}

Path_node *construct_path_node_1
(
    Path_node *prev_node,
    Word_sense *word_wp
)
{
    Path_node *node_ptr = NULL;

    node_ptr = (Path_node*)malloc(sizeof(Path_node));
    if(node_ptr == NULL)
    {
        return NULL;
    }
    
    node_ptr->prev_node = prev_node;
    node_ptr->word_wp = word_wp;

    return node_ptr;
}

int compare_path_node
(
    const void *path_node_ptr1,
    const void *path_node_ptr2
)
{
    Path_node *ptr1;
    Path_node *ptr2;

    ptr1 = (Path_node*)path_node_ptr1;
    ptr2 = (Path_node*)path_node_ptr2;
    if(ptr1 == NULL || ptr2 == NULL)
    {
        return 0;
    }

     return compare_word_sense((void*)ptr1->word_wp, (void*)ptr2->word_wp);
}

int hash_path_node
(
    const void *path_node_ptr
)
{
    Path_node *ptr = NULL;
    ptr = (Path_node*)path_node_ptr;
    return hash_word_sense(ptr->word_wp);
}

void print_path_node
(
    const void *path_node_ptr
)
{
    Path_node *ptr;
    
    ptr = (Path_node*)path_node_ptr;
    if(ptr == NULL) return;

    if(ptr->prev_node == NULL)
    {
        pso("No previous node!\n");
    }
    else
    {
        print_word_sense(ptr->prev_node->word_wp);
    }
    print_word_sense(ptr->word_wp);
}

void free_path_node
(
    void *path_node_ptr
)
{
    Path_node *ptr = NULL;
    ptr = (Path_node*)path_node_ptr;
    if(ptr == NULL) return;
    free(ptr->word_wp);
    ptr->word_wp = NULL;
    free(ptr);
    ptr = NULL;
}

#ifdef __cplusplus
}
#endif
