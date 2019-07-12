
/* $Id: n_invert.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef N_INVERT_INCLUDED 
#define N_INVERT_INCLUDED 

#ifdef __cplusplus
#endif 


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_matrix_inversion_options(const char* option, const char* value);

int get_matrix_inverse(Matrix** target_mpp, const Matrix* input_mp);

int do_gaussian_elimination_matrix_inversion
(
    Matrix**      target_mpp,
    const Matrix* input_mp 
);

int do_svd_matrix_inversion(Matrix** target_mpp, const Matrix* input_mp);

int do_svd_matrix_inversion_2
(
    Matrix**      target_mpp,
    const Matrix* u_mp,
    const Vector* d_vp,
    const Matrix* v_trans_mp 
);

int get_MP_inverse(Matrix** result_mpp, const Matrix* mp);

int get_MP_inverse_of_transpose(Matrix** result_mpp, const Matrix* mp);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif 

#endif

