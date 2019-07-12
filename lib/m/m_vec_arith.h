
/* $Id: m_vec_arith.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef M_VEC_ARITH_INCLUDED
#define M_VEC_ARITH_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int multiply_vector_by_scalar
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        scalar
);

int ow_multiply_vector_by_scalar(Vector* source_vp, double scalar);

int divide_vector_by_scalar
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        scalar
);

int ow_divide_vector_by_scalar(Vector* source_vp, double scalar);

int add_scalar_to_vector
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        scalar
);

int ow_add_scalar_to_vector(Vector* source_vp, double scalar);

int subtract_scalar_from_vector
(
    Vector**      target_vpp,
    const Vector* source_vp,
    double        scalar
);

int ow_subtract_scalar_from_vector(Vector* source_vp, double scalar);

int add_vectors
(
    Vector**      target_vpp,
    const Vector* first_vp,
    const Vector* second_vp
);

int subtract_vectors
(
    Vector**      target_vpp,
    const Vector* first_vp,
    const Vector* second_vp
);

int multiply_vectors
(
    Vector**      target_vpp,
    const Vector* first_vp,
    const Vector* second_vp
);

int divide_vectors
(
    Vector**      target_vpp,
    const Vector* first_vp,
    const Vector* second_vp
);

int ow_add_scalar_times_vector_to_vector
(
    Vector*       first_vp,
    const Vector* second_vp,
    double        scalar
);

int    ow_add_vectors                 
(
    Vector*       first_vp,
    const Vector* second_vp 
);

int    ow_add_vector_times_scalar     
(
    Vector*       first_vp,
    const Vector* second_vp,
    double        scalar 
);

int    ow_subtract_vectors            
(
    Vector*       first_vp,
    const Vector* second_vp 
);

int    ow_subtract_vector_times_scalar
(
    Vector*       first_vp,
    const Vector* second_vp,
    double        scalar 
);

int    ow_multiply_vectors            
(
    Vector*       first_vp,
    const Vector* second_vp 
);

int    ow_divide_vectors              
(
    Vector*       first_vp,
    const Vector* second_vp 
);

int    invert_vector                  
(
    Vector**      target_vpp,
    const Vector* source_vp 
);

int    sqrt_vector                    
(
    Vector**      target_vpp,
    const Vector* vp 
);

int    log_vector                     
(
    Vector**      target_vpp,
    const Vector* vp 
);

int    log_vector_2                   
(
    Vector**      target_vpp,
    const Vector* vp,
    double        log_zero 
);

int    exp_vector                     
(
    Vector**      target_vpp,
    const Vector* vp 
);

int    get_abs_of_vector              
(
    Vector**      target_vpp,
    const Vector* source_vp 
);

int    ow_invert_vector               (Vector* vp);
int    ow_sqrt_vector                 (Vector* vp);
int    ow_log_vector                  (Vector* vp);
int    ow_log_vector_2                (Vector* vp, double log_zero);
int    ow_exp_vector                  (Vector* vp);
int    ow_get_abs_of_vector           (Vector* vp);
double average_vector_elements        (const Vector* vp);
double sum_vector_elements            (const Vector* vp);
double average_vector_squared_elements(const Vector* vp);
double sum_vector_squared_elements    (const Vector* vp);
double multiply_vector_elements       (const Vector* vp);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

