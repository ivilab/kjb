
/* $Id: m_fortran.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef M_FORTRAN_INCLUDED
#define M_FORTRAN_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


double* get_1D_dp_array_from_matrix(const Matrix* mp);

Matrix* put_1D_dp_array_into_matrix
(
    int           num_rows,
    int           num_cols,
    const double* data_ptr
);

double* get_fortran_1D_dp_array_from_matrix(const Matrix* mp);

int get_matrix_from_fortran_1D_dp_array
(
    Matrix**      target_mpp,
    int           num_rows,
    int           num_cols,
    const double* data_ptr
);

Matrix* create_matrix_from_fortran_1D_dp_array
(
    int           num_rows,
    int           num_cols,
    const double* data_ptr
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

