
/* $Id: c_sensor.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef C_SENSOR_INCLUDED
#define C_SENSOR_INCLUDED


#include "c/c_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int set_sensor_options(const char* option, const char* value);

int sensor_data_is_up_to_date(int proposed_version);
int get_sensor_version(void);

int normalize_illum_spectra
(
    Spectra**      normalized_illum_sp_ptr,
    const Spectra* illum_sp,
    const Spectra* sensor_sp,
    double         max_rgb
);

int ow_normalize_illum_spectra
(
    Spectra*       illum_sp,
    const Spectra* sensor_sp,
    double         max_rgb
);

int get_RGB_sensors
(
    Spectra** sp_ptr,
    int       num_intervals,
    double    offset,
    double    step
);

int get_spectrum_xy_locus(Vector**, Vector**);
int get_spectrum_uv_locus(Vector**, Vector**);
int get_spectrum_rg_locus(Vector**, Vector**);

int get_xy_from_spectrum(Vector*, double, double, double*, double*);

int get_xy_from_XYZ(double, double, double, double*, double*);

int get_uv_from_spectrum(Vector*, double, double, double*, double*);

int get_uv_from_XYZ(double, double, double, double*, double*);

int get_rg_from_spectrum(Vector*, double, double, double*, double*);

int get_rg_from_RGB(double, double, double, double*, double*);

int get_XYZ_from_spectrum
(
    Vector* spectrum_vp,
    double  offset,
    double  step,
    double* X_ptr,
    double* Y_ptr,
    double* Z_ptr
);

int get_RGB_from_spectrum
(
    Vector* spectrum_vp,
    double  offset,
    double  step,
    double* R_ptr,
    double* G_ptr,
    double* B_ptr
);

int get_sum_from_spectrum(Vector*, double, double*);

int get_ct_from_spectrum(Vector*, double, double, double*);

int get_RGB_vector_from_spectrum
(
    Vector** target_vpp,
    Vector*  input_spectrum_vp,
    double   offset,
    double   step
);

int generate_RGB_data
(
    Matrix**       target_mpp,
    const Spectra* illum_sp,
    int            index,
    const Spectra* reflect_sp,
    const Spectra* sensor_sp
);

int get_RGB_data_from_spectra
(
    Matrix**       target_mpp,
    const Spectra* input_sp,
    const Spectra* sensor_sp
);

int get_RGB_vector_from_spectra
(
    Vector**       target_vpp,
    const Spectra* input_sp,
    int            index
);

int get_RGB_from_spectra
(
    const Spectra* input_sp,
    int            index,
    double*        R_ptr,
    double*        G_ptr,
    double*        B_ptr
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

