
/* $Id: m_vec_basic.h 21522 2017-07-22 15:14:27Z kobus $ */

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

#ifndef M_VEC_BASIC_INCLUDED
#define M_VEC_BASIC_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int split_v4
(
    V_v_v_v**         target_1_vvvvpp,
    V_v_v_v**         target_2_vvvvpp,
    const V_v_v_v*    source_vvvvp,
    const Int_vector* index_list_vp
);

int split_vector_vector
(
    Vector_vector**      target_1_vvpp,
    Vector_vector**      target_2_vvpp,
    const Vector_vector* source_vvp,
    const Int_vector*    index_list_vp
);

int split_vector
(
    Vector**          target_1_vpp,
    Vector**          target_2_vpp,
    const Vector*     source_vp,
    const Int_vector* index_list_vp
);

int copy_v4(V_v_v_v** target_vvvvpp, const V_v_v_v* source_vvvvp);

int copy_v3(V_v_v** target_vvvpp, const V_v_v* source_vvvp);

int copy_indexed_vector
(
    Indexed_vector**      target_vpp,
    const Indexed_vector* source_vp
);

Vector* create_vector_copy(const Vector* source_vp);

int select_from_vector_vector
(
    Vector_vector**      target_vvpp,
    const Vector_vector* source_vvp,
    const Int_vector*    enable_vp 
);

int copy_vector_vector
(
    Vector_vector**      target_vvpp,
    const Vector_vector* source_vvp
);

int copy_vector_vector_block
(
    Vector_vector**      target_vvpp,
    const Vector_vector* source_vvp,
    int start_index,
    int length
);

int copy_vector_segment
(
    Vector**      target_vpp,
    const Vector* source_vp,
    int           start_index,
    int           length
);

#ifdef TRACK_MEMORY_ALLOCATION

#   define copy_vector(x,y)  debug_copy_vector(x,y, __FILE__, __LINE__)

    int debug_copy_vector
    (
        Vector**      target_vpp,
        const Vector* source_vp,
        const char*   file_name,
        int           line_number
    );
#else
    int copy_vector(Vector** target_vpp, const Vector* source_vp);
#endif

int ow_copy_vector
(
    Vector*       target_vp,
    int           offset,
    const Vector* source_vp
);

int concat_vectors
(
    Vector**      vpp,
    int           num_vectors,
    const Vector* vectors[]
);

int concat_vector_vectors
(
    Vector_vector**       vvpp,
    int                   num_vectors,
    const Vector_vector** vector_vectors
);

#ifdef TEST

#define verify_vector(x,y)   debug_verify_vector(x,y, __FILE__, __LINE__)

void debug_verify_vector
(
    const Vector* vp,
    Bool*         failed_ptr, 
    const char*   file_name,
    int           line_number
);

#else

#define verify_vector(x,y)

#endif

int vector_vector_from_vector(Vector_vector** target_vvpp, const Vector* source_vp, int size);

int vector_vector_from_matrix(Vector_vector** target_vvpp, const Matrix* source_mp);

int flatten_vector_vector(Vector** target_vpp, const Vector_vector* source_vvp);

int get_vector_vector_transpose(Vector_vector** target_vvpp, const Vector_vector* source_vvp);

int is_element_in_vector(int* index, const Vector* vp, double elem);

int is_vector_vector_consistent(const Vector_vector* vvp);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

