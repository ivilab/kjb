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

#ifndef WN_WORD_SENSE
#define WN_WORD_SENSE

#include "m/m_incl.h"
#include "n/n_incl.h" 
#include "i/i_incl.h" 
#include "i/i_float.h"
#include "wrap_wordnet/wn_linkedlist.h"
#include "wrap_wordnet/wn_hash.h"

#ifdef KJB_HAVE_WN
#include "wn.h"
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define MAX_WORD_LENGTH  1024
#define MAX_WORD_SENSE   100

#define DEFAULT_WORD_INDEX -1
#define NOT_USED_INDEX    -9999

#define IS_A        0x0001
#define PART_OF     0x0010
#define MEMBER_OF   0x0100
#define INSTANCE_OF 0x1000


#define PARENT_TYPE(x,t)   ((x&t) == t)

typedef struct Word_sense
{
    int    index;                   /* which word. identifier of a word*/
    char   word[MAX_WORD_LENGTH];
    int    sense;
    double value;                   /* can store anything useful */
} Word_sense;

Word_sense *create_word_sense
(
    int        index,
    const char *word, 
    int        sense,
    double     value
);

int get_target_word_sense
(
    Word_sense **word_wpp,
    int        index,
    const char *word,
    int        sense,
    double     value
);

int copy_word_sense
(
    Word_sense       **target_word_wpp,
    const Word_sense *source_word_wp
);

int compare_word_sense
(
    const void *word_wp1,
    const void *word_wp2
);

void print_word_sense
(
    const void *word_wp
);

void fp_print_word_sense
(
    FILE       *fp,
    const void *word_wp
);

int hash_word_sense
(
    const void *word_wp
);

#ifdef KJB_HAVE_WN
int is_word_in_synset
(
    const SynsetPtr  synset,
    const Word_sense *word_wp
);
#endif

int is_synset_in_table
(
    Word_sense *word_wp,
    const Hash_table *ht_ptr,
    Word_sense       **word_wpp
);

int is_synonym
(
    Word_sense *word1_wp,
    Word_sense *word2_wp
);

int is_ancestor
(
    Word_sense *word1_wp,
    Word_sense *word2_wp,
    int              parent_type
);

int compare_word_synonym
(
    const void *word1_wp,
    const void *word2_wp
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
