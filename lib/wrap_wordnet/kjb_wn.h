
/* $Id: m_matrix.h 6352 2010-07-11 20:13:21Z kobus $ */

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

#ifndef KJB_WORDNET
#define KJB_WORDNET

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

#ifdef KJB_HAVE_WN

SynsetPtr find_synset(const char * word, int pos, int query_type, int word_sense);

void print_synset(const SynsetPtr syn_sp, int level);

void print_synset_list(const SynsetPtr syn_sp);

void recursively_print_synset(const SynsetPtr syn_sp, int level);

int is_word_sense_in_synset(const char * word, int sense, const SynsetPtr tosearch);

int recursively_find_word_in_synset(const char * word, int sense, const SynsetPtr tosearch );

int is_synset_in_synset(const SynsetPtr query, const SynsetPtr tosearch);

int synset_contains_word(const char* word, int pos, int sense, int query_type, const SynsetPtr tosearch);

int recursively_find_word_in_label_list(const Array * words, const char * word, int sense );

int init_wordnet();

int recursively_find_word_in_label_file(const char * label_path, const char * word, int sense, int * is_word_in_list);

int recursively_find_word_in_subtree(const char * word, int sense, const char * word_to_find, int sense_to_find, int * is_word_in_subtree);


#endif

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif /** Define Worndet */

