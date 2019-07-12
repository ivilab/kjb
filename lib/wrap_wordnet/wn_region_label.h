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

#ifndef WN_REGION_LABEL
#define WN_REGION_LABEL

#include <stdlib.h>
#include <stdio.h>

#include "m/m_incl.h"
#include "n/n_incl.h" 
#include "i/i_incl.h" 
#include "i/i_float.h"
#include "wrap_wordnet/wn_linkedlist.h"
#include "wrap_wordnet/wn_word_sense.h"
#include "wrap_wordnet/wn_array.h"
#include "wrap_wordnet/wn_segment.h"
#include "wrap_wordnet/wn_semantic_tree.h"

#ifdef KJB_HAVE_WN
#include "wn.h"
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

#define NOT_MORPHED     0
#define MORPHED         1

#define NOT_SENSED     0
#define SENSED         1

#define LABEL_START_SIGN_STR "# labels"
#define UNKNOWN_LABEL_STR     "unknown"

typedef struct Region_label
{
    Word_sense **words;
    int length;
} Region_label;

Region_label *construct_region_label
(
    int length
);

void print_region_label
(
    const void *label_ptr
);

void free_region_label
(
    void *label_ptr
);

int read_words_to_array
(
    const char *vocabulary_file,
    int        is_morphed,
    int        is_sensed,
    Array **vacabulary_ptr_ptr
);

int read_words_to_array_2
(
    const char *vocabulary_file,
    int        is_morphed,
    int        is_sensed,
    Array **vacabulary_ptr_ptr
);

int read_words_to_list
(
    const char *vocabulary_file,
    int        is_morphed,
    int        is_sensed,
    Linkedlist **list_ptr_ptr
);

int read_region_labels_to_array
(
    const char *label_file,
    int        is_morphed,
    int        is_sensed,
    Array      **label_array_ptr_ptr
);

int count_word_frequency
(
    const Int_matrix *word_imp,
    int              num,
    Vector           **freq_vpp
);

int parse_word_sense_from_str
(
    const char *line_str,
    int        is_morphed,
    int        is_sensed,
    Word_sense **word_wpp
);

int parse_region_label_from_str
(
    const char *line_str,
    int        is_morphed,
    int        is_sensed,
    Region_label **label_ptr_ptr
);

int get_new_words
(
    const Array *vocabulary_array_ptr,
    const Array *label_array_ptr,
    Array **new_array_ptr_ptr
);

int is_word_in_vocabulary
(
    const   Word_sense *word_wp,
    const   Array      *vocabulary_array_ptr
);

int merge_word_occurrence
(
    const Array      *vocabulary_array_ptr,
    const Int_matrix *word_occurrence_mp,
    Array            **new_array_ptr_ptr,
    Int_matrix       **new_occurrence_mpp
);

int relabel_image_by_segmentation
(
    const Int_matrix *src_segment_imp,
    const Int_matrix *dst_segment_imp,
    const Array      *src_label_array_ptr,
    Array            **dst_label_array_ptr_ptr
);

int read_filename_list
(
    const char  *list_filename,
    Array      **file_array_ptr_ptr
);

int read_images_labels
(
    const char *list_filename,
    int        is_morphed,
    int        is_sensed,
    Array      **label_array_array_ptr_ptr
);

int relabel_images
(
    const Array *label_array_array_ptr,
    const Array  *srcfile_array_ptr,
    const Array  *dstfile_array_ptr,
    Array      **newlabel_array_array_ptr_ptr
);

int get_sorted_segment_by_area
(
    const char *list_filename
);

int get_vocabulary_from_labels
(
    const Array *label_array_array_ptr, 
    Array       **word_array_ptr_ptr
);

void free_label_array_array
(
   Array *label_array_array_ptr
);

int merge_word_arrays
(
    const Array *word_array1_ptr, 
    const Array *word_array2_ptr,
    Array       **merged_array_ptr_ptr
);

int ow_remove_synonyms_in_label
(
    Region_label *label_ptr
);

int ow_remove_synonyms_in_label_array
(
    Array *label_array_ptr
);

int ow_remove_synonyms_in_label_array_array
(
    Array *label_array_array_ptr
);

int ow_remove_ancestors_in_label
(
    Region_label *label_ptr,
    int   parent_type
);

int ow_remove_ancestors_in_label_array
(
    Array *label_array_ptr,
    int   parent_type
);

int ow_remove_ancestors_in_label_array_array
(
    Array *label_array_array_ptr,
    int   parent_type
);

int index_word_array
(
    const Array *array_ptr
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
