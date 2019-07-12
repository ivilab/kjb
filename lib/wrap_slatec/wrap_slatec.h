
/* $Id: wrap_slatec.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.      |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */

#ifndef WRAP_SLATEC_INCLUDED
#define WRAP_SLATEC_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int slatec_i1mach(int* machine_constant_ptr, int machine_constant_id);
int slatec_d1mach(double* machine_constant_ptr, int machine_constant_id);

int do_dlsei_quadratic
(
    Vector**      result_vpp,
    const Matrix* mp,
    const Vector* target_vp,
    const Matrix* le_constraint_mp,
    const Vector* le_constraint_col_vp,
    const Matrix* eq_constraint_mp,
    const Vector* eq_constraint_col_vp,
    const Vector* lb_row_vp,
    const Vector* ub_row_vp
);

int do_dbocls_quadratic
(
    Vector**      result_vpp,
    const Matrix* mp,
    const Vector* target_vp,
    const Matrix* eq_constraint_mp,
    const Vector* eq_constraint_col_vp,
    const Matrix* le_constraint_mp,
    const Vector* le_constraint_col_vp,
    const Vector* lb_row_vp,
    const Vector* ub_row_vp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

