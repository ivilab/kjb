
/* $Id: i_ave.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef I_AVE_INCLUDED
#define I_AVE_INCLUDED


#include "l/l_def.h"
#include "i/i_type.h"
#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_image_average_options(const char* option, const char* value);

int magnify_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count
);

int ave_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count
);

int ave_image_without_invalid
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count,
    int              min_good_pixels
);

int ave_image_where_uniform
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count,
    int              min_good_pixels,
    double           thresh
);

int sample_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              row_count,
    int              col_count
);

int median_filter_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    int              block_size
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


