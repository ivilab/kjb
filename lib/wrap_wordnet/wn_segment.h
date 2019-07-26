
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

#ifndef WN_SEGMENT
#define WN_SEGMENT

#include <stdlib.h>
#include <stdio.h>

#include "m/m_incl.h"
#include "n/n_incl.h" 
#include "i/i_incl.h" 
#include "i/i_float.h"
#include "wrap_wordnet/wn_array.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

#define MAX_LINE_LENGTH   2048

#define HUMAN_SEGMENTATION       1
#define MACHINE_SEGMENTATION     2 

int set_human_seg_stripped_rows
(
    int rows
);

int set_human_seg_stripped_cols
(
    int cols
);

int set_machine_seg_stripped_rows
(
    int rows
);

int set_machine_seg_stripped_cols
(
    int cols
);

int read_segmentation
(
    Int_matrix **segment_impp,
    const char *seg_file,
    int        seg_type
);

int read_Corel_segmentation
(
    Int_matrix **segment_impp,
    const char *seg_file
);

int read_UCB_segmentation
(
    Int_matrix **segment_impp,
    const char *seg_file
);

int resize_segmentation_bitmap
(
    const Int_matrix *src_imp,
    int              dst_height,
    int              dst_width,
    Int_matrix       **dst_impp
);

int sort_segment_by_area
(
    const Int_matrix *segment_imp,
    Int_vector **segment_index_ivpp,
    Int_vector **segment_area_ivpp
);

int get_segment_index
(
    const Array       *segfile_array_ptr,
    Int_vector_vector **segment_index_ivvpp
);

int read_UCB_num_segments
(
    int * num_segments,
    const char *seg_file
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
