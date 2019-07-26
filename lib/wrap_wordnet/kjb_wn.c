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

#include "wrap_wordnet/kjb_wn.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef KJB_HAVE_WN

SynsetPtr find_synset(const char * word, int pos, int query_type, int word_sense)
{
    SynsetPtr temp;
    char * word_original = 0;
    temp =  findtheinfo_ds(word, pos, query_type, word_sense);
    if(! temp)
    {
        word_original = morphword(word, NOUN);
        if(!word_original)
        {
            return NULL;
        }

        return findtheinfo_ds(word_original, pos, query_type, word_sense);
    }
    return temp;
}

void print_synset(const SynsetPtr syn_sp, int level)
{
    int i,j;
    if(syn_sp == NULL)
    {
        printf("Null synset\r\n");
        return;
    }

    for(j = 0; j < level; j++)
    {
        printf("    ");
    }
    printf("Printing synset\r\n");
    for(i = 0; i < syn_sp->wcount; i++)
    {
        for(j = 0; j < level; j++)
        {
            printf("    ");
        }
        printf("  Word: %s\r\n", syn_sp->words[i]);
    }
}

void print_synset_list(const SynsetPtr syn_sp)
{
    SynsetPtr to_print = syn_sp;
    if(to_print == NULL)
    {
        printf("Null synset\r\n");
        return;
    }

    while(to_print)
    {
        print_synset(to_print, 0);
        to_print = to_print->nextss;
    }
}

void recursively_print_synset(const SynsetPtr syn_sp, int level)
{
    SynsetPtr to_print = syn_sp;
    if(to_print == NULL)
    {
        return;
    }

    print_synset(to_print, level);
    if(to_print->ptrlist)
    {
        recursively_print_synset(to_print->ptrlist, level +1);
    }

    to_print = to_print->nextss;
    while(to_print)
    {
        print_synset(to_print, level);
        if(to_print->ptrlist)
        {
            recursively_print_synset(to_print->ptrlist, level+1);
        }
        to_print = to_print->nextss;
    }
}

int is_word_sense_in_synset(const char * word, int sense, const SynsetPtr tosearch)
{
    int i;

    for(i = 0; i < tosearch->wcount; i++)
    {
        if(strncmp(tosearch->words[i],word,strlen(word)))
        {
            continue;
        }
        if(strncmp(tosearch->words[i],word,strlen(tosearch->words[i])))
        {
            continue;
        }
        if(sense == tosearch->wnsns[i])
        {
            return 1;
        }
    }
    return 0;
}

int recursively_find_word_in_synset(const char * word, int sense, const SynsetPtr tosearch )
{
    SynsetPtr to_print = tosearch;
    if(tosearch == NULL)
    {
        return 0;
    }

    if(is_word_sense_in_synset(word, sense, tosearch))
    {
        return 1;
    }
    if(to_print->ptrlist)
    {
        if(recursively_find_word_in_synset(word, sense, to_print->ptrlist))
        {
            return 1;
        }
    }

    to_print = to_print->nextss;
    while(to_print)
    {
        if(is_word_sense_in_synset(word, sense, to_print))
        {
            return 1;
        }
        if(to_print->ptrlist)
        {
            if(recursively_find_word_in_synset(word, sense, to_print->ptrlist))
            {
                return 1;
            }
        }
        to_print = to_print->nextss;
    }
    return 0;
}

int is_synset_in_synset(const SynsetPtr query, const SynsetPtr tosearch)
{
    return recursively_find_word_in_synset(query->words[query->whichword -1], query->wnsns[query->whichword -1], tosearch );
}

int synset_contains_word(const char* word, int pos, int sense, int query_type, const SynsetPtr tosearch)
{
    SynsetPtr query = find_synset(word, pos, query_type, sense);
    int result;
    if(!query)
    {
        return 0;
    }
    result = is_synset_in_synset(query, tosearch);
    free_synset(query);
    return result;
}

int recursively_find_word_in_label_list(const Array * words, const char * word, int sense )
{
    const Region_label * rl = 0;
    const Word_sense * ws = 0;
    int i = 0;
    int j = 0;
    SynsetPtr sptr = NULL;
    SynsetPtr rsptr = NULL;

    sptr = find_synset(word, NOUN, HYPOPTR, sense);
    if(!sptr)
    {
        /** The word does not exist in the vocabulary */
        return 0;
    }
    sptr->ptrlist = rsptr;

    rsptr = traceptrs_ds(sptr, HYPOPTR, NOUN, 1);
    sptr->ptrlist = rsptr;

    for(i = 0; i < words->length; i++)
    {
        rl = (const Region_label *) words->elements[i];

        for(j = 0; j < rl->length; j++)
        {
            ws = (const Word_sense *) rl->words[j];
            printf("%s-%d\r\n",ws->word,ws->sense);
            if(recursively_find_word_in_synset(ws->word, ws->sense, sptr))
            {
                printf("I am returning 1\r\n");
                return 1;
            }
        }
    }

    free_synset(sptr);

    return 0;
}

int recursively_find_word_in_label_file(const char * label_path, const char * word, int sense, int * is_word_in_list)
{
    Array * words = NULL;
    if(!read_region_labels_to_array(label_path, 1, 1, &words))
    {
        printf("Could not read words\r\n");
            add_error("Could not read label file\r\n");
        printf("Could not read label file\r\n");
        return ERROR;
    }

    (*is_word_in_list) = recursively_find_word_in_label_list(words, word, sense);
    printf("Value of is_w_in_l: %d\r\n",(*is_word_in_list));
    free_array(words,free);
    return NO_ERROR;
}

int recursively_find_word_in_subtree(const char * word, int sense, const char * word_to_find, int sense_to_find, int * is_word_in_subtree)
{
    SynsetPtr sptr = NULL;
    SynsetPtr rsptr = NULL;

    sptr = find_synset(word, NOUN, HYPOPTR, sense);
    if(!sptr)
    {
        /** The word does not exist in the vocabulary */
        return ERROR;
    }
    sptr->ptrlist = rsptr;

    rsptr = traceptrs_ds(sptr, HYPOPTR, NOUN, 1);
    sptr->ptrlist = rsptr;

    (*is_word_in_subtree) = recursively_find_word_in_synset(word_to_find, sense_to_find, sptr);

    free_synset(sptr);
    return NO_ERROR;
}

int init_wordnet()
{
    wninit();
    /** TODO CHECK RETURN STATUS OF MORPHINIT */
    morphinit();
    return NO_ERROR;
}


#endif


#ifdef __cplusplus
}
#endif

