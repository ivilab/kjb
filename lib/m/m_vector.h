
/* $Id: m_vector.h 21448 2017-06-28 22:00:33Z kobus $ */

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

#ifndef M_VECTOR_INCLUDED
#define M_VECTOR_INCLUDED


#include "l/l_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
#    ifdef PROGRAMMER_IS_kobus
         /* This a bit dangerous as often freeing non-initialized memory is not
          * really a bug. */
#        define CHECK_VECTOR_INITIALZATION_ON_FREE
#    endif
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                                   Vector
 *
 * Vector type
 *
 * This type is the vector type for the KJB library and is used by many
 * routines.
 *
 * Related:
 *    free_vector, get_target_vector
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Vector
{
    int   length;
    int   max_length;
    double* elements;
}
Vector;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

#    define SET_VEC_LENGTH(vp,length) \
        UNTESTED_CODE(): \
        \
        if ((length < 0) || (length > vp->max_length)) \
        { \
            SET_BOUNDS_BUG() \
        } \
        else \
        { \
            vp->length = length; \
        }


#    define VEC_VAL(vp,i)     debug_vec_val(vp, i,  __FILE__, __LINE__)

     double debug_vec_val
     (
         const Vector* vp,
         int           i,
         const char*   file_name,
         int           line_number
     );

#else
        /*  Production versions  */

#    define SET_VEC_SIZE(vp,length,num_cols) \
            vp->length = length; \
            vp->num_cols = num_cols;


#    define SET_VEC_LENGTH(vp,length) \
            vp->length = length;


#    define SET_NUM_VEC_COLS(vp,length) \
            vp->num_cols = num_cols;

#    define VEC_VAL(vp,i)  (vp->elements[ i ])


#endif


/* -------------------------------------------------------------------------- */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   Vector_vector
 *
 * Vector of vectors type
 *
 * A type which implements a vector of vectors.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Vector_vector
{
    int      length;
    Vector** elements;
}
Vector_vector;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 V_v_v
 *
 * Vector of Vector_vector's
 *
 * A type which implements a vector of Vector_vector's, or if you like a vector
 * of vectors of vectors. (More useful than you may guess).
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct V_v_v
{
    int             length;
    Vector_vector** elements;
}
V_v_v;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 V_v_v_v
 *
 * Vector of V_v_v's
 *
 * A type which implements a vector of V_v_v's, or if you like a vector
 * of vectors of vectors of vectors. Any more levels should probably be done as a generic
 * object for arbitrary number of levels. However, this structure is ocasionaly
 * useful and is currently used in the clustering code.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct V_v_v_v
{
    int     length;
    V_v_v** elements;
}
V_v_v_v;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                Indexed_vector_element
 *
 * Structure for Indexed_vector
 *
 * This type is the structure from which Indexed_vector is built.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Indexed_vector_element
{
    double element;
    int  index;
}
Indexed_vector_element;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   Indexed_vector
 *
 * Indexed vector type
 *
 * This type is an allocated vector of type Indexed_vector_element which is a
 * structure of two members: element, and index. Two typical uses of this data
 * type is to access N elements in random order, but without duplications, and
 * to sort a vector while keeping track of where the elements were originally
 * (as that might be an index into other data structures). The first use is
 * accomplished using the routines get_random_indexed_vector(), followed by
 * ascend_sort_indexed_vector(), followed by stepping throught the index values
 * of the now sorted, originally random vector. The second task is accomplished
 * by creating an indexed version of the vector with vp_get_indexed_vector(),
 * and doing either an ascend_sort_indexed_vector() or a
 * descend_sort_indexed_vector(), and then accessing the elements in order, but
 * using the index member of the structure to link to other information
 * regarding those elements.
 *
 * Index: vectors
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Indexed_vector
{
    int                     length;
    Indexed_vector_element* elements;
}
Indexed_vector;

/* -------------------------------------------------------------------------- */

int get_unity_vector(Vector** vpp, int len);
int get_zero_vector(Vector** vpp, int len);

int get_initialized_vector(Vector** vpp, int len, double initial_value);

#ifdef TRACK_MEMORY_ALLOCATION

#   define create_vector(x)    debug_create_vector(x, __FILE__, __LINE__)

#   define get_target_vector(x,y)   \
                               debug_get_target_vector(x,y, __FILE__, __LINE__)
#   define allocate_2D_vp_array(x, y) \
                 debug_allocate_2D_vp_array((x), (y), __FILE__, __LINE__)


    Vector* debug_create_vector
    (
        int         len,
        const char* file_name,
        int         line_number
    );

    int debug_get_target_vector
    (
        Vector**    target_vpp,
        int         length,
        const char* file_name,
        int         line_number
    );

    Vector*** debug_allocate_2D_vp_array(int, int, const char*, int);
#else
    Vector* create_vector(int len);
    int get_target_vector(Vector** out_arg_vp, int length);
    Vector*** allocate_2D_vp_array(int, int);
#endif

void free_vector(Vector* vp);
Vector* create_zero_vector(int);
int ow_zero_vector(Vector*);
int ow_set_vector(Vector*, double);
Vector* create_random_vector(int length);
Vector* create_random_vector_2(int length);

int get_random_vector(Vector** target_vpp, int length);

int get_random_vector_2(Vector** target_vpp, int length);

int get_random_unit_vector(Vector** target_vpp, int length);
/*int get_random_unit_vector_2(Vector** target_vpp); */


double* get_1D_dp_array_from_vector(Vector* vp);

Vector* put_1D_dp_array_into_vector(int length, double* data_ptr);

int ascend_sort_vector(Vector* vp);

int vp_get_indexed_vector(Indexed_vector** vpp, const Vector* init_vp);

int get_random_indexed_vector(Indexed_vector** vpp, int length);

int get_zero_indexed_vector(Indexed_vector** vpp, int length);

int get_target_indexed_vector(Indexed_vector** vpp, int length);

Indexed_vector* create_indexed_vector(int len);
Indexed_vector* create_zero_indexed_vector(int len);
Indexed_vector* vp_create_indexed_vector(const Vector* vp);
void free_indexed_vector(Indexed_vector* vp);
int ascend_sort_indexed_vector(Indexed_vector* vp);
int descend_sort_indexed_vector(Indexed_vector* vp);

int get_target_vector_vector(Vector_vector** target_vvpp, int count);

#ifdef EXPOSE_CREATE_ROUTINES
Vector_vector* create_vector_vector(int count);
#endif 

void free_vector_vector(Vector_vector* vvp);

void free_2D_vp_array_and_vectors
(
    Vector*** array,
    int       num_rows,
    int       num_cols
);

int get_target_v3(V_v_v** target_vvvpp, int count);
#ifdef EXPOSE_CREATE_ROUTINES
V_v_v* create_v3(int length);
#endif 
void free_v3(V_v_v* vvvp);
int get_target_v4(V_v_v_v** target_vvvvpp, int count);
#ifdef EXPOSE_CREATE_ROUTINES
V_v_v_v* create_v4(int length);
#endif 
void free_v4(V_v_v_v* vvvvp);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

