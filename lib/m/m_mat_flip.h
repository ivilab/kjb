
/* $Id: m_mat_flip.h 15078 2013-08-01 07:44:50Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2013 by members of the Interdisciplinary Visual
|  Intelligence Lab.
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT sista DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
|  Authors:  Kobus Barnard, Andrew Predoehl
* =========================================================================== */

#ifndef M_MAT_FLIP_H_INCLUDED_IVILAB
#define M_MAT_FLIP_H_INCLUDED_IVILAB

#include <m/m_matrix.h>

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

int ow_horizontal_flip_matrix(Matrix*);

int ow_vertical_flip_matrix(Matrix*);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif /* __cplusplus */

#endif /* M_MAT_FLIP_H_INCLUDED_IVILAB */
