
/* $Id: m_error.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef M_ERROR_INCLUDED
#define M_ERROR_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define USE_COMPARE_WITH_REAL_MAX_FOR_DIVIDE_BY_ZERO
/* #define USE_IS_ZERO_DBL_FOR_DIVIDE_BY_ZERO */


#ifdef TEST

#    define SET_DIVIDE_BY_ZERO_ERROR()  \
                          set_divide_by_zero_error(__LINE__, __FILE__)

#    define SET_OVERFLOW_ERROR()  \
                          set_overflow_error(__LINE__, __FILE__)

#else
#    define SET_DIVIDE_BY_ZERO_ERROR() set_divide_by_zero_error()
#    define SET_OVERFLOW_ERROR()       set_overflow_error()
#endif


#ifdef TEST
#    define check_same_matrix_vector_dimensions(x,y,z) \
                debug_check_same_matrix_vector_dimensions(x,y,__LINE__, __FILE__,z)
#endif

#ifdef TEST
int debug_check_same_matrix_vector_dimensions
#else
int check_same_matrix_vector_dimensions
#endif
(
    const Matrix_vector* first_mvp,
    const Matrix_vector* second_mvp,
#ifdef TEST
    int           line_number,
    const char*   file_name,
#endif
    const char*   constext_str
);

#ifdef TEST
#    define check_same_matrix_dimensions(x,y,z)  \
                debug_check_same_matrix_dimensions(x,y,__LINE__, __FILE__,z)
#endif

#ifdef TEST
int debug_check_same_matrix_dimensions
#else
int check_same_matrix_dimensions
#endif
(
    const Matrix* first_mp,
    const Matrix* second_mp,
#ifdef TEST
    int           line_number,
    const char*   file_name,
#endif
    const char*   context_str
);

#ifdef TEST
#    define check_same_vector_lengths(x,y,z)  \
                debug_check_same_vector_lengths(x,y,__LINE__, __FILE__,z)
#endif

#ifdef TEST
int debug_check_same_vector_lengths
#else
int check_same_vector_lengths
#endif
(
    const Vector* first_vp,
    const Vector* second_vp,
#ifdef TEST
    int           line_number,
    const char*   file_name,
#endif
    const char*   constext_str
);

Matrix* expand_error_box(const Matrix*);
Matrix* create_error_points(const Matrix*, double);

int get_expanded_error_box(Matrix** result_mpp, const Matrix* mp);

int get_points_with_error
(
    Matrix**      result_mpp,
    const Matrix* mp,
    double        abs_err,
    double        rel_err,
    double        min_valid_value
);

#ifdef TEST
    void set_divide_by_zero_error(int, const char*);
    void set_overflow_error(int, const char*);
#else
    void set_divide_by_zero_error(void);
    void set_overflow_error(void);
#endif

int check_quadratic_constraints
(
    const Vector* result_vp,
    const Matrix* le_constraint_mp,
    const Vector* b_vp,
    const Vector* lb_constraint_vp,
    const Vector* ub_constraint_vp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

