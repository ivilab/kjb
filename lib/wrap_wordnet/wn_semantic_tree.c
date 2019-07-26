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

#include "wrap_wordnet/wn_semantic_tree.h"
#ifdef KJB_HAVE_WN
#include "wn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

static int fs_hash_table_size = 1000;
static int fs_parent_type = IS_A|
                            PART_OF|
                            MEMBER_OF|
                            INSTANCE_OF;

static int fs_max_sense_searched = 100;

static int traverse_to_parent
(
    Path_node *path_node_ptr, 
    const Hash_table *vocabulary_ht_ptr, 
    Linkedlist       *to_do_list, 
    Hash_table       *wordnet_ht_ptr,
    Tree             *tree_ptr
);

static int traverse_to_neighbor
(
    Path_node  *path_node_ptr, 
    const Hash_table *vocabulary_ht_ptr, 
    int              search_type,
    Linkedlist       *to_do_list, 
    Hash_table       *wordnet_ht_ptr,
    Tree             *tree_ptr
);

static int insert_word_to_tree
(
    const Word_sense *word_wp,
    const Path_node  *prev_path_node,
    Tree             *tree_ptr
);

static int insert_path_node_to_hashtable
(
    int              index,
    char       *word,
    int              sense,
    Path_node  *prev_path_node,
    Linkedlist       *list_ptr,
    Hash_table        *ht_ptr
);

static int insert_path_node_to_hashtable_1
(
    int              index,
    const char       *word,
    int              sense,
    Path_node        *prev_path_node,
    Linkedlist       *list_ptr,
    Hash_table        *ht_ptr
);

#ifdef KJB_HAVE_WN
static Path_node *is_synset_in_table_1
(
    const SynsetPtr  synset_ptr,
    Path_node  *prev_node_ptr,
    const Hash_table *ht_ptr
);

static int is_synset_in_vocabulary
(
    const SynsetPtr  synset_ptr,
    const Hash_table *vocabulary_ht_ptr,
    Word_sense       **word_wpp
);
#endif

static void print_tree_edge
(
    FILE            *fp,
    const Tree_node *node_ptr,
    const Int_matrix      *visited_imp
);


int construct_semantic_tree_for_vocabulary
(
    const Array *vocabulary_array_ptr,
    const Array *word_array_ptr,
    Array  **tree_array_ptr_ptr
)
{
   int i;
   int length;
   Hash_table *ht_ptr;
   Word_sense *word_wp = NULL;
   Tree *tree_ptr = NULL;
   Array *tree_array_ptr = NULL;

   if(vocabulary_array_ptr == NULL || word_array_ptr == NULL)
   {
       return NO_ERROR;
   }
   
   length = word_array_ptr->length;
   if(tree_array_ptr_ptr == NULL)
   {
       return NO_ERROR;
   }
   if((tree_array_ptr = create_array(length)) == NULL)
   {
       add_error("Failed to create array!\n");
       return ERROR;
   }
   *tree_array_ptr_ptr = tree_array_ptr;
   
   ERE(from_array_to_hashtable(vocabulary_array_ptr, hash_word_sense,
       compare_word_sense, fs_hash_table_size, &ht_ptr));

   for(i = 0; i < length; i++)
   {
       word_wp = (Word_sense*)word_array_ptr->elements[i];

       #ifdef TEST
       pso("\nStarting %s\n", word_wp->word);
       #endif
       ERE(construct_semantic_tree_for_word(word_wp, ht_ptr, &tree_ptr));
       /*print_tree(tree_ptr, print_word_sense);
       append_element_with_contents(list_ptr, (void*)tree_ptr);*/
       tree_array_ptr->elements[i] = (void*)tree_ptr;
       tree_ptr = NULL;
       #ifdef TEST
       pso("Ending %s\n", word_wp->word);
       #endif
   }
   
   clean_hash_table(ht_ptr, NULL);

   return NO_ERROR;
}
       
int construct_semantic_tree_for_word
(
    Word_sense *word_wp,
    const Hash_table *vocabulary_ht_ptr,
    Tree **tree_ptr_ptr
)
{
    Tree *tree_ptr;
    Hash_table *wordnet_ht_ptr = NULL;
    Linkedlist *to_do_list = NULL;
    Path_node *current_path_node = NULL;
    int res = NO_ERROR;
    
    if(tree_ptr_ptr == NULL)
    {
        return NO_ERROR;
    }
    if((tree_ptr = create_tree()) == NULL)
    {
        add_error("Failed to create tree structure!\n");
        return ERROR;
    }
    *tree_ptr_ptr = tree_ptr;
    /*if(insert_tree_node_with_contents(tree_ptr, (Tree_node*)NULL, word_wp) == NULL)
    {
        add_error("Failed to insert node to the tree!\n");
        return ERROR;
    }*/

    /* create a hashtable */
    wordnet_ht_ptr = create_hash_table(fs_hash_table_size);
    ASSERT(wordnet_ht_ptr != NULL);

    set_hash_func(wordnet_ht_ptr, hash_path_node);
    set_hash_comp_func(wordnet_ht_ptr, compare_path_node);

    /* create a queue */
    to_do_list = create_linkedlist();
    ASSERT(to_do_list != NULL);
   
   /* ERE(copy_word_sense(&tmp_word_wp, word_wp));
    tmp_path_node = construct_path_node_1(NULL, tmp_word_wp);
    ASSERT(tmp_path_node != NULL);*/
    
    /* Basically, below is a breadth first search */
    ERE(insert_word_to_tree(word_wp, NULL,tree_ptr));
    ERE(insert_path_node_to_hashtable(word_wp->index, word_wp->word, 
       word_wp->sense, NULL, to_do_list, wordnet_ht_ptr));
    /*ERE(append_element_with_contents(to_do_list, tmp_path_node));
    ERE(hash_insert(wordnet_ht_ptr, (void*)tmp_path_node,
    (void*)tmp_path_node));*/
    while(1)
    {
        current_path_node = (Path_node*)remove_list_head(to_do_list);
        if(current_path_node == NULL) break;
        #ifdef TEST
        pso("\n----- WORD %s(%d)---- \n", current_path_node->word_wp->word,
                 current_path_node->word_wp->sense);
        #endif
        res = traverse_to_parent(current_path_node, vocabulary_ht_ptr,
        to_do_list, wordnet_ht_ptr, tree_ptr);

       /* print_path_node(dequeue_node_ptr);*/
        if(res == ERROR) /* found or terminated*/
        {
            kjb_print_error();
            break;
        }
    }

    /*print_hash_table(wordnet_ht_ptr, print_path_node);*/
    ASSERT(to_do_list->length == 0);
    free_linkedlist(to_do_list, NULL);
    clean_hash_table(wordnet_ht_ptr, free_path_node);
         
    return res;
}

void free_tree_array
(
   Array *tree_array_ptr
)
{
    int i;
    int len;

    if(tree_array_ptr == NULL) return;

    len = tree_array_ptr->length;
    for(i=0; i<len; i++)
    {
        free_tree(tree_array_ptr->elements[i], free);
    }
    free_array(tree_array_ptr, NULL);
}

int write_semantic_tree_result
(
    const Array *tree_array_ptr, 
    const char  *filename
)
{
    FILE *fp = NULL;
    int tree;
    int len;

    if(tree_array_ptr == NULL)
    {
        return NO_ERROR;
    }

    if((fp = fopen(filename, "w"))== NULL)
    {
        add_error("Failed to open file %s for writing!\n", filename);
        return ERROR;
    }

    len = tree_array_ptr->length;
    for(tree = 0; tree < len; tree++)
    {
        fp_print_tree(fp, (Tree *)tree_array_ptr->elements[tree], fp_print_word_sense);
    }
    
    fclose(fp);

    return NO_ERROR;
}

/* find paths to the parent node */
static int traverse_to_parent
(
    Path_node *path_node_ptr, 
    const Hash_table *vocabulary_ht_ptr, 
    Linkedlist       *to_do_list, 
    Hash_table       *wordnet_ht_ptr,
    Tree             *tree_ptr
)
{
#ifdef KJB_HAVE_WN
    SynsetPtr synset_ptr = NULL;
    Word_sense *word_wp = NULL;
    int res = NO_ERROR;
    
    word_wp = path_node_ptr->word_wp;
    synset_ptr = findtheinfo_ds(word_wp->word, NOUN, HYPERPTR, word_wp->sense);
    if(synset_ptr == NULL)
    {
        pso("Warning: WORD: [%s(%d)] doesn't not exist in the WordNet!\n",
            word_wp->word, word_wp->sense);
        return NO_ERROR;
    }
    free_synset(synset_ptr);
    
    if(PARENT_TYPE(fs_parent_type, IS_A))
    {
        /* do is_a */
        res = traverse_to_neighbor(path_node_ptr, vocabulary_ht_ptr, HYPERPTR,
            to_do_list, wordnet_ht_ptr, tree_ptr);
        if(res == ERROR) return res;
    }


    if(PARENT_TYPE(fs_parent_type, PART_OF))
    {
        /* do is_part_of */
        res = traverse_to_neighbor(path_node_ptr, vocabulary_ht_ptr, ISPARTPTR,
            to_do_list, wordnet_ht_ptr, tree_ptr);
        if(res == ERROR) return res;
    }

    if(PARENT_TYPE(fs_parent_type, MEMBER_OF))
    {
        /* do is_member_of */
        res = traverse_to_neighbor(path_node_ptr, vocabulary_ht_ptr, ISMEMBERPTR,
            to_do_list, wordnet_ht_ptr, tree_ptr);
        if(res == ERROR) return res;
    }

    if(PARENT_TYPE(fs_parent_type, INSTANCE_OF))
    {
        /* do is_member_of */
        res = traverse_to_neighbor(path_node_ptr, vocabulary_ht_ptr, ISSTUFFPTR,
            to_do_list, wordnet_ht_ptr, tree_ptr);
        if(res == ERROR) return res;
    }

    return res;
#else
    add_error("Missing dependency: WORDNET");
#endif
}

static int traverse_to_neighbor
(
    Path_node  *path_node_ptr, 
    const Hash_table *vocabulary_ht_ptr, 
    int              search_type,
    Linkedlist       *to_do_list, 
    Hash_table       *wordnet_ht_ptr,
    Tree             *tree_ptr
)
{
#ifdef KJB_HAVE_WN
    Word_sense *word_wp;
    char *word;
    int sense;
    int index;
    SynsetPtr synset_ptr = NULL;
    SynsetPtr next_synset_ptr = NULL;
    SynsetPtr trace_synset_ptr = NULL;
    Word_sense *vocabulary_word_wp = NULL;
    Path_node *prev_path_node = NULL;
    void *found_ptr;
    int res = NO_ERROR;
    int which_word;

    word_wp = path_node_ptr->word_wp;
    ASSERT(word_wp != NULL);

    /* determine the prev. node along the path */
    if(hash_find(vocabulary_ht_ptr, word_wp) != NULL)
    {
       prev_path_node = path_node_ptr;
    }
    else
    {
        prev_path_node = path_node_ptr->prev_node;
    }
    
    word = word_wp->word;
    sense = word_wp->sense;
    synset_ptr = findtheinfo_ds(word, NOUN, search_type, sense);
    
    /* recursive search to find all parents of the sensed word according
       to the search type */
    trace_synset_ptr = traceptrs_ds(synset_ptr, search_type, NOUN, 1);
    next_synset_ptr = trace_synset_ptr;
    while(next_synset_ptr != NULL)
    {
        found_ptr = is_synset_in_table_1(next_synset_ptr, prev_path_node, wordnet_ht_ptr);
        if(found_ptr == NULL)
        {
            /* need to to the check every word in the synset */
            res = is_synset_in_vocabulary(next_synset_ptr,
                vocabulary_ht_ptr, &vocabulary_word_wp);

            if(res == ERROR) break;

            if(res == 1) /* word in the vocabulary */
            {
                ERE(insert_word_to_tree(vocabulary_word_wp, prev_path_node, tree_ptr));
                word = vocabulary_word_wp->word;
                sense = vocabulary_word_wp->sense;
                index = vocabulary_word_wp->index;
                ERE(insert_path_node_to_hashtable(index, word, sense, 
                    prev_path_node, to_do_list, wordnet_ht_ptr));
            }
            else
            {
                which_word = next_synset_ptr->whichword - 1;
                if(which_word < 0) which_word = 0;
                    word = next_synset_ptr->words[which_word]; 
                    sense = next_synset_ptr->wnsns[which_word];
                    index = DEFAULT_WORD_INDEX;
                    ERE(insert_path_node_to_hashtable(index, word, sense, 
                    prev_path_node, to_do_list, wordnet_ht_ptr));
            }
        }

        next_synset_ptr = next_synset_ptr->nextss;
    }

    if(synset_ptr != NULL)
    {
        free_synset(synset_ptr);
    }
    
    if(trace_synset_ptr != NULL)
    {
        free_synset(trace_synset_ptr);
    }

    free(vocabulary_word_wp);

    if(res == ERROR)
    {
        return ERROR;
    }

    return NO_ERROR;
#else
    add_error("Missing dependency: WORDNET");
#endif
}

/* if the word is sensed, then all senses of that word is inserted into the 
   to-do list */
static int insert_word_to_tree
(
    const Word_sense *word_wp,
    const Path_node  *prev_path_node,
    Tree             *tree_ptr
)
{
    Word_sense *tmp_word_wp = NULL;

    ERE(copy_word_sense(&tmp_word_wp, word_wp));

    if(prev_path_node == NULL)
    {
        if(tree_ptr->root != NULL) /* insert as a child of the root */
        {
             if(insert_tree_node_with_contents_1(tree_ptr,
                 tree_ptr->root->contents, compare_word_sense, tmp_word_wp) == NULL)
             {
                free(tmp_word_wp);
                return ERROR;
             }
             #ifdef TEST
             pso("\n*************[%d]%s(%d) added to the TREE as a child of root.***********\n",
                 tmp_word_wp->index, tmp_word_wp->word, tmp_word_wp->sense);
             #endif
        }
        else /* insert as the root */
        {
             if(insert_tree_node_with_contents(tree_ptr, (Tree_node*)NULL, tmp_word_wp) == NULL)
             {
                free(tmp_word_wp);
                return ERROR;
             }
             #ifdef TEST
         pso("\n*************[%d]%s(%d) added to the TREE as the root.***********\n",
             tmp_word_wp->index, tmp_word_wp->word, tmp_word_wp->sense);
             #endif
        }

    }
    else
    {
        if(insert_tree_node_with_contents_1(tree_ptr, prev_path_node->word_wp,
               compare_word_sense, tmp_word_wp) == NULL)
         {
            free(tmp_word_wp);
            return ERROR;
         }
             #ifdef TEST
         pso("\n*************[%d]%s(%d) added to the TREE as the child of[%d]%s(%d).***********\n", 
                tmp_word_wp->index, tmp_word_wp->word, tmp_word_wp->sense, 
                prev_path_node->word_wp->index,
                prev_path_node->word_wp->word, 
                prev_path_node->word_wp->sense);
             #endif
    }

     return NO_ERROR;

}

static int insert_path_node_to_hashtable
(
    int              index,
    char       *word,
    int              sense,
    Path_node  *prev_path_node,
    Linkedlist       *list_ptr,
    Hash_table        *ht_ptr
)
{
#ifdef KJB_HAVE_WN
    SynsetPtr synset_ptr = NULL;
    int tmp_sense;

    if(sense < 0)
    {
        tmp_sense = 0;
        while(tmp_sense < fs_max_sense_searched)
        {
            tmp_sense++;
            synset_ptr = findtheinfo_ds(word, NOUN, HYPERPTR, tmp_sense);
            if(synset_ptr == NULL)
            {
                if(tmp_sense == 1)
                {
                     pso("Warning: WORD: [%s(%d)] doesn't not exist in the WordNet!\n",
                     word, tmp_sense);
                     return NO_ERROR;
                }

                break;
            }

            /*ERE(get_target_word_sense(&word_wp, DEFAULT_WORD_INDEX, word,
                sense, 0.0));*/
            ERE(insert_path_node_to_hashtable_1(index, word, tmp_sense, prev_path_node,
                      list_ptr, ht_ptr));
            free_synset(synset_ptr);
            synset_ptr = NULL;
        }
       /* pso("Numbe of sense: %d\n", sense-1);*/
    }
    else
    {
        ERE(insert_path_node_to_hashtable_1(index, word, sense, prev_path_node,
              list_ptr, ht_ptr));
    }

    if(synset_ptr != NULL)
    {
        free_synset(synset_ptr);
    }

    return NO_ERROR;
#else
    add_error("Missing dependency: WORDNET");
#endif
}

static int insert_path_node_to_hashtable_1
(
    int              index,
    const char       *word,
    int              sense,
    Path_node        *prev_path_node,
    Linkedlist       *list_ptr,
    Hash_table        *ht_ptr
)
{
    Path_node *path_node_ptr = NULL;
    Word_sense *tmp_word_wp = NULL;

    int res = NO_ERROR;

    ERE(get_target_word_sense(&tmp_word_wp, index, word,
        sense, 0.0));

    path_node_ptr = construct_path_node_1(prev_path_node, tmp_word_wp);
    if(path_node_ptr == NULL)
    {
        add_error("Failed to create path node !\n");
        return ERROR;
    }
     
     if(ht_ptr != NULL)
     {
         res = hash_insert(ht_ptr, (void*)path_node_ptr, (void*)path_node_ptr);
             #ifdef TEST
         if(prev_path_node != NULL)
         {
             pso("\n*************[%d]%s(%d) ---> parent:[%d]%s(%d) added to the TABLE.***********\n",
                 index, word, sense, prev_path_node->word_wp->index,
                 prev_path_node->word_wp->word, prev_path_node->word_wp->sense);
          }
          else
          {
             pso("\n*************[%d]%s(%d) ---> parent:NULL added to the TABLE.***********\n",
                 index, word, sense);
          }
             #endif
     }

     if(res != ERROR && list_ptr != NULL)
     {
        res = append_element_with_contents(list_ptr, (void*)path_node_ptr);
     }


     if(res == ERROR)
     {
         free_path_node(path_node_ptr);
     }

     return res;
}

#ifdef KJB_HAVE_WN
static Path_node *is_synset_in_table_1
(
    const SynsetPtr  synset_ptr,
    Path_node  *prev_node_ptr,
    const Hash_table *ht_ptr
)
{
    int i;
    int n;
    int sense;
    Word_sense *word_wp = NULL;
    Path_node *path_node_ptr = NULL;
    Path_node *found_node_ptr = NULL;

    char *word;
    int res = NO_ERROR;

    if(synset_ptr == NULL) return NULL;

    path_node_ptr = construct_path_node();
    ASSERT(path_node_ptr != NULL);
    path_node_ptr->prev_node = prev_node_ptr;

    n =synset_ptr->wcount;
    for(i=0; i<n; i++)
    {
        word = synset_ptr->words[i];
        sense = synset_ptr->wnsns[i];
        res = get_target_word_sense(&word_wp, DEFAULT_WORD_INDEX, word, sense,
            0.0);
        ASSERT(res != ERROR);
        path_node_ptr->word_wp = word_wp;
        found_node_ptr = (Path_node*)hash_find(ht_ptr, path_node_ptr);
        if(found_node_ptr != NULL)
        {
            break;
        }
    }

    free(word_wp);
    free(path_node_ptr);

    return found_node_ptr;
}

static int is_synset_in_vocabulary
(
    const SynsetPtr  synset_ptr,
    const Hash_table *vocabulary_ht_ptr,
    Word_sense       **word_wpp
)
{
    int i;
    int n;
    int sense;
    Word_sense *word_wp = NULL;
    Word_sense *vocabulary_word_wp = NULL;
    char *word;

    if(synset_ptr == NULL) return 0;

    n =synset_ptr->wcount;
    for(i=0; i<n; i++)
    {
        word = synset_ptr->words[i];
        sense = synset_ptr->wnsns[i];
        if(sense > fs_max_sense_searched) continue;
        ERE(get_target_word_sense(&word_wp, DEFAULT_WORD_INDEX, word, sense,
            0.0));
        vocabulary_word_wp = (Word_sense*)hash_find(vocabulary_ht_ptr, word_wp);
        if(vocabulary_word_wp != NULL) break;
    }
    
    free(word_wp);
    
    if(vocabulary_word_wp == NULL)
    {
        return 0;   
    }

    ERE(copy_word_sense(word_wpp, vocabulary_word_wp));

    return 1;
}
#endif

/* generate a dot file for graph visualization */
int generate_dot_file
(
    const Array *tree_array_ptr,
    const char *filename
)
{
    int i;
    int len;
    FILE *fp = NULL;
    Tree *tree_ptr = NULL;
    Word_sense *word_wp = NULL;
    Int_matrix *visited_imp = NULL;

    fp = kjb_fopen(filename, "w");
    if(fp == NULL) return ERROR;

    /* write the head info. */
    kjb_fprintf(fp, "digraph G {\n");
    kjb_fprintf(fp, "page=\"8.5,11\";\n");
    kjb_fprintf(fp, "margin=0;\n");
    kjb_fprintf(fp, "ratio=auto;\n");
    kjb_fprintf(fp, "pagedir=TL;\n");
    
    /* write the nodes first */
    len = tree_array_ptr->length;
    for(i=0; i<len; i++)
    {
        tree_ptr = (Tree*)tree_array_ptr->elements[i];
        word_wp = (Word_sense*)tree_ptr->root->contents;
        kjb_fprintf(fp, "n%d [label=\"%s(%d)\"];\n", i+1, word_wp->word, word_wp->sense);
    }

    ERE(get_zero_int_matrix(&visited_imp, len, len));
    /* now the edges */
    for(i=0; i<len; i++)
    {
        tree_ptr = (Tree*)tree_array_ptr->elements[i];
        print_tree_edge(fp, tree_ptr->root, visited_imp);
    }

    kjb_fprintf(fp, "}\n");

    free_int_matrix(visited_imp);
    return NO_ERROR;
}

/* The matrix is sparse so it's not a good idea here when there
   are too many words. */
static void print_tree_edge
(
    FILE            *fp,
    const Tree_node *node_ptr,
    const Int_matrix      *visited_imp
)
{
    Tree_node *child_ptr = NULL;
    Tree_node *current_node_ptr = NULL;
    int index;
    int child_index;

    if(node_ptr == NULL) return;

    child_ptr = node_ptr->children;
    if(child_ptr == NULL) return;

    index = ((Word_sense*)node_ptr->contents)->index;
    /* depth first search */
    current_node_ptr = child_ptr;
    while(current_node_ptr != NULL)
    {
        child_index = ((Word_sense*)current_node_ptr->contents)->index;
        if(visited_imp->elements[index][child_index] == 0)
        {
            kjb_fprintf(fp, "n%d -> n%d [label=1];\n", index+1, child_index+1);
            visited_imp->elements[index][child_index] = 1;
        }
        print_tree_edge(fp, current_node_ptr, visited_imp);
        current_node_ptr = current_node_ptr->next;
    }
}

#ifdef __cplusplus
}
#endif
