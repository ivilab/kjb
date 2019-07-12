
/* $Id: l_int_mat_io.h 21593 2017-07-30 16:48:05Z kobus $ */

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

#ifndef L_INT_MAT_IO_INCLUDED
#define L_INT_MAT_IO_INCLUDED


#include "l/l_gen.h"
#include "l/l_int_matrix.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int read_int_matrix_from_config_file
(
    Int_matrix** result_mpp,            /* Double pointer to Int_matrix result.    */
    const char*  env_var,               /* Config file name from envirnonment. */
    const char*  directory,             /* Directory to scan for config file.  */
    const char*  file_name,             /* Name of config file to search for.  */
    const char*  message_name,          /* File name for error messages.       */
    char*        config_file_name,      /* Name of actual config file found.   */
    size_t       config_file_name_size  /* Size of actual config file name.    */
);

int read_int_matrix
(
    Int_matrix** result_mpp,
    const char*  file_name   /* Pointer to file name string. */
);

int fp_read_int_matrix(Int_matrix** result_mpp, FILE* fp);

int fp_read_raw_int_matrix
(
    Int_matrix** result_mpp,
    FILE*        fp          /* Pointer to FILE object containing data. */
);

int fp_read_int_matrix_from_matlab_file
(
    Int_matrix** result_mpp,
    FILE*        fip
);

int fp_read_formatted_int_matrix
(
    Int_matrix** result_mpp,
    FILE*        fp          /* Pointer to FILE object containing data. */
);

int fp_ow_read_int_matrix_by_rows
(
    Int_matrix* mp, /* Pointer to Int_matrix to receive data. */
    FILE*       fp  /* Pointer to FILE object containing data. */
);

int fp_ow_read_int_matrix_by_cols
(
    Int_matrix* mp, /* Pointer to Int_matrix to receive data. */
    FILE*       fp  /* Pointer to FILE containing data. */
);

int read_int_matrix_by_rows
(
    Int_matrix** result_mpp,
    const char*  file_name,  /* Pointer to file name string. */
    int          num_cols    /* Number of columns in Int_matrix. */
);

int fp_read_int_matrix_by_rows
(
    Int_matrix** result_mpp,
    FILE*        fp,         /* Pointer to FILE object containing data. */
    int          num_cols    /* Number of columns in new Int_matrix. */
);

int read_int_matrix_by_cols
(
    Int_matrix** result_mpp,
    const char*  file_name,  /* Pointer to file name string. */
    int          num_rows    /* Number of rows in new Int_matrix. */
);

int fp_read_int_matrix_by_cols
(
    Int_matrix** result_mpp,
    FILE*        fp,         /* Pointer to FILE object containing data. */
    int          num_rows
);

int fp_ow_read_formatted_int_matrix
(
    Int_matrix* mp, /* Int_matrix to receive the data. */
    FILE*       fp  /* Pointer to FILE object containing data. */
);

int fp_read_int_matrix_with_header
(
    Int_matrix** result_mpp,
    FILE*        fp          /* Pointer to FILE object containing data. */
);

int fp_read_matrix_size_header
(
    FILE* fp,           /* Pointer to file to search for header.    */
    int*  num_rows_ptr, /* Number of matrix rows found in header.   */
    int*  num_cols_ptr  /* Number of matrix columns found in header.*/
);

int fp_write_int_matrix_with_title
(
    const Int_matrix* mp,    /* Pointer to Int_matrix to output. */
    FILE*             fp,    /* Pointer to FILE object containing data. */
    const char*       title
);

int write_int_matrix
(
    const Int_matrix* mp,        /* Pointer to Int_matrix to output. */
    const char*       file_name  /* Pointer to file name string. */
);

int fp_write_int_matrix
(
    const Int_matrix* mp, /* Pointer to Int_matrix to output. */
    FILE*             fp  /* Pointer to FILE object containing data. */
);

int write_raw_int_matrix
(
    const Int_matrix* mp,        /* Pointer to Matrix to output. */
    const char*       file_name  /* Pointer to file name string. */
);

int fp_write_raw_int_matrix
(
    const Int_matrix* mp, /* Pointer to Matrix to output. */
    FILE*             fp  /* Pointer to FILE object containing data. */
);

int write_int_matrix_rows
(
    const Int_matrix* mp,        /* Pointer to Int_matrix to output. */
    const char*       file_name  /* Pointer to output file name string. */
);

int fp_write_int_matrix_rows
(
    const Int_matrix* mp, /* Pointer to Int_matrix to output. */
    FILE*             fp  /* Pointer to output FILE object. */
);

int write_int_matrix_cols
(
    const Int_matrix* mp,        /* Pointer to Int_matrix to output. */
    const char*       file_name  /* Pointer to output file name string. */
);

int fp_write_int_matrix_cols
(
    const Int_matrix* mp, /* Pointer to Int_matrix to output. */
    FILE*             fp  /* Pointer to output FILE object. */
);

int write_int_matrix_with_header
(
    const Int_matrix* mp,
    const char*       file_name
);

int fp_write_int_matrix_with_header(const Int_matrix* mp, FILE* fp);

int fp_write_matrix_size_header(FILE* fp, int num_rows, int num_cols);

int write_int_matrix_to_matlab_file
(
    const Int_matrix* mp,        /* Pointer to Int_matrix to output. */
    const char*       file_name  /* Pointer to file name string. */
);

int fp_write_int_matrix_to_matlab_file
(
    const Int_matrix* mp,                   /* Pointer to Int_matrix to output. */
    FILE*             fp,
    const char*       matlab_variable_name
);

int output_int_matrix
(
    const char*       output_dir,
    const char*       file_name,
    const Int_matrix* mp
);

int output_raw_int_matrix
(
    const char*       output_dir,
    const char*       file_name,
    const Int_matrix* mp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

