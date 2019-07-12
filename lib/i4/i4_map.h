
/* $Id: i4_map.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef I4_MAP_INCLUDED
#define I4_MAP_INCLUDED


#include "i4/i4_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int change_basis_post_map_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Matrix*          map_mp,
    Matrix*          change_basis_mp
);

int change_basis_post_map_projected_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Matrix*          change_basis_mp,
    Matrix*          map_mp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


