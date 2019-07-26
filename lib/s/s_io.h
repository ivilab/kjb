
/* $Id: s_io.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef S_IO_INCLUDED
#define S_IO_INCLUDED


#include "s/s_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int read_spectra_from_config_file
(
    Spectra**   result_sp_ptr,
    const char* env_var,
    const char* directory,
    const char* file_name,
    const char* message_name,
    char*       config_file_name,
    size_t      config_file_name_size
);

int read_reflectance_spectra
(
    Spectra**   result_sp_ptr,
    const char* file_name
);

int read_illuminant_spectra
(
    Spectra**   result_sp_ptr,
    const char* file_name
);

int read_sensor_spectra(Spectra** result_sp_ptr, const char* file_name);

int read_spectra(Spectra** result_sp_ptr, const char* file_name);

int fp_read_spectra(Spectra** result_sp_ptr, FILE* fp);

int read_spectra_file_header
(
    char*           file_name,
    int*            num_freq_intervals_ptr,
    double*         offset_ptr,
    double*         step_ptr,
    Spectra_origin* type_ptr
);

int fp_read_spectra_file_header
(
    FILE*           fp,
    int*            num_freq_intervals_ptr,
    double*         offset_ptr,
    double*         step_ptr,
    Spectra_origin* type_ptr
);

int write_spectra_full_precision
(
    const Spectra* sp,
    const char*    file_name
);

int fp_write_spectra_full_precision(const Spectra* sp, FILE* fp);

int write_spectra(const Spectra* sp, const char* file_name);

int fp_write_spectra(const Spectra* sp, FILE* fp);

int fp_write_spectra_file_header
(
    FILE*          output_fp,
    int            num_freq_intervals,
    double         offset,
    double         step,
    Spectra_origin type
);


Spectra_origin get_spectra_type_from_str(char* type_str);

int put_spectra_type_into_str
(
    Spectra_origin spectra_type,
    char*          buff,
    size_t         buff_size
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

