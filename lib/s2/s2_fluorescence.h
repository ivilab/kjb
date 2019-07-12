
/* $Id: s2_fluorescence.h 6352 2010-07-11 20:13:21Z kobus $ */

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

#ifndef S2_FLOURESCENCE_INCLUDED
#define S2_FLOURESCENCE_INCLUDED


#include "s/s_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* =============================================================================
 *                               Fluorescent_surface
 *
 * Fluorescent surface type
 *
 * This type is the fluorescent reflectance type for the KJB library. It is
 * normally used by library routines, with the external interface being
 * provided by the fluorescent database type. It is documented here mostly to
 * help explain the fluorescent database type, but it may ocasionally be useful
 * by itself.
 *
 * A fluorescent reflectance is described in theory by a triangular matrix, but
 * in practice determining that matrix is hard with the equipment available at
 * SFU. Hence a fluorescent surface is represnted as a collection of input and
 * output spectra--in other words, the data used to charactersize the surface.
 * If we chose to work with the full fluorescent reflectance matrix, then a
 * second fluorescent surface data type would be in order.
 *
 * Index: fluorescence
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Fluorescent_surface
{
    Spectra* illum_sp;
    Spectra* response_sp;
}
Fluorescent_surface;


/* =============================================================================
 *                              Fluorescent_database
 *
 * Flourescent database type
 *
 * This type is used to store a collection of fluorscent surfaces. It uses the
 * type Fluorescent_surface, but normally the interface with the KJB library is
 * through this type.
 *
 * Related:
 *    create_fluorescent_database, get_target_fluorescent_database
 *
 * Index: fluorescence
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Fluorescent_database
{
    int                  num_fluorescent_surfaces;
    Fluorescent_surface* fluorescent_surfaces;
}
Fluorescent_database;

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION


#    define create_fluorescent_database(v) \
               debug_create_fluorescent_database(v, __FILE__, __LINE__)

#    define get_target_fluorescent_database(u, v) \
               debug_get_target_fluorescent_database(u, v, __FILE__, __LINE__)


Fluorescent_database* debug_create_fluorescent_database
(
    int         num_fluorescent_surfaces,
    const char* file_name,
    int         line_number
);

int debug_get_target_fluorescent_database
(
    Fluorescent_database** out_fl_db_ptr_ptr,
    int                    num_fluorescent_surfaces,
    const char*            file_name,
    int                    line_number
);

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

Fluorescent_database* create_fluorescent_database
(
    int num_fluorescent_surfaces
);

int get_target_fluorescent_database
(
    Fluorescent_database** out_fl_db_ptr_ptr,
    int                    num_fluorescent_surfaces
);

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */


void free_fluorescent_database(Fluorescent_database* fl_db_ptr);

int copy_fluorescent_database
(
    Fluorescent_database** target_fl_db_ptr_ptr,
    Fluorescent_database*  fl_db_ptr
);

int read_fl_db_from_config_file
(
    Fluorescent_database** fl_db_ptr_ptr,
    const char*            env_var,
    const char*            directory,
    const char*            file_name,
    const char*            message_name,
    char*                  config_file_name,
    size_t                 config_file_name_size
);

int read_fluorescent_database
(
    Fluorescent_database** fl_db_ptr_ptr,
    char*                  file_name
);

int fp_read_fluorescent_database
(
    Fluorescent_database** fl_db_ptr_ptr,
    FILE*                  fp
);

int fp_read_fluorescent_database_header
(
    FILE* fp,
    int*  num_fluorescent_surfaces_ptr
);

int fp_read_fluorescent_database_surface
(
    FILE*                fp,
    Fluorescent_surface* fl_ptr
);

int write_fluorescent_database
(
    char*                 file_name,
    Fluorescent_database* fl_db_ptr
);

int fp_write_fluorescent_database
(
    FILE*                 fp,
    Fluorescent_database* fl_db_ptr
);

int fp_write_fluorescent_database_header
(
    FILE* fp,
    int   num_fluorescent_surfaces
);

int fp_write_fluorescent_database_surface
(
    FILE*                fp,
    Fluorescent_surface* fl_ptr
);

int get_fluorescent_response_spectra
(
    Spectra**             response_sp_ptr,
    Fluorescent_database* fl_db_ptr,
    Spectra*              illum_sp,
    int                   index
);

int get_fluorescent_surface_response
(
    Vector**             response_vpp,
    Fluorescent_surface* fl_ptr,
    Vector*              illum_vp,
    Vector**             estimated_illum_vpp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif



