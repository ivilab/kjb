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

#include "wrap_wordnet/wn_word_sense.h"

#ifdef __cplusplus
extern "C" {
#endif

static int traverse_to_parent
(
    Word_sense *word_wp, 
    Word_sense *target_word_wp,
    int               parent_type,
    Linkedlist       *to_do_list, 
    Hash_table       *ht_ptr
);

static int traverse_to_neighbor
(
    Word_sense  *word_wp, 
    Word_sense  *target_word_wp,
    int              search_type,
    Linkedlist       *to_do_list_ptr,
    Hash_table       *ht_ptr
);

#ifdef KJB_HAVE_WN
static int is_synset_in_table_1
(
    const SynsetPtr  synset_ptr,
    const Hash_table *ht_ptr,
    Word_sense       **word_wpp
);
#endif

Word_sense *create_word_sense
(    
     int        index,
     const char *word, 
     int        sense,
     double     value
)
{
    Word_sense *word_wp;
    
    word_wp = (Word_sense*)malloc(sizeof(Word_sense));
    if(word_wp == NULL)
    {
        return NULL;
    }

    word_wp->index = index;
    strcpy(word_wp->word, word);
    word_wp->sense = sense;
    word_wp->value = value;

    return word_wp;
}

int get_target_word_sense
(
    Word_sense **word_wpp,
    int        index,
    const char *word,
    int        sense,
    double     value
)
{
    if(*word_wpp == NULL)
    {
        *word_wpp = create_word_sense(index, word, sense, value);
        if(*word_wpp == NULL)
        {
            add_error("failed to create word sense!\n");
            return ERROR;
        }
    }
    else
    {
        (*word_wpp)->index = index;
        strcpy((*word_wpp)->word, word);
        (*word_wpp)->sense = sense;
        (*word_wpp)->value = value;
    }

    return NO_ERROR;
}

int copy_word_sense
(
    Word_sense       **target_word_wpp,
    const Word_sense *source_word_wp
)
{
    int index = source_word_wp->index;
    int sense = source_word_wp->sense;
    double value = source_word_wp->value;
    const char *word = source_word_wp->word;

    ERE(get_target_word_sense(target_word_wpp, index, word, sense, value));

    return NO_ERROR;
}

int compare_word_sense
(
    const void *word1_wp,
    const void *word2_wp
)
{
    Word_sense *wp1;
    Word_sense *wp2;

    wp1 = (Word_sense*)word1_wp;
    wp2 = (Word_sense*)word2_wp;

    if(wp1 == NULL || wp2 == NULL)
    {
        return 0;
    }

    /* match any sense */
    if(wp1->sense < 0 || wp2->sense < 0)
    {
        if(strcasecmp(wp1->word, wp2->word) == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    if(wp1->sense == wp2->sense &&
         strcasecmp(wp1->word, wp2->word) == 0)
     {
         return 1;
     }
     else
     {
         return 0;
     }
}

void print_word_sense
(
    const void *word_wp
)
{
    Word_sense *wp = (Word_sense*)word_wp;

    if(wp == NULL) return;

    pso("[%4d] %-12s(%2d)#%8.4f", wp->index, 
       wp->word, wp->sense, wp->value);
}

void fp_print_word_sense
(
    FILE       *fp,
    const void *word_wp
)
{
    Word_sense *wp = (Word_sense*)word_wp;

    if(wp == NULL) return;

    fprintf(fp, "[%4d] %-12s(%2d)#%8.4f", wp->index, 
       wp->word, wp->sense, wp->value);
}

int hash_word_sense
(
    const void *word_wp
)
{
#ifdef KJB_HAVE_WN
    int value = 0;
    unsigned int c;
    char *word;

    if(word_wp == NULL)
    {
        return -1;
    }

    word = ((Word_sense*)word_wp)->word;
    if(word == NULL) return -1;
    
    ToLowerCase(word);

    while (c = (unsigned int)(*(word++)))
    {
       value = value * 11 + c;
    }

    if (value < 0) value = -value;

    return value;
#else 
    set_dont_have_wn_error();
    return ERROR; 
#endif 
}

#ifdef KJB_HAVE_WN
int is_word_in_synset
(
    const SynsetPtr  synset_ptr,
    const Word_sense *word_wp
)
{
    char *word;
    int i;
    int n;
    int sense;
    int res = 0;

    if(synset_ptr == NULL) return 0;

    n =synset_ptr->wcount;
    for(i=0; i<n; i++)
    {
        word = synset_ptr->words[i];
        sense = synset_ptr->wnsns[i];
        if(strcmp(word, word_wp->word) == 0 && sense == word_wp->sense)
        {
            res = 1;
            break;
        }
    }
    
    return res;
}
#endif

int is_synset_in_table
(
    Word_sense *word_wp,
    const Hash_table *ht_ptr,
    Word_sense       **word_wpp
)
{
#ifdef KJB_HAVE_WN
    SynsetPtr synset_ptr = NULL;
    Word_sense *found_word_wp = NULL;
    int res;

    if(word_wp == NULL || word_wp->word == NULL)
    {
        return 0;
    }
    
    synset_ptr = findtheinfo_ds(word_wp->word, NOUN, HYPERPTR, word_wp->sense);

    /* It is not in the wordnet. so search directly in the table */
    if(synset_ptr == NULL)
    {
        found_word_wp = (Word_sense*)hash_find(ht_ptr, word_wp);
        if(found_word_wp != NULL)
        {
            if(word_wpp != NULL)
            {
                ERE(copy_word_sense(word_wpp, found_word_wp));
            }

            return 1;
        }

        return 0;
    }

    res = is_synset_in_table_1(synset_ptr, ht_ptr, word_wpp);
    free_synset(synset_ptr);

    return res;
#else
    add_error("Missing dependency: WORDNET");
    return ERROR;
#endif
}

int is_synonym
(
    Word_sense *word1_wp,
    Word_sense *word2_wp
)
{
#ifdef KJB_HAVE_WN
    SynsetPtr synset_ptr = NULL;
    int res = 0;

    synset_ptr = findtheinfo_ds(word1_wp->word, NOUN, HYPERPTR, word1_wp->sense);
    if(synset_ptr == NULL) return 0;

    res = is_word_in_synset(synset_ptr, word2_wp);
    free_synset(synset_ptr);

    return res;
#else
    add_error("Missing dependency: WORDNET");
    return ERROR;
#endif
}

int compare_word_synonym
(
    const void *word1_wp,
    const void *word2_wp
)
{
    Word_sense *wp1;
    Word_sense *wp2;

    wp1 = (Word_sense*)word1_wp;
    wp2 = (Word_sense*)word2_wp;

    if(wp1 == NULL || wp2 == NULL)
    {
        return 0;
    }

    if(compare_word_sense(wp1, wp2)) return 1;
    if(is_synonym(wp1, wp2)) return 1;
 
    return 0;
}

/* is word 2 an ancestor of word 1?*/
int is_ancestor
(
    Word_sense *word1_wp,
    Word_sense *word2_wp,
    int              parent_type
)
{
    Hash_table *ht_ptr = NULL;
    Linkedlist *to_do_list = NULL;
    Word_sense *tmp_word_wp = NULL;
    Word_sense *current_word_wp = NULL;
    int res = NO_ERROR;
    
    #ifdef TEST
    pso("\nSTART doing: [%d]%s(%d) ----- [%d]%s(%d) \n",
                  word1_wp->index, word1_wp->word, word1_wp->sense,
                  word2_wp->index, word2_wp->word, word2_wp->sense);
    #endif

    /* create a hashtable */
    ht_ptr = create_hash_table(DEFAULT_HASH_SIZE * 10);
    ASSERT(ht_ptr != NULL);

    set_hash_func(ht_ptr, hash_word_sense);
    set_hash_comp_func(ht_ptr, compare_word_sense);

    /* create a queue */
    to_do_list = create_linkedlist();
    ASSERT(to_do_list != NULL);
   
    
    /* Basically, below is a breadth first search */
    ERE(copy_word_sense(&tmp_word_wp, word1_wp));
    ERE(hash_insert(ht_ptr, (void*)tmp_word_wp, (void*)tmp_word_wp));
    ERE(append_element_with_contents(to_do_list, (void*)tmp_word_wp));
    #ifdef TEST
    pso("\n *************[%d]%s(%d) added to the TABLE.***********\n",
                  tmp_word_wp->index, tmp_word_wp->word, tmp_word_wp->sense);
    #endif
    while(1)
    {
        current_word_wp = (Word_sense*)remove_list_head(to_do_list);
        if(current_word_wp == NULL) /* not found ! */
        {
             res = 0;
             break;
        }

        #ifdef TEST
        pso("\n----- WORD %s(%d)---- \n", current_word_wp->word,
                 current_word_wp->sense);
        #endif
        res = traverse_to_parent(current_word_wp, word2_wp, parent_type, to_do_list, ht_ptr);

        if(res == ERROR) /* found or terminated*/
        {
            kjb_print_error();
            break;
        }
    
        if(res == 1)
        {
            break;
        }
    }

    ASSERT(to_do_list->length == 0);
    free_linkedlist(to_do_list, NULL);
    clean_hash_table(ht_ptr, free);
         
    return res;
}

static int traverse_to_parent
(
    Word_sense *word_wp, 
    Word_sense *target_word_wp,
    int              parent_type,
    Linkedlist       *to_do_list, 
    Hash_table       *ht_ptr
)
{
#ifdef KJB_HAVE_WN 
    int res = NO_ERROR;
    
    if(PARENT_TYPE(parent_type, IS_A))
    {
        /* do is_a */
        res = traverse_to_neighbor(word_wp, target_word_wp, HYPERPTR,
            to_do_list, ht_ptr);
        if(res == ERROR || res == 1) return res;
    }


    if(PARENT_TYPE(parent_type, PART_OF))
    {
        /* do is_part_of */
        res = traverse_to_neighbor(word_wp, target_word_wp, ISPARTPTR,
            to_do_list, ht_ptr);
        if(res == ERROR || res == 1) return res;
    }

    if(PARENT_TYPE(parent_type, MEMBER_OF))
    {
        /* do is_member_of */
        res = traverse_to_neighbor(word_wp, target_word_wp, ISMEMBERPTR,
            to_do_list, ht_ptr);
        if(res == ERROR || res == 1) return res;
    }

    if(PARENT_TYPE(parent_type, INSTANCE_OF))
    {
        /* do is_member_of */
        res = traverse_to_neighbor(word_wp, target_word_wp, ISSTUFFPTR,
            to_do_list, ht_ptr);
        if(res == ERROR || res == 1) return res;
    }

    return res;
#else
   add_error("Missing dependency: WORDNET");
   return ERROR;
#endif
}

static int traverse_to_neighbor
(
    Word_sense  *word_wp, 
    Word_sense  *target_word_wp,
    int              search_type,
    Linkedlist       *to_do_list_ptr,
    Hash_table       *ht_ptr
)
{
#ifdef KJB_HAVE_WN
    SynsetPtr synset_ptr = NULL;
    SynsetPtr next_synset_ptr = NULL;
    SynsetPtr trace_synset_ptr = NULL;
    Word_sense *tmp_word_wp = NULL;
    char *word;
    int which_word;
    int sense;
    int res = 0;

    if(word_wp == NULL || word_wp->word == NULL)
    {
        add_error("traverse_to_neighbor: word_wp == NULL or word_wp->word == NULL\n");
        return ERROR;
    }

    word = word_wp->word;
    sense = word_wp->sense;
    
    synset_ptr = findtheinfo_ds(word, NOUN, search_type, sense);
    if(synset_ptr == NULL)
    {
        return res;
    }
    
    /* recursive search to find all parents of the sensed word according
       to the search type */
    trace_synset_ptr = traceptrs_ds(synset_ptr, search_type, NOUN, 1);
    next_synset_ptr = trace_synset_ptr;
    while(next_synset_ptr != NULL)
    {
        if(is_word_in_synset(next_synset_ptr, target_word_wp))
        {
            res = 1;
            break;
        }
        else if(!is_synset_in_table_1(next_synset_ptr, ht_ptr, NULL))
        {
            which_word = next_synset_ptr->whichword - 1;
            if(which_word < 0) which_word = 0;
            word = next_synset_ptr->words[which_word]; 
            sense = next_synset_ptr->wnsns[which_word];
            ERE(get_target_word_sense(&tmp_word_wp, DEFAULT_WORD_INDEX,
                word, sense, 0.0));
            ERE(hash_insert(ht_ptr, (void*)tmp_word_wp, (void*)tmp_word_wp));
            ERE(append_element_with_contents(to_do_list_ptr, (void*)tmp_word_wp));
            #ifdef TEST
             pso("\n*************[%d]%s(%d) added to the TABLE.***********\n",
                  tmp_word_wp->index, tmp_word_wp->word, tmp_word_wp->sense);
            #endif
            tmp_word_wp = NULL;
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

    if(res == ERROR)
    {
        return ERROR;
    }

    return res;
#else
   add_error("Missing dependency: WORDNET");
   return ERROR;
#endif
}

#ifdef KJB_HAVE_WN
static int is_synset_in_table_1
(
    const            SynsetPtr  synset_ptr,
    const Hash_table *ht_ptr,
    Word_sense       **word_wpp
)
{
    int i;
    int n;
    int sense;
    Word_sense *word_wp = NULL;
    Word_sense *found_word_wp = NULL;

    char *word;
    int res = 0;

    if(synset_ptr == NULL) return NO_ERROR;

    n =synset_ptr->wcount;
    for(i=0; i<n; i++)
    {
        word = synset_ptr->words[i];
        sense = synset_ptr->wnsns[i];
        res = get_target_word_sense(&word_wp, DEFAULT_WORD_INDEX, word, sense,
            0.0);
        ASSERT(res != ERROR);
        found_word_wp = (Word_sense*)hash_find(ht_ptr, word_wp);
        if(found_word_wp != NULL)
        {
            res = 1;
            if(word_wpp != NULL)
            {
                ERE(copy_word_sense(word_wpp, found_word_wp));
            }
            break;
        }
    }

    free(word_wp);

    return res;
}
#endif

#ifdef __cplusplus
}
#endif

