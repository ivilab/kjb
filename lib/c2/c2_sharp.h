
/* $Id: c2_sharp.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef C2_SHARP_INCLUDED
#define C2_SHARP_INCLUDED


#include "c2/c2_gen.h"
#include "s/s_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_sharpening_transform_options
(
    const char* option,
    const char* value
);

int sharpen_spectra
(
    Spectra**      sharpend_sp_ptr,
    const Spectra* sp,
    const Matrix*  sharpen_post_map_mp
);

int fix_sharpen_map(Matrix** new_post_map_mpp, const Matrix* post_map_mp);

int get_db_sharp_transform
(
    Matrix**      T_mpp,
    const Matrix* data_mp,
    const Matrix* canon_data_mp
);

int get_mip_db_sharp_transform
(
    Matrix**       T_mpp,
    const Spectra* reflect_spectra_sp,
    const Spectra* illum_spectra_sp,
    const Spectra* canon_spectra_sp,
    const Matrix*  initial_T_mp
);

int sharp_db_mip_env_is_up_to_date(int version);

int get_two_mode_perfect_sharp_transform
(
    Matrix**       sharpen_post_map_mpp,
    const Spectra* sensor_sp,
    const Spectra* illum_sp,
    const Spectra* reflect_sp
);

int get_canon_perfect_sharp_transform
(
    Matrix**       sharpen_post_map_mpp,
    const Spectra* sensor_sp,
    const Spectra* canon_sp,
    const Spectra* illum_sp,
    const Spectra* reflect_sp
);

int get_standard_perfect_sharp_transform
(
    Matrix**       sharpen_post_map_mpp,
    const Spectra* sensor_sp,
    const Spectra* illum_sp,
    const Spectra* reflect_sp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

