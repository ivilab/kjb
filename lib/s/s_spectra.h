
/* $Id: s_spectra.h 6352 2010-07-11 20:13:21Z kobus $ */

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

#ifndef S_SPECTRA_INCLUDED
#define S_SPECTRA_INCLUDED


#include "l/l_incl.h"
#include "m/m_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define USE_ALL_SPECTRA    (-1000)        /* Must be negative! */


typedef enum Spectra_origin
{
    SPECTRA_TYPE_ERROR = ERROR,
    ILLUMINANT_SPECTRA,
    REFLECTANCE_SPECTRA,
    SENSOR_SPECTRA,
    GENERIC_SPECTRA,
    SPECTRA_TYPE_NOT_SET
}
Spectra_origin;


/* =============================================================================
 *                                   Spectra
 *
 * Spectra type
 *
 * This type is the spectra type for the KJB library and is used by many
 * routines. It stores spectra in the rows of the matrix pointed to by the field
 * spectra_mp.
 *
 * The "type" field can be used to specify the origin of the spectra. If you are
 * not interested in this information, then use GENERIC_SPECTRA or
 * SPECTRA_TYPE_NOT_SET. The possible values of "type" are as follows:
 *
 * |     typedef enum Spectra_origin
 * |     {
 * |         SPECTRA_TYPE_ERROR = ERROR,
 * |         ILLUMINANT_SPECTRA,
 * |         REFLECTANCE_SPECTRA,
 * |         SENSOR_SPECTRA,
 * |         GENERIC_SPECTRA,
 * |         SPECTRA_TYPE_NOT_SET
 * |     }
 * |     Spectra_origin;
 *
 * Related:
 *    create_spectra, free_spectra, get_target_spectra
 *
 * Index: spectra
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Spectra
{
    Matrix*        spectra_mp;
    double         step;       /* Very often a parameter, so just use double. */
    double         offset;     /* Very often a parameter, so just use double. */
    Spectra_origin type;
}
Spectra;

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION


#    define create_spectra(v, w, x, y, z) \
               debug_create_spectra(v, w, x, y, z, __FILE__, __LINE__)

#    define get_target_spectra(u, v, w, x, y, z) \
               debug_get_target_spectra(u, v, w, x, y, z, __FILE__, __LINE__)


    Spectra* debug_create_spectra
    (
        int            num_spectra,
        int            num_freq_intervals,
        double         offset,
        double         step,
        Spectra_origin type,
        const char*    file_name,
        int            line_number
    );

    int debug_get_target_spectra
    (
        Spectra**      out_sp_ptr,
        int            num_spectra,
        int            num_freq_intervals,
        double         offset,
        double         step,
        Spectra_origin type,
        const char*    file_name,
        int            line_number
    );

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

    Spectra* create_spectra
    (
        int            num_spectra,
        int            num_freq_intervals,
        double         offset,
        double         step,
        Spectra_origin type
    );

    int get_target_spectra
    (
        Spectra**      out_sp_ptr,
        int            num_spectra,
        int            num_freq_intervals,
        double         offset,
        double         step,
        Spectra_origin type
    );

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

void free_spectra(Spectra* spectra_ptr);

int copy_spectra(Spectra** target_sp_ptr, const Spectra* sp);

int convert_spectra
(
    Spectra**      target_sp_ptr,
    const Spectra* original_sp,
    int            count,
    double         offset,
    double         step
);

int multiply_spectra
(
    Spectra**      output_sp_ptr,
    const Spectra* sp1,
    int            index,
    const Spectra* sp2
);

int check_spectra_are_comparable(const Spectra* sp1, const Spectra* sp2);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif



