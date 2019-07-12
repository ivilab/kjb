
/* $Id: m_lut.h 6352 2010-07-11 20:13:21Z kobus $ */

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

#ifndef M_LUT_INCLUDED
#define M_LUT_INCLUDED


#include "m/m_vector.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* =============================================================================
 *                                   Lut
 *
 * Lut type
 *
 * This type is the lookup table type for the KJB library.
 *
 * Related:
 *    create_lut, free_lut, get_target_lut
 *
 * Index: lut
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Lut
{
    Vector*        lut_vp;
    double         step;       /* Very often a parameter, so just use double. */
    double         offset;     /* Very often a parameter, so just use double. */
}
Lut;

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION


#    define create_lut(x, y, z) \
               debug_create_lut(x, y, z, __FILE__, __LINE__)

#    define get_target_lut(w, x, y, z) \
               debug_get_target_lut(w, x, y, z, __FILE__, __LINE__)


Lut* debug_create_lut
(
    int         lut_len,
    double      offset,
    double      step,
    const char* file_name,
    int         line_number
);

int debug_get_target_lut
(
    Lut**       out_sp_ptr,
    int         lut_len,
    double      offset,
    double      step,
    const char* file_name,
    int         line_number
);

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

    Lut* create_lut(int lut_len, double offset, double step);

    int get_target_lut
    (
        Lut**  out_sp_ptr,
        int    lut_len,
        double offset,
        double step
    );


#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */


void free_lut(Lut* lut_ptr);
int copy_lut(Lut** target_sp_ptr, Lut* sp);

int convert_lut
(
    Lut**  target_sp_ptr,
    Lut*   original_sp,
    int    count,
    double offset,
    double step
);

int read_lut_from_config_file
(
    Lut**       result_sp_ptr,
    const char* env_var,
    const char* directory,
    const char* file_name,
    const char* message_name,
    char*       config_file_name,
    size_t      config_file_name_size
);

int read_lut(Lut** lut_ptr_ptr, char* file_name);
int fp_read_lut(Lut** lut_ptr_ptr, FILE* fp);
int write_lut(const Lut* lp, const char* file_name);
int fp_write_lut(const Lut* lp, FILE* fp);

int apply_lut(const Lut* lut_ptr, double x, double* y_ptr);

int apply_lut_2(const Lut* lut_ptr, double x, double* y_ptr);

int apply_lut_inverse(const Lut* lut_ptr, double y, double* x_ptr);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif



