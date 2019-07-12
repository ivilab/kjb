
/* $Id: i_gamma.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef I_GAMMA_INCLUDED
#define I_GAMMA_INCLUDED


#include "m/m_gen.h"
#include "i/i_type.h"
#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_gamma_options(const char* option, const char* value);

int ow_gamma_correct_image(KJB_image* in_ip, const Vector* gamma_vp);

int gamma_correct_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    const Vector*    gamma_vp
);

int invert_image_gamma
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Vector*          gamma_vp
);

int ow_invert_image_gamma(KJB_image* in_ip, Vector* gamma_vp);

int ow_linearize_pcd(KJB_image* ip);

int ow_apply_pcd_output_lut(KJB_image* ip);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

