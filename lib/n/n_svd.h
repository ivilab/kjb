
/* $Id: n_svd.h 5576 2010-02-21 15:30:58Z kobus $ */

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

#ifndef N_SVD_INCLUDED
#define N_SVD_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_svd_options(const char* option, const char* value);

int do_svd
(
    const Matrix* a_mp,
    Matrix**      u_mp_mp,
    Vector**      d_vpp,
    Matrix**      v_trans_mpp,
    int*          rank_ptr
);

int get_determinant_abs(const Matrix* mp, double* det_ptr);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

