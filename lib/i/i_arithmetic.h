

/* $Id: i_arithmetic.h 20918 2016-10-31 22:08:27Z kobus $ */

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

#ifndef I_ARITHMETIC_INCLUDED
#define I_ARITHMETIC_INCLUDED


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


int power_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    double           power
);

int ow_power_image(KJB_image* in_ip, double power);

int scale_image(KJB_image**, const KJB_image*, double);

int ow_scale_image(KJB_image*, double);

int scale_image_by_channel(KJB_image**, const KJB_image*, Vector*);

int ow_scale_image_by_channel(KJB_image*, Vector*);

int subtract_images
(
    KJB_image**      target_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip
);

int ow_subtract_images(KJB_image* in1_ip, const KJB_image* in2_ip);

int multiply_images
(
    KJB_image**      out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip
);

int ow_multiply_images(KJB_image* in1_ip, const KJB_image* in2_ip);

int divide_images
(
    KJB_image**      out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip
);

int ow_divide_images(KJB_image* in1_ip, const KJB_image* in2_ip);

int ow_min_of_images(KJB_image* in1_ip, const KJB_image* in2_ip);

int add_images
(
    KJB_image**      out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip
);

int ow_add_images(KJB_image* in1_ip, const KJB_image* in2_ip);

int ow_subtract_vector_from_image(KJB_image* ip, const Vector* vp);
int ow_add_vector_to_image(KJB_image* ip, const Vector* vp);
int ow_min_thresh_image(KJB_image* ip, double min);
int ow_max_thresh_image(KJB_image* ip, double max);

int log_one_plus_image(KJB_image** out_ipp, const KJB_image* in_ip);

int ow_log_one_plus_image(KJB_image* in_ip);

int ow_log_brightness_image
(
    KJB_image* in_ip,
    double     (*brightness_fn) (double, double, double),
    double     power
);

int ow_exponantiate_image(KJB_image* in_ip);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


