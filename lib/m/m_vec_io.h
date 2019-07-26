
/* $Id: m_vec_io.h 15922 2013-10-27 22:38:12Z kobus $ */

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

#ifndef M_VEC_IO_INCLUDED
#define M_VEC_IO_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int read_vector_from_config_file
(
    Vector**    result_vpp,
    const char* env_var,
    const char* directory,
    const char* file_name,
    const char* message_name,
    char*       config_file_name,
    size_t      config_file_name_size
);

int read_vector
(
    Vector**    result_vpp,
    const char* file_name   /* Pointer to file name string. */
);

int fp_read_vector(Vector** result_vpp, FILE* fp);

int fp_read_raw_vector  (Vector** result_vpp, FILE* fp);
int fp_read_vector_with_header(Vector** vpp, FILE* fp);
int fp_read_ascii_vector(Vector** result_vpp, FILE* fp);

int fp_ow_read_vector
(
    Vector* vp, /* Pointer to Vector to receive data.      */
    FILE*   fp  /* Pointer to FILE object containing data. */
);

int fp_write_row_vector_with_title
(
    const Vector* vp,
    FILE*         fp,
    const char*   title
);

int write_indexed_vector(const Indexed_vector* ivp, const char* file_name);
int fp_write_indexed_vector(const Indexed_vector* ivp, FILE* fp);

int fp_write_col_vector_with_title
(
    const Vector* vp,
    FILE*         fp,
    const char*   title
);

int write_col_vector_with_header
(
    const Vector* vp,
    const char*   file_name
);

int fp_write_col_vector_with_header(const Vector* vp, FILE* fp);

int write_row_vector(const Vector* vp, const char* file_name);

int fp_write_row_vector(const Vector* vp, FILE* fp);

int write_col_vector(const Vector* vp, const char* file_name);

int fp_write_col_vector(const Vector* vp, FILE* fp);

int write_row_vector_full_precision
(
    const Vector* vp,
    const char*   file_name
);

int fp_write_row_vector_full_precision(const Vector* vp, FILE* fp);

int write_col_vector_full_precision
(
    const Vector* vp,
    const char*   file_name
);

int fp_write_col_vector_full_precision(const Vector* vp, FILE* fp);

int write_raw_vector   (const Vector* vp, const char* file_name);
int fp_write_raw_vector(const Vector* vp, FILE* fp);

int read_vector_vector_from_config_file
(
    Vector_vector** result_vvpp,
    const char*     env_var,               /* Config file name from envirnonment. */
    const char*     directory,             /* Directory to scan for config file.  */
    const char*     file_name,             /* Name of config file to search for.  */
    const char*     message_name,          /* File name for error messages.       */
    char*           config_file_name,      /* Name of actual config file found.   */
    size_t          config_file_name_size  /* Size of actual config file name.*/
);

int read_vector_vector
(
    Vector_vector** result_vvpp,
    const char*     file_name    /* Pointer to file name string. */
);

int fp_read_vector_vector(Vector_vector** result_vvpp, FILE* fp);

int fp_read_raw_vector_vector(Vector_vector** vvpp, FILE* fp);

int fp_read_formatted_vector_vector
(
    Vector_vector** result_vvpp,
    FILE*           fp           /* Pointer to FILE object containing data. */
);

int fp_write_vector_vector_with_title
(
    const Vector_vector* vvp,   /* Pointer to Vector_vector to output. */
    FILE*                fp,    /* Pointer to FILE object containing data. */
    const char*          title
);

int write_vector_vector
(
    const Vector_vector* vvp,       /* Pointer to Vector_vector to output. */
    const char*          file_name  /* Pointer to file name string. */
);

int fp_write_vector_vector
(
    const Vector_vector* vvp, /* Pointer to Vector_vector to output. */
    FILE*                fp   /* Pointer to FILE object containing data. */
);

int write_raw_vector_vector(const Vector_vector* vvp, const char* file_name);

int fp_write_raw_vector_vector       (const Vector_vector* vvp, FILE* fp);
int fp_write_raw_vector_vector_header(int num_vectors, FILE* fp);

int write_vector_vector_full_precision
(
    const Vector_vector* vvp,       /* Pointer to Vector_vector to output. */
    const char*          file_name  /* Pointer to file name string. */
);

int fp_write_vector_vector_full_precision
(
    const Vector_vector* vvp, /* Pointer to Vector_vector to output. */
    FILE*                fp   /* Pointer to FILE object containing data. */
);

int write_v3
(
    const V_v_v* vvvp,      /* Pointer to V_v_v to output. */
    const char*  file_name  /* Pointer to file name string. */
);

int fp_write_v3(const V_v_v* vvvp, FILE* fp);

int write_v3_full_precision
(
    const V_v_v* vvvp,      /* Pointer to V_v_v to output. */
    const char*  file_name  /* Pointer to file name string. */
);

int fp_write_v3_full_precision(const V_v_v* vvvp, FILE* fp);

int read_v3
(
    V_v_v**     vvvpp,
    const char* file_name  /* Pointer to file name string. */
);

int fp_read_v3(V_v_v** vvvpp, FILE* fp);

int output_vector
(
    const char*   output_dir,
    const char*   file_name,
    const Vector* dist_vp
);

int output_raw_vector
(
    const char*   output_dir,
    const char*   file_name,
    const Vector* dist_vp
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


