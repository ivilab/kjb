
/* $Id: m_missing.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef M_MISSING_INCLUDED
#define M_MISSING_INCLUDED


#include "l/l_int_vector.h"
#include "m/m_def.h"
#include "m/m_type.h"
#include "m/m_vector.h"
#include "m/m_matrix.h"
#include "m/m_mat_vector.h"


/* -------------------------------------------------------------------------- */

#define DBL_MISSING    DBL_NOT_SET

#define IS_NOT_MISSING_DBL(x) \
    (((x) > DBL_MISSING + 1e-12) || ((x) < DBL_MISSING - 1e-12))

#define IS_MISSING_DBL(x) \
    (((x) < DBL_MISSING + 1e-12) && ((x) > DBL_MISSING - 1e-12))

/* -------------------------------------------------------------------------- */

#ifdef __C2MAN__
int respect_missing_values        (void);
#else

#include "m/m_global.h"

#define respect_missing_values() (kjb_respect_missing_values)
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int enable_respect_missing_values (void);
int disable_respect_missing_values(void);
int restore_respect_missing_values(void);

int index_matrix_vector_rows_with_missing
(
    Int_vector_vector**  missing_index_vvpp,
    const Matrix_vector* mvp
);

int index_matrix_rows_with_missing
(
    Int_vector**  missing_index_vpp,
    const Matrix* mp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


