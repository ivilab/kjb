
/* $Id: l_io.h 6352 2010-07-11 20:13:21Z kobus $ */

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

#ifndef L_IO_INCLUDED
#define L_IO_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

typedef enum Continue_query_response
{
    QUERY_RESPONSE_NOT_SET    = NOT_SET,
    NEGATIVE_RESPONSE         = FALSE,
    POSITIVE_RESPONSE         = TRUE,
    STRONG_NEGATIVE_RESPONSE,
    STRONG_POSITIVE_RESPONSE,
    DISABLE_QUERY,
    QUERY_ERROR               = ERROR
}
Continue_query_response;

/* -------------------------------------------------------------------------- */

#define BUFF_MULT_FGET_LINE(x, y)   mult_fget_line((x), (y), sizeof(y))
#define BUFF_GET_REAL_LINE(x, y)    get_real_line((x), (y), sizeof(y))
#define BUFF_STRING_REAL_LINE(x, y) string_get_real_line((x), (y), sizeof(y))

/* =============================================================================
 *                             BUFF_GET_COMMAND_TEXT
 *
 * (MACRO) Sets up call to get_command_text
 *
 * The max_len parameter is set to sizeof(command).  Using sizeof to set the
 * buffer size is recomended where applicable, as the code will not be broken if
 * the buffer size changes. HOWEVER, neither this method, nor the macro, is
 * applicable if command is NOT a character array.  If command is declared by
 * "char *command", then the size of command is the number of bytes in a
 * character pointer (usually 4), which is NOT what is normally intended.  You
 * have been WARNED!
 *
 * Related:
 *    get_command_text
 *
 * Index: I/O, input, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    long BUFF_GET_COMMAND_TEXT(const char* progam_name, char command[ ]);
#else
#   define BUFF_GET_COMMAND_TEXT(w, x)    get_command_text((w), (x), sizeof(x))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define BUFF_SGET_REAL_LINE(x, y)   string_get_real_line((x), (y), sizeof(y))

/* -------------------------------------------------------------------------- */


int  set_io_options    (const char* option, const char* value);
void set_program_prompt(const char* prompt);
void prompt_to_continue(void);

long kjb_query
(
    const char* prompt,
    char*       input,
    size_t      input_size
);

long overwrite_query
(
    const char* prompt,
    char*       input,
    size_t      input_size
);
int yes_no_query(const char* prompt);
int confirm_risky_action(const char* prompt);

long multi_fget_line
(
    FILE*** fp_ptr_ptr,
    char*   line,
    size_t  max_len
);

int multi_fclose(FILE** fp_ptr);

int print_blanks(FILE* fp, int num_blanks);

int rep_print(FILE* fp, int c, int n);

long count_real_lines(FILE* fp);
long count_data_lines_until_next_header(FILE* fp);

int read_header_options
(
    FILE*   fp,
    char*** option_list_ptr,
    char*** value_list_ptr
);

long count_tokens_in_file(FILE* fp);

long gen_count_tokens_in_file(FILE* fp, const char* terminators);

long get_real_line(FILE* fp, char* line, size_t max_len);

long string_count_real_lines(const char* buff);

long string_get_real_line
(
    const char** buff_ptr,
    char*        line,
    size_t       max_len
);

long get_command_text
(
    const char* program_name,
    char*       command,
    size_t      max_len
);

int add_line_to_input_stream(const char* line);

int set_input_line_buffer_size(int max_len);

Continue_query_response continue_query(void);

int read_int_from_file(int* int_ptr, const char* file_name);
int read_dbl_from_file(double* dbl_ptr, const char* file_name); 

int read_and_create_int_array
(
    char* file_name,
    int*  num_elements_ptr,
    int** array_ptr
);

int kjb_fread_4_bytes          (FILE* fp, void* buff);
int kjb_fread_4_bytes_backwards(FILE* fp, void* buff);
int read_double                (double* value_ptr, const char* file_name);
int fp_read_double             (double* value_ptr, FILE* fp);

int output_int
(
    const char* output_dir,
    const char* file_name,
    int         value
);

int output_double
(
    const char* output_dir,
    const char* file_name,
    double      value
);


int  push_no_overwrite        (int no_overwrite);
void pop_no_overwrite         (void);
void set_no_overwrite         (int no_overwrite);
int  get_no_overwrite         (void);
int  skip_because_no_overwrite(const char* file_name);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

