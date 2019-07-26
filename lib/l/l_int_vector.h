
/* $Id: l_int_vector.h 21445 2017-06-28 20:39:44Z kobus $ */

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

#ifndef L_INT_LIST_INCLUDED
#define L_INT_LIST_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {


#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                   Int_vector
 *
 * Integer list type
 *
 * This type is the int_vector type for the KJB library
 *
 * Related:
 *    get_target_int_vector, ra_get_target_int_vector, free_int_vector
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Int_vector
{
    int  length;
    int  max_length;
    int* elements;
}
Int_vector;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

#    define SET_INT_VEC_LENGTH(vp,length) \
        UNTESTED_CODE(); \
        \
        if ((length < 0) || (length > vp->max_length)) \
        { \
            SET_BOUNDS_BUG() \
        } \
        else \
        { \
            vp->length = length; \
        }


#    define INT_VEC_VAL(vp,i)     debug_int_vec_val(vp, i,  __FILE__, __LINE__)

     double debug_int_vec_val
     (
         const Int_vector* vp,
         int           i,
         const char*   file_name,
         int           line_number
     );

#else
        /*  Production versions  */

#    define SET_INT_VEC_LENGTH(vp,length) \
            vp->length = length;


#    define SET_NUM_VEC_COLS(vp,length) \
            vp->num_cols = num_cols;

#    define INT_VEC_VAL(vp,i)  (vp->elements[ i ])


#endif
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   Int_vector_vector
 *
 * Vector of int vectors type
 *
 * A type which implements a vector of integer vectors.
 *
 * Index: integer vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Int_vector_vector
{
    int          length;
    Int_vector** elements;
}
Int_vector_vector;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 Int_v_v_v
 *
 * Vector of Int_vector_vector's
 *
 * A type which implements a vector of Int_vector_vector's, or if you like a vector
 * of vectors of vectors. (More useful than you may guess).
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Int_v_v_v
{
    int             length;
    Int_vector_vector** elements;
}
Int_v_v_v;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

typedef struct Indexed_int_vector_element
{
    int element;
    int index;
}
Indexed_int_vector_element;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

typedef struct Indexed_int_vector
{
    int                         length;
    Indexed_int_vector_element* elements;
}
Indexed_int_vector;

/* -------------------------------------------------------------------------- */

int get_zero_int_vector(Int_vector** mpp, int len);
int get_unity_int_vector(Int_vector** vpp, int len);

int get_initialized_int_vector
(
    Int_vector** mpp,
    int          len,
    int          initial_value
);


#ifdef TRACK_MEMORY_ALLOCATION
#   define create_int_vector(x)    debug_create_int_vector(x, __FILE__, __LINE__)

#   define get_target_int_vector(x,y)   \
                               debug_get_target_int_vector(x,y, __FILE__, __LINE__)

#   define ra_get_target_int_vector(x,y)   \
                               debug_ra_get_target_int_vector(x,y, __FILE__, __LINE__)


    Int_vector* debug_create_int_vector
    (
        int         len,
        const char* file_name,
        int         line_number
    );

    int debug_get_target_int_vector
    (
        Int_vector** target_mpp,
        int          length,
        const char*  file_name,
        int          line_number
    );

    int debug_ra_get_target_int_vector
    (
        Int_vector** target_mpp,
        int          length,
        const char*  file_name,
        int          line_number
    );
#else
    Int_vector* create_int_vector(int len);
    int get_target_int_vector(Int_vector** out_arg_mp, int length);
    int ra_get_target_int_vector(Int_vector** out_arg_mp, int length);
#endif

void        free_int_vector       (Int_vector* vp);
#if 0 /* was ifdef OBSOLETE */
Int_vector* create_zero_int_vector(int);
#endif 
int         ow_zero_int_vector    (Int_vector*);

int ow_set_int_vector              (Int_vector*, int);
int ascend_sort_int_vector         (Int_vector* vp);
int ascend_sort_indexed_int_vector (Indexed_int_vector* vp);
int descend_sort_indexed_int_vector(Indexed_int_vector* vp);

int vp_get_indexed_int_vector
(
    Indexed_int_vector** vpp,
    const Int_vector*    init_vp
);

int get_zero_indexed_int_vector    (Indexed_int_vector** vpp, int length);
int get_target_indexed_int_vector  (Indexed_int_vector** vpp, int length);

Indexed_int_vector* create_indexed_int_vector      (int len);
Indexed_int_vector* vp_create_indexed_int_vector   (const Int_vector* vp);

void free_indexed_int_vector(Indexed_int_vector* vp);

int add_int_vectors
(
    Int_vector**      target_vpp,
    const Int_vector* first_vp,
    const Int_vector* second_vp
);

int subtract_int_vectors
(
    Int_vector**      target_vpp,
    const Int_vector* first_vp,
    const Int_vector* second_vp
);

int ow_add_int_vectors(Int_vector* first_vp, const Int_vector* second_vp);
int ow_subtract_int_vectors(Int_vector* first_vp, const Int_vector* second_vp);

int max_int_vector_element(const Int_vector* vp);
int min_int_vector_element(const Int_vector* vp);

#if 0 /* was ifdef OBSOLETE */
Int_vector* create_int_vector_copy(Int_vector* source_mp);
#endif 

int get_random_index_vector
(
     Int_vector** target_vpp,
     int      length,
     double   fraction
);

int split_int_vector_vector
(
    Int_vector_vector**      target_1_vvpp,
    Int_vector_vector**      target_2_vvpp,
    const Int_vector_vector* source_vvp,
    const Int_vector*        index_list_vp
);

int split_int_vector
(
    Int_vector** target_1_vpp,
    Int_vector** target_2_vpp,
    const Int_vector* source_vp,
    const Int_vector* index_list_vp
);

int                copy_int_vector_vector
(
    Int_vector_vector**      target_vvpp,
    const Int_vector_vector* source_vvp
);

int                copy_int_vector
(
    Int_vector**      target_vpp,
    const Int_vector* source_vp
);

int                copy_int_vector_section
(
    Int_vector**      target_vpp,
    const Int_vector* source_vp,
    int               first_elem,
    int               length
);

int read_int_vector
(
    Int_vector** vpp,
    const char*  file_name
);

int fp_read_int_vector            (Int_vector** vpp, FILE* fp);
int fp_read_int_vector_with_header(Int_vector** vpp, FILE* fp);
int fp_read_raw_int_vector        (Int_vector** result_vpp, FILE* fp);
int fp_read_ascii_int_vector      (Int_vector** result_vpp, FILE* fp);
int fp_read_vector_length_header  (FILE* fp, int* length_ptr);

int write_col_int_vector
(
    const Int_vector* vp,
    const char*       file_name
);

int fp_write_col_int_vector           (const Int_vector* vp, FILE* fp);

int fp_write_col_int_vector_with_title
(
    const Int_vector* vp,
    FILE*             fp,
    const char*       title
);

int write_col_int_vector_with_header(const Int_vector* vp, const char* file_name);

int fp_write_col_int_vector_with_header(const Int_vector* vp, FILE* fp);

int write_row_int_vector
(
    const Int_vector* vp,
    const char*       file_name
);

int fp_write_row_int_vector           (const Int_vector* vp, FILE* fp);

int fp_write_row_int_vector_with_title
(
    const Int_vector* vp,
    FILE*             fp,
    const char*       title
);

int fp_write_vector_length_header     (FILE* fp, int length);

int write_raw_int_vector
(
    const Int_vector* vp,
    const char*       file_name
);

int fp_write_raw_int_vector           (const Int_vector* vp, FILE* fp);

int                get_target_int_vector_vector
(
    Int_vector_vector** target_vvpp,
    int                 count
);

void               free_int_vector_vector
(
    Int_vector_vector* vvp
);

int get_target_int_v3(Int_v_v_v** target_vvvpp, int length);

void free_int_v3(Int_v_v_v* vvvp);

int                concat_int_vectors
(
    Int_vector**      vpp,
    int               num_vectors,
    const Int_vector* vectors[]
);

int                int_vector_linear_search
(
    const Int_vector* vp,
    int               search_elem
);

int                int_vector_binary_search
(
    const Int_vector* vp,
    int               search_elem
);

int                sum_int_vector_elements
(
    const Int_vector* vp
);

int                read_int_vector_vector
(
    Int_vector_vector** result_vvpp,
    const char*         file_name
);

int                fp_read_int_vector_vector
(
    Int_vector_vector** result_vvpp,
    FILE*               fp
);

int                fp_read_raw_int_vector_vector
(
    Int_vector_vector** vvpp,
    FILE*               fp
);

int                fp_read_formatted_int_vector_vector
(
    Int_vector_vector** result_vvpp,
    FILE*               fp
);

int                write_int_vector_vector
(
    const Int_vector_vector* vvp,
    const char*              file_name
);

int                fp_write_int_vector_vector
(
    const Int_vector_vector* vvp,
    FILE*                    fp
);

int                write_raw_int_vector_vector
(
    const Int_vector_vector* vvp,
    const char*                  file_name
);

int                fp_write_raw_int_vector_vector
(
    const Int_vector_vector* vvp,
    FILE*                        fp
);

int                fp_write_raw_int_vector_vector_header
(
    int   num_vectors,
    FILE* fp
);

int                concat_int_vector_vectors
(
    Int_vector_vector**       vvpp,
    int                       num_vectors,
    const Int_vector_vector** int_vector_vectors
);

int sget_int_vector             (Int_vector** vpp, const char* line);
int sget_positive_int_vector    (Int_vector** vpp, const char* line);
int sget_non_negative_int_vector(Int_vector** vpp, const char* line);

int output_int_vector
(
    const char*       output_dir,
    const char*       file_name,
    const Int_vector* vp
);

int output_raw_int_vector
(
    const char*       output_dir,
    const char*       file_name,
    const Int_vector* vp
);

int output_int_vector_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Int_vector_vector* vvp
);

int ow_copy_int_vector
(
    Int_vector*         target_vp,
    int                 offset,
    const Int_vector*   source_vp
);

int get_max_int_vector_element(const Int_vector* vp, int* max_ptr);

int get_min_int_vector_element(const Int_vector* vp, int* max_ptr);

int multiply_int_vector_by_int_scalar
(
    Int_vector**        target_vpp,
    const Int_vector*   source_vp,
    int                 scalar
);

int ow_multiply_int_vector_by_int_scalar(Int_vector* source_vp, int scalar);

int add_int_scalar_to_int_vector
(
    Int_vector**        target_vpp,
    const Int_vector*   source_vp,
    int                 scalar
);

int int_set_difference
(
    Int_vector**      diff,
    const Int_vector* s1,
    const Int_vector* s2
);

int is_element_in_int_vector(int* index, const Int_vector* vp, int elem);

int get_int_dot_product
(
    const Int_vector* vp1,
    const Int_vector* vp2,
    long int*       result_ptr
);

double int_vector_magnitude(const Int_vector* vp);
long int sum_int_vector_squared_elements(const Int_vector* vp);

int check_same_int_vector_lengths
(
    const Int_vector* first_vp,
    const Int_vector* second_vp,
    const char*   context_str
);

int max_abs_int_vector_difference
(
    const Int_vector* first_vp,
    const Int_vector* second_vp
);

int int_vector_is_permutation(
    const Int_vector* vp,
    int start_value,
    int* result
);

int get_string_why_int_vector_is_not_permutation(
    const Int_vector* vp,
    int start_value,
    char *out_buf,
    size_t buf_size
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

