
/* $Id: l_sys_io.h 21593 2017-07-30 16:48:05Z kobus $ */

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

#ifndef L_SYS_IO_INCLUDED
#define L_SYS_IO_INCLUDED


#include "l/l_def.h"
#include "l/l_word_list.h"

#ifdef UNIX
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <sys/param.h>
#endif
#endif

#ifndef MAXPATHLEN
#    ifdef __BORLANDC__
#        include <dir.h>
#        define MAXPATHLEN MAXPATH
#    else 
#        define MAXPATHLEN 1024    /* FIXME --- what is the standard way? */
#    endif
#endif

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define NOT_ALIGNED_LARGE_IO_BUFF_SIZE          (MIN_OF(1000000, STACK_SIZE / 16))
#define LARGE_IO_BUFF_SIZE      (16 *(1+ (NOT_ALIGNED_LARGE_IO_BUFF_SIZE/16)))

#define GET_LINE_BUFF_SIZE          LARGE_IO_BUFF_SIZE

#define NOT_ALIGNED_MAX_FORMAT_STRING_SIZE      (MAX_OF(1000,                             \
                                                         MIN_OF(10000, STACK_SIZE / 20)))
#define MAX_FORMAT_STRING_SIZE   (16 *(1+ (NOT_ALIGNED_MAX_FORMAT_STRING_SIZE/16)))

#define NOT_ALIGNED_MAX_FILE_NAME_SIZE     (MAX_OF(MAXPATHLEN, \
                                              (MIN_OF(2000, STACK_SIZE / 20))))
#define MAX_FILE_NAME_SIZE     (16 *(1+ (NOT_ALIGNED_MAX_FILE_NAME_SIZE/16)))

#define MAX_PATH_SIZE   MAX_FILE_NAME_SIZE 


#ifndef SEEK_SET
#    define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#    define SEEK_CUR 1
#endif

#ifndef SEEK_END
#    define SEEK_END 2
#endif

#define kjb_printf   pso
#define kjb_flush()  kjb_fflush(stdout)


#define put_line(x)                fput_line(stdout, (x))


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             kjb_puts
 *
 * (MACRO) Writes a null terminated string to stdout.
 *
 * This macro is defined as kjb_fputs(stdout, (x)). This is the safe way to
 * write a string which is not known. If you use formatted IO on such a string,
 * then you risk problems if the unknown string has a formatting character.
 *
 * Related:
 *    kjb_fputs, fput_line, pso, kjb_fprintf
 *
 * Index: I/O, input, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    long kjb_puts(const char* output_string);
#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line.
                      I have no idea what the problem is. */
#   define kjb_puts(x)                kjb_fputs(stdout, (x))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define kjb_putc(x)                kjb_fputc(stdout, (x))
#define kjb_getc()                 kjb_fgetc(stdout)

#define FPUT_FIELD(x, y)           kjb_fwrite((x), (y), sizeof(y))
#define FIELD_READ(x, y)           kjb_fread_exact((x), (void*)&(y), sizeof(y))
#define FIELD_WRITE(x, y)          kjb_fwrite((x), (const void*)&(y), sizeof(y))
#define ARRAY_READ(x, y)           kjb_fread_exact((x), (void*)(y), sizeof(y))
#define ARRAY_WRITE(x, y)          kjb_fwrite((x), (const void*)(y), sizeof(y))

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_STDIN_GET_LINE
 *
 * (MACRO) Sets up call to stdin_get_line
 *
 * The max_len parameter is set to sizeof(line).  Using sizeof to set the
 * buffer size is recomended where applicable, as the code will not be broken
 * if the buffer size changes. HOWEVER, neither this method, nor the macro, is
 * applicable if line is NOT a character array.  If line is declared by "char
 * *line", then the size of line is the number of bytes in a character pointer
 * (usually 8), which is NOT what is normally intended.  You have been WARNED!
 *
 * Related:
 *    fget_line, BUFF_DGET_LINE, dget_line, BUFF_SGET_LINE, sget_line
 *
 * Index: I/O, input, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    long BUFF_STDIN_GET_LINE(const char*, char line[ ]);
#else
#   define BUFF_STDIN_GET_LINE(x,y)   stdin_get_line((x), (y), (int)sizeof(y))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_GET_LINE
 *
 * (MACRO) Sets up call to fget_line with stdin
 *
 * The max_len parameter is set to sizeof(line).  Using sizeof to set the
 * buffer size is recomended where applicable, as the code will not be broken
 * if the buffer size changes. HOWEVER, neither this method, nor the macro, is
 * applicable if line is NOT a character array.  If line is declared by "char
 * *line", then the size of line is the number of bytes in a character pointer
 * (usually 8), which is NOT what is normally intended.  You have been WARNED!
 *
 * Related:
 *    fget_line, BUFF_DGET_LINE, dget_line, BUFF_SGET_LINE, sget_line
 *
 * Index: I/O, input, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    long BUFF_GET_LINE(char line[ ]);
#else
#   define BUFF_GET_LINE(y)    fget_line(stdin, (y), (int)sizeof(y))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_FGET_LINE
 *
 * (MACRO) Sets up call to fget_line
 *
 * The max_len parameter is set to sizeof(line).  Using sizeof to set the
 * buffer size is recomended where applicable, as the code will not be broken
 * if the buffer size changes. HOWEVER, neither this method, nor the macro, is
 * applicable if line is NOT a character array.  If line is declared by "char
 * *line", then the size of line is the number of bytes in a character pointer
 * (usually 8), which is NOT what is normally intended.  You have been WARNED!
 *
 * Related:
 *    fget_line, BUFF_DGET_LINE, dget_line, BUFF_SGET_LINE, sget_line
 *
 * Index: I/O, input, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    long BUFF_FGET_LINE(FILE* fp, char line[ ]);
#else
#   define BUFF_FGET_LINE(x, y)     fget_line((x), (y), (int)sizeof(y))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_DGET_LINE
 *
 * (MACRO) Sets up call to dget_line
 *
 * The max_len parameter is set to sizeof(line).  Using sizeof to set the
 * buffer size is recomended where applicable, as the code will not be broken
 * if the buffer size changes. HOWEVER, neither this method, nor the macro, is
 * applicable if line is NOT a character array.  If line is declared by "char
 * *line", then the size of line is the number of bytes in a character pointer
 * (usually 8), which is NOT what is normally intended.  You have been WARNED!
 *
 * Related:
 *    dget_line, BUFF_FGET_LINE, fget_line, BUFF_SGET_LINE, sget_line
 *
 * Index: I/O, input, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    long BUFF_DGET_LINE(int file_descriptor, char line[ ]);
#else
#    define BUFF_DGET_LINE(x, y)      dget_line((x), (y), (int)sizeof(y))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define BUFF_REAL_PATH(x, y)           kjb_realpath((x), (y), sizeof(y))
#define BUFF_GET_FD_NAME(x, y)         get_fd_name((x), (y), sizeof(y))
#define BUFF_GET_USER_FD_NAME(x, y)    get_user_fd_name((x), (y), sizeof(y))
#define BUFF_READ(x, y)                kjb_read((x), (y), sizeof(y))
#define BUFF_WRITE(x, y)               kjb_write((x), (y), sizeof(y))

/* ------------------------------------------------------------------------- */

typedef enum Path_type
{
    PATH_ERROR = ERROR,
    PATH_DOES_NOT_EXIST = NOT_FOUND,
    PATH_IS_REGULAR_FILE = 1,
    PATH_IS_DIRECTORY,
    PATH_IS_LINK,
    PATH_IS_PIPE,
    PATH_IS_SOCKET,
    PATH_IS_CHARACTER_SPECIAL,
    PATH_IS_BLOCK_SPECIAL,
    PATH_IS_UNCLASSIFIED
}
Path_type;

/* ------------------------------------------------------------------------- */

int set_low_level_io_options(const char* option, const char* value);

long kjb_read      (int, void*, size_t);
int  kjb_read_exact(int, void*, size_t);

int  kjb_read_2
(
    int     des,
    void*   buff,
    size_t  num_bytes,
    size_t* num_bytes_read_ptr
);

long kjb_write     (int, const void*, size_t);

int  kjb_write_2
(
    int         des,     /* File descriptor */
    const void* line,    /* Pointer to bytes to write */
    size_t      len,     /* Number of bytes to write. */
    size_t*     len_ptr  /* Number of bytes written. */
);
int safe_pipe_write
(
    int         des,       /* File descriptor */
    const void* buff,      /* Location of bytes to write.  */
    size_t      num_bytes
);

int  kjb_fread_exact(FILE*, void*, size_t);
long kjb_fread      (FILE*, void*, size_t);
int  kjb_fread_2    (FILE* fp, void* buff, size_t len, size_t* len_ptr);
long kjb_fwrite     (FILE*, const void*, size_t);

int  kjb_fwrite_2
(
    FILE*       fp,      /* File pointer */
    const void* line,    /* Pointer to bytes to write */
    size_t      len,     /* Number of bytes to write. */
    size_t*     len_ptr  /* Number of bytes written. */
);

int  kjb_safe_fflush(FILE* fp);
int  kjb_fflush     (FILE* fp);
int  kjb_ioctl      (int, IOCTL_REQUEST_TYPE request, void* arg);
int  set_no_blocking(int);
int  set_blocking   (int);

#ifdef OLD_SYSV_NON_BLOCKING
    int is_blocking(int);
#endif

long  stdin_get_line (const char*, char*, size_t);
long  fget_line       (FILE*, char*, size_t);
long  dget_line       (int, char*, size_t);
long  fput_line       (FILE*, const char*);
int   get_fd_name     (int, char*, size_t);
int   get_user_fd_name(int, char*, size_t);
int   kjb_mkdir       (const char*);
int   kjb_mkdir_2     (const char*);
int   kjb_unlink      (const char* file_name);
int   kjb_unlink_2    (const char* dir, const char* file_name);
int   kjb_rmdir       (const char*);
FILE* kjb_fopen       (const char*, const char*);
int   kjb_realpath    (const char*, char*, size_t);
FILE* kjb_freopen     (const char*, const char*, FILE*);
FILE* kjb_fdopen      (FILE*, const char*);
int   kjb_fclose      (FILE*);

#ifdef TEST          /* Currently only used in test programs.   */
    FILE* kjb_popen(const char*, const char*);
    int kjb_pclose(FILE*);
#endif

int kjb_fseek(FILE*, long, int);
long kjb_ftell(FILE*);
long kjb_fputs(FILE*, const char*);
int kjb_fgetc(FILE*);
int kjb_fputc(FILE*, int);

#ifdef LINT
/*PRINTFLIKE2*/
#endif
long kjb_fprintf(FILE*, const char*, ...);

#ifdef LINT
/*PRINTFLIKE1*/
#endif
long pso(const char*, ...);

#ifdef LINT
/*PRINTFLIKE1*/
#endif
long p_stderr(const char*, ...);

#ifdef LINT
/*PRINTFLIKE1*/
#endif
long pdo(const char*, ...);

long      kjb_vfprintf    (FILE*, const char*, va_list ap);
int       is_file         (const char* path);
int       is_directory    (const char* path);
Path_type get_path_type   (const char* path);
Path_type fp_get_path_type(FILE* fp);
int       get_file_size   (const char* file_name, off_t* size_ptr);
int       fp_get_byte_size(FILE* fp, off_t* size_ptr);
int       get_file_age    (const char* file_name, time_t* age_ptr);
int       stream_younger(time_t age_limit, const char* file_name);
int       stream_older(time_t age_limit, const char* file_name);
int       get_file_mod_time(const char* file_name, time_t *mod_time_ptr);

#ifdef TEST
    int print_open_files(FILE*);
#endif

int  kjb_isatty         (int file_des);
long print_underlined   (FILE* out_fp, const char* text_to_print);
int  start_stdout_shadow(const char* file_name);
void stop_stdout_shadow (void);
int  start_stderr_shadow(const char* file_name);
void stop_stderr_shadow (void);
void enable_stdout      (void);
void disable_stdout     (void);

int  kjb_glob
(
    Word_list** dir_files_pp,
    const char* pattern,
    int         filter_fn(const char* path)
);

int  kjb_simple_glob
(
    Word_list** dir_files_pp,
    Word_list** star_str_pp,
    const char* beg_pattern,
    const char* end_pattern,
    int         filter_fn(const char* path)
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

