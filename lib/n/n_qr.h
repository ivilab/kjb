
/* $Id: n_qr.h 10635 2011-09-29 19:51:13Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2010 by Kobus Barnard (author).
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
|  Author: Kyle Simek
* =========================================================================== */

#ifndef N_QR_INCLUDED
#define N_QR_INCLUDED

#include "m/m_matrix.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int qr_decompose(const Matrix* mp, Matrix** Q_mpp, Matrix** R_mpp);
int rq_decompose(const Matrix* mp, Matrix** R_mpp, Matrix** Q_mpp);

int ql_decompose(const Matrix* mp, Matrix** Q_mpp, Matrix** L_mpp);
int lq_decompose(const Matrix* mp, Matrix** L_mpp, Matrix** Q_mpp);
#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
