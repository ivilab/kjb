
/* $Id: l_word_list.h 15639 2013-10-10 21:18:59Z predoehl $ */

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

#ifndef L_WORD_LIST_INCLUDED
#define L_WORD_LIST_INCLUDED


#include "l/l_def.h"
#include "l/l_int_vector.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   Word_list
 *
 * KJB library string vector type
 *
 * This type is for vectors of strings. 
 *
 * Index: strings, word lists, data types
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Word_list
{
    int num_words;   /* Length of vector */
    char** words;    /* Pointer to string pointers */
}
Word_list;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int  get_target_word_list   
(
    Word_list** word_list_ptr_ptr,
    int         num_words 
);

int  ra_get_target_word_list
(
    Word_list** target_word_list_ptr_ptr,
    int         num_words 
);

void free_word_list         (Word_list* word_list_ptr);

int append_word_list(Word_list** word_list_ptr_ptr, const char* word);

int  select_from_word_list  
(
    Word_list**       target_word_list_ptr_ptr,
    const Word_list*  source_word_list_ptr,
    const Int_vector* enable_vp 
);

int  copy_word_list         
(
    Word_list**      target_word_list_ptr_ptr,
    const Word_list* word_list_ptr 
);

int copy_word(char** target_word_ptr_ptr, const char* source_word_ptr);

int count_strings_in_word_list( const Word_list* word_list_ptr );

int trim_word_list_empty_entries_at_tail( Word_list* word_list_ptr );

int sort_word_list
(
    Word_list**      target_word_list_ptr_ptr,
    const Word_list* word_list_ptr 
);

int  search_word_list       
(
    const Word_list* word_list_ptr,
    const char*      search_str 
);

int search_sorted_word_list
(
    const Word_list* word_list_ptr,
    const char*      search_str 
);

int  concat_word_lists      
(
    Word_list** target_word_list_ptr_ptr,
    int         num_word_lists,
    const Word_list* const* word_list_ptr_array 
);

int  read_word_list         
(
    Word_list** word_list_ptr_ptr,
    const char* file_name 
);

int  fp_read_word_list      (Word_list** word_list_ptr_ptr, FILE* fp);

int  sget_word_list         
(
    Word_list** word_list_ptr_ptr,
    const char* line,
    const char* delimiters 
);

int  write_word_list        
(
    const Word_list* word_list_ptr,
    const char*      file_name 
);

int  fp_write_word_list     (const Word_list* word_list_ptr, FILE* fp);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

