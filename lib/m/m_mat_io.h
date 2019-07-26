
/* $Id: m_mat_io.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef M_MAT_IO_INCLUDED
#define M_MAT_IO_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int read_matrix_from_config_file
(
    Matrix**    result_mpp,            /* Double pointer to Matrix result.    */
    const char* env_var,               /* Config file name from envirnonment. */
    const char* directory,             /* Directory to scan for config file.  */
    const char* file_name,             /* Name of config file to search for.  */
    const char* message_name,          /* File name for error messages.       */
    char*       config_file_name,      /* Name of actual config file found.   */
    size_t      config_file_name_size  /* Size of actual config file name.    */
);

int read_matrix
(
    Matrix**    result_mpp,
    const char* file_name   /* Pointer to file name string. */
);

int fp_read_matrix(Matrix** result_mpp, FILE* fp);

int fp_read_matrix_with_header
(
    Matrix** result_mpp,
    FILE*    fp          /* Pointer to FILE containing data. */
);

int fp_read_formatted_matrix
(
    Matrix** result_mpp,
    FILE*    fp          /* Pointer to FILE containing data. */
);

int read_matrix_by_rows
(
    Matrix**    result_mpp,
    const char* file_name,  /* Pointer to file name string. */
    int         num_cols    /* Number of columns in Matrix. */
);

int fp_read_matrix_by_rows
(
    Matrix** result_mpp,
    FILE*    fp,         /* Pointer to FILE containing data. */
    int      num_cols    /* Number of columns in new Matrix. */
);

int read_matrix_by_cols
(
    Matrix**    result_mpp,
    const char* file_name,  /* Pointer to file name string. */
    int         num_rows    /* Number of rows in new Matrix. */
);

int fp_read_matrix_by_cols
(
    Matrix** result_mpp,
    FILE*    fp,         /* Pointer to FILE containing data. */
    int      num_rows
);

int fp_ow_read_formatted_matrix
(
    Matrix* mp, /* Matrix to receive the data. */
    FILE*   fp  /* Pointer to FILE containing data. */
);

int fp_ow_read_matrix_by_rows
(
    Matrix* mp, /* Pointer to Matrix to receive data. */
    FILE*   fp  /* Pointer to FILE containing data. */
);

int fp_ow_read_matrix_by_cols
(
    Matrix* mp, /* Pointer to Matrix to receive data. */
    FILE*   fp  /* Pointer to FILE containing data. */
);

int fp_write_matrix_with_title
(
    const Matrix* mp,    /* Pointer to Matrix to output. */
    FILE*         fp,    /* Pointer to FILE containing data. */
    const char*   title
);

int fp_write_matrix_full_precision_with_title
(
    const Matrix* mp,    /* Pointer to Matrix to output. */
    FILE*         fp,    /* Pointer to FILE containing data. */
    const char*   title
);

int write_matrix
(
    const Matrix* mp,        /* Pointer to Matrix to output. */
    const char*   file_name  /* Pointer to file name string. */
);

int write_matrix_2
(
    const Matrix*    mp,
    const char*      file_name,
    const Word_list* row_labels,
    const Word_list* col_labels
);

int fp_write_matrix
(
    const Matrix* mp, /* Pointer to Matrix to output. */
    FILE*         fp  /* Pointer to FILE containing data. */
);

int fp_write_matrix_2
(
    const Matrix*    mp,
    FILE*            fp,
    const Word_list* row_labels,
    const Word_list* col_labels
);

int write_matrix_with_header
(
    const Matrix*    mp,
    const char*      file_name
);

int fp_write_matrix_with_header(const Matrix* mp, FILE* fp);

int write_matrix_full_precision
(
    const Matrix* mp,        /* Pointer to Matrix to output. */
    const char*   file_name  /* Pointer to file name string. */
);

int fp_write_matrix_full_precision
(
    const Matrix* mp, /* Pointer to Matrix to output. */
    FILE*         fp  /* Pointer to FILE containing data. */
);

int write_matrix_rows
(
    const Matrix* mp,        /* Pointer to Matrix to output. */
    const char*   file_name  /* Pointer to output file name string. */
);

int fp_write_matrix_rows
(
    const Matrix* mp, /* Pointer to Matrix to output. */
    FILE*         fp  /* Pointer to output FILE. */
);

int fp_write_matrix_rows_full_precision
(
    const Matrix* mp, /* Pointer to source matrix.      */
    FILE*         fp  /* Pointer to output FILE. */
);

int write_matrix_cols
(
    const Matrix* mp,        /* Pointer to Matrix to output. */
    const char*   file_name  /* Pointer to output file name string. */
);

int fp_write_matrix_cols
(
    const Matrix* mp, /* Pointer to Matrix to output. */
    FILE*         fp  /* Pointer to output FILE. */
);

int write_matrix_vector
(
    const Matrix_vector* mvp,       /* Pointer to Matrix_vector to output. */
    const char*          file_name  /* Pointer to file name string. */
);

int fp_write_matrix_vector(const Matrix_vector* mvp, FILE* fp);

int write_matrix_vector_full_precision
(
    const Matrix_vector* mvp,       /* Pointer to Matrix_vector to output. */
    const char*          file_name  /* Pointer to file name string. */
);

int fp_write_matrix_vector_full_precision
(
    const Matrix_vector* mvp,
    FILE*                fp
);

int read_indexed_matrix_vector
(
    Matrix_vector** mvpp,
    const char*     index_file_name,
    const char*     file_name
);

int read_matrix_vector
(
    Matrix_vector** mvpp,
    const char*     file_name  /* Pointer to file name string. */
);

int fp_read_matrix_vector(Matrix_vector** mvpp, FILE* fp);

int fp_read_matrix_vector_with_headers(Matrix_vector** mvpp, FILE* fp);

int fp_read_raw_matrix(Matrix** result_mpp, FILE* fp);

int write_raw_matrix(const Matrix* mp, const char* file_name);

int fp_write_raw_matrix(const Matrix* mp, FILE* fp);

int fp_read_raw_matrix_vector(Matrix_vector** mvpp, FILE* fp);

int write_raw_matrix_vector
(
    const Matrix_vector* mvp,
    const char*          file_name
);

int fp_write_raw_matrix_vector(const Matrix_vector* mvp, FILE* fp);

int fp_write_raw_matrix_vector_header(int num_matrices, FILE* fp);

int output_matrix
(
    const char*   output_dir,
    const char*   file_name,
    const Matrix* mp
);

int output_raw_matrix
(
    const char*   output_dir,
    const char*   file_name,
    const Matrix* mp
);

int output_matrix_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Matrix_vector* mvp
);

int output_raw_matrix_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Matrix_vector* mvp
);

int output_vector_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Vector_vector* vvp
);

int output_raw_vector_vector
(
    const char*          output_dir,
    const char*          file_name,
    const Vector_vector* vvp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

