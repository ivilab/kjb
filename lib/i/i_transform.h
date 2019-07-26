
/* $Id: i_transform.h 6453 2010-08-11 16:15:58Z ernesto $ */

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

#ifndef I_TRANSFORM_INCLUDED
#define I_TRANSFORM_INCLUDED


#include "l/l_def.h"
#include "m/m_matrix.h"
#include "i/i_type.h"
#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int ow_invert_image(KJB_image* ip);
int ow_invert_gamma_image(KJB_image* ip);
int rotate_image_left(KJB_image** target_ipp, const KJB_image* ip);
int rotate_image_right(KJB_image** target_ipp, const KJB_image* ip);
int ow_horizontal_flip_image(KJB_image* ip);
int ow_vertical_flip_image(KJB_image* ip);

int scale_image_size
(
    KJB_image**      target_ipp,
    const KJB_image* ip,
    double           scale_factor
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


